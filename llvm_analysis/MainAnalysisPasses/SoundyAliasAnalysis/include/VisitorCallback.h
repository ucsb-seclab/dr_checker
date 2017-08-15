//
// Created by machiry on 12/4/16.
//

#ifndef PROJECT_VISITORCALLBACK_H
#define PROJECT_VISITORCALLBACK_H

#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
//#include "dsa/DSGraph.h"
//#include "dsa/DataStructure.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include <set>

using namespace llvm;

namespace DRCHECKER {

    /***
     *  All the flow analysis techniques which wish to use the
     *  global visitor should implement this call back.
     */
    class VisitorCallback {
    public:

        /***
         *  Function which will be called by the GlobalVisitor
         *  to indicate that the analysis is within loop
         * @param inside_loop true/false which indicates
         *                    that the analysis is within loop.
         */
        virtual void setLoopIndicator(bool inside_loop) {

        }

        virtual void visit(Instruction &I) {
        }

        virtual void visitBinaryOperator(BinaryOperator &I) {

        }

        virtual void visitPHINode(PHINode &I) {

        }

        virtual void visitSelectInst(SelectInst &I) {

        }

        virtual void visitLoadInst(LoadInst &I) {

        }

        virtual void visitStoreInst(StoreInst &I) {

        }

        virtual void visitGetElementPtrInst(GetElementPtrInst &I) {

        }

        virtual void visitAllocaInst(AllocaInst &I) {

        }

        virtual void visitVAArgInst(VAArgInst &I) {

        }

        virtual void visitVACopyInst(VACopyInst &I) {

        }

        virtual void visitCastInst(CastInst &I) {

        }

        virtual void visitICmpInst(ICmpInst &I) {

        }

        virtual void visitBranchInst(BranchInst &I) {

        }

        /***
         *  Visit the call instruction, this function should setup a new CallBack
         *  which will be used the GlobalVisitor to analyze the corresponding function.
         * @param I Call instruction.
         * @param targetFunction Function which is called by the provided call instruction.
         * @param oldFuncCallSites Context of the caller.
         * @param currFuncCallSites Context, basically list of call instructions.
         * @return VisitorCallback which should be used to analyze the targetFunction.
         */
        virtual VisitorCallback* visitCallInst(CallInst &I, Function *targetFunction,
                                               std::vector<Instruction *> *oldFuncCallSites,
                                               std::vector<Instruction *> *currFuncCallSites) {
            return nullptr;
        }

        /***
         * This function provides the VisitorCallback an opportunity to stitch back the result of function callback
         *  with the original callback.
         * @param I Callinstruction handled by the childCallback
         * @param childCallback Visitor which handled the call instruction (I)
         */
        virtual void stitchChildContext(CallInst &I, VisitorCallback *childCallback) {

        }

        virtual void visitReturnInst(ReturnInst &I) {

        }

        void visit(BasicBlock *BB) {

        }
    protected:
        // instructions where the warning has been generated.
        std::set<Instruction *> warnedInstructions;
    };
}

#endif //PROJECT_VISITORCALLBACK_H
