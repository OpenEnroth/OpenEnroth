#include "Engine/Time.h"

#include <memory>

#include "Io/KeyboardInputHandler.h"
#include "EngineGlobals.h"
#include "Platform/Platform.h"

Timer *pMiscTimer = new Timer;
Timer *pEventTimer;

//----- (00426317) --------------------------------------------------------
uint64_t Timer::Time() {
    int64_t ms = platform->tickCount();
    uint64_t v2 = 128 * ms / 1000;
    if (v2 < uStartTime) uStartTime = 0;
    return v2;
}

//----- (00426349) --------------------------------------------------------
void Timer::Pause() {
    if (!bPaused) {
        uStopTime = Time();
        bPaused = true;
    }
}

//----- (00426363) --------------------------------------------------------
void Timer::Resume() {
    if (bPaused) {
        keyboardInputHandler->ResetKeys();

        bPaused = 0;
        uStartTime = Time();
    }
}

//----- (00426386) --------------------------------------------------------
void Timer::TrackGameTime() {
    if (!bTackGameTime) {
        uGameTimeStart = Time();
        bTackGameTime = true;
    }
}

//----- (004263A0) --------------------------------------------------------
void Timer::StopGameTime() {
    if (bTackGameTime) {
        bTackGameTime = 0;
        uStartTime = Time();
    }
}

//----- (004263B7) --------------------------------------------------------
void Timer::Update() {
    // Timer *v1; // esi@1
    // unsigned int v2; // eax@2
    // signed int v3; // eax@3
    // char v4; // zf@5

    uint64_t new_time = Time();

    // TODO(captainurist): I had to comment the line below because it's now hooking into platform, and platform
    // code return the same tick count on every call when playing back an event trace.
    // while (new_time <= uStartTime) new_time = Time();

    // TODO(captainurist): this magically works with EventTracer because of how Time() is written:
    // it sets uStartTime to zero if it's larger than current time. And TickCount() in EventTracer starts at zero.
    // This looks very fragile, but rethinking it would require diving into how timers work.
    uTimeElapsed = new_time - uStartTime;
    uStartTime = new_time;

    if (uTimeElapsed > 32)
        uTimeElapsed = 32; // 32 is 250ms

    if (!bPaused && !bTackGameTime)
        uTotalTimeElapsed += uTimeElapsed;

    dt_fixpoint = (uTimeElapsed << 16) / 128;
}

//----- (00426402) --------------------------------------------------------
void Timer::Initialize() {
    uTotalTimeElapsed = 0;
    bReady = true;
}
