cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(UI_NEW_SYSTEM_SOURCES
    OpenGLUiRenderer.cpp
    RmlSystemInterface.cpp
    UiInputEventHandler.cpp
    UiSystem.cpp)

set(UI_NEW_SYSTEM_HEADERS
    OpenGLUiRenderer.h
    RmlSystemInterface.h
    UiRenderer.h
    UiInputEventHandler.h
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