#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <utility>

#include "Engine/GameType.h"
#include "Engine/Spells/SpellEnums.h"

#include "Utility/Segment.h"

#include "CharacterEnums.h"

//
// Condition
//

PortraitId portraitForCondition(Condition condition);

/**
 * @return                              List of conditions that can be applied to the character.
 */
inline Segment<Condition> allConditions() {
    return {CONDITION_FIRST, CONDITION_LAST};
}

//
// CharacterSkillMastery
//

inline Segment<Mastery> allSkillMasteries() {
    return {MASTERY_FIRST, MASTERY_LAST};
}

inline int spellCountForMastery(Mastery maxMastery) {
    switch(maxMastery) {
    case MASTERY_NOVICE:
        return 4;
    case MASTERY_EXPERT:
        return 7;
    case MASTERY_MASTER:
        return 10;
    case MASTERY_GRANDMASTER:
        return 11;
    default:
        assert(false);
        return 0;
    }
}


//
// CharacterSkillType
//

inline Segment<Skill> allSkills() {
    return {SKILL_FIRST, SKILL_LAST};
}

/**
 * @return                              List of skills that are visible to the player and that are stored in a savegame.
 */
inline Segment<Skill> allVisibleSkills() {
    return {SKILL_FIRST_VISIBLE, SKILL_LAST_VISIBLE};
}

/**
 * @return                              List of skills that are drawn in the "Armor" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<Skill> allArmorSkills() {
    static constexpr std::initializer_list<Skill> result = {
        SKILL_LEATHER, SKILL_CHAIN, SKILL_PLATE,
        SKILL_SHIELD,  SKILL_DODGE
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Weapons" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<Skill> allWeaponSkills() {
    static constexpr std::initializer_list<Skill> result = {
        SKILL_AXE,   SKILL_BOW,     SKILL_DAGGER,
        SKILL_MACE,  SKILL_SPEAR,   SKILL_STAFF,
        SKILL_SWORD, SKILL_UNARMED, SKILL_BLASTER
        // CHARACTER_SKILL_CLUB is not displayed in skills.
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Misc" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<Skill> allMiscSkills() {
    static constexpr std::initializer_list<Skill> result = {
        SKILL_ALCHEMY,      SKILL_ARMSMASTER,
        SKILL_BODYBUILDING, SKILL_ITEM_ID,
        SKILL_MONSTER_ID,   SKILL_LEARNING,
        SKILL_TRAP_DISARM,  SKILL_MEDITATION,
        SKILL_MERCHANT,     SKILL_PERCEPTION,
        SKILL_REPAIR,       SKILL_STEALING
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Magic" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<Skill> allMagicSkills() {
    static constexpr std::initializer_list<Skill> result = {
        SKILL_FIRE,  SKILL_AIR,    SKILL_WATER,
        SKILL_EARTH, SKILL_SPIRIT, SKILL_MIND,
        SKILL_BODY,  SKILL_LIGHT,  SKILL_DARK
    };

    return result;
}

Skill skillForMagicSchool(MagicSchool school);

Skill skillForSpell(SpellId spell);


//
// CharacterClass
//

inline Class getTier1Class(Class classType) {
    return static_cast<Class>(std::to_underlying(classType) & ~3);
}

inline Class getTier2Class(Class classType) {
    return static_cast<Class>((std::to_underlying(classType) & ~3) + 1);
}

inline Class getTier3LightClass(Class classType) {
    return static_cast<Class>((std::to_underlying(classType) & ~3) + 2);
}

inline Class getTier3DarkClass(Class classType) {
    return static_cast<Class>((std::to_underlying(classType) & ~3) + 3);
}

inline int getClassTier(Class classType) {
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
inline Segment<Class> promotionsForClass(Class classType) {
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
// MM8 races & classes
//
// MM8 replaces the MM7 race/class split with race-classes: picking a race in character creation also picks
// your starting class. Each race-class has a single promotion (no light/dark branching). See `Race` &
// `Class` in CharacterEnums.h for the full value list.
//

inline bool isMm8Race(Race race) {
    return race >= RACE_MM8_FIRST && race <= RACE_MM8_LAST;
}

/**
 * @return                              All 8 MM8 races, including the recruit-only `RACE_MM8_DRAGON`.
 */
