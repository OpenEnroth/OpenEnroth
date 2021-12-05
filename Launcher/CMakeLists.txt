cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(WIN32)
  add_subdirectory(Launcher.Windows)
else()
endif()
