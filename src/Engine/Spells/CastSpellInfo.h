#pragma once

#include <cstdint>

#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Pid.h"

#include "Utility/Flags.h"

#include "SpellEnums.h"

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
    ON_CAST_CastViaScroll = 0x0001,            // Spell is cast via scroll
    ON_CAST_TargetedCharacter = 0x0002,        // Targeted spell, target is character
    // 0x0004 unused
    ON_CAST_TargetedActor = 0x0008,            // Targeted spell, target is actor
    ON_CAST_TargetIsParty = 0x0010,            // Spell target is whole party
    ON_CAST_NoRecoverySpell = 0x0020,          // No recovery time after spell cast
    ON_CAST_TargetedTelekinesis = 0x0040,      // Targeted spell, telekinesis
    ON_CAST_TargetedEnchantment = 0x0080,      // Targeted spell, target is item in inventory
    ON_CAST_TargetedActorOrCharacter = 0x0100, // Targeted spell, target either actor or character
    ON_CAST_TargetedHireling = 0x0200,         // Targeted spell, target is hireling

    // Cumulative flags indicating that spell is targeted
    ON_CAST_CastingInProgress =
        ON_CAST_TargetedCharacter | ON_CAST_TargetedActor |
        ON_CAST_TargetedTelekinesis | ON_CAST_TargetedEnchantment |
        ON_CAST_TargetedActorOrCharacter | ON_CAST_TargetedHireling
};
using enum SpellCastFlag;
MM_DECLARE_FLAGS(SpellCastFlags, SpellCastFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SpellCastFlags)

// Scrolls or NPC spells casted with MASTER mastery of skill level 5
static const CombinedSkillValue SCROLL_OR_NPC_SPELL_SKILL_VALUE(5, CHARACTER_SKILL_MASTERY_MASTER);

// Recovery time when spell failed because of curse
static const uint16_t SPELL_FAILURE_RECOVERY_TIME_ON_CURSE = 100;

/*  271 */
struct CastSpellInfo {
    CastSpellInfo();

    GUIWindow *GetCastSpellInInventoryWindow();

    SPELL_TYPE uSpellID;
    uint16_t uPlayerID;
    uint16_t uPlayerID_2;
    int16_t field_6; // ITEM_EQUIP_TYPE when enchanting.
    SpellCastFlags uFlags;
    CombinedSkillValue forced_spell_skill_level;
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
 * @param spell                         ID of spell.
 * @param uPlayerID                     ID of player casting spell.
 * @param skill_value                   Skill value that the spell is cast with.
 * @param flags                         Spell flags. Can be empty or have several flags.
 * @param a6                            ???
 */
void pushSpellOrRangedAttack(SPELL_TYPE spell,
                             unsigned int uPlayerID,
                             CombinedSkillValue skill_value,
                             SpellCastFlags flags,
                             int a6);

/**
 * Register spell cast on party with temple donation.
 * Temple spells are cast with MASTER mastery of skill level equal to the day of week.
 *
 * @param spell                         ID of spell.
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
 * Process successful picking target for spell.
 *
 * @param pid            `Pid` of the target.
 * @param playerTarget   Target player index.
 */
void spellTargetPicked(int pid, int playerTarget);
