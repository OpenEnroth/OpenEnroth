cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(OE_BUILD_TOOLS)
    add_subdirectory(CodeGen)
    add_subdirectory(LodTool)
    add_subdirectory(FontTool)
endif()

add_subdirectory(OpenEnroth)
