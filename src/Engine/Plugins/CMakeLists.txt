cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(PLUGINS_SOURCES EngineController.cpp
                    EngineControlPlugin.cpp
                    EngineDeterministicPlugin.cpp
                    EngineTracePlugin.cpp
                    EngineTracer.cpp)

set(PLUGINS_HEADERS EngineController.h
                    EngineControlPlugin.h
                    EngineControlState.h
                    EngineControlStateHandle.h
                    EngineDeterministicPlugin.h
                    EngineTracePlaybackFlags.h
                    EngineTracePlugin.h
                    EngineTracer.h)

# TODO(captainurist): lib naming policy needs revisiting. engine_plugins would make more sense here.
add_library(plugins STATIC ${PLUGINS_SOURCES} ${PLUGINS_HEADERS})
target_check_style(plugins)

target_link_libraries(plugins engine platform utility application)
