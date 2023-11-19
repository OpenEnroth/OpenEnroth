#pragma once

#include <memory>

#include "Library/Platform/Proxy/ProxyPlatform.h"
#include "Library/Platform/Proxy/ProxyOpenGLContext.h"
#include "Engine/Random/RandomEnums.h"
#include "Library/Platform/Application/PlatformApplicationAware.h"

class RandomEngine;

/**
 * This component can be used to make the engine more deterministic. It replaces the system timer with a fake one that
 * advances by 16ms on each frame, and also resets the global random number generator.
 *
 * Note that this component is intentionally very dumb. All the methods just do exactly what you would expect right
 * away, so where you're calling them from becomes very important. E.g. calling `restart` from an event
 * handler would probably make little sense.
 */
class EngineDeterministicComponent : private ProxyPlatform, private ProxyOpenGLContext, private PlatformApplicationAware {
 public:
    EngineDeterministicComponent();
    virtual ~EngineDeterministicComponent();

    /**
     * Starts a new deterministic segment, resetting all state - rolls the tick count back to zero, and resets `grng`.
     *
     * Why we're passing frame time in milliseconds here and not setting the target fps directly?
     * For example, it is possible to target 60fps, but since the platform API is in milliseconds, this will result in
     * staggered frame times, with every 1st and 2nd frame taking 17ms, and every 3rd one taking 16ms. This might
     * result in some non-determinism down the line, e.g. changing the code in level loading will change the number
     * of frames it takes to load a level, and this will shift the timing sequence for the actual game frames after
     * the level is loaded. Unlikely to really affect anything, but we'd rather not find out.
     *
     * @param frameTimeMs               Frame time to use inside the segment, in milliseconds.
     * @param rngType                   Random engine type to use inside the segment.
     */
    void restart(int frameTimeMs, RandomEngineType rngType);

    /**
     * Leaves deterministic mode if currently is in it, does nothing otherwise.
     */
    void finish();

    [[nodiscard]] bool isActive() const {
        return _oldRandomEngine != nullptr;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    virtual int64_t tickCount() const override;
    virtual void swapBuffers() override;
    virtual void removeNotify() override;

 private:
    int64_t _tickCount = 0;
    int _frameTimeMs = 0;
    std::unique_ptr<RandomEngine> _oldRandomEngine;
};
