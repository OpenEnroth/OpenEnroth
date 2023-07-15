cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(BIN_CODEGEN_SOURCES
        CodeGen.cpp
        CodeGenOptions.cpp)

set(BIN_CODEGEN_HEADERS
        CodeGenOptions.h)

add_executable(CodeGen ${BIN_CODEGEN_SOURCES} ${BIN_CODEGEN_HEADERS})
target_fix_libcxx_assertions(CodeGen)
target_link_libraries(CodeGen application)
target_check_style(CodeGen)
