#pragma once

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace pcx {
/**
 * Decodes a PCX image from a `Blob`.
 *
 * @param data                          Compressed PCX image to decode.
 * @return                              Decoded `RgbaImage`.
 * @throws Exception                    On error.
 */
RgbaImage decode(const Blob &data);

/**
 * @param data                          Compressed PCX image.
 * @return                              Size of the image, read from the header without decoding the pixels.
 * @throws Exception                    On error.
 */
Sizei decodeSize(const Blob &data);

Blob encode(RgbaImageView image);

/**
 * @param data                          Compressed PCX image data.
 * @return                              Whether the data signature matches the PCX format.
 */
bool detect(const Blob &data);
}  // namespace pcx
