//
// Created by machiry on 12/27/16.
//

#include "bug_detectors/TaintedPointerDereference.h"
#include "bug_detectors/warnings/VulnerabilityWarning.h"

using namespace llvm;

#include "TaintUtils.h"

namespace DRCHECKER {

/*#define DEBUG_LOAD_INSTR
#define DEBUG_STORE_INSTR*/
#define ONLY_ONE_WARNING
#define NO_POINTER_CHECK

    void TaintedPointerDereference::visitLoadInst(LoadInst &I) {
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }
#ifdef DEBUG_LOAD_INSTR
        dbgs() << TAG << " Visiting Load Instruction:";
        I.print(dbgs());
        dbgs() << "\n";
#endif
        Value *srcPointer = I.getPointerOperand();
        std::set<TaintFlag*> *srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                                      this->currFuncCallSites,
                                                                      srcPointer);
        if(srcTaintInfo == nullptr) {
            srcPointer = srcPointer->stripPointerCasts();
            srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                    this->currFuncCallSites,
                                                    srcPointer);
        }

        // OK, the src pointer is tainted.
        // we are trying to dereference tainted pointer.
        if(srcTaintInfo != nullptr) {
            for(TaintFlag *currFlag:*srcTaintInfo) {
                if(currFlag->isTainted()) {
                    std::string warningMsg = "Trying to read from a user pointer.";
                    VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites,
                                                                                 &(currFlag->instructionTrace),
                                                                                 warningMsg, &I,
                                                                                 TAG);
                    this->currState.addVulnerabilityWarning(currWarning);
                    if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                        this->warnedInstructions.insert(&I);
                    }
#ifdef ONLY_ONE_WARNING
                    return;
#endif
                }
            }
        }

    }

    void TaintedPointerDereference::visitStoreInst(StoreInst &I) {
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }
#ifdef DEBUG_STORE_INSTR
        dbgs() << TAG << " Visiting Store Instruction:";
        I.print(dbgs());
        dbgs() << "\n";
#endif
        Value *srcPointer = I.getPointerOperand();
        std::set<TaintFlag*> *srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                                      this->currFuncCallSites,
                                                                      srcPointer);
        if(srcTaintInfo == nullptr) {
            srcPointer = srcPointer->stripPointerCasts();
            srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                    this->currFuncCallSites,
                                                    srcPointer);
        }

        // OK, the src pointer is tainted.
        // we are trying to dereference tainted pointer.
        if(srcTaintInfo != nullptr) {
            for(TaintFlag *currFlag:*srcTaintInfo) {
                if(currFlag->isTainted()) {
                    std::string warningMsg = "Trying to write to a user pointer.";
                    VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites,
                                                                                 &(currFlag->instructionTrace),
                                                                                 warningMsg, &I,
                                                                                 TAG);
                    this->currState.addVulnerabilityWarning(currWarning);
                    if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                        this->warnedInstructions.insert(&I);
                    }
#ifdef ONLY_ONE_WARNING
                    return;
#endif
                }
            }
        }
    }

    void TaintedPointerDereference::visitGetElementPtrInst(GetElementPtrInst &I) {
#ifdef NO_POINTER_CHECK
        return;
#endif
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }


#ifdef DEBUG_STORE_INSTR
        dbgs() << TAG << " Visiting GetElementPtr Instruction:";
        I.print(dbgs());
        dbgs() << "\n";
#endif
        std::set<TaintFlag*> resultTaintFlags;
        for(unsigned i=0; i<I.getNumOperands();i++) {
            Value *currOp = I.getOperand(i);
            std::set<TaintFlag*> *srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                                          this->currFuncCallSites,
                                                                          currOp);
            if(srcTaintInfo == nullptr) {
                srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                        this->currFuncCallSites,
                                                        currOp->stripPointerCasts());
            }
            if(srcTaintInfo != nullptr) {
                resultTaintFlags.insert(srcTaintInfo->begin(), srcTaintInfo->end());
            }

        }

        // OK, the src pointer is tainted.
        // we are trying to dereference tainted pointer.
        for(TaintFlag *currFlag:resultTaintFlags) {
            if(currFlag->isTainted()) {
                std::string warningMsg = "Trying to use tainted value as index.";
                VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites,
                                                                             &(currFlag->instructionTrace),
                                                                             warningMsg, &I,
                                                                             TAG);
                this->currState.addVulnerabilityWarning(currWarning);
                if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                    this->warnedInstructions.insert(&I);
                }
#ifdef ONLY_ONE_WARNING
                return;
#endif
            }
        }
    }



    VisitorCallback* TaintedPointerDereference::visitCallInst(CallInst &I, Function *targetFunction,
                                                              std::vector<Instruction *> *oldFuncCallSites,
                                                              std::vector<Instruction *> *currFuncCallSites) {
        if(!targetFunction->isDeclaration()) {
            // only if the function has source.

            TaintedPointerDereference *newVis = new TaintedPointerDereference(this->currState, targetFunction,
                                                                              currFuncCallSites, nullptr);

            return newVis;
        }
        return nullptr;
    }


}

