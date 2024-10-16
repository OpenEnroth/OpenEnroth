#include "IconFrameTable.h"

#include <cassert>

#include "Engine/AssetsManager.h"

#include "Utility/String/Ascii.h"

IconFrameTable *pIconsFrameTable = nullptr;

GraphicsImage *Icon::GetTexture() {
    if (!this->img) {
        this->img = assets->getImage_ColorKey(this->pTextureName);
    }

    return this->img;
}

int IconFrameTable::animationId(std::string_view animationName) const {
    for (size_t i = 0; i < pIcons.size(); i++)
        if (ascii::noCaseEquals(animationName, this->pIcons[i].GetAnimationName()))
            return i;
    return -1;
}

Duration IconFrameTable::animationLength(int animationId) const {
    const Icon &icon = pIcons[animationId];
    assert(icon.uFlags & FRAME_FIRST);
    return icon.GetAnimLength();
}

GraphicsImage *IconFrameTable::animationFrame(int animationId, Duration frameTime) {
    Icon &icon = pIcons[animationId];
    assert(icon.uFlags & FRAME_FIRST);

    if (!(icon.uFlags & FRAME_HAS_MORE))
        return icon.GetTexture();

    assert(icon.GetAnimLength());
    Duration t = frameTime % icon.GetAnimLength();

    int i;
    for (i = animationId; t >= pIcons[i].GetAnimTime(); i++)
        t -= pIcons[i].GetAnimTime();
    return pIcons[i].GetTexture();
}
