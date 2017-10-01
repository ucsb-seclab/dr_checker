include_directories(include/ ../Utils/include ../Customizations/include ../LinuxKernelCustomizations/include
        ../RangeAnalysis/include)

file(GLOB SAA_SRCS
        "src/*.cpp"
        "src/bug_detectors/*.cpp"
        "src/bug_detectors/warnings/*.cpp"
        "include/*.h"
        "include/bug_detectors/*.h"
        "include/bug_detectors/warnings/*.h"
        )

add_library(SoundyAliasAnalysis MODULE
        # List your source files here.
        ${SAA_SRCS} src/bug_detectors/InvalidCastDetector.cpp include/bug_detectors/InvalidCastDetector.h include/bug_detectors/warnings/InvalidCastWarning.h src/bug_detectors/warnings/InvalidCastWarning.cpp include/bug_detectors/ImproperTaintedDataUseDetector.h src/bug_detectors/ImproperTaintedDataUseDetector.cpp include/bug_detectors/warnings/ImproperTaintedDataUseWarning.h src/bug_detectors/warnings/ImproperTaintedDataUseWarning.cpp include/bug_detectors/IntegerOverflowDetector.h src/bug_detectors/IntegerOverflowDetector.cpp include/bug_detectors/TaintedLoopBoundDetector.h src/bug_detectors/TaintedLoopBoundDetector.cpp include/bug_detectors/GlobalVariableRaceDetector.h src/bug_detectors/GlobalVariableRaceDetector.cpp)

add_library(SoundyAliasAnalysisStatic STATIC
        # List your source files here.
        ${SAA_SRCS} src/bug_detectors/InvalidCastDetector.cpp include/bug_detectors/InvalidCastDetector.h include/bug_detectors/warnings/InvalidCastWarning.h src/bug_detectors/warnings/InvalidCastWarning.cpp include/bug_detectors/ImproperTaintedDataUseDetector.h src/bug_detectors/ImproperTaintedDataUseDetector.cpp include/bug_detectors/warnings/ImproperTaintedDataUseWarning.h src/bug_detectors/warnings/ImproperTaintedDataUseWarning.cpp include/bug_detectors/IntegerOverflowDetector.h src/bug_detectors/IntegerOverflowDetector.cpp include/bug_detectors/TaintedLoopBoundDetector.h src/bug_detectors/TaintedLoopBoundDetector.cpp include/bug_detectors/GlobalVariableRaceDetector.h src/bug_detectors/GlobalVariableRaceDetector.cpp)


target_link_libraries(SoundyAliasAnalysis Utils Customizations LinuxKernelCustomizations RangeAnalysis)

target_link_libraries(SoundyAliasAnalysisStatic Utils Customizations LinuxKernelCustomizations RangeAnalysis)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(SoundyAliasAnalysis PRIVATE cxx_range_for cxx_auto_type)

target_compile_features(SoundyAliasAnalysisStatic PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(SoundyAliasAnalysisStatic
        PROPERTIES POSITION_INDEPENDENT_CODE ON)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(SoundyAliasAnalysis PROPERTIES
    COMPILE_FLAGS "-g -fno-rtti"
)

set_target_properties(SoundyAliasAnalysisStatic PROPERTIES
        COMPILE_FLAGS "-g -fno-rtti"
        )

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(SoundyAliasAnalysis PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
    set_target_properties(SoundyAliasAnalysisStatic PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif(APPLE)


