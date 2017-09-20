//
// Created by machiry on 10/24/16.
//

#include <set>
#include <llvm/Support/Debug.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "TaintInfo.h"
#ifndef PROJECT_ALIASOBJECT_H
#define PROJECT_ALIASOBJECT_H

using namespace llvm;
#ifdef DEBUG
#undef DEBUG
#endif
namespace DRCHECKER {
//#define DEBUG_FUNCTION_ARG_OBJ_CREATION
//#define DEBUG_FETCH_POINTS_TO_OBJECTS

    class AliasObject;


    /***
     * Handles general points to relation.
     */
    class ObjectPointsTo {
    public:
        // field id, if the parent object is a structure.
        long fieldId;
        // field id of the destination object to which this pointer points tp
        long dstfieldId;
        // object to which we point to.
        AliasObject *targetObject;
        // instruction which resulted in this points to information.
        Value* propogatingInstruction;
        ObjectPointsTo() {

        }
        ~ObjectPointsTo() {

        }
        ObjectPointsTo(ObjectPointsTo *srcObjPointsTo) {
            this->fieldId = srcObjPointsTo->fieldId;
            this->dstfieldId = srcObjPointsTo->dstfieldId;
            this->targetObject = srcObjPointsTo->targetObject;
            this->propogatingInstruction = srcObjPointsTo->propogatingInstruction;
        }
        virtual ObjectPointsTo* makeCopy() {
            return new ObjectPointsTo(this);
        }
        virtual bool isIdenticalPointsTo(const ObjectPointsTo *that) const {
            // No default implementation
            assert(false);
        }

        virtual bool pointsToSameObject(const ObjectPointsTo *that) const {
            if(that != nullptr) {
                return this->targetObject == that->targetObject && this->dstfieldId == that->dstfieldId;
            }
            return false;
        }

        virtual long getTargetType() const {
            // Simple polymorphism.
            return 1;
        }

