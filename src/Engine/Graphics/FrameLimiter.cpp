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
  int64_t targetDeltaNs = 1'000'000'000 / 120;

  int64_t currentNs = nowNs();
  int64_t diff = currentNs - _lastFrameTimeNs;
  if (diff < targetDeltaNs)
    std::this_thread::sleep_for(std::chrono::nanoseconds(targetDeltaNs - diff));

  _lastFrameTimeNs = currentNs + (targetDeltaNs - diff);
}
