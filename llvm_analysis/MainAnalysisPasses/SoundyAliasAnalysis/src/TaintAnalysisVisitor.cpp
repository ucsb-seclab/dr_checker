//
// Created by machiry on 12/5/16.
//

#include "TaintAnalysisVisitor.h"
#include "PointsToUtils.h"
#include "TaintUtils.h"
#include "CFGUtils.h"

using namespace llvm;
namespace DRCHECKER {

/*#define DEBUG_CALL_INSTR
#define DEBUG_RET_INSTR
#define DEBUG_LOAD_INSTR
#define DEBUG_CAST_INSTR
#define DEBUG
#define DEBUG_BIN_INSTR*/

//#define DEBUG_CALL_INSTR

    std::set<TaintFlag*>* TaintAnalysisVisitor::getTaintInfo(Value *targetVal) {
        return TaintUtils::getTaintInfo(this->currState, this->currFuncCallSites, targetVal);
    }

    void TaintAnalysisVisitor::getPtrTaintInfo(Value *targetVal, std::set<TaintFlag*> &retTaintFlag) {
        std::set<PointerPointsTo*> currValPointsTo;
        std::set<PointerPointsTo*> *currPtsTo = PointsToUtils::getPointsToObjects(this->currState, this->currFuncCallSites, targetVal);
        if(currPtsTo != nullptr) {
            currValPointsTo.insert(currPtsTo->begin(), currPtsTo->end());
        }

        for(PointerPointsTo *currPtTo: currValPointsTo) {
            std::set<TaintFlag *> *currTaintSet = currPtTo->targetObject->getFieldTaintInfo(currPtTo->dstfieldId);
            if(currTaintSet != nullptr) {
                for(auto a: *currTaintSet) {
                    if(std::find_if(retTaintFlag.begin(), retTaintFlag.end(), [a](const TaintFlag *n) {
                        return  n->isTaintEquals(a);
                    }) == retTaintFlag.end()) {
                        // if not insert the new taint flag into the newTaintInfo.
                        retTaintFlag.insert(a);
                    }
                }
            }
        }
    }

    void TaintAnalysisVisitor::updateTaintInfo(Value *targetVal, std::set<TaintFlag*> *targetTaintInfo) {
        TaintUtils::updateTaintInfo(this->currState, this->currFuncCallSites, targetVal, targetTaintInfo);
    }

    std::set<TaintFlag*>* TaintAnalysisVisitor::makeTaintInfoCopy(Value *srcOperand, Instruction *targetInstruction,
                                                                  std::set<TaintFlag*>* srcTaintInfo,
                                                                  std::set<TaintFlag*> *dstTaintInfo) {
        if(srcTaintInfo != nullptr) {
            std::set<TaintFlag *> *newTaintInfo = new std::set<TaintFlag *>();
            bool add_taint = false;
            for (auto currTaint:*srcTaintInfo) {
                add_taint = true;
                if(currTaint->targetInstr != nullptr) {
                    Instruction *srcInstruction = dyn_cast<Instruction>(currTaint->targetInstr);
                    if (srcInstruction != nullptr && targetInstruction != nullptr) {
                        add_taint = BBTraversalHelper::isReachable(srcInstruction, targetInstruction,
                                                                   this->currFuncCallSites);
                    }
                }
                if(add_taint) {
                    TaintFlag *newTaintFlag = new TaintFlag(currTaint, targetInstruction, srcOperand);
                    newTaintInfo->insert(newTaintInfo->end(), newTaintFlag);
                }
            }
            // if no taint info is propagated.
            if(newTaintInfo->empty()) {
                delete(newTaintInfo);
                newTaintInfo = nullptr;
            }
            // if dstTaintInfo is not null.
            if(dstTaintInfo != nullptr && newTaintInfo != nullptr) {
                // copy all the taint info into dstTaintInfo.
                dstTaintInfo->insert(newTaintInfo->begin(), newTaintInfo->end());
                // delete new taint info
                delete(newTaintInfo);
                // set return value of dstTaintInfo
                newTaintInfo = dstTaintInfo;
            }
            return newTaintInfo;
        }
        return nullptr;
    }

