#include "LodFormatSnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

#include "LodFont.h"

void reconstruct(const LodFontHeader_MM7 &src, LodFontHeader *dst) {
    dst->firstChar = src.firstChar;
    dst->lastChar = src.lastChar;
    dst->fontHeight = src.height;
}

void reconstruct(const LodFontMetrics_MM7 &src, LodFontMetrics *dst) {
    dst->leftSpacing = src.leftSpacing;
    dst->width = src.width;
    dst->rightSpacing = src.rightSpacing;
}

void reconstruct(const LodFontAtlas_MM7 &src, LodFontAtlas *dst) {
    reconstruct(src.metrics, &dst->metrics);
    reconstruct(src.offsets, &dst->offsets, tags::cast<uint32_t, int>);
}

void reconstruct(const LodFontAtlas_MMX &src, LodFontAtlas *dst) {
    for (size_t i = 0; i < 256; i++) {
        dst->metrics[i].width = src.widths[i];
        dst->metrics[i].leftSpacing = dst->metrics[i].rightSpacing = 0;
    }

    reconstruct(src.offsets, &dst->offsets, tags::cast<uint32_t, int>);
}
