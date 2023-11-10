#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Library/Binary/MemCopySerialization.h"

//
// Runtime structs.
//

struct SndEntry {
    std::string name;
    size_t offset;
    size_t size;
    size_t decompressedSize;
};


//
// Snapshots.
//

#pragma pack(push, 1)

// Note that there is no SndHeader_MM7. Entries are just stored as a serialized vector in the SND file.

struct SndEntry_MM7 {
    std::array<char, 40> name;
    uint32_t offset;
    uint32_t size;
    uint32_t decompressedSize;
};
static_assert(sizeof(SndEntry_MM7) == 52);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SndEntry_MM7)

void reconstruct(const SndEntry_MM7 &src, SndEntry *dst);

#pragma pack(pop)
