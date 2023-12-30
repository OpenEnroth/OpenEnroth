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
    // Timer *v1; // esi@1
    // unsigned int v2; // eax@2
    // signed int v3; // eax@3
    // char v4; // zf@5

    Duration new_time = platformTime();

    // TODO(captainurist): I had to comment the line below because it's now hooking into platform, and platform
    // code return the same tick count on every call when playing back an event trace.
    // while (new_time <= uStartTime) new_time = Time();

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
