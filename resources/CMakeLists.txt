cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

file(GLOB_RECURSE RESOURCE_SOURCES "scripts/*" "shaders/*" "images/*")

message(WARNING ${RESOURCE_SOURCES})

cmrc_add_resource_library(resources NAMESPACE openenroth ${RESOURCE_SOURCES})

add_subdirectory(scripts)
