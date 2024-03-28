cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_GRAPHICS_RENDERER_SOURCES
        Decoration.cpp)

set(ENGINE_GRAPHICS_RENDERER_HEADERS
        Decoration.h)

add_library(engine_graphics_renderer_sources STATIC ${ENGINE_GRAPHICS_RENDERER_SOURCES} ${ENGINE_GRAPHICS_RENDERER_HEADERS})
target_check_style(engine_graphics_renderer)

target_link_libraries(engine_graphics_renderer
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
