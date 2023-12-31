#include "Timer.h"

#include "Engine/EngineGlobals.h"

#include "Io/KeyboardInputHandler.h"

Timer *pMiscTimer = new Timer;
Timer *pEventTimer;

//----- (00426317) --------------------------------------------------------
Duration Timer::platformTime() {
    Duration result = Duration::fromRealtimeMilliseconds(platform->tickCount());
    if (result < _lastFrameTime) _lastFrameTime = 0_ticks;
    return result;
}

//----- (004263B7) --------------------------------------------------------
void Timer::tick() {
    Duration new_time = platformTime();

    // TODO(captainurist): this magically works with EventTracer because of how Time() is written:
    // it sets uStartTime to zero if it's larger than current time. And TickCount() in EventTracer starts at zero.
    // This looks very fragile, but rethinking it would require diving into how timers work.
    _dt = new_time - _lastFrameTime;
    _lastFrameTime = new_time;

    if (_dt > 32_ticks)
        _dt = 32_ticks; // 32 is 250ms

    if (!_paused && !_turnBased)
        _time += _dt;
}

void Timer::setPaused(bool paused) {
    if (_paused == paused)
        return;

    _paused = paused;

    if (!_paused) {
        keyboardInputHandler->ResetKeys(); // TODO(captainurist): doesn't belong here.
        _lastFrameTime = platformTime();
    }
}

void Timer::setTurnBased(bool turnBased) {
    if (_turnBased == turnBased)
        return;

    _turnBased = turnBased;

    if (!_turnBased)
        _lastFrameTime = platformTime();
}
