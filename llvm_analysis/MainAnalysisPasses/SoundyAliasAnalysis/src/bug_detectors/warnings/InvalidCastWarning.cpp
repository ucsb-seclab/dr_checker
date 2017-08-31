//
// Created by machiry on 1/8/17.
//

#include <InstructionUtils.h>
#include "bug_detectors/warnings/InvalidCastWarning.h"

using namespace llvm;

namespace DRCHECKER {

    void InvalidCastWarning::printCompleteWarning(llvm::raw_ostream &O) const {
        O << "Trying to cast an object:";
        if(this->srcObjPtr != nullptr) {
            this->srcObjPtr->print(O);
        } else {
            O << " UNKNOWN OBJECT.";
        }
        O << " of size:";
        if(this->srcObjectSize != -1) {
            O << this->srcObjectSize;
        } else {
            O << " Unknown";
            if(this->srcSizeTainted) {
                O << " (Also TAINTED)";
            }

        }
        O << " to object of size:" << this->dstObjectSize;
        O << " :" << this->found_by << "\n";
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

    void InvalidCastWarning::printWarning(llvm::raw_ostream &O) const {
        /*O << "Trying to cast an object:";
        if(this->srcObjPtr != nullptr) {
            this->srcObjPtr->print(O);
        } else {
            O << " UNKNOWN OBJECT.";
        }
        O << " of size:";
        if(this->srcObjectSize != -1) {
            O << this->srcObjectSize;
        } else {
            O << " Unknown";
            if(this->srcSizeTainted) {
                O << " (Also TAINTED)";
            }

        }
        O << " to object of size:" << this->dstObjectSize;
        O << " :" << this->found_by << "\n";
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


        O << "\"warn_data\":{";
        O << "\"by\":\"";
        O << InstructionUtils::escapeJsonString(this->found_by);
        O << "\",";
        O << "\"warn_str\":\"";
        O << InstructionUtils::escapeJsonString("Trying to cast an object");
        O << "\",";
        O << "\"src_obj\":\"";
        if(this->srcObjPtr != nullptr) {
            //this->srcObjPtr->print(O);
            O << InstructionUtils::escapeValueString(this->srcObjPtr);
        } else {
            //O << " UNKNOWN OBJECT.";
            O << InstructionUtils::escapeJsonString("UNKNOWN OBJECT");
        }
        O << "\",\"src_obj_size\":";
        if(this->srcObjectSize != -1) {
            O << this->srcObjectSize;
        } else {
            O << -1;
            if(this->srcSizeTainted) {
                O << ", \"src_obj_taint\":\"tainted\"";
            }

        }
        O << ",\"dst_obj_size\":";
        O << this->dstObjectSize << ",";
        //O << "Potential vulnerability detected by:" << this->found_by << "\n";
        //O << " " << this->warning_string << "\n";
        //O << "  at:";
        //this->target_instr->print(O);
        O << "\"at\":\"";
        O << InstructionUtils::escapeValueString(this->target_instr) << "\",";
        O << "\"at_line\":";
        DILocation *instrLoc = nullptr;
        //instrLoc = this->target_instr->getDebugLoc().get();
        instrLoc = InstructionUtils::getCorrectInstrLocation(this->target_instr);
        if(instrLoc != nullptr) {
            //O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
            O << instrLoc->getLine() << ",\"at_file\":\"" << InstructionUtils::escapeJsonString(instrLoc->getFilename()) << "\",";

        } else {
            //O << ", No line";
            O << "-1,";
        }
        O << "\"at_func\":\"" << InstructionUtils::escapeJsonString(this->target_instr->getFunction()->getName()) << "\",";
        //O << ", Func:" << this->target_instr->getFunction()->getName();
        //O << "\n";
        O << "\"inst_trace\":[";
        //O << "  Instruction Trace:";
        bool hasComma = false;
        for(Instruction *currInstruction:this->trace) {
            //O << "   ";
            if(hasComma) {
                O << ",";
            }
            O << "{\"instr\":\"";
            //currInstruction->print(O);
            O << InstructionUtils::escapeValueString(currInstruction) << "\",";
            O << "\"instr_loc\":";
            //instrLoc = currInstruction->getDebugLoc().get();
            instrLoc = InstructionUtils::getCorrectInstrLocation(currInstruction);
            if(instrLoc != nullptr) {
                //O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
                O << instrLoc->getLine() << ",\"instr_file\":\"" << InstructionUtils::escapeJsonString(instrLoc->getFilename()) << "\",";
            } else {
                //O << ", No line";
                O << "-1,";
            }
            //O << ", Func:" << currInstruction->getFunction()->getName();
            //O << "\n";
            O << "\"instr_func\":\"" << InstructionUtils::escapeJsonString(currInstruction->getFunction()->getName()) << "\"";
            O << "}";
            hasComma = true;
        }
        O <<"]";
        O << "}\n";
    }
}
