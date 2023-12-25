#include "Timer.h"

#include "Engine/EngineGlobals.h"

#include "Io/KeyboardInputHandler.h"

Timer *pMiscTimer = new Timer;
Timer *pEventTimer;

//----- (00426317) --------------------------------------------------------
Duration Timer::Time() {
    Duration v2 = Duration::fromRealtimeMilliseconds(platform->tickCount());
    if (v2 < lastFrameTime) lastFrameTime = 0_ticks;
    return v2;
}

//----- (00426349) --------------------------------------------------------
void Timer::Pause() {
    bPaused = true;
}

//----- (00426363) --------------------------------------------------------
void Timer::Resume() {
    if (bPaused) {
        keyboardInputHandler->ResetKeys();

        bPaused = false;
        lastFrameTime = Time();
    }
}

//----- (00426386) --------------------------------------------------------
void Timer::TrackGameTime() {
    if (!bTackGameTime) {
        bTackGameTime = true;
    }
}

//----- (004263A0) --------------------------------------------------------
void Timer::StopGameTime() {
    if (bTackGameTime) {
        bTackGameTime = false;
        lastFrameTime = Time();
    }
}

//----- (004263B7) --------------------------------------------------------
void Timer::Update() {
    // Timer *v1; // esi@1
    // unsigned int v2; // eax@2
    // signed int v3; // eax@3
    // char v4; // zf@5

    Duration new_time = Time();

    // TODO(captainurist): I had to comment the line below because it's now hooking into platform, and platform
    // code return the same tick count on every call when playing back an event trace.
    // while (new_time <= uStartTime) new_time = Time();

    // TODO(captainurist): this magically works with EventTracer because of how Time() is written:
    // it sets uStartTime to zero if it's larger than current time. And TickCount() in EventTracer starts at zero.
    // This looks very fragile, but rethinking it would require diving into how timers work.
    uTimeElapsed = new_time - lastFrameTime;
    lastFrameTime = new_time;

    if (uTimeElapsed > 32_ticks)
        uTimeElapsed = 32_ticks; // 32 is 250ms

    if (!bPaused && !bTackGameTime)
        uTotalTimeElapsed += uTimeElapsed;
}
