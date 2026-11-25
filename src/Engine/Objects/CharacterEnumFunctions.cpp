#include "CharacterEnumFunctions.h"

#include <string>

#include "Engine/mm7_data.h"
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
    } else if (spell == SPELL_VAMPIRE_LEVITATE) {
        return SKILL_VAMPIRE;
    } else if (spell == SPELL_BLASTER_PROJECTILE) {
        return SKILL_BLASTER;
    } else {
        assert(false && "Unknown spell");
        return SKILL_INVALID;
    }
}

// Sourced from MM8's "Data/Tables/Class Starting Skills.txt". Skills not listed below (denied for every MM8
// race) are also denied here, including SKILL_CLUB/SKILL_MISC (hidden MM7 skills, not part of the MM8 table)
// and SKILL_DIPLOMACY/SKILL_THIEVERY (no MM8 class starts with them). The MM8-only "ability" skills
// (Regeneration, Dark Elf/Vampire/Dragon ability) are denied here: the races start them untrained and
// nothing offers them at character creation.
SkillAffinity mm8StartingSkillAffinity(Race race, Skill skill) {
    assert(isMm8Race(race));
    switch (race) {
    case RACE_MM8_NECROMANCER:
        switch (skill) {
        case SKILL_STAFF: return SKILL_AFFINITY_PRIMARY;
        case SKILL_DAGGER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_FIRE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_AIR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_WATER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_DARK: return SKILL_AFFINITY_PRIMARY;
        case SKILL_MEDITATION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MONSTER_ID: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ALCHEMY: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEARNING: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_CLERIC:
        switch (skill) {
        case SKILL_STAFF: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MACE: return SKILL_AFFINITY_PRIMARY;
        case SKILL_SHIELD: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SPIRIT: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MIND: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BODY: return SKILL_AFFINITY_PRIMARY;
        case SKILL_MERCHANT: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MEDITATION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_PERCEPTION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEARNING: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_KNIGHT:
        switch (skill) {
        case SKILL_SWORD: return SKILL_AFFINITY_PRIMARY;
        case SKILL_AXE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SPEAR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BOW: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SHIELD: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEATHER: return SKILL_AFFINITY_PRIMARY;
        case SKILL_CHAIN: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_REPAIR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BODYBUILDING: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_PERCEPTION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ARMSMASTER: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_TROLL:
        switch (skill) {
        case SKILL_STAFF: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SWORD: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_AXE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BOW: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MACE: return SKILL_AFFINITY_PRIMARY;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_REPAIR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BODYBUILDING: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MONSTER_ID: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ARMSMASTER: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_MINOTAUR:
        switch (skill) {
        case SKILL_STAFF: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_AXE: return SKILL_AFFINITY_PRIMARY;
        case SKILL_SPEAR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MACE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_CHAIN: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SPIRIT: return SKILL_AFFINITY_PRIMARY;
        case SKILL_BODYBUILDING: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_PERCEPTION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_TRAP_DISARM: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ARMSMASTER: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_DARK_ELF:
        switch (skill) {
        case SKILL_SWORD: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_DAGGER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_BOW: return SKILL_AFFINITY_PRIMARY;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_FIRE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_AIR: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ITEM_ID: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MERCHANT: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_PERCEPTION: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_TRAP_DISARM: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_VAMPIRE:
        switch (skill) {
        case SKILL_SWORD: return SKILL_AFFINITY_PRIMARY;
        case SKILL_DAGGER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_AXE: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_SHIELD: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_LEATHER: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MIND: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_TRAP_DISARM: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_MONSTER_ID: return SKILL_AFFINITY_AVAILABLE;
        case SKILL_ALCHEMY: return SKILL_AFFINITY_AVAILABLE;
        default: return SKILL_AFFINITY_DENIED;
        }
    case RACE_MM8_DRAGON:
        // Dragons fight with claws & breath, not trained weapon/armor/magic skills.
        return SKILL_AFFINITY_DENIED;
    default:
        assert(false);
        return SKILL_AFFINITY_DENIED;
    }
}

// Sourced from MM8's "Data/Tables/Class Skills.txt", which gives a per-class max mastery ('B' = Novice,
// 'E' = Expert, 'M' = Master, 'G' = Grandmaster, '-' = not learnable) for every skill. Skills not listed in
// the table (denied for every MM8 class) are also denied here, including SKILL_CLUB/SKILL_MISC/
// SKILL_DIPLOMACY/SKILL_THIEVERY (not part of the MM8 table, see `mm8StartingSkillAffinity`) and the four
// MM8-only "ability" rows (Regeneration, Dark Elf/Vampire/Dragon ability), handled first below.
Mastery mm8SkillMaxMastery(Class classType, Skill skill) {
    assert(isMm8Class(classType));
    switch (skill) {
    case SKILL_DARK_ELF:
        return classType == CLASS_MM8_DARK_ELF ? MASTERY_MASTER : classType == CLASS_MM8_PATRIARCH ? MASTERY_GRANDMASTER : MASTERY_NONE;
    case SKILL_VAMPIRE:
        return classType == CLASS_MM8_VAMPIRE ? MASTERY_MASTER : classType == CLASS_MM8_NOSFERATU ? MASTERY_GRANDMASTER : MASTERY_NONE;
    case SKILL_DRAGON:
        return classType == CLASS_MM8_DRAGON ? MASTERY_MASTER : classType == CLASS_MM8_GREAT_WYRM ? MASTERY_GRANDMASTER : MASTERY_NONE;
    case SKILL_REGENERATION:
        return classType == CLASS_MM8_TROLL ? MASTERY_MASTER : classType == CLASS_MM8_WAR_TROLL ? MASTERY_GRANDMASTER : MASTERY_NONE;
    default:
        break;
    }
    switch (classType) {
    case CLASS_MM8_NECROMANCER:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_FIRE: return MASTERY_MASTER;
        case SKILL_AIR: return MASTERY_MASTER;
        case SKILL_WATER: return MASTERY_MASTER;
        case SKILL_EARTH: return MASTERY_MASTER;
        case SKILL_DARK: return MASTERY_MASTER;
        case SKILL_ITEM_ID: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_MEDITATION: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_NOVICE;
        case SKILL_TRAP_DISARM: return MASTERY_NOVICE;
        case SKILL_MONSTER_ID: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_MASTER;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_LICH:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_FIRE: return MASTERY_GRANDMASTER;
        case SKILL_AIR: return MASTERY_GRANDMASTER;
        case SKILL_WATER: return MASTERY_GRANDMASTER;
        case SKILL_EARTH: return MASTERY_GRANDMASTER;
        case SKILL_DARK: return MASTERY_GRANDMASTER;
        case SKILL_ITEM_ID: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_MEDITATION: return MASTERY_GRANDMASTER;
        case SKILL_PERCEPTION: return MASTERY_NOVICE;
        case SKILL_TRAP_DISARM: return MASTERY_NOVICE;
        case SKILL_MONSTER_ID: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_GRANDMASTER;
        case SKILL_LEARNING: return MASTERY_GRANDMASTER;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_CLERIC:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_SHIELD: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_SPIRIT: return MASTERY_MASTER;
        case SKILL_MIND: return MASTERY_MASTER;
        case SKILL_BODY: return MASTERY_MASTER;
        case SKILL_LIGHT: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_MASTER;
        case SKILL_REPAIR: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_NOVICE;
        case SKILL_MEDITATION: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_ARMSMASTER: return MASTERY_NOVICE;
        case SKILL_ALCHEMY: return MASTERY_EXPERT;
        case SKILL_LEARNING: return MASTERY_EXPERT;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_PRIEST_OF_LIGHT:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_SHIELD: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_SPIRIT: return MASTERY_GRANDMASTER;
        case SKILL_MIND: return MASTERY_GRANDMASTER;
        case SKILL_BODY: return MASTERY_GRANDMASTER;
        case SKILL_LIGHT: return MASTERY_GRANDMASTER;
        case SKILL_MERCHANT: return MASTERY_MASTER;
        case SKILL_REPAIR: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_NOVICE;
        case SKILL_MEDITATION: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_ARMSMASTER: return MASTERY_NOVICE;
        case SKILL_ALCHEMY: return MASTERY_EXPERT;
        case SKILL_LEARNING: return MASTERY_EXPERT;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_KNIGHT:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_EXPERT;
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_MASTER;
        case SKILL_SPEAR: return MASTERY_MASTER;
        case SKILL_BOW: return MASTERY_EXPERT;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_SHIELD: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_MASTER;
        case SKILL_PLATE: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_REPAIR: return MASTERY_MASTER;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_TRAP_DISARM: return MASTERY_EXPERT;
        case SKILL_MONSTER_ID: return MASTERY_NOVICE;
        case SKILL_ARMSMASTER: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_CHAMPION:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_EXPERT;
        case SKILL_SWORD: return MASTERY_GRANDMASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_MASTER;
        case SKILL_SPEAR: return MASTERY_GRANDMASTER;
        case SKILL_BOW: return MASTERY_EXPERT;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_SHIELD: return MASTERY_GRANDMASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_MASTER;
        case SKILL_PLATE: return MASTERY_GRANDMASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_REPAIR: return MASTERY_GRANDMASTER;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_TRAP_DISARM: return MASTERY_EXPERT;
        case SKILL_MONSTER_ID: return MASTERY_NOVICE;
        case SKILL_ARMSMASTER: return MASTERY_GRANDMASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_TROLL:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_MASTER;
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_MASTER;
        case SKILL_SPEAR: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_MASTER;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_MERCHANT: return MASTERY_NOVICE;
        case SKILL_REPAIR: return MASTERY_MASTER;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_NOVICE;
        case SKILL_TRAP_DISARM: return MASTERY_NOVICE;
        case SKILL_MONSTER_ID: return MASTERY_EXPERT;
        case SKILL_ARMSMASTER: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_WAR_TROLL:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_GRANDMASTER;
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_MASTER;
        case SKILL_SPEAR: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_GRANDMASTER;
        case SKILL_LEATHER: return MASTERY_GRANDMASTER;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_MERCHANT: return MASTERY_NOVICE;
        case SKILL_REPAIR: return MASTERY_MASTER;
        case SKILL_BODYBUILDING: return MASTERY_GRANDMASTER;
        case SKILL_PERCEPTION: return MASTERY_NOVICE;
        case SKILL_TRAP_DISARM: return MASTERY_NOVICE;
        case SKILL_MONSTER_ID: return MASTERY_EXPERT;
        case SKILL_ARMSMASTER: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_MINOTAUR:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_MASTER;
        case SKILL_SWORD: return MASTERY_EXPERT;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_MASTER;
        case SKILL_SPEAR: return MASTERY_MASTER;
        case SKILL_BOW: return MASTERY_EXPERT;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_MASTER;
        case SKILL_PLATE: return MASTERY_MASTER;
        case SKILL_SPIRIT: return MASTERY_NOVICE;
        case SKILL_MIND: return MASTERY_NOVICE;
        case SKILL_BODY: return MASTERY_NOVICE;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_MASTER;
        case SKILL_TRAP_DISARM: return MASTERY_MASTER;
        case SKILL_ARMSMASTER: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_NOVICE;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_MINOTAUR_LORD:
        switch (skill) {
        case SKILL_STAFF: return MASTERY_MASTER;
        case SKILL_SWORD: return MASTERY_EXPERT;
        case SKILL_DAGGER: return MASTERY_EXPERT;
        case SKILL_AXE: return MASTERY_GRANDMASTER;
        case SKILL_SPEAR: return MASTERY_MASTER;
        case SKILL_BOW: return MASTERY_EXPERT;
        case SKILL_MACE: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_EXPERT;
        case SKILL_CHAIN: return MASTERY_MASTER;
        case SKILL_PLATE: return MASTERY_MASTER;
        case SKILL_SPIRIT: return MASTERY_EXPERT;
        case SKILL_MIND: return MASTERY_EXPERT;
        case SKILL_BODY: return MASTERY_EXPERT;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_GRANDMASTER;
        case SKILL_TRAP_DISARM: return MASTERY_MASTER;
        case SKILL_ARMSMASTER: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_NOVICE;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_DARK_ELF:
        switch (skill) {
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_MASTER;
        case SKILL_BOW: return MASTERY_MASTER;
        case SKILL_MACE: return MASTERY_NOVICE;
        case SKILL_SHIELD: return MASTERY_EXPERT;
        case SKILL_LEATHER: return MASTERY_MASTER;
        case SKILL_CHAIN: return MASTERY_MASTER;
        case SKILL_FIRE: return MASTERY_EXPERT;
        case SKILL_AIR: return MASTERY_EXPERT;
        case SKILL_WATER: return MASTERY_EXPERT;
        case SKILL_EARTH: return MASTERY_EXPERT;
        case SKILL_ITEM_ID: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_MASTER;
        case SKILL_REPAIR: return MASTERY_EXPERT;
        case SKILL_MEDITATION: return MASTERY_EXPERT;
        case SKILL_PERCEPTION: return MASTERY_MASTER;
        case SKILL_TRAP_DISARM: return MASTERY_MASTER;
        case SKILL_ARMSMASTER: return MASTERY_EXPERT;
        case SKILL_ALCHEMY: return MASTERY_EXPERT;
        case SKILL_LEARNING: return MASTERY_EXPERT;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_PATRIARCH:
        switch (skill) {
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_MASTER;
        case SKILL_BOW: return MASTERY_GRANDMASTER;
        case SKILL_MACE: return MASTERY_NOVICE;
        case SKILL_SHIELD: return MASTERY_EXPERT;
        case SKILL_LEATHER: return MASTERY_MASTER;
        case SKILL_CHAIN: return MASTERY_GRANDMASTER;
        case SKILL_FIRE: return MASTERY_MASTER;
        case SKILL_AIR: return MASTERY_MASTER;
        case SKILL_WATER: return MASTERY_MASTER;
        case SKILL_EARTH: return MASTERY_MASTER;
        case SKILL_ITEM_ID: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_GRANDMASTER;
        case SKILL_REPAIR: return MASTERY_EXPERT;
        case SKILL_MEDITATION: return MASTERY_EXPERT;
        case SKILL_PERCEPTION: return MASTERY_MASTER;
        case SKILL_TRAP_DISARM: return MASTERY_GRANDMASTER;
        case SKILL_ARMSMASTER: return MASTERY_EXPERT;
        case SKILL_ALCHEMY: return MASTERY_EXPERT;
        case SKILL_LEARNING: return MASTERY_EXPERT;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_VAMPIRE:
        switch (skill) {
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_MASTER;
        case SKILL_AXE: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_EXPERT;
        case SKILL_SHIELD: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_MASTER;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_SPIRIT: return MASTERY_EXPERT;
        case SKILL_MIND: return MASTERY_EXPERT;
        case SKILL_BODY: return MASTERY_EXPERT;
        case SKILL_ITEM_ID: return MASTERY_EXPERT;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_TRAP_DISARM: return MASTERY_EXPERT;
        case SKILL_MONSTER_ID: return MASTERY_MASTER;
        case SKILL_ARMSMASTER: return MASTERY_NOVICE;
        case SKILL_ALCHEMY: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_NOSFERATU:
        switch (skill) {
        case SKILL_SWORD: return MASTERY_MASTER;
        case SKILL_DAGGER: return MASTERY_GRANDMASTER;
        case SKILL_AXE: return MASTERY_EXPERT;
        case SKILL_BOW: return MASTERY_NOVICE;
        case SKILL_MACE: return MASTERY_EXPERT;
        case SKILL_SHIELD: return MASTERY_MASTER;
        case SKILL_LEATHER: return MASTERY_MASTER;
        case SKILL_CHAIN: return MASTERY_EXPERT;
        case SKILL_SPIRIT: return MASTERY_MASTER;
        case SKILL_MIND: return MASTERY_MASTER;
        case SKILL_BODY: return MASTERY_MASTER;
        case SKILL_ITEM_ID: return MASTERY_EXPERT;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_PERCEPTION: return MASTERY_EXPERT;
        case SKILL_TRAP_DISARM: return MASTERY_EXPERT;
        case SKILL_MONSTER_ID: return MASTERY_GRANDMASTER;
        case SKILL_ARMSMASTER: return MASTERY_NOVICE;
        case SKILL_ALCHEMY: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_NOVICE;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_DRAGON:
        switch (skill) {
        case SKILL_ITEM_ID: return MASTERY_MASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_MEDITATION: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_MASTER;
        case SKILL_MONSTER_ID: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_MASTER;
        default: return MASTERY_NONE;
        }
    case CLASS_MM8_GREAT_WYRM:
        switch (skill) {
        case SKILL_ITEM_ID: return MASTERY_GRANDMASTER;
        case SKILL_MERCHANT: return MASTERY_EXPERT;
        case SKILL_BODYBUILDING: return MASTERY_MASTER;
        case SKILL_MEDITATION: return MASTERY_MASTER;
        case SKILL_PERCEPTION: return MASTERY_GRANDMASTER;
        case SKILL_MONSTER_ID: return MASTERY_MASTER;
        case SKILL_ALCHEMY: return MASTERY_MASTER;
        case SKILL_LEARNING: return MASTERY_GRANDMASTER;
        default: return MASTERY_NONE;
        }
    default:
        assert(false);
        return MASTERY_NONE;
    }
}

Mastery skillMaxMasteryForClass(Class classType, Skill skill) {
    if (isMm8Class(classType)) {
        return mm8SkillMaxMastery(classType, skill);
    }
    return skillMaxMasteryPerClass[classType][skill];
}

std::string mm8RaceName(Race race) {
    assert(isMm8Race(race));
    switch (race) {
    case RACE_MM8_NECROMANCER: return "Necromancer";
    case RACE_MM8_CLERIC: return "Cleric";
    case RACE_MM8_KNIGHT: return "Knight";
    case RACE_MM8_TROLL: return "Troll";
    case RACE_MM8_MINOTAUR: return "Minotaur";
    case RACE_MM8_DARK_ELF: return "Dark Elf";
    case RACE_MM8_VAMPIRE: return "Vampire";
    case RACE_MM8_DRAGON: return "Dragon";
    default:
        assert(false);
        return std::string();
    }
}

std::string mm8ClassName(Class classType) {
    assert(isMm8Class(classType));
    if (isMm8BaseClass(classType)) {
        return mm8RaceName(mm8RaceForClass(classType));
    }
    switch (classType) {
    case CLASS_MM8_LICH: return "Lich";
    case CLASS_MM8_PRIEST_OF_LIGHT: return "Priest of the Sun";
    case CLASS_MM8_CHAMPION: return "Champion";
    case CLASS_MM8_WAR_TROLL: return "War Troll";
    case CLASS_MM8_MINOTAUR_LORD: return "Minotaur Lord";
    case CLASS_MM8_PATRIARCH: return "Patriarch";
    case CLASS_MM8_NOSFERATU: return "Nosferatu";
    case CLASS_MM8_GREAT_WYRM: return "Great Wyrm";
    default:
        assert(false);
        return std::string();
    }
}

// Sourced from MM8's "Data/Tables/Class Starting Stats.txt". The table lists a base/max pair and a "+ add" step
// per race-class column, with a race's base & promoted class (e.g. Necromancer & Lich) always sharing identical
// values, so this switches on the race rather than on all 16 classes.
Mm8StartingStatRange mm8StartingStats(Class classType, Attribute attribute) {
    assert(isMm8Class(classType));
    assert(attribute >= ATTRIBUTE_FIRST_STAT && attribute <= ATTRIBUTE_LAST_STAT);

    switch (mm8RaceForClass(classType)) {
    case RACE_MM8_NECROMANCER:
    case RACE_MM8_CLERIC:
    case RACE_MM8_KNIGHT:
        return {11, 25, 1, 1};
    case RACE_MM8_TROLL:
        switch (attribute) {
        case ATTRIBUTE_MIGHT: return {14, 35, 2, 2};
        case ATTRIBUTE_INTELLIGENCE: return {7, 15, 1, 2};
        case ATTRIBUTE_PERSONALITY: return {7, 15, 1, 2};
        case ATTRIBUTE_ENDURANCE: return {14, 35, 2, 2};
        default: return {11, 25, 1, 1};
        }
    case RACE_MM8_MINOTAUR:
        switch (attribute) {
        case ATTRIBUTE_MIGHT: return {14, 30, 2, 2};
        case ATTRIBUTE_INTELLIGENCE: return {7, 20, 1, 2};
        default: return {11, 25, 1, 1};
        }
    case RACE_MM8_DARK_ELF:
        switch (attribute) {
        case ATTRIBUTE_ENDURANCE: return {7, 20, 1, 2};
        case ATTRIBUTE_ACCURACY: return {14, 30, 2, 2};
        default: return {11, 25, 1, 1};
        }
    case RACE_MM8_VAMPIRE:
    case RACE_MM8_DRAGON:
        return {11, 30, 1, 1};
    default:
        assert(false);
        return {11, 25, 1, 1};
    }
}

// Sourced from MM8's "Data/Tables/Class HP SP.txt". Unlike the starting-stats table, HP/SP progression genuinely
// differs between a race's base & promoted class, so this switches on all 16 classes.
Mm8HpSpPerLevel mm8BaseHpSpPerLevel(Class classType) {
    assert(isMm8Class(classType));
    switch (classType) {
    case CLASS_MM8_NECROMANCER: return {20, 2, 25, 3, ATTRIBUTE_INTELLIGENCE};
    case CLASS_MM8_LICH:        return {20, 3, 25, 5, ATTRIBUTE_INTELLIGENCE};
    case CLASS_MM8_CLERIC:      return {30, 2, 15, 3, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_PRIEST_OF_LIGHT: return {30, 3, 15, 5, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_KNIGHT:      return {35, 5, 0, 0, std::nullopt};
    case CLASS_MM8_CHAMPION:    return {35, 8, 0, 0, std::nullopt};
    case CLASS_MM8_TROLL:       return {45, 5, 0, 0, std::nullopt};
    case CLASS_MM8_WAR_TROLL:   return {45, 8, 0, 0, std::nullopt};
    case CLASS_MM8_MINOTAUR:    return {30, 4, 5, 1, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_MINOTAUR_LORD: return {30, 6, 5, 2, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_DARK_ELF:    return {25, 3, 10, 2, ATTRIBUTE_INTELLIGENCE};
    case CLASS_MM8_PATRIARCH:   return {25, 4, 10, 4, ATTRIBUTE_INTELLIGENCE};
    case CLASS_MM8_VAMPIRE:     return {30, 3, 10, 2, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_NOSFERATU:   return {30, 4, 10, 4, ATTRIBUTE_PERSONALITY};
    case CLASS_MM8_DRAGON:      return {50, 10, 50, 5, ATTRIBUTE_INTELLIGENCE};
    case CLASS_MM8_GREAT_WYRM:  return {50, 10, 50, 10, ATTRIBUTE_INTELLIGENCE};
    default:
        assert(false);
        return {0, 0, 0, 0, std::nullopt};
    }
}
