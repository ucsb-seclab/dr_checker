//
// Created by machiry on 12/27/16.
//

#ifndef PROJECT_TAINTEDPOINTERDEREFERENCE_H
#define PROJECT_TAINTEDPOINTERDEREFERENCE_H

#include <FunctionChecker.h>
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "../VisitorCallback.h"
#include "../ModuleState.h"

using namespace llvm;

namespace DRCHECKER {

    /***
     * This detector detects if a store or load is trying to write to or
     * read from tainted address.
     * TPDD
     *
     */
    class TaintedPointerDereference : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;

        TaintedPointerDereference(GlobalState &targetState, Function *toAnalyze,
                                  std::vector<Instruction *> *srcCallSites,
                                  FunctionChecker *currChecker): currState(targetState) {
            this->targetFunction = toAnalyze;
            this->currFuncCallSites = srcCallSites;
            TAG = "TaintedPointerDereferenceChecker says:";
        }

        // only function which we the current checker is interested in.
        virtual void visitLoadInst(LoadInst &I);
        virtual void visitStoreInst(StoreInst &I);
        virtual void visitGetElementPtrInst(GetElementPtrInst &I);
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}

#endif //PROJECT_TAINTEDPOINTERDEREFERENCE_H
