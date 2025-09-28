#pragma once

#include <string_view>
#include <vector>

#include "Engine/Data/TextureFrameData.h"

struct TriBlob;
class GraphicsImage;

class TextureFrameTable {
 public:
    TextureFrameTable() = default;
    explicit TextureFrameTable(std::vector<TextureFrameData> frames);

    int animationId(std::string_view textureName);

    /**
     * @param animationId               Animation index in this table.
     * @return                          Total duration of the corresponding animation. Passed frame must be the first
     *                                  one in a sequence.
     */
    Duration animationLength(int animationId);

    /**
     * @param frameId                   Frame index in this table.
     * @return                          Dwell time of the frame.
     */
    Duration animationFrameLength(int frameId);

    GraphicsImage *animationFrame(int animationId, Duration frameTime);

    friend void deserialize(const TriBlob &src, TextureFrameTable *dst); // In TableSerialization.cpp.

 private:
    GraphicsImage *loadTexture(int frameId);

 private:
    std::vector<TextureFrameData> _frames;
    std::vector<GraphicsImage *> _textures;
};

extern TextureFrameTable *pTextureFrameTable;