    std::set<TaintFlag*>* TaintAnalysisVisitor::mergeTaintInfo(std::set<Value *> &srcVals, Value *targetInstr) {

        std::set<TaintFlag*> *newTaintInfo = new std::set<TaintFlag*>();

        for(auto currVal:srcVals) {
            std::set<TaintFlag*> *currValTaintInfo = getTaintInfo(currVal);
            // we do not have taint info? strip and check
            if(currValTaintInfo == nullptr) {
                currVal = currVal->stripPointerCasts();
            }
            currValTaintInfo = getTaintInfo(currVal);
            if(currValTaintInfo != nullptr) {
                this->makeTaintInfoCopy(targetInstr, dyn_cast<Instruction>(targetInstr),
                                        currValTaintInfo, newTaintInfo);
            }
        }
        // if there is no taint info?
        if(newTaintInfo->empty()) {
            // delete the object.
            delete(newTaintInfo);
            newTaintInfo = nullptr;
        }
        return newTaintInfo;

    }

    void TaintAnalysisVisitor::addNewTaintFlag(std::set<TaintFlag*> *newTaintInfo, TaintFlag *newTaintFlag) {
        TaintUtils::addNewTaintFlag(newTaintInfo, newTaintFlag);
    }

    void TaintAnalysisVisitor::visitAllocaInst(AllocaInst &I) {
        // Nothing to do for TaintAnalysis
    }

    void TaintAnalysisVisitor::visitCastInst(CastInst &I) {
        // handles cast instruction.

        // if the src operand is tainted then the instruction is tainted.

        Value *srcOperand = I.getOperand(0);
        std::set<TaintFlag*>* srcTaintInfo = getTaintInfo(srcOperand);
        if(srcTaintInfo == nullptr) {
            srcOperand = srcOperand->stripPointerCasts();
            srcTaintInfo = getTaintInfo(srcOperand);
        }

        // if there exists some taintflags..propagate them
        if(srcTaintInfo != nullptr) {
            std::set<TaintFlag*> *newTaintInfo = this->makeTaintInfoCopy(srcOperand, &I, srcTaintInfo);
            if(newTaintInfo != nullptr) {
                this->updateTaintInfo(&I, newTaintInfo);
            } else {
#ifdef DEBUG_CAST_INSTR
                dbgs() << "Taint Info cannot be propagated because the current instruction is not reachable from";
                dbgs() << "  tainted source at ";
                I.print(dbgs());
                dbgs() << "\n";
#endif
            }
        }

    }

    void TaintAnalysisVisitor::visitBinaryOperator(BinaryOperator &I) {
        // merge taint flag of all the operands.
        std::set<Value*> allVals;
        allVals.insert(allVals.end(), I.getOperand(0));
        allVals.insert(allVals.end(), I.getOperand(1));
        std::set<TaintFlag*> *newTaintInfo = mergeTaintInfo(allVals, &I);
        if(newTaintInfo != nullptr) {
#ifdef DEBUG_BIN_INSTR
            dbgs() << "Propagating taint in binary instruction\n";
#endif
            updateTaintInfo(&I, newTaintInfo);
        }

    }

    void TaintAnalysisVisitor::visitPHINode(PHINode &I) {
        // get all values that need to be merged.
        std::set<Value*> allVals;
        for(unsigned i=0;i<I.getNumIncomingValues(); i++) {
            allVals.insert(allVals.end(), I.getIncomingValue(i));
        }
        std::set<TaintFlag*> *newTaintInfo = mergeTaintInfo(allVals, &I);
        if(newTaintInfo != nullptr) {
            updateTaintInfo(&I, newTaintInfo);
        }
    }

    void TaintAnalysisVisitor::visitSelectInst(SelectInst &I) {
        /***
         *  Merge taintinfo of all the objects
         *  reaching this select instruction.
         */
        // get all values that need to be merged.
        std::set<Value*> allVals;
        allVals.insert(allVals.end(), I.getTrueValue());
        allVals.insert(allVals.end(), I.getFalseValue());

        std::set<TaintFlag*> *newTaintInfo = mergeTaintInfo(allVals, &I);
        if(newTaintInfo != nullptr) {
            updateTaintInfo(&I, newTaintInfo);
        }

    }

