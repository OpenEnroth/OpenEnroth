#include "ChestTable.h"

#include "Utility/String/Format.h"

static int chestPixelOffsetX[8] = {42, 18, 18, 42, 42, 42, 18, 42};
static int chestPixelOffsetY[8] = {34, 30, 30, 34, 34, 34, 30, 34};
static int chestWidthsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};
static int chestHeightsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};

std::array<ChestData, 8> chestTable = {{}};

void initializeChests() {
    // MM7 has dchest.bin in events.lod to store the chest table, and it's storing parts of chest data that we need
    // (see ChestData_MM7). However, most of its fields, are unused, and texture ids are always equal chest id + 1.
    // So we just skip loading it, the sizes and offsets were hardcoded into the binary anyway.
    for (size_t i = 0; i < chestTable.size(); i++) {
        chestTable[i].size = Sizei(chestWidthsByType[i], chestHeightsByType[i]);
        chestTable[i].inventoryOffset = Pointi(chestPixelOffsetX[i], chestPixelOffsetY[i]);
        chestTable[i].textureName = fmt::format("chest{:02}", i + 1);
    }
}
