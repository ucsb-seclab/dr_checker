//
// Created by machiry on 12/28/16.
//

#include "bug_detectors/BugDetectorDriver.h"
#include "bug_detectors/TaintedPointerDereference.h"
#include "bug_detectors/KernelUninitMemoryLeakDetector.h"
#include "bug_detectors/TaintedSizeDetector.h"
#include "bug_detectors/InvalidCastDetector.h"
#include "bug_detectors/IntegerOverflowDetector.h"
#include "bug_detectors/ImproperTaintedDataUseDetector.h"
#include "bug_detectors/TaintedLoopBoundDetector.h"
#include "bug_detectors/GlobalVariableRaceDetector.h"

using namespace llvm;

namespace DRCHECKER {

//#define DISABLE_INVALIDCASTDETECTOR
//#define DISABLE_INTEGEROVERFLOWDETECTOR
//#define DISABLE_TAINTEDPOINTERDEREFERENCE
//#define DISABLE_KERNELUNITMEMORYLEAKDETECTOR
//#define DISABLE_TAINEDSIZEDETECTOR
//#define DISABLE_IMPROPERTAINTEDDATAUSEDETECTOR
//#define DISABLE_TAINTEDLOOPBOUNDDETECTOR
//#define DISABLE_GLOBALVARIABLERACEDETECTOR

    void BugDetectorDriver::addPreAnalysisBugDetectors(GlobalState &targetState,
                                                       Function *toAnalyze,
                                                       std::vector<Instruction *> *srcCallSites,
                                                       std::vector<VisitorCallback *> *allCallbacks,
                                                       FunctionChecker *targetChecker) {

#ifndef DISABLE_INVALIDCASTDETECTOR
        VisitorCallback *invalidCastDetector = new InvalidCastDetector(targetState,
                                                                       toAnalyze,
                                                                       srcCallSites, targetChecker);
        allCallbacks->push_back(invalidCastDetector);
#endif
#ifndef DISABLE_INTEGEROVERFLOWDETECTOR

        VisitorCallback *integerOverflowDetector = new IntegerOverflowDetector(targetState,
                                                                           toAnalyze,
                                                                           srcCallSites, targetChecker);
        allCallbacks->push_back(integerOverflowDetector);
#endif

    }

    void BugDetectorDriver::addPostAnalysisBugDetectors(GlobalState &targetState,
                                                        Function *toAnalyze,
                                                        std::vector<Instruction *> *srcCallSites,
                                                        std::vector<VisitorCallback *> *allCallbacks,
                                                        FunctionChecker *targetChecker) {

#ifndef DISABLE_TAINTEDPOINTERDEREFERENCE
        VisitorCallback *currTaintDetector = new TaintedPointerDereference(targetState,
                                                                           toAnalyze,
                                                                           srcCallSites, targetChecker);
        allCallbacks->push_back(currTaintDetector);
#endif
#ifndef DISABLE_KERNELUNITMEMORYLEAKDETECTOR

        VisitorCallback *currLeakDetector = new KernelUninitMemoryLeakDetector(targetState,
                                                                               toAnalyze,
                                                                               srcCallSites, targetChecker);
        allCallbacks->push_back(currLeakDetector);
#endif
#ifndef DISABLE_TAINEDSIZEDETECTOR
        VisitorCallback *currTaintSizeDetector = new TaintedSizeDetector(targetState,
                                                                         toAnalyze,
                                                                         srcCallSites, targetChecker);
        allCallbacks->push_back(currTaintSizeDetector);
#endif

#ifndef DISABLE_IMPROPERTAINTEDDATAUSEDETECTOR
        VisitorCallback *currImproperDataUseDetector = new ImproperTaintedDataUseDetector(targetState,
                                                                                          toAnalyze,
                                                                                          srcCallSites, targetChecker);
        allCallbacks->push_back(currImproperDataUseDetector);
#endif

#ifndef DISABLE_TAINTEDLOOPBOUNDDETECTOR
        VisitorCallback *taintedCondDetector = new TaintedLoopBoundDetector(targetState,
                                                                            toAnalyze,
                                                                            srcCallSites, targetChecker);
        allCallbacks->push_back(taintedCondDetector);
#endif
#ifndef DISABLE_GLOBALVARIABLERACEDETECTOR

        VisitorCallback *globalVarRaceDetector = new GlobalVariableRaceDetector(targetState,
                                                                                toAnalyze,
                                                                                srcCallSites, targetChecker);
        allCallbacks->push_back(globalVarRaceDetector);
#endif

    }

