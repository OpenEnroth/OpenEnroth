
set(DEBUG_LIBCXX_COMPILES FALSE)
set(DEBUG_LIBCXX_WORKAROUND_WORKS FALSE)
set(DEBUG_LIBCXX_SOURCE_ROOT ${CMAKE_CURRENT_LIST_DIR})

if(BUILD_PLATFORM STREQUAL "darwin")
    try_compile(
        DEBUG_LIBCXX_COMPILES
        ${CMAKE_BINARY_DIR}/apple_libcxx_assertion_test
        ${DEBUG_LIBCXX_SOURCE_ROOT}/AppleLibcxxAssertionTest.cpp
        COMPILE_DEFINITIONS -D_LIBCPP_DEBUG=0
        OUTPUT_VARIABLE DEBUG_LIBCXX_COMPILE_OUTPUT
    )
    if(NOT DEBUG_LIBCXX_COMPILES)
        string(FIND ${DEBUG_LIBCXX_COMPILE_OUTPUT} "__libcpp_debug_function" DEBUG_LIBCXX_STR_POS)
        if(NOT ${DEBUG_LIBCXX_STR_POS} EQUAL -1)
            set(DEBUG_LIBCXX_WORKAROUND_WORKS TRUE)
        else()
            message(STATUS "Haven't found a way to fix debug libcxx build, target_fix_libcxx_assertions() won't work.")
        endif()
    endif()
endif()

function(enable_libcxx_assertions ENABLE_IN_RELEASE_TOO)
    if(NOT BUILD_PLATFORM STREQUAL "darwin")
        message(WARNING "enable_libcxx_assertions() only works on mac os builds.")
        return()
    endif()

    if(ENABLE_IN_RELEASE_TOO)
        add_compile_definitions(_LIBCPP_DEBUG=0)
    else()
        add_compile_definitions($<$<CONFIG:Debug>:_LIBCPP_DEBUG=0>)
    endif()
endfunction()

function(target_fix_libcxx_assertions TARGET)
    if(NOT BUILD_PLATFORM STREQUAL "darwin" OR DEBUG_LIBCXX_COMPILES)
        return()
    endif()

    if(DEBUG_LIBCXX_WORKAROUND_WORKS)
        target_sources(${TARGET} PRIVATE ${DEBUG_LIBCXX_SOURCE_ROOT}/AppleLibcxxAssertionWorkaround.cpp)
    endif()
endfunction()
