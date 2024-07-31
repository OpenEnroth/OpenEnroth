cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(OVERLAY_SOURCES
        OverlayEventHandler.cpp
        OverlaySystem.cpp
        ExampleOverlay.cpp
        ScriptedOverlay.cpp)

set(OVERLAY_HEADERS
        Overlay.h
        OverlayEventHandler.h
        OverlaySystem.h
        ExampleOverlay.h
        ScriptedOverlay.h)

add_library(gui_overlay STATIC ${OVERLAY_SOURCES} ${OVERLAY_HEADERS})
target_check_style(gui_overlay)

target_link_libraries(gui_overlay
    PUBLIC
    engine
    utility
    PRIVATE
    imgui
    scripting
)
