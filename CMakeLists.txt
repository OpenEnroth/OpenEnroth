cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# Use upper-case <PACKAGENAME>_ROOT variables in find_package().
cmake_policy(SET CMP0144 NEW)

# Use /Z7 instead of /Zi for MSVC debug information, this puts debug info into .obj files. This is needed for ccache
# to work. Note that for the binaries we're still getting debug info in .pdb files.
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

project("OpenEnroth")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
include(Git)
include(Detection)
include(ExternalProject)
include(CppLint)
include(LlsLint)
include(Dependencies)
include(CheckLinkerFlag)
include(Assertions)

# Defaults.
if (CMAKE_BUILD_TYPE)
    set(OE_USE_PREBUILT_DEPENDENCIES_DEFAULT ON)
else()
    set(OE_USE_PREBUILT_DEPENDENCIES_DEFAULT OFF)
endif()

# User-settable options.
set(OE_BUILD_TESTS ON CACHE BOOL "Build OpenEnroth tests.")
set(OE_BUILD_TOOLS ON CACHE BOOL "Build OpenEnroth tools - LodTool and CodeGen.")
set(OE_CHECK_STYLE ON CACHE BOOL "Enable style checks.")
set(OE_CHECK_LUA_STYLE ON CACHE BOOL "Enable lua style checks.")
set(OE_USE_PREBUILT_DEPENDENCIES ${OE_USE_PREBUILT_DEPENDENCIES_DEFAULT} CACHE BOOL "Use prebuilt dependencies.")
set(OE_USE_DUMMY_DEPENDENCIES OFF CACHE BOOL "Use dummy dependencies. Build will fail if this is set to ON, only style checks will work.")
set(OE_USE_CCACHE ON CACHE BOOL "Use ccache if available.")
set(OE_USE_SCCACHE ON CACHE BOOL "Use sccache if available, note that ccache takes precedence.")
set(OE_USE_LD_MOLD ON CACHE BOOL "Use mold linker if available.")
set(OE_USE_LD_LLD ON CACHE BOOL "Use lld linker if available, note that mold takes precedence.")

if(OE_USE_PREBUILT_DEPENDENCIES AND OE_USE_DUMMY_DEPENDENCIES)
    message(FATAL_ERROR "Only one of OE_USE_PREBUILT_DEPENDENCIES and OE_USE_DUMMY_DEPENDENCIES must be set.")
endif()

# Find ccache / sccache.
if(OE_USE_CCACHE)
    message(STATUS "Find ccache.")
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        # Check for if we detected a shim
        file(SIZE ${CCACHE_FOUND} FILE_SIZE_IN_BYTES)
        if(FILE_SIZE_IN_BYTES GREATER 500000)
            # MSVC requires ccache to be setup this way https://github.com/ccache/ccache/wiki/MS-Visual-Studio
            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

                message(STATUS "Using ccache at ${CCACHE_FOUND}.")
                set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
                set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})

                file(COPY_FILE
                    ${CCACHE_FOUND} ${CMAKE_BINARY_DIR}/cl.exe
                    ONLY_IF_DIFFERENT)

                # By default Visual Studio generators will use /Zi which is not compatible
                # with ccache, so tell Visual Studio to use /Z7 instead.
                message(STATUS "Setting MSVC debug information format to 'Embedded'")
                set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

                set(CMAKE_VS_GLOBALS
                    "CLToolExe=cl.exe"
                    "CLToolPath=${CMAKE_BINARY_DIR}"
                    "UseMultiToolTask=true"
                    "DebugInformationFormat=OldStyle"
                )

            else()
                message(STATUS "Using ccache at ${CCACHE_FOUND}.")
                set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
                set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
            endif()
        else()
            message(WARNING "ccache shim detected if you are using chocolatey you need to copy the exe from 'C:\\ProgramData\\chocolatey\\lib\\ccache\\tools' to 'C:\\ProgramData\\chocolatey\\bin'.")
        endif()
    else()
        message(STATUS "ccache is not found.")
    endif()
    message(STATUS "Not configured to use ccache.")
endif()

if(OE_USE_SCCACHE AND NOT CCACHE_FOUND)
    find_program(SCCACHE_FOUND sccache)
    if(SCCACHE_FOUND)
        message(STATUS "Using sccache at ${SCCACHE_FOUND}.")
        set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    else()
        message(STATUS "Neither sccache or ccache is found.")
    endif()
endif()

# Set up warnings-as-errors for all compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_options(-Werror=return-type) # Control reaches the end of non-void function. This is an error on MSVC.
    add_compile_options(-Werror=unused-result) # Ignoring return value of function declared with 'nodiscard' attribute.
    add_compile_options(-Werror=unused-value) # Expression result unused.
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=mismatched-tags>) # Class 'X' was previously declared as a struct.
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=suggest-override>) # 'func' overrides a member function but is not marked 'override'.
endif ()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Werror=return-local-addr) # Returning reference to temporary.
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_options(-Werror=return-stack-address) # Returning reference to temporary.
    add_compile_options(-Werror=unused-comparison) # Comparison result unused.
    add_compile_options(-Werror=inconsistent-missing-override) # 'override' only used on some of the overridden functions.
    add_compile_options(-Werror=invalid-noreturn) # function declared 'noreturn' should not return.
    add_compile_options(-Werror=uninitialized) # variable 'x' is used uninitialized.
    add_compile_options(-Werror=delete-abstract-non-virtual-dtor) # delete called on 'x' that is abstract but has non-virtual destructor.
    add_compile_options(-Werror=switch) # enumeration value not handled in switch.
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/we4834) # Discarding return value of function with 'nodiscard' attribute.
    add_compile_options(/we4172) # Returning address of local variable or temporary.
    add_compile_options(/we26433) # Function should be marked with 'override'.
