#include "CharacterFrameTable.h"

#include "Library/Random/Random.h"


//----- (00494AED) --------------------------------------------------------
unsigned int PlayerFrameTable::GetFrameIdByExpression(CharacterExpressionID expression) {
    for (size_t i = 0; i < this->pFrames.size(); i++) {
        if (this->pFrames[i].expression == expression) return i;
    }
    return 0;
}

int PlayerFrameTable::GetDurationByExpression(CharacterExpressionID expression) {
    int index = GetFrameIdByExpression(expression);
    if (index == 0)
        return 0;
    return this->pFrames[index].uAnimLength * 8;
}

//----- (00494B10) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_x(int uFramesetID, int gameTime) {
    if (this->pFrames[uFramesetID].uFlags & 1 && this->pFrames[uFramesetID].uAnimLength != 0) {
        // Processing animated character expressions - e.g., CHARACTER_EXPRESSION_YES & CHARACTER_EXPRESSION_NO.
        int time = (gameTime >> 3) % this->pFrames[uFramesetID].uAnimLength;

        while (true) {
            int frameTime = this->pFrames[uFramesetID].uAnimTime;
            if (time < frameTime)
                break;
            time -= frameTime;
            ++uFramesetID;
            assert(this->pFrames[uFramesetID].expression == CHARACTER_EXPRESSION_INVALID); // Shouldn't jump into another expression.
        }
    }
    return &this->pFrames[uFramesetID];
}

//----- (00494B5E) --------------------------------------------------------
PlayerFrame *PlayerFrameTable::GetFrameBy_y(int *pFramesetID, int *pAnimTime,
                                            int a4) {
    int v5;  // esi@1
    int v6;  // eax@2

    v5 = a4 + *pAnimTime;
    if (v5 < 8 * this->pFrames[*pFramesetID].uAnimTime) {
        *pAnimTime = v5;
    } else {
        v6 = vrng->random(4) + 21;
        *pFramesetID = v6;
        *pAnimTime = 8 * v5 % this->pFrames[v6].uAnimTime;
    }
    return &this->pFrames[*pFramesetID];
}
