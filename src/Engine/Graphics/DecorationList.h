#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "Media/Audio/SoundEnums.h"

#include "Library/Color/Color.h"

#include "Utility/Flags.h"
#include "Utility/Memory/Blob.h"

enum class DecorationDescFlag : uint16_t {
    DECORATION_DESC_MOVE_THROUGH = 0x0001,
    DECORATION_DESC_DONT_DRAW = 0x0002,
    DECORATION_DESC_FLICKER_SLOW = 0x0004,
    DECORATION_DESC_FICKER_AVERAGE = 0x0008,
    DECORATION_DESC_FICKER_FAST = 0x0010,
    DECORATION_DESC_MARKER = 0x0020,
    DECORATION_DESC_SLOW_LOOP = 0x0040,
    DECORATION_DESC_EMITS_FIRE = 0x0080,
    DECORATION_DESC_SOUND_ON_DAWN = 0x0100,
    DECORATION_DESC_SOUND_ON_DUSK = 0x0200,
    DECORATION_DESC_EMITS_SMOKE = 0x0400,
};
using enum DecorationDescFlag;
MM_DECLARE_FLAGS(DecorationDescFlags, DecorationDescFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(DecorationDescFlags)

struct DecorationDesc {
    std::string name;
    std::string field_20; // TODO(captainurist): actually interactionText?
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

    void InitializeDecorationSprite(unsigned int uDecID);
    uint16_t GetDecorIdByName(std::string_view pName);

    const DecorationDesc *GetDecoration(unsigned int index) const {
        return &pDecorations[index];
    }

 public:
    std::vector<DecorationDesc> pDecorations;
};

extern class DecorationList *pDecorationList;

void RespawnGlobalDecorations();
