#pragma once

#include <vector>
#include <memory>

#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Filters/PlatformEventFilter.h"

#include "Library/Application/PlatformApplicationAware.h"

/**
 * Component that can be used to record events.
 *
 * Note that this component is intentionally very dumb. Calling `start` starts recording events right away, and
 * calling `finish` just returns everything that was recorded.
 */
class EngineTraceComponent : private ProxyOpenGLContext, private PlatformEventFilter, private PlatformApplicationAware {
 public:
    EngineTraceComponent();
    virtual ~EngineTraceComponent();

    void start();
    std::vector<std::unique_ptr<PlatformEvent>> finish();

    bool isTracing() const {
        return _tracing;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    virtual void swapBuffers() override;
    virtual bool event(const PlatformEvent *event) override;

 private:
    bool _tracing = false;
    std::vector<std::unique_ptr<PlatformEvent>> _trace;
};
