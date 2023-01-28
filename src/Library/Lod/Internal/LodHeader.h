#pragma once

#include <stdint.h>


#pragma pack(push, 1)
struct LodHeader_Mm6 {
    inline LodHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    uint8_t signature[4];
    uint8_t version[80];
    uint8_t description[80];
    uint32_t size;
    uint32_t dword_0000A8;
    uint32_t num_directories;
    uint8_t array_0000B0[80];  // never set/accessed so stores random memory garbage
};
#pragma pack(pop)
