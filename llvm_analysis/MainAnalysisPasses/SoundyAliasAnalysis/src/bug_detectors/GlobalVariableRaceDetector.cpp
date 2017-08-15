//
// Created by machiry on 2/12/17.
//
#include "bug_detectors/GlobalVariableRaceDetector.h"

using namespace llvm;
using namespace std;

namespace DRCHECKER {
    std::set<Function*> GlobalVariableRaceDetector::reportedFunctions;
    void GlobalVariableRaceDetector::visit(Instruction &I) {
        if(reportedFunctions.find(I.getParent()->getParent()) != reportedFunctions.end()) {
            return;
        }
        if(visitedInstructions.find(&I) != visitedInstructions.end()) {
            return;
        }
        CallInst *isCall = dyn_cast<CallInst>(&I);
        std::set<llvm::GlobalVariable*> usedGlobals;
        usedGlobals.clear();
        if(isCall == nullptr && !this->is_mutex_called) {
            this->getAllReferencedGlobals(&I, usedGlobals);
        }
        if(usedGlobals.size() != 0) {
            std::string warningMsg = "Trying to use a global variable without locking.";
            VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites1,
                                                                         this->currFuncCallSites1,
                                                                         warningMsg, &I,
                                                                         TAG);
            reportedFunctions.insert(I.getParent()->getParent());
            this->currState.addVulnerabilityWarning(currWarning);
        }
        visitedInstructions.insert(&I);
    }

    VisitorCallback* GlobalVariableRaceDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                               std::vector<Instruction *> *oldFuncCallSites,
                                                               std::vector<Instruction *> *currFuncCallSites) {
        if(targetFunction->isDeclaration()) {
            if(targetFunction->hasName()) {
                std::string currFunc = targetFunction->getName();
                if (currFunc.find("lock") != std::string::npos || currFunc == "up" ||
                        currFunc == "down" ||
                        currFunc.find("spin_lock") != std::string::npos ) {
                    this->is_mutex_called = true;
                }
            }
        } else {
            GlobalVariableRaceDetector *newVis = new GlobalVariableRaceDetector(this->currState, targetFunction,
                                                                              currFuncCallSites, nullptr);
            newVis->is_mutex_called = this->is_mutex_called;

            return newVis;

        }
        return nullptr;
    }

    void GlobalVariableRaceDetector::getAllReferencedGlobals(Instruction *currInst,
                                                             std::set<llvm::GlobalVariable*> &refGlobals) {
        if(currInst != nullptr) {
            if(visitedInstructions.find(currInst) != visitedInstructions.end()) {
                return;
            }
            this->visitedInstructions.insert(currInst);
            for(unsigned i=0; i< currInst->getNumOperands(); i++) {
                Value *currVal = currInst->getOperand(i);
                if(currVal != nullptr) {
                    llvm::GlobalVariable *currGlob = dyn_cast<GlobalVariable>(currVal);
                    if (currGlob != nullptr) {
                        if (refGlobals.find(currGlob) == refGlobals.end()) {
                            refGlobals.insert(currGlob);
                        }
                    } else {
                        Instruction *currInst2 = dyn_cast<Instruction>(currVal);
                        if (currInst2 != nullptr) {
                            this->getAllReferencedGlobals(currInst2, refGlobals);
                        }
                    }
                }
            }
        }
    }
}

