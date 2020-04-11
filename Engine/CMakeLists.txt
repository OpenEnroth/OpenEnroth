cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(ENGINE_SOURCES AssetsManager.cpp
                   Conditions.cpp
                   Events.cpp
                   ErrorHandling.cpp
                   Localization.cpp
                   LOD.cpp
                   Log.cpp
                   MapInfo.cpp
                   mm7text_ru.cpp
                   mm7_data.cpp
                   OurMath.cpp
                   Party.cpp
                   Random.cpp
                   SaveLoad.cpp
                   SpellFxRenderer.cpp
                   Strings.cpp
                   stru298.cpp
                   Time.cpp
                   VectorTypes.cpp
                   ZlibWrapper.cpp
                   IocContainer.cpp
                   EngineConfig.cpp
                   EngineConfigFactory.cpp
                   EngineFactory.cpp)

set(ENGINE_HEADERS AssetsManager.h
                   Autonotes.h
                   Awards.h
                   Conditions.h
                   Engine.h
                   ErrorHandling.h
                   Events.h
                   Events2D.h
                   Localization.h
                   LOD.h
                   Log.h
                   MapInfo.h
                   MapsLongTimer.h
                   MM7.h
                   mm7_data.h
                   NZIArray.h
                   OurMath.h
                   Party.h
                   Point.h
                   Random.h
                   Rect.h
                   SaveLoad.h
                   SpellFxRenderer.h
                   Strings.h
                   stru123.h
                   stru159.h
                   stru160.h
                   stru298.h
                   stru314.h
                   stru367.h
                   Time.h
                   VectorTypes.h
                   ZlibWrapper.h
                   IocContainer.h
                   EngineConfig.h
                   EngineConfigFactory.h
                   EngineFactory.h)

if(WIN32)
else()
endif()

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_check_style(engine)

target_link_libraries(engine zlib)
target_link_libraries(engine game)

add_subdirectory(Graphics)
add_subdirectory(Objects)
add_subdirectory(Serialization)
add_subdirectory(Spells)
add_subdirectory(Tables)
add_subdirectory(TurnEngine)
