//
// Created by machiry on 1/31/17.
//

#ifndef PROJECT_INTEGEROVERFLOWDETECTOR_H
#define PROJECT_INTEGEROVERFLOWDETECTOR_H

#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "../ModuleState.h"
#include "FunctionChecker.h"
#include "VisitorCallback.h"

using namespace llvm;

namespace DRCHECKER {
    /***
     * This is an integer overflow detector.
     *
     * It detects, if a tainted value is used in operations
     * that result in integer overflow.
     * TAD
     */
    class IntegerOverflowDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;
        FunctionChecker *targetChecker;

        IntegerOverflowDetector(GlobalState &targetState, Function *toAnalyze,
                            std::vector<Instruction *> *srcCallSites,
                            FunctionChecker *currChecker): currState(targetState) {
            this->targetFunction = toAnalyze;
            this->currFuncCallSites = srcCallSites;
            this->targetChecker = currChecker;
            TAG = "IntegerOverflowDetector says:";
        }

        // visit binary op instruction, this is where we find potential integer overflows.
        virtual void visitBinaryOperator(BinaryOperator &I);

        // Handle call instruction to continue into called function.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}


#endif //PROJECT_INTEGEROVERFLOWDETECTOR_H
