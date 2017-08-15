//
// Created by machiry on 12/4/16.
//
#include "KernelFunctionChecker.h"
using namespace llvm;

namespace DRCHECKER {

    // These are allocators
    const std::set<std::string> KernelFunctionChecker::known_allocators{"__kmalloc", "kmem_cache_alloc",
                                                                        "mempool_alloc",
                                                                        "__get_free_pages", "get_free_pages",
                                                                        "__get_free_page", "get_free_page",
                                                                        "__vmalloc", "vmalloc",
                                                                        "alloc_percpu", "__alloc_percpu",
                                                                        "alloc_bootmem"};

    // these are initializers
    const std::set<std::string> KernelFunctionChecker::zero_initializers{"__kmalloc"};
    const std::set<std::string> KernelFunctionChecker::memset_function_names{"memset"};
    // copy to user function.
    const std::set<std::string> KernelFunctionChecker::copy_out_function_names{"__copy_to_user"};
    // init functions
    const std::set<std::string> KernelFunctionChecker::init_section_names{".init.text"};
    // memcpy functions: for points to and taint propagation.
    const std::set<std::string> KernelFunctionChecker::memcpy_function_names{"llvm.memcpy", "strcpy", "strncpy",
                                                                             "strcat", "strncat", "strlcpy",
                                                                             "strlcat"};

    bool KernelFunctionChecker::is_debug_function(const Function *targetFunction) {
        if(targetFunction->hasName()) {
            std::string currFuncName = targetFunction->getName().str();
            if(currFuncName.find("llvm.dbg") != std::string::npos) {
                return true;
            }

        }
        return false;
    }

    bool KernelFunctionChecker::is_init_function(const Function *targetFunction) {
        if(!targetFunction->isDeclaration()) {
            for(const std::string &curr_sec:KernelFunctionChecker::init_section_names) {
                if(targetFunction->getSection() != nullptr && strlen(targetFunction->getSection()) &&
                   (curr_sec.find(targetFunction->getSection()) != std::string::npos)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool KernelFunctionChecker::is_copy_out_function(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            for (const std::string &curr_func:KernelFunctionChecker::copy_out_function_names) {
                if (func_name.find(curr_func.c_str()) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    bool KernelFunctionChecker::is_kmalloc_function(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            for (const std::string &curr_func:KernelFunctionChecker::zero_initializers) {
                if (func_name.find(curr_func.c_str()) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    bool KernelFunctionChecker::is_memset_function(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            for (const std::string &curr_func:KernelFunctionChecker::memset_function_names) {
                if (func_name.find(curr_func.c_str()) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }
    
    bool KernelFunctionChecker::is_function_allocator(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            for (const std::string &curr_func:KernelFunctionChecker::known_allocators) {
                if (func_name.find(curr_func.c_str()) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    bool KernelFunctionChecker::is_custom_function(const Function *targetFunction) {
        // is this a kernel function and returns a pointer?
        return targetFunction->isDeclaration() && targetFunction->getReturnType()->isPointerTy();
    }

    bool KernelFunctionChecker::is_memcpy_function(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            for (const std::string &curr_func:KernelFunctionChecker::memcpy_function_names) {
                if (func_name.find(curr_func.c_str()) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<long> KernelFunctionChecker::get_memcpy_arguments(const Function *targetFunction) {
        std::vector<long> memcpy_args;
        if(this->is_memcpy_function(targetFunction)) {
            // src argument is the second parameter
            memcpy_args.push_back(1);
            // dst argument is the first parameter
            memcpy_args.push_back(0);
            return memcpy_args;
        }
        // should never reach here..make sure that you call is_memcpy_function function
        // before this.
        assert(false);
        return memcpy_args;
    }

    bool KernelFunctionChecker::is_taint_initiator(const Function *targetFunction) {
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            return func_name == "__copy_from_user" || func_name == "simple_write_to_buffer";
        }
        return false;
    }

    std::set<long> KernelFunctionChecker::get_tainted_arguments(const Function *targetFunction) {
        std::set<long> tainted_args;
        if(targetFunction->isDeclaration() && targetFunction->hasName()) {
            std::string func_name = targetFunction->getName().str();
            if(func_name == "__copy_from_user" || func_name == "simple_write_to_buffer") {
                // first argument will get tainted.
                tainted_args.insert(tainted_args.end(), 0);
                return tainted_args;
            }

        }
        // should never reach here..make sure that you call is_taint_initiator function
        // before this.
        assert(false);
        return tainted_args;
    }
    
}

