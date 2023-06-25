#pragma once

#include <memory>

#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Filters/PlatformEventFilter.h"

#include "Library/Application/PlatformApplicationAware.h"

struct EventTrace;

/**
 * Component that can be used to record events.
 *
 * Note that this component is intentionally very dumb. Calling `startRecording` starts recording events right away,
 * and calling `finishRecording` just returns everything that was recorded.
 *
 * @see EngineTraceRecorder
 */
class EngineTraceComponent : private ProxyOpenGLContext, private PlatformEventFilter, private PlatformApplicationAware {
 public:
    EngineTraceComponent();
    virtual ~EngineTraceComponent();

    /**
     * Starts trace recording.
     */
    void startRecording();

    /**
     * @return                          Recorded trace. Note that it's up to the caller to fill the save file-related
     *                                  fields of `EventTraceHeader` because `EngineTraceComponent` doesn't know
     *                                  anything about save files.
     */
    EventTrace finishRecording();

    bool isRecording() const {
        return _trace != nullptr;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    virtual void swapBuffers() override;
    virtual bool event(const PlatformEvent *event) override;

 private:
    std::unique_ptr<EventTrace> _trace;
};