    void TaintAnalysisVisitor::visitGetElementPtrInst(GetElementPtrInst &I) {
        // the GEP instruction always computes pointer, which is used to

        // check if one of the operand is tainted?
        // get all values that need to be merged.
        std::set<Value*> allVals;
        for(unsigned i=0;i<I.getNumOperands(); i++) {
            Value* currOp = I.getOperand(i);
            Range currRange = this->currState.getRange(currOp);
            if(currRange.isBounded()) {
                // if the range of the index we use is bounded?
                // it may not be bad.
                continue;
            }
            allVals.insert(allVals.end(), currOp);
        }
        std::set<TaintFlag*> *newTaintInfo = mergeTaintInfo(allVals, &I);
        if(newTaintInfo != nullptr) {
            updateTaintInfo(&I, newTaintInfo);
        }
    }

    void TaintAnalysisVisitor::visitLoadInst(LoadInst &I) {


#ifdef DEBUG_LOAD_INSTR
        dbgs() << "In taint\n";
        dbgs() << "Taint Analysis Visiting Load Instruction:";
        I.print(dbgs());
        dbgs() << "\n";
#endif
        Value *srcPointer = I.getPointerOperand();
        std::set<TaintFlag*> *srcTaintInfo = getTaintInfo(srcPointer);

        std::set<TaintFlag*> *newTaintInfo = new std::set<TaintFlag*>();

        bool already_stripped = true;
        if(srcTaintInfo == nullptr) {
            already_stripped = false;
            if(getTaintInfo(srcPointer->stripPointerCasts())) {
                srcPointer = srcPointer->stripPointerCasts();
                srcTaintInfo = getTaintInfo(srcPointer);
                already_stripped = true;
            }
        }

        //Copy the taint from tainted pointer.
        if(srcTaintInfo != nullptr) {
            for(auto currTaintFlag:*srcTaintInfo) {
                TaintFlag *newTaintFlag = new TaintFlag(currTaintFlag, &I, srcPointer);
                TaintAnalysisVisitor::addNewTaintFlag(newTaintInfo, newTaintFlag);
            }
        }

        if(!already_stripped) {
            if(!PointsToUtils::hasPointsToObjects(currState, this->currFuncCallSites, srcPointer)) {
                srcPointer = srcPointer->stripPointerCasts();
            }
        }

        // Get the src points to information.
        std::set<PointerPointsTo*>* srcPointsTo = PointsToUtils::getPointsToObjects(currState,
                                                                                    this->currFuncCallSites,
                                                                                    srcPointer);
        if(srcPointsTo != nullptr) {
            // this set stores the <fieldid, targetobject> of all the objects to which the srcPointer points to.
            std::set<std::pair<long, AliasObject *>> targetObjects;
            for (PointerPointsTo *currPointsToObj:*srcPointsTo) {
                long target_field = currPointsToObj->dstfieldId;
                AliasObject *dstObj = currPointsToObj->targetObject;
                auto to_check = std::make_pair(target_field, dstObj);
                if (std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                    targetObjects.insert(targetObjects.end(), to_check);
                }
            }

            // make sure we have some objects.
            assert(targetObjects.size() > 0);

            // add the taint from the corresponding fields of the objects.
            for (auto fieldObject: targetObjects) {
                long currFieldID = fieldObject.first;
                AliasObject *currObject = fieldObject.second;
                // get the taint info of the field.
                std::set<TaintFlag *> *fieldTaintInfo = currObject->getFieldTaintInfo(currFieldID);
#ifdef DEBUG_LOAD_INSTR
                dbgs() << "Trying to get taint from object:" << currObject << " fieldID:" << currFieldID << "\n";
#endif
                // if the field is tainted, add the taint from the field
                // to the result of this instruction.
                if (fieldTaintInfo != nullptr) {
                    this->makeTaintInfoCopy(&I, &I, fieldTaintInfo, newTaintInfo);
                }
            }
        } else {
#ifdef DEBUG_LOAD_INSTR
            dbgs() << "TaintAnalysis: Src Pointer does not point to any object:";
            srcPointer->print(dbgs());
            dbgs() << "\n";
#endif
        }

        if(newTaintInfo->size()) {
            // okay. Now add the newTaintInfo
#ifdef DEBUG_LOAD_INSTR
            dbgs() << "TaintAnalysis: Updating Taint in LoadInstruction, from:";
            srcPointer->print(dbgs());
            dbgs() << "\n";
#endif
            updateTaintInfo(&I, newTaintInfo);
        } else {
            delete(newTaintInfo);
        }

    }

