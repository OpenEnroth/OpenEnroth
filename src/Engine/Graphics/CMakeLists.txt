cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_GRAPHICS_SOURCES
        BSPModel.cpp
        BspRenderer.cpp
        Camera.cpp
        ClippingFunctions.cpp
        Collisions.cpp
        DecalBuilder.cpp
        DecorationList.cpp
        FrameLimiter.cpp
        HWLContainer.cpp
        IRenderFactory.cpp
        Image.cpp
        ImageLoader.cpp
        Indoor.cpp
        Level/Decoration.cpp
        LightmapBuilder.cpp
        LightsStack.cpp
        Nuklear.cpp
        NuklearEventHandler.cpp
        OpenGL/GLShaderLoader.cpp
        OpenGL/RenderOpenGL.cpp
        OpenGL/TextureOpenGL.cpp
        Outdoor.cpp
        Overlays.cpp
        PCX.cpp
        PaletteManager.cpp
        ParticleEngine.cpp
        PortalFunctions.cpp
        RenderBase.cpp
        Sprites.cpp
        Viewport.cpp
        Vis.cpp
        Weather.cpp)

set(ENGINE_GRAPHICS_HEADERS
        BSPModel.h
        BspRenderer.h
        Camera.h
        ClippingFunctions.h
        Collisions.h
        DecalBuilder.h
        DecorationList.h
        FrameLimiter.h
        HWLContainer.h
        IRender.h
        IRenderFactory.h
        Image.h
        ImageFormatConverter.h
        ImageLoader.h
        Indoor.h
        Level/Decoration.h
        LightmapBuilder.h
        LightsStack.h
        Nuklear.h
        NuklearEventHandler.h
        OpenGL/GLShaderLoader.h
        OpenGL/RenderOpenGL.h
        OpenGL/TextureOpenGL.h
        Outdoor.h
        Overlays.h
        PCX.h
        PaletteManager.h
        ParticleEngine.h
        Polygon.h
        PortalFunctions.h
        RenderBase.h
        RendererType.h
        Sprites.h
        Texture.h
        Viewport.h
        Vis.h
        Weather.h)

add_library(engine_graphics STATIC ${ENGINE_GRAPHICS_SOURCES} ${ENGINE_GRAPHICS_HEADERS})
target_check_style(engine_graphics)

target_link_libraries(engine_graphics libluajit glad nuklear utility library_serialization)
if (NOT BUILD_PLATFORM STREQUAL "android")
    find_package(OpenGL REQUIRED)
    target_link_libraries(engine_graphics ${OPENGL_opengl_LIBRARY})
endif ()
