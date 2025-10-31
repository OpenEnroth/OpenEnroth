#pragma once

#include <string_view>
#include <vector>

#include "Engine/Data/IconFrameData.h"
#include "Engine/Time/Duration.h"

class GraphicsImage;
class Blob;

class IconFrameTable {
 public:
    int animationId(std::string_view animationName) const; // By animation name.
    Duration animationLength(int animationId) const;
    GraphicsImage *animationFrame(int animationId, Duration frameTime);

    friend void deserialize(const Blob &src, IconFrameTable *dst); // In TableSerialization.cpp.

 private:
    GraphicsImage *loadTexture(int frameId);

 private:
    std::vector<IconFrameData> _frames;
    std::vector<GraphicsImage *> _textures;
};

extern IconFrameTable *pIconsFrameTable;
