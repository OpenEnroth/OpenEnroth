#include "Timer.h"

#include <algorithm>

#include "Engine/EngineGlobals.h"

#include "Io/KeyboardInputHandler.h"

Timer *pMiscTimer = new Timer;
Timer *pEventTimer;

//----- (00426317) --------------------------------------------------------
Duration Timer::platformTime() {
    return Duration::fromRealtimeMilliseconds(platform->tickCount());
}

//----- (004263B7) --------------------------------------------------------
void Timer::tick() {
    Duration newTime = platformTime();

    // TODO(captainurist): This is needed because we roll back time in tests. We're dancing around with 32_ticks to
    //                     maintain trace compatibility. Think how to do this better.
    if (newTime < _lastFrameTime)
        _lastFrameTime = std::max(0_ticks, newTime - 32_ticks);

    _dt = newTime - _lastFrameTime;
    _lastFrameTime = newTime;

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
