cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_GRAPHICS_SOURCES
        BSPModel.cpp
        BspRenderer.cpp
        Camera.cpp
        ClippingFunctions.cpp
        Collisions.cpp
        DecalBuilder.cpp
        DecorationList.cpp
        FrameLimiter.cpp
        Image.cpp
        ImageLoader.cpp
        Indoor.cpp
        Level/Decoration.cpp
        LightmapBuilder.cpp
        LightsStack.cpp
        LocationFunctions.cpp
        Nuklear.cpp
        NuklearEventHandler.cpp
        Outdoor.cpp
        Overlays.cpp
        PaletteManager.cpp
        ParticleEngine.cpp
        PortalFunctions.cpp
        Renderer/BaseRenderer.cpp
        Renderer/NullRenderer.cpp
        Renderer/OpenGLRenderer.cpp
        Renderer/OpenGLShader.cpp
        Renderer/Renderer.cpp
        Renderer/RendererEnums.cpp
        Renderer/RendererFactory.cpp
        Sprites.cpp
        TextureFrameTable.cpp
        Texture_MM7.cpp
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
        FaceEnums.h
        FrameLimiter.h
        Image.h
        ImageLoader.h
        Indoor.h
        Level/Decoration.h
        LightmapBuilder.h
        LightsStack.h
        LocationFunctions.h
        LocationInfo.h
        LocationTime.h
        Nuklear.h
        NuklearEventHandler.h
        Outdoor.h
        Overlays.h
        PaletteManager.h
        ParticleEngine.h
        Polygon.h
        PortalFunctions.h
        RenderEntities.h
        Renderer/BaseRenderer.h
        Renderer/NullRenderer.h
        Renderer/OpenGLRenderer.h
        Renderer/OpenGLShader.h
        Renderer/Renderer.h
        Renderer/RendererEnums.h
        Renderer/RendererFactory.h
        Renderer/TextureRenderId.h
        Sprites.h
        TextureFrameTable.h
        Texture_MM7.h
        Viewport.h
        Vis.h
        Weather.h)

add_library(engine_graphics STATIC ${ENGINE_GRAPHICS_SOURCES} ${ENGINE_GRAPHICS_HEADERS})
target_check_style(engine_graphics)

target_link_libraries(engine_graphics
        PUBLIC
        engine
        utility
        library_serialization
        library_color
        library_image
        glm::glm
        PRIVATE
        libluajit
        glad
        nuklear)

if (NOT BUILD_PLATFORM STREQUAL "android")
    find_package(OpenGL REQUIRED)
    target_link_libraries(engine_graphics PUBLIC OpenGL::GL)
endif ()
