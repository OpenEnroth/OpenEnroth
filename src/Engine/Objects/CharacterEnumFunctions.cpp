#include "CharacterEnumFunctions.h"

#include "Engine/Spells/SpellEnumFunctions.h"

CharacterSkillType skillForMagicSchool(MagicSchool school) {
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

CharacterSkillType skillForSpell(SpellId spell) {
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
