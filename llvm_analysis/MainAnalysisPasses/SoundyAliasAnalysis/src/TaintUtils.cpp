//
// Created by machiry on 12/27/16.
//

#include "TaintUtils.h"

using namespace llvm;

namespace DRCHECKER {
    std::set<TaintFlag*>* TaintUtils::getTaintInfo(GlobalState &currState,
                                                   std::vector<Instruction *> *currFuncCallSites,
                                                   Value *targetVal) {
        // get total taint information for the context.
        std::map<Value *, std::set<TaintFlag*>*> *contextTaintInfo = currState.getTaintInfo(currFuncCallSites);
        // check if taint flags exits for the provided value?
        // if yes fetch it.
        if(contextTaintInfo->find(targetVal) != contextTaintInfo->end()) {
            return (*contextTaintInfo)[targetVal];
        }
        // else return null
        return nullptr;
    }

    void TaintUtils::updateTaintInfo(GlobalState &currState,
                                     std::vector<Instruction *> *currFuncCallSites,
                                     Value *targetVal,
                                     std::set<TaintFlag*> *targetTaintInfo) {

        std::set<TaintFlag*> *existingTaintInfo = TaintUtils::getTaintInfo(currState, currFuncCallSites, targetVal);
        // if there exists no previous taint info.
        if(existingTaintInfo == nullptr) {
            // get total taint information for the context.
            std::map<Value *, std::set<TaintFlag*>*> *contextTaintInfo = currState.getTaintInfo(currFuncCallSites);
            (*contextTaintInfo)[targetVal] = targetTaintInfo;
            return;
        }

        // ok there exists previous taint info.
        // check that for every taint flag if it is
        // already present? if yes, do not insert else insert
        for(auto currTaintFlag:(*targetTaintInfo)) {
            TaintUtils::addNewTaintFlag(existingTaintInfo, currTaintFlag);
        }
        // free the vector
        delete(targetTaintInfo);

    }

    void TaintUtils::addNewTaintFlag(std::set<TaintFlag*> *newTaintInfo, TaintFlag *newTaintFlag) {
        // check if the set already contains same taint?
        if(std::find_if(newTaintInfo->begin(), newTaintInfo->end(), [newTaintFlag](const TaintFlag *n) {
            return  n->isTaintEquals(newTaintFlag);
        }) == newTaintInfo->end()) {
            // if not insert the new taint flag into the newTaintInfo.
            newTaintInfo->insert(newTaintInfo->end(), newTaintFlag);
        } else {
            delete(newTaintFlag);
        }
    }
}