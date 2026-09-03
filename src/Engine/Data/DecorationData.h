#pragma once

#include <cstdint>
#include <string>

#include "DecorationEnums.h"
#include "SoundEnums.h"

#include "Library/Color/Color.h"

struct DecorationData {
    std::string internalName; // Internal name of the decoration (e.g. "dec03").
    std::string hint; // Hint for the decoration (e.g. "campfire", "cauldron"). This text is shown in the status bar
                      // when clicking on the decoration if it's not scripted.
    int16_t uType = 0;
    uint16_t uDecorationHeight = 0;
    int16_t uRadius = 0;
    int16_t uLightRadius = 0;
    uint16_t uSpriteID = 0;
    DecorationDescFlags uFlags;
    SoundId uSoundID = SOUND_Invalid;
    Color uColoredLight;

    inline bool CanMoveThrough() const {
        return uFlags & (DECORATION_DESC_MOVE_THROUGH | DECORATION_DESC_DONT_DRAW);
    }
    inline bool DontDraw() const { return uFlags & DECORATION_DESC_DONT_DRAW; }
    inline bool SoundOnDawn() const {
        return uFlags & DECORATION_DESC_SOUND_ON_DAWN;
    }
    inline bool SoundOnDusk() const {
        return uFlags & DECORATION_DESC_SOUND_ON_DUSK;
    }
};
