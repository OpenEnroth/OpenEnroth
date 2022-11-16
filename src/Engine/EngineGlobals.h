#pragma once

#include "Platform/Platform.h"
#include "Platform/PlatformWindow.h"
#include "Platform/PlatformOpenGLContext.h"
#include "Platform/PlatformEventLoop.h"

extern int dword_6BE364_game_settings_1;  // GAME_SETTINGS_*

extern Platform *platform;
extern PlatformWindow *window;
extern PlatformOpenGLContext *openGlContext;
extern PlatformEventLoop *eventLoop;
extern PlatformEventHandler *eventHandler;

#define MessageLoopWithWait() {                                                 \
    eventLoop->ProcessMessages(eventHandler);                                   \
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {            \
        eventLoop->WaitForMessages();                                           \
        continue;                                                               \
    }                                                                           \
}
