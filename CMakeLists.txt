cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Setting CMAKE_OSX_DEPLOYMENT_TARGET should go before the project() call.
if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set (CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum OS X deployment version" FORCE)
    else()
        set (CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    endif()
endif()

project("OpenEnroth")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
include(Git)
include(Detection)
include(AppleLibcxxAssertions)
include(ExternalProject)
include(CppLint)
include(Dependencies)

set(CCACHE_SUPPORT On CACHE BOOL "Enable ccache support")
set(SCCACHE_SUPPORT ON CACHE BOOL "Enable sccache support")

if(CCACHE_SUPPORT)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message(STATUS "Using ccache at ${CCACHE_FOUND}")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
    elseif(NOT SCCACHE_SUPPORT)
        message(STATUS "ccache is not found.")
    endif()
endif()

if(SCCACHE_SUPPORT AND NOT CCACHE_FOUND)
    find_program(SCCACHE_FOUND sccache)
    if(SCCACHE_FOUND)
        message(STATUS "Using sccache at ${SCCACHE_FOUND}")
        set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    else()
        message(STATUS "Neither sccache or ccache is found.")
    endif()
endif()

# Settings defaults.
if(BUILD_PLATFORM STREQUAL "windows")
    set(PREBUILT_DEPENDENCIES_DEFAULT ON)
else()
    set(PREBUILT_DEPENDENCIES_DEFAULT OFF)
endif()


# Configurable settings.
set(ENABLE_TESTS OFF CACHE BOOL "Enable tests")
set(PREBUILT_DEPENDENCIES ${PREBUILT_DEPENDENCIES_DEFAULT} CACHE BOOL "Use prebuilt dependencies")

# Compiler options.
add_compile_definitions(FMT_USE_NONTYPE_TEMPLATE_ARGS) # Enable _cf literals
add_compile_definitions(PACKAGE_BUGREPORT="https://github.com/OpenEnroth/OpenEnroth/issues")

if(BUILD_COMPILER STREQUAL "gcc" OR BUILD_COMPILER STREQUAL "clang")
    add_compile_options(-Werror=return-type) # Control reaches the end of non-void function, this is an error on MSVC
    add_compile_options(-Werror=unused-result) # Ignoring return value of function declared with 'nodiscard' attribute
    add_compile_options(-Werror=unused-value) # Expression result unused
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=suggest-override>) # 'func' overrides a member function but is not marked 'override'

    if("-m32" IN_LIST CMAKE_CXX_FLAGS)
        add_compile_options(-msse2 -mfpmath=sse -march=pentium4)
    endif()
endif()

include(CheckLinkerFlag)
check_linker_flag(CXX "-fuse-ld=gold" COMPILER_RT_HAS_FUSE_LD_GOLD_FLAG)
check_linker_flag(CXX "-fuse-ld=lld" COMPILER_RT_HAS_FUSE_LD_LLD_FLAG)

add_compile_options(USE_LD_GOLD "Use GNU gold linker" ON)
add_compile_options(USE_LD_LLD "Use LLD linker" ON)

if(BUILD_COMPILER STREQUAL "gcc")
    add_compile_definitions($<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>) # Enable assertions in libstdc++.
    if(USE_LD_LLD AND COMPILER_RT_HAS_FUSE_LD_LLD_FLAG)
        message(STATUS "Using lld linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
    elseif(NOT USE_LD_GOLD AND COMPILER_RT_HAS_FUSE_LD_GOLD_FLAG)
        message(STATUS "Using gold linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    else()
        message(STATUS "Fallback to ld linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=ld")
    endif()
elseif(BUILD_COMPILER STREQUAL "clang")
    add_compile_options(-Werror=unused-comparison) # Comparison result unused, not available in gcc.
    add_compile_options(-Werror=inconsistent-missing-override) # Clang-specific, goes with -Werror=suggest-override.
    add_compile_options(-Werror=invalid-noreturn) # function declared 'noreturn' should not return.
    add_compile_options(-Werror=uninitialized) # variable 'x' is used uninitialized.
    enable_libcxx_assertions(FALSE)
    if(USE_LD_LLD AND COMPILER_RT_HAS_FUSE_LD_LLD_FLAG)
        message(STATUS "Using lld linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
    elseif(NOT USE_LD_GOLD AND COMPILER_RT_HAS_FUSE_LD_GOLD_FLAG)
        message(STATUS "Using gold linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    else()
        message(STATUS "Fallback to ld linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=ld")
    endif()
elseif(BUILD_COMPILER STREQUAL "msvc")
    # /Zi -> /Z7: Pack debug info into .obj files, this is needed for ccache to work. Note that pdb for the binary
    #             itself is still generated.
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    add_compile_definitions(NOMINMAX) # please don't pull in these macros from <Windows.h>
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # STL security warnings are just noise
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE) # POSIX deprecation warnings are also just noise
    add_compile_definitions(_USE_MATH_DEFINES) # Pull in M_PI and other <cmath> defines
    add_compile_definitions(FMT_CONSTEVAL=) # MSVC chokes on fmt consteval formatting, so we define FMT_CONSTEVAL=<empty>
    add_compile_options(/MP) # Multi-threaded build
    add_compile_options(/Zc:preprocessor) # Use standard compliant preprocessor
    add_compile_options(/we4834) # Discarding return value of function with 'nodiscard' attribute
    add_link_options(/LARGEADDRESSAWARE) # Enable heap size over 2gb for x86 builds
    add_link_options(/SAFESEH:NO) # This is for x86 build
endif()

resolve_dependencies()

add_subdirectory(thirdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/test)
add_subdirectory(test)
add_subdirectory(src)

