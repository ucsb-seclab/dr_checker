//
// Created by machiry on 12/4/16.
//

#ifndef PROJECT_ALIASANALYSISVISITOR_H
#define PROJECT_ALIASANALYSISVISITOR_H

#include "ModuleState.h"
#include "CFGUtils.h"
#include "VisitorCallback.h"
#include <FunctionChecker.h>


using namespace llvm;

namespace DRCHECKER {

    /***
     * The main class that implements the alias analysis visitor for all the relevant
     * instructions.
     */
    class AliasAnalysisVisitor: public VisitorCallback {
    public:
        GlobalState &currState;
        Function *targetFunction;
        // all pointer variables in the current function
        std::set<Value *> allLocalPointers;
        // all local allocs
        std::set<AliasObject *> allLocalAllocs;
        // function arguments
        std::set<AliasObject *> functionArguments;

        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;

        // set of points to which the return value(s) can point to
        std::set<PointerPointsTo*> retValPointsTo;

        // the call back that needs to be called for handling undefined functions.
        static FunctionHandlerCallback *callback;

        // object which handles functions, which are not defined.
        static FunctionHandler *functionHandler;

        bool inside_loop;

        AliasAnalysisVisitor(GlobalState &targetState,
        Function *toAnalyze,
                std::vector<Instruction *> *srcCallSites): currState(targetState) {
                targetFunction = toAnalyze;
                // Initialize the call site list
                this->currFuncCallSites = srcCallSites;
                // ensure that we have a context for current function.
                targetState.getOrCreateContext(this->currFuncCallSites);
                // clear all points to information for retval;
                retValPointsTo.clear();
        }

        virtual void setLoopIndicator(bool inside_loop);

        virtual void visit(Instruction &I) {
#ifdef DEBUG_ALIAS_INSTR_VISIT
                dbgs() << "Visiting instruction(In AliasAnalysis):";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        }

        // Implement the visitors

        virtual void visitBinaryOperator(BinaryOperator &I);
        virtual void visitPHINode(PHINode &I);
        virtual void visitSelectInst(SelectInst &I);


        virtual void visitLoadInst(LoadInst &I);
        virtual void visitStoreInst(StoreInst &I);
        virtual void visitGetElementPtrInst(GetElementPtrInst &I);

        // Allocator instructions.
        virtual void visitAllocaInst(AllocaInst &I);

        //The following instructions are ignored.
        virtual void visitVAArgInst(VAArgInst &I);
        virtual void visitVACopyInst(VACopyInst &I);

        // all casting instructions
        virtual void visitCastInst(CastInst &I);


        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);

        virtual void stitchChildContext(CallInst &I, VisitorCallback *childCallback);

        virtual void visitReturnInst(ReturnInst &I);



        /*virtual bool merge(BasicBlock *curBB, BasicBlock *succBB);*/

        void printAliasAnalysisResults(llvm::raw_ostream& O) const;


    private:
        // maximum number of times a basic block can be analyzed.
        const static unsigned long MAX_NUM_TO_VISIT = 5;


        /***
         * Gets the set of objects to which the provided pointer points to.
         * @param srcPointer Pointer whose points to information needs to be fetched.
         * @return Pointer to the set of objects to which the provided pointer points to.
         */
        std::set<PointerPointsTo*>* getPointsToObjects(Value *srcPointer);
        /***
         * Update points to information for the provided pointer.
         * @param srcPointer pointer whose points to information need to be updated.
         * @param newPointsToInfo the set of points to information for the provided pointer.
         */
        void updatePointsToObjects(Value *srcPointer, std::set<PointerPointsTo*>* newPointsToInfo);
        /***
         * This function checks if the provided pointer has points to information.
         * @param srcPointer Pointer which needs to be checked for points to information.
         * @return true/false depending on whether points to information exists or not.
         */
        bool hasPointsToObjects(Value *srcPointer);
        /***
         * Make copy of the points-to information such that srcPointer points to
         *  all the objects at provided fieldId.
         *
         * @param propInstruction The instruction responsible for this.
         * @param srcPointer Pointer whose points to copies need to be made.
         * @param srcPointsTo Points-to information whose copy needs to be made.
         * @param fieldId fieldId of the objects to which the points-to copy need to be made.
         * @return Set of new points to information.
         */
        std::set<PointerPointsTo*>* makePointsToCopy(Instruction *propInstruction, Value *srcPointer,
                                                     std::set<PointerPointsTo*>* srcPointsTo, long fieldId=-1);

        /***
         * Merge points-to information of all the provided values.
         * @param valuesToMerge set of values whose points to information need to be merged.
         * @param targetInstruction instruction responsible for this.
         * @param targetPtr Resulting pointer which points to the merged points to information.
         * @return Set of new points to information.
         */
        std::set<PointerPointsTo*>* mergePointsTo(std::set<Value*> &valuesToMerge, Instruction *targetInstruction,
                                                  Value *targetPtr=nullptr);

        /***
         * Set up call context for the provided function.
         * @param I Call Instruction the should be handled.
         * @param currFunction Function which is the destination of the provided call instruction.
         * @param newCallContext Context (list of call instructions) to be used for the call.
         */
        void setupCallContext(CallInst &I, Function *currFunction, std::vector<Instruction *> *newCallContext);

        /***
         * Make copy of the provided points to information, such that provided instruction points to the
         * new points to copy.
         *
         * @param srcInstruction Instruction which points to the newly made points-to copy
         * @param srcPointsTo Points to information whose copy needs to be made.
         * @return newly created set of points to information.
         */
        std::set<PointerPointsTo*>* copyPointsToInfo(Instruction *srcInstruction,
                                                     std::set<PointerPointsTo*>* srcPointsTo);

        /***
         * Handle memcpy function relavant to this analysis.
         * @param memcpyArgs arguments of the function called by the provided CallInst
         * @param I Call instruction which resulted in calling of the memcpy function.
         */
        void handleMemcpyFunction(std::vector<long> &memcpyArgs, CallInst &I);

        void handleInlinePointerOperand(Instruction &currIns, Value **srcPointer);
    };

}

#endif //PROJECT_ALIASANALYSISVISITOR_H
