cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_CODEGEN_SOURCES
        CodeGen.cpp
        CodeGenOptions.cpp
        CodeGenEnums.cpp)

set(BIN_CODEGEN_HEADERS
        CodeGenOptions.h
        CodeGenEnums.h
        CodeGenMap.h)

if(NOT BUILD_PLATFORM STREQUAL "android")
    add_executable(CodeGen ${BIN_CODEGEN_SOURCES} ${BIN_CODEGEN_HEADERS})
    target_fix_libcxx_assertions(CodeGen)
    target_link_libraries(CodeGen PUBLIC platform_main application CLI11::CLI11)
    target_check_style(CodeGen)
endif()
