//
// Created by machiry on 1/8/17.
//

#ifndef PROJECT_TAINTEDSIZEDETECTOR_H
#define PROJECT_TAINTEDSIZEDETECTOR_H

#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "../VisitorCallback.h"
#include "../ModuleState.h"
#include "FunctionChecker.h"

using namespace llvm;

namespace DRCHECKER {
    /***
     * This detector detects if a tainted size is used in the copy to user or
     * copy from user functions.
     * TSD
     *
     */
    class TaintedSizeDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;
        FunctionChecker *targetChecker;

        TaintedSizeDetector(GlobalState &targetState, Function *toAnalyze,
                            std::vector<Instruction *> *srcCallSites,
                            FunctionChecker *currChecker): currState(targetState) {
            this->targetFunction = toAnalyze;
            this->currFuncCallSites = srcCallSites;
            this->targetChecker = currChecker;
            TAG = "TaintedSizeDetector says:";
        }

        // only function which we the current checker is interested in.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}

#endif //PROJECT_TAINTEDSIZEDETECTOR_H
