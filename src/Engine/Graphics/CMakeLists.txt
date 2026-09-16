cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

add_subdirectory(Renderer)

set(ENGINE_GRAPHICS_SOURCES
        
        BspRenderer.cpp
        Camera.cpp
        ClippingFunctions.cpp
        Collisions.cpp
        DecalBuilder.cpp
        FrameLimiter.cpp
        Image.cpp
        ImageLoader.cpp
        Indoor.cpp
        Lighting.cpp
        LightsStack.cpp
        LocationFunctions.cpp
        Outdoor.cpp
        Overlays.cpp
        PaletteManager.cpp
        ParticleEngine.cpp
        PortalFunctions.cpp
        Sprites.cpp
        TileGenerator.cpp
        TurnBasedOverlay.cpp
        Viewport.cpp
        Vis.cpp
        Weather.cpp)

set(ENGINE_GRAPHICS_HEADERS
        AtlasLayout.h
        BSPModel.h
        BspRenderer.h
        Camera.h
        ClippingFunctions.h
        Collisions.h
        DecalBuilder.h
        FaceEnums.h
        FrameLimiter.h
        Image.h
        ImageLoader.h
        Indoor.h
        Lighting.h
        LightsStack.h
        LocationFunctions.h
        MapWeather.h
        Outdoor.h
        Overlays.h
        PaletteManager.h
        ParticleEngine.h
        PortalFunctions.h
        RenderEntities.h
        Sprites.h
        SpriteEnums.h
        SpriteEnumFunctions.h
        TileGenerator.h
        TurnBasedOverlay.h
        Viewport.h
        Vis.h
        Weather.h
        OutdoorTerrain.h
        OutdoorTerrain.cpp)

add_library(engine_graphics STATIC ${ENGINE_GRAPHICS_SOURCES} ${ENGINE_GRAPHICS_HEADERS})
target_check_style(engine_graphics)

target_link_libraries(engine_graphics
        PUBLIC
        engine
        engine_graphics_renderer
        utility
        library_serialization
        library_color
        library_image
        glm::glm
        OpenGL::GL
        libluajit
        sol2
        PRIVATE
        glad)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_GRAPHICS_SOURCES
            Tests/DecalBuilder_ut.cpp)

    add_library(test_engine_graphics OBJECT ${TEST_ENGINE_GRAPHICS_SOURCES})
    target_link_libraries(test_engine_graphics PUBLIC testing_unit engine_graphics)

    target_check_style(test_engine_graphics)

    target_link_libraries(OpenEnroth_GameTest PUBLIC test_engine_graphics)
endif()
