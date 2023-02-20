#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "Engine/Objects/PlayerEnums.h"

#include "Utility/Flags.h"

#include "Spells.h"

namespace CastSpellInfoHelpers {
    void CancelSpellCastInProgress();
    void CastSpell();
};  // namespace CastSpellInfoHelpers

class GUIWindow;

// flags
enum class SpellCastFlag : uint16_t {
    ON_CAST_CastViaScroll = 0x0001,
    ON_CAST_SinglePlayer_BigImprovementAnim = 0x0002,
    // 0x0004 unused
    ON_CAST_TargetCrosshair = 0x0008,
    ON_CAST_TargetIsParty = 0x0010,
    ON_CAST_NoRecoverySpell = 0x0020,
    ON_CAST_Telekenesis = 0x0040,
    ON_CAST_Enchantment = 0x0080,
    ON_CAST_MonsterSparkles = 0x0100,
    ON_CAST_DarkSacrifice = 0x0200,

    ON_CAST_CastingInProgress =
        // TODO: these flags need to be renamed to correctly represent
        //       spell target pick UI for particular spells
        ON_CAST_SinglePlayer_BigImprovementAnim | ON_CAST_TargetCrosshair |
        ON_CAST_Telekenesis | ON_CAST_Enchantment | ON_CAST_MonsterSparkles |
        ON_CAST_DarkSacrifice
};
using enum SpellCastFlag;
MM_DECLARE_FLAGS(SpellCastFlags, SpellCastFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SpellCastFlags)

// Scrolls or NPC spells casted with MASTER mastery of skill level 5
static const PLAYER_SKILL SCROLL_OR_NPC_SPELL_SKILL_VALUE = ConstructSkillValue(PLAYER_SKILL_MASTERY_MASTER, 5);

/*  271 */
struct CastSpellInfo {
    inline CastSpellInfo() {
        memset(this, 0, sizeof(*this));
    }

    GUIWindow *GetCastSpellInInventoryWindow();

    SPELL_TYPE uSpellID;
    uint16_t uPlayerID;
    uint16_t uPlayerID_2;
    int16_t field_6; // ITEM_EQUIP_TYPE when enchanting.
    SpellCastFlags uFlags;
    int16_t forced_spell_skill_level;
    int spell_target_pid;
    int sound_id;
};

void RegisterSpellOrSpellLikeSkill(SPELL_TYPE spell,
                                   unsigned int uPlayerID,
                                   PLAYER_SKILL skill_value,
                                   SpellCastFlags flags,
                                   int a6);

void RegisterTempleSpell(SPELL_TYPE spell);
void RegisterNPCSpell(SPELL_TYPE spell);
void RegisterScrollSpell(SPELL_TYPE spell, unsigned int uPlayerID);
