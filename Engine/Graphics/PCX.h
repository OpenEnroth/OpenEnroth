#pragma once

#include <cstdint>

namespace PCX {
bool IsValid(const void *pcx_data);
void GetSize(const void *pcx_data, unsigned int *width, unsigned int *height);
bool Decode(const void *pcx_data, uint16_t *pOutPixels, unsigned int *width,
            unsigned int *height);
void Encode16(void *picture_data, unsigned int width, unsigned int height,
              void *pcx_data, int max_buff_size, unsigned int *packed_size);
void Encode32(void *picture_data, unsigned int width, unsigned int height,
              void *pcx_data, int max_buff_size, unsigned int *packed_size);
}  // namespace PCX
