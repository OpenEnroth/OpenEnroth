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
    SOUND_TYPE eType;
    uint32_t uSoundID;
    SOUND_FLAGS uFlags;
    std::shared_ptr<Blob> buffer;
    PAudioDataSource dataSource;
};
