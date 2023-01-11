#include "ProxyEventLoop.h"

ProxyEventLoop::ProxyEventLoop(PlatformEventLoop *base) : ProxyBase<PlatformEventLoop>(base) {}

void ProxyEventLoop::Exec(PlatformEventHandler *eventHandler) {
    NonNullBase()->Exec(eventHandler);
}

void ProxyEventLoop::Quit() {
    NonNullBase()->Quit();
}

void ProxyEventLoop::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    NonNullBase()->ProcessMessages(eventHandler, count);
}

void ProxyEventLoop::WaitForMessages() {
    NonNullBase()->WaitForMessages();
}

