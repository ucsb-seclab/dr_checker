//
// Created by machiry on 1/5/17.
//

#ifndef PROJECT_KERNELMEMORYLEAKDETECTOR_H
#define PROJECT_KERNELMEMORYLEAKDETECTOR_H

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
     * This detector detects if an object, that is not fully initialized (i.e not memset)
     * is copied into user space.
     * ULD
     */
    class KernelUninitMemoryLeakDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites1;
        FunctionChecker *targetChecker;

        KernelUninitMemoryLeakDetector(GlobalState &targetState, Function *toAnalyze,
                    std::vector<Instruction *> *srcCallSites, FunctionChecker *currChecker): currState(targetState) {
            this->targetFunction = toAnalyze;
            this->currFuncCallSites1 = srcCallSites;
            this->targetChecker = currChecker;
            TAG = "KernelUninitMemoryLeakDetector says:";
        }

        // only function which we the current checker is interested in.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;

    };
}

#endif //PROJECT_KERNELMEMORYLEAKDETECTOR_H