        /*virtual std::ostream& operator<<(std::ostream& os, const ObjectPointsTo& obj) {
            os << "Field :" << fieldId << " points to " << dstfieldId <<" of the object, with ID:" << obj.targetObject;
            return os;
        }*/
        friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const ObjectPointsTo& obj) {
            os << "Field :" << obj.fieldId << " points to " << obj.dstfieldId <<" of the object, with ID:" << obj.targetObject;
            return os;
        }
    };


    /***
     * Handles the pointer point to relation.
     */
    class PointerPointsTo: public ObjectPointsTo {
    public:
        const static long TYPE_CONST=2;
        // The src pointer that points to
        Value *targetPointer;

        PointerPointsTo(PointerPointsTo *srcPointsTo): ObjectPointsTo(srcPointsTo) {
            this->targetPointer = srcPointsTo->targetPointer;
        }

        PointerPointsTo() {

        }

        ObjectPointsTo* makeCopy() {
            return new PointerPointsTo(this);
        }
        long getTargetType() const {
            // Simple polymorphism.
            return PointerPointsTo::TYPE_CONST;
        }
        bool isIdenticalPointsTo(const ObjectPointsTo *that) const {
            if(that != nullptr && that->getTargetType() == PointerPointsTo::TYPE_CONST) {
                PointerPointsTo* actualObj = (PointerPointsTo*)that;
                return this->targetPointer == actualObj->targetPointer &&
                       this->targetObject == actualObj->targetObject &&
                       this->fieldId == actualObj->fieldId &&
                       this->dstfieldId == actualObj->dstfieldId;
            }
            return false;
        }

        /*std::ostream& operator<<(std::ostream& os, const ObjectPointsTo& obj) {
            PointerPointsTo* actualObj = (PointerPointsTo*)(&obj);
            os << "Pointer:";
            os << actualObj->targetPointer->getName().str();
            os << " from field:" << fieldId <<" points to field:"<< dstfieldId <<" of the object, with ID:" << this->targetObject;
            return os;
        }*/
        friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const PointerPointsTo& obj) {
            PointerPointsTo* actualObj = (PointerPointsTo *)(&obj);
            os << "Pointer:";
            os << actualObj->targetPointer->getName().str();
            os << " from field:" << obj.fieldId <<" points to field:"<< obj.dstfieldId <<" of the object, with ID:" << obj.targetObject;
            return os;
        }
    };


    static unsigned long idCount;

    static unsigned long getCurrID() {
        return idCount++;
    }

    /***
     * The alias object. Refer Definition 3.7 of the paper.
     */
    class AliasObject {
    public:
        Type* targetType;
        // All pointer variables that can point to this object.
        std::vector<PointerPointsTo *> pointersPointsTo;
        // This represents points from information, all objects which can point to this.
        std::vector<AliasObject*> pointsFrom;
        // All Objects that could be pointed by this object.
        std::vector<ObjectPointsTo*> pointsTo;

        //Information needed for Taint Analysis.
        // fields that store information which is tainted.
        std::vector<FieldTaint*> taintedFields;

        bool auto_generated;

        // field to indicate that all contents of this object
        // are tainted or not.
        bool all_contents_tainted = false;
        TaintFlag *all_contents_taint_flag = nullptr;

        // flag which indicates whether the object is initialized or not.
        // by default every object is initialized.
        bool is_initialized = true;
        // the set of instructions which initialize this object
        std::set<Instruction*> initializingInstructions;

        unsigned long id;



        unsigned long getID() const{
            return this->id;
        }

        AliasObject(AliasObject *srcAliasObject) {
            assert(srcAliasObject != nullptr);
            this->targetType = srcAliasObject->targetType;
            this->pointersPointsTo.insert(this->pointersPointsTo.end(), srcAliasObject->pointersPointsTo.begin(),
                                          srcAliasObject->pointersPointsTo.end());
            this->pointsFrom.insert(this->pointsFrom.end(), srcAliasObject->pointsFrom.begin(),
                                          srcAliasObject->pointsFrom.end());
            this->pointsTo.insert(this->pointsTo.end(), srcAliasObject->pointsTo.begin(),
                                          srcAliasObject->pointsTo.end());
            this->id = getCurrID();

            this->is_initialized = srcAliasObject->is_initialized;
            this->initializingInstructions.insert(srcAliasObject->initializingInstructions.begin(),
                                                  srcAliasObject->initializingInstructions.end());

        }
        AliasObject() {
        }

        ~AliasObject() {
            // delete all object points to
            for(ObjectPointsTo *ob:pointsTo) {
                delete(ob);
            }

            // delete all field taint.
            for(auto ft:taintedFields) {
                delete(ft);
            }
        }

        unsigned long countObjectPointsTo(long srcfieldId) {
            /***
             * Count the number of objects that could be pointer by
             * a field (i.e srcfieldId).
             */
            unsigned long numObjects = 0;
            for(ObjectPointsTo *obj:pointsTo) {
                if(obj->fieldId == srcfieldId) {
                    numObjects++;
                }
            }
            return numObjects;
        }

        void getAllPointsToObj(std::set<AliasObject*> &dstObjects) {
            /***
             * Get all objects this object can point to, from all the fields
             */
            for(auto currpo:this->pointsTo) {
                if(dstObjects.find(currpo->targetObject) == dstObjects.end()) {
                    dstObjects.insert(currpo->targetObject);
                }
            }
        }

        void performStrongUpdate(long srcfieldId, std::set<PointerPointsTo*>* dstPointsTo, Instruction *propogatingInstr) {
            /***
             * Make the field (srcfieldId) of this object point to
             * any of the objects pointed by dstPointsTo
             *
             * This function does strong update, i.e first it removes all points to information
             * for the field srcfieldId and then adds the new objects into points to set.
             */

            std::vector<ObjectPointsTo*> tmpCopy;
            tmpCopy.clear();
            // remove all objects, that could be pointed by srcfield id.
            for(auto a: this->pointsTo) {
                if(a->fieldId == srcfieldId) {
                    delete(a);
                } else {
                    tmpCopy.push_back(a);
                }
            }


            this->pointsTo.clear();
            this->pointsTo.insert(this->pointsTo.end(), tmpCopy.begin(), tmpCopy.end());

            this->updateFieldPointsTo(srcfieldId, dstPointsTo, propogatingInstr);
        }

        void performWeakUpdate(long srcfieldId, std::set<PointerPointsTo*>* dstPointsTo, Instruction *propogatingInstr) {
            /***
             * Similar to strong update but does weak update.
             * i.e it does not remove existing points to information of the field srcFieldId
             */
            this->updateFieldPointsTo(srcfieldId, dstPointsTo, propogatingInstr);

        }

        void performUpdate(long srcfieldId, std::set<PointerPointsTo*>* dstPointsTo, Instruction *propogatingInstr) {
            /***
             * Update the pointto information of the field pointed by srcfieldId
             */

            // check if we can perform strong update
            if(this->countObjectPointsTo(srcfieldId) <= 1) {
                this->performStrongUpdate(srcfieldId, dstPointsTo, propogatingInstr);
            } else {
                this->performWeakUpdate(srcfieldId, dstPointsTo, propogatingInstr);
            }
        }

        void getAllObjectsPointedByField(long srcfieldID, std::set<AliasObject *> &retSet) {
            for(ObjectPointsTo *obj:pointsTo) {
                if(obj->fieldId == srcfieldID) {
                    if(retSet.find(obj->targetObject) == retSet.end())  {
                        retSet.insert(obj->targetObject);
                    }
                }
            }
        }

        void updateFieldPointsToFromObjects(std::vector<ObjectPointsTo*>* dstPointsToObject,
                                            Instruction *propagatingInstr) {
            /***
            * Add all objects in the provided pointsTo set to be pointed by the provided srcFieldID
            */
            if(dstPointsToObject != nullptr) {
                std::set<AliasObject *> currObjects;
                //Add all objects that are in the provided set.
                for (ObjectPointsTo *currPointsTo: *dstPointsToObject) {
                    long srcfieldId = currPointsTo->fieldId;
                    // clear all the objects
                    currObjects.clear();
                    // first get all objects that could be pointed by srcfieldId of the current object.
                    getAllObjectsPointedByField(srcfieldId, currObjects);
                    // insert points to information only, if it is not present.
                    if (currObjects.find(currPointsTo->targetObject) == currObjects.end()) {
                        ObjectPointsTo *newPointsTo = currPointsTo->makeCopy();
                        newPointsTo->propogatingInstruction = propagatingInstr;
                        this->pointsTo.push_back(newPointsTo);
                    }
                }
            }
        }

        void addObjectToFieldPointsTo(long fieldId, AliasObject *dstObject, Instruction *propagatingInstr) {
            /***
            * Add provided object into pointsTo set of the provided fieldId
            */
            if(dstObject != nullptr) {
                std::set<AliasObject *> currObjects;
                long srcfieldId = fieldId;
                // clear all the objects
                currObjects.clear();
                // first get all objects that could be pointed by srcfieldId.
                getAllObjectsPointedByField(srcfieldId, currObjects);
                // insert points to information only,
                // if the object to be added is not present.
                if (currObjects.find(dstObject) == currObjects.end()) {
                    ObjectPointsTo *newPointsTo = new ObjectPointsTo();
                    newPointsTo->propogatingInstruction = propagatingInstr;
                    newPointsTo->fieldId = srcfieldId;
                    newPointsTo->dstfieldId = 0;
                    newPointsTo->targetObject = dstObject;
                    this->pointsTo.push_back(newPointsTo);
                }
            }
        }

        /*virtual void fetchPointsToObjects(long srcfieldId, std::set<AliasObject *> &dstObjects, Instruction *targetInstr = nullptr, bool create_arg_obj=false) {

             // Get all objects pointed by field identified by srcfieldID

            for(ObjectPointsTo *obj:pointsTo) {
                if(obj->fieldId == srcfieldId) {
                    if(std::find(dstObjects.begin(), dstObjects.end(), obj->targetObject) == dstObjects.end()) {
                        dstObjects.insert(dstObjects.end(), obj->targetObject);
                    }
                }
            }
        }*/

        void fetchPointsToObjects(long srcfieldId, std::set<std::pair<long, AliasObject*>> &dstObjects,
                                  Instruction *targetInstr = nullptr, bool create_arg_obj=false) {
            /***
             * Get all objects pointed by field identified by srcfieldID
             *
             * i.e if a field does not point to any object.
             * Automatically generate an object and link it with srcFieldId
             */
            bool hasObjects = false;
#ifdef DEBUG_FETCH_POINTS_TO_OBJECTS
            dbgs() << "In AliasObject fetch pointsTo object\n";
#endif
            for(ObjectPointsTo *obj:pointsTo) {
                if(obj->fieldId == srcfieldId) {
                    auto p = std::make_pair(obj->dstfieldId, obj->targetObject);
                    if(std::find(dstObjects.begin(), dstObjects.end(), p) == dstObjects.end()) {
                        dstObjects.insert(dstObjects.end(), p);
                        hasObjects = true;
                    }
                }
            }
            // if there are no objects that this field points to, generate a dummy object.
            if(!hasObjects && (create_arg_obj || this->isFunctionArg())) {
#ifdef DEBUG_FETCH_POINTS_TO_OBJECTS
                dbgs() << "Creating a new dynamic AliasObject at:";
                targetInstr->print(dbgs());
                dbgs() << "\n";
#endif
                AliasObject *newObj = this->makeCopy();
                ObjectPointsTo *newPointsToObj = new ObjectPointsTo();
                newPointsToObj->propogatingInstruction = targetInstr;
                newPointsToObj->targetObject = newObj;
                newPointsToObj->fieldId = srcfieldId;
                // this is the field of the newly created object to which
                // new points to points to
                newPointsToObj->dstfieldId = 0;
                newObj->auto_generated = true;

                // get the taint for the field and add that taint to the newly created object
                std::set<TaintFlag*> *fieldTaint = getFieldTaintInfo(srcfieldId);
#ifdef DEBUG_FETCH_POINTS_TO_OBJECTS
                dbgs() << "Trying to get taint for field:" << srcfieldId << " for object:" << this << "\n";
#endif
                //TODO: debug add this info
                if(fieldTaint != nullptr) {
#ifdef DEBUG_FETCH_POINTS_TO_OBJECTS
                    dbgs() << "Adding taint for field:" << srcfieldId << " for object:" << newObj << "\n";
#endif
                    for(auto existingTaint:*fieldTaint) {
                        newObj->taintAllFields(existingTaint);
                    }
                } else {
                    // if all the contents are tainted?
                    if(this->all_contents_tainted) {
                        dbgs() << "Trying to get field from an object whose contents are fully tainted\n";
                        assert(this->all_contents_taint_flag != nullptr);
                        newObj->taintAllFields(this->all_contents_taint_flag);
                    }
                }

                //insert the newly create object.
                pointsTo.push_back(newPointsToObj);

                dstObjects.insert(dstObjects.end(), std::make_pair(0, newObj));
            }
        }

        //TaintInfo helpers start

        /***
         * Get the set of taint flag of the provided field.
         * @param srcfieldId field id for which taint need to be fetched.
         * @return set of taint flags.
         */
        std::set<TaintFlag*> *getFieldTaintInfo(long srcfieldId) {
            FieldTaint *targetFieldTaint = this->getFieldTaint(srcfieldId);
            if(targetFieldTaint != nullptr) {
                return &(targetFieldTaint->targetTaint);
            } else {
                if (this->all_contents_taint_flag) {
                    this->addFieldTaintFlag(srcfieldId, this->all_contents_taint_flag);
                    // This cannot be null because we have just added it.
                    return &(this->getFieldTaint(srcfieldId)->targetTaint);
                }
            }
            return nullptr;
        }

        /***
         * Add provided taint flag to the object at the provided field.
         * @param srcfieldId field to which taint needs to be added.
         * @param targetTaintFlag TaintFlag which needs to be added to the
         *                         provided field.
         * @return true if added else false if the taint flag is a duplicate.
         */
        bool addFieldTaintFlag(long srcfieldId, TaintFlag *targetTaintFlag) {
            FieldTaint *targetFieldTaint = this->getFieldTaint(srcfieldId);
            if(targetFieldTaint == nullptr) {
                targetFieldTaint = new FieldTaint(srcfieldId);
                this->taintedFields.push_back(targetFieldTaint);
            }
            return targetFieldTaint->addTaintFlag(targetTaintFlag);
        }

        /***
         * Add provided taint to all the fields of this object.
         * @param targetTaintFlag TaintFlag that need to be added to all the fields.
         *
         * @return true if added else false if the taint flag is a duplicate.
         */
        bool taintAllFields(TaintFlag *targetTaintFlag) {
            std::set<long> allAvailableFields;
            if(!this->all_contents_tainted) {
                this->all_contents_tainted = true;
                this->all_contents_taint_flag = targetTaintFlag;
                if (this->targetType->isStructTy()) {
                    StructType *resStType = dyn_cast<StructType>(this->targetType);
#ifdef DEBUG
                    dbgs() << "\nIs a structure type:" << resStType->getNumElements() << "\n";
#endif
                    for (long i = 0; i < (resStType->getNumElements()); i++) {
                        allAvailableFields.insert(allAvailableFields.end(), i);
                    }

                } else if (this->targetType->isPointerTy() && this->targetType->getContainedType(0)->isStructTy()) {
                    // if this is a pointer to a struct.
                    StructType *resStType = dyn_cast<StructType>(this->targetType->getContainedType(0));
                    for (long i = 0; i < (resStType->getNumElements()); i++) {
                        allAvailableFields.insert(allAvailableFields.end(), i);
                    }
                } else if (this->pointsTo.size()) {
                    // has some points to?
                    // iterate thru pointsTo and get all fields.
                    for (auto objPoint:this->pointsTo) {
                        long currFieldID = objPoint->fieldId;
                        // no added? then add.
                        if (allAvailableFields.find(currFieldID) == allAvailableFields.end()) {
                            allAvailableFields.insert(allAvailableFields.end(), currFieldID);
                        }
                    }

                } else {
                    // This must be a scalar type.
                    // just add taint to the field 0.
                    allAvailableFields.insert(allAvailableFields.end(), 0);
                }

                // add the taint to all available fields.
                for (auto fieldId:allAvailableFields) {
#ifdef DEBUG
                    dbgs() << "Adding taint to field:" << fieldId << "\n";
#endif
                    addFieldTaintFlag(fieldId, targetTaintFlag);
                }

                return true;
            }
            return false;

        }

        //TaintInfo helpers end



        virtual AliasObject* makeCopy() {
            return new AliasObject(this);
        }

        virtual Value* getObjectPtr() {
            return nullptr;
        }


        virtual bool isSameObject(AliasObject *other) {
            return this == other;
        }

        virtual Value* getAllocSize() {
            return nullptr;
        }

        virtual int64_t getTypeAllocSize(DataLayout *targetDL) {
            // if there is no type or this is a void*, then we do not know the alloc size.
            if(targetType == nullptr ||
                    (targetType->isPointerTy() &&
                                         targetType->getContainedType(0)->isIntegerTy(8)) ||
                    (!targetType->isSized())) {
                return -1;
            }
            return targetDL->getTypeAllocSize(targetType);
        }

        friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const AliasObject& obj) {
            os << "Object with type:";
            obj.targetType->print(os);
            os <<" ID:" << &(obj) << "\n";
            obj.printPointsTo(os);
            return os;
        }

        virtual bool isFunctionArg() {
            /***
             * checks whether the current object is a function argument.
             */
            return false;
        }

        virtual bool isHeapObject() {
            /***
             * Returns True if this object is a Heap object.
             */
            return false;
        }

        virtual bool isGlobalObject() {
            /***
             * Returns True if this object is a Global object.
             */
            return false;
        }

        virtual long getArraySize() {
            /***
             *  Returns array size, if this is array object.
             *  else returns -1
             */
             if(this->targetType != nullptr && this->targetType->isArrayTy()) {
                 return this->targetType->getArrayNumElements();
             }
            return -1;
        }

    private:

        FieldTaint* getFieldTaint(long srcfieldId) {
            for(auto currFieldTaint:taintedFields) {
                if(currFieldTaint->fieldId == srcfieldId) {
                    return currFieldTaint;
                }
            }
            return nullptr;
        }


        void updateFieldPointsTo(long srcfieldId, std::set<PointerPointsTo*>* dstPointsTo, Instruction *propogatingInstr) {
            /***
             * Add all objects in the provided pointsTo set to be pointed by the provided srcFieldID
             */
            if(dstPointsTo != nullptr) {
                std::set<AliasObject*> currObjects;
                // first get all objects that could be pointed by srcfieldId of the current object.
                getAllObjectsPointedByField(srcfieldId, currObjects);
                //Add all objects that are in the provided set by changing the field id.
                for (PointerPointsTo *currPointsTo: *dstPointsTo) {
                    // insert points to information only, if it is not present.
                    if(currObjects.find(currPointsTo->targetObject) == currObjects.end()) {
                        ObjectPointsTo *newPointsTo = currPointsTo->makeCopy();
                        newPointsTo->fieldId = srcfieldId;
                        newPointsTo->propogatingInstruction = propogatingInstr;
                        this->pointsTo.push_back(newPointsTo);
                    }
                }
            }

        }



    protected:
        void printPointsTo(llvm::raw_ostream& os) const {
            os << "Points To Information:\n";
            for(ObjectPointsTo *obp: this->pointsTo) {
                os << "\t";
                os << (*obp);
                os << "\n";
            }
        }
    };

    class FunctionLocalVariable : public AliasObject {
    public:
        Function *targetFunction;
        AllocaInst *targetAllocaInst;
        Value *targetVar;
        // This differentiates local variables from different calling context.
        std::vector<Instruction *> *callSiteContext;

        FunctionLocalVariable(AllocaInst &targetInst, std::vector<Instruction *> *callSites) {
            this->targetFunction = targetInst.getFunction();
            this->targetType = targetInst.getAllocatedType();
            this->targetAllocaInst = &targetInst;
            this->callSiteContext = callSites;
            // get the local variable to which this is allocated.
            this->targetVar = &targetInst;
            if(targetInst.getAllocatedType()->isStructTy()) {
                this->is_initialized = false;
                this->initializingInstructions.clear();
            }
        }

        FunctionLocalVariable(FunctionLocalVariable *srcLocalVariable): AliasObject(srcLocalVariable) {
            this->targetFunction = srcLocalVariable->targetFunction;
            this->targetAllocaInst = srcLocalVariable->targetAllocaInst;
            this->targetVar = srcLocalVariable->targetVar;
            this->callSiteContext = srcLocalVariable->callSiteContext;
            this->targetType = srcLocalVariable->targetType;
        }

        AliasObject* makeCopy() {
            return new FunctionLocalVariable(this);
        }

        Value* getObjectPtr() {
            return this->targetAllocaInst;
        }

        friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const FunctionLocalVariable& obj) {
            os << "Function Local variable with type:";
            obj.targetType->print(os);
            os <<" ID:" << obj.id << "\n";
            obj.printPointsTo(os);
            return os;
        }


    };

    class GlobalObject : public AliasObject {
    public:
        Value *targetVar;
        GlobalObject(llvm::GlobalVariable *globalDef, Type *globVarType) {
            this->targetVar = (Value*)globalDef;
            this->targetType = globVarType;
        }
        GlobalObject(Value* globalVal, Type *globVarType) {
            this->targetVar = globalVal;
            this->targetType = globVarType;
        }
        GlobalObject(Function *targetFunction) {
            this->targetVar = targetFunction;
            this->targetType = targetFunction->getType();
        }
        GlobalObject(GlobalObject *origGlobVar): AliasObject(origGlobVar) {
            this->targetVar = origGlobVar->targetVar;
            this->targetType = origGlobVar->targetType;
        }
        AliasObject* makeCopy() {
            return new GlobalObject(this);
        }
        Value* getObjectPtr() {
            return this->targetVar;
        }

        bool isGlobalObject() {
            return true;
        }
    };

    class HeapLocation : public AliasObject {
    public:
        Function *targetFunction;
        Instruction *targetAllocInstruction;
        std::vector<Instruction *> *callSiteContext;
        Value *targetAllocSize;
        bool is_malloced;

        HeapLocation(Instruction &allocSite, Type* targetType, std::vector<Instruction *> *callSites,
                     Value *allocSize, bool is_malloced) {
            this->targetType = targetType;
            this->targetAllocInstruction = &allocSite;
            this->targetFunction = allocSite.getParent()->getParent();
            this->callSiteContext = callSites;
            this->targetAllocSize = allocSize;
            this->is_malloced = is_malloced;
            this->is_initialized = false;
            this->initializingInstructions.clear();
        }
        HeapLocation(HeapLocation *srcHeapLocation): AliasObject(srcHeapLocation) {
            this->targetAllocInstruction = srcHeapLocation->targetAllocInstruction;
            this->targetFunction = srcHeapLocation->targetFunction;
            this->targetType = srcHeapLocation->targetType;
            this->callSiteContext = srcHeapLocation->callSiteContext;
            this->targetAllocSize = srcHeapLocation->targetAllocSize;
            this->is_malloced = srcHeapLocation->is_malloced;
        }
        AliasObject* makeCopy() {
            return new HeapLocation(this);
        }
        Value* getObjectPtr() {
            return this->targetAllocInstruction;
        }

        Value* getAllocSize() {
            return targetAllocSize;
        }

        bool isHeapObject() {
            /***
             * Return true if this is malloced
             */
            return this->is_malloced;
        }

    };

    class FunctionArgument : public AliasObject {
    public:
        std::vector<Instruction *> *callSiteContext;
        Function *targetFunction;
        Value *targetArgument;
        // TODO: handle pointer args
        FunctionArgument(Value *targetArgument, Type* targetType, Function *tarFunction, std::vector<Instruction *> *callSites) {
            this->targetType = targetType;
            this->targetFunction = tarFunction;
            this->targetArgument = targetArgument;
            this->callSiteContext = callSites;
        }
        FunctionArgument(FunctionArgument *srcFunctionArg) : AliasObject(srcFunctionArg) {
            this->targetFunction = srcFunctionArg->targetFunction;
            this->targetArgument = srcFunctionArg->targetArgument;
            this->callSiteContext = srcFunctionArg->callSiteContext;
            this->targetType = srcFunctionArg->targetType;
        }
        AliasObject* makeCopy() {
            return new FunctionArgument(this);
        }
        Value* getObjectPtr() {
            return this->targetArgument;
        }

        /*void fetchPointsToObjects(long srcfieldId, std::set<AliasObject *> &dstObjects, Instruction *targetInstr = nullptr, bool create_arg_obj=false) {
            /***
             * Get all objects pointed by field identified by srcfieldID
             *
             * If this is a function argument, we should be able to generate the object on demand.
             *
             * i.e if a field does not point to any object.
             * Automatically generate an object and link it with srcFieldId

            bool hasObjects = false;
#ifdef DEBUG_FUNCTION_ARG_OBJ_CREATION
            dbgs() << "In Function ARG fetch pointsTo object\n";
#endif
            for(ObjectPointsTo *obj:pointsTo) {
                if(obj->fieldId == srcfieldId) {
                    if(std::find(dstObjects.begin(), dstObjects.end(), obj->targetObject) == dstObjects.end()) {
                        dstObjects.insert(dstObjects.end(), obj->targetObject);
                        hasObjects = true;
                    }
                }
            }
            // if there are no objects that this field points to, generate a dummy object.
            if(!hasObjects && create_arg_obj) {
#ifdef DEBUG_FUNCTION_ARG_OBJ_CREATION
                dbgs() << "Creating a new dynamic function arg object at:";
                targetInstr->print(dbgs());
                dbgs() << "\n";
#endif
                AliasObject *newObj = this->makeCopy();
                ObjectPointsTo *newPointsToObj = new ObjectPointsTo();
                newPointsToObj->propogatingInstruction = targetInstr;
                newPointsToObj->targetObject = newObj;
                newPointsToObj->fieldId = srcfieldId;
                newPointsToObj->dstfieldId = 0;
                newObj->auto_generated = true;

                //insert the newly create object.
                pointsTo.push_back(newPointsToObj);

                dstObjects.insert(dstObjects.end(), newObj);
            }
        }*/

        bool isFunctionArg() {
            return true;
        }
    };


}

#endif //PROJECT_ALIASOBJECT_H
