#pragma once

#include <string>
#include <memory>

#include "Media/AudioDataSource.h"

#include "Utility/Memory/Blob.h"

#include "SoundEnums.h"

class SoundInfo {
 public:
    bool Is3D() { return flags & SOUND_FLAG_3D; }

 public:
    std::string name;
    SoundType type;
    SoundId soundId;
    SoundFlags flags;
    PAudioDataSource dataSource;
};
