#pragma once

#include <array>

#include <stdint.h>


#pragma pack(push, 1)
struct LodFileHeader_Mm6 {
    inline LodFileHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    std::array<uint8_t, 16> name;
    uint32_t dataOffset;
    uint32_t size;
    uint32_t dword_000018;
    uint16_t numItems;
    uint16_t priority;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileHeader_Mm8 {
    inline LodFileHeader_Mm8() {
        memset(this, 0, sizeof(this));
    }

    std::array<uint8_t, 16> name;
    int32_t unk_0;
    int32_t unk_1;
    int32_t unk_2;
    int32_t unk_3;
    int32_t unk_4;
    int32_t unk_5;
    int32_t unk_6;
    int32_t unk_7;
    int32_t unk_8;
    int32_t unk_9;
    int32_t unk_10;
    int32_t unk_11;
    int32_t dataOffset;
    int32_t dataSize;
    int32_t unk_14;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileCompressionHeader_Mm6 {
    inline LodFileCompressionHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    uint32_t version;
    std::array<uint8_t, 4> signature;
    uint32_t compressedSize;
    uint32_t decompressedSize;
};
#pragma pack(pop)
