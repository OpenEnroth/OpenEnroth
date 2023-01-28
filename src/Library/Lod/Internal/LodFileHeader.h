#pragma once

#include <stdint.h>


#pragma pack(push, 1)
struct LodFileHeader_Mm6 {
    inline LodFileHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    uint8_t name[16];
    uint32_t data_offset;
    uint32_t size;
    uint32_t dword_000018;
    uint16_t num_items;
    uint16_t priority;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileHeader_Mm8 {
    inline LodFileHeader_Mm8() {
        memset(this, 0, sizeof(this));
    }

    uint8_t name[16];
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
    int32_t data_offset;
    int32_t data_size;
    int32_t unk_14;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct LodFileCompressionHeader_Mm6 {
    inline LodFileCompressionHeader_Mm6() {
        memset(this, 0, sizeof(this));
    }

    uint32_t version;
    uint8_t signature[4];
    uint32_t compressed_size;
    uint32_t decompressed_size;
};
#pragma pack(pop)