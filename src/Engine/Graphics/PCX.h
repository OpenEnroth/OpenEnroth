#pragma once

#include <cstdint>
#include <memory>

#include "Image.h"

namespace PCX {
std::unique_ptr<uint8_t[]> Decode(const void *data, size_t size, size_t *width,
                                  size_t *height, IMAGE_FORMAT *format, IMAGE_FORMAT requestedFormat);
/**
 * @param picture_data                  Pointer to the image data to encode.
 * @param width                         Final width of image to create.
 * @param height                        Final height of image to create.
 * @return                              Returns Blob containing packed pcx data and its size.
 */
Blob Encode(const void *picture_data, size_t width, size_t height);
}  // namespace PCX
