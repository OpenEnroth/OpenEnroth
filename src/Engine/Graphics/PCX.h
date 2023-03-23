#pragma once

#include <cstdint>

#include "Image.h"

namespace PCX {
uint8_t *Decode(const void *pcx_data, size_t filesize, unsigned int *width,
    unsigned int *height, IMAGE_FORMAT *format, IMAGE_FORMAT requested_format);
/**
 * @param picture_data                  Pointer to the image data to encode.
 * @param width                         Final width of image to create.
 * @param height                        Final height of image to create.
 * @return                              Returns Blob containing packed pcx data and its size.
 */
Blob Encode(const void *picture_data, const unsigned int width, const unsigned int height);
}  // namespace PCX
