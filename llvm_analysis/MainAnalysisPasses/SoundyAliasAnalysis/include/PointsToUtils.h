//
// Created by machiry on 12/6/16.
//

#ifndef PROJECT_POINTSTOUTILS_H
#define PROJECT_POINTSTOUTILS_H

#include "AliasObject.h"
#include "ModuleState.h"

using namespace llvm;
namespace DRCHECKER {
    /***
     * Class that implements common pointtos helper functions.
     */
    class PointsToUtils {
    public:
        /***
         * Get set of points to objects that could be pointed by the provided pointer.
         *
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context of the analysis.
         * @param srcPointer pointer whose points to information needs to be fetched.
         * @return pointer to set of points to objects.
         */
        static std::set<PointerPointsTo*>* getPointsToObjects(GlobalState &currState,
                                                              std::vector<Instruction *> *currFuncCallSites,
                                                              Value *srcPointer);


        /***
         * Function which checks if the provided pointer has any points to information.
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context of the analysis.
         * @param srcPointer Pointer whose points to information need to checked.
         * @return true / false depending on whether points to information exists or not.
         */
        static bool hasPointsToObjects(GlobalState &currState,
                                       std::vector<Instruction *> *currFuncCallSites,
                                       Value *srcPointer);


        /***
         * This function returns all the functions that could be pointed by the provided pointer.
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context of the analysis.
         * @param srcPointer Pointer whose target functions needs to be fetched.
         * @param dstFunctions list of functions which are possible targets of srcPointer.
         * @return true if there is atleast one function else false.
         */
        static bool getTargetFunctions(GlobalState &currState, std::vector<Instruction *> *currFuncCallSites,
                                       Value *srcPointer, std::vector<Function *> &dstFunctions);

        /***
         * Gets all objects that could be pointed by the provided pointer.
         *
         * @param currState Global state of the analysis.
         * @param currFuncCallSites Context of the analysis.
         * @param srcPointer Pointer whose points to objects needs to be fetched.
         * @param dstObjects Reference to the vector which the function fill ups with pointer
         *                   to destination objects.
         * @return
         */
        static bool getAllAliasObjects(GlobalState &currState, std::vector<Instruction *> *currFuncCallSites,
                                       Value *srcPointer,
                                       std::set<AliasObject*> &dstObjects);

        /***
         * Get potential targets of a call instruction from its type information.
         * @param callInst Call instruction whose targets need to be fetched.
         * @param targetFunctions Set to which possible targets should be added.
         * @return true/false depending on targets is non-empty or empty.
         */
        static bool getPossibleFunctionTargets(CallInst &callInst, std::vector<Function*> &targetFunctions);



    };

}
#endif //PROJECT_POINTSTOUTILS_H
