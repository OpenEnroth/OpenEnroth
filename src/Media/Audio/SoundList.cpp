#include "SoundList.h"

#include "Utility/MapAccess.h"

#include "SoundInfo.h"

SoundList *pSoundList;

SoundInfo *SoundList::soundInfo(SoundId soundId) {
    return valuePtr(_mapSounds, soundId);
}
