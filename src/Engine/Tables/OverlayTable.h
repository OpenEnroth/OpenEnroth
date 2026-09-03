#pragma once

#include <memory>
#include <vector>

#include "Engine/Data/OverlayData.h"

struct OverlayTable {
    void initializeSprites();

    std::vector<OverlayData> overlays;
};

extern std::unique_ptr<OverlayTable> pOverlayTable;