endif()

# Set compiler options. We do a lot of tweaks here.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    # -Og is a better choice than -O0 for producing debuggable code because some compiler passes that collect debug information are disabled at -O0.
    string(REPLACE " -O0 " "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE " -O0 " "" CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE " -g " " -g -Og " CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE " -g " " -g -Og " CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # Detect available linkers.
    check_linker_flag(CXX "-fuse-ld=mold" COMPILER_RT_HAS_FUSE_LD_MOLD_FLAG)
    check_linker_flag(CXX "-fuse-ld=lld" COMPILER_RT_HAS_FUSE_LD_LLD_FLAG)

    if(OE_USE_LD_MOLD AND COMPILER_RT_HAS_FUSE_LD_MOLD_FLAG)
        message(STATUS "Using mold linker.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    elseif(OE_USE_LD_LLD AND COMPILER_RT_HAS_FUSE_LD_LLD_FLAG)
        message(STATUS "Using lld linker.")
        # "-Wl,-z,notext" is needed because on x86 linux prebuilt ffmpeg is built with --enable-asm, and asm code there
        # isn't position-independent. Thus, when linking a position-independent binary, LLD has two options:
        # 1. Choke.
        # 2. Generate a bunch of relocations in the .text segment.
        # Default is #1, so we need to instruct it not to choke. Note that LLD behaves differently from GNU LD here,
        # which picks #2 by default.
        if (OE_BUILD_PLATFORM STREQUAL "linux")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -Wl,-z,notext")
        endif ()
    elseif(OE_BUILD_PLATFORM STREQUAL "linux")
        message(FATAL_ERROR "Supported linkers are lld and mold, other linkers won't work so we won't even try.")
    else()
        message(STATUS "Using default linker.")
    endif()

    if(OE_BUILD_ARCHITECTURE STREQUAL "x86")
        add_compile_options(-msse2 -mfpmath=sse -march=pentium4) # Don't use x87 fpu in 32-bit builds.
    endif()

    # Don't set errno for math functions. This is the default on Darwin, so makes sense to pass this flag everywhere.
    add_compile_options(-fno-math-errno)

    # We don't use fp traps, so compiler can assume that they are disabled.
    add_compile_options(-fno-trapping-math)

    # Clang & gcc implementations of fp-contract differ, and we are getting diffs in tests. Better to disable it.
    add_compile_options(-ffp-contract=off)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_definitions($<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>) # Enable assertions in libstdc++.
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_definitions($<$<CONFIG:Debug>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG>) # Enable assertions in libcxx.
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    add_compile_definitions(NOMINMAX) # Please don't pull in these macros from <Windows.h>
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # STL security warnings are just noise
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE) # POSIX deprecation warnings are also just noise
    add_compile_definitions(_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS) # These are also useless
    add_compile_definitions(_USE_MATH_DEFINES) # Pull in M_PI and other <cmath> defines
    add_compile_options(/MP) # Multi-threaded build
    add_compile_options(/Zc:preprocessor) # Use standard compliant preprocessor
    add_link_options(/OPT:REF) # Eliminate unreferenced data & functions.
    add_link_options(/OPT:ICF) # Enable COMDAT folding, should decrease binary sizes?
    if (OE_BUILD_ARCHITECTURE STREQUAL "x86")
        add_link_options(/LARGEADDRESSAWARE) # Enable heap size over 2gb for x86 builds
        add_link_options(/SAFESEH:NO) # /SAFESEH is x86-only, don't produce safe exception handler tables
    endif()

    # Allow the compiler to package individual functions in the form of packaged functions (COMDATs)
    add_compile_options("$<$<CONFIG:RELEASE>:/Gy>")

    # Retains almost all relevant debug information for debugging, but makes a very big difference in performance and is debug safe
    # /Ob controls inline expansions and /Ob2 expands any function not explicitly marked for no inlining.
    string(REPLACE "/Ob0" "/Ob2" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Ob0" "/Ob2" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # Prebuilt deps are built with MT/MTd.
    if (OE_USE_PREBUILT_DEPENDENCIES)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# We're getting linker errors on Android if we don't pass -Bsymbolic.
# See https://github.com/google/ExoPlayer/issues/9933#issuecomment-1029775358.
if(OE_BUILD_PLATFORM STREQUAL "android")
    add_link_options("-Wl,-Bsymbolic")
endif()

init_check_style()
init_check_lua_style()
resolve_dependencies()

add_subdirectory(thirdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
if(OE_BUILD_TESTS)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_subdirectory(test)
endif()
add_subdirectory(src)
add_subdirectory(resources)
