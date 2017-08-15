//
// Created by machiry on 2/1/17.
//

#include "bug_detectors/TaintedLoopBoundDetector.h"
#include "TaintUtils.h"

using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_TAINTED_LOOP_COND
#define ONLY_ONE_WARNING

    TaintedLoopBoundDetector::TaintedLoopBoundDetector(GlobalState &targetState, Function *toAnalyze,
                                                       std::vector<Instruction *> *srcCallSites,
                                                       FunctionChecker *currChecker): currState(targetState) {
        this->targetFunction = toAnalyze;
        this->currFuncCallSites = srcCallSites;
        // get loop exit basic blocks
        currFuncExitBlocks = GlobalState::getLoopExitBlocks(toAnalyze);
        TAG = "TaintedLoopBoundDetector says:";
    }

    void  TaintedLoopBoundDetector::visitBranchInst(BranchInst &I) {
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }
        // if there are any loops in this function?
        if(this->currFuncExitBlocks != nullptr && I.isConditional()) {
            BasicBlock *targetBB = I.getParent();
            if(this->currFuncExitBlocks->find(targetBB) != this->currFuncExitBlocks->end()) {
                // OK, the basic block is a loop exit basic block.
                // check if the branch instruction is using any tainted value
                Value *targetCondition = I.getCondition();
                std::set<TaintFlag *> *conditionTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                                                     this->currFuncCallSites,
                                                                                     targetCondition);
                if(conditionTaintInfo != nullptr) {
                    // for each of the taint flag, add Warning.
                    for(auto currTaintFlag:*conditionTaintInfo) {
                        if(currTaintFlag->isTainted()) {
                            std::string warningMsg = "Loop is bounded by a tainted value.";
                            VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites,
                                                                                         &(currTaintFlag->instructionTrace),
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
        } else {
#ifdef DEBUG_TAINTED_LOOP_COND
            dbgs() << TAG << " IGNORING:" << *(I.getParent()) << "\n";
#endif
        }
    }

    VisitorCallback* TaintedLoopBoundDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                             std::vector<Instruction *> *oldFuncCallSites,
                                                             std::vector<Instruction *> *currFuncCallSites) {
        if(!targetFunction->isDeclaration()) {
            // only if the function has source.

            TaintedLoopBoundDetector *newVis = new TaintedLoopBoundDetector(this->currState, targetFunction,
                                                                  currFuncCallSites, nullptr);

            return newVis;
        }
        return nullptr;
    }
}