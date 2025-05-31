#pragma once

#include <cstdint>

class FrameLimiter {
 public:
    FrameLimiter();

    void reset();

    void tick(int targetFps);

 private:
    int64_t _lastFrameTimeNs = 0;

    #define SPIN_TIME_FRACTION 10
};
