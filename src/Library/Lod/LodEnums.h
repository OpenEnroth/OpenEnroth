#pragma once

#include "Library/Serialization/SerializationFwd.h"

enum class LodVersion {
    LOD_VERSION_MM6,
    LOD_VERSION_MM6_GAME,
    LOD_VERSION_MM7,
    LOD_VERSION_MM8,
};
using enum LodVersion;
MM_DECLARE_SERIALIZATION_FUNCTIONS(LodVersion)
