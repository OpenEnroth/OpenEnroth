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

 private:
    uint64_t Time();
};

// TODO(captainurist): pAnimTimer? Also, if we are being purists, this is not a Timer. It's not measuring Enroth time,
//                     it's measuring real time. So should operate with std::chrono primitives. Look at all the places
//                     where it's used and write proper docs here first, maybe I'm missing smth.
extern Timer *pMiscTimer;

// TODO(captainurist): pGameTimer?
extern Timer *pEventTimer;
