#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "Media/Audio/SoundEnums.h"

#include "Library/Color/Color.h"

#include "DecorationEnums.h"

struct DecorationDesc {
    std::string name; // Internal name (e.g. "dec03").
    std::string type; // Type of the decoration (e.g. "campfire", "cauldron"). This text is shown in the status bar
                      // when clicking on the decoration if it's not scripted.
    int16_t uType;
    uint16_t uDecorationHeight;
    int16_t uRadius;
    int16_t uLightRadius;
    uint16_t uSpriteID;
    DecorationDescFlags uFlags;
    SoundId uSoundID;
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

class DecorationList {
 public:
    inline DecorationList() {}

    void InitializeDecorationSprite(DecorationId uDecID);
    DecorationId GetDecorIdByName(std::string_view pName);

    const DecorationDesc *GetDecoration(DecorationId index) const {
        return &pDecorations[std::to_underlying(index)];
    }

 public:
    std::vector<DecorationDesc> pDecorations; // TODO(captainurist): IndexedArray.
};

extern DecorationList *pDecorationList;

void RespawnGlobalDecorations();
