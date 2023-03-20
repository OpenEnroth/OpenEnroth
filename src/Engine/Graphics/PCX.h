#pragma once

#include <cstdint>

#include "Image.h"

namespace PCX {
uint8_t *Decode(const void *pcx_data, size_t filesize, unsigned int *width,
    unsigned int *height, IMAGE_FORMAT *format, IMAGE_FORMAT requested_format);
void Encode32(const void *picture_data, const unsigned int width, const unsigned int height,
    uint8_t *&pcx_data, unsigned int &packed_size);
}  // namespace PCX
