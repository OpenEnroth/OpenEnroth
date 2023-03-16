cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_SOURCES AssetsManager.cpp
                   Conditions.cpp
                   Engine.cpp
                   EngineGlobals.cpp
                   Events.cpp
                   ErrorHandling.cpp
                   Localization.cpp
                   LOD.cpp
                   MapInfo.cpp
                   mm7text_ru.cpp
                   mm7_data.cpp
                   OurMath.cpp
                   Party.cpp
                   SaveLoad.cpp
                   SpellFxRenderer.cpp
                   AttackList.cpp
                   Time.cpp
                   IocContainer.cpp
                   EngineFactory.cpp)

set(ENGINE_HEADERS AssetsManager.h
                   Autonotes.h
                   Awards.h
                   Conditions.h
                   Engine.h
                   EngineGlobals.h
                   ErrorHandling.h
                   Events.h
                   Events2D.h
                   Localization.h
                   LOD.h
                   MapInfo.h
                   MapsLongTimer.h
                   MM7.h
                   mm7_data.h
                   OurMath.h
                   Party.h
                   SaveLoad.h
                   SpellFxRenderer.h
                   stru123.h
                   stru159.h
                   stru160.h
                   AttackList.h
                   stru314.h
                   stru367.h
                   Time.h
                   IocContainer.h
                   EngineFactory.h)

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_check_style(engine)

target_link_libraries(engine objects graphics engine_serialization spells tables turnengine utility logger compression serialization)
target_compile_definitions(engine PRIVATE PROJECT_VERSION="${PROJECT_VERSION}"
                                          BUILD_COMPILER="${BUILD_COMPILER}"
                                          BUILD_PLATFORM="${BUILD_PLATFORM}"
                                          BUILD_TYPE="${BUILD_TYPE}")

add_subdirectory(Graphics)
add_subdirectory(Objects)
add_subdirectory(Serialization)
add_subdirectory(Spells)
add_subdirectory(Tables)
add_subdirectory(Plugins)
add_subdirectory(TurnEngine)

