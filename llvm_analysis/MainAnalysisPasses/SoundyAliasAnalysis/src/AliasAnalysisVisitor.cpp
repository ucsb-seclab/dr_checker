//
// Created by machiry on 12/4/16.
//
#include <llvm/IR/Operator.h>
#include "AliasObject.h"
#include "AliasAnalysisVisitor.h"

namespace DRCHECKER {

/*#define DEBUG_GET_ELEMENT_PTR
#define DEBUG_ALLOCA_INSTR
#define DEBUG_CAST_INSTR
#define DEBUG_BINARY_INSTR
#define DEBUG_PHI_INSTR
#define DEBUG_LOAD_INSTR
#define DEBUG_STORE_INSTR
#define DEBUG_BB_VISIT*/
//#define DEBUG_CALL_INSTR
//#define STRICT_CAST
//#define DEBUG_RET_INSTR
//#define FAST_HEURISTIC
//#define MAX_ALIAS_OBJ 50

    std::set<PointerPointsTo*>* AliasAnalysisVisitor::getPointsToObjects(Value *srcPointer) {
        // Get points to objects set of the srcPointer at the entry of the instruction
        // currInstruction.
        // Note that this is at the entry of the instruction. i.e INFLOW.
        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = this->currState.getPointsToInfo(this->currFuncCallSites);
        // Here srcPointer should be present in points to map.
        if(targetPointsToMap->find(srcPointer) != targetPointsToMap->end()) {
            return (*targetPointsToMap)[srcPointer];
        }
        return nullptr;
    }



    void AliasAnalysisVisitor::updatePointsToObjects(Value *srcPointer, std::set<PointerPointsTo*>* newPointsToInfo) {
        /***
         *  Update the pointsto objects of the srcPointer to newPointstoInfo
         *  At the current instruction.
         *
         *  This also takes care of freeing the elements if they are already present.
         */

        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = this->currState.getPointsToInfo(this->currFuncCallSites);
        auto prevPointsToSet = targetPointsToMap->find(srcPointer);
        if(prevPointsToSet != targetPointsToMap->end()) {
            // OK, there are some previous values for this
            std::set<PointerPointsTo*>* existingPointsTo = prevPointsToSet->second;
            for(PointerPointsTo *currPointsTo: *newPointsToInfo) {
                assert(existingPointsTo != nullptr);
                // for each points to, see if we already have that information, if yes, ignore it.
                if(std::find_if(existingPointsTo->begin(), existingPointsTo->end(), [currPointsTo](const PointerPointsTo *n) {
                    return  n->isIdenticalPointsTo(currPointsTo);
                }) == existingPointsTo->end()) {
                    existingPointsTo->insert(existingPointsTo->end(), currPointsTo);
                } else {
                    //delete the points to object, as we already have a similar pointsTo object.
                    delete (currPointsTo);
                }
            }
            // delete the set pointer.
            delete(newPointsToInfo);

        } else {
            assert(newPointsToInfo != nullptr);
            (*targetPointsToMap)[srcPointer] = newPointsToInfo;
        }
    }

    bool AliasAnalysisVisitor::hasPointsToObjects(Value *srcPointer) {
        /***
         * Check if the srcPointer has any pointto objects at currInstruction
         */
        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = this->currState.getPointsToInfo(this->currFuncCallSites);
        //Value *strippedPtr = srcPointer->stripPointerCasts();
        return targetPointsToMap != nullptr &&
               targetPointsToMap->find(srcPointer) != targetPointsToMap->end();
    }

    std::set<PointerPointsTo*>* AliasAnalysisVisitor::makePointsToCopy(Instruction *propInstruction, Value *srcPointer,
                                                             std::set<PointerPointsTo*>* srcPointsTo, long fieldId) {
        /***
         * Makes copy of points to information from srcPointer to propInstruction
         */
        std::set<PointerPointsTo*>* newPointsToInfo = new std::set<PointerPointsTo*>();
        // set of all visited objects.
        // to avoid added duplicate pointsto objects
        std::set<AliasObject*> visitedObjects;
        for(PointerPointsTo *currPointsToObj:*srcPointsTo) {
            // if the target object is not visited, then add into points to info.
            if(visitedObjects.find(currPointsToObj->targetObject) == visitedObjects.end()) {
                PointerPointsTo *newPointsToObj = new PointerPointsTo();
                newPointsToObj->propogatingInstruction = propInstruction;
                if(fieldId >= 0) {
                    newPointsToObj->dstfieldId = fieldId;
                } else {
                    newPointsToObj->dstfieldId = currPointsToObj->dstfieldId;
                }
                newPointsToObj->fieldId = 0;
                newPointsToObj->targetObject = currPointsToObj->targetObject;
                newPointsToObj->targetPointer = srcPointer;
                visitedObjects.insert(visitedObjects.begin(), newPointsToObj->targetObject);
                newPointsToInfo->insert(newPointsToInfo->begin(), newPointsToObj);
            }
        }
        return newPointsToInfo;
    }

