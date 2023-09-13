#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
    inline TextureFrame() : tex(nullptr) {}

    std::string name = "null";
    int16_t animTime = 0; // Frame time, in 1/16ths of a real-time second.
    int16_t animLength = 0; // Total animation length, in 1/16ths of a real-time second. Set only for the first frame in a sequence.
    TextureFrameFlags flags = 0;

    GraphicsImage *GetTexture();

 protected:
    GraphicsImage *tex;
};

struct TextureFrameTable {
    GraphicsImage *GetFrameTexture(int frameId, int time);
    /**
    * @param   frameID        TextureFrameTable index
    * @return                 Total length of texture animation
    */
    int textureFrameAnimLength(int frameID);
    /**
    * @param   frameID        TextureFrameTable index
    * @return                 Dwell time of this texture
    */
    int textureFrameAnimTime(int frameID);
    int64_t FindTextureByName(const std::string &Str2);

    std::vector<TextureFrame> textures;
};

extern TextureFrameTable *pTextureFrameTable;
