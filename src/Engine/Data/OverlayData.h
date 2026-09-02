#pragma once

#include <cstdint>

struct OverlayData {
    uint16_t uOverlayID = 0; // Id this row is looked up by, see ActiveOverlayList::_4418B6.
    uint16_t uOverlayType = 0; // Placement mode, 0 centers the sprite and 2 is transparent. MM6 branches on it, we don't.
    uint16_t uSpriteFramesetID = 0; // Index into SpriteFrameTable. Set only in MM6, zero in every MM7 and MM8 record.
    int16_t spriteFramesetGroup = 0; // TODO(captainurist): name is a guess, zero everywhere and no engine reads it.
};
