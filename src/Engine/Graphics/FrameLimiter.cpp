#include "FrameLimiter.h"

#include <chrono>
#include <thread>

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
    // This code is somewhat inspired by https://blog.bearcats.nl/perfect-sleep-function/, read the article for
    // details on what we're doing here.
    constexpr int64_t spinTimeNs =  1'500'000; // 1.5ms.
    int64_t targetTimeNs = _lastFrameTimeNs + 1'000'000'000 / targetFps;
    int64_t currentTimeNs = nowNs();
    int64_t waitTimeNs = targetTimeNs - currentTimeNs;

    // Try to sleep first.
    if (waitTimeNs > spinTimeNs) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(waitTimeNs - spinTimeNs));
        currentTimeNs = nowNs();
    }

    // Then spin.
    while (currentTimeNs < targetTimeNs)
        currentTimeNs = nowNs();

    _lastFrameTimeNs = currentTimeNs;
}
