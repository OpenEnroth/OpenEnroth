#pragma once

#include <cstdint>
#include <vector>

#include "Utility/String.h"
#include "Utility/Flags.h"
#include "Utility/Memory/Blob.h"

/*  321 */
enum class DECORATION_DESC_FLAG : uint16_t {
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
using enum DECORATION_DESC_FLAG;
MM_DECLARE_FLAGS(DECORATION_DESC_FLAGS, DECORATION_DESC_FLAG)
MM_DECLARE_OPERATORS_FOR_FLAGS(DECORATION_DESC_FLAGS)

/*   54 */
#pragma pack(push, 1)
// TODO(captainurist): move to legacyimages
struct DecorationDesc_mm6 {
    inline bool CanMoveThrough() const {
        return uFlags & DECORATION_DESC_MOVE_THROUGH;
    }
    inline bool DontDraw() const { return uFlags & DECORATION_DESC_DONT_DRAW; }
    inline bool SoundOnDawn() const {
        return uFlags & DECORATION_DESC_SOUND_ON_DAWN;
    }
    inline bool SoundOnDusk() const {
        return uFlags & DECORATION_DESC_SOUND_ON_DUSK;
    }

    char pName[32];
    char field_20[32];
    int16_t uType;
    uint16_t uDecorationHeight;
    int16_t uRadius;
    int16_t uLightRadius;
    uint16_t uSpriteID;
    DECORATION_DESC_FLAGS uFlags;
    int16_t uSoundID;
    int16_t _pad;
};

struct DecorationDesc : public DecorationDesc_mm6 {
    uint8_t uColoredLightRed;
    uint8_t uColoredLightGreen;
    uint8_t uColoredLightBlue;
    uint8_t __padding;
};
#pragma pack(pop)

class DecorationList {
 public:
    inline DecorationList() {}

    void ToFile();
    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);
    void InitializeDecorationSprite(unsigned int uDecID);
    uint16_t GetDecorIdByName(std::string_view pName);

    const DecorationDesc *GetDecoration(unsigned int index) const {
        return &pDecorations[index];
    }

 protected:
    std::vector<DecorationDesc> pDecorations;
};

extern class DecorationList *pDecorationList;

void RespawnGlobalDecorations();
