#pragma once

#include <cstdint>

#include "Library/Snapshots/RawSnapshots.h"

#include "Duration.h"

struct RawTimer {
    bool bPaused = false;
    int bTackGameTime = 0;
    Duration lastFrameTime; // "Realtime" tick count, as Duration, at the last frame.
    Duration uTimeElapsed; // dt since last frame.
    Duration uTotalTimeElapsed; // Total time elapsed.
};

class Timer : private RawTimer {
    MM_DECLARE_RAW_PRIVATE_BASE(RawTimer)
 public:
    Timer() = default;

    void Update();
    void Pause();
    void Resume();
    void TrackGameTime();
    void StopGameTime();

    // TODO(captainurist): encapsulate.
    using RawTimer::bPaused;
    using RawTimer::bTackGameTime;
    using RawTimer::uTimeElapsed;
    using RawTimer::uTotalTimeElapsed;

 private:
    Duration Time();
};

// TODO(captainurist): pAnimTimer? Also, if we are being purists, this is not a Timer. It's not measuring Enroth time,
//                     it's measuring real time. So should operate with std::chrono primitives. Look at all the places
//                     where it's used and write proper docs here first, maybe I'm missing smth.
extern Timer *pMiscTimer;

// TODO(captainurist): pGameTimer?
extern Timer *pEventTimer;
