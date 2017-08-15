//
// Created by machiry on 1/8/17.
//
#include "bug_detectors/InvalidCastDetector.h"
#include "PointsToUtils.h"
#include "bug_detectors/warnings/InvalidCastWarning.h"

using namespace llvm;

namespace DRCHECKER {

//#define DEBUG_INVALID_CAST_DETECTOR

    void InvalidCastDetector::visitCastInst(CastInst &I) {
        Type *dstType = I.getDestTy();
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }
        // OK, are we converting into a struct pointer?
        if(dstType->isPointerTy() && dstType->getContainedType(0)->isStructTy()) {
            // get the size of the type we are converting into
            uint64_t targetSize = this->currState.getTypeSize(dstType->getContainedType(0));
            // Now, get all objects the src operand can point to.
            Value* srcOperand = I.getOperand(0);

            std::set<PointerPointsTo*>* srcPointsToInfo =
                    PointsToUtils::getPointsToObjects(this->currState, this->currFuncCallSites, srcOperand);
            if(srcPointsToInfo == nullptr) {
                // handle inline casting.
                srcOperand = srcOperand->stripPointerCasts();
            }
            srcPointsToInfo =
                    PointsToUtils::getPointsToObjects(this->currState, this->currFuncCallSites, srcOperand);

            std::set<AliasObject*> targetObjects;
            if(srcPointsToInfo != nullptr) {
                for(PointerPointsTo *currPointsTo:*srcPointsToInfo) {
                    // if this is not a dynamically created function argument object
                    // and is not already processed.
                    if(currPointsTo->targetObject->isHeapObject() &&
                            (targetObjects.find(currPointsTo->targetObject) == targetObjects.end())) {
                        targetObjects.insert(currPointsTo->targetObject);
                    }
                }
            }
            for(AliasObject *currObj:targetObjects) {
                // check for each object, if the casting is valid.
                Value *targetAllocSize = currObj->getAllocSize();
                std::vector<Instruction *> targetTrace;
                if(targetAllocSize != nullptr) {
                    // Dynamically created object.
                    Range targetRange = this->currState.getRange(targetAllocSize);
                    if(!targetRange.isBounded() || targetRange.getLower() != targetRange.getUpper()) {
                        // raise a warning: Potentially tainted size.
                        // TODO: handle taint flag and check that the taint introducing instruction
                        // can reach the current instruction.
                        VulnerabilityWarning *targetWarning = new InvalidCastWarning(currObj->getObjectPtr(), -1,
                                                                                     targetSize,
                                                                                     true, this->currFuncCallSites,
                                                                                     &targetTrace,
                                                                                     "Invalid Cast Detected",
                                                                                     &I, TAG);
                        this->currState.addVulnerabilityWarning(targetWarning);
                        if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                            this->warnedInstructions.insert(&I);
                        }
                    } else {
                        uint64_t srcObjectSize = targetRange.getLower().getZExtValue();
                        if(srcObjectSize < targetSize) {
                            //  raise a warning: Super Bad.
                            VulnerabilityWarning *targetWarning = new InvalidCastWarning(currObj->getObjectPtr(),
                                                                                         srcObjectSize, targetSize,
                                                                                         false, this->currFuncCallSites,
                                                                                         &targetTrace,
                                                                                         "Invalid Cast Detected",
                                                                                         &I, TAG);
                            this->currState.addVulnerabilityWarning(targetWarning);
                            if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                                this->warnedInstructions.insert(&I);
                            }
                        }
                    }
                } else {
                    int64_t srcObjectSize = currObj->getTypeAllocSize(this->currState.getDataLayout());
                    if(srcObjectSize != -1) {
                        if ((uint64_t)srcObjectSize < targetSize) {
                            // raise a warning: Super Bad.
                            VulnerabilityWarning *targetWarning = new InvalidCastWarning(currObj->getObjectPtr(),
                                                                                         srcObjectSize, targetSize,
                                                                                         false, this->currFuncCallSites,
                                                                                         &targetTrace,
                                                                                         "Invalid Cast Detected",
                                                                                         &I, TAG);
                            this->currState.addVulnerabilityWarning(targetWarning);
                            if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                                this->warnedInstructions.insert(&I);
                            }
                        }
                    } else {
#ifdef DEBUG_INVALID_CAST_DETECTOR
                        dbgs() << TAG << "Unable to get Size for Object:";
                        currObj->getObjectPtr()->print(dbgs());
                        dbgs() << "\n";
#endif
                    }
                }
            }
        }
    }

    VisitorCallback* InvalidCastDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                        std::vector<Instruction *> *oldFuncCallSites,
                                                        std::vector<Instruction *> *currFuncCallSites) {
        if(!targetFunction->isDeclaration()) {
            // only if the function has source, follow into the new function.

            InvalidCastDetector *newVis = new InvalidCastDetector(this->currState, targetFunction,
                                                                  currFuncCallSites, this->targetChecker);

            return newVis;
        }
        return nullptr;
    }
}
