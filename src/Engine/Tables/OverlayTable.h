#pragma once

#include <vector>

#include "Engine/Data/OverlayData.h"

class Blob;

struct OverlayTable {
    void initializeSprites();

    friend void deserialize(const Blob &src, OverlayTable *dst); // In TableSerialization.cpp.

    std::vector<OverlayData> overlays;
};

extern OverlayTable *pOverlayTable;