    void TaintAnalysisVisitor::visitStoreInst(StoreInst &I) {
        Value *srcPointer = I.getValueOperand();
        std::set<TaintFlag *> *srcTaintInfo = getTaintInfo(srcPointer);
        if(srcTaintInfo == nullptr) {
            srcPointer = srcPointer->stripPointerCasts();
            srcTaintInfo = getTaintInfo(srcPointer);
        }

        // if the value, we are trying to store is tainted? Then process, else
        // ignore.
        if(srcTaintInfo != nullptr) {

            // create newTaintInfo set.
            std::set<TaintFlag *> *newTaintInfo = new std::set<TaintFlag *>();

            bool add_taint;
            for(auto currTaintFlag:*srcTaintInfo) {
                add_taint = true;
                Instruction *currTaintInstr = dyn_cast<Instruction>(&I);
                if(currTaintInstr != nullptr) {
                    add_taint = BBTraversalHelper::isReachable(currTaintInstr, &I, this->currFuncCallSites);
                }
                if(add_taint) {
                    TaintFlag *newTaintFlag = new TaintFlag(currTaintFlag, &I, srcPointer);
                    // add the current instruction in the instruction trace.
                    newTaintFlag->addInstructionToTrace(&I);
                    TaintAnalysisVisitor::addNewTaintFlag(newTaintInfo, newTaintFlag);
                }
            }


            // check dstTaintInfo.

            Value *dstPointer = I.getPointerOperand();
            std::set<TaintFlag *> *dstTaintInfo = getTaintInfo(dstPointer);

            bool already_stripped = true;

            if(dstTaintInfo == nullptr) {
                already_stripped = false;
                if(getTaintInfo(dstPointer->stripPointerCasts())) {
                    dstPointer = dstPointer->stripPointerCasts();
                    dstTaintInfo = getTaintInfo(dstPointer);
                    already_stripped = true;
                }
            }

            if(!already_stripped) {
                if(!PointsToUtils::hasPointsToObjects(currState, this->currFuncCallSites, dstPointer)) {
                    dstPointer = dstPointer->stripPointerCasts();
                }
            }

            std::set<PointerPointsTo*>* dstPointsTo = PointsToUtils::getPointsToObjects(currState,
                                                                                        this->currFuncCallSites,
                                                                                        dstPointer);

            if(dstPointsTo != nullptr) {

                // Now store the taint into correct fields.
                // this set stores the <fieldid, targetobject> of all the objects to which the dstPointer points to.
                std::set<std::pair<long, AliasObject *>> targetObjects;
                for (PointerPointsTo *currPointsToObj:*dstPointsTo) {
                    long target_field = currPointsToObj->dstfieldId;
                    AliasObject *dstObj = currPointsToObj->targetObject;
                    auto to_check = std::make_pair(target_field, dstObj);
                    if (std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                        targetObjects.insert(targetObjects.end(), to_check);
                    }
                }

                bool is_added;
                // Now try to store the newTaintInfo into all of these objects.
                for(auto newTaintFlag:*newTaintInfo) {
                    is_added = false;
                    for(auto fieldObject:targetObjects) {
                        if(fieldObject.second->addFieldTaintFlag(fieldObject.first, newTaintFlag)) {
                            is_added = true;
                        }
                    }
                    // if the current taint is not added to any object.
                    // delete the newTaint object.
                    if(!is_added) {
                        delete(newTaintFlag);
                    }
                }

            }
            // clean up
            delete(newTaintInfo);
        }

    }

    // The following instructions are ignored.
    // we will deal with them, if we find them
    void TaintAnalysisVisitor::visitVAArgInst(VAArgInst &I) {
        // NO idea how to handle this
        assert(false);
    }

    void TaintAnalysisVisitor::visitVACopyInst(VACopyInst &I) {
        // No idea how to handle this
        assert(false);
    }

