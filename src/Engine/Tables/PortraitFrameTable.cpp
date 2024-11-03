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
