//
// Created by machiry on 12/4/16.
//

#ifndef PROJECT_ALIASFUNCHANDLERCALLBACK_H
#define PROJECT_ALIASFUNCHANDLERCALLBACK_H

#include <FunctionChecker.h>
#include "ModuleState.h"

using namespace llvm;

namespace DRCHECKER {
    /***
     * FunctionHandlerCallback for AliasAnalysis
     * The call-back that implements the operations that need to be done on the
     * occurrence of various functions.
     */
    class AliasFuncHandlerCallback : public FunctionHandlerCallback {

    public:
        GlobalState *currState;
        virtual void* handleAllocationFunction(CallInst &callInst, Function *targetFunction, void *private_data);
        virtual void* handleCustomFunction(CallInst &callInst, Function *targetFunction, void *private_data);
        virtual void setPrivateData(void *data);

    private:
        void* createNewHeapObject(CallInst &callInst, Function *targetFunction, void *private_data);

    };

}

#endif //PROJECT_ALIASFUNCHANDLERCALLBACK_H
