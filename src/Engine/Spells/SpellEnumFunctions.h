#pragma once

#include <cassert>

#include "Utility/Segment.h"

#include "SpellEnums.h"

//
// SpellId
//

/**
 * @return                              All regular spell types.
 */
inline Segment<SpellId> allRegularSpells() {
    return {SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR};
}

/**
 * @returns                             Whether the spell is targeting an inventory item.
 */
inline bool isSpellTargetsItem(SpellId uSpellID) {
    return uSpellID == SPELL_WATER_ENCHANT_ITEM ||
           uSpellID == SPELL_FIRE_FIRE_AURA ||
           uSpellID == SPELL_DARK_VAMPIRIC_WEAPON ||
           uSpellID == SPELL_WATER_RECHARGE_ITEM;
}

/**
 * Is spell ID references any regular spell?
 */
inline bool isRegularSpell(SpellId uSpellID) {
    return uSpellID >= SPELL_FIRST_REGULAR && uSpellID <= SPELL_LAST_REGULAR;
}


//
// MagicSchool
//

inline Segment<MagicSchool> allMagicSchools() {
    return {MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST};
}

inline Segment<SpellId> spellsForMagicSchool(MagicSchool school) {
    int first = 1 + std::to_underlying(school) * 11;
    int last = first + 10;
    return {static_cast<SpellId>(first), static_cast<SpellId>(last)};
}

inline MagicSchool magicSchoolForSpell(SpellId spell) {
    assert(spell >= SPELL_FIRST_REGULAR && spell <= SPELL_LAST_REGULAR);

    return static_cast<MagicSchool>((std::to_underlying(spell) - 1) / 11);
}

// TODO(captainurist): I think we can drop most usages of this function.
inline int spellIndexInMagicSchool(SpellId spell) {
    return (std::to_underlying(spell) - 1) % 11;
}