inline Segment<Race> allMm8Races() {
    return {RACE_MM8_FIRST, RACE_MM8_LAST};
}

/**
 * @return                              The 7 MM8 races offered at character creation, i.e. all MM8 races except
 *                                      `RACE_MM8_DRAGON`, which can only be recruited later.
 */
inline Segment<Race> allMm8StartableRaces() {
    return {RACE_MM8_STARTABLE_FIRST, RACE_MM8_STARTABLE_LAST};
}

inline bool isMm8StartableRace(Race race) {
    return race >= RACE_MM8_STARTABLE_FIRST && race <= RACE_MM8_STARTABLE_LAST;
}

inline bool isMm8BaseClass(Class classType) {
    return classType >= CLASS_MM8_BASE_FIRST && classType <= CLASS_MM8_BASE_LAST;
}

inline bool isMm8PromotedClass(Class classType) {
    return classType >= CLASS_MM8_PROMOTED_FIRST && classType <= CLASS_MM8_PROMOTED_LAST;
}

inline bool isMm8Class(Class classType) {
    return isMm8BaseClass(classType) || isMm8PromotedClass(classType);
}

/**
 * `getClassTier()` only understands MM7's four-tier-per-branch bit encoding, so it can't be called directly with
 * an MM8 class. This is the GameType-aware wrapper that every promotion-tier read (e.g. training cost scaling)
 * should go through instead: MM8 classes get their own two-tier scheme (1 = base, 2 = promoted, granted on
 * promotion-quest completion - see `mm8PromotedClassForRace`), MM7 classes keep reading `getClassTier()` unchanged.
 *
 * @param classType                     Character class, MM7 or MM8.
 * @return                              Promotion tier: 1-3 for MM7 classes, 1-2 for MM8 classes.
 */
inline int classTierForClass(Class classType) {
    if (isMm8Class(classType)) {
        return isMm8BaseClass(classType) ? 1 : 2;
    }
    return getClassTier(classType);
}

/**
 * @return                              All 16 MM8 classes (8 base + 8 promoted).
 */
inline Segment<Class> allMm8Classes() {
    return {CLASS_MM8_FIRST, CLASS_MM8_LAST};
}

/**
 * @param race                          An MM8 race, as tested by `isMm8Race`.
 * @return                              The starting class for the given MM8 race.
 */
inline Class mm8BaseClassForRace(Race race) {
    assert(isMm8Race(race));
    int index = std::to_underlying(race) - std::to_underlying(RACE_MM8_FIRST);
    return static_cast<Class>(std::to_underlying(CLASS_MM8_BASE_FIRST) + index);
}

/**
 * @param race                          An MM8 race, as tested by `isMm8Race`.
 * @return                              The (single) promoted class for the given MM8 race.
 */
inline Class mm8PromotedClassForRace(Race race) {
    assert(isMm8Race(race));
    int index = std::to_underlying(race) - std::to_underlying(RACE_MM8_FIRST);
    return static_cast<Class>(std::to_underlying(CLASS_MM8_PROMOTED_FIRST) + index);
}

/**
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @return                              The MM8 race that the given class belongs to.
 */
inline Race mm8RaceForClass(Class classType) {
    assert(isMm8Class(classType));
    int index = isMm8BaseClass(classType) ?
        std::to_underlying(classType) - std::to_underlying(CLASS_MM8_BASE_FIRST) :
        std::to_underlying(classType) - std::to_underlying(CLASS_MM8_PROMOTED_FIRST);
    return static_cast<Race>(std::to_underlying(RACE_MM8_FIRST) + index);
}

/**
 * Translates MM8's own raw class id into the engine's `Class` enum. This raw id is what promotion-quest `.evt`
 * scripts actually encode in a `VAR_Class` instruction (confirmed against MM8's real `global.evt`: e.g. the
 * Knight-promotion event compares raw id 4 then sets raw id 5) - it numbers each race's base & promoted class as
 * a consecutive pair (0/1 = Necromancer/Lich, 2/3 = Cleric/Priest of the Sun, ..., 14/15 = Dragon/Great Wyrm), which
 * is unrelated to the engine's own numbering (see `Class` in CharacterEnums.h), where all 8 base classes are
 * grouped before all 8 promoted ones. `Character::CompareVariable`/`SetVariable`'s `VAR_Class` handling is the
 * only place that should call this.
 *
 * @param rawClassId                    MM8's raw class id (0-15), as read from a `VAR_Class` evt instruction.
 * @return                              The corresponding engine `Class` enum value.
 */
inline Class mm8ClassFromRawId(int rawClassId) {
    assert(rawClassId >= 0 && rawClassId <= 15);
    Race race = static_cast<Race>(std::to_underlying(RACE_MM8_FIRST) + rawClassId / 2);
    return (rawClassId % 2 == 0) ? mm8BaseClassForRace(race) : mm8PromotedClassForRace(race);
}

/**
 * Inverse of `mm8ClassFromRawId()`.
 *
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @return                              MM8's raw class id (0-15) for `classType`.
 */
inline int mm8RawIdForClass(Class classType) {
    assert(isMm8Class(classType));
    int raceIndex = std::to_underlying(mm8RaceForClass(classType)) - std::to_underlying(RACE_MM8_FIRST);
    return raceIndex * 2 + (isMm8PromotedClass(classType) ? 1 : 0);
}

/**
 * Returns the starting skill affinity for the given MM8 race & skill, as per the "Class Starting Skills" table.
 *
 * @param race                          An MM8 race, as tested by `isMm8Race`.
 * @param skill                         Character skill.
 * @return                              Skill affinity for a freshly-created character of the given race.
 */
SkillAffinity mm8StartingSkillAffinity(Race race, Skill skill);

/**
 * Returns the max learnable mastery for the given MM8 class & skill, as per the "Class Skills" table. Unlike
 * `mm8StartingSkillAffinity()`, which only covers the skills a freshly-created character starts with, this is
 * the full trainer-time table: it also covers skills that aren't available at creation but can be learned
 * later, and it's what governs whether a trainer will teach a given mastery level at all.
 *
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @param skill                         Character skill.
 * @return                              Max mastery obtainable for the class/skill combination, or
 *                                      `MASTERY_NONE` if the skill can't be learned by this class at all.
 */
Mastery mm8SkillMaxMastery(Class classType, Skill skill);

/**
 * `skillMaxMasteryPerClass` (declared in mm7_data.h) only has entries for MM7's classes, so it can't be
 * indexed directly with an MM8 class. This is the GameType-aware wrapper that every mastery-cap read (trainer
 * eligibility, skill-up UI, scripting) should go through instead: MM8 classes are routed to
 * `mm8SkillMaxMastery()`, MM7 classes keep reading `skillMaxMasteryPerClass` unchanged.
 *
 * @param classType                     Character class, MM7 or MM8.
 * @param skill                         Character skill.
 * @return                              Max mastery obtainable for the class/skill combination.
 */
Mastery skillMaxMasteryForClass(Class classType, Skill skill);

/**
 * Display name for an MM8 race-class, as a stand-in until proper localization strings are added for MM8 (see
 * `Localization::className()` for the MM7 equivalent).
 *
 * @param race                          An MM8 race, as tested by `isMm8Race`.
 * @return                              English display name for the race-class.
 */
std::string mm8RaceName(Race race);

/**
 * Display name for an MM8 class (base or promoted), as a stand-in until proper localization strings are added
 * for MM8 (see `Localization::className()`, which routes MM8 classes here). Base classes share their race's name
 * (see `mm8RaceName`); promoted classes get their own promotion-quest reward name (e.g. `CLASS_MM8_LICH` ->
 * "Lich"), sourced from the "Class Skills.txt" table's race-class column headers.
 *
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @return                              English display name for the class.
 */
std::string mm8ClassName(Class classType);

