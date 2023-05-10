cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_SOURCES
        AssetsManager.cpp
        AttackList.cpp
        Conditions.cpp
        Engine.cpp
        EngineFactory.cpp
        EngineGlobals.cpp
        ErrorHandling.cpp
        EngineIocContainer.cpp
        LOD.cpp
        Localization.cpp
        MapInfo.cpp
        OurMath.cpp
        Party.cpp
        PriceCalculator.cpp
        SaveLoad.cpp
        SpellFxRenderer.cpp
        Time.cpp
        mm7_data.cpp
        mm7text_ru.cpp)

set(ENGINE_HEADERS
        AssetsManager.h
        AttackList.h
        Autonotes.h
        Awards.h
        Conditions.h
        Engine.h
        EngineFactory.h
        EngineGlobals.h
        ErrorHandling.h
        EngineIocContainer.h
        LOD.h
        Localization.h
        MM7.h
        MapInfo.h
        OurMath.h
        Party.h
        PriceCalculator.h
        SaveLoad.h
        SpellFxRenderer.h
        Time.h
        mm7_data.h
        stru160.h
        stru314.h
        stru367.h)

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_check_style(engine)

target_link_libraries(engine
        engine_graphics
        engine_objects
        engine_serialization
        engine_spells
        engine_tables
        engine_turnengine
        engine_events
        library_compression
        library_logger
        library_serialization
        utility)

target_compile_definitions(engine PRIVATE
        PROJECT_VERSION="${PROJECT_VERSION}"
        BUILD_COMPILER="${BUILD_COMPILER}"
        BUILD_PLATFORM="${BUILD_PLATFORM}"
        BUILD_TYPE="${BUILD_TYPE}")

add_subdirectory(Graphics)
add_subdirectory(Objects)
add_subdirectory(Serialization)
add_subdirectory(Spells)
add_subdirectory(Tables)
add_subdirectory(Components)
add_subdirectory(TurnEngine)
add_subdirectory(Events)
