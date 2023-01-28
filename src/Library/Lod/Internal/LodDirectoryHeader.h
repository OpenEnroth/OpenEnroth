#pragma once

#include <array>

#include <stdint.h>


#pragma pack(push, 1)
struct LodDirectoryHeader_Mm6 {
    inline LodDirectoryHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    std::array<uint8_t, 16> filename;
    uint32_t dataOffset;
    uint32_t dataSize;
    uint32_t dword_000018;
    uint16_t numFiles;
    uint16_t priority;
};
#pragma pack(pop)
