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
    int64_t _targetDeltaNs = 1'000'000'000 / targetFps;
    int64_t _spinTimeNs = _targetDeltaNs / SPIN_TIME_FRACTION;
    int64_t _currentTimeNs = nowNs();
    int64_t diff = _currentTimeNs - _lastFrameTimeNs + _spinTimeNs;

    // sleep
    if (diff < _targetDeltaNs)
    {
        int64_t diff2 = _targetDeltaNs - diff;
        std::this_thread::sleep_for(std::chrono::nanoseconds(diff2));
        _currentTimeNs += diff2;
    }

    // spin
    while (_currentTimeNs - _lastFrameTimeNs < _targetDeltaNs)
        _currentTimeNs = nowNs();

    _lastFrameTimeNs = _currentTimeNs;
}
