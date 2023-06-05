#pragma once

#include <cstdint>
#include <memory>

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace PCX {
/**
 * Decodes a PCX image from a `Blob`.
 *
 * @param data                          Compressed PCX image to decode.
 * @return                              Decoded `RgbaImage`.
 * @throws Exception                    On error.
 */
RgbaImage Decode(const Blob &data);

Blob Encode(RgbaImageView image);
}  // namespace PCX
