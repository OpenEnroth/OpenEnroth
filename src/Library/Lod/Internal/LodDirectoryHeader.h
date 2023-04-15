#pragma once

#include <array>
#include <cstring> // memset
#include <cstdint>


#pragma pack(push, 1)
struct LodDirectoryHeader_Mm6 {
    inline LodDirectoryHeader_Mm6() {
        memset(this, 0, sizeof(*this));
    }

    std::array<std::uint8_t, 16> filename;
    std::uint32_t dataOffset;
    std::uint32_t dataSize;
    std::uint32_t dword_000018;
    std::uint16_t numFiles;
    std::uint16_t priority;
};
#pragma pack(pop)
