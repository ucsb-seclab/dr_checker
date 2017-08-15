//
// Created by machiry on 2/12/17.
//

#ifndef PROJECT_GLOBALVARIABLERACEDETECTOR_H
#define PROJECT_GLOBALVARIABLERACEDETECTOR_H

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
     * This detector detects if a global variable is used without calling mutex.
     * GVRD
     *
     */
    class GlobalVariableRaceDetector : public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites1;
        FunctionChecker *targetChecker;
        bool is_mutex_called;
        std::set<Instruction *> visitedInstructions;
        static std::set<Function*> reportedFunctions;

        GlobalVariableRaceDetector(GlobalState &targetState, Function *toAnalyze,
                                       std::vector<Instruction *> *srcCallSites, FunctionChecker *currChecker): currState(targetState) {
            this->targetFunction = toAnalyze;
            this->currFuncCallSites1 = srcCallSites;
            this->targetChecker = currChecker;
            this->is_mutex_called = false;
            TAG = "GlobalVariableRaceDetector says:";
        }

        // check instruction, uses any global variable.
        virtual void visit(Instruction &I);

        // only function which we the current checker is interested in.
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);
    private:
        std::string TAG;
        void getAllReferencedGlobals(Instruction *currInst, std::set<llvm::GlobalVariable*> &refGlobals);

    };
}

#endif //PROJECT_GLOBALVARIABLERACEDETECTOR_H
