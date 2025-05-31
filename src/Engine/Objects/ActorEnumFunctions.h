#pragma once

#include <cassert>

#include "ActorEnums.h"
#include "Engine/Spells/SpellEnums.h"

inline SpellId spellForActorBuff(ActorBuff buff) {
    switch (buff) {
    default:
        assert(false);
        [[fallthrough]];
    case ACTOR_BUFF_NONE:                       return SPELL_NONE;
    case ACTOR_BUFF_CHARM:                      return SPELL_MIND_CHARM;
    case ACTOR_BUFF_SUMMONED:                   return SPELL_LIGHT_SUMMON_ELEMENTAL;
    case ACTOR_BUFF_SHRINK:                     return SPELL_DARK_SHRINKING_RAY;
    case ACTOR_BUFF_AFRAID:                     return SPELL_MIND_MASS_FEAR;
    case ACTOR_BUFF_STONED:                     return SPELL_LIGHT_PARALYZE;
    case ACTOR_BUFF_PARALYZED:                  return SPELL_LIGHT_PARALYZE;
    case ACTOR_BUFF_SLOWED:                     return SPELL_EARTH_SLOW;
    case ACTOR_BUFF_BERSERK:                    return SPELL_MIND_BERSERK;
    case ACTOR_BUFF_SOMETHING_THAT_HALVES_AC:   return SPELL_NONE;
    case ACTOR_BUFF_MASS_DISTORTION:            return SPELL_EARTH_MASS_DISTORTION;
    case ACTOR_BUFF_FATE:                       return SPELL_SPIRIT_FATE;
    case ACTOR_BUFF_ENSLAVED:                   return SPELL_MIND_ENSLAVE;
    case ACTOR_BUFF_DAY_OF_PROTECTION:          return SPELL_LIGHT_DAY_OF_PROTECTION;
    case ACTOR_BUFF_HOUR_OF_POWER:              return SPELL_LIGHT_HOUR_OF_POWER;
    case ACTOR_BUFF_SHIELD:                     return SPELL_AIR_SHIELD;
    case ACTOR_BUFF_STONESKIN:                  return SPELL_EARTH_STONESKIN;
    case ACTOR_BUFF_BLESS:                      return SPELL_SPIRIT_BLESS;
    case ACTOR_BUFF_HEROISM:                    return SPELL_SPIRIT_HEROISM;
    case ACTOR_BUFF_HASTE:                      return SPELL_FIRE_HASTE;
    case ACTOR_BUFF_PAIN_REFLECTION:            return SPELL_DARK_PAIN_REFLECTION;
    case ACTOR_BUFF_HAMMERHANDS:                return SPELL_BODY_HAMMERHANDS;
    }
}
