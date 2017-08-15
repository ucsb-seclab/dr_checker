//
// Created by machiry on 8/23/16.
//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "../../Utils/include/InstructionUtils.h"
#include <vector>

#ifndef PROJECT_TAINTFLAG_H
#define PROJECT_TAINTFLAG_H
using namespace llvm;
namespace DRCHECKER {
    //Class that holds the taint flag
    class TaintFlag {

    public:
        //Constructors
        TaintFlag(Value *targetInstr, bool is_tainted) {
            assert(targetInstr != nullptr && "Target Instruction cannot be NULL");
            this->targetInstr = targetInstr;
            this->is_tainted = is_tainted;
        }

        TaintFlag(TaintFlag *copyTaint, Value *targetInstr, Value *srcOperand) {
            this->targetInstr = targetInstr;
            this->is_tainted = copyTaint->isTainted();
            // copy the instruction trace from the source taint guy
            this->instructionTrace.insert(instructionTrace.begin(),
                                          copyTaint->instructionTrace.begin(), copyTaint->instructionTrace.end());
            Instruction *lastInstr = this->instructionTrace.back();

            // add the source instruction into the trace.
            Instruction *srcInstr = dyn_cast<Instruction>(srcOperand);
            if(srcInstr != nullptr && lastInstr != srcInstr) {
                this->instructionTrace.push_back(srcInstr);
            }
        }
        TaintFlag(Value * targetInstr) : TaintFlag(targetInstr, false) {}
        //Destructors
        ~TaintFlag() {}

        bool isTainted() const {
            return is_tainted;
        }

        bool isTaintEquals(const TaintFlag *dstTaint) const {
            if(dstTaint != nullptr) {
                // we do not care about instruction trace here.
                return this->targetInstr == dstTaint->targetInstr &&
                        this->isTainted() == dstTaint->isTainted() &&
                        this->instructionTrace == dstTaint->instructionTrace;
            }
            return false;
        }

        void addInstructionToTrace(Instruction *toAdd) {
            if(this->instructionTrace.size() > 0) {
                // check if last instruction is the current instruction.
                Instruction *lastInstr = this->instructionTrace.back();
                if (toAdd == lastInstr) {
                    return;
                }
            }
            this->instructionTrace.push_back(toAdd);
        }

        Value *targetInstr;
        // trace of instructions that resulted in this taint.
        std::vector<Instruction *> instructionTrace;
        void dumpInfo(raw_ostream &OS) {

            OS << "Taint Flag for:";
            this->targetInstr->print(OS);
            OS << ", Tainted=" << this->isTainted() << "\n";
            OS << " Instruction Trace: [";
            for(std::vector<Instruction *>::iterator SI = this->instructionTrace.begin(); SI != this->instructionTrace.end(); ++SI) {
                AAMDNodes targetMDNodes;
                (*SI)->getAAMetadata(targetMDNodes, true);
                OS << (InstructionUtils::getInstructionName((*SI))) << ": at line " << InstructionUtils::getLineNumber(**SI) << " ,";
            }
            OS << "]\n";

        }
    private:
        // flag to indicate the taint flag.
        bool is_tainted;

    };

    /***
     * Class that represents taint of an object field.
     */
    class FieldTaint {
    public:
        long fieldId;
        std::set<TaintFlag *> targetTaint;

        FieldTaint(long srcField) {
            this->fieldId = srcField;
        }

        ~FieldTaint() {
            //TODO: implement this in a smart way.
            /*for(auto currT:targetTaint) {
                delete(currT);
            }*/
        }

        bool addTaintFlag(TaintFlag *toAdd) {
            // check if the set already contains same taint?
            if(std::find_if(targetTaint.begin(), targetTaint.end(), [toAdd](const TaintFlag *n) {
                return  n->isTaintEquals(toAdd);
            }) == targetTaint.end()) {
                // if not insert the new taint flag into the newTaintInfo.
                targetTaint.insert(targetTaint.end(), toAdd);
                return true;
            }
            return false;
        }

    };

}

#endif //PROJECT_TAINTFLAG_H
