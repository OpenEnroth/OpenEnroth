#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Library/Binary/BinarySerialization.h"

#include "LodEnums.h"

//
// Runtime structs.
//

struct LodHeader {
    std::string signature;
    std::string version;
    std::string description;
    size_t numDirectories = 0;
};

struct LodEntry {
    std::string name;
    size_t dataOffset = 0;
    size_t dataSize = 0;
    size_t numItems = 0;
};


//
// Snapshots.
//

#pragma pack(push, 1)

struct LodHeader_MM6 {
    std::array<char, 4> signature; // File magic, "LOD\0".
    std::array<char, 80> version; // LOD version, see `LodVersion`.
    std::array<char, 80> description; // Textual description, not used for anything.
    std::uint32_t size; // Somehow this field always equals 100.
    std::uint32_t unk_0; // Always zero.
    std::uint32_t numDirectories; // Number of LOD entries that follow immediately after this struct, always 1 for MM LODs.
    std::array<char, 80> unk_1;  // Never set/accessed so stores random garbage.
};
static_assert(sizeof(LodHeader_MM6) == 256);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodHeader_MM6);

void snapshot(const LodHeader &src, LodHeader_MM6 *dst);
void reconstruct(const LodHeader_MM6 &src, LodHeader *dst);


struct LodEntry_MM6 {
    std::array<char, 16> name;
    std::uint32_t dataOffset; // For a root entry offset is 0-based, otherwise it's relative to directory offset.
    std::uint32_t dataSize; // For a directory this also includes the directory contents.
    std::uint32_t unk_0; // Seems to be always 0.
    std::uint16_t numItems; // Non-zero if this is a directory.
    std::uint16_t priority; // Seems to be always 0.
};
static_assert(sizeof(LodEntry_MM6) == 32);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodEntry_MM6)

void snapshot(const LodEntry &src, LodEntry_MM6 *dst);
void reconstruct(const LodEntry_MM6 &src, LodEntry *dst);


struct LodFileEntry_MM8 {
    std::array<char, 16> name;
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
static_assert(sizeof(LodFileEntry_MM8) == 76);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodFileEntry_MM8)

void snapshot(const LodEntry &src, LodFileEntry_MM8 *dst);
void reconstruct(const LodFileEntry_MM8 &src, LodEntry *dst);

#pragma pack(pop)

size_t fileEntrySize(LodVersion version);
