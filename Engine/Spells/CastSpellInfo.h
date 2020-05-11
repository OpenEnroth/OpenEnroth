#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "Spells.h"

namespace CastSpellInfoHelpers {
void Cancel_Spell_Cast_In_Progress();
void _427E01_cast_spell();
};  // namespace CastSpellInfoHelpers

class GUIWindow;

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
    uint16_t uFlags;
    __int16 forced_spell_skill_level;
    int spell_target_pid;
    int sound_id;
};
#pragma pack(pop)

// flags
#define ON_CAST_WholeParty_BigImprovementAnim 0x0002
#define ON_CAST_TargetCrosshair 0x0008
#define ON_CAST_NoRecoverySpell 0x0020
#define ON_CAST_Telekenesis 0x0040
#define ON_CAST_Enchantment 0x0080
#define ON_CAST_MonsterSparkles 0x0100
#define ON_CAST_DarkSacrifice 0x0200
#define ON_CAST_CastingInProgress                                          \
    (ON_CAST_WholeParty_BigImprovementAnim | ON_CAST_TargetCrosshair |     \
     ON_CAST_Telekenesis | ON_CAST_Enchantment | ON_CAST_MonsterSparkles | \
     ON_CAST_DarkSacrifice)
void _42777D_CastSpell_UseWand_ShootArrow(SPELL_TYPE spell,
                                          unsigned int uPlayerID,
                                          unsigned int a4, __int16 flags,
                                          int a6);
