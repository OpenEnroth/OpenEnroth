#pragma once

#include <cstdint>
#include <cassert>

#include "Engine/Objects/CharacterEnums.h"

#include "Utility/Segment.h"

enum class SpellId : uint8_t {
    SPELL_NONE = 0,

    SPELL_FIRE_TORCH_LIGHT = 1,
    SPELL_FIRE_FIRE_BOLT = 2,
    SPELL_FIRE_PROTECTION_FROM_FIRE = 3,
    SPELL_FIRE_FIRE_AURA = 4,
    SPELL_FIRE_HASTE = 5,
    SPELL_FIRE_FIREBALL = 6,
    SPELL_FIRE_FIRE_SPIKE = 7,
    SPELL_FIRE_IMMOLATION = 8,
    SPELL_FIRE_METEOR_SHOWER = 9,
    SPELL_FIRE_INFERNO = 10,
    SPELL_FIRE_INCINERATE = 11,

    SPELL_AIR_WIZARD_EYE = 12,
    SPELL_AIR_FEATHER_FALL = 13,
    SPELL_AIR_PROTECTION_FROM_AIR = 14,
    SPELL_AIR_SPARKS = 15,
    SPELL_AIR_JUMP = 16,
    SPELL_AIR_SHIELD = 17,
    SPELL_AIR_LIGHTNING_BOLT = 18,
    SPELL_AIR_INVISIBILITY = 19,
    SPELL_AIR_IMPLOSION = 20,
    SPELL_AIR_FLY = 21,
    SPELL_AIR_STARBURST = 22,

    SPELL_WATER_AWAKEN = 23,
    SPELL_WATER_POISON_SPRAY = 24,
    SPELL_WATER_PROTECTION_FROM_WATER = 25,
    SPELL_WATER_ICE_BOLT = 26,
    SPELL_WATER_WATER_WALK = 27,
    SPELL_WATER_RECHARGE_ITEM = 28,
    SPELL_WATER_ACID_BURST = 29,
    SPELL_WATER_ENCHANT_ITEM = 30,
    SPELL_WATER_TOWN_PORTAL = 31,
    SPELL_WATER_ICE_BLAST = 32,
    SPELL_WATER_LLOYDS_BEACON = 33,

    SPELL_EARTH_STUN = 34,
    SPELL_EARTH_SLOW = 35,
    SPELL_EARTH_PROTECTION_FROM_EARTH = 36,
    SPELL_EARTH_DEADLY_SWARM = 37,
    SPELL_EARTH_STONESKIN = 38,
    SPELL_EARTH_BLADES = 39,
    SPELL_EARTH_STONE_TO_FLESH = 40,
    SPELL_EARTH_ROCK_BLAST = 41,
    SPELL_EARTH_TELEKINESIS = 42,
    SPELL_EARTH_DEATH_BLOSSOM = 43,
    SPELL_EARTH_MASS_DISTORTION = 44,

    SPELL_SPIRIT_DETECT_LIFE = 45,
    SPELL_SPIRIT_BLESS = 46,
    SPELL_SPIRIT_FATE = 47,
    SPELL_SPIRIT_TURN_UNDEAD = 48,
    SPELL_SPIRIT_REMOVE_CURSE = 49,
    SPELL_SPIRIT_PRESERVATION = 50,
    SPELL_SPIRIT_HEROISM = 51,
    SPELL_SPIRIT_SPIRIT_LASH = 52,
    SPELL_SPIRIT_RAISE_DEAD = 53,
    SPELL_SPIRIT_SHARED_LIFE = 54,
    SPELL_SPIRIT_RESSURECTION = 55,

    SPELL_MIND_REMOVE_FEAR = 56,
    SPELL_MIND_MIND_BLAST = 57,
    SPELL_MIND_PROTECTION_FROM_MIND = 58,
    SPELL_MIND_TELEPATHY = 59,
    SPELL_MIND_CHARM = 60,
    SPELL_MIND_CURE_PARALYSIS = 61,
    SPELL_MIND_BERSERK = 62,
    SPELL_MIND_MASS_FEAR = 63,
    SPELL_MIND_CURE_INSANITY = 64,
    SPELL_MIND_PSYCHIC_SHOCK = 65,
    SPELL_MIND_ENSLAVE = 66,

    SPELL_BODY_CURE_WEAKNESS = 67,
    SPELL_BODY_FIRST_AID = 68,
    SPELL_BODY_PROTECTION_FROM_BODY = 69,
    SPELL_BODY_HARM = 70,
    SPELL_BODY_REGENERATION = 71,
    SPELL_BODY_CURE_POISON = 72,
    SPELL_BODY_HAMMERHANDS = 73,
    SPELL_BODY_CURE_DISEASE = 74,
    SPELL_BODY_PROTECTION_FROM_MAGIC = 75,
    SPELL_BODY_FLYING_FIST = 76,
    SPELL_BODY_POWER_CURE = 77,