/**
 * Starting-attribute range for a freshly-created MM8 character, as per the "Class Starting Stats" table.
 *
 * The table gives identical ranges for a race's base & promoted class (e.g. Necromancer == Lich), so this
 * struct is the same for `mm8BaseClassForRace(race)` and `mm8PromotedClassForRace(race)`.
 */
struct Mm8StartingStatRange {
    int base;               ///< Starting attribute value.
    int max;                ///< Maximum attribute value obtainable at character creation.
    int stepBelowBase;      ///< Points added/removed per click while the attribute is below `base` (the table's
                             ///< "+ add" value before the "/", or the whole value if there's no "/").
    int stepAtOrAboveBase;  ///< Points added/removed per click while the attribute is at or above `base` (the
                             ///< table's "+ add" value after the "/", or the whole value if there's no "/").

    friend bool operator==(const Mm8StartingStatRange &l, const Mm8StartingStatRange &r) = default;
};

/**
 * Returns the starting-attribute range for the given MM8 class & attribute, as per the "Class Starting Stats" table.
 *
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @param attribute                     One of the 7 basic stats, as tested by `allStatAttributes`.
 * @return                              Base/max/step values for a freshly-created character of the given class.
 */
Mm8StartingStatRange mm8StartingStats(Class classType, Attribute attribute);

/**
 * Base HP & SP progression for an MM8 class, as per the "Class HP SP" table.
 */
struct Mm8HpSpPerLevel {
    int hpBase;                        ///< HP at character creation (level 1), before per-level scaling.
    int hpFactor;                      ///< HP gained per level.
    int spBase;                        ///< SP at character creation (level 1), before per-level scaling.
    int spFactor;                      ///< SP gained per level.
    std::optional<Attribute> spAttribute;  ///< The attribute whose bonus adds to SP (the table's "SP Stats"
                                            ///< column), or `std::nullopt` for classes that don't cast spells
                                            ///< (Knight/Champion/Troll/WarTroll, the table's "-").

    friend bool operator==(const Mm8HpSpPerLevel &l, const Mm8HpSpPerLevel &r) = default;
};

/**
 * Returns base HP/SP-per-level values for the given MM8 class, as per the "Class HP SP" table.
 *
 * @param classType                     An MM8 class (base or promoted), as tested by `isMm8Class`.
 * @return                              HP/SP progression for the given class.
 */
Mm8HpSpPerLevel mm8BaseHpSpPerLevel(Class classType);

/**
 * @param type                          Engine-wide game type, as returned by `gameType()`.
 * @return                              The race-classes offered at character creation for the given game type:
 *                                      the 4 MM7 races under MM6/MM7, or the 7 startable MM8 race-classes
 *                                      (`RACE_MM8_DRAGON` excluded) under MM8.
 */
inline Segment<Race> startableRacesForGameType(GameType type) {
    return type == GAME_TYPE_MM8 ? allMm8StartableRaces() : Segment<Race>(RACE_FIRST, RACE_LAST);
}

// BodyType
//

inline Segment<BodyType> allBodyTypes() {
    return {BODY_TYPE_FIRST, BODY_TYPE_LAST};
}

/**
 * @param bodyType                      Body type to map.
 * @return                              Body type of a human of the same sex. Dwarves draw most belts and helms with
 *                                      these textures.
 */
inline BodyType humanBodyType(BodyType bodyType) {
    return bodyType == BODY_TYPE_HUMAN_MALE || bodyType == BODY_TYPE_DWARF_MALE ? BODY_TYPE_HUMAN_MALE : BODY_TYPE_HUMAN_FEMALE;
}


//
// CharacterAttributeType
//

/**
 * @return                              All attributes that can be improved though attribute item enchantments, like
 *                                      "of Might".
 */
inline Segment<Attribute> allEnchantableAttributes() {
    return {ATTRIBUTE_FIRST_ENCHANTABLE, ATTRIBUTE_LAST_ENCHANTABLE};
}

/**
 * @return                              Segment containing the 7 basic character stats (starting with might & ending
 *                                      with luck).
 */
inline Segment<Attribute> allStatAttributes() {
    return {ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT};
}
