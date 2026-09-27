#include "EngineGlobals.h"

#include "Library/Platform/Application/PlatformApplication.h"

Platform *platform = nullptr;
PlatformWindow *window = nullptr;
PlatformOpenGLContext *openGLContext = nullptr;
PlatformEventLoop *eventLoop = nullptr;
PlatformEventHandler *eventHandler = nullptr;
PlatformApplication *application = nullptr;

EngineFlags engineFlags;


void detail::globalProcessMessages() {
    application->processMessages();
}

void detail::globalWaitForMessages() {
    application->waitForMessages();
}
