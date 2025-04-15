#pragma once

#include <cstdint>

#include "Duration.h"

struct Timer_MM7;

class Timer {
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

    friend void snapshot(const Timer &src, Timer_MM7 *dst); // In EntitySnapshots.cpp.
    friend void reconstruct(const Timer_MM7 &src, Timer *dst); // In EntitySnapshots.cpp.

 private:
    Duration platformTime();

 private:
    bool _paused = false;
    bool _turnBased = false;
    Duration _lastFrameTime; // "Realtime" tick count, as Duration, at the last frame.
    Duration _dt; // dt since last frame.
    Duration _time; // Total time elapsed.
};

// TODO(captainurist): pAnimTimer?
extern Timer *pMiscTimer;

// TODO(captainurist): pGameTimer?
extern Timer *pEventTimer;
