#pragma once

#include <cassert>
#include <utility>

#include "Engine/Spells/SpellEnums.h"

#include "Utility/Segment.h"

#include "CharacterEnums.h"

//
// Condition
//

CharacterPortrait portraitForCondition(Condition condition);

/**
 * @return                              List of conditions that can be applied to the character.
 */
inline Segment<Condition> allConditions() {
    return {CONDITION_FIRST, CONDITION_LAST};
}

//
// CharacterSkillMastery
//

inline Segment<CharacterSkillMastery> allSkillMasteries() {
    return {CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST};
}

inline int spellCountForMastery(CharacterSkillMastery maxMastery) {
    switch(maxMastery) {
    case CHARACTER_SKILL_MASTERY_NOVICE:
        return 4;
    case CHARACTER_SKILL_MASTERY_EXPERT:
        return 7;
    case CHARACTER_SKILL_MASTERY_MASTER:
        return 10;
    case CHARACTER_SKILL_MASTERY_GRANDMASTER:
        return 11;
    default:
        assert(false);
        return 0;
    }
}


//
// CharacterSkillType
//

inline Segment<CharacterSkillType> allSkills() {
    return {CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST};
}

/**
 * @return                              List of skills that are visible to the player and that are stored in a savegame.
 */
inline Segment<CharacterSkillType> allVisibleSkills() {
    return {CHARACTER_SKILL_FIRST_VISIBLE, CHARACTER_SKILL_LAST_VISIBLE};
}

/**
 * @return                              List of skills that are drawn in the "Armor" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allArmorSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_LEATHER, CHARACTER_SKILL_CHAIN, CHARACTER_SKILL_PLATE,
        CHARACTER_SKILL_SHIELD,  CHARACTER_SKILL_DODGE
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Weapons" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allWeaponSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_AXE,   CHARACTER_SKILL_BOW,     CHARACTER_SKILL_DAGGER,
        CHARACTER_SKILL_MACE,  CHARACTER_SKILL_SPEAR,   CHARACTER_SKILL_STAFF,
        CHARACTER_SKILL_SWORD, CHARACTER_SKILL_UNARMED, CHARACTER_SKILL_BLASTER
        // CHARACTER_SKILL_CLUB is not displayed in skills.
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Misc" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allMiscSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_ALCHEMY,      CHARACTER_SKILL_ARMSMASTER,
        CHARACTER_SKILL_BODYBUILDING, CHARACTER_SKILL_ITEM_ID,
        CHARACTER_SKILL_MONSTER_ID,   CHARACTER_SKILL_LEARNING,
        CHARACTER_SKILL_TRAP_DISARM,  CHARACTER_SKILL_MEDITATION,
        CHARACTER_SKILL_MERCHANT,     CHARACTER_SKILL_PERCEPTION,
        CHARACTER_SKILL_REPAIR,       CHARACTER_SKILL_STEALING
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Magic" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allMagicSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_FIRE,  CHARACTER_SKILL_AIR,    CHARACTER_SKILL_WATER,
        CHARACTER_SKILL_EARTH, CHARACTER_SKILL_SPIRIT, CHARACTER_SKILL_MIND,
        CHARACTER_SKILL_BODY,  CHARACTER_SKILL_LIGHT,  CHARACTER_SKILL_DARK
    };

    return result;
}

CharacterSkillType skillForMagicSchool(MagicSchool school);

CharacterSkillType skillForSpell(SpellId spell);


//
// CharacterClass
//

inline CharacterClass getTier1Class(CharacterClass classType) {
    return static_cast<CharacterClass>(std::to_underlying(classType) & ~3);
}

inline CharacterClass getTier2Class(CharacterClass classType) {
    return static_cast<CharacterClass>((std::to_underlying(classType) & ~3) + 1);
}

inline CharacterClass getTier3LightClass(CharacterClass classType) {
    return static_cast<CharacterClass>((std::to_underlying(classType) & ~3) + 2);
}

inline CharacterClass getTier3DarkClass(CharacterClass classType) {
    return static_cast<CharacterClass>((std::to_underlying(classType) & ~3) + 3);
}

inline int getClassTier(CharacterClass classType) {
    int index = (std::to_underlying(classType) & 3);
    return index == 3 ? 3 : index + 1;
}

/**
 * Get promotions of higher tier class relative to given one.
 *
 * Base class is of tier 1.
 * After initial promotion class becomes tier 2.
 * Tier 2 class is promoted through light or dark path to tier 3 class.
 *
 * @param classType                     Character class.
 * @return                              All classes that the given class can be promoted to.
 */
inline Segment<CharacterClass> promotionsForClass(CharacterClass classType) {
    int tier = getClassTier(classType);

    if (tier == 1) {
        return {getTier2Class(classType), getTier3DarkClass(classType)};
    } else if (tier == 2) {
        return {getTier3LightClass(classType), getTier3DarkClass(classType)};
    } else {
        return {}; // tier 3 max
    }
}


//
// CharacterAttributeType
//

/**
 * @return                              All attributes that can be improved though attribute item enchantments, like
 *                                      "of Might".
 */
inline Segment<CharacterAttribute> allEnchantableAttributes() {
    return {ATTRIBUTE_FIRST_ENCHANTABLE, ATTRIBUTE_LAST_ENCHANTABLE};
}

/**
 * @return                              Segment containing the 7 basic character stats (starting with might & ending
 *                                      with luck).
 */
inline Segment<CharacterAttribute> allStatAttributes() {
    return {ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT};
}