    std::set<PointerPointsTo*>* AliasAnalysisVisitor::mergePointsTo(std::set<Value*> &valuesToMerge, Instruction *targetInstruction, Value *targetPtr) {
        /***
         * Merge pointsToInformation of all the objects pointed by pointers in
         * valuesToMerge
         *
         * targetPtr(if not null) is the pointer that points to all objects in the merge set.
         */
        // Set of pairs of field and corresponding object.
        std::set<std::pair<long, AliasObject*>> targetObjects;
        targetObjects.clear();
        for(Value *currVal:valuesToMerge) {
            // if the value doesn't have points to information.
            // try to strip pointer casts.
            if(!hasPointsToObjects(currVal)) {
                currVal = currVal->stripPointerCasts();
            }
            if(hasPointsToObjects(currVal)) {
                std::set<PointerPointsTo*>* tmpPointsTo = getPointsToObjects(currVal);
                for(PointerPointsTo *currPointsTo:*tmpPointsTo) {
                    auto to_check = std::make_pair(currPointsTo->dstfieldId, currPointsTo->targetObject);
                    if(std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                        targetObjects.insert(targetObjects.end(), to_check);
                    }
                }
            }
        }
        // if there are any objects?
        if(targetObjects.size() > 0) {
            std::set<PointerPointsTo*>* toRetPointsTo = new std::set<PointerPointsTo*>();
            for(auto currItem: targetObjects) {
                PointerPointsTo* currPointsToObj = new PointerPointsTo();
                currPointsToObj->targetPointer = targetInstruction;
                if(targetPtr != nullptr) {
                    currPointsToObj->targetPointer = targetPtr;
                }
                currPointsToObj->targetObject = currItem.second;
                currPointsToObj->dstfieldId = currItem.first;
                currPointsToObj->fieldId = 0;
                currPointsToObj->propogatingInstruction = targetInstruction;
                toRetPointsTo->insert(toRetPointsTo->begin(), currPointsToObj);
            }
            return toRetPointsTo;
        }

        return nullptr;
    }

    std::set<PointerPointsTo*>* AliasAnalysisVisitor::copyPointsToInfo(Instruction *srcInstruction, std::set<PointerPointsTo*>* srcPointsTo) {
        /***
         *  Copy pointsto information from the provided set (srcPointsTo)
         *  to be pointed by srcInstruction.
         */
        std::set<std::pair<long, AliasObject*>> targetObjects;
        targetObjects.clear();
        for(auto currPointsToObj:(*srcPointsTo)) {
            auto to_check = std::make_pair(currPointsToObj->dstfieldId, currPointsToObj->targetObject);
            if(std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                targetObjects.insert(targetObjects.end(), to_check);
            }
        }

        // if there are any objects?
        if(targetObjects.size() > 0) {
            std::set<PointerPointsTo*>* toRetPointsTo = new std::set<PointerPointsTo*>();
            for(auto currItem: targetObjects) {
                PointerPointsTo* currPointsToObj = new PointerPointsTo();
                currPointsToObj->targetPointer = srcInstruction;
                currPointsToObj->targetObject = currItem.second;
                currPointsToObj->dstfieldId = currItem.first;
                currPointsToObj->fieldId = 0;
                currPointsToObj->propogatingInstruction = srcInstruction;
                toRetPointsTo->insert(toRetPointsTo->begin(), currPointsToObj);
            }
            return toRetPointsTo;
        }

        return nullptr;

    }

    void AliasAnalysisVisitor::setLoopIndicator(bool inside_loop) {
        this->inside_loop = inside_loop;
    }

    void AliasAnalysisVisitor::visitAllocaInst(AllocaInst &I) {
        /***
         *  Visit alloca instruction.
         *  This is the origin of an object
         */
        if(hasPointsToObjects(&I)){
            /*
             * We have already visited this instruction before.
             */
#ifdef DEBUG_ALLOCA_INSTR
            dbgs() << "The Alloca instruction, already processed:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            return;
        }
        AliasObject *targetObj = new FunctionLocalVariable(I, this->currFuncCallSites);
        PointerPointsTo *newPointsTo = new PointerPointsTo();
        newPointsTo->fieldId = 0;
        newPointsTo->dstfieldId = 0;
        newPointsTo->propogatingInstruction = &I;
        newPointsTo->targetObject = targetObj;
        newPointsTo->targetPointer = &I;
        std::set<PointerPointsTo*>* newPointsToInfo = new std::set<PointerPointsTo*>();
        newPointsToInfo->insert(newPointsToInfo->end(), newPointsTo);
#ifdef DEBUG_ALLOCA_INSTR
        dbgs() << "Processed Alloca Instruction, Created new points to information:" << (*newPointsTo) << "\n";
#endif
        this->updatePointsToObjects(&I, newPointsToInfo);

    }

