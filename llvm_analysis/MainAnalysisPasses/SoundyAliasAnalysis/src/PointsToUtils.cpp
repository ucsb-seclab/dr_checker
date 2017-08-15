//
// Created by machiry on 12/6/16.
//

#include "PointsToUtils.h"

using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_FUNCTION_PTR_ALIASING

    std::set<PointerPointsTo*>* PointsToUtils::getPointsToObjects(GlobalState &currState,
                                                                  std::vector<Instruction *> *currFuncCallSites,
                                                                  Value *srcPointer) {
        // Get points to objects set of the srcPointer at the entry of the instruction
        // currInstruction.
        // Note that this is at the entry of the instruction. i.e INFLOW.
        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = currState.getPointsToInfo(currFuncCallSites);
        // Here srcPointer should be present in points to map.
        if(targetPointsToMap->find(srcPointer) != targetPointsToMap->end()) {
            return (*targetPointsToMap)[srcPointer];
        }
        return nullptr;

    }

    bool PointsToUtils::hasPointsToObjects(GlobalState &currState,
                                           std::vector<Instruction *> *currFuncCallSites,
                                           Value *srcPointer) {
        /***
         * Check if the srcPointer has any pointto objects at currInstruction
         */
        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = currState.getPointsToInfo(currFuncCallSites);
        return targetPointsToMap != nullptr &&
               targetPointsToMap->find(srcPointer) != targetPointsToMap->end();
    }

    bool PointsToUtils::getTargetFunctions(GlobalState &currState, std::vector<Instruction *> *currFuncCallSites,
                                           Value *srcPointer, std::vector<Function *> &dstFunctions) {
        // first get the type of the function we are looking for.
        Type *targetFunctionType = srcPointer->getType();
        bool retVal = false;

        // get points to information, handling casts.
        std::set<PointerPointsTo*>* basePointsTo = PointsToUtils::getPointsToObjects(currState, currFuncCallSites,
                                                                                     srcPointer);
        if(basePointsTo == nullptr) {
            basePointsTo = PointsToUtils::getPointsToObjects(currState, currFuncCallSites,
                                                             srcPointer->stripPointerCasts());
        }

        // OK, we have some points to information for the srcPointer.
        if(basePointsTo != nullptr) {
            for(PointerPointsTo *currPointsTo: *(basePointsTo)) {
                // OK, this is a global object.
                if(currPointsTo->targetObject->isGlobalObject()) {
                    // Check if it is function.
                    GlobalObject *currGlobObj = (GlobalObject*)(currPointsTo->targetObject);
                    Function *targetFunction = dyn_cast<Function>(currGlobObj->targetVar);
                    if(targetFunction != nullptr) {
                        if (targetFunction->getType() == targetFunctionType) {
                            retVal = true;
                            dstFunctions.push_back(targetFunction);
#ifdef DEBUG_FUNCTION_PTR_ALIASING
                            dbgs() << "Found Target Function:" << targetFunction->getName() << " for Pointer:";
                            srcPointer->print(dbgs());
                            dbgs() << "\n";
#endif
                        } else {
#ifdef DEBUG_FUNCTION_PTR_ALIASING
                            dbgs()
                                    << "Function pointer points to a function whose type does not match the pointer type.\n";
                            dbgs() << "Pointer Type:";
                            targetFunctionType->print(dbgs());
                            dbgs() << "\n";
                            dbgs() << "Destination Function Type:";
                            targetFunction->getType()->print(dbgs());
                            dbgs() << "\n";
#endif

                        }
                    }
                }
            }
        }
        return retVal;
    }

    bool PointsToUtils::getAllAliasObjects(GlobalState &currState, std::vector<Instruction *> *currFuncCallSites,
                            Value *srcPointer,
                            std::set<AliasObject*> &dstObjects) {
        std::set<PointerPointsTo*>* pointsToSet = PointsToUtils::getPointsToObjects(currState,
                                                                                    currFuncCallSites, srcPointer);
        bool addedAtleastOne = false;
        if(pointsToSet != nullptr) {
            for(auto currp:*pointsToSet) {
                // if the current object is not present?
                // then add the object into the dstObjects.
                if(dstObjects.find(currp->targetObject) == dstObjects.end()) {
                    dstObjects.insert(currp->targetObject);
                    addedAtleastOne = true;
                }
            }
        }
        return addedAtleastOne;
    }

    bool PointsToUtils::getPossibleFunctionTargets(CallInst &callInst, std::vector<Function *> &targetFunctions) {
        FunctionType *targetFunctionType = callInst.getFunctionType();
        Module *currModule = callInst.getParent()->getParent()->getParent();
        for(auto a = currModule->begin(), b = currModule->end(); a != b; a++) {
            Function *currFunction = &(*a);
            // does the current function has same type of the call instruction?
            if(!currFunction->isDeclaration() && currFunction->getFunctionType() == targetFunctionType) {
                // if yes, see if the function is used in non-call instruction.
                for (Value::user_iterator i = currFunction->user_begin(), e = currFunction->user_end(); i != e; ++i) {
                    Instruction *currI = dyn_cast<Instruction>(*i);
                    CallInst *currC = dyn_cast<CallInst>(*i);
                    if(currI != nullptr && currC == nullptr) {
                        // oh the function is used in a non-call instruction.
                        // potential target, insert into potential targets
                        targetFunctions.push_back(currFunction);
                        break;
                    }
                }
            }
        }

        // Find only those functions which are part of the driver.
        DILocation *instrLoc = nullptr;
        instrLoc = callInst.getDebugLoc().get();
        if(instrLoc != nullptr) {
            std::string currFileName = instrLoc->getFilename();
            size_t found = currFileName.find_last_of("/");
            std::string parFol = currFileName.substr(0, found);
            std::vector<Function *> newList;
            for(auto cf:targetFunctions) {
                instrLoc = cf->getEntryBlock().getFirstNonPHIOrDbg()->getDebugLoc().get();
                if(instrLoc != nullptr) {
                    currFileName = instrLoc->getFilename();
                    if(currFileName.find(parFol) != std::string::npos) {
                        newList.push_back(cf);
                    }
                }
            }
            targetFunctions.clear();
            targetFunctions.insert(targetFunctions.end(), newList.begin(), newList.end());

        } else {
            targetFunctions.clear();
        }

        return targetFunctions.size() > 0;
    }


};