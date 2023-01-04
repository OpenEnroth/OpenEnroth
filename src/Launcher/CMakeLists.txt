cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(BUILD_PLATFORM STREQUAL "windows")
  add_subdirectory(Launcher.Windows)
endif()
