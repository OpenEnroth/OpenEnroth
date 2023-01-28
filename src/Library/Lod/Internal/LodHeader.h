#pragma once

#include <array>

#include <stdint.h>


#pragma pack(push, 1)
struct LodHeader_Mm6 {
    inline LodHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    std::array<uint8_t, 4> signature;
    std::array<uint8_t, 80> version;
    std::array<uint8_t, 80> description;
    uint32_t size;
    uint32_t dword_0000A8;
    uint32_t numDirectories;
    std::array<uint8_t, 80> _unused;  // never set/accessed so stores random memory garbage
};
#pragma pack(pop)
