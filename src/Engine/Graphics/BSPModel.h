#pragma once

#include <vector>

#include "Engine/Graphics/Indoor.h"

#include "Library/Geometry/BBox.h"

class BSPModel {
 public:
    int index = 0;
    int32_t field_40 = 0; // visibility flag TODO(pskelton): use for map tooltip checking or remove
    Vec3f position {};
    BBoxi boundingBox = {0, 0, 0, 0, 0, 0};
    Vec3f boundingCenter {};
    float boundingRadius = 0;

    std::vector<Vec3f> vertices;
    std::vector<BLVFace> faces;
    std::vector<BSPNode> nodes;
};
