//
// Created by machiry on 12/28/16.
//

#ifndef PROJECT_BUGDETECTORDRIVER_H
#define PROJECT_BUGDETECTORDRIVER_H

#include <FunctionChecker.h>
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "../ModuleState.h"
#include "../VisitorCallback.h"

using namespace llvm;

namespace DRCHECKER {
    /***
     * Base class that handles all the bug detectors.
     *
     */
    class BugDetectorDriver {
    public:

        /***
         * Add bug detectors that need to be run before any analysis passes.
         * For ex: InvalidCastDetector, because once the analysis runs, it could change the type of the object.
         *
         * @param targetState GlobalState to operate on.
         * @param toAnalyze Function that is being analyzed.
         * @param srcCallSites pointer to the list of callSites (Context).
         * @param allCallbacks list of all callbacks, this is where bug detector visitors will be added.
         * @param targetChecker Function checker implementation to be used by the bug detectors.
         */
        static void addPreAnalysisBugDetectors(GlobalState &targetState,
                                               Function *toAnalyze,
                                               std::vector<Instruction *> *srcCallSites,
                                               std::vector<VisitorCallback *> *allCallbacks,
                                               FunctionChecker *targetChecker);


        /***
         *  Add bug detectors that need to be run after analysis passes.
         *
         * @param targetState GlobalState to operate on.
         * @param toAnalyze Function that is being analyzed.
         * @param srcCallSites pointer to the list of callSites (Context).
         * @param allCallbacks list of all callbacks, this is where bug detector visitors will be added.
         * @param targetChecker Function checker object that should be used by the created bug detectors.
         */
        static void addPostAnalysisBugDetectors(GlobalState &targetState,
                                               Function *toAnalyze,
                                               std::vector<Instruction *> *srcCallSites,
                                               std::vector<VisitorCallback *> *allCallbacks,
                                               FunctionChecker *targetChecker);
        
        

        /***
         * Print all the warnings from the provided global state.
         * @param targetState State that contains all the warnings.
         * @param O output stream to which the warnings should be written to
         */
        static void printAllWarnings(GlobalState &targetState, llvm::raw_ostream& O);

        /***
         * Print all the warnings by indexed by instruction.
         * @param targetState State that contains all the warnings.
         * @param O output stream to which the warnings should be written to
         */
        static void printWarningsByInstr(GlobalState &targetState, llvm::raw_ostream& O);
    };
}

#endif //PROJECT_BUGDETECTORDRIVER_H
