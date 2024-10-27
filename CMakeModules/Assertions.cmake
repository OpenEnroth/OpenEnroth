# Enable assertions in release builds.
if(MSVC)
    set(NDEBUG_FLAG "/DNDEBUG")
else()
    set(NDEBUG_FLAG "-DNDEBUG")
endif()
foreach(BUILD_TYPE RELEASE RELWITHDEBINFO MINSIZEREL)
    string(REPLACE "${NDEBUG_FLAG}" "" CMAKE_CXX_FLAGS_${BUILD_TYPE} "${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")
    string(REPLACE "${NDEBUG_FLAG}" "" CMAKE_C_FLAGS_${BUILD_TYPE} "${CMAKE_C_FLAGS_${BUILD_TYPE}}")
endforeach()

function(generate_assertion_test_file PATH EXPR)
    file(WRITE "${PATH}" "
    #include <assert.h>
    #include <stdlib.h>
    int main() {
    #ifdef _WIN32
        _set_abort_behavior(0, _WRITE_ABORT_MSG);
    #endif
        assert(${EXPR});
        return 0;
    }")
endfunction()

# We don't run tests when cross-compiling b/c it's a mess...
if(NOT CMAKE_CROSSCOMPILING)
    generate_assertion_test_file("${CMAKE_BINARY_DIR}/test_assert_false.cpp" false)
    generate_assertion_test_file("${CMAKE_BINARY_DIR}/test_assert_true.cpp" true)
    try_run(TEST_ASSERT_FALSE_RUN_RESULT TEST_ASSERT_FALSE_COMPILE_RESULT
            SOURCES "${CMAKE_BINARY_DIR}/test_assert_false.cpp")
    try_run(TEST_ASSERT_TRUE_RUN_RESULT TEST_ASSERT_TRUE_COMPILE_RESULT
            SOURCES "${CMAKE_BINARY_DIR}/test_assert_true.cpp")
    if((TEST_ASSERT_FALSE_RUN_RESULT EQUAL 0) OR (NOT TEST_ASSERT_FALSE_COMPILE_RESULT) OR (NOT TEST_ASSERT_TRUE_RUN_RESULT EQUAL 0) OR (NOT TEST_ASSERT_TRUE_COMPILE_RESULT))
        message(FATAL_ERROR "Could not enable assertions for this build.")
    endif()
endif()
