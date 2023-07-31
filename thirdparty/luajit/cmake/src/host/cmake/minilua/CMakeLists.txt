cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(minilua)

find_library(LIBM_LIBRARIES NAMES m)

add_executable(minilua ../../minilua.c)
if (LIBM_LIBRARIES)
  target_link_libraries(minilua ${LIBM_LIBRARIES})
endif()
