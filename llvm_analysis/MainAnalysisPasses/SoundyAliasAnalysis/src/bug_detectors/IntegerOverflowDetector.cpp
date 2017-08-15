//
// Created by machiry on 1/31/17.
//

#include "bug_detectors/IntegerOverflowDetector.h"
#include "TaintUtils.h"

using namespace llvm;

namespace DRCHECKER {
#define BIN_OP_START 11
#define BIN_OP_END 16
#define ONLY_ONE_WARNING

    void IntegerOverflowDetector::visitBinaryOperator(BinaryOperator &I){
        unsigned long opCode = I.getOpcode();
        // warning already raised for this instruction.
        if(this->warnedInstructions.find(&I) != this->warnedInstructions.end()) {
            return;
        }
        // if the binary operation is overflow inducing?
        if(opCode >= BIN_OP_START && opCode <= BIN_OP_END) {
            std::set<TaintFlag*> resultingTaintInfo;
            resultingTaintInfo.clear();
            std::set<Value*> targetValues;
            // add both the operands into values to be checked.
            targetValues.insert(I.getOperand(0));
            targetValues.insert(I.getOperand(1));
            for(auto currVal:targetValues) {
                std::set<TaintFlag *> *srcTaintInfo = TaintUtils::getTaintInfo(this->currState,
                                                                               this->currFuncCallSites,
                                                                               currVal);
                if(srcTaintInfo != nullptr) {
                    resultingTaintInfo.insert(srcTaintInfo->begin(), srcTaintInfo->end());
                }
            }

            // raise warning for each of the tainted values.
            for(TaintFlag *currFlag:resultingTaintInfo) {
                if(currFlag->isTainted()) {
                    std::string warningMsg = "Potential overflow, using tainted value in binary operation.";
                    VulnerabilityWarning *currWarning = new VulnerabilityWarning(this->currFuncCallSites,
                                                                                 &(currFlag->instructionTrace),
                                                                                 warningMsg, &I,
                                                                                 TAG);
                    this->currState.addVulnerabilityWarning(currWarning);

                    if(this->warnedInstructions.find(&I) == this->warnedInstructions.end()) {
                        this->warnedInstructions.insert(&I);
                    }

#ifdef ONLY_ONE_WARNING
                    return;
#endif
                }
            }

        }
    }

    VisitorCallback* IntegerOverflowDetector::visitCallInst(CallInst &I, Function *targetFunction,
                                                            std::vector<Instruction *> *oldFuncCallSites,
                                                            std::vector<Instruction *> *currFuncCallSites) {
        if (!targetFunction->isDeclaration()) {
            // only if the function has source.

            IntegerOverflowDetector *newVis = new IntegerOverflowDetector(this->currState, targetFunction,
                                                                  currFuncCallSites, this->targetChecker);

            return newVis;
        }
        return nullptr;
    }
}