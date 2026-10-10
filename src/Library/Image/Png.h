#pragma once

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace png {
RgbaImage decode(const Blob &data);

/**
 * @param data                          PNG image.
 * @return                              Size of the image, read from the header without decoding the pixels.
 * @throws Exception                    On error.
 */
Sizei decodeSize(const Blob &data);

Blob encode(RgbaImageView image);
Blob encode(GrayscaleImageView image);
bool detect(const Blob &data);
} // namespace png
