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

void TextureFrameTable::LoadAnimationSequenceAndPalettes(int uFrameID) {
    if (uFrameID <= textures.size() && uFrameID >= 0) {
        for (unsigned int i = uFrameID;; ++i) {
            // this->pTextures[i].uTextureID =
            // pBitmaps_LOD->LoadTexture(this->pTextures[i].pTextureName,
            // TEXTURE_DEFAULT);

            // if (this->pTextures[i].uTextureID != -1)
            //    pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id2
            //    =
            //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id1);

            if (textures[i].uFlags & TEXTURE_FRAME_TABLE_MORE_FRAMES) break;
        }
    }
    return;
}

int64_t TextureFrameTable::FindTextureByName(const std::string &Str2) {
    std::string name = toLower(Str2);

    for (size_t i = 0; i < textures.size(); ++i)
        if (textures[i].name == name)
            return i;
    return -1;
}

GraphicsImage *TextureFrameTable::GetFrameTexture(int frameId, int time) {
    int animLength = textures[frameId].uAnimLength;

    if ((textures[frameId].uFlags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && animLength != 0) {
        int step = (time >> 3) % animLength;
        while (textures[frameId].uAnimTime < step) {
            step -= textures[frameId].uAnimTime;
            ++frameId;
        }
    }

    return textures[frameId].GetTexture();
}

int TextureFrameTable::textureFrameAnimLength(int frameID) {
    int animLength = textures[frameID].uAnimLength;
    if ((textures[frameID].uFlags & TEXTURE_FRAME_TABLE_MORE_FRAMES) && animLength != 0) {
        return animLength;
    }
    return 1;
}

int TextureFrameTable::textureFrameAnimTime(int frameID) {
    return textures[frameID].uAnimTime;
}
