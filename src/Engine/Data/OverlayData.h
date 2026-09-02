#pragma once

#include <cstdint>

struct OverlayData {
    uint16_t uOverlayID = 0; // Id this row is looked up by, see ActiveOverlayList::_4418B6.
    uint16_t uOverlayType = 0; // Zero in all shipped MM6 and MM7 data, and nothing reads it.
    uint16_t uSpriteFramesetID = 0; // Index into SpriteFrameTable, zero for every MM7 overlay.
    int16_t spriteFramesetGroup = 0; // Zero in all shipped MM6 and MM7 data, and nothing reads it.
};