    void TaintAnalysisVisitor::propogateTaintToArguments(std::set<long> &taintedArgs, CallInst &I) {
        assert(taintedArgs.size() > 0);
#ifdef DEBUG_CALL_INSTR
        dbgs() << "Propagating Taint To Arguments.\n";
#endif
        for(auto currArgNo: taintedArgs) {
            Value *currArg = I.getArgOperand(currArgNo);
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Current argument:";
            currArg->print(dbgs());
            dbgs() << "\n";
#endif
            std::set<PointerPointsTo*>* dstPointsTo = PointsToUtils::getPointsToObjects(currState,
                                                                                        this->currFuncCallSites,
                                                                                        currArg);
            if(dstPointsTo == nullptr) {
                currArg = currArg->stripPointerCasts();
                dstPointsTo = PointsToUtils::getPointsToObjects(currState,
                                                                this->currFuncCallSites,
                                                                currArg);
            }
            if(dstPointsTo != nullptr) {
                std::set<std::pair<long, AliasObject *>> targetObjects;
                for (PointerPointsTo *currPointsToObj:*dstPointsTo) {
                    long target_field = currPointsToObj->dstfieldId;
                    AliasObject *dstObj = currPointsToObj->targetObject;
                    auto to_check = std::make_pair(target_field, dstObj);
                    if (std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                        targetObjects.insert(targetObjects.end(), to_check);
                    }
                }

                bool is_added = false;

                assert(targetObjects.size() > 0);
                TaintFlag *newTaintFlag = new TaintFlag(currArg, true);
                newTaintFlag->addInstructionToTrace(&I);

                for(auto fieldObject:targetObjects) {
                    // if it is pointing to first field, then taint everything
                    // else taint only corresponding field.
                    if(fieldObject.first != 0) {
#ifdef DEBUG_CALL_INSTR
                        dbgs() << "Adding Taint To field ID:"<< fieldObject.first << " of:" << fieldObject.second;
#endif

                        if (fieldObject.second->addFieldTaintFlag(fieldObject.first, newTaintFlag)) {
#ifdef DEBUG_CALL_INSTR
                            dbgs() << ":Success\n";
#endif
                            is_added = true;
                        } else {
#ifdef DEBUG_CALL_INSTR
                            dbgs() << ":Failed\n";
#endif
                        }
                    } else {
#ifdef DEBUG_CALL_INSTR
                        dbgs() << "Adding Taint To All fields:"<< fieldObject.first << " of:" << fieldObject.second;
#endif
                        if(fieldObject.second->taintAllFields(newTaintFlag)) {
#ifdef DEBUG_CALL_INSTR
                            dbgs() << ":Success\n";
#endif
                            is_added = true;
                        } else {
#ifdef DEBUG_CALL_INSTR
                            dbgs() << ":Failed\n";
#endif
                        }
                    }
                }
                // if the current taint is not added to any object.
                // delete the newTaint object.
                if(!is_added) {
                    delete(newTaintFlag);
                }

            } else {
                // TODO: raise warning that we do not have any points to information.
#ifdef DEBUG_CALL_INSTR
                dbgs() << "TaintAnalysis: Argument does not have points to information:";
                currArg->print(dbgs());
                dbgs() << "\n";
#endif
            }
        }
    }

    void TaintAnalysisVisitor::setupCallContext(CallInst &I, Function *currFunction,
                                                std::vector<Instruction *> *newCallContext) {

        std::map<Value *, std::set<TaintFlag*>*> *contextTaintInfo = currState.getTaintInfo(newCallContext);

        unsigned int arg_no = 0;

        for(User::op_iterator arg_begin = I.arg_begin(), arg_end = I.arg_end(); arg_begin != arg_end; arg_begin++) {
            Value *currArgVal =(*arg_begin).get();

            if(getTaintInfo(currArgVal) || getTaintInfo(currArgVal->stripPointerCasts())) {
                unsigned int farg_no;
                farg_no = 0;
                std::set<Value*> valuesToMerge;
                // handle pointer casts
                if(!getTaintInfo(currArgVal)) {
                    currArgVal = currArgVal->stripPointerCasts();
                }
#ifdef DEBUG_CALL_INSTR
                dbgs() << "Has Taint Info:" << getTaintInfo(currArgVal)->size() << " taint flags\n";
#endif
                valuesToMerge.clear();
                valuesToMerge.insert(valuesToMerge.end(), currArgVal);

                for(Function::arg_iterator farg_begin = currFunction->arg_begin(), farg_end = currFunction->arg_end();
                    farg_begin != farg_end; farg_begin++) {
                    Value *currfArgVal = &(*farg_begin);
                    if(farg_no == arg_no) {
                        std::set<TaintFlag*> *currArgTaintInfo = mergeTaintInfo(valuesToMerge, &I);
                        // ensure that we didn't mess up.
                        assert(currArgTaintInfo != nullptr);
#ifdef DEBUG_CALL_INSTR
                        // OK, we need to add taint info.
                        dbgs() << "Argument:" << (arg_no + 1) << " has taint info\n";
#endif
                        (*contextTaintInfo)[currfArgVal] = currArgTaintInfo;
                        break;
                    }
                    farg_no++;
                }
            } else {
#ifdef DEBUG_CALL_INSTR
                dbgs() << "Argument:" << (arg_no + 1) << " does not have taint info\n";
#endif
            }
            arg_no++;
        }

    }

