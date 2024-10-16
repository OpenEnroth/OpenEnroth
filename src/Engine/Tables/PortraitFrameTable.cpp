#include "PortraitFrameTable.h"

#include "Engine/Random/Random.h"

PortraitFrameTable *pPortraitFrameTable = nullptr;

//----- (00494AED) --------------------------------------------------------
int PortraitFrameTable::animationId(CharacterPortrait portrait) {
    for (size_t i = 0; i < this->pFrames.size(); i++)
        if (this->pFrames[i].portrait == portrait)
            return i;
    return 0;
}

Duration PortraitFrameTable::animationDuration(CharacterPortrait portrait) {
    int index = animationId(portrait);
    if (index == 0)
        return 0_ticks;
    return this->pFrames[index].animationLength;
}

//----- (00494B10) --------------------------------------------------------
int PortraitFrameTable::animationFrameIndex(int animationId, Duration frameTime) {
    if (this->pFrames[animationId].flags & FRAME_HAS_MORE && this->pFrames[animationId].animationLength) {
        // Processing animated character expressions - e.g., PORTRAIT_YES & PORTRAIT_NO.
        Duration time = frameTime % this->pFrames[animationId].animationLength;

        while (true) {
            Duration frameTime = this->pFrames[animationId].frameLength;
            if (time < frameTime)
                break;
            time -= frameTime;
            ++animationId;
            assert(this->pFrames[animationId].portrait == PORTRAIT_INVALID); // Shouldn't jump into another portrait.
        }
    }
    return pFrames[animationId].textureIndex;
}

//----- (00494B5E) --------------------------------------------------------
int PortraitFrameTable::talkFrameIndex(int *animationId, Duration *currentTime, Duration dt) {
    // TODO(captainurist): just move this code out of PortraitFrameTable.
    Duration updatedTime = *currentTime + dt;
    if (updatedTime < pFrames[*animationId].frameLength) {
        *currentTime = updatedTime;
    } else {
        int v6 = vrng->random(4) + 21; // 21 is index of PORTRAIT_TALK.
        *animationId = v6;
        // TODO(captainurist): Technically this is bugged, we need to redo this in the same fashion as it's done for
        //                     other animations, just with a random index on each iteration.
        *currentTime = updatedTime % this->pFrames[v6].frameLength;
    }
    return this->pFrames[*animationId].textureIndex;
}
