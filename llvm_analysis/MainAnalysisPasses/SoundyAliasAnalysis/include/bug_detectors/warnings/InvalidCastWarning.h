//
// Created by machiry on 1/8/17.
//

#ifndef PROJECT_INVALIDCASTWARNING_H
#define PROJECT_INVALIDCASTWARNING_H
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "VulnerabilityWarning.h"


using namespace llvm;
namespace DRCHECKER {

    /***
     * This is a warning when an invalid cast is applied to a pointer.
     */
    class InvalidCastWarning: public VulnerabilityWarning {
    public:
        Value *srcObjPtr;
        long srcObjectSize;
        bool srcSizeTainted;
        long dstObjectSize;
        InvalidCastWarning(Value *srcObj, long srcSize, long dstSize, bool srcSizeTainted,
                           std::vector<Instruction*> *callTrace,
                           std::vector<Instruction*> *srcTrace, std::string warningMsg,
                           Instruction *targetInstr, std::string found_by): VulnerabilityWarning(callTrace,
                                                                                                 srcTrace,
                                                                                                 warningMsg,
                                                                                                 targetInstr,
                                                                                                 found_by) {
            this->srcObjPtr = srcObj;
            this->srcSizeTainted = srcSizeTainted;
            this->srcObjectSize = srcSize;
            this->dstObjectSize = dstSize;
        }

        virtual void printCompleteWarning(llvm::raw_ostream& O) const;

        virtual void printWarning(llvm::raw_ostream& O) const;

    };
}
#endif //PROJECT_INVALIDCASTWARNING_H
