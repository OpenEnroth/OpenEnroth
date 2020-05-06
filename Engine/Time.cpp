#include "Engine/Time.h"

#include <chrono>

#include "Io/KeyboardInputHandler.h"

Timer *pMiscTimer = new Timer;
Timer *pEventTimer;

//----- (00426317) --------------------------------------------------------
uint64_t Timer::Time() {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    uint64_t v2 = TIME_QUANT * ms.count() / 1000;
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
    while (new_time <= uStartTime) new_time = Time();

    uTimeElapsed = new_time - uStartTime;
    uStartTime = new_time;

    if (uTimeElapsed > 32) uTimeElapsed = 32;

    if (!bPaused && !bTackGameTime) uTotalGameTimeElapsed += uTimeElapsed;
    dt_in_some_format = (uTimeElapsed << 16) / 128;
}

//----- (00426402) --------------------------------------------------------
void Timer::Initialize() {
    uTotalGameTimeElapsed = 0;
    bReady = true;
}
