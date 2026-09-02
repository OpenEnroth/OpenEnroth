#pragma once

#include <vector>

#include "Engine/Data/OverlayData.h"

struct OverlayTable {
    void initializeSprites();

    std::vector<OverlayData> overlays;
};

extern OverlayTable *pOverlayTable;
