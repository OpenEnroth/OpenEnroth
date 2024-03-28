cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_CODEGEN_SOURCES
        CodeGen.cpp
        CodeGenOptions.cpp
        CodeGenEnums.cpp)

set(BIN_CODEGEN_HEADERS
        CodeGenOptions.h
        CodeGenEnums.h
        CodeGenFunctions.h
        CodeGenMap.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(CodeGen ${BIN_CODEGEN_SOURCES} ${BIN_CODEGEN_HEADERS})
    target_link_libraries(CodeGen PUBLIC application library_cli library_platform_main)
    target_check_style(CodeGen)
endif()
