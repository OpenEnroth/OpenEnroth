#include "TextureFrameTable.h"

#include <string>

#include "Engine/AssetsManager.h"

#include "Library/Logger/Logger.h"
#include "Utility/String/Ascii.h"

TextureFrameTable *pTextureFrameTable;

GraphicsImage *TextureFrame::GetTexture() {
    if (!this->tex) {
        this->tex = assets->getBitmap(this->name);
    }
    return this->tex;
}

int64_t TextureFrameTable::FindTextureByName(std::string_view Str2) {
    std::string name = ascii::toLower(Str2);

    for (size_t i = 0; i < textures.size(); ++i)
        if (textures[i].name == name)
            return i;
    return -1;
}

GraphicsImage *TextureFrameTable::GetFrameTexture(int64_t frameId, Duration offset) {
    if (frameId < 0 || frameId >= textures.size()) {
        logger->error("Failed to retreive OOB frameID '{}' from TextureFrameTable::GetFrameTexture", frameId);
        return nullptr;
    }

    Duration animationDuration = textures[frameId].animationDuration;
    if ((textures[frameId].flags & FRAME_HAS_MORE) && animationDuration) {
        offset = offset % animationDuration;
        while (offset >= textures[frameId].frameDuration) {
            offset -= textures[frameId].frameDuration;
            ++frameId;
        }
    }

    assert(frameId < textures.size() && "TextureFrameTable::GetFrameTexture animated frame OOB");
    return textures[frameId].GetTexture();
}

Duration TextureFrameTable::textureFrameAnimLength(int64_t frameID) {
    if (frameID < 0 || frameID >= textures.size()) {
        logger->error("Failed to retreive OOB frameID '{}' from TextureFrameTable::textureFrameAnimLength", frameID);
        return 1_ticks;
    }
    Duration result = textures[frameID].animationDuration;
    if ((textures[frameID].flags & FRAME_HAS_MORE) && result)
        return result;
    return 1_ticks;
}

Duration TextureFrameTable::textureFrameAnimTime(int64_t frameID) {
    if (frameID < 0 || frameID >= textures.size()) {
        logger->error("Failed to retreive OOB frameID '{}' from TextureFrameTable::textureFrameAnimTime", frameID);
        return 1_ticks;
    }
    return textures[frameID].frameDuration;
}
