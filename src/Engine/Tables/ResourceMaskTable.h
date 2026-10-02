#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Data/ResourceMask.h"

#include "Library/Json/JsonFwd.h"

#include "Utility/String/TransparentFunctors.h"

/**
 * Masks that override what the game data says about an image's transparency, keyed by lowercase resource name.
 */
struct ResourceMaskTable {
    std::unordered_map<std::string, ResourceMask, TransparentStringHash, TransparentStringEquals> bitmaps;
    std::unordered_map<std::string, ResourceMask, TransparentStringHash, TransparentStringEquals> icons;
};
MM_DECLARE_JSON_SERIALIZATION_FUNCTIONS(ResourceMaskTable)
