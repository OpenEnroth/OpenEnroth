cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(UI_NEW_SYSTEM_SOURCES
    OpenGLUiRenderer.cpp
    RmlEventListenerInstancer.cpp
    RmlFontEngineInterface.cpp
    RmlSystemInterface.cpp
    RmlFileInterface.cpp
    UiEventHandler.cpp
    UiSystem.cpp)

set(UI_NEW_SYSTEM_HEADERS
    OpenGLUiRenderer.h
    RmlEventListenerInstancer.h
    RmlFontEngineInterface.h
    RmlSystemInterface.h
    RmlFileInterface.h
    UiRenderer.h
    UiEventHandler.h
    UiSystem.h)

add_library(ui_new_system STATIC ${UI_NEW_SYSTEM_SOURCES} ${UI_NEW_SYSTEM_HEADERS})
target_check_style(ui_new_system)

target_link_libraries(ui_new_system
        PUBLIC
        utility
        glm::glm
        PRIVATE
        glad
        RmlCore
        RmlDebugger)