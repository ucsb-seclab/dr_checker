//
// Created by machiry on 8/23/16.
//

#ifndef PROJECT_INSTRUCTIONUTILS_H
#define PROJECT_INSTRUCTIONUTILS_H
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <string>
#include <sstream>

using namespace llvm;

namespace DRCHECKER {
    class InstructionUtils {
        public:
        /***
         *  Is any of the operands to the instruction is a pointer?
         * @param I  Instruction to be checked.
         * @return  true/false
         */
        static bool isPointerInstruction(Instruction *I);

        /***
         *  Get the line number of the instruction.
         * @param I instruction whose line number need to be fetched.
         * @return unsigned int representing line number.
         */
        static unsigned getLineNumber(Instruction &I);

        /***
         *  Get the name of the provided instruction.
         * @param I instruction whose name needs to be fetched.
         * @return string representing the instruction name.
         */
        static std::string getInstructionName(Instruction *I);

        /***
         * Get the name of the provided value operand.
         * @param v The value operand whose name needs to be fetched.
         * @return string representing name of the provided value.
         */
        static std::string getValueName(Value *v);

        /***
         *  Method to convert string to be json friendly.
         *  Copied from: https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
         * @param input input string
         * @return converted string.
         */
        static std::string escapeJsonString(const std::string& input);

        /***
         * Method to convert the provided value to escaped json string.
         *
         * @param currInstr Value object which needs to be converted to json string.
         * @return Converted string.
         */
        static std::string escapeValueString(Value *currInstr);

        /***
         * Get the instruction line number corresponding to the provided instruction.
         * @param I Instruction whose line number needs to be fetched.
         * @return Line number.
         */
        static int getInstrLineNumber(Instruction *I);

        /***
         * Get the correct Debug Location (handles in lineing) for the provided instruction.
         *
         * @param I instruction whose correct debug location needs to be fetched.
         * @return DILocation correct debug location corresponding to the provided instruction.
         */
        static DILocation* getCorrectInstrLocation(Instruction *I);
    };
}
#endif //PROJECT_INSTRUCTIONUTILS_H
