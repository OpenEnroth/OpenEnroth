#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Engine/Data/FrameEnums.h"
#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"

class GraphicsImage;

class TextureFrame {
 public:
    std::string name = "null";
    Duration frameDuration; // Duration of this frame.
    Duration animationDuration; // Total animation duration. Set only for the first frame in a sequence.
    FrameFlags flags;

    GraphicsImage *GetTexture();

 protected:
    GraphicsImage *tex = nullptr;
};

struct TextureFrameTable {
    GraphicsImage *GetFrameTexture(int64_t frameId, Duration offset);

    /**
     * @param frameID                   Texture index in this table.
     * @return                          Total duration of the corresponding animation. Passed frame must be the first
     *                                  one in a sequence.
     */
    Duration textureFrameAnimLength(int64_t frameID);

    /**
     * @param frameID                   Texture index in this table.
     * @return                          Dwell time of this texture.
     */
    Duration textureFrameAnimTime(int64_t frameID);

    int64_t FindTextureByName(std::string_view Str2);

    std::vector<TextureFrame> textures;
};

extern TextureFrameTable *pTextureFrameTable;
