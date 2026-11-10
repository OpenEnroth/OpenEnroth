#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Engine/Data/TextureFrameData.h"

class GraphicsImage;
class Blob;

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

    const std::shared_ptr<GraphicsImage> &animationFrame(int animationId, Duration frameTime);

    /**
     * Drops the textures of every frame. Frames load them again when `animationFrame()` next returns them.
     */
    void resetLoadedTextures();

    friend void deserialize(const Blob &src, TextureFrameTable *dst); // In TableSerialization.cpp.

 private:
    const std::shared_ptr<GraphicsImage> &loadTexture(int frameId);

 private:
    std::vector<TextureFrameData> _frames;
    std::vector<std::shared_ptr<GraphicsImage>> _textures;
};

extern TextureFrameTable *pTextureFrameTable;
