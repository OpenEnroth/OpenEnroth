#pragma once

#include <memory>
#include <vector>

#include "Library/Platform/Proxy/ProxyOpenGLContext.h"
#include "Library/Platform/Filters/PlatformEventFilter.h"

#include "Library/Platform/Application/PlatformApplicationAware.h"

/**
 * Component that can be used to record events.
 *
 * Note that this component is intentionally very dumb. Calling `startRecording` starts recording events right away,
 * and calling `finishRecording` just returns everything that was recorded.
 *
 * @see EngineTraceRecorder
 */
class EngineTraceSimpleRecorder : private ProxyOpenGLContext, private PlatformEventFilter, private PlatformApplicationAware {
 public:
    EngineTraceSimpleRecorder();
    virtual ~EngineTraceSimpleRecorder();

    /**
     * Starts trace recording.
     */
    void startRecording();

    /**
     * @return                          Recorded events.
     */
    std::vector<std::unique_ptr<PlatformEvent>> finishRecording();

    bool isRecording() const {
        return _recording;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    virtual void swapBuffers() override;
    virtual bool event(const PlatformEvent *event) override;

 private:
    bool _recording = false;
    std::vector<std::unique_ptr<PlatformEvent>> _events;
};
