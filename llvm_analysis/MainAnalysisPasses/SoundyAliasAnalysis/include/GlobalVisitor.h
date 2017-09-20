//
// Created by machiry on 12/4/16.
//

#ifndef PROJECT_GLOBALVISITOR_H
#define PROJECT_GLOBALVISITOR_H
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
//#include "dsa/DSGraph.h"
//#include "dsa/DataStructure.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "ModuleState.h"
#include "VisitorCallback.h"

using namespace llvm;

namespace DRCHECKER {
//#define DEBUG_INSTR_VISIT
//#define FAST_HEURISTIC
    /***
     * The main guy handling the visiting of driver code.
     * SDTraversal of Fig 1
     */
    class GlobalVisitor : public InstVisitor<GlobalVisitor> {
    public:

        GlobalState &currState;

        std::vector<VisitorCallback *> &allCallbacks;

#ifdef FAST_HEURISTIC
        // a map of basic block to number of times it is analyzed.
        std::map<const BasicBlock*, unsigned long> numTimeAnalyzed;
#endif

        // order in which BBs needs to be analyzed.
        // This ideally should be in topological order of the
        // SCCs (Strongly connected components) in the CFG
        // of the function.
        std::vector<std::vector<BasicBlock *> *> *traversalOrder;

        // is the analysis within loop.
        bool inside_loop;

        // context of the analysis, basically list of call sites
        std::vector<Instruction *> *currFuncCallSites;

        // set of call sites already visited.
        // this will help in preventing analyzing function call
        // multiple times when in a loop.
        std::set<Instruction *> visitedCallSites;

        GlobalVisitor(GlobalState &targetState, Function *toAnalyze,
                      std::vector<Instruction *> *srcCallSites,
                      std::vector<std::vector<BasicBlock *> *> *bbTraversalOrder,
                      std::vector<VisitorCallback *> &targetCallbacks): allCallbacks(targetCallbacks), currState(targetState) {
            _super = static_cast<InstVisitor *>(this);
            // Initialize the call site list
            this->currFuncCallSites = srcCallSites;
            // BB traversal order should not be empty.
            assert(bbTraversalOrder != nullptr);
            this->traversalOrder = bbTraversalOrder;
            // ensure that we have a context for current function.
            targetState.getOrCreateContext(this->currFuncCallSites);
            // clearing all visited call sites.
            this->visitedCallSites.clear();
            this->inside_loop = false;
#ifdef FAST_HEURISTIC
            this->numTimeAnalyzed.clear();
#endif
        }

        virtual void visit(Instruction &I) {
#ifdef DEBUG_INSTR_VISIT
            dbgs() << "Visiting instruction:";
            I.print(dbgs());
            dbgs() << "\n";
#endif
            for(VisitorCallback *currCallback:allCallbacks) {
                currCallback->visit(I);
            }
            this->_super->visit(I);
        }


        // visitor functions.
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


        virtual void visitCallInst(CallInst &I);
        virtual void visitReturnInst(ReturnInst &I);
        virtual void visitICmpInst(ICmpInst &I);
        virtual void visitBranchInst(BranchInst &I);


        void visit(BasicBlock *BB);

        // main analysis function.
        void analyze();
    private:
        // maximum number of times a basic block can be analyzed.
        const static unsigned long MAX_NUM_TO_VISIT = 5;
        InstVisitor *_super;

        /***
         *  Process the function which is a target of the provided call instruction.
         *
         * @param I Current call instruction.
         * @param currFunc Potential function (a target of the call instruction)
         */
        void processCalledFunction(CallInst &I, Function *currFunc);
    };
}

#endif //PROJECT_GLOBALVISITOR_H
