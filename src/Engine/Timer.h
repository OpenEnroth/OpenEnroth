#pragma once

#include <cstdint>

#include "Core/Time/Duration.h"

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

// Timer for UI animations. Keeps ticking while the game is paused.
extern Timer *animTimer;

// Timer for the game simulation - events, AI, physics. Also drives in-world visuals, which freeze together
// with the simulation when the game pauses.
extern Timer *gameTimer;
