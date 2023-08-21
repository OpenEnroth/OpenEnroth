#include "EngineGlobals.h"

#include "Library/Application/PlatformApplication.h"

class Platform;
class PlatformEventHandler;
class PlatformEventLoop;
class PlatformOpenGLContext;
class PlatformWindow;

Platform *platform = nullptr;
PlatformWindow *window = nullptr;
PlatformOpenGLContext *openGLContext = nullptr;
PlatformEventLoop *eventLoop = nullptr;
PlatformEventHandler *eventHandler = nullptr;
PlatformApplication *application = nullptr;


void detail::globalProcessMessages() {
    application->processMessages();
}

void detail::globalWaitForMessages() {
    application->waitForMessages();
}
