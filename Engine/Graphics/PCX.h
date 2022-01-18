#pragma once

#include <cstdint>

#include "Image.h"

namespace PCX {
uint8_t *Decode(const void *pcx_data, size_t filesize, unsigned int *width,
    unsigned int *height, IMAGE_FORMAT *format, IMAGE_FORMAT requested_format);
void Encode16(const void *picture_data, unsigned int width, unsigned int height,
              void *pcx_data, int max_buff_size, unsigned int *packed_size);
void Encode32(const void *picture_data, unsigned int width, unsigned int height,
              void *pcx_data, int max_buff_size, unsigned int *packed_size);
}  // namespace PCX
