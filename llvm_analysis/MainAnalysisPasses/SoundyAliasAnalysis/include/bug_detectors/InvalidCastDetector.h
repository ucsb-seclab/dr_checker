//
// Created by machiry on 1/8/17.
//

#ifndef PROJECT_INVALIDCASTDETECTOR_H
#define PROJECT_INVALIDCASTDETECTOR_H

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
     * This is an invalid cast detector.
     *
     * It detects, if a malloced memory is wrongly casted into type whose size
     * is different than the size of the malloced object.
     * ICD
     */
    class InvalidCastDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;
        FunctionChecker *targetChecker;

        InvalidCastDetector(GlobalState &targetState, Function *toAnalyze,
                std::vector<Instruction *> *srcCallSites,
        FunctionChecker *currChecker): currState(targetState) {
                this->targetFunction = toAnalyze;
                this->currFuncCallSites = srcCallSites;
                this->targetChecker = currChecker;
                TAG = "InvalidCastDetector says:";
        }

        // visit cast instruction, this is where we find invalid casts.
        virtual void visitCastInst(CastInst &I);

        // Handle call instruction to continue into called function.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}

#endif //PROJECT_INVALIDCASTDETECTOR_H
