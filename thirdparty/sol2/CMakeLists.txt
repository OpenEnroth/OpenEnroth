cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

project(sol2)

set(SOL2_HEADERS sol/sol.hpp
                    sol/config.hpp
                    sol/forward.hpp)


add_library(sol2 INTERFACE ${SOL2_HEADERS})
target_include_directories(sol2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
