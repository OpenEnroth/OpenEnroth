#include "ImageFunctions.h"

#include <cassert>

RgbaImage makeRgbaImage(GrayscaleImageView indexedImage, const Palette &palette) {
    if (!indexedImage)
        return RgbaImage();

    RgbaImage result = RgbaImage::uninitialized(indexedImage.width(), indexedImage.height());
    auto srcPixels = indexedImage.pixels();
    auto dstPixels = result.pixels();
    assert(srcPixels.size() == dstPixels.size());

    for (size_t i = 0, size = srcPixels.size(); i < size; i++)
        dstPixels[i] = palette.colors[srcPixels[i]];

    return result;
}
