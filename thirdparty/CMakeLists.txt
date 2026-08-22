cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BUILD_TESTING OFF CACHE BOOL "Don't build tests for subprojects. Value is forced." FORCE)

# Allow overriding options in subprojects by setting a normal variable of the same name.
# Note that setting a CMAKE_POLICY_* variable is the intended way to pass this policy to a subproject.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Using EXCLUDE_FROM_ALL when adding subdirectories so that we dont add the libs to install target.
add_subdirectory(cmrc EXCLUDE_FROM_ALL)
add_subdirectory(fast_float EXCLUDE_FROM_ALL)
add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(cli11 EXCLUDE_FROM_ALL)
add_subdirectory(nlohmann_json EXCLUDE_FROM_ALL)
add_subdirectory(mio EXCLUDE_FROM_ALL)
add_subdirectory(imgui EXCLUDE_FROM_ALL)

# sol2
add_subdirectory(sol2 EXCLUDE_FROM_ALL)
target_compile_definitions(sol2 INTERFACE SOL_EXCEPTIONS_ALWAYS_UNSAFE=1) # Make sure sol2 doesn't propagate C++ exceptions to luajit.

# fmt
add_subdirectory(fmt EXCLUDE_FROM_ALL)
target_compile_definitions(fmt INTERFACE FMT_USE_NONTYPE_TEMPLATE_ARGS) # Enable _cf literals.

# glad
add_subdirectory(glad EXCLUDE_FROM_ALL)
target_include_directories(glad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# luajit
set(LUAJIT_DIR ${CMAKE_CURRENT_LIST_DIR}/luajit)
include(${CMAKE_CURRENT_LIST_DIR}/luajit-cmake/LuaJIT.cmake)
target_include_directories(libluajit INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(libluajit PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}" LINK_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}") # Make it compile under x86.

# inifile_cpp, would rather not use add_subdirectory
add_library(inicpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inifile_cpp/include/inicpp.h)
target_include_directories(inicpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inifile_cpp/include)
add_library(inicpp::inicpp ALIAS inicpp)

# tcpp (header-only C preprocessor)
add_library(tcpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tcpp/source/tcppLibrary.hpp)
target_include_directories(tcpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tcpp/source)
add_library(tcpp::tcpp ALIAS tcpp)

# small_vector
add_subdirectory(small_vector EXCLUDE_FROM_ALL)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
set(SPDLOG_DISABLE_DEFAULT_LOGGER ON)
if(OE_BUILD_PLATFORM STREQUAL "windows")
    set(SPDLOG_WCHAR_SUPPORT ON) # Use unicode APIs on Windows.
endif()
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# googletest
if(OE_BUILD_TESTS)
    set(INSTALL_GTEST OFF)
    set(BUILD_GMOCK ON)
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

# cpptrace, it clones libdwarf with FetchContent at configure time. The options below are libdwarf's and
# cpptrace's, and they only take effect thanks to the CMP0077 default set at the top of this file.
add_subdirectory(libunwind)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    # Cpptrace picks execinfo on macos and the libgcc unwinder on every other unix. Execinfo starts its traces
    # two frames in, and the libgcc unwinder can only walk from where it's called, never from a register state
    # we hand it - which is what a crash handler needs, see thirdparty/libunwind/README.md. So both cpptrace and
    # the crash handler walk with the vendored libunwind instead. Windows is left alone because it wants dbghelp.
    if(NOT OE_BUILD_PLATFORM STREQUAL "windows")
        set(CPPTRACE_UNWIND_WITH_LIBUNWIND ON)
        set(LIBUNWIND_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include")
        # Linked by file rather than by target. Cpptrace exports its targets, and a public dependency on a
        # target outside its export set is a configure error, while a plain path is just a path.
        set(LIBUNWIND_LDFLAGS "$<TARGET_FILE:libunwind>" ${CMAKE_DL_LIBS})
    endif()
    set(ENABLE_DECOMPRESSION OFF) # We don't emit compressed debug sections, and libdwarf can't export its zlib dep.
    set(CPPTRACE_USE_EXTERNAL_ZSTD ON) # Only used for decompression, so this just stops cpptrace downloading it.
    set(CPPTRACE_DISABLE_CXX_20_MODULES ON) # Nothing imports cpptrace, and module scanning needs a ninja generator.
    add_subdirectory(cpptrace EXCLUDE_FROM_ALL)
    if(NOT OE_BUILD_PLATFORM STREQUAL "windows")
        add_dependencies(cpptrace-lib libunwind)
    endif()
endif()

# generator
set(GENERATOR_BUILD_PACKAGE OFF)
set(GENERATOR_BUILD_TESTS OFF)
add_subdirectory(generator EXCLUDE_FROM_ALL)

# lambda2
add_subdirectory(lambda2)

# ztd.text
set(ZTD_IDK_EXCLUDE_FROM_ALL ON)
add_subdirectory(ztd_text EXCLUDE_FROM_ALL)
