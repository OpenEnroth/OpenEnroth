#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "src/tools/Flags.h"

#include "Spells.h"

namespace CastSpellInfoHelpers {
void Cancel_Spell_Cast_In_Progress();
void CastSpell();
};  // namespace CastSpellInfoHelpers

class GUIWindow;

// flags
enum class SpellCastFlag : uint16_t {
    ON_CAST_0x0001 = 0x0001,
    ON_CAST_WholeParty_BigImprovementAnim = 0x0002,
    ON_CAST_0x0004 = 0x0004,
    ON_CAST_TargetCrosshair = 0x0008,
    ON_CAST_0x0010 = 0x0010,
    ON_CAST_NoRecoverySpell = 0x0020,
    ON_CAST_Telekenesis = 0x0040,
    ON_CAST_Enchantment = 0x0080,
    ON_CAST_MonsterSparkles = 0x0100,
    ON_CAST_DarkSacrifice = 0x0200,
    ON_CAST_CastingInProgress =
        ON_CAST_WholeParty_BigImprovementAnim | ON_CAST_TargetCrosshair |
        ON_CAST_Telekenesis | ON_CAST_Enchantment | ON_CAST_MonsterSparkles |
        ON_CAST_DarkSacrifice,
    ON_CAST_Mask_0x007F = 0x007F,
};
using enum SpellCastFlag;
DECLARE_FLAGS(SpellCastFlags, SpellCastFlag)
DECLARE_OPERATORS_FOR_FLAGS(SpellCastFlags)

/*  271 */
#pragma pack(push, 1)
struct CastSpellInfo {
    inline CastSpellInfo() {
        memset(this, 0, sizeof(*this));
    }

    GUIWindow *GetCastSpellInInventoryWindow();

    uint16_t uSpellID;
    uint16_t uPlayerID;
    uint16_t uPlayerID_2;
    __int16 field_6;
    SpellCastFlags uFlags;
    __int16 forced_spell_skill_level;
    int spell_target_pid;
    int sound_id;
};
#pragma pack(pop)

void _42777D_CastSpell_UseWand_ShootArrow(SPELL_TYPE spell,
                                          unsigned int uPlayerID,
                                          unsigned int a4, SpellCastFlags flags,
                                          int a6);
