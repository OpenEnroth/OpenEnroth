#pragma once

#include <string>
#include <memory>

#include "Media/AudioDataSource.h"

#include "Utility/Memory/Blob.h"

#include "SoundEnums.h"

class SoundInfo {
 public:
    bool Is3D() { return uFlags & SOUND_FLAG_3D; }

 public:
    std::string sName;
    SoundType eType;
    SoundId uSoundID;
    SoundFlags uFlags;
    PAudioDataSource dataSource;
};
