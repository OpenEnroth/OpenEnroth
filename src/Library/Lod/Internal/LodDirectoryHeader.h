#pragma once

#include <stdint.h>


#pragma pack(push, 1)
struct LodDirectoryHeader_Mm6 {
    inline LodDirectoryHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    uint8_t filename[16];
    uint32_t data_offset;
    uint32_t uDataSize;
    uint32_t dword_000018;
    uint16_t num_items;
    uint16_t priority;
};
#pragma pack(pop)