    void AliasAnalysisVisitor::visitCastInst(CastInst &I) {
        /***
         * This method handles Cast Instruction.
         * First check if we are converting to pointer, if yes, then we need to compute points to
         * If not, check if src value has points to information, if yes, then we need to compute points to
         */
        Type* dstType = I.getDestTy();
        Type* srcType = I.getSrcTy();

        Value* srcOperand = I.getOperand(0);
        // handle inline casting.
        if(!hasPointsToObjects(srcOperand)) {
            srcOperand = srcOperand->stripPointerCasts();
        }

        if(dstType->isPointerTy() || hasPointsToObjects(srcOperand)) {
            // OK, we need to compute points to information for the current instruction.
            if(hasPointsToObjects(srcOperand)) {
                std::set<PointerPointsTo*>* srcPointsToInfo = getPointsToObjects(srcOperand);

                assert(srcPointsToInfo != nullptr);
                // Create new pointsTo info for the current instruction.
                std::set<PointerPointsTo*>* newPointsToInfo = new std::set<PointerPointsTo*>();
                for(PointerPointsTo *currPointsToObj: *srcPointsToInfo) {
                    PointerPointsTo *newPointsToObj = (PointerPointsTo*)currPointsToObj->makeCopy();
                    newPointsToObj->propogatingInstruction = &I;
                    newPointsToObj->targetPointer = &I;
                    // If the destination object is void type and
                    // we are trying to cast into non-void type?
                    // change the type of the object.
                    Type *currSrcType = newPointsToObj->targetObject->targetType;
                    if(!dstType->isVoidTy() && currSrcType->isPointerTy() && currSrcType->getContainedType(0)->isIntegerTy(8)) {
                        // No need to make copy
                        if(dstType->isPointerTy()) {
                            dstType = dstType->getContainedType(0);
                        }
                        newPointsToObj->targetObject->targetType = dstType;
                    }
                    newPointsToInfo->insert(newPointsToInfo->end(), newPointsToObj);
                }
                // Update the points to Info of the current instruction.
#ifdef DEBUG_CAST_INSTR
                dbgs() << "Adding new points to information in cast instruction\n";
#endif
                this->updatePointsToObjects(&I, newPointsToInfo);
            } else {
                // we are type casting to a pointer and there is no points to information for
                // the srcOperand, what should be done here?
                // Should we leave empty?
#ifdef DEBUG_CAST_INSTR
                dbgs() << "WARNING: Trying to cast a value (that points to nothing) to pointer, Ignoring\n";
#endif
                //assert(false);
            }
        } else {
            //Should we ignore?
            // make sure that source type is not pointer
            if(!this->inside_loop) {
#ifdef STRICT_CAST
                assert(!srcType->isPointerTy());
#endif
#ifdef DEBUG_CAST_INSTR
                dbgs() << "Ignoring casting as pointer does not point to anything\n";
#endif
            } else {
#ifdef DEBUG_CAST_INSTR
                dbgs() << "Is inside the loop..Ignoring\n";
#endif
            }
        }
    }

    void AliasAnalysisVisitor::visitBinaryOperator(BinaryOperator &I) {
        /***
         *  Handle binary instruction.
         *
         *  get the points to information of both the operands and merge them.
         */

        // merge points to of all objects.
        std::set<Value*> allVals;
        allVals.insert(allVals.end(), I.getOperand(0));
        allVals.insert(allVals.end(), I.getOperand(1));
        std::set<PointerPointsTo*>* finalPointsToInfo = mergePointsTo(allVals, &I);
        if(finalPointsToInfo != nullptr) {
            // Update the points to object of the current instruction.
#ifdef DEBUG_BINARY_INSTR
            dbgs() << "Updating points to information in the binary instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            this->updatePointsToObjects(&I, finalPointsToInfo);
        } else {
#ifdef DEBUG_BINARY_INSTR
            dbgs() << "No value is a pointer in the binary instruction.";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        }

        // Sanity,
        // it is really weired if we are trying to do a binary operation on 2-pointers
        if(hasPointsToObjects(I.getOperand(0)) && hasPointsToObjects(I.getOperand(1))) {
#ifdef DEBUG_BINARY_INSTR
            dbgs() << "WARNING: Trying to perform binary operation on 2-pointers.";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        }

    }

