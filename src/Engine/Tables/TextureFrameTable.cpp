#include "TextureFrameTable.h"

#include <cassert>
#include <utility>
#include <vector>

#include "Engine/AssetsManager.h"

#include "Utility/String/Ascii.h"

TextureFrameTable *pTextureFrameTable;

TextureFrameTable::TextureFrameTable(std::vector<TextureFrameData> frames) : _frames(std::move(frames)) {
    _textures.resize(_frames.size());
}

int TextureFrameTable::animationId(std::string_view textureName) {
    for (size_t i = 0; i < _frames.size(); i++)
        if (ascii::noCaseEquals(textureName, this->_frames[i].textureName))
            return i;
    return -1;
}

Duration TextureFrameTable::animationLength(int animationId) {
    const TextureFrameData &data = _frames[animationId];
    assert(data.flags & FRAME_FIRST);
    return data.animationLength;
}

Duration TextureFrameTable::animationFrameLength(int frameId) {
    return _frames[frameId].frameLength;
}

GraphicsImage *TextureFrameTable::animationFrame(int animationId, Duration frameTime) {
    TextureFrameData &data = _frames[animationId];
    assert(data.flags & FRAME_FIRST);

    if (!(data.flags & FRAME_HAS_MORE))
        return loadTexture(animationId);

    assert(data.animationLength);
    Duration t = frameTime % data.animationLength;

    int i;
    for (i = animationId; t >= _frames[i].frameLength; i++)
        t -= _frames[i].frameLength;
    return loadTexture(i);
}

GraphicsImage *TextureFrameTable::loadTexture(int frameId) {
    assert(_textures.size() == _frames.size());

    if (!_textures[frameId])
        _textures[frameId] = assets->getBitmap(_frames[frameId].textureName);
    return _textures[frameId];
}
