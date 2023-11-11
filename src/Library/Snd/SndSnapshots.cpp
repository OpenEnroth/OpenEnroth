#include "SndSnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

void reconstruct(const SndEntry_MM7 &src, SndEntry *dst) {
    reconstruct(src.name, &dst->name);
    dst->offset = src.offset;
    dst->size = src.size;
    dst->decompressedSize = src.decompressedSize;
}
