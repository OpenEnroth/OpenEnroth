#include "TalkAnimation.h"

#include "Engine/Random/Random.h"
#include "Engine/Tables/PortraitFrameTable.h"

static CharacterPortrait randomTalkPortrait() {
    return vrng->randomSample({PORTRAIT_TALK, PORTRAIT_MOUTH_OPEN_WIDE, PORTRAIT_MOUTH_OPEN_A, PORTRAIT_MOUTH_OPEN_O});
}

void TalkAnimation::init() {
    _currentPortrait = randomTalkPortrait();
    _currentTime = 0_ticks;
}

void TalkAnimation::update(Duration dt) {
    _currentTime += dt;

    while (true) {
        Duration currentDuration = pPortraitFrameTable->animationDuration(_currentPortrait);
        if (_currentTime < currentDuration)
            break;

        _currentTime -= currentDuration;
        _currentPortrait = randomTalkPortrait();
    }
}

int TalkAnimation::currentFrameIndex() const {
    return pPortraitFrameTable->animationFrameIndex(pPortraitFrameTable->animationId(_currentPortrait), _currentTime);
}
