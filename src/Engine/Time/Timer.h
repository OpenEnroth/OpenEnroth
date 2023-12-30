#pragma once

#include <cstdint>

#include "Library/Snapshots/RawSnapshots.h"

#include "Duration.h"

struct RawTimer {
    bool _paused = false;
    bool _turnBased = false;
    Duration _lastFrameTime; // "Realtime" tick count, as Duration, at the last frame.
    Duration _dt; // dt since last frame.
    Duration _time; // Total time elapsed.
};

class Timer : private RawTimer {
    MM_DECLARE_RAW_PRIVATE_BASE(RawTimer)
 public: // NOLINT: Linter, why???
    Timer() = default;

    void tick();

    bool isPaused() const {
        return _paused;
    }

    void setPaused(bool paused);

    // TODO(captainurist): isTurnBased is just a synonym for 'really-totally-paused'. Doesn't belong here, move out.

    bool isTurnBased() const {
        return _turnBased;
    }

    void setTurnBased(bool turnBased);

    Duration dt() const {
        return _dt;
    }

    Duration time() const {
        return _time;
    }

 private:
    Duration platformTime();
};

// TODO(captainurist): pAnimTimer?
extern Timer *pMiscTimer;

// TODO(captainurist): pGameTimer?
extern Timer *pEventTimer;
