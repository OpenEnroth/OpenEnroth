#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "Engine/Objects/PlayerEnums.h"

#include "Utility/Flags.h"

#include "Spells.h"

namespace CastSpellInfoHelpers {
    /**
     * Cast spell processing.
     *
     * @offset 0x00427E01
     */
    void castSpell();

    /**
     * Remove all targeted spells from spell queue.
     *
     * @offset 0x00427D48
     */
    void cancelSpellCastInProgress();
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
    ON_CAST_TargetedTelekinesis = 0x0040,
    ON_CAST_Enchantment = 0x0080,
    ON_CAST_MonsterSparkles = 0x0100,
    ON_CAST_DarkSacrifice = 0x0200,

    ON_CAST_CastingInProgress =
        // TODO: these flags need to be renamed to correctly represent
        //       spell target pick UI for particular spells
        ON_CAST_SinglePlayer_BigImprovementAnim | ON_CAST_TargetCrosshair |
        ON_CAST_TargetedTelekinesis | ON_CAST_Enchantment | ON_CAST_MonsterSparkles |
        ON_CAST_DarkSacrifice
};
using enum SpellCastFlag;
MM_DECLARE_FLAGS(SpellCastFlags, SpellCastFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SpellCastFlags)

// Scrolls or NPC spells casted with MASTER mastery of skill level 5
static const PLAYER_SKILL SCROLL_OR_NPC_SPELL_SKILL_VALUE = ConstructSkillValue(PLAYER_SKILL_MASTERY_MASTER, 5);

// Recovery time when spell failed because of curse
static const uint16_t SPELL_FAILURE_RECOVERY_TIME_ON_CURSE = 100;

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

/**
 * General function that registers spell or skill implemented
 * through spell casting mechanism to be cast/performed later.
 *
 * Actual casting will be performed with event queue processing.
 *
 * If spell is targeted then corresponding target mode is entered
 * and actual casting will be performed after target is picked.
 * Registered targeted spells will have one of the flags
 * listed in ON_CAST_CastingInProgress and this flag will be removed
 * in event queue if correct target is picked.
 *
 * @offset 0x0042777D
 *
 * @param spell          ID of spell.
 * @param uPlayerID      ID of player casting spell.
 * @param skill_value    Skill value (mastery+skill level) spell is casted with.
 * @param flags          Spell flags. Can be empty or have several flags.
 * @param a6             ???
 */
void pushSpellOrRangedAttack(SPELL_TYPE spell,
                             unsigned int uPlayerID,
                             PLAYER_SKILL skill_value,
                             SpellCastFlags flags,
                             int a6);

/**
 * Register spell cast on party with temple donation.
 * Temple spells are cast with MASTER mastery of skill level equal to the day of week.
 *
 * @param spell        ID of spell.
 */
void pushTempleSpell(SPELL_TYPE spell);

/**
 * Register spell cast by NPC companions.
 *
 * @param spell        ID of spell.
 */
void pushNPCSpell(SPELL_TYPE spell);

/**
 * Register spell cast through scroll.
 *
 * @param spell        ID of spell.
 * @param uPlayerID    ID of player casting spell.
 */
void pushScrollSpell(SPELL_TYPE spell, unsigned int uPlayerID);

/**
 * Process successfull picking target for telekinesis spell.
 */
void telekinesisTargetPicked(int pid);
