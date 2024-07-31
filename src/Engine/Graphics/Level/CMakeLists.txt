cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_GRAPHICS_LEVEL_SOURCES
        Decoration.cpp)

set(ENGINE_GRAPHICS_LEVEL_HEADERS
        Decoration.h)

add_library(engine_graphics_level STATIC ${ENGINE_GRAPHICS_LEVEL_SOURCES} ${ENGINE_GRAPHICS_LEVEL_HEADERS})
target_check_style(engine_graphics_level)

target_link_libraries(engine_graphics_level
        PUBLIC
        engine
        engine_graphics
        engine_graphics_renderer
        utility
        library_serialization
        library_color
        library_image
        glm::glm
        OpenGL::GL
        engine_graphics
        PRIVATE
        libluajit
        glad)
