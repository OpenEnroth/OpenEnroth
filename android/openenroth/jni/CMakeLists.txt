cmake_minimum_required(VERSION 3.27)

project(GAME)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # CMake 4.5 builds `import std` for scanned targets, and the NDK's clang finds the host's libstdc++ module sources.

add_subdirectory(../../../ OpenEnroth)
