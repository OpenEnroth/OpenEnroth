cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Note that it's possible to drop the --ls option, but it will likely eventually break on Windows with its
# limit on command line size. So it's either --ls, or passing args in a @file.
add_custom_target(Run_RetraceTest
        OpenEnroth retrace --check-canonical --ls ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.

add_custom_target(Run_RetraceTest_Headless
        OpenEnroth retrace --headless --check-canonical --ls ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_RetraceTest_Parallel
        Python::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/ParallelRetrace.py --ls ${OE_TESTDATA_PATH} $<TARGET_FILE:OpenEnroth>
        DEPENDS OpenEnroth OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_RetraceTest_Headless_Parallel
        Python::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/ParallelRetrace.py --ls ${OE_TESTDATA_PATH} --headless $<TARGET_FILE:OpenEnroth>
        DEPENDS OpenEnroth OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)
