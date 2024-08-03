cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_GRAPHICS_RENDERER_SOURCES
        BaseRenderer.cpp
        NullRenderer.cpp
        OpenGLRenderer.cpp
        OpenGLShader.cpp
        Renderer.cpp
        RendererEnums.cpp
        RendererFactory.cpp
        )

set(ENGINE_GRAPHICS_RENDERER_HEADERS
        BaseRenderer.h
        NullRenderer.h
        OpenGLRenderer.h
        OpenGLShader.h
        Renderer.h
        RendererEnums.h
        RendererFactory.h
        TextureRenderId.h
        )

add_library(engine_graphics_renderer STATIC ${ENGINE_GRAPHICS_RENDERER_SOURCES} ${ENGINE_GRAPHICS_RENDERER_HEADERS})
target_check_style(engine_graphics_renderer)

target_link_libraries(engine_graphics_renderer
        PUBLIC
        engine
        engine_graphics
        utility
        library_serialization
        library_color
        library_image
        glm::glm
        OpenGL::GL
        engine_graphics
        PRIVATE
        glad)
