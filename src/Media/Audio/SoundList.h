#pragma once

#include <unordered_map>

#include "SoundInfo.h"

#include "Library/Snapshots/RawSnapshots.h"

struct RawSoundList {
    std::unordered_map<SoundId, SoundInfo> _mapSounds;
};

class SoundList : private RawSoundList {
    MM_DECLARE_RAW_PRIVATE_BASE(RawSoundList)
 public:
    SoundInfo *soundInfo(SoundId soundId); // TODO(captainurist): should be const
};

extern SoundList *pSoundList;
