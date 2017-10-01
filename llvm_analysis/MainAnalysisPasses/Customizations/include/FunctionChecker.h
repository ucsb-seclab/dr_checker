//
// Created by machiry on 12/3/16.
//

#ifndef PROJECT_FUNCTIONCHECKER_H
#define PROJECT_FUNCTIONCHECKER_H

#include <set>
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace DRCHECKER {

    /***
     * Class that checks if the function satisfies one of the constraint.
     *
     */
    class FunctionChecker {
    public:

        /***
         * Check if the function is an allocator function.
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_function_allocator(const Function *targetFunction)=0;


        /***
         * Is this function a driver initializer?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_init_function(const Function *targetFunction)=0;

        /***
         *  Is this function a kmalloc or malloc function
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_kmalloc_function(const Function *targetFunction)=0;

        /***
         *  Is this function a memset function?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_memset_function(const Function *targetFunction)=0;

        /***
         * Is the function a debug function?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_debug_function(const Function *targetFunction)=0;

        /***
         * Is the function a custom function?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_custom_function(const Function *targetFunction)=0;

        /***
         * Is this function initiates taint or takes input from user?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_taint_initiator(const Function *targetFunction) {
            return false;
        }

        /***
         * Is this function copies content to user? or copy out function?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_copy_out_function(const Function *targetFunction) {
            return false;
        }

        /***
         * Is this function a memcpy function?
         * @param targetFunction Function to be checked?
         * @return true/false
         */
        virtual bool is_memcpy_function(const Function *targetFunction) {
           return false;
        }

        /***
         *  Is this function atoi like function?
         *
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_atoi_function(const Function *targetFunction) {
            return false;
        }

        /***
         * Is this function sscanf?
         * @param targetFunction Function to be checked.
         * @return true/false
         */
        virtual bool is_sscanf_function(const Function *targetFunction) {
           return false;
        }

        /***
         *  Get the argument index of source and destination operands of the memcpy function.
         *  srcOperand -> 0
         *  dstOperand -> 1
         * @param targetFunction memcpy function.
         * @return vector containing indexes of source and destination operands.
         */
        virtual std::vector<long> get_memcpy_arguments(const Function *targetFunction) {
            std::vector<long> defaultArgumentsOrder;
            // make sure that the function is taint initiator
            assert(this->is_memcpy_function(targetFunction));
            return defaultArgumentsOrder;
        }


        /***
         * get the indexes of tainted arguments of the provided function (which should be a taint initiator)
         * @param targetFunction Function whose tainted arguments index need to be fetched.
         * @return vector containing indexes of tainted arguments.
         */
        virtual std::set<long> get_tainted_arguments(const Function *targetFunction) {
            std::set<long> defaultTaintArguments;
            // make sure that the function is taint initiator
            assert(this->is_taint_initiator(targetFunction));
            return defaultTaintArguments;
        }


    };

    /***
     * Class the is used as call back to handle a set of functions.
     */
    class FunctionHandlerCallback {
    public:
        // target handler for checking.
        FunctionChecker* targetChecker;
        /***
         * Handle allocation function.
         * @param callInst target call instruction.
         * @param targetFunction Function that is a target of the call instruction.
         * @param private_data opaque data that could be  used by the actual implementation to maintain state.
         * @return opaque pointer whose interpretation depends on the implementation.
         */
        virtual void* handleAllocationFunction(CallInst &callInst, Function *targetFunction, void *private_data) = 0;
        /***
         * Handle custom function.
         * @param callInst target call instruction.
         * @param targetFunction Function that is a target of the call instruction.
         * @param private_data opaque data that could be  used by the actual implementation to maintain state.
         * @return opaque pointer whose interpretation depends on the implementation.
         */
        virtual void* handleCustomFunction(CallInst &callInst, Function *targetFunction, void *private_data) = 0;

        /***
         * Set private data for this handler call back.
         * This data could be used by the handler to perform some custom functions.
         * @param data pointer to the data that should be used by the handler call back.
         * @return void.
         */
        virtual void setPrivateData(void *data) = 0;

    };

    /***
     * Class that handles functions whose definition is missing.
     */
    class FunctionHandler {
    public:
        FunctionChecker *targetChecker;

        FunctionHandler(FunctionChecker *checker) {
            this->targetChecker = checker;
        }

        /***
         * Handle the function, this function has only declaration and the definition is missing.
         * @param callInst target call instruction.
         * @param targetFunction Function that is a target of the call instruction.
         * @param private_data data that needs to be passed to the callback.
         * @param callback FunctionHandlerCallback which need to be called, which the targetrFunction
         *                  matches certain criteria.
         * @param is_handled a value written by the function, which indicates that the function is
         *                   handled by this handler.
         * @return opaque pointer returned by the callback.
         */
        void* handleFunction(CallInst &callInst, Function *targetFunction, void *private_data,
                                    FunctionHandlerCallback *callback, bool &is_handled);
    };
}

#endif //PROJECT_FUNCTIONCHECKER_H
