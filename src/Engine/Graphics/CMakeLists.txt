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
        IRenderFactory.cpp
        Image.cpp
        ImageLoader.cpp
        Indoor.cpp
        IRender.cpp
        Level/Decoration.cpp
        LightmapBuilder.cpp
        LightsStack.cpp
        LocationFunctions.cpp
        Nuklear.cpp
        NuklearEventHandler.cpp
        OpenGL/GLShaderLoader.cpp
        OpenGL/RenderOpenGL.cpp
        Outdoor.cpp
        Overlays.cpp
        PaletteManager.cpp
        ParticleEngine.cpp
        PortalFunctions.cpp
        RenderBase.cpp
        RenderNull.cpp
        RenderEnums.cpp
        Sprites.cpp
        Texture_MM7.cpp
        TextureFrameTable.cpp
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
        IRender.h
        IRenderFactory.h
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
        OpenGL/GLShaderLoader.h
        OpenGL/RenderOpenGL.h
        Outdoor.h
        Overlays.h
        PaletteManager.h
        ParticleEngine.h
        Polygon.h
        PortalFunctions.h
        RenderBase.h
        RenderEntities.h
        RenderNull.h
        RenderEnums.h
        Sprites.h
        TextureRenderId.h
        Texture_MM7.h
        TextureFrameTable.h
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
