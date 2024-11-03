#include "IconFrameTable.h"

#include <cassert>

#include "Engine/AssetsManager.h"

#include "Utility/String/Ascii.h"

IconFrameTable *pIconsFrameTable = nullptr;

int IconFrameTable::animationId(std::string_view animationName) const {
    for (size_t i = 0; i < frames.size(); i++)
        if (ascii::noCaseEquals(animationName, this->frames[i].animationName))
            return i;
    return -1;
}

Duration IconFrameTable::animationLength(int animationId) const {
    const IconFrameData &icon = frames[animationId];
    assert(icon.flags & FRAME_FIRST);
    return icon.animationLength;
}

GraphicsImage *IconFrameTable::animationFrame(int animationId, Duration frameTime) {
    IconFrameData &icon = frames[animationId];
    assert(icon.flags & FRAME_FIRST);

    if (!(icon.flags & FRAME_HAS_MORE))
        return loadTexture(animationId);

    assert(icon.animationLength);
    Duration t = frameTime % icon.animationLength;

    int i;
    for (i = animationId; t >= frames[i].frameLength; i++)
        t -= frames[i].frameLength;
    return loadTexture(i);
}

GraphicsImage *IconFrameTable::loadTexture(int frameId) {
    assert(textures.size() == frames.size());

    if (!textures[frameId])
        textures[frameId] = assets->getImage_ColorKey(frames[frameId].textureName);
    return textures[frameId];
}
