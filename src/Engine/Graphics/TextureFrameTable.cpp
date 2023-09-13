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

int64_t TextureFrameTable::FindTextureByName(const std::string &Str2) {
    std::string name = toLower(Str2);

    for (size_t i = 0; i < textures.size(); ++i)
        if (textures[i].name == name)
            return i;
    return -1;
}

GraphicsImage *TextureFrameTable::GetFrameTexture(int frameId, int time) {
    int animLength = textures[frameId].animLength;

    if ((textures[frameId].flags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && animLength != 0) {
        int step = (time >> 3) % animLength;
        while (textures[frameId].animTime < step) {
            step -= textures[frameId].animTime;
            ++frameId;
        }
    }

    return textures[frameId].GetTexture();
}

int TextureFrameTable::textureFrameAnimLength(int frameID) {
    int animLength = textures[frameID].animLength;
    if ((textures[frameID].flags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && animLength != 0) {
        return animLength;
    }
    return 1;
}

int TextureFrameTable::textureFrameAnimTime(int frameID) {
    return textures[frameID].animTime;
}
