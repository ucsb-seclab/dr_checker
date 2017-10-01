//
// Created by machiry on 1/30/17.
//
#include <llvm/IR/Operator.h>
#include "bug_detectors/ImproperTaintedDataUseDetector.h"
#include "bug_detectors/warnings/ImproperTaintedDataUseWarning.h"
#include "PointsToUtils.h"

using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_KERNEL_MEMORY_LEAK_DETECTOR
#define ONLY_ONE_WARNING

    VisitorCallback* ImproperTaintedDataUseDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                                   std::vector<Instruction *> *oldFuncCallSites,
                                                                   std::vector<Instruction *> *currFuncCallSites1) {
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return nullptr;
        }

        if(targetFunction->isDeclaration()) {

            FunctionChecker *currChecker = this->targetChecker;
            // ok this is a copy_to(or from)_user function?
            if(currChecker->is_copy_out_function(targetFunction) ||
               currChecker->is_taint_initiator(targetFunction)) {
                // if this is a copy_from_user method?
                // check if dst object is a heap object
                Value *sizeArg = I.getArgOperand(2);
                Range sizeRange = this->currState.getRange(sizeArg);
                if(sizeRange.isBounded()) {
                    std::set<Value *> toCheck;
                    toCheck.clear();

                    // Add destination value
                    if(currChecker->is_copy_out_function(targetFunction)) {
                        toCheck.insert(I.getArgOperand(1));
                    }
                    if(currChecker->is_taint_initiator(targetFunction)) {
                        toCheck.insert(I.getArgOperand(0));
                    }

                    std::set<AliasObject *> allObjects;
                    allObjects.clear();
                    for (auto currVal:toCheck) {
                        PointsToUtils::getAllAliasObjects(currState, oldFuncCallSites, currVal, allObjects);
                    }

                    std::set<AliasObject *> allInterestingObjects;
                    allInterestingObjects.clear();
                    // Okay constant size used to copy from user space.
                    // check if we are in a read/write function or dst object is a kmalloc
                    // had enough space.
                    if(this->currState.is_read_write_function) {
                        allInterestingObjects.insert(allObjects.begin(), allObjects.end());
                    } else {
                        // get all malloced objects which are not bounded.
                        // i.e we are copying from user using a size which is bounded into
                        // a malloced object which is not bounded i.e whose size can vary.
                        for(AliasObject *currObj:allObjects) {
                            if(currObj->isHeapObject()) {
                                Range currObjRange = this->currState.getRange(currObj->getAllocSize());
                                if(!currObjRange.isBounded()) {
                                    allInterestingObjects.insert(currObj);
                                }
                            }
                        }
                    }

                    // OK. Raise warning for each of the objects in allInterestingObjects.
                    for (AliasObject *currObj:allInterestingObjects) {
                        std::string warningMsg = "Constant size used to copy into object allocated "
                                                 "with non-constant size";
                        if(this->currState.is_read_write_function) {
                            warningMsg = "Constant size used in file read/write function";
                        }
                        std::vector<Instruction *> instructionTrace;

                        VulnerabilityWarning *currWarning = new ImproperTaintedDataUseWarning(
                                                                currObj->getObjectPtr(),
                                                                currFuncCallSites1,
                                                                &instructionTrace,
                                                                warningMsg, &I,
                                                                TAG);
                        this->currState.addVulnerabilityWarning(currWarning);
                        if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                            this->warnedInstructions.insert(&I);
                        }

                    }
                }

            } else if (targetFunction->hasName()) {
                    // string functions.
                    std::string strFunc("str");
                    std::string strNFunc("strn");
                    std::string strLFunc("strl");

                    //second argument is source
                    std::vector<std::string> second_arg_srcs;
                    second_arg_srcs.push_back("strcpy");
                    second_arg_srcs.push_back("strcat");

                    //scanf functions
                    std::string sscanfFunc("sscanf");

                    //kstrto functions
                    //std::string kstrtoFunc("kstrto");
                    //std::string simple_strto("simple_strto");

                    std::set<Value *> toCheck;
                    toCheck.clear();

                    std::string funcName = targetFunction->getName();
                    if (funcName.compare(0, strFunc.length(), strFunc) == 0) {
                        if ((funcName.compare(0, strNFunc.length(), strNFunc) == 0) ||
                            (funcName.compare(0, strLFunc.length(), strLFunc) == 0)) {
                            // these are safe functions..nothing to do.
                        } else {
                            // these are potentially risky functions.
                            // is second argument src
                            if (std::find(second_arg_srcs.begin(), second_arg_srcs.end(), funcName) !=
                                second_arg_srcs.end()) {
                                toCheck.insert(I.getArgOperand(1));

                            } else {
                                //is first argument source?
                                toCheck.insert(I.getArgOperand(0));
                            }
                        }
                    }
                    if ((funcName.compare(0, sscanfFunc.length(), sscanfFunc) == 0) /*||
                        (funcName.compare(0, kstrtoFunc.length(), kstrtoFunc) == 0) ||
                        (funcName.compare(0, simple_strto.length(), simple_strto) == 0)*/ ) {
                        // this is sscanf function. or
                        // kstr function
                        // first argument is the pointer to be checked.

                        toCheck.insert(I.getArgOperand(0));

                        Value *formatString = I.getArgOperand(1);
                        llvm::GlobalVariable *targetGlobal =dyn_cast<llvm::GlobalVariable>(formatString);
                        if(targetGlobal == nullptr) {
                            GEPOperator *gep = dyn_cast<GEPOperator>(formatString);
                            if (gep && gep->getNumOperands() > 0 && gep->getPointerOperand()) {
                                formatString = gep->getPointerOperand();
                                targetGlobal =dyn_cast<llvm::GlobalVariable>(formatString);
                            }
                        }
                        if(targetGlobal != nullptr && targetGlobal->hasInitializer()) {
                            const Constant *currConst = targetGlobal->getInitializer();
                            if(dyn_cast<ConstantDataArray>(currConst)) {
                                const ConstantDataArray *currA = dyn_cast<ConstantDataArray>(currConst);
                                if(currA->getAsString().find("%s") != std::string::npos) {
                                    std::string warningMsg = "%s used in sscanf";
                                    std::vector<Instruction *> instructionTrace;
                                    VulnerabilityWarning *currWarning = new VulnerabilityWarning(
                                            this->currFuncCallSites1,
                                            &instructionTrace,
                                            warningMsg,
                                            &I,
                                            "DangerousFormatSpecifier says:");
                                    this->currState.addVulnerabilityWarning(currWarning);
#ifdef ONLY_ONE_WARNING
                                    return nullptr;

#endif
                                } else {
                                    return nullptr;
                                }
                            }
                        }
                    }

                    std::set<PointerPointsTo*> currValPointsTo;
                    for(auto currVal:toCheck) {
                        std::set<PointerPointsTo*> *currPtsTo = PointsToUtils::getPointsToObjects(this->currState, oldFuncCallSites, currVal);
                        if(currPtsTo != nullptr) {
                            currValPointsTo.insert(currPtsTo->begin(), currPtsTo->end());
                        }
                    }

                    for(PointerPointsTo *currPtTo: currValPointsTo) {
                        std::set<TaintFlag*> *currTaintSet = currPtTo->targetObject->getFieldTaintInfo(currPtTo->dstfieldId);
                        if(currTaintSet != nullptr) {
                            for(auto currTaint:*currTaintSet) {
                                std::string warningMsg = "Tainted Data used in risky function";
                                std::vector<Instruction *> &instructionTrace = currTaint->instructionTrace;
                                VulnerabilityWarning *currWarning = new ImproperTaintedDataUseWarning(
                                        currPtTo->targetObject->getObjectPtr(),
                                        this->currFuncCallSites1,
                                        &instructionTrace,
                                        warningMsg, &I,
                                        TAG);
                                this->currState.addVulnerabilityWarning(currWarning);
                                if (this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                                    this->warnedInstructions.insert(&I);
                                }
#ifdef ONLY_ONE_WARNING
                                break;

#endif
                            }
                        }
                    }
                /*


                    std::set<AliasObject *> allObjects;
                    allObjects.clear();
                    for (auto currVal:toCheck) {
                        PointsToUtils::getAllAliasObjects(this->currState, oldFuncCallSites, currVal, allObjects);
                    }

#ifdef DEBUG_KERNEL_MEMORY_LEAK_DETECTOR
                dbgs() << TAG << ", Got:" << allObjects.size() << " that the src pointer can point to \n";
#endif

                    // This is a pointer, so get all objects that this can point to
                    for(auto currObj:allObjects) {
                        currObj->getAllPointsToObj(allObjects);
                    }

                    // check if any value in toCheck is tainted, if yes.
                    // raise a warning.

                    for (AliasObject *currObj:allObjects) {
                        if (currObj->all_contents_tainted) {
                            std::string warningMsg = "Tainted Data used in risky function";
                            std::vector<Instruction *> instructionTrace;
                            VulnerabilityWarning *currWarning = new ImproperTaintedDataUseWarning(
                                    currObj->getObjectPtr(),
                                    this->currFuncCallSites1,
                                    &instructionTrace,
                                    warningMsg, &I,
                                    TAG);
                            this->currState.addVulnerabilityWarning(currWarning);
                            if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                                this->warnedInstructions.insert(&I);
                            }
                        } else {
#ifdef DEBUG_KERNEL_MEMORY_LEAK_DETECTOR
                            dbgs() << TAG << "Ignoring object:" << *currObj->getObjectPtr()
                                            << ", as its contents are not fully tainted\n";
#endif
                        }
                    }
                    */


            }
        } else {
            // only if the function has source.

            ImproperTaintedDataUseDetector *newVis = new ImproperTaintedDataUseDetector(this->currState, targetFunction,
                                                                                        currFuncCallSites1,
                                                                                        this->targetChecker);

            return newVis;
        }
        return nullptr;
    }
}
