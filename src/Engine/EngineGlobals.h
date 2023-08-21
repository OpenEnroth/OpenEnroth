#pragma once

#include "Platform/Platform.h"
#include "Platform/PlatformWindow.h"
#include "Platform/PlatformOpenGLContext.h"
#include "Platform/PlatformEventLoop.h"

class PlatformApplication;
class Platform;
class PlatformEventHandler;
class PlatformEventLoop;
class PlatformOpenGLContext;
class PlatformWindow;

namespace detail {
void globalProcessMessages();
void globalWaitForMessages();
} // namespace detail

extern int dword_6BE364_game_settings_1;  // GAME_SETTINGS_*

// TODO(captainurist): drop all of these, they are accessible through PlatformApplication
extern Platform *platform;
extern PlatformWindow *window;
extern PlatformEventHandler *eventHandler;
extern PlatformEventLoop *eventLoop;
extern PlatformOpenGLContext *openGLContext;

// TODO(captainurist): this global should go, together with this header file.
extern PlatformApplication *application;

#define MessageLoopWithWait() {                                                                                         \
    detail::globalProcessMessages();                                                                                    \
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {                                                    \
        detail::globalWaitForMessages();                                                                                \
        continue;                                                                                                       \
    }                                                                                                                   \
}
