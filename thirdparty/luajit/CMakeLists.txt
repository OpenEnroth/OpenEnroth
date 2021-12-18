cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/luajit/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/luajit/cmake/modules"
)

set(LUAJIT_TOP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/luajit)

project(luajit)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/extra")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/luajit")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/luajit")

set(can_use_assembler TRUE)
enable_language(ASM)

include (GNUInstallDirs)

add_subdirectory(luajit/src)
