//
// Created by machiry on 1/6/17.
//
#include "bug_detectors/KernelUninitMemoryLeakDetector.h"
#include "bug_detectors/warnings/KernelUninitMemoryLeakWarning.h"
#include "PointsToUtils.h"

using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_KERNEL_MEMORY_LEAK_DETECTOR

    VisitorCallback* KernelUninitMemoryLeakDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                                   std::vector<Instruction *> *oldFuncCallSites,
                                                                   std::vector<Instruction *> *newFuncCallSites) {
        if(targetFunction->isDeclaration()) {

            // warning already raised for this instruction.
            if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
                return nullptr;
            }

            FunctionChecker *currChecker = this->targetChecker;
            // ok we are calling memset/memcpy/copy_from_user function on an object.
            if(currChecker->is_memset_function(targetFunction) ||
               currChecker->is_memcpy_function(targetFunction) ||
               currChecker->is_taint_initiator(targetFunction)) {
                Value *targetObj = I.getArgOperand(0);
                std::set<PointerPointsTo*> *pointsToObj =
                        PointsToUtils::getPointsToObjects(currState, oldFuncCallSites, targetObj);
                if(pointsToObj == nullptr) {
                    pointsToObj =
                            PointsToUtils::getPointsToObjects(currState, oldFuncCallSites,
                                                              targetObj->stripPointerCasts());
                }
                if(pointsToObj == nullptr) {
                    // ok, the pointer doesn't point to any objects.
                    // raise a warning.
#ifdef DEBUG_KERNEL_MEMORY_LEAK_DETECTOR
                    errs() << TAG << " Pointer:";
                    targetObj->print(errs());
                    errs() << "\n";
                    errs() << " does not point to any object. Ignoring\n";
#endif
                } else {
                    std::set<AliasObject*> targetObjects;
                    targetObjects.clear();
                    // get all objects.
                    for(PointerPointsTo *currObj:*pointsToObj) {
                        if(targetObjects.find(currObj->targetObject) == targetObjects.end()) {
                            targetObjects.insert(currObj->targetObject);
                            currObj->targetObject->is_initialized = true;
                            currObj->targetObject->initializingInstructions.insert(&I);
                        }
                    }
                }
            }

            if(currChecker->is_copy_out_function(targetFunction)) {
                Value *targetObj = I.getArgOperand(1);
                //check if targetObj is uninitialized? if yes, raise a warning.
                std::set<PointerPointsTo*> *pointsToObj =
                        PointsToUtils::getPointsToObjects(currState, oldFuncCallSites, targetObj);
                if(pointsToObj == nullptr) {
                    pointsToObj =
                            PointsToUtils::getPointsToObjects(currState, oldFuncCallSites,
                                                              targetObj->stripPointerCasts());
                }
                if(pointsToObj == nullptr) {
                    errs() << "Trying to copy from an object which does not point to any object.\n";
                } else {
                    std::set<AliasObject*> targetObjects;
                    targetObjects.clear();
                    // get all uninitialized objects.
                    for(PointerPointsTo *currObj:*pointsToObj) {
                        if(!currObj->targetObject->is_initialized) {
                            if (targetObjects.find(currObj->targetObject) == targetObjects.end()) {
                                targetObjects.insert(currObj->targetObject);
                            }
                        }
                    }
                    for(AliasObject *currObj:targetObjects) {

                        std::string warningMsg = "Potentially leaking memory on kernel stack (via padded fields) "
                                "into user space";
                        std::vector<Instruction *> instructionTrace;

                        VulnerabilityWarning *currWarning = new KernelUninitMemoryLeakWarning(currObj->getObjectPtr(),
                                                                                              this->currFuncCallSites1,
                                                                                              &instructionTrace,
                                                                                              warningMsg, &I,
                                                                                              TAG);
                        this->currState.addVulnerabilityWarning(currWarning);
                        if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                            this->warnedInstructions.insert(&I);
                        }
                    }

                }
            }
        }

        if(!targetFunction->isDeclaration()) {
            // only if the function has source.

            KernelUninitMemoryLeakDetector *newVis = new KernelUninitMemoryLeakDetector(this->currState, targetFunction,
                                                                                        newFuncCallSites, this->targetChecker);

            return newVis;
        }
        return nullptr;
    }
}
