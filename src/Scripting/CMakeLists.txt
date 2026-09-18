cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(SCRIPTING_SOURCES
        AudioBindings.cpp
        ConfigBindings.cpp
        EvtBindings.cpp
        OverlayBindings.cpp
        GameBindings.cpp
        ImGuiBindings.cpp
        InputBindings.cpp
        InputScriptEventHandler.cpp
        LoggerBindings.cpp
        LuaEvtScripts.cpp
        PlatformBindings.cpp
        RendererBindings.cpp
        ScriptingSystem.cpp
        ScriptLogSink.cpp)

set(SCRIPTING_HEADERS
        AudioBindings.h
        ConfigBindings.h
        EvtBindings.h
        OverlayBindings.h
        ImGuiBindings.h
        GameBindings.h
        IBindings.h
        InputBindings.h
        InputScriptEventHandler.h
        LoggerBindings.h
        LuaEvtScripts.h
        LuaItemQueryTable.h
        PlatformBindings.h
        RendererBindings.h
        ScriptingSystem.h
        ScriptLogSink.h)

add_library(scripting STATIC ${SCRIPTING_SOURCES} ${SCRIPTING_HEADERS})
target_check_style(scripting)

target_link_libraries(scripting
        PUBLIC
        engine
        engine_events
        library_logger
        gui_overlay
        PRIVATE
        libluajit
        sol2::sol2)
