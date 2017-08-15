//
// Created by machiry on 12/27/16.
//

#ifndef PROJECT_TAINTUTILS_H
#define PROJECT_TAINTUTILS_H

#include <set>
#include "ModuleState.h"
#include "TaintInfo.h"

using namespace llvm;

namespace DRCHECKER {
    /***
     * Class that implements common functions related to retrieving taint information.
     */
    class TaintUtils {

    public:

        /***
         * Get set of taintinfo objects of the provided value.
         *
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context (list of call sites) of the analysis.
         * @param targetVal Value whose taint information need to be fetched.
         * @return Set of taintflags of the provided value.
         */
        static std::set<TaintFlag*>* getTaintInfo(GlobalState &currState,
                                           std::vector<Instruction *> *currFuncCallSites,
                                           Value *targetVal);

        /***
         * Update taint into of the provided targetVal on the provided state at given context.
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context (list of call sites) of the analysis.
         * @param targetVal Value whose taint information need to be updated.
         * @param targetTaintInfo TaintInfo that needs to be updated.
         */
        static void updateTaintInfo(GlobalState &currState,
                                    std::vector<Instruction *> *currFuncCallSites,
                                    Value *targetVal,
                                    std::set<TaintFlag*> *targetTaintInfo);
        /***
         * Add taint flag into the provided set.
         * @param newTaintInfo Set to which taintflag need to be added.
         * @param newTaintFlag new taint flag to be added.
         */
        static void addNewTaintFlag(std::set<TaintFlag*> *newTaintInfo, TaintFlag *newTaintFlag);


    };
}

#endif //PROJECT_TAINTUTILS_H
