#pragma once

#include <memory>

#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Filters/PlatformEventFilter.h"

#include "EventTrace.h"

class RandomEngine;

/**
 * This class implements event trace collection.
 *
 * To use it:
 * - Install it as an event filter. This is how it will record events, so you have a certain degree of control of
 *   what to record.
 * - Install it as a platform proxy. This is needed to override the default timer.
 */
class EventTracer : private ProxyPlatform, private ProxyOpenGLContext, public PlatformEventFilter {
 public:
    EventTracer();
    ~EventTracer();

    void start();
    void finish(std::string_view path); // TODO(captainurist): return trace here.

    bool isTracing() const {
        return _tracing;
    }

 private:
    friend class PlatformProxyIntrospection;

    virtual int64_t TickCount() const override;
    virtual void SwapBuffers() override;
    virtual bool Event(const PlatformEvent *event) override;

 private:
    bool _tracing = false;
    int64_t _tickCount = 0;
    EventTrace _trace;
    std::unique_ptr<RandomEngine> _oldRandomEngine;
};
