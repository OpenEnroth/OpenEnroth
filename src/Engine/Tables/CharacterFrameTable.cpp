#include "CharacterFrameTable.h"

#include "Engine/Random/Random.h"


//----- (00494AED) --------------------------------------------------------
unsigned int PlayerFrameTable::GetFrameIdByPortrait(CharacterPortrait portrait) {
    for (size_t i = 0; i < this->pFrames.size(); i++) {
        if (this->pFrames[i].portrait == portrait) return i;
    }
    return 0;
}

Duration PlayerFrameTable::GetDurationByPortrait(CharacterPortrait portrait) {
    int index = GetFrameIdByPortrait(portrait);
    if (index == 0)
        return 0_ticks;
    return this->pFrames[index].uAnimLength;
}

//----- (00494B10) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_x(int uFramesetID, Duration gameTime) {
    if (this->pFrames[uFramesetID].uFlags & 1 && this->pFrames[uFramesetID].uAnimLength) {
        // Processing animated character expressions - e.g., PORTRAIT_YES & PORTRAIT_NO.
        Duration time = gameTime % this->pFrames[uFramesetID].uAnimLength;

        while (true) {
            Duration frameTime = this->pFrames[uFramesetID].uAnimTime;
            if (time < frameTime)
                break;
            time -= frameTime;
            ++uFramesetID;
            assert(this->pFrames[uFramesetID].portrait == PORTRAIT_INVALID); // Shouldn't jump into another expression.
        }
    }
    return &this->pFrames[uFramesetID];
}

//----- (00494B5E) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_y(int *pFramesetID, Duration *pAnimTime,
                                            Duration a4) {
    int v6;  // eax@2

    Duration v5 = a4 + *pAnimTime;
    if (v5 < this->pFrames[*pFramesetID].uAnimTime) {
        *pAnimTime = v5;
    } else {
        v6 = vrng->random(4) + 21;
        *pFramesetID = v6;
        *pAnimTime = v5 % this->pFrames[v6].uAnimTime;
    }
    return &this->pFrames[*pFramesetID];
}
