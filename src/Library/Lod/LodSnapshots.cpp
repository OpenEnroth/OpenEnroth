#include "LodSnapshots.h"

#include <cassert>

#include "Library/Snapshots/CommonSnapshots.h"

#include "Utility/Memory/MemSet.h"

void snapshot(const LodHeader &src, LodHeader_MM6 *dst) {
    memzero(dst);

    snapshot(src.signature, &dst->signature);
    snapshot(src.version, &dst->version);
    snapshot(src.description, &dst->description);
    dst->size = 100;
    dst->numDirectories = src.numDirectories;
}

void reconstruct(const LodHeader_MM6 &src, LodHeader *dst) {
    reconstruct(src.signature, &dst->signature);
    reconstruct(src.version, &dst->version);
    reconstruct(src.description, &dst->description);
    dst->numDirectories = src.numDirectories;
}

void snapshot(const LodEntry &src, LodEntry_MM6 *dst) {
    memzero(dst);

    snapshot(src.name, &dst->name);
    dst->dataOffset = src.dataOffset;
    dst->dataSize = src.dataSize;
    dst->numItems = src.numItems;
}

void reconstruct(const LodEntry_MM6 &src, LodEntry *dst) {
    reconstruct(src.name, &dst->name);
    dst->dataOffset = src.dataOffset;
    dst->dataSize = src.dataSize;
    dst->numItems = src.numItems;
}

void snapshot(const LodEntry &src, LodFileEntry_MM8 *dst) {
    assert(src.numItems == 0); // Only for file entries.

    memzero(dst);

    snapshot(src.name, &dst->name);
    dst->dataOffset = src.dataOffset;
    dst->dataSize = src.dataSize;
}

void reconstruct(const LodFileEntry_MM8 &src, LodEntry *dst) {
    reconstruct(src.name, &dst->name);
    dst->dataOffset = src.dataOffset;
    dst->dataSize = src.dataSize;
    dst->numItems = 0;
}
