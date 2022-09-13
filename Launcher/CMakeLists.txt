cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(WIN32)
  add_subdirectory(Launcher.Windows)
endif()
