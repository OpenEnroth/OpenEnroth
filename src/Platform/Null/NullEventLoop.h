#pragma once

#include "Platform/PlatformEventLoop.h"

class NullPlatformSharedState;

class NullEventLoop : public PlatformEventLoop {
 public:
    explicit NullEventLoop(NullPlatformSharedState *state);
    virtual ~NullEventLoop();

    virtual void exec(PlatformEventHandler *eventHandler) override;
    virtual void quit() override;
    virtual void processMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void waitForMessages() override;

 private:
    void deadlock();

 private:
    NullPlatformSharedState *_state = nullptr;
};
