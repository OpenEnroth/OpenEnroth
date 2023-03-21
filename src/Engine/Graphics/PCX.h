#pragma once

#include <cstdint>

#include "Image.h"

namespace PCX {
uint8_t *Decode(const void *pcx_data, size_t filesize, unsigned int *width,
    unsigned int *height, IMAGE_FORMAT *format, IMAGE_FORMAT requested_format);
/**
 * @param picture_data                  Pointer to the image data to encode.
 * @param width                         Size of image width to store.
 * @param height                        Size of image height to store.
 * @param[in,out] pcx_data              Reference to pointer - must be nullptr before call.
 *                                      Buffer is created in function and packed pcx file stored within.
 *                                      Must be freed by delete[] after use.
 * @param[out] packed_size              Size of the packed PCX image - must be 0 before call.
 */
void Encode32(const void *picture_data, const unsigned int width, const unsigned int height,
    uint8_t *&pcx_data, unsigned int &packed_size);
}  // namespace PCX
