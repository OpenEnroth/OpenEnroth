#include "VidSnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

void reconstruct(const VidEntry_MM7 &src, VidEntry *dst) {
    reconstruct(src.name, &dst->name);
    dst->offset = src.offset;
}
