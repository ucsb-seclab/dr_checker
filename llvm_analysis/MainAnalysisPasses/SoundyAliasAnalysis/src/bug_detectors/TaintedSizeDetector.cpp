//
// Created by machiry on 1/8/17.
//

#include "bug_detectors/TaintedSizeDetector.h"
#include "bug_detectors/warnings/TaintedSizeWarning.h"

using namespace llvm;

namespace DRCHECKER {

    VisitorCallback* TaintedSizeDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                        std::vector<Instruction *> *oldFuncCallSites,
                                                        std::vector<Instruction *> *currFuncCallSites) {
        if(targetFunction->isDeclaration()) {
            // warning already raised for this instruction.
            if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
                return nullptr;
            }
            FunctionChecker *currChecker = this->targetChecker;
            // ok this is a copy_to(or from)_user function?
            if(currChecker->is_copy_out_function(targetFunction) ||
                    currChecker->is_taint_initiator(targetFunction)) {
                Value *sizeArg = nullptr;
                // if this is a
                if(targetFunction->getName() == "simple_write_to_buffer") {
                    sizeArg = I.getArgOperand(1);
                } else {
                    // get the size argument.
                    sizeArg = I.getArgOperand(2);
                }
                // get the range of the size argument.
                Range sizeRange = this->currState.getRange(sizeArg);
                // check, if the Range is not constant, if not.
                // raise a warning.
                if(!sizeRange.isBounded() || sizeRange.getLower() != sizeRange.getUpper()) {
                    std::string warningMsg = "Non-constant size used in copy_to(or from)_user function.";
                    // no instruction trace.
                    std::vector<Instruction *> instructionTrace;

                    VulnerabilityWarning *currWarning = new TaintedSizeWarning(this->currFuncCallSites,
                                                                               &instructionTrace,
                                                                               warningMsg, &I,
                                                                               TAG);
                    this->currState.addVulnerabilityWarning(currWarning);
                    if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                        this->warnedInstructions.insert(&I);
                    }
                }
            }
        }

        if(!targetFunction->isDeclaration()) {
            // only if the function has source.

            TaintedSizeDetector *newVis = new TaintedSizeDetector(this->currState, targetFunction,
                                                                  currFuncCallSites, this->targetChecker);

            return newVis;
        }
        return nullptr;
    }
}
