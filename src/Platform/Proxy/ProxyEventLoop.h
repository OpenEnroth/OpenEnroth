#pragma once

#include "Platform/PlatformEventLoop.h"
#include "ProxyBase.h"

class PlatformEventHandler;
class PlatformEventLoop;

class ProxyEventLoop: public ProxyBase<PlatformEventLoop> {
 public:
    explicit ProxyEventLoop(PlatformEventLoop *base = nullptr);
    virtual ~ProxyEventLoop() = default;

    virtual void exec(PlatformEventHandler *eventHandler) override;
    virtual void quit() override;
    virtual void processMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void waitForMessages() override;
};
