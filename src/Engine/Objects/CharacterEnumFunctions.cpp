#include "CharacterEnumFunctions.h"

#include "Engine/Spells/SpellEnumFunctions.h"

CharacterExpressionID expressionForCondition(Condition condition) {
    switch (condition) {
    case CONDITION_CURSED:
        return CHARACTER_EXPRESSION_CURSED;
    case CONDITION_WEAK:
        return CHARACTER_EXPRESSION_WEAK;
    case CONDITION_SLEEP:
        return CHARACTER_EXPRESSION_SLEEP;
    case CONDITION_FEAR:
        return CHARACTER_EXPRESSION_FEAR;
    case CONDITION_DRUNK:
        return CHARACTER_EXPRESSION_DRUNK;
    case CONDITION_INSANE:
        return CHARACTER_EXPRESSION_INSANE;
    case CONDITION_POISON_WEAK:
    case CONDITION_POISON_MEDIUM:
    case CONDITION_POISON_SEVERE:
        return CHARACTER_EXPRESSION_POISONED;
    case CONDITION_DISEASE_WEAK:
    case CONDITION_DISEASE_MEDIUM:
    case CONDITION_DISEASE_SEVERE:
        return CHARACTER_EXPRESSION_DISEASED;
    case CONDITION_PARALYZED:
        return CHARACTER_EXPRESSION_PARALYZED;
    case CONDITION_UNCONSCIOUS:
        return CHARACTER_EXPRESSION_UNCONCIOUS;
    case CONDITION_DEAD:
        return CHARACTER_EXPRESSION_DEAD;
    case CONDITION_PETRIFIED:
        return CHARACTER_EXPRESSION_PETRIFIED;
    case CONDITION_ERADICATED:
        return CHARACTER_EXPRESSION_ERADICATED;
    default: // CONDITION_ZOMBIE & CONDITION_GOOD are handled externally.
        assert(false);
        return CHARACTER_EXPRESSION_NORMAL;
    }
}


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
