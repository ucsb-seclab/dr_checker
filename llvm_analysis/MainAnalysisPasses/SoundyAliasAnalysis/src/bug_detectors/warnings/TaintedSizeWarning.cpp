//
// Created by machiry on 1/8/17.
//
#include "bug_detectors/warnings/TaintedSizeWarning.h"

using namespace llvm;

namespace DRCHECKER {

    void TaintedSizeWarning::printCompleteWarning(llvm::raw_ostream &O) const {
        O << "Non constant size used in copy_to(or from)_user function :" << this->found_by << "\n";
        O << "  at:";
        this->target_instr->print(O);
        DILocation *instrLoc = nullptr;
        instrLoc = this->target_instr->getDebugLoc().get();
        if(instrLoc != nullptr) {
            O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
        } else {
            O << ", No line";
        }
        O << ", Func:" << this->target_instr->getFunction()->getName();
        O << "\n";
        O << "  Call Context:";
        for(Instruction *currCallSite:this->callSiteTrace) {
            O << "   ";
            currCallSite->print(O);
            instrLoc = currCallSite->getDebugLoc().get();
            if(instrLoc != nullptr) {
                O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
            } else {
                O << ", No line";
            }
            O << "\n";
        }
    }

    /*void TaintedSizeWarning::printWarning(llvm::raw_ostream &O) const {
        //this->warning_string.clear();
        //this->warning_string.append("Non constant size used in copy_to(or from)_user function");
        //this->warning_string = "Non constant size used in copy_to(or from)_user function";

        //VulnerabilityWarning::printWarning(O);
        /*O << "Non constant size used in copy_to(or from)_user function :" << this->found_by << "\n";
        O << "  at:";
        this->target_instr->print(O);
        DILocation *instrLoc = nullptr;
        instrLoc = this->target_instr->getDebugLoc().get();
        if(instrLoc != nullptr) {
            O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
        } else {
            O << ", No line";
        }
        O << ", Func:" << this->target_instr->getFunction()->getName();
        O << "\n";*/

}
