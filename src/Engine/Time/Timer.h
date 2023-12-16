#pragma once

#include <cstdint>

#include "Duration.h"

class Timer {
 public:
    Timer() = default;

    void Update();
    void Pause();
    void Resume();
    void TrackGameTime();
    void StopGameTime();

    bool bPaused = false;
    int bTackGameTime = 0;
    unsigned int uStartTime = 0; // Last frame time, in real time ticks (128 ticks is 1 real time second).
    unsigned int uStopTime = 0;
    int uGameTimeStart = 0;
    Duration uTimeElapsed; // dt since last frame.
    Duration uTotalTimeElapsed; // Total time elapsed.

    // Real time intervals in timer ticks.
    static const unsigned int Second = 128;
    static const unsigned int Minute = 60 * Second;
    static const unsigned int Hour = 60 * Minute;
    static const unsigned int Day = 24 * Hour;
    static const unsigned int Week = 7 * Day;
    static const unsigned int Month = 4 * Week;
    static const unsigned int Year = 12 * Month;

 private:
    uint64_t Time();
};

extern Timer *pMiscTimer;
extern Timer *pEventTimer;
