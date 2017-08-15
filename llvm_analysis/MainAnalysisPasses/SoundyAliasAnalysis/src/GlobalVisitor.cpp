//
// Created by machiry on 12/4/16.
//
#include <CFGUtils.h>
#include "PointsToUtils.h"
#include "GlobalVisitor.h"

namespace DRCHECKER {

//#define DEBUG_GLOBAL_ANALYSIS
//#define DEBUG_CALL_INSTR
// #define DONOT_CARE_COMPLETION
#define MAX_CALLSITE_DEPTH 5
#define MAX_FUNC_PTR 5
#define SMART_FUNCTION_PTR_RESOLVING

    // Basic visitor functions.
    // call the corresponding function in the child callbacks.
    void GlobalVisitor::visitAllocaInst(AllocaInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitAllocaInst(I);
        }

    }

    void GlobalVisitor::visitCastInst(CastInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitCastInst(I);
        }
    }

    void GlobalVisitor::visitBinaryOperator(BinaryOperator &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitBinaryOperator(I);
        }
    }

    void GlobalVisitor::visitPHINode(PHINode &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitPHINode(I);
        }
    }

    void GlobalVisitor::visitSelectInst(SelectInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitSelectInst(I);
        }
    }

    void GlobalVisitor::visitGetElementPtrInst(GetElementPtrInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitGetElementPtrInst(I);
        }
    }

    void GlobalVisitor::visitLoadInst(LoadInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitLoadInst(I);
        }
    }

    void GlobalVisitor::visitStoreInst(StoreInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitStoreInst(I);
        }
    }

    void GlobalVisitor::visitVAArgInst(VAArgInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitVAArgInst(I);
        }
    }

    void GlobalVisitor::visitVACopyInst(VACopyInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitVACopyInst(I);
        }
    }

    void GlobalVisitor::visitReturnInst(ReturnInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitReturnInst(I);
        }
    }

    void GlobalVisitor::visitICmpInst(ICmpInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitICmpInst(I);
        }
    }

    void GlobalVisitor::visitBranchInst(BranchInst &I) {
        for(VisitorCallback *currCallback:allCallbacks) {
            currCallback->visitBranchInst(I);
        }
    }

    void GlobalVisitor::processCalledFunction(CallInst &I, Function *currFunc) {
        std::string currFuncName = currFunc->getName().str();
#ifdef DONOT_CARE_COMPLETION
        if(this->currFuncCallSites->size() > MAX_CALLSITE_DEPTH) {
            errs() << "MAX CALL SITE DEPTH REACHED, IGNORING:" << currFuncName << "\n";
            return;
        }
#endif

        // Create new context.
        //Set up arguments of the called function.
        std::vector<Instruction *> *newCallContext = new std::vector<Instruction *>();
        newCallContext->insert(newCallContext->end(), this->currFuncCallSites->begin(), this->currFuncCallSites->end());
        // create context.
        newCallContext->insert(newCallContext->end(), &I);
        this->currState.getOrCreateContext(newCallContext);

        // new callbacks that handles the current function.
        std::vector<VisitorCallback *> newCallBacks;

        // map of the parent visitor to corresponding child visitor.
        std::map<VisitorCallback *, VisitorCallback *> parentChildCallBacks;

        for(VisitorCallback *currCallback:allCallbacks) {
            VisitorCallback *newCallBack = currCallback->visitCallInst(I, currFunc, this->currFuncCallSites, newCallContext);
            if(newCallBack != nullptr) {
                newCallBacks.insert(newCallBacks.end(), newCallBack);
                parentChildCallBacks[currCallback] = newCallBack;
            }
        }
        // if there are new call backs? then create a GlobalVisitor and run the corresponding  visitor
        if(newCallBacks.size() > 0) {

            // Make sure we have the function definition.
            assert(!currFunc->isDeclaration());
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Analyzing new function:" << currFuncName << " Call depth:" << newCallContext->size() << "\n";
#endif

            std::vector<std::vector<BasicBlock *> *> *traversalOrder = BBTraversalHelper::getSCCTraversalOrder(
                    *currFunc);
            // Create a GlobalVisitor
            GlobalVisitor *vis = new GlobalVisitor(currState, currFunc, newCallContext, traversalOrder, newCallBacks);
            // Start analyzing the function.
            vis->analyze();

            // stitch back the contexts of all the member visitor callbacks.
            for(std::map<VisitorCallback *, VisitorCallback *>::iterator iter = parentChildCallBacks.begin();
                iter != parentChildCallBacks.end();
                ++iter)
            {
                VisitorCallback *parentCallback = iter->first;
                VisitorCallback *childCallback = iter->second;
                parentCallback->stitchChildContext(I, childCallback);
                delete(childCallback);
            }
            delete(vis);
        }
    }

    // Visit Call Instruction.
    void GlobalVisitor::visitCallInst(CallInst &I) {

        Function *currFunc = I.getCalledFunction();
        if(currFunc == nullptr) {
            // this is to handle casts.
            currFunc = dyn_cast<Function>(I.getCalledValue()->stripPointerCasts());
        }

        // ignore only if the current function is an external function
        if(currFunc == nullptr || !currFunc->isDeclaration()) {
            // check if the call instruction is already processed?
            if (this->visitedCallSites.find(&I) != this->visitedCallSites.end()) {
#ifdef DEBUG_CALL_INSTR
                dbgs() << "Function already processed:";
                I.print(dbgs());
                dbgs() << "\n";
#endif
                return;
            }
        }
        // insert into visited call sites.
        this->visitedCallSites.insert(this->visitedCallSites.end(), &I);


        if(currFunc != nullptr) {
            this->processCalledFunction(I, currFunc);
        } else {
            // if this is inline assembly, ignore the call instruction.
            if(I.isInlineAsm()) {
                return;
            }
#ifdef DEBUG_CALL_INSTR
            dbgs() << "Visiting Indirect call instruction.";
#endif
            Value *calledValue = I.getCalledValue();

            // get points to information of calledValue and look for only functions.
            std::vector<Function*> targetFunctions;
            targetFunctions.clear();
            bool hasTargets = PointsToUtils::getTargetFunctions(this->currState, this->currFuncCallSites,
                                                                calledValue, targetFunctions);
#ifdef SMART_FUNCTION_PTR_RESOLVING
            if(!hasTargets) {
                hasTargets = PointsToUtils::getPossibleFunctionTargets(I, targetFunctions);
#ifdef DEBUG_CALL_INSTR
                if(targetFunctions.size() > 0) {
                    dbgs() << "Function Pointer targets:" << targetFunctions.size() << "\n";
                }
#endif
                std::vector<Function *> filteredFunctions;
                for(unsigned i=0; i<MAX_FUNC_PTR && i<targetFunctions.size(); i++) {
                    filteredFunctions.push_back(targetFunctions[i]);
                }
#ifdef DEBUG_CALL_INSTR
                if(filteredFunctions.size() != targetFunctions.size()) {
                    dbgs() << "Ignoring Target Functions, Doing:"
                           << filteredFunctions.size()
                           << ", Got:" << targetFunctions.size() << "\n";
                }
#endif
                targetFunctions.clear();
                targetFunctions.insert(targetFunctions.end(), filteredFunctions.begin(), filteredFunctions.end());
            }
#endif
            // get potential target function from a given pointer.
            if(hasTargets) {
                assert(targetFunctions.size() > 0);
#ifdef DEBUG_CALL_INSTR
                dbgs() << "There are:" << targetFunctions.size() << " Target Functions.\n";
#endif
                for(Function *currFunction:targetFunctions) {
                    this->processCalledFunction(I, currFunction);
                }

            } else {

#ifdef DEBUG_CALL_INSTR
                dbgs() << "Function pointer does not point to any functions:";
                calledValue->print(dbgs());
                dbgs() << ", So Ignoring\n";
#endif
            }
        }
    }


    void GlobalVisitor::visit(BasicBlock *BB) {
        if(this->numTimeAnalyzed.find(BB) != this->numTimeAnalyzed.end()) {
            if(this->numTimeAnalyzed[BB] >= GlobalVisitor::MAX_NUM_TO_VISIT) {
#ifdef DEBUG_BB_VISIT
                dbgs() << "Ignoring BB:" << BB->getName().str()
                       << " ad it has been analyzed more than:"
                       << SAAVisitor::MAX_NUM_TO_VISIT << " times\n";
#endif
                return;
            }
            this->numTimeAnalyzed[BB] = this->numTimeAnalyzed[BB] + 1;
        } else {
            this->numTimeAnalyzed[BB] = 1;
        }
#ifdef DEBUG_BB_VISIT
        dbgs() << "Starting to analyze BB:" << BB->getName().str() << "\n";
#endif
        _super->visit(BB->begin(), BB->end());
    }

    void GlobalVisitor::analyze() {
        // the traversal order should not be null
        assert(this->traversalOrder != nullptr);
        for(unsigned int i = 0; i < this->traversalOrder->size(); i++){
            // current strongly connected component.
            std::vector<BasicBlock *> *currSCC = (*(this->traversalOrder))[i];
            if(currSCC->size() == 1) {
                for(VisitorCallback *currCallback:allCallbacks) {
                    currCallback->setLoopIndicator(false);
                }
                //Analyzing single basic block.
                for(unsigned int j=0; j < currSCC->size(); j++) {
                    BasicBlock* currBB = (*currSCC)[j];
                    this->visit(currBB);
                }

            } else {
                unsigned long opt_num_to_analyze = BBTraversalHelper::getNumTimesToAnalyze(currSCC);
#ifdef DEBUG_GLOBAL_ANALYSIS
                dbgs() << "Analyzing Loop BBS for:" << opt_num_to_analyze <<" number of times\n";
#endif

                for(VisitorCallback *currCallback:allCallbacks) {
                    currCallback->setLoopIndicator(true);
                }

                for(unsigned int l=0; l <= opt_num_to_analyze; l++) {
                    for (unsigned int j = 0; j < currSCC->size(); j++) {
                        BasicBlock *currBB = (*currSCC)[j];
                        this->visit(currBB);
                    }
                    // ensure that loop has been analyzed minimum number of times.
                    if(l >= (opt_num_to_analyze-1)) {
                        for(VisitorCallback *currCallback:allCallbacks) {
                            currCallback->setLoopIndicator(false);
                        }
                    }
                }
#ifdef DEBUG_GLOBAL_ANALYSIS
                dbgs() << "Analyzing Loop BBS END\n";
#endif
                //Analyzing loop.
            }
        }
    }
}

