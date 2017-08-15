//
// Created by machiry on 1/8/17.
//

#ifndef PROJECT_TAINTEDSIZEWARNING_H
#define PROJECT_TAINTEDSIZEWARNING_H
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
     * This is a vulnerability warning emitted when a TaintedSize is used to copy into user
     * or from user.
     */
    class TaintedSizeWarning: public VulnerabilityWarning {
    public:
        TaintedSizeWarning(std::vector<Instruction*> *callTrace,
                           std::vector<Instruction*> *srcTrace, std::string warningMsg,
                           Instruction *targetInstr, std::string found_by): VulnerabilityWarning(callTrace,
                                                                                                 srcTrace,
                                                                                                 warningMsg,
                                                                                                 targetInstr,
                                                                                                 found_by) {
        }

        virtual void printCompleteWarning(llvm::raw_ostream& O) const;

        //virtual void printWarning(llvm::raw_ostream& O) const;

    };
}
#endif //PROJECT_TAINTEDSIZEWARNING_H
