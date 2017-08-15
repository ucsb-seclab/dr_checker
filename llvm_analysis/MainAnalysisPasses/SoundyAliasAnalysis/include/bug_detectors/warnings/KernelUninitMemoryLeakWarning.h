//
// Created by machiry on 1/8/17.
//

#ifndef PROJECT_KERNELMEMORYLEAKWARNING_H
#define PROJECT_KERNELMEMORYLEAKWARNING_H
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
     * This is a warning when a memory not initialized by memset is copied to user space.
     */
    class KernelUninitMemoryLeakWarning: public VulnerabilityWarning {
    public:

        Value *targetObj;

        KernelUninitMemoryLeakWarning(Value *targetObj, std::vector<Instruction*> *callTrace,
        std::vector<Instruction*> *srcTrace, std::string warningMsg,
        Instruction *targetInstr, std::string found_by): VulnerabilityWarning(callTrace, srcTrace, warningMsg,
                                                                              targetInstr, found_by) {
            // Just make sure, we are leaking something.
            assert(targetObj != nullptr);
            this->targetObj = targetObj;
        }

        virtual void printCompleteWarning(llvm::raw_ostream& O) const;

        virtual void printWarning(llvm::raw_ostream& O) const;

    };
}

#endif //PROJECT_KERNELMEMORYLEAKWARNING_H
