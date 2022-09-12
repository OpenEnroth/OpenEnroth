cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)
project("World of Might and Magic")
set(PROJECT_VERSION "Unknown")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
include(Git)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(BUILD_PLATFORM "darwin")
  set(APPLE TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(BUILD_PLATFORM "linux")
else()
  set(BUILD_PLATFORM "win32")
endif()

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(BUILD_TYPE "x64")
else ()
  set(BUILD_TYPE "x86")
endif ()

message(STATUS "WoMM version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${BUILD_TYPE}")
message(STATUS "Build platform: ${BUILD_PLATFORM}")

if(NOT BUILD_TYPE STREQUAL "x86" )
  message(WARNING "Please note that currently only x86 builds are relatively stable" )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold -pthread")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH="") # vs2019 complains about using #pragma pack()
  add_definitions(-DNOMINMAX) # please don't pull in these macros from <Windows.h>
  add_definitions(-D_CRT_SECURE_NO_WARNINGS) # STL security warnings are just noise
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE) # POSIX deprecation warnings are also just noise
  add_definitions(-D_USE_MATH_DEFINES) # Pull in M_PI and other <cmath> defines
  set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

set(THIRDPARTY_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(GLAD_DIRECTORY ${THIRDPARTY_DIRECTORY}/glad)
set(NUKLEAR_DIRECTORY ${THIRDPARTY_DIRECTORY}/nuklear)
set(LUAJIT_DIRECTORY ${THIRDPARTY_DIRECTORY}/luajit)
set(GLM_DIRECTORY ${THIRDPARTY_DIRECTORY}/glm)

include_directories(${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${THIRDPARTY_DIRECTORY} ${LUAJIT_DIRECTORY}/luajit/src ${GLAD_DIRECTORY}/include ${GLM_DIRECTORY}/glm)

add_definitions(-DPACKAGE_BUGREPORT="https://github.com/gp-alex/world-of-might-and-magic/issues")

include(CppLint)
enable_check_style()

if(NOT WIN32)
  include_directories("/usr/local/include")
  link_directories("/usr/local/lib")
endif()

include(PrebuiltDeps)

add_subdirectory(${GLAD_DIRECTORY})
add_subdirectory(${GLM_DIRECTORY})
add_subdirectory(${LUAJIT_DIRECTORY})
add_subdirectory(${NUKLEAR_DIRECTORY})

add_subdirectory(Arcomage)
add_subdirectory(Engine)
add_subdirectory(src)
add_subdirectory(GUI)
add_subdirectory(Io)
add_subdirectory(Launcher)
add_subdirectory(Media)
add_subdirectory(Platform)

set_target_properties(libluajit PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}" LINK_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")

add_executable(World_of_Might_and_Magic MACOSX_BUNDLE WIN32 ./Engine/Engine.cpp)
target_check_style(World_of_Might_and_Magic)

target_link_libraries(World_of_Might_and_Magic game arcomage engine gui io media platform objects spells tables graphics turnengine serialization glad nuklear libluajit glm)

set_property(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT World_of_Might_and_Magic
)

RESOLVE_DEPENDENCIES(World_of_Might_and_Magic)
