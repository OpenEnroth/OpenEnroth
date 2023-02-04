#include "FrameLimiter.h"

#include <chrono>

static int64_t nowNs() {
    // We're going through std::chrono here and not through Platform because we need actual clock time, not
    // "simulation time".
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

FrameLimiter::FrameLimiter() {
    reset();
}

void FrameLimiter::reset() {
    _lastFrameTimeNs = nowNs();
}

void FrameLimiter::tick(int targetFps) {
    int64_t targetDeltaNs = 1'000'000'000 / targetFps;

    int64_t currentTimeNs;
    do {
        currentTimeNs = nowNs();
    } while (currentTimeNs - _lastFrameTimeNs < targetDeltaNs);

    _lastFrameTimeNs = currentTimeNs;
}