    void AliasAnalysisVisitor::visitPHINode(PHINode &I) {
        /***
         *  Merge points to of all objects reaching this phi node.
         */
        // get all values that need to be merged.
        std::set<Value*> allVals;
        for(unsigned i=0;i<I.getNumIncomingValues(); i++) {
            allVals.insert(allVals.end(), I.getIncomingValue(i));
        }

        std::set<PointerPointsTo*>* finalPointsToInfo = mergePointsTo(allVals, &I);
        if(finalPointsToInfo != nullptr) {
            // Update the points to object of the current instruction.
            this->updatePointsToObjects(&I, finalPointsToInfo);
#ifdef DEBUG_PHI_INSTR
            dbgs() << "Merging points to information in the PHI instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        } else {
#ifdef DEBUG_PHI_INSTR
            dbgs() << "None of the operands are pointers in the PHI instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        }

    }

    void AliasAnalysisVisitor::visitSelectInst(SelectInst &I) {
        /***
         *  Merge points to of all objects reaching this select instruction.
         */
        // get all values that need to be merged.
        std::set<Value*> allVals;
        allVals.insert(allVals.end(), I.getTrueValue());
        allVals.insert(allVals.end(), I.getFalseValue());

        std::set<PointerPointsTo*>* finalPointsToInfo = mergePointsTo(allVals, &I);
        if(finalPointsToInfo != nullptr) {
            // Update the points to object of the current instruction.
            this->updatePointsToObjects(&I, finalPointsToInfo);
        }

    }

    void AliasAnalysisVisitor::visitGetElementPtrInst(GetElementPtrInst &I) {
        /***
         *  Handle GetElementPtr instruction.
         *  This is tricky instruction.
         *  this is where accessing structure fields happen.
         */
        Value* srcPointer = I.getPointerOperand();
        GEPOperator *gep = dyn_cast<GEPOperator>(I.getPointerOperand());
        if(gep && gep->getNumOperands() > 0 && gep->getPointerOperand()) {
            srcPointer = gep->getPointerOperand();
        } else {
            if(!hasPointsToObjects(srcPointer)) {
                srcPointer = srcPointer->stripPointerCasts();
            }
        }
        if(I.getPointerOperand()->getType()->getContainedType(0)->isStructTy() && (I.getNumOperands() > 2) ) {
            // Are we indexing a struct?
            // OK, we are de-referencing a structure.
            if (ConstantInt *CI = dyn_cast<ConstantInt>(I.getOperand(2))) {
                unsigned long structFieldId = CI->getZExtValue();
                if(hasPointsToObjects(srcPointer)) {
#ifdef DEBUG_GET_ELEMENT_PTR
                    dbgs() << "Has Points to information for:";
                    srcPointer->print(dbgs());
                    dbgs() << "\n";
#endif

                    std::set<PointerPointsTo*>* srcPointsTo = getPointsToObjects(srcPointer);
                    std::set<PointerPointsTo*>* newPointsToInfo = makePointsToCopy(&I, &I, srcPointsTo, structFieldId);
                    this->updatePointsToObjects(&I, newPointsToInfo);
                } else {
                    // we are trying to dereference a structure or an array
                    // however the src pointer does not point to any object.
                    // How sad??
#ifdef DEBUG_GET_ELEMENT_PTR
                    errs() << "Error occurred, Trying to dereference a structure, which does not point to any object.";
                    errs() << " Ignoring:" << srcPointer << "\n";
                    srcPointer->print(errs());
                    errs() << "  END\n";
#endif
                    //assert(false);
                }

            }
        } else {
            for(int i=0;i<I.getNumOperands();i++) {
                if(dyn_cast<Constant>(I.getOperand(i))) {
                    continue;
                }
                srcPointer = I.getOperand(i);

                // OK, we are not indexing a struct. This means, we are indexing an array
                //ConstantInt *CI = dyn_cast<ConstantInt>(I.getOperand(1));
                // OK, we are trying to access an array, first number should be constant, actually it should be zero

                // are we incrementing pointer? if yes, then index 1 may not be constant.
                /*if (I.getNumOperands() > 2) {
                    assert(CI != nullptr);
                }*/
                //dbgs() << "CURRINST:" << I << "\n";
                //for (int i = 0; i < I.getNumOperands(); i++) {
                    //dbgs() << "Oper:" << *(I.getOperand(i)) << "\n";
                //}

                // we could have array operand as first operand, rather than pointer operand.
                // array operand could be at end
                if (!hasPointsToObjects(srcPointer)) {
                    // check if this is the array operand.
                    // srcPointer = I.getOperand(1);
                    if (!hasPointsToObjects(srcPointer)) {
                        srcPointer = srcPointer->stripPointerCasts();
                    }
                }
                //Ignore the index.
                if (hasPointsToObjects(srcPointer)) {
                    std::set<PointerPointsTo *> *srcPointsTo = getPointsToObjects(srcPointer);
                    std::set<PointerPointsTo *> *newPointsToInfo = makePointsToCopy(&I, &I, srcPointsTo, -1);
                    this->updatePointsToObjects(&I, newPointsToInfo);
                    break;
                } else {
                    // we are trying to dereference an array
                    // however the src pointer does not point to any object.
                    // How sad??
#ifdef DEBUG_GET_ELEMENT_PTR
                    errs() << "Array pointer does not point to any object:";
                    srcPointer->print(dbgs());
                    errs() << "Ignoring.\n";
#endif
                    //assert(false);
                }
            }

        }
    }

    void AliasAnalysisVisitor::visitLoadInst(LoadInst &I) {

        Value* srcPointer = I.getPointerOperand();
        GEPOperator *gep = dyn_cast<GEPOperator>(I.getPointerOperand());
        if(gep && gep->getNumOperands() > 0 && gep->getPointerOperand()) {
            srcPointer = gep->getPointerOperand();
        } else {
            if(!hasPointsToObjects(srcPointer)) {
                srcPointer = srcPointer->stripPointerCasts();
            }
        }

        if(!hasPointsToObjects(srcPointer) && !hasPointsToObjects(srcPointer->stripPointerCasts())) {
#ifdef DEBUG_LOAD_INSTR
            errs() << "Load instruction does not point to any object.";
            I.print(errs());
            errs() << "\n";
#endif
            return;
        }

        // strip pointer casts. if we cannot find any points to for the srcPointer.
        if(!hasPointsToObjects(srcPointer)) {
            srcPointer = srcPointer->stripPointerCasts();
        }


        // srcPointer should have pointsTo information.
        //assert(hasPointsToObjects(srcPointer));

        // Get the src points to information.
        std::set<PointerPointsTo*>* srcPointsTo = getPointsToObjects(srcPointer);
        // OK, now what? :)
        // Get all objects pointed by all the objects in the srcPointsTo

        // this set stores the <fieldid, targetobject> of all the objects to which the srcPointer points to.
        std::set<std::pair<long, AliasObject*>> targetObjects;
        for(PointerPointsTo *currPointsToObj:*srcPointsTo) {
            long target_field = currPointsToObj->dstfieldId;
            AliasObject* dstObj = currPointsToObj->targetObject;
            auto to_check = std::make_pair(target_field, dstObj);
            if(std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                targetObjects.insert(targetObjects.end(), to_check);
            }
        }
#ifdef DEBUG_LOAD_INSTR
        dbgs() << "Number of target objects:" << targetObjects.size() << "\n";
#endif


        // Now get the list of objects to which the fieldid of the corresponding object points to.
        std::set<std::pair<long,AliasObject*>> finalObjects;
        finalObjects.clear();
        for(const std::pair<long, AliasObject*> &currObjPair:targetObjects) {
            // fetch objects that could be pointed by the field.
            // if this object is a function argument then
            // dynamically try to create an object, if we do not have any object
            currObjPair.second->fetchPointsToObjects(currObjPair.first, finalObjects, &I, finalObjects.empty());
        }
        if(finalObjects.size() > 0) {
#ifdef FAST_HEURISTIC
            if(finalObjects.size() > MAX_ALIAS_OBJ) {
                auto end = std::next(finalObjects.begin(), std::min((long)MAX_ALIAS_OBJ, (long)finalObjects.size()));
                std::set<std::pair<long,AliasObject*>> tmpList;
                tmpList.clear();
                tmpList.insert(finalObjects.begin(), end);
                finalObjects.clear();
                finalObjects.insert(tmpList.begin(), tmpList.end());
            }
#endif
            // Create new pointsTo set and add all objects of srcPointsTo
            std::set<PointerPointsTo*>* newPointsToInfo = new std::set<PointerPointsTo*>();
            for(auto currPto:finalObjects) {
                PointerPointsTo *newPointsToObj = new PointerPointsTo();
                newPointsToObj->targetPointer = &I;
                newPointsToObj->propogatingInstruction = &I;
                newPointsToObj->targetObject = currPto.second;
                newPointsToObj->fieldId = 0;
                newPointsToObj->dstfieldId = currPto.first;
                newPointsToInfo->insert(newPointsToInfo->end(), newPointsToObj);
            }
            // Just save the newly created set as points to set for this instruction.
#ifdef DEBUG_LOAD_INSTR
            dbgs() << "Updating points to information for Load instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            this->updatePointsToObjects(&I, newPointsToInfo);
        } else {
            // points to set is empty.
            // Make sure that we are not trying to load a pointer.
            if(!this->inside_loop) {
                assert(!I.getType()->isPointerTy());
            }
        }

    }

    void AliasAnalysisVisitor::visitStoreInst(StoreInst &I) {
        Value *targetPointer = I.getPointerOperand();
        GEPOperator *gep = dyn_cast<GEPOperator>(I.getPointerOperand());
        if(gep && gep->getNumOperands() > 0 && gep->getPointerOperand()) {
            targetPointer = gep->getPointerOperand();
        } else {
            if(!hasPointsToObjects(targetPointer)) {
                targetPointer = targetPointer->stripPointerCasts();
            }
        }
        Value *targetValue = I.getValueOperand();
        gep = dyn_cast<GEPOperator>(targetValue);
        if(gep && gep->getNumOperands() > 0 && gep->getPointerOperand()) {
            targetValue = gep->getPointerOperand();
        }
        if(hasPointsToObjects(targetValue) || hasPointsToObjects(targetValue->stripPointerCasts())) {

            // handle pointer casts
            if(!hasPointsToObjects(targetValue)) {
                targetValue = targetValue->stripPointerCasts();
            }
            // Get the src points to information.
            std::set<PointerPointsTo *> *srcPointsTo = getPointsToObjects(targetValue);

            if(!hasPointsToObjects(targetPointer)) {
                targetPointer = targetPointer->stripPointerCasts();
            }
            // Get the dst points to information.
            std::set<PointerPointsTo *> *dstPointsTo = getPointsToObjects(targetPointer);
#ifdef STRICT_STORE

            assert(dstPointsTo != nullptr);
#endif
            if(dstPointsTo == nullptr) {
#ifdef DEBUG_STORE_INSTR
                dbgs() << "Trying to store something into pointer, which does not point to anything\n";
#endif
                return;
            }


            // we need to create new points to information.
            std::set<PointerPointsTo *> *newPointsToInfo = new std::set<PointerPointsTo *>();
            newPointsToInfo->insert(dstPointsTo->begin(), dstPointsTo->end());

            if (newPointsToInfo->size() <= 1) {
                //Strong update.
                if (newPointsToInfo->size() == 1) {
                    PointerPointsTo *dstPointsToObject = *(newPointsToInfo->begin());
                    // we have a pointer which points to only one object.
                    // Do a strong update
                    // Basic sanity
                    assert((dstPointsToObject->targetPointer == targetPointer ||
                            dstPointsToObject->targetPointer == targetPointer->stripPointerCasts()) &&
                           dstPointsToObject->fieldId == 0);

                    //OK, we need to change this points to.
                    PointerPointsTo *newDstPointsToObject = (PointerPointsTo *) dstPointsToObject->makeCopy();

                    // OK, now we got the target object to which the pointer points to.
                    // We are trying to store a pointer(*) into an object field

                    newDstPointsToObject->targetObject->performUpdate(newDstPointsToObject->dstfieldId,
                                                                      srcPointsTo, &I);

#ifdef DEBUG_STORE_INSTR
                    dbgs() << "Trying to perform strong update for store instruction:";
                    I.print(dbgs());
                    dbgs() << "\n";
#endif
                    // Now insert
                    newPointsToInfo->clear();
                    newPointsToInfo->insert(newPointsToInfo->begin(), newDstPointsToObject);
                } else {
                    // This is impossible.
                    // we are trying to store a value into pointer and the pointer
                    // cannot point to any object???
#ifdef DEBUG_STORE_INSTR
                    errs() << "Trying to store a value into pointer, which does not point to any object:";
                    I.print(errs());
#endif
#ifdef STRICT_STORE
                    assert(false);
#endif
                }

            } else {
                //Ok, this pointer can point to multiple objects
                //Perform weak update for each of the dst pointer points to
                newPointsToInfo->clear();
                for (PointerPointsTo *currPointsTo: *dstPointsTo) {
                    PointerPointsTo *newPointsToObj = (PointerPointsTo *) currPointsTo->makeCopy();
                    //Basic Sanity
                    assert(newPointsToObj->targetPointer == targetPointer && newPointsToObj->fieldId == 0);
                    // perform weak update
                    newPointsToObj->targetObject->performWeakUpdate(newPointsToObj->dstfieldId, srcPointsTo, &I);
#ifdef DEBUG_STORE_INSTR
                    dbgs() << "Performing weak update for store instruction:";
                    I.print(dbgs());
                    dbgs() << "\n";
#endif
                    newPointsToInfo->insert(newPointsToInfo->end(), newPointsToObj);
                }
            }
            this->updatePointsToObjects(targetPointer, newPointsToInfo);
        } else {
            // OK, we are storing something, which have no points to information.
            // Check if destination is not a pointer to pointer, which means
            // src value should have some points to information.
            // tl;dr This branch should never be entered.
            // Ensure that we are not storing into pointer to pointer
            if(!this->inside_loop) {
#ifdef DEBUG_STORE_INSTR
                errs() << "Source pointer does not point to any thing:";
                targetValue->print(errs());
                errs() << "; Ignoring.\n";
#endif
            }
            //assert(!I.getPointerOperand()->getType()->getContainedType(0)->isPointerTy());
        }

    }

    // The following instructions are ignored.
    // we will deal with them, if we find them
    void AliasAnalysisVisitor::visitVAArgInst(VAArgInst &I) {
        assert(false);
    }

    void AliasAnalysisVisitor::visitVACopyInst(VACopyInst &I) {
        assert(false);
    }

    void AliasAnalysisVisitor::setupCallContext(CallInst &I, Function *currFunction, std::vector<Instruction *> *newCallContext) {

        std::map<Value *, std::set<PointerPointsTo*>*> *currFuncPointsTo = currState.getPointsToInfo(newCallContext);

        // This ensures that we never analyzed this function with the same context.
        //assert(currFuncPointsTo->size() == 0);

        unsigned int arg_no = 0;

        for(User::op_iterator arg_begin = I.arg_begin(), arg_end = I.arg_end(); arg_begin != arg_end; arg_begin++) {
            Value *currArgVal =(*arg_begin).get();

            if(hasPointsToObjects(currArgVal) || hasPointsToObjects(currArgVal->stripPointerCasts())) {
                unsigned int farg_no;
                farg_no = 0;
                std::set<Value*> valuesToMerge;
                // handle pointer casts
                if(!hasPointsToObjects(currArgVal)) {
                    currArgVal = currArgVal->stripPointerCasts();
                }
                valuesToMerge.clear();
                valuesToMerge.insert(valuesToMerge.end(), currArgVal);

                for(Function::arg_iterator farg_begin = currFunction->arg_begin(), farg_end = currFunction->arg_end();
                    farg_begin != farg_end; farg_begin++) {
                    Value *currfArgVal = &(*farg_begin);
                    if(farg_no == arg_no) {
                        std::set<PointerPointsTo*> *currArgPointsTo = mergePointsTo(valuesToMerge, &I, currfArgVal);
                        // ensure that we didn't mess up.
                        assert(currArgPointsTo != nullptr);
#ifdef DEBUG_CALL_INSTR
                        // OK, we need to add pointsto.
                        dbgs() << "Argument:" << (arg_no + 1) << " has points to information\n";
#endif
                        (*currFuncPointsTo)[currfArgVal] = currArgPointsTo;
                        break;
                    }
                    farg_no++;
                }
            } else {
#ifdef DEBUG_CALL_INSTR
                dbgs() << "Argument:" << (arg_no + 1) << " does not point to any object\n";
#endif
            }
            arg_no++;
        }
    }


    void AliasAnalysisVisitor::handleMemcpyFunction(std::vector<long> &memcpyArgs, CallInst &I) {
        // handle memcpy instruction.
#ifdef DEBUG_CALL_INSTR
        dbgs() << "Processing memcpy function\n";
#endif
        // get src operand
        Value *srcOperand = I.getArgOperand((unsigned int) memcpyArgs[0]);
        // get dst operand
        Value *dstOperand = I.getArgOperand((unsigned int) memcpyArgs[1]);
        // handle pointer casts
        if(!hasPointsToObjects(srcOperand)) {
            srcOperand = srcOperand->stripPointerCasts();
        }
        if(!hasPointsToObjects(dstOperand)) {
            dstOperand = dstOperand->stripPointerCasts();
        }


        // get points to information.
        std::set<PointerPointsTo*>* srcPointsTo = getPointsToObjects(srcOperand);
        std::set<PointerPointsTo*>* dstPointsTo = getPointsToObjects(dstOperand);

        if(srcPointsTo != nullptr && dstPointsTo != nullptr) {
            // get all src objects.

            std::set<std::pair<long, AliasObject*>> srcAliasObjects;
            for(PointerPointsTo *currPointsTo:(*srcPointsTo)) {
                auto a = std::make_pair(currPointsTo->dstfieldId, currPointsTo->targetObject);
                if(srcAliasObjects.find(a) == srcAliasObjects.end()) {
                    srcAliasObjects.insert(a);
                }
            }

            std::set<std::pair<long, AliasObject*>> srcDrefObjects;
            for(auto a:srcAliasObjects) {
                a.second->fetchPointsToObjects(a.first, srcDrefObjects);
            }

            std::set<PointerPointsTo*> targetElements;
            for(auto a:srcDrefObjects) {
                PointerPointsTo *newRel = new PointerPointsTo();
                newRel->dstfieldId = a.first;
                newRel->targetObject = a.second;
                newRel->propogatingInstruction = &I;
                targetElements.insert(newRel);
            }

#ifdef DEBUG_CALL_INSTR
            dbgs() << "Got:" << targetElements.size() << " to add\n";
#endif
            for(auto a:(*dstPointsTo)) {
#ifdef DEBUG_CALL_INSTR
                dbgs() << "Adding:" << targetElements.size() << "elements to the fieldid:" << a->dstfieldId << "\n";
#endif
                a->targetObject->performWeakUpdate(a->dstfieldId, &targetElements, &I);
            }

            for(auto a:targetElements) {
                delete(a);
            }


        } else {
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Either src or dst doesn't have any points to information, "
                    "ignoring memory copy function in propagating points to information\n";
#endif
        }
    }


    // Need to implement these
    VisitorCallback* AliasAnalysisVisitor::visitCallInst(CallInst &I, Function *currFunc,
                                                         std::vector<Instruction *> *oldFuncCallSites,
                                                         std::vector<Instruction *> *callSiteContext) {

        std::string currFuncName = currFunc->getName().str();
        // if we do not have function definition
        // that means, it is a kernel internal function.
        // call kernel intra-function handler.
        if(currFunc->isDeclaration()) {
            FunctionChecker *targetChecker = (AliasAnalysisVisitor::functionHandler)->targetChecker;
            if(targetChecker->is_memcpy_function(currFunc)) {
                // handle memcpy function.
                std::vector<long> memcpyArgs = targetChecker->get_memcpy_arguments(currFunc);
                this->handleMemcpyFunction(memcpyArgs, I);
            } else {
                //std::set<PointerPointsTo*>* newPointsToInfo = KernelFunctionHandler::handleKernelFunction(I, currFunc, this->currFuncCallSites);
                bool is_handled;
                is_handled = false;
                std::set<PointerPointsTo *> *newPointsToInfo = (std::set<PointerPointsTo *> *) ((AliasAnalysisVisitor::functionHandler)->handleFunction(
                        I, currFunc,
                        (void *) (this->currFuncCallSites),
                        AliasAnalysisVisitor::callback,
                        is_handled));
                if (is_handled) {
#ifdef DEBUG_CALL_INSTR
                    dbgs() << "Function:" << currFuncName << " handled by the function handler\n";
#endif
                    if (newPointsToInfo != nullptr) {
#ifdef DEBUG_CALL_INSTR
                        dbgs() << "Function handler returned some points to info to add\n";
#endif
                        this->updatePointsToObjects(&I, newPointsToInfo);
                    }
                } else {
#ifdef DEBUG_CALL_INSTR
                    dbgs() << "Ignoring Kernel Function:" << currFuncName << "\n";
#endif
                }
            }
            return nullptr;
        }

        // Setup call context.
        setupCallContext(I, currFunc, callSiteContext);

        // Create a AliasAnalysisVisitor
        AliasAnalysisVisitor *vis = new AliasAnalysisVisitor(currState, currFunc, callSiteContext);

        return vis;
    }

    void AliasAnalysisVisitor::stitchChildContext(CallInst &I, VisitorCallback *childCallback) {
        AliasAnalysisVisitor *vis = (AliasAnalysisVisitor *)childCallback;
        if(vis->retValPointsTo.size() > 0 ){
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Stitching return value for call instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            std::set<PointerPointsTo*>* newPointsToInfo = this->copyPointsToInfo(&I, &(vis->retValPointsTo));
            if(newPointsToInfo != nullptr) {
                this->updatePointsToObjects(&I, newPointsToInfo);
            }
        }

    }

    void AliasAnalysisVisitor::visitReturnInst(ReturnInst &I) {
        Value *targetRetVal = I.getReturnValue();
        if(targetRetVal != nullptr && (hasPointsToObjects(targetRetVal) || hasPointsToObjects(targetRetVal->stripPointerCasts()))) {
            // check if pointer casts has a role to play?
            if(!hasPointsToObjects(targetRetVal)){
                targetRetVal = targetRetVal->stripPointerCasts();
            }
            std::set<PointerPointsTo*>* srcPointsTo = getPointsToObjects(targetRetVal);
            // Get all objects pointed by all the objects in the targetRetVal

            // this set stores the <fieldid, targetobject> of all the objects to which the targetRetVal points to.
            std::set<std::pair<long, AliasObject*>> targetObjects;
            for(PointerPointsTo *currPointsToObj:*srcPointsTo) {
                if(std::find_if(retValPointsTo.begin(), retValPointsTo.end(), [currPointsToObj](const PointerPointsTo *n) {
                    return  n->pointsToSameObject(currPointsToObj);
                }) == retValPointsTo.end()) {
                    long target_field = currPointsToObj->dstfieldId;
                    AliasObject *dstObj = currPointsToObj->targetObject;
                    auto to_check = std::make_pair(target_field, dstObj);
                    if (std::find(targetObjects.begin(), targetObjects.end(), to_check) == targetObjects.end()) {
                        targetObjects.insert(targetObjects.end(), to_check);
                        // insert into retval points to
#ifdef DEBUG_RET_INSTR
                        dbgs() << "Return value:";
                        I.print(dbgs());
                        dbgs() << ", points to some objects\n";
#endif
                        retValPointsTo.insert(retValPointsTo.end(), currPointsToObj);
                    }
                }
            }
        } else {
#ifdef DEBUG_RET_INSTR
            dbgs() << "Return value:";
            I.print(dbgs());
            dbgs() << ", does not point to any object. Ignoring.\n";
#endif
        }
    }


    void AliasAnalysisVisitor::printAliasAnalysisResults(llvm::raw_ostream& O) const {
        /***
         * Dump all the alias analysis result information into provided stream.
         */
        std::map<Value *, std::set<PointerPointsTo*>*>* targetPointsToMap = this->currState.getPointsToInfo(this->currFuncCallSites);
        for(auto ai:*targetPointsToMap) {
            O << "\nPointer:";
            ai.first->print(O);
            O << " has following points to information:\n";
            for(auto pp:*(ai.second)) {
                O << (*pp);
                O << "\n";
            }
        }
    }
}
