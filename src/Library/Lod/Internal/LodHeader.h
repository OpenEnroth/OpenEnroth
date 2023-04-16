#pragma once

#include <array>
#include <cstring> // memset
#include <cstdint>


#pragma pack(push, 1)
struct LodHeader_Mm6 {
    inline LodHeader_Mm6() {
        memset(this, 0, sizeof(*this));
    }

    std::array<std::uint8_t, 4> signature;
    std::array<std::uint8_t, 80> version;
    std::array<std::uint8_t, 80> description;
    std::uint32_t size;
    std::uint32_t dword_0000A8;
    std::uint32_t numDirectories;
    std::array<std::uint8_t, 80> _unused;  // never set/accessed so stores random memory garbage
};
#pragma pack(pop)