    void BugDetectorDriver::printAllWarnings(GlobalState &targetState, llvm::raw_ostream& O) {
        O << "{\"num_contexts\":";
        if(targetState.allVulnWarnings.size() == 0) {
            O << "0";
            //O << "No Warnings. Everything looks good\n";
        } else {
            O << targetState.allVulnWarnings.size() << ",\n";
            bool addout_comma = false;
            O << "\"all_contexts\":[\n";
            for (auto warn_iter = targetState.allVulnWarnings.begin(); warn_iter != targetState.allVulnWarnings.end();
                 warn_iter++) {
                bool addin_comma = false;
                if(addout_comma) {
                    O << ",\n";
                }
                O << "{";
                AnalysisContext *targetContext = warn_iter->first;
                std::set<VulnerabilityWarning *> *allWarnings = warn_iter->second;
                O << "\"num_warnings\":" << allWarnings->size() << ",\n";
                // O << "At Calling Context:";
                targetContext->printContext(O);
                O << ",";

                //O << "Found:" << allWarnings->size() << " warnings.\n";
                long currWarningNo = 1;
                O << "\"warnings\":[\n";
                for (VulnerabilityWarning *currWarning:*(allWarnings)) {
                    if(addin_comma) {
                        O << ",\n";
                    }
                    O << "{";
                    O << "\"warn_no\":" << currWarningNo << ",";
                    currWarning->printWarning(O);
                    currWarningNo++;
                    addin_comma = true;
                    O << "}";
                }
                O << "\n]";
                addout_comma = true;
                O << "}";
            }
            O << "]\n";
        }
        O << "\n}";
    }

    void BugDetectorDriver::printWarningsByInstr(GlobalState &targetState, llvm::raw_ostream& O) {
        O << "{\"num_instructions\":";
        if(targetState.warningsByInstr.size() == 0) {
            O << "0";
            //O << "No Warnings. Everything looks good\n";
        } else {
            O << targetState.warningsByInstr.size() << ",\n";
            bool addout_comma = false;
            O << "\"all_instrs\":[\n";
            for (auto warn_iter = targetState.warningsByInstr.begin(); warn_iter != targetState.warningsByInstr.end();
                 warn_iter++) {
                bool addin_comma = false;
                if(addout_comma) {
                    O << ",\n";
                }
                O << "{";
                Instruction *currInstr = warn_iter->first;
                std::set<VulnerabilityWarning *> *allWarnings = warn_iter->second;
                O << "\"num_warnings\":" << allWarnings->size() << ",\n";

                O << "\"at\":\"";
                O << InstructionUtils::escapeValueString(currInstr) << "\",";
                O << "\"at_line\":";
                DILocation *instrLoc = nullptr;
                //instrLoc = this->target_instr->getDebugLoc().get();
                instrLoc = InstructionUtils::getCorrectInstrLocation(currInstr);
                if(instrLoc != nullptr) {
                    //O << ", src line:" << instrLoc->getLine() << " file:" << instrLoc->getFilename();
                    O << instrLoc->getLine() << ",\"at_file\":\"" << InstructionUtils::escapeJsonString(instrLoc->getFilename()) << "\",";

                } else {
                    //O << ", No line";
                    O << "-1,";
                }
                O << "\"at_func\":\"" << InstructionUtils::escapeJsonString(currInstr->getFunction()->getName()) << "\",";

                //O << "Found:" << allWarnings->size() << " warnings.\n";
                long currWarningNo = 1;
                O << "\"warnings\":[\n";
                for (VulnerabilityWarning *currWarning:*(allWarnings)) {
                    if(addin_comma) {
                        O << ",\n";
                    }
                    O << "{";
                    O << "\"warn_no\":" << currWarningNo << ",";
                    currWarning->printWarning(O);
                    currWarningNo++;
                    addin_comma = true;
                    O << "}";
                }
                O << "\n]";
                addout_comma = true;
                O << "}";
            }
            O << "]\n";
        }
        O << "\n}";
    }
}
