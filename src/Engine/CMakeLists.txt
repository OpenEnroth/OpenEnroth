cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_SOURCES
        AssetsManager.cpp
        AttackList.cpp
        Conditions.cpp
        Engine.cpp
        EngineGlobals.cpp
        EngineIocContainer.cpp
        GpuHints.cpp
        Localization.cpp
        MapEnums.cpp
        Party.cpp
        PartyEnums.cpp
        PartyPlacement.cpp
        PriceCalculator.cpp
        SaveLoad.cpp
        SpellFxRenderer.cpp
        Timer.cpp
        mm7_data.cpp
        mm7text_ru.cpp
        stru314.cpp
        Seasons.cpp)

set(ENGINE_HEADERS
        ArenaEnumFunctions.h
        ArenaEnums.h
        AssetsManager.h
        AttackList.h
        Conditions.h
        Engine.h
        EngineCallObserver.h
        EngineGlobals.h
        EngineIocContainer.h
        HitMap.h
        Localization.h
        LocalizationEnums.h
        LocationInfo.h
        MapEnumFunctions.h
        MapEnums.h
        Party.h
        PartyEnums.h
        PartyPlacement.h
        Pid.h
        PriceCalculator.h
        SaveLoad.h
        SpawnPoint.h
        SpellFxRenderer.h
        Timer.h
        mm7_data.h
        mm7text_ru.h
        stru314.h
        Seasons.h)

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_check_style(engine)

target_link_libraries(engine
        PUBLIC
        engine_data
        engine_graphics
        engine_graphics_renderer
        engine_objects
        engine_serialization
        engine_spells
        engine_tables
        engine_turn_engine
        engine_events
        engine_random
        engine_resources
        core_time
        library_compression
        library_logger
        library_serialization
        library_color
        library_lod_formats
        library_buildinfo
        library_filesystem_embedded
        library_filesystem_merging
        library_filesystem_masking
        library_filesystem_directory
        library_filesystem_lowercase
        library_filesystem_proxy
        resources
        utility)

target_compile_definitions(engine PRIVATE
        PROJECT_VERSION="${PROJECT_VERSION}"
        OE_BUILD_COMPILER="${CMAKE_CXX_COMPILER_ID}"
        OE_BUILD_PLATFORM="${OE_BUILD_PLATFORM}"
        OE_BUILD_ARCHITECTURE="${OE_BUILD_ARCHITECTURE}")

add_subdirectory(Data)
add_subdirectory(Components)
add_subdirectory(Evt)
add_subdirectory(Graphics)
add_subdirectory(Objects)
add_subdirectory(Random)
add_subdirectory(Resources)
add_subdirectory(Snapshots)
add_subdirectory(Spells)
add_subdirectory(Tables)
add_subdirectory(TurnEngine)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_SOURCES
            Tests/Mm7TextRu_ut.cpp)

    add_library(test_engine OBJECT ${TEST_ENGINE_SOURCES})
    target_link_libraries(test_engine PUBLIC testing_unit engine)

    target_check_style(test_engine)

    target_link_libraries(OpenEnroth_GameTest PUBLIC test_engine)
endif()
