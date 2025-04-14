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
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(fmt INTERFACE FMT_CONSTEVAL=) # MSVC chokes on fmt consteval formatting, so we define FMT_CONSTEVAL=<empty>.
endif()

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

# backward
if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_subdirectory(backward_cpp EXCLUDE_FROM_ALL)
    if (OE_BUILD_PLATFORM STREQUAL "linux" AND NOT OE_USE_DUMMY_DEPENDENCIES)
        string(FIND "${BACKWARD_DEFINITIONS}" "BACKWARD_HAS_DWARF=1" DWARF_POS)
        if (${DWARF_POS} EQUAL -1)
            message(WARNING "libdwarf is required for backward-cpp on linux.")
        endif()
    endif()
endif()

# generator
set(GENERATOR_BUILD_PACKAGE OFF)
set(GENERATOR_BUILD_TESTS OFF)
add_subdirectory(generator EXCLUDE_FROM_ALL)
