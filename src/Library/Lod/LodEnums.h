#pragma once

#include "Library/Serialization/SerializationFwd.h"

#include "Utility/Flags.h"

enum class LodVersion {
    LOD_VERSION_MM6,
    LOD_VERSION_MM6_GAME,
    LOD_VERSION_MM7,
    LOD_VERSION_MM8,
};
using enum LodVersion;
MM_DECLARE_SERIALIZATION_FUNCTIONS(LodVersion)

enum class LodOpenFlag {
    LOD_ALLOW_DUPLICATES = 0x1, // Allow duplicate entries, read only the 1st one.
};
using enum LodOpenFlag;
MM_DECLARE_FLAGS(LodOpenFlags, LodOpenFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(LodOpenFlags)
