//
// Created by machiry on 12/4/16.
//

#ifndef PROJECT_KERNELFUNCTIONCHECKER_H
#define PROJECT_KERNELFUNCTIONCHECKER_H

#include <set>
#include <FunctionChecker.h>

namespace DRCHECKER {

    /***
     * An implementation of the function checker specific to linux kernel.
     */
    class KernelFunctionChecker: public FunctionChecker{
    public:
        static const std::set<std::string> known_allocators;
        static const std::set<std::string> init_section_names;
        static const std::set<std::string> memcpy_function_names;
        static const std::set<std::string> memset_function_names;
        static const std::set<std::string> zero_initializers;
        static const std::set<std::string> copy_out_function_names;
        static const std::set<std::string> atoiLikeFunctions;

        virtual bool is_function_allocator(const Function *targetFunction);

        virtual bool is_init_function(const Function *targetFunction);

        virtual bool is_kmalloc_function(const Function *targetFunction);

        virtual bool is_memset_function(const Function *targetFunction);

        virtual bool is_debug_function(const Function *targetFunction);

        virtual bool is_custom_function(const Function *targetFunction);

        virtual bool is_memcpy_function(const Function *targetFunction);

        virtual std::vector<long> get_memcpy_arguments(const Function *targetFunction);

        virtual bool is_taint_initiator(const Function *targetFunction);

        virtual bool is_copy_out_function(const Function *targetFunction);

        virtual std::set<long> get_tainted_arguments(const Function *targetFunction);

        virtual bool is_atoi_function(const Function *targetFunction);
        virtual bool is_sscanf_function(const Function *targetFunction);
    };
}

#endif //PROJECT_KERNELFUNCTIONCHECKER_H
