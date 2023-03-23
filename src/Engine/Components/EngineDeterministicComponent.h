#pragma once

#include <memory>

#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"

class RandomEngine;

/**
 * This component can be used to make the engine more deterministic. It replaces the system timer with a fake one that
 * advances by 16ms on each frame, and also resets the global random number generator.
 *
 * Note that this component is intentionally very dumb. All the methods just do exactly what you would expect right
 * away, so where you're calling them from becomes very important. E.g. calling `enterDeterministicMode` from an event
 * handler would probably make little sense.
 */
class EngineDeterministicComponent : private ProxyPlatform, private ProxyOpenGLContext {
 public:
    EngineDeterministicComponent();
    virtual ~EngineDeterministicComponent();

    /**
     * 16ms translates to 62.5fps.
     *
     * It is possible to target 60fps and use double here (or store microseconds / nanoseconds in state), but since the
     * platform API is in milliseconds, this will result in staggered frame times, with every 1st and 2nd frame taking
     * 17ms, and every 3rd one taking 16ms. This might result in some non-determinism down the line, e.g. changing the
     * code in level loading will change the number of frames it takes to load a level, and this will shift the timing
     * sequence for the actual game frames after the level is loaded. Unlikely to really affect anything, but we'd
     * rather not find out.
     */
    static constexpr int64_t FRAME_TIME_MS = 16;
    static constexpr int TARGET_FPS = (1000 + FRAME_TIME_MS - 1) / FRAME_TIME_MS; // Rounded up.

    /**
     * Enters deterministic mode.
     *
     * Note that this method works akin to a recursive mutex - it can be called several times, and to leave
     * deterministic mode `leaveDeterministicMode` should then be called the same number of times.
     */
    void enterDeterministicMode();

    /**
     * Resets deterministic state - rolls the tick count back to zero, and resets `grng`. Must already be in
     * deterministic mode.
     */
    void resetDeterministicState();

    /**
     * Leaves deterministic mode. See the note in `enterDeterministicMode`.
     */
    void leaveDeterministicMode();

    bool isInDeterministicMode() const {
        return _deterministicCounter > 0;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    virtual int64_t tickCount() const override;
    virtual void swapBuffers() override;

 private:
    int _deterministicCounter = 0;
    int64_t _tickCount = 0;
    std::unique_ptr<RandomEngine> _oldRandomEngine;
};
