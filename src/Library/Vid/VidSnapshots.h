#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Library/Binary/MemCopySerialization.h"

//
// Runtime structs.
//

struct VidEntry {
    std::string name;
    size_t offset;
};


//
// Snapshots.
//

#pragma pack(push, 1)

// Note that there is no VidHeader_MM7. Entries are just stored as a serialized vector in the VID file.

struct VidEntry_MM7 {
    std::array<char, 40> name;
    uint32_t offset;
};
static_assert(sizeof(VidEntry_MM7) == 44);
MM_DECLARE_MEMCOPY_SERIALIZABLE(VidEntry_MM7)

void reconstruct(const VidEntry_MM7 &src, VidEntry *dst);

#pragma pack(pop)
