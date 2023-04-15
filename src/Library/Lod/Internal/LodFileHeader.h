#pragma once

#include <array>
#include <cstring> // memset
#include <cstdint>


#pragma pack(push, 1)
struct LodFileHeader_Mm6 {
    inline LodFileHeader_Mm6() {
        memset(this, 0, sizeof(*this));
    }

    std::array<std::uint8_t, 16> name;
    std::uint32_t dataOffset;
    std::uint32_t size;
    std::uint32_t dword_000018;
    std::uint16_t numItems;
    std::uint16_t priority;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileHeader_Mm8 {
    inline LodFileHeader_Mm8() {
        memset(this, 0, sizeof(*this));
    }

    std::array<std::uint8_t, 16> name;
    std::int32_t unk_0;
    std::int32_t unk_1;
    std::int32_t unk_2;
    std::int32_t unk_3;
    std::int32_t unk_4;
    std::int32_t unk_5;
    std::int32_t unk_6;
    std::int32_t unk_7;
    std::int32_t unk_8;
    std::int32_t unk_9;
    std::int32_t unk_10;
    std::int32_t unk_11;
    std::int32_t dataOffset;
    std::int32_t dataSize;
    std::int32_t unk_14;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileCompressionHeader_Mm6 {
    inline LodFileCompressionHeader_Mm6() {
        memset(this, 0, sizeof(*this));
    }

    std::uint32_t version;
    std::array<std::uint8_t, 4> signature;
    std::uint32_t compressedSize;
    std::uint32_t decompressedSize;
};
#pragma pack(pop)
