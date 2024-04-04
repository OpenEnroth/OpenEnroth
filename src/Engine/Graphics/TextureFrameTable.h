#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Flags.h"

class GraphicsImage;

enum class TextureFrameFlag {
    TEXTURE_FRAME_TABLE_MORE_FRAMES = 0x1,
    TEXTURE_FRAME_TABLE_FIRST = 0x2,
};
using enum TextureFrameFlag;
MM_DECLARE_FLAGS(TextureFrameFlags, TextureFrameFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(TextureFrameFlags)

class TextureFrame {
 public:
    std::string name = "null";
    Duration frameDuration; // Duration of this frame.
    Duration animationDuration; // Total animation duration. Set only for the first frame in a sequence.
    TextureFrameFlags flags = 0;

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

    int64_t FindTextureByName(const std::string &Str2);

    std::vector<TextureFrame> textures;
};

extern TextureFrameTable *pTextureFrameTable;
