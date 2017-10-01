//
// Created by machiry on 12/5/16.
//

#ifndef PROJECT_TAINTANALYSISVISITOR_H
#define PROJECT_TAINTANALYSISVISITOR_H

#include "ModuleState.h"
#include "VisitorCallback.h"
#include "TaintInfo.h"
#include <FunctionChecker.h>

using namespace llvm;

namespace DRCHECKER {

    /***
     * The main class that implements the taint propagation for all the relevant
     * instructions.
     */
    class TaintAnalysisVisitor : public VisitorCallback {

    public:
        GlobalState &currState;
        Function *targetFunction;

        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;

        // set of taint flags and the possible return values.
        std::set<TaintFlag*> retValTaints;

        // object which is used to check some functions for taint tracking.
        static FunctionChecker *functionChecker;

        TaintAnalysisVisitor(GlobalState &targetState,
                             Function *toAnalyze,
                             std::vector<Instruction *> *srcCallSites): currState(targetState) {
            targetFunction = toAnalyze;
            // Initialize the call site list
            this->currFuncCallSites = srcCallSites;
            // ensure that we have a context for current function.
            targetState.getOrCreateContext(this->currFuncCallSites);
            // clear all points to information for retval;
            retValTaints.clear();
        }

        ~TaintAnalysisVisitor() {
        }

        // Visitors

        virtual void setLoopIndicator(bool inside_loop) {
            // nothing special to do when inside loop
        }

        virtual void visit(Instruction &I) {
#ifdef DEBUG_TAINT_INSTR_VISIT
            dbgs() << "Visiting instruction(In TaintAnalysis):";
            I.print(dbgs());
            dbgs() << "\n";
#endif
        }

        virtual void visitBinaryOperator(BinaryOperator &I);

        virtual void visitPHINode(PHINode &I);

        virtual void visitSelectInst(SelectInst &I);

        virtual void visitLoadInst(LoadInst &I);

        virtual void visitStoreInst(StoreInst &I);

        virtual void visitGetElementPtrInst(GetElementPtrInst &I);

        virtual void visitAllocaInst(AllocaInst &I);

        virtual void visitVAArgInst(VAArgInst &I);

        virtual void visitVACopyInst(VACopyInst &I);

        virtual void visitCastInst(CastInst &I);

        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites);

        virtual void stitchChildContext(CallInst &I, VisitorCallback *childCallback);

        virtual void visitReturnInst(ReturnInst &I);

        virtual void visitICmpInst(ICmpInst &I);

    private:
        /***
         * Get the set of taint flags of the provided value.
         * @param targetVal Value whose taint flags needs to be fetched.
         * @return Set of taint flags of the provided value.
         */
        std::set<TaintFlag*>* getTaintInfo(Value *targetVal);

        /***
         *
         * @param targetVal
         * @param retTaintFlag
         */
        void getPtrTaintInfo(Value *targetVal, std::set<TaintFlag*> &retTaintFlag);

        /***
         * Update the taint information of the provided value by the the set of flags.
         * @param targetVal value whose taint information need to be updated.
         * @param targetTaintInfo set containing the new taint flags for the provided value.
         */
        void updateTaintInfo(Value *targetVal, std::set<TaintFlag*> *targetTaintInfo);


        /***
         * Merge the taint flags of all the values into taint flags of the provided targetInstr.
         *
         * @param srcVals values whose taint values need to be merged.
         * @return Set of new taint flags
         */
        std::set<TaintFlag*> *mergeTaintInfo(std::set<Value *> &srcVals, Value *targetInstr);


        /***
         * Add a new taint flag in to the provided set.
         * This function adds only if the taint flag does not already exists in the provided set.
         * @param newTaintInfo set of taint flag to which the new taint flag should be added.
         * @param newTaintFlag new taint flag that needs to be added.
         */
        static void addNewTaintFlag(std::set<TaintFlag*> *newTaintInfo, TaintFlag *newTaintFlag);

        /***
         * Propogate taint to the provided arguments according to their index.
         * @param taintedArgs list containing indexes of tainted arguments.
         * @param I Call instruction responsible for this operation.
         */
        void propogateTaintToArguments(std::set<long> &taintedArgs, CallInst &I);

        /***
         * Propagate taint to the arguments of a memcpy function.
         * @param memcpyArgs indexes of source and destination pointers of the memcpy function.
         * @param I Call instruction responsible for this operation.
         */
        void propagateTaintToMemcpyArguments(std::vector<long> &memcpyArgs, CallInst &I);

        /***
         * Set up new call context for the taint analysis to run on the provided function.
         *
         * @param I Call Instruction responsible for this operation.
         * @param currFunction target function to which the context needs to be setup.
         * @param newCallContext list of callsites to be used for the new context.
         */
        void setupCallContext(CallInst &I, Function *currFunction, std::vector<Instruction *> *newCallContext);

        /***
         * Handle kernel internal function, this function handles function which are kernel internal.
         * i.e function which are kernel internal.
         * @param I Instruction responsible for this operation.
         * @param currFunc target internal kernel function.
         */
        void handleKernelInternalFunction(CallInst &I, Function *currFunc);

        /***
         * Copy taint info from one operand to other operand.
         *
         * @param srcOperand source operand from which the taint info need to be copied.
         * @param targetInstruction Destination instruction to which the taint information needs to be copied to.
         * @param srcTaintInfo set containing taint info that needs to be copied.
         * @param dstTaintInfo [optional] set to which the copied taint info needs to be added
         * @return pointer to the newly created or provided via dstTaintInfo set of taint info
         */
        std::set<TaintFlag*>* makeTaintInfoCopy(Value *srcOperand, Instruction *targetInstruction,
                                                std::set<TaintFlag*>* srcTaintInfo,
                                                std::set<TaintFlag*> *dstTaintInfo = nullptr);


    };
}

#endif //PROJECT_TAINTANALYSISVISITOR_H
