cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_SOURCES
        AssetsManager.cpp
        AttackList.cpp
        Conditions.cpp
        Engine.cpp
        EngineGlobals.cpp
        EngineIocContainer.cpp
        GpuHints.cpp
        LOD.cpp
        LodTextureCache.cpp
        LodSpriteCache.cpp
        Localization.cpp
        MapEnums.cpp
        MapInfo.cpp
        OurMath.cpp
        Party.cpp
        PriceCalculator.cpp
        SaveLoad.cpp
        SpellFxRenderer.cpp
        TeleportPoint.cpp
        Time.cpp
        GameResourceManager.cpp
        mm7_data.cpp
        mm7text_ru.cpp)

set(ENGINE_HEADERS
        AssetsManager.h
        AttackList.h
        Conditions.h
        Engine.h
        EngineGlobals.h
        EngineIocContainer.h
        LOD.h
        LodTextureCache.h
        LodSpriteCache.h
        Localization.h
        MapEnums.h
        MapInfo.h
        OurMath.h
        Party.h
        PartyEnums.h
        Pid.h
        PriceCalculator.h
        SaveLoad.h
        SpellFxRenderer.h
        TeleportPoint.h
        Time.h
        GameResourceManager.h
        mm7_data.h
        stru160.h
        stru314.h)

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_check_style(engine)

target_link_libraries(engine
        PUBLIC
        engine_graphics
        engine_objects
        engine_serialization
        engine_spells
        engine_tables
        engine_turn_engine
        engine_events
        library_compression
        library_logger
        library_serialization
        library_color
        library_lod_formats
        library_buildinfo
        utility)

target_compile_definitions(engine PRIVATE
        PROJECT_VERSION="${PROJECT_VERSION}"
        BUILD_COMPILER="${BUILD_COMPILER}"
        BUILD_PLATFORM="${BUILD_PLATFORM}"
        BUILD_ARCHITECTURE="${BUILD_ARCHITECTURE}")

add_subdirectory(Graphics)
add_subdirectory(Objects)
add_subdirectory(Snapshots)
add_subdirectory(Spells)
add_subdirectory(Tables)
add_subdirectory(Components)
add_subdirectory(TurnEngine)
add_subdirectory(Events)
