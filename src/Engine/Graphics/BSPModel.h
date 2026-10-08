#pragma once

#include <vector>

#include "Engine/Graphics/Indoor.h"

#include "Library/Geometry/BBox.h"

class BSPModel {
 public:
    int index = 0;
    Vec3f position {};
    BBoxi boundingBox = {0, 0, 0, 0, 0, 0};
    Vec3f boundingCenter {};
    float boundingRadius = 0;

    std::vector<int> faces; // Indices into OutdoorLocation::faces.
    std::vector<BSPNode> nodes;
};
