#pragma once

#include "Library/Platform/Interface/PlatformEventLoop.h"

class NullPlatformSharedState;

class NullEventLoop : public PlatformEventLoop {
 public:
    explicit NullEventLoop(NullPlatformSharedState *state);
    virtual ~NullEventLoop();

    virtual void exec(PlatformEventHandler *eventHandler) override;
    virtual void quit() override;
    virtual void processMessages(PlatformEventHandler *eventHandler) override;
    virtual void waitForMessages() override;

 private:
    void deadlock();

 private:
    NullPlatformSharedState *_state = nullptr;
};
