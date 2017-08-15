//
// Created by machiry on 1/30/17.
//

#ifndef PROJECT_IMPROPERTAINTEDDATAUSEDETECTOR_H
#define PROJECT_IMPROPERTAINTEDDATAUSEDETECTOR_H

#include <FunctionChecker.h>
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "../VisitorCallback.h"
#include "../ModuleState.h"
#include "warnings/VulnerabilityWarning.h"

using namespace llvm;

namespace DRCHECKER {

    /***
     * This detector detects if a store or load is trying to write to or
     * read from tainted address.
     * ITDUD
     *
     */
    class ImproperTaintedDataUseDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites1;
        FunctionChecker *targetChecker;

        ImproperTaintedDataUseDetector(GlobalState &targetState, Function *toAnalyze,
                std::vector<Instruction *> *srcCallSites, FunctionChecker *currChecker): currState(targetState) {
                this->targetFunction = toAnalyze;
                this->currFuncCallSites1 = srcCallSites;
                this->targetChecker = currChecker;
                TAG = "ImproperTaintedDataUseDetector says:";
        }

        // only function which we the current checker is interested in.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}

#endif //PROJECT_IMPROPERTAINTEDDATAUSEDETECTOR_H
