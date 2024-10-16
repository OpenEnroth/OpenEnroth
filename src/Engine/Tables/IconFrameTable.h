#pragma once

#include <string_view>
#include <vector>

#include "Engine/Data/IconFrameData.h"
#include "Engine/Time/Duration.h"

#include "Library/Snapshots/RawSnapshots.h"

class GraphicsImage;

struct RawIconFrameTable {
    std::vector<IconFrameData> frames;
    std::vector<GraphicsImage *> textures;
};

class IconFrameTable : private RawIconFrameTable {
    MM_DECLARE_RAW_PRIVATE_BASE(RawIconFrameTable)
 public:
    int animationId(std::string_view animationName) const; // By animation name.
    Duration animationLength(int animationId) const;
    GraphicsImage *animationFrame(int animationId, Duration frameTime);

 private:
    GraphicsImage *loadTexture(int frameId);
};

extern IconFrameTable *pIconsFrameTable;
