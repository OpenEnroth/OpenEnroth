#include "SoundList.h"

#include <vector>

#include "Engine/Snapshots/EntitySnapshots.h"

#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/MapAccess.h"

#include "SoundInfo.h"

SoundList *pSoundList;

void SoundList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    std::vector<SoundInfo> sounds;

    if (data_mm6)
        deserialize(data_mm6, &sounds, tags::append, tags::via<SoundInfo_MM6>);
    if (data_mm7)
        deserialize(data_mm7, &sounds, tags::append, tags::via<SoundInfo_MM7>);
    if (data_mm8)
        deserialize(data_mm8, &sounds, tags::append, tags::via<SoundInfo_MM7>);

    assert(!sounds.empty());

    for (const SoundInfo &sound : sounds)
        _mapSounds[sound.uSoundID] = sound;
}

SoundInfo *SoundList::soundInfo(SoundID soundId) {
    return valuePtr(_mapSounds, soundId);
}
