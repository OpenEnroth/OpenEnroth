#include "LodFormatSnapshots.h"

#include <cassert>

#include "Library/Snapshots/CommonSnapshots.h"

#include "LodFont.h"

void snapshot(const LodFontHeader &src, LodFontHeader_MM6 *dst) {
    *dst = LodFontHeader_MM6();
    dst->firstChar = src.firstChar;
    dst->lastChar = src.lastChar;
    dst->field_3 = 8;
    dst->height = src.fontHeight;
}

void reconstruct(const LodFontHeader_MM6 &src, LodFontHeader *dst) {
    dst->firstChar = src.firstChar;
    dst->lastChar = src.lastChar;
    dst->fontHeight = src.height;
}

void snapshot(const LodFontMetrics &src, LodFontMetrics_MM6 *dst) {
    dst->leftSpacing = src.leftSpacing;
    dst->width = src.width;
    dst->rightSpacing = src.rightSpacing;
}

void reconstruct(const LodFontMetrics_MM6 &src, LodFontMetrics *dst) {
    dst->leftSpacing = src.leftSpacing;
    dst->width = src.width;
    dst->rightSpacing = src.rightSpacing;
}

void snapshot(const LodFontAtlas &src, LodFontAtlas_MM6 *dst) {
    snapshot(src.metrics, &dst->metrics);
    snapshot(src.offsets, &dst->offsets, tags::cast<int, uint32_t>);
}

void reconstruct(const LodFontAtlas_MM6 &src, LodFontAtlas *dst) {
    reconstruct(src.metrics, &dst->metrics);
    reconstruct(src.offsets, &dst->offsets, tags::cast<uint32_t, int>);
}

void snapshot(const LodFontAtlas &src, LodFontAtlas_MMZ *dst) {
    for (size_t i = 0; i < 256; i++) {
        assert(src.metrics[i].leftSpacing == 0 && src.metrics[i].rightSpacing == 0 && src.metrics[i].width <= 255); // MMZ only stores widths.
        dst->widths[i] = src.metrics[i].width;
    }

    snapshot(src.offsets, &dst->offsets, tags::cast<int, uint32_t>);
}

void reconstruct(const LodFontAtlas_MMZ &src, LodFontAtlas *dst) {
    for (size_t i = 0; i < 256; i++) {
        dst->metrics[i].width = src.widths[i];
        dst->metrics[i].leftSpacing = dst->metrics[i].rightSpacing = 0;
    }

    reconstruct(src.offsets, &dst->offsets, tags::cast<uint32_t, int>);
}
