//
// Created by machiry on 12/4/16.
//
#include "FunctionChecker.h"

namespace DRCHECKER {

//#define DEBUG_HANDLE_FUNCTION

    void* FunctionHandler::handleFunction(CallInst &callInst, Function *targetFunction, void *private_data,
                                         FunctionHandlerCallback *callback, bool &is_handled) {

        // Ensure that we not have declaration
        assert(targetFunction->isDeclaration());

        std::string currFuncName = targetFunction->getName().str();

        if(this->targetChecker->is_debug_function(targetFunction)) {
#ifdef DEBUG_HANDLE_FUNCTION
            dbgs() << "The function:" << currFuncName << " is an LLVM Debug Instruction. Ignoring!!!" << "\n";
#endif
            is_handled = true;
            return nullptr;
        }

        // either this is an allocation function?

        if(this->targetChecker->is_function_allocator(targetFunction)) {
#ifdef DEBUG_HANDLE_FUNCTION
            dbgs() << "Processing allocation function:" << currFuncName << "\n";
#endif
            is_handled = true;
            return callback->handleAllocationFunction(callInst, targetFunction, private_data);
        }

        if(this->targetChecker->is_custom_function(targetFunction)) {
#ifdef DEBUG_HANDLE_FUNCTION
            dbgs() << "Processing custom function:" << currFuncName << "\n";
#endif
            is_handled = true;
            return callback->handleCustomFunction(callInst, targetFunction, private_data);
        }

        is_handled = false;
        return nullptr;

    }
}
