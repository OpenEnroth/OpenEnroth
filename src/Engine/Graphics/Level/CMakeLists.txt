cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_RENDERER_LEVEL_SOURCES
        Decoration.cpp)

set(ENGINE_RENDERER_LEVEL_HEADERS
        Decoration.h)

add_library(engine_renderer_level_sources STATIC ${ENGINE_RENDERER_LEVEL_SOURCES} ${ENGINE_RENDERER_LEVEL_HEADERS})
target_check_style(engine_renderer_level_sources)

target_link_libraries(engine_renderer_level
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