    void TaintAnalysisVisitor::propagateTaintToMemcpyArguments(std::vector<long> &memcpyArgs, CallInst &I) {
#ifdef DEBUG_CALL_INSTR
        dbgs() << "Processing memcpy function\n";
#endif
        // we do not need any special taint handling..because alias takes care of propagating
        // pointer, here we just need to update taint of the arguments.
        // get src operand
        Value *srcOperand = I.getArgOperand((unsigned int) memcpyArgs[0]);
        // get dst operand
        Value *dstOperand = I.getArgOperand((unsigned int) memcpyArgs[1]);

        std::set<Value*> mergeVals;
        mergeVals.insert(srcOperand);

        std::set<TaintFlag*>* newTaintInfo = this->mergeTaintInfo(mergeVals, &I);
        if(newTaintInfo != nullptr) {
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Trying to memcpy from tainted argument\n";
#endif
            this->updateTaintInfo(dstOperand, newTaintInfo);
        }

    }

    void TaintAnalysisVisitor::handleKernelInternalFunction(CallInst &I, Function *currFunc) {
        // see if this is a taint initiator function.
        if(TaintAnalysisVisitor::functionChecker->is_taint_initiator(currFunc)) {
#ifdef DEBUG_CALL_INSTR
            dbgs() << "This function is a taint initiator function:" << currFunc->getName() << "\n";
#endif
            // handling __copy_from_user and its friends.
            std::set<long> taintedArgs = TaintAnalysisVisitor::functionChecker->get_tainted_arguments(currFunc);
            this->propogateTaintToArguments(taintedArgs, I);

        } else if(TaintAnalysisVisitor::functionChecker->is_memcpy_function(currFunc)) {
            // Handle memcpy function..
            // get memcpy argument numbers
            std::vector<long> memcpyArgs = TaintAnalysisVisitor::functionChecker->get_memcpy_arguments(currFunc);
            //propagate taint from src to dst.
            this->propagateTaintToMemcpyArguments(memcpyArgs, I);
        } else if(TaintAnalysisVisitor::functionChecker->is_atoi_function(currFunc)) {
          // This is an atoi like function?
           // if yes? get the taint of the object pointed by the first argument.
            // propagate that to the return value.
            std::set<TaintFlag*> allPointerTaint;
            allPointerTaint.clear();
            this->getPtrTaintInfo(I.getArgOperand(0), allPointerTaint);
            if(!allPointerTaint.empty()) {
                std::set<TaintFlag *> *newTaintSet = this->makeTaintInfoCopy(&I, nullptr, &allPointerTaint);
                this->updateTaintInfo(&I, newTaintSet);
            }

        } else if(TaintAnalysisVisitor::functionChecker->is_sscanf_function(currFunc)) {
            // This is a sscanf function?
            // if yes? get the taint of the object pointed by the first argument.
            std::set<TaintFlag*> allPointerTaint;
            allPointerTaint.clear();
            this->getPtrTaintInfo(I.getArgOperand(0), allPointerTaint);
            if(!allPointerTaint.empty()) {
                std::set<TaintFlag *> *newTaintSet = this->makeTaintInfoCopy(&I, nullptr, &allPointerTaint);

                std::set<TaintFlag *> addedTaints;

                // now add taint to all objects pointed by the arguments.
                unsigned int arg_idx;
                for (arg_idx = 2; arg_idx < I.getNumArgOperands(); arg_idx++) {
                    Value *argVal = I.getArgOperand(arg_idx);
                    std::set<PointerPointsTo*> *currPtsTo = PointsToUtils::getPointsToObjects(this->currState,
                                                                                              this->currFuncCallSites,
                                                                                              argVal);
                    if(currPtsTo != nullptr) {
                        for(auto currP:*currPtsTo) {
                            for(auto currT:*newTaintSet) {
                                if(currP->targetObject->addFieldTaintFlag(currP->dstfieldId, currT)) {
                                    addedTaints.insert(currT);
                                }
                            }
                        }
                    }
                }

                for(auto currT:*newTaintSet) {
                    if(addedTaints.find(currT) == addedTaints.end()) {
                        delete(currT);
                    }
                }

                delete(newTaintSet);
            }

        } else {
            // TODO (below):
            // untaint all the arguments, depending on whether we are
            // indeed calling kernel internal functions.
        }
    }

