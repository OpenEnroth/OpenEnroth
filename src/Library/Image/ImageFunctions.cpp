#include "ImageFunctions.h"
#include <span>
#include <string.h>
#include <cassert>
#include <array>
#include "Library/Color/Color.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

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

RgbaImage flipVertically(RgbaImageView image) {
    if (!image)
        return RgbaImage();

    RgbaImage result = RgbaImage::uninitialized(image.width(), image.height());
    for (size_t y = 0, h = image.height(); y < h; y++)
        memcpy(result[h - y - 1].data(), image[y].data(), image[y].size_bytes());
    return result;
}
