cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BUILD_TESTING OFF CACHE BOOL "Don't build tests for subprojects. Value is forced." FORCE)

# Allow overriding options in subprojects by setting a normal variable of the same name.
# Note that setting a CMAKE_POLICY_* variable is the intended way to pass this policy to a subproject.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

add_subdirectory(fast_float)
add_subdirectory(glm)
add_subdirectory(magic_enum)
add_subdirectory(cli11)
add_subdirectory(nlohmann_json)
add_subdirectory(mio)
add_subdirectory(sol2)
add_subdirectory(imgui)

# glob
# not adding a subdirectory because it will download more libraries at build tree generaion stage
add_library(Glob INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glob/single_include/glob/glob.hpp)
target_include_directories(Glob INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glob/single_include)

# fmt
add_subdirectory(fmt)
target_compile_definitions(fmt INTERFACE FMT_USE_NONTYPE_TEMPLATE_ARGS) # Enable _cf literals.
if(OE_BUILD_COMPILER STREQUAL "msvc")
    target_compile_definitions(fmt INTERFACE FMT_CONSTEVAL=) # MSVC chokes on fmt consteval formatting, so we define FMT_CONSTEVAL=<empty>.
endif()

# glad
add_subdirectory(glad)
target_include_directories(glad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# luajit
add_subdirectory(luajit)
target_include_directories(libluajit INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/luajit/luajit/src)
set_target_properties(libluajit PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}" LINK_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}") # Make it compile under x86.

# inifile_cpp, would rather not use add_subdirectory
add_library(inicpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inifile_cpp/include/inicpp.h)
target_include_directories(inicpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inifile_cpp/include)
add_library(inicpp::inicpp ALIAS inicpp)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
set(SPDLOG_DISABLE_DEFAULT_LOGGER ON)
set(SPDLOG_WCHAR_SUPPORT ON) # Use unicode APIs on Windows.
add_subdirectory(spdlog)

# googletest
if(OE_BUILD_TESTS)
    set(INSTALL_GTEST OFF)
    set(BUILD_GMOCK ON)
    add_subdirectory(googletest)
endif()

# backward
if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_subdirectory(backward_cpp)
    if (OE_BUILD_PLATFORM STREQUAL "linux" AND NOT OE_USE_DUMMY_DEPENDENCIES)
        string(FIND "${BACKWARD_DEFINITIONS}" "BACKWARD_HAS_DWARF=1" DWARF_POS)
        if (${DWARF_POS} EQUAL -1)
            message(WARNING "libdwarf is required for backward-cpp on linux.")
        endif()
    endif()
endif()
