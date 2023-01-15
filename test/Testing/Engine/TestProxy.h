#pragma once

#include <queue>
#include <memory>

#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Proxy/ProxyEventLoop.h"

#include "TestStateHandle.h"

class PlatformEventHandler;

/**
 * Platform proxy class that substitutes some of the platform functionality:
 * - Overrides the default timer.
 * - Ignores all spontaneous (OS-generated) events.
 * - Provides an API to send synthetic events instead.
 *
 * To use it, install it as a platform proxy.
 */
class TestProxy : private ProxyPlatform, private ProxyEventLoop, private ProxyOpenGLContext {
 public:
    explicit TestProxy(TestStateHandle state);
    virtual ~TestProxy();

    void reset();

    void postEvent(PlatformWindow *window, std::unique_ptr<PlatformEvent> event);

 private:
    friend class PlatformProxyIntrospection;

    virtual int64_t TickCount() const override;
    virtual void SwapBuffers() override;
    virtual void Exec(PlatformEventHandler *eventHandler) override;
    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count = -1) override;

    void processSyntheticMessages(PlatformEventHandler *eventHandler);

    struct PostedEvent {
        PlatformWindow *window;
        std::unique_ptr<PlatformEvent> event;
    };

 private:
    TestStateHandle _state;
    int64_t _tickCount = 0;
    std::queue<PostedEvent> postedEvents_;
    std::unique_ptr<PlatformEventHandler> emptyHandler_;
};