    VisitorCallback* TaintAnalysisVisitor::visitCallInst(CallInst &I, Function *currFunc,
                                                         std::vector<Instruction *> *oldFuncCallSites,
                                                         std::vector<Instruction *> *callSiteContext) {
        // if this is a kernel internal function.
        if(currFunc->isDeclaration()) {
            this->handleKernelInternalFunction(I, currFunc);
            return nullptr;
        }

        // else, we need to setup taint context and create a new visitor.
        setupCallContext(I, currFunc, callSiteContext);

        // debugging
        /*if(currFunc->getName() == "m4u_monitor_start") {
            assert(false);
        }*/
        // create a new TaintAnalysisVisitor
        TaintAnalysisVisitor *vis = new TaintAnalysisVisitor(currState, currFunc, callSiteContext);

        return vis;

    }

    void TaintAnalysisVisitor::stitchChildContext(CallInst &I, VisitorCallback *childCallback) {
        // just connect the taint of the return values.
        TaintAnalysisVisitor *vis = (TaintAnalysisVisitor *)childCallback;
        if(vis->retValTaints.size() > 0 ){
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Stitching return value for call instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            // create new taint info.
            std::set<TaintFlag*>* newTaintInfo = new std::set<TaintFlag*>();
            for(auto currRetTaint:vis->retValTaints) {
                TaintFlag *newTaintFlag = new TaintFlag(currRetTaint, &I, &I);
                newTaintInfo->insert(newTaintInfo->end(), newTaintFlag);
            }

            //update taint info
            updateTaintInfo(&I, newTaintInfo);
        }
    }

    void TaintAnalysisVisitor::visitReturnInst(ReturnInst &I) {
        // add taint of the return value to the retTaintInfo set.
        Value *targetRetVal = I.getReturnValue();
        if(targetRetVal != nullptr && (getTaintInfo(targetRetVal) || getTaintInfo(targetRetVal->stripPointerCasts()))) {
            // check if pointer casts has a role to play?
            if(!getTaintInfo(targetRetVal)){
                targetRetVal = targetRetVal->stripPointerCasts();
            }
            std::set<TaintFlag*> *currRetTaintInfo = getTaintInfo(targetRetVal);

            for(auto a:*currRetTaintInfo) {
                if(std::find_if(this->retValTaints.begin(), this->retValTaints.end(), [a](const TaintFlag *n) {
                    return  n->isTaintEquals(a);
                }) == this->retValTaints.end()) {
                    // if not insert the new taint flag into the newTaintInfo.
                    this->retValTaints.insert(this->retValTaints.end(), a);
                }
            }

        } else {
#ifdef DEBUG_RET_INSTR
            dbgs() << "Return value:";
            I.print(dbgs());
            dbgs() << ", does not have TaintFlag.\n";
#endif
        }
    }

    void TaintAnalysisVisitor::visitICmpInst(ICmpInst &I) {
        // merge taint flag of all the operands.
        std::set<Value*> allVals;
        for(unsigned i=0;i<I.getNumOperands(); i++) {
            Value* currOp = I.getOperand(i);
            allVals.insert(currOp);
        }
        std::set<TaintFlag*> *newTaintInfo = mergeTaintInfo(allVals, &I);
        if(newTaintInfo != nullptr) {
            updateTaintInfo(&I, newTaintInfo);
        }
    }

}