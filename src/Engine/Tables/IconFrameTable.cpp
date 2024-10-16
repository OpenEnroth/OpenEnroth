#include "IconFrameTable.h"

#include <cassert>

#include "Engine/AssetsManager.h"

#include "Utility/String/Ascii.h"

IconFrameTable *pIconsFrameTable = nullptr;

int IconFrameTable::animationId(std::string_view animationName) const {
    for (size_t i = 0; i < frames.size(); i++)
        if (ascii::noCaseEquals(animationName, this->frames[i].anim_name))
            return i;
    return -1;
}

Duration IconFrameTable::animationLength(int animationId) const {
    const IconFrameData &icon = frames[animationId];
    assert(icon.uFlags & FRAME_FIRST);
    return icon.anim_length;
}

GraphicsImage *IconFrameTable::animationFrame(int animationId, Duration frameTime) {
    IconFrameData &icon = frames[animationId];
    assert(icon.uFlags & FRAME_FIRST);

    if (!(icon.uFlags & FRAME_HAS_MORE))
        return loadTexture(animationId);

    assert(icon.anim_length);
    Duration t = frameTime % icon.anim_length;

    int i;
    for (i = animationId; t >= frames[i].anim_time; i++)
        t -= frames[i].anim_time;
    return loadTexture(i);
}

GraphicsImage *IconFrameTable::loadTexture(int frameId) {
    assert(frameId < frames.size());

    if (textures.size() < frames.size())
        textures.resize(frames.size());

    if (!textures[frameId])
        textures[frameId] = assets->getImage_ColorKey(frames[frameId].pTextureName);

    return textures[frameId];
}
