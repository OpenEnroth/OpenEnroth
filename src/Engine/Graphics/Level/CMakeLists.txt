cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_GRAPHICS_LEVEL_SOURCES
        Decoration.cpp)

set(ENGINE_GRAPHICS_LEVEL_HEADERS
        Decoration.h)

add_library(engine_graphics_level_sources STATIC ${ENGINE_GRAPHICS_LEVEL_SOURCES} ${ENGINE_GRAPHICS_LEVEL_HEADERS})

target_link_libraries(engine_graphics_level
        PUBLIC
        engine
        utility
        library_serialization
        library_color
        library_image
        glm::glm
        OpenGL::GL
        PRIVATE
        libluajit
        glad
        nuklear)

target_check_style(engine_graphics_level)
