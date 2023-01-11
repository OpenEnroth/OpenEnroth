#pragma once

#include "Platform/PlatformEventLoop.h"

#include "ProxyBase.h"

class ProxyEventLoop: public ProxyBase<PlatformEventLoop> {
 public:
    explicit ProxyEventLoop(PlatformEventLoop *base = nullptr);
    virtual ~ProxyEventLoop() = default;

    virtual void Exec(PlatformEventHandler *eventHandler) override;
    virtual void Quit() override;
    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void WaitForMessages() override;
};
