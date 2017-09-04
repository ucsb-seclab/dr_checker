//
// Created by machiry on 10/25/16.
//

#ifndef PROJECT_MODULESTATE_H
#define PROJECT_MODULESTATE_H
#include "AliasObject.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "TaintInfo.h"
#include "bug_detectors/warnings/VulnerabilityWarning.h"
#include "RangeAnalysis.h"
#include <set>


using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_GLOBALS
    /***
     * Class that abstracts the context.
     * Definition 3.5 of the paper.
     */
    class AnalysisContext {
    public:
        std::vector<Instruction *> *callSites;
        void printContext(llvm::raw_ostream& O) {
            //O << "\n  Call Context:";
            O << "\"context\":[";
            bool putComma = false;
            //std::string str;
            for(Instruction *currCallSite:*(this->callSites)) {
                //O << "   ";
                if(putComma) {
                    O << ",";
                }

                //llvm::raw_string_ostream rso(str);
                //currCallSite->print(rso);
                //std::string instrSt = InstructionUtils::escapeJsonString(rso.str());

                O << "{\"instr\":\"";
                //currCallSite->print(O);
                //currCallSite->print(O);
                //DILocation *instrLoc = currCallSite->getDebugLoc().get();
                DILocation *instrLoc = InstructionUtils::getCorrectInstrLocation(currCallSite);
                O << InstructionUtils::escapeValueString(currCallSite) << "\",";
                O << "\"lineno\":";
                if (instrLoc != nullptr) {
                    //O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
                    O << instrLoc->getLine() << ",\"file\":\"";
                    O << InstructionUtils::escapeJsonString(instrLoc->getFilename()) << "\"";
                } else {
                    O << "-1";
                }
                O << "}\n";
                putComma = true;
            }
            O << "\n]";
        }
    };

    /***
     *  Object which represents GlobalState.
     *  Everything we need in one place.
     *  Refer Fig1 in the paper.
     *  It contains pointsTo, globalVariables and TaintInformation.
     */
    class GlobalState {
    public:

        // map containing analysis context to corresponding vulnerability warnings.
        std::map<AnalysisContext*, std::set<VulnerabilityWarning*>*> allVulnWarnings;

        // map containing vulnerability warnings w.r.t instruction.
        std::map<Instruction*, std::set<VulnerabilityWarning*>*> warningsByInstr;
        // set containing all analysis contexts that were analyzed using this global state
        std::set<AnalysisContext*> availableAnalysisContexts;

        // Range analysis results.
        RangeAnalysis *range_analysis;

        //is the current function being analyzed read/write?
        bool is_read_write_function = false;

        // Map, which contains at each instruction.
        // set of objects to which the pointer points to.
        // Information needed for AliasAnalysis
        std::map<AnalysisContext*, std::map<Value *, std::set<PointerPointsTo*>*>*> pointToInformation;
        static std::map<Value *, std::set<PointerPointsTo*>*> globalVariables;

        static std::map<Function *, std::set<BasicBlock*>*> loopExitBlocks;


        // Data layout for the current module
        DataLayout *targetDataLayout;

        // Information needed for TaintAnalysis
        std::map<AnalysisContext*, std::map<Value *, std::set<TaintFlag*>*>*> taintInformation;

        GlobalState(RangeAnalysis *ra, DataLayout *currDataLayout) {
            this->range_analysis = ra;
            this->targetDataLayout = currDataLayout;
        }

        ~GlobalState() {
            cleanup();

        }

        void cleanup() {
            // clean up
            std::set<AliasObject*> deletedObjects;
            // all global variables.
            for(auto glob_iter = globalVariables.begin(); glob_iter != globalVariables.end(); glob_iter++) {
                auto targetPointsTo = glob_iter->second;
                for(auto currPointsTo: *targetPointsTo) {
                    auto targetAliasObj = currPointsTo->targetObject;
                    if(deletedObjects.find(targetAliasObj) == deletedObjects.end()) {
                        deletedObjects.insert(targetAliasObj);
                        delete(targetAliasObj);
                    }
                    delete(currPointsTo);
                }
                delete(targetPointsTo);
            }
            globalVariables.clear();

            // all pointsToInformation
            for(auto ptInfo = pointToInformation.begin(); ptInfo != pointToInformation.end(); ptInfo++) {
                for(auto pointsTo_iter = ptInfo->second->begin(); pointsTo_iter != ptInfo->second->begin();
                    pointsTo_iter++) {
                    auto targetPointsTo = pointsTo_iter->second;
                    for(auto currPointsTo: *targetPointsTo) {
                        auto targetAliasObj = currPointsTo->targetObject;
                        if(deletedObjects.find(targetAliasObj) == deletedObjects.end()) {
                            deletedObjects.insert(targetAliasObj);
                            delete(targetAliasObj);
                        }
                        delete(currPointsTo);
                    }
                    delete(targetPointsTo);
                }
            }
            pointToInformation.clear();
        }

        /***
         * Get the DataLayout for the current module being analyzed.
         * @return pointer to the DataLayout*
         */
        DataLayout* getDataLayout() {
            return this->targetDataLayout;
        }

        /***
         * Get the type size for the provided type.
         * @param currType Type for which size needs to fetched.
         * @return uint64_t representing size of the type.
         */
        uint64_t getTypeSize(Type *currType) {
            if(currType->isSized()) {
                return this->getDataLayout()->getTypeAllocSize(currType);
            }
            return 0;
        }


        /***
         * Get the AliasObject referenced by the currVal.
         *
         * @param currVal Value whose reference needs to be fetched.
         * @param globalObjectCache Map containing values and corresponding
         *                          AliasObject.
         * @return Corresponding AliasObject.
         */
        static AliasObject* getReferencedGlobal(std::vector<llvm::GlobalVariable *> &visitedCache, Value *currVal,
                                                std::map<Value*, AliasObject*> &globalObjectCache) {

            llvm::GlobalVariable *actualGlobal = dyn_cast<llvm::GlobalVariable>(currVal);
            if (actualGlobal == nullptr) {
                // OK, check with stripped.
                Value *strippedVal = currVal->stripPointerCasts();
                actualGlobal = dyn_cast<llvm::GlobalVariable>(strippedVal);
            }

            if (actualGlobal == nullptr) {
                ConstantExpr *targetExpr = dyn_cast<ConstantExpr>(currVal);
                if (targetExpr != nullptr) {
                    AliasObject *refObj = nullptr;
                    // Even stripping din't help. Check if this is an instruction and get the first
                    // global variable in operand list
                    for (unsigned int i = 0; i < targetExpr->getNumOperands(); i++) {
                        Value *currOperand = targetExpr->getOperand(i);
                        llvm::GlobalVariable *globalCheck = dyn_cast<llvm::GlobalVariable>(currOperand);
                        if (globalCheck == nullptr) {
                            // check with strip
                            globalCheck = dyn_cast<llvm::GlobalVariable>(currOperand->stripPointerCasts());
                        }
                        if (globalCheck != nullptr) {
                            actualGlobal = globalCheck;
                            break;
                        }
                        refObj = getReferencedGlobal(visitedCache, currOperand, globalObjectCache);
                        if(refObj != nullptr) {
                            return refObj;
                        }
                    }

                }
            }

            if (actualGlobal == nullptr) {
                Function *targetFunction = dyn_cast<Function>(currVal);
                if(targetFunction != nullptr) {
                    if (globalObjectCache.find((Value *) targetFunction) != globalObjectCache.end()) {
                        return globalObjectCache[(Value *) targetFunction];
                    }
                }
            }
            if(actualGlobal != nullptr) {
                return addGlobalVariable(visitedCache, actualGlobal, globalObjectCache);
            }
            return nullptr;
        }

        /***
         *  Check if the Constant is a constant variable. ie. it uses
         *  some global variables.
         * @param targetConstant Constant to check
         * @return true/false depending on whether the constant
         *         references global variable.
         */
        static bool isConstantVariable(Constant *targetConstant) {
            Function* functionCheck = dyn_cast<Function>(targetConstant);
            if(functionCheck) {
                return true;
            }
            llvm::GlobalVariable *globalCheck = dyn_cast<llvm::GlobalVariable>(targetConstant);
            if(globalCheck) {
                return true;
            }
            ConstantExpr *targetExpr = dyn_cast<ConstantExpr>(targetConstant);
            if(targetExpr) {
                return true;
            }
            return false;
        }


        /***
         *  Get the global object from variable initializers.
         * @param constantType Type of the constant.
         * @param targetConstant Constant for which AliasObject needs to be created.
         * @param globalObjectCache Cache containing value to AliasObject.
         * @return Alias Object corresponding to the initializer.
         */
        static AliasObject* getGlobalObjectFromInitializer(std::vector<llvm::GlobalVariable *> &visitedCache,
                                                           Type* constantType, Constant* targetConstant,
                                                     std::map<Value*, AliasObject*> &globalObjectCache) {
            AliasObject *glob = nullptr;
            if(constantType->isStructTy()) {
                glob = new GlobalObject(targetConstant, constantType);
                ConstantStruct *actualStType = dyn_cast<ConstantStruct>(targetConstant);
                if(actualStType != nullptr) {
                    for (unsigned int i = 0; i < actualStType->getNumOperands(); i++) {
                        Value *currFieldVal = actualStType->getOperand(i);
                        AliasObject *currFieldObj = nullptr;
                        Constant *constCheck = dyn_cast<Constant>(currFieldVal);
                        if(isConstantVariable(constCheck)) {
                            // OK, the field is initialized but it is not a constant?
                            // check if this is a global variable?
                            currFieldObj = getReferencedGlobal(visitedCache, currFieldVal, globalObjectCache);
                        }
                        else {
                            // the field is initialized with constant?
                            currFieldObj = getGlobalObjectFromInitializer(visitedCache,
                                                                          currFieldVal->getType(), constCheck,
                                                                          globalObjectCache);
                        }
                        if (currFieldObj != nullptr) {
                            glob->addObjectToFieldPointsTo(i, currFieldObj, nullptr);
                        }
                    }
                }

            } else if(constantType->isAggregateType()) {
                glob = new GlobalObject(targetConstant, constantType);
            }
            return glob;

        }


        /***
         * Add global variable into the global state and return corresponding AliasObject.
         *
         * Handles global variables in a rather complex way.
         * A smart person should implement this in a better way.
         *
         *
         * @param globalVariable Global variable that needs to be added.
         * @param globalObjectCache Cache of Values to corresponding AliasObject.
         * @return AliasObject corresponding to the global variable.
         */
        static AliasObject* addGlobalVariable(std::vector<llvm::GlobalVariable *> &visitedCache,
                                              llvm::GlobalVariable *globalVariable,
                                      std::map<Value*, AliasObject*> &globalObjectCache) {

            if(std::find(visitedCache.begin(), visitedCache.end(), globalVariable) != visitedCache.end()) {
#ifdef DEBUG_GLOBALS
                dbgs() << "Cycle Detected for:";
                globalVariable->print(dbgs());
                dbgs() << "\n";
#endif
                return nullptr;
            }

            Value *objectCacheKey = dyn_cast<Value>(globalVariable);
            AliasObject *toRet = nullptr;
            assert(objectCacheKey != nullptr);
            // if its already processed? Return previously created object.
            if(globalObjectCache.find(objectCacheKey) != globalObjectCache.end()) {
                return globalObjectCache[objectCacheKey];
            } else {

                visitedCache.push_back(globalVariable);

                // OK, the global variable has no initializer.
                // Just create a default object.
                std::set<PointerPointsTo *> *newPointsTo = new std::set<PointerPointsTo *>();


                // This is new global variable.
                Type *baseType = globalVariable->getType();
                // global variables are always pointers
                assert(baseType->isPointerTy());
                // next check if it has any initializers.
                if (globalVariable->hasInitializer()) {
                    Constant *baseInitializer = globalVariable->getInitializer();
                    toRet = getGlobalObjectFromInitializer(visitedCache, baseInitializer->getType(),
                                                           baseInitializer, globalObjectCache);

                }

                if(toRet == nullptr) {
                    // OK, the global variable has no initializer.
                    // Just create a default object.
                    toRet = new GlobalObject(globalVariable, baseType->getContainedType(0));
                }

                PointerPointsTo *pointsToObj = new PointerPointsTo();
                pointsToObj->targetObject = toRet;
                pointsToObj->fieldId = pointsToObj->dstfieldId = 0;
                pointsToObj->propogatingInstruction = globalVariable;
                pointsToObj->targetPointer = globalVariable;
                newPointsTo->insert(newPointsTo->end(), pointsToObj);
                assert(GlobalState::globalVariables.find(globalVariable) == GlobalState::globalVariables.end());
                GlobalState::globalVariables[globalVariable] = newPointsTo;
                //dbgs() << "Adding:" << *globalVariable << " into cache\n";
            }
            // make sure that object cache doesn't already contain the object.
            assert(globalObjectCache.find(objectCacheKey) == globalObjectCache.end());
            // insert into object cache.
            globalObjectCache[objectCacheKey] = toRet;
            // Make sure that we have created a pointsTo information for globals.
            assert(GlobalState::globalVariables.find(globalVariable) != GlobalState::globalVariables.end());
            assert(GlobalState::globalVariables[globalVariable] != nullptr);
            visitedCache.pop_back();
            return toRet;

        }

        /***
         * Add global function into GlobalState.
         * @param currFunction Function that needs to be added.
         * @param globalObjectCache Map of values and corresponding AliasObject.
         */
        static void addGlobalFunction(Function *currFunction, std::map<Value*, AliasObject*> &globalObjectCache) {
            // add to the global cache, only if there is a definition.
            if(!currFunction->isDeclaration()) {
                std::set<PointerPointsTo *> *newPointsTo = new std::set<PointerPointsTo *>();
                GlobalObject *glob = new GlobalObject(currFunction);
                PointerPointsTo *pointsToObj = new PointerPointsTo();
                pointsToObj->targetObject = glob;
                pointsToObj->fieldId = pointsToObj->dstfieldId = 0;
                pointsToObj->propogatingInstruction = currFunction;
                pointsToObj->targetPointer = currFunction;
                newPointsTo->insert(newPointsTo->end(), pointsToObj);

                GlobalState::globalVariables[currFunction] = newPointsTo;
                globalObjectCache[currFunction] = glob;
            }
        }

        /***
         * Add loop exit blocks for the provided function.
         * @param targetFunction Pointer to the function for which the loop exit block needs to be added.
         * @param allExitBBs List of the basicblocks to be added
         */
        static void addLoopExitBlocks(Function *targetFunction, SmallVector<BasicBlock *, 1000> &allExitBBs) {
            if(loopExitBlocks.find(targetFunction) == loopExitBlocks.end()) {
                loopExitBlocks[targetFunction] = new std::set<BasicBlock*>();
            }
            std::set<BasicBlock*> *toAddList;
            toAddList = loopExitBlocks[targetFunction];
            toAddList->insert(allExitBBs.begin(), allExitBBs.end());
        }

        /***
         * Get all loop exit basic blocks for the provided function.
         * @param targetFunction Target function for which the exit blocks needs to be fetched.
         * @return pointer to set of all loop exit basic blocks for the provided function.
         */
        static std::set<BasicBlock*> * getLoopExitBlocks(Function *targetFunction) {
            if(loopExitBlocks.find(targetFunction) != loopExitBlocks.end()) {
                return loopExitBlocks[targetFunction];
            }
            return nullptr;
        }


        // Get the context for the provided instruction at given call sites.
        AnalysisContext* getContext(std::vector<Instruction *> *callSites) {
            for(auto curr_a:availableAnalysisContexts) {
                if(*(curr_a->callSites) == *callSites) {
                    return curr_a;
                }
            }
            return nullptr;
        }


        /***
         *  Get or create context at the provided list of callsites,
         *  with corresponding pointsto and taint information.
         *
         * @param callSites list of call sites for the target context.
         * @param targetInfo Points-To info as std::set<PointerPointsTo*>*>*
         * @param targetTaintInfo Taint into as std::map<Value *, std::set<TaintFlag*>*> *
         * @return Target context updated with the provided information.
         *
         */
        AnalysisContext* getOrCreateContext(std::vector<Instruction *> *callSites, std::map<Value *,
                std::set<PointerPointsTo*>*> *targetInfo = nullptr, std::map<Value *, std::set<TaintFlag*>*> *targetTaintInfo = nullptr) {

            AnalysisContext* currContext = getContext(callSites);
            if(currContext == nullptr) {
                // Create a new context for the provided instruction with provided points to info.
                AnalysisContext *newContext = new AnalysisContext();
                newContext->callSites = callSites;
                // insert the new context into available contexts.
                availableAnalysisContexts.insert(availableAnalysisContexts.end(), newContext);

                // create new points to information.
                std::map<Value *, std::set<PointerPointsTo *> *> *newInfo = new std::map<Value *, std::set<PointerPointsTo *> *>();
                if (targetInfo != nullptr) {
                    newInfo->insert(targetInfo->begin(), targetInfo->end());
                } else {
                    // Add all global variables in to the context.
                    newInfo->insert(GlobalState::globalVariables.begin(), GlobalState::globalVariables.end());
                }
                pointToInformation[newContext] = newInfo;

                // create taint info for the newly created context.
                std::map<Value *, std::set<TaintFlag*>*> *newTaintInfo = new std::map<Value *, std::set<TaintFlag*>*>();
                if(targetTaintInfo != nullptr) {
                    newTaintInfo->insert(targetTaintInfo->begin(), targetTaintInfo->end());
                }
                taintInformation[newContext] = newTaintInfo;

                return newContext;
            }
            return currContext;
        }

        void insertPointsTo(std::vector<Instruction *> *callSites, std::map<Value *, std::set<PointerPointsTo*>*> *targetInfo) {
            AnalysisContext* currContext = getContext(callSites);
            pointToInformation[currContext] = targetInfo;
        }

        void copyPointsToInfo(AnalysisContext *targetContext) {
            // Make a shallow copy of points to info for the current context.
            std::map<Value *, std::set<PointerPointsTo*>*> *currInfo = pointToInformation[targetContext];

            // we need to make a shallow copy of currInfo
            std::map<Value *, std::set<PointerPointsTo*>*> *newInfo = new std::map<Value *, std::set<PointerPointsTo*>*>();
            newInfo->insert(currInfo->begin(), currInfo->end());

            pointToInformation[targetContext] = newInfo;
        }

        /***
         * Get all points to information at the provided context i.e., list of call sites.
         * @param callSites target context: List of call-sites
         * @return PointsTo information as std::map<Value *, std::set<PointerPointsTo*>*>*
         */
        std::map<Value *, std::set<PointerPointsTo*>*>* getPointsToInfo(std::vector<Instruction *> *callSites) {
            AnalysisContext* currContext = getContext(callSites);
            assert(currContext != nullptr && pointToInformation.count(currContext));
            //if(currContext != nullptr && pointToInformation.count(currContext)) {
                return pointToInformation[currContext];
            //}
            //return nullptr;
        }

        // Taint Handling functions

        /***
         * get all taint information at the provided context i.e., list of call sites
         * @param callSites target context: List of call-sites
         * @return Taint information as: std::map<Value *, std::set<TaintFlag*>*>*
         */
        std::map<Value *, std::set<TaintFlag*>*>* getTaintInfo(std::vector<Instruction *> *callSites) {
            AnalysisContext* currContext = getContext(callSites);
            if(currContext != nullptr && taintInformation.count(currContext)) {
                return taintInformation[currContext];
            }
            return nullptr;
        };

        // Range analysis helpers

        /***
         * Get range for the provided value
         * @param targetValue Value for which range needs to be fetched.
         * @return Pointer to range object, if exists, else Null.
         */
        Range getRange(Value *targetValue) {
            return this->range_analysis->getRange(targetValue);
        }

        // Adding vulnerability warning

        /***
         * Add the provided vulnerability warning to the current state indexed by instruction.
         * @param currWarning Vulnerability warning that needs to be added.
         */
        void addVulnerabilityWarningByInstr(VulnerabilityWarning *currWarning) {
            Instruction *targetInstr = currWarning->target_instr;
            std::set<VulnerabilityWarning*> *warningList = nullptr;
            if(warningsByInstr.find(targetInstr) == warningsByInstr.end()) {
                warningsByInstr[targetInstr] = new std::set<VulnerabilityWarning*>();
            }
            warningList = warningsByInstr[targetInstr];

            for(auto a:*warningList) {
                if(a->isSameVulWarning(currWarning)) {
                    return;
                }
            }
            warningList->insert(currWarning);
        }

        /***
         * Add the provided vulnerability warning to the current state.
         * @param currWarning Vulnerability warning that needs to be added.
         */
        void addVulnerabilityWarning(VulnerabilityWarning *currWarning) {
            assert(currWarning != nullptr);
            AnalysisContext* currContext = getContext(currWarning->getCallSiteTrace());
            assert(currContext != nullptr);
            if(allVulnWarnings.find(currContext) == allVulnWarnings.end()) {
                // first vulnerability warning.
                allVulnWarnings[currContext] = new std::set<VulnerabilityWarning*>();
            }
            allVulnWarnings[currContext]->insert(currWarning);

            this->addVulnerabilityWarningByInstr(currWarning);

        }
    };
}

#endif //PROJECT_MODULESTATE_H
