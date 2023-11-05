#pragma once

#include "Library/Platform/Interface/PlatformEventLoop.h"

#include "ProxyBase.h"

class ProxyEventLoop: public ProxyBase<PlatformEventLoop> {
 public:
    explicit ProxyEventLoop(PlatformEventLoop *base = nullptr);
    virtual ~ProxyEventLoop() = default;

    virtual void exec(PlatformEventHandler *eventHandler) override;
    virtual void quit() override;
    virtual void processMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void waitForMessages() override;
};
