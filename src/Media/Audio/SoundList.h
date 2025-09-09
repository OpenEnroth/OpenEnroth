#pragma once

#include <unordered_map>

#include "SoundInfo.h"

struct TriBlob;

class SoundList {
 public:
    SoundInfo *soundInfo(SoundId soundId); // TODO(captainurist): should be const

    friend void deserialize(const TriBlob &src, SoundList *dst); // In TableSerialization.cpp.

 private:
    std::unordered_map<SoundId, SoundInfo> _mapSounds;
};

extern SoundList *pSoundList;
