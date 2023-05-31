#pragma once

#include <cstdint>
#include <memory>

#include "Image.h"
#include "Utility/Memory/Blob.h"

namespace PCX {
std::unique_ptr<Color[]> Decode(const Blob &data, size_t *width, size_t *height);
/**
 * @param data                          Pointer to the image data to encode.
 * @param width                         Final width of image to create.
 * @param height                        Final height of image to create.
 * @return                              Returns Blob containing packed pcx data and its size.
 */
Blob Encode(const Color *data, size_t width, size_t height);
}  // namespace PCX
