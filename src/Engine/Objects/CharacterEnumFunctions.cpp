#include "CharacterEnumFunctions.h"

#include "Engine/Spells/SpellEnumFunctions.h"

PortraitId portraitForCondition(Condition condition) {
    switch (condition) {
    case CONDITION_CURSED:
        return PORTRAIT_CURSED;
    case CONDITION_WEAK:
        return PORTRAIT_WEAK;
    case CONDITION_SLEEP:
        return PORTRAIT_SLEEP;
    case CONDITION_FEAR:
        return PORTRAIT_FEAR;
    case CONDITION_DRUNK:
        return PORTRAIT_DRUNK;
    case CONDITION_INSANE:
        return PORTRAIT_INSANE;
    case CONDITION_POISON_WEAK:
    case CONDITION_POISON_MEDIUM:
    case CONDITION_POISON_SEVERE:
        return PORTRAIT_POISONED;
    case CONDITION_DISEASE_WEAK:
    case CONDITION_DISEASE_MEDIUM:
    case CONDITION_DISEASE_SEVERE:
        return PORTRAIT_DISEASED;
    case CONDITION_PARALYZED:
        return PORTRAIT_PARALYZED;
    case CONDITION_UNCONSCIOUS:
        return PORTRAIT_UNCONSCIOUS;
    case CONDITION_DEAD:
        return PORTRAIT_DEAD;
    case CONDITION_PETRIFIED:
        return PORTRAIT_PETRIFIED;
    case CONDITION_ERADICATED:
        return PORTRAIT_ERADICATED;
    default: // CONDITION_ZOMBIE & CONDITION_GOOD are handled externally.
        assert(false);
        return PORTRAIT_NORMAL;
    }
}


Skill skillForMagicSchool(MagicSchool school) {
    switch (school) {
    case MAGIC_SCHOOL_FIRE:     return SKILL_FIRE;
    case MAGIC_SCHOOL_AIR:      return SKILL_AIR;
    case MAGIC_SCHOOL_WATER:    return SKILL_WATER;
    case MAGIC_SCHOOL_EARTH:    return SKILL_EARTH;
    case MAGIC_SCHOOL_SPIRIT:   return SKILL_SPIRIT;
    case MAGIC_SCHOOL_MIND:     return SKILL_MIND;
    case MAGIC_SCHOOL_BODY:     return SKILL_BODY;
    case MAGIC_SCHOOL_LIGHT:    return SKILL_LIGHT;
    case MAGIC_SCHOOL_DARK:     return SKILL_DARK;
    default:
        assert(false);
        return SKILL_INVALID;
    }
}

Skill skillForSpell(SpellId spell) {
    if (isRegularSpell(spell)) {
        return skillForMagicSchool(magicSchoolForSpell(spell));
    } else if (spell == SPELL_BOW_ARROW) {
        return SKILL_BOW;
    } else if (spell == SPELL_LASER_PROJECTILE) {
        return SKILL_BLASTER;
    } else {
        assert(false && "Unknown spell");
        return SKILL_INVALID;
    }
}