    SPELL_LIGHT_LIGHT_BOLT = 78,
    SPELL_LIGHT_DESTROY_UNDEAD = 79,
    SPELL_LIGHT_DISPEL_MAGIC = 80,
    SPELL_LIGHT_PARALYZE = 81,
    SPELL_LIGHT_SUMMON_ELEMENTAL = 82,
    SPELL_LIGHT_DAY_OF_THE_GODS = 83,
    SPELL_LIGHT_PRISMATIC_LIGHT = 84,
    SPELL_LIGHT_DAY_OF_PROTECTION = 85,
    SPELL_LIGHT_HOUR_OF_POWER = 86,
    SPELL_LIGHT_SUNRAY = 87,
    SPELL_LIGHT_DIVINE_INTERVENTION = 88,

    SPELL_DARK_REANIMATE = 89,
    SPELL_DARK_TOXIC_CLOUD = 90,
    SPELL_DARK_VAMPIRIC_WEAPON = 91,
    SPELL_DARK_SHRINKING_RAY = 92,
    SPELL_DARK_SHARPMETAL = 93,
    SPELL_DARK_CONTROL_UNDEAD = 94,
    SPELL_DARK_PAIN_REFLECTION = 95,
    SPELL_DARK_SACRIFICE = 96,
    SPELL_DARK_DRAGON_BREATH = 97,
    SPELL_DARK_ARMAGEDDON = 98,
    SPELL_DARK_SOULDRINKER = 99,

    SPELL_BOW_ARROW = 100,
    SPELL_101 = 101,
    SPELL_LASER_PROJECTILE = 102,

    SPELL_FIRST_REGULAR = SPELL_FIRE_TORCH_LIGHT,
    SPELL_LAST_REGULAR = SPELL_DARK_SOULDRINKER,

    SPELL_FIRST_WITH_SPRITE = SPELL_FIRE_TORCH_LIGHT,
    SPELL_LAST_WITH_SPRITE = SPELL_LASER_PROJECTILE,

    BECOME_MAGIC_GUILD_MEMBER = 150,
    SPELL_QUEST_COMPLETED = 151,
    SPELL_STAT_DECREASE = 152, // used for face overlay when something is subtracted from character like stat/res/gold/condition etc.
    SPELL_DISEASE = 153
};
using enum SpellId;

/**
 * @return                              All regular spell types.
 */
inline Segment<SpellId> allRegularSpells() {
    return {SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR};
}

/**
 * Is spell target is item in inventory?
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

/**
 * Magic school, note that order corresponds to the enum order in `SPELL_TYPE`.
 */
enum class MagicSchool {
    MAGIC_SCHOOL_FIRE = 0,
    MAGIC_SCHOOL_AIR = 1,
    MAGIC_SCHOOL_WATER = 2,
    MAGIC_SCHOOL_EARTH = 3,
    MAGIC_SCHOOL_SPIRIT = 4,
    MAGIC_SCHOOL_MIND = 5,
    MAGIC_SCHOOL_BODY = 6,
    MAGIC_SCHOOL_LIGHT = 7,
    MAGIC_SCHOOL_DARK = 8,

    MAGIC_SCHOOL_FIRST = MAGIC_SCHOOL_FIRE,
    MAGIC_SCHOOL_LAST = MAGIC_SCHOOL_DARK
};
using enum MagicSchool;

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

inline CharacterSkillType skillForMagicSchool(MagicSchool school) {
    switch (school) {
    case MAGIC_SCHOOL_FIRE:     return CHARACTER_SKILL_FIRE;
    case MAGIC_SCHOOL_AIR:      return CHARACTER_SKILL_AIR;
    case MAGIC_SCHOOL_WATER:    return CHARACTER_SKILL_WATER;
    case MAGIC_SCHOOL_EARTH:    return CHARACTER_SKILL_EARTH;
    case MAGIC_SCHOOL_SPIRIT:   return CHARACTER_SKILL_SPIRIT;
    case MAGIC_SCHOOL_MIND:     return CHARACTER_SKILL_MIND;
    case MAGIC_SCHOOL_BODY:     return CHARACTER_SKILL_BODY;
    case MAGIC_SCHOOL_LIGHT:    return CHARACTER_SKILL_LIGHT;
    case MAGIC_SCHOOL_DARK:     return CHARACTER_SKILL_DARK;
    default:
        assert(false);
        return CHARACTER_SKILL_INVALID;
    }
}

inline CharacterSkillType skillForSpell(SpellId spell) {
    if (isRegularSpell(spell)) {
        return skillForMagicSchool(magicSchoolForSpell(spell));
    } else if (spell == SPELL_BOW_ARROW) {
        return CHARACTER_SKILL_BOW;
    } else if (spell == SPELL_LASER_PROJECTILE) {
        return CHARACTER_SKILL_BLASTER;
    } else {
        assert(false && "Unknown spell");
        return CHARACTER_SKILL_INVALID;
    }
}
