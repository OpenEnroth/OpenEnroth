#include "ProxyEventLoop.h"

ProxyEventLoop::ProxyEventLoop(PlatformEventLoop *base) : ProxyBase<PlatformEventLoop>(base) {}

void ProxyEventLoop::exec(PlatformEventHandler *eventHandler) {
    nonNullBase()->exec(eventHandler);
}

void ProxyEventLoop::quit() {
    nonNullBase()->quit();
}

void ProxyEventLoop::processMessages(PlatformEventHandler *eventHandler) {
    nonNullBase()->processMessages(eventHandler);
}

void ProxyEventLoop::waitForMessages() {
    nonNullBase()->waitForMessages();
}

