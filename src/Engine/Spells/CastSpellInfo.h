#pragma once

#include <cstdint>

#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Pid.h"

#include "Utility/Flags.h"

#include "SpellEnums.h"

class TargetedSpellUI;

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

    /**
    * Clear queue
    */
    void clearSpellQueue();
    /**
    * Test for spells in queue
    */
    bool hasQueuedSpells();
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
// Wands cast at novice mastery skill level 8
static const CombinedSkillValue WANDS_SKILL_VALUE(8, CHARACTER_SKILL_MASTERY_NOVICE);

struct CastSpellInfo {
    TargetedSpellUI *GetCastSpellInInventoryWindow();

    SpellId uSpellID = SPELL_NONE; // Spell being cast.
    int casterCharacterIndex = -1; // 0-based index of the character who cast the spell.
    int targetCharacterIndex = -1; // 0-based index of the target character, if any.
                                   // TODO(captainurist): also pParty->hirelingScrollPosition-based hireling index for dark sacrifice.
    SpellCastFlags flags = 0;
    CombinedSkillValue overrideSkillValue; // If set - skill value to use for casting.
    Pid targetPid; // Target pid, if any.
    int targetInventoryIndex = -1; // Target inventory item index (in Character::pInventoryItemList) in target
                                   // character's inventory, if any.

    int overrideSoundId = 0; // TODO(captainurist): doesn't look like sound id. Maybe flags?
                             //                     Bits 0-2 for caster (1-based), bit 3 for blaster.
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
 * @param spell                         Spell id.
 * @param casterIndex                   Zero-based index of a character casting the spell.
 * @param skill_value                   Skill value that the spell is cast with.
 * @param flags                         Spell flags. Can be empty or have several flags.
 * @param overrideSoundId                            ???
 */
void pushSpellOrRangedAttack(SpellId spell,
                             int casterIndex,
                             CombinedSkillValue skill_value,
                             SpellCastFlags flags,
                             int overrideSoundId);

/**
 * Register spell cast on party with temple donation.
 * Temple spells are cast with MASTER mastery of skill level equal to the day of week.
 *
 * @param spell                         Spell id.
 */
void pushTempleSpell(SpellId spell);

/**
 * Register spell cast by NPC companions.
 *
 * @param spell                         Spell id.
 */
void pushNPCSpell(SpellId spell);

/**
 * Register spell cast through scroll.
 *
 * @param spell                         Spell id.
 * @param casterIndex                   0-based index of the character casting the spell.
 */
void pushScrollSpell(SpellId spell, int casterIndex);

/**
 * Process successful picking target for spell.
 *
 * @param targetPid                     `Pid` of the target.
 * @param targetCharacterIndex          0-based index of the character being targeted by the spell.
 */
void spellTargetPicked(Pid targetPid, int targetCharacterIndex);
