cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(BUILD_TESTING OFF CACHE BOOL "Don't build tests for subprojects. Value is forced." FORCE)

# Allow overriding options in subprojects by setting a normal variable of the same name.
# Note that setting a CMAKE_POLICY_* variable is the intended way to pass this policy to a subproject.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

add_subdirectory(fast_float)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(magic_enum)
add_subdirectory(nuklear)
add_subdirectory(cli11)
add_subdirectory(nlohmann_json)
add_subdirectory(mio)

# glad
add_subdirectory(glad)
target_include_directories(glad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# luajit
add_subdirectory(luajit)
target_include_directories(libluajit INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/luajit/luajit/src)
set_target_properties(libluajit PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}" LINK_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}") # Make it compile under x86.

# mini
add_library(mini INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mini/src/mini/ini.h)
add_library(mini::mini ALIAS mini)
target_include_directories(mini INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mini/src)

# openal_soft
set(LIBTYPE STATIC)
set(ALSOFT_UTILS OFF)
set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_TESTS OFF)
add_subdirectory(openal_soft)

# googletest
if(ENABLE_TESTS)
    set(INSTALL_GTEST OFF)
    set(BUILD_GMOCK OFF)
    add_subdirectory(googletest)
endif()
