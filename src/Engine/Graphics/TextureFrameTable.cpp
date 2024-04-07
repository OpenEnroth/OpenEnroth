#include "TextureFrameTable.h"

#include "Engine/AssetsManager.h"

#include "Utility/String.h"

struct TextureFrameTable *pTextureFrameTable;

GraphicsImage *TextureFrame::GetTexture() {
    if (!this->tex) {
        this->tex = assets->getBitmap(this->name);
    }
    return this->tex;
}

int64_t TextureFrameTable::FindTextureByName(std::string_view Str2) {
    std::string name = toLower(Str2);

    for (size_t i = 0; i < textures.size(); ++i)
        if (textures[i].name == name)
            return i;
    return -1;
}

GraphicsImage *TextureFrameTable::GetFrameTexture(int frameId, Duration offset) {
    Duration animationDuration = textures[frameId].animationDuration;

    if ((textures[frameId].flags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && animationDuration) {
        offset = offset % animationDuration;
        while (offset >= textures[frameId].frameDuration) {
            offset -= textures[frameId].frameDuration;
            ++frameId;
        }
    }

    return textures[frameId].GetTexture();
}

Duration TextureFrameTable::textureFrameAnimLength(int frameID) {
    Duration result = textures[frameID].animationDuration;
    if ((textures[frameID].flags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && result)
        return result;
    return 1_ticks;
}

Duration TextureFrameTable::textureFrameAnimTime(int frameID) {
    return textures[frameID].frameDuration;
}
