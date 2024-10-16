#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

#include "Engine/Data/FrameEnums.h"
#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"

class GraphicsImage;

struct IconFrameData {
    std::string pTextureName;
    FrameFlags uFlags;
    std::string anim_name;
    Duration anim_length;
    Duration anim_time;
};

class IconFrameTable {
public:
    int animationId(std::string_view animationName) const; // By animation name.
    Duration animationLength(int animationId) const;
    GraphicsImage *animationFrame(int animationId, Duration frameTime);

private:
    GraphicsImage *loadTexture(int frameId);

public: // TODO(captainurist): make private
    std::vector<IconFrameData> frames;
    std::vector<GraphicsImage *> textures;
};

extern IconFrameTable *pIconsFrameTable;
