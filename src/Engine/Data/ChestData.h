#pragma once

#include <string>

#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"

struct ChestData {
    std::string textureName; // Chest texture (in icons.lod).
    Sizei size; // Size in inventory cells.
    Pointi inventoryOffset; // Offset of the inventory matrix inside the chest texture.
};
