#pragma once

#include "Library/Color/Color.h"
#include "Library/Json/JsonFwd.h"
#include "Library/Serialization/SerializationFwd.h"

/**
 * How the transparent pixels of a game image are picked.
 */
enum class MaskMode {
    MASK_DEFAULT,   // Palette entry #0 is transparent if the image's `zeroIsTransparent` flag says so.
    MASK_NONE,      // Opaque, whatever the flag says.
    MASK_ZERO,      // Palette entry #0 is transparent, whatever the flag says.
    MASK_COLOR,     // Pixels of the mask color are transparent.
};
using enum MaskMode;
MM_DECLARE_SERIALIZATION_FUNCTIONS(MaskMode)

struct ResourceMask {
    MaskMode mode = MASK_DEFAULT;
    Color color; // Only used with `MASK_COLOR`.
};
MM_DECLARE_SERIALIZATION_FUNCTIONS(ResourceMask)
MM_DECLARE_JSON_SERIALIZATION_FUNCTIONS(ResourceMask)
