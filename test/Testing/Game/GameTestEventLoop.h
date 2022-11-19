#pragma once

#include <memory>
#include <queue>

#include "Platform/PlatformEventLoop.h"

#include "GameTestStateHandle.h"

class PlatformWindow;
class PlatformEvent;

class GameTestEventLoop: public PlatformEventLoop {
 public:
    GameTestEventLoop(std::unique_ptr<PlatformEventLoop> base, GameTestStateHandle state);
    virtual ~GameTestEventLoop();

    virtual void Exec(PlatformEventHandler *eventHandler) override;
    virtual void Quit() override;
    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void WaitForMessages() override;

    void PostEvent(PlatformWindow *window, std::unique_ptr<PlatformEvent> event);

 private:
    void ProcessSyntheticMessages(PlatformEventHandler *eventHandler);

 private:
    struct PostedEvent {
        PlatformWindow *window;
        std::unique_ptr<PlatformEvent> event;
    };

 private:
    std::queue<PostedEvent> postedEvents_;
    std::unique_ptr<PlatformEventLoop> base_;
    GameTestStateHandle state_;
};
