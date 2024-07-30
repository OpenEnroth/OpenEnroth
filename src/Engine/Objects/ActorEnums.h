#pragma once

#include <cstdint>

#include "Utility/Flags.h"

enum class ActorKillCheckPolicy {
    KILL_CHECK_ANY = 0,
    KILL_CHECK_GROUPID = 1,
    KILL_CHECK_MONSTERID = 2,
    KILL_CHECK_ACTORID = 3
};
using enum ActorKillCheckPolicy;

enum class ActorAbility : char {
    ABILITY_ATTACK1 = 0,
    ABILITY_ATTACK2 = 1,
    ABILITY_SPELL1 = 2,
    ABILITY_SPELL2 = 3,
    ABILITY_SPECIAL = 4, // E.g. gogs exploding on death
};
using enum ActorAbility;

enum class ActorBuff {
    ACTOR_BUFF_NONE = 0,
    ACTOR_BUFF_CHARM = 1,
    ACTOR_BUFF_SUMMONED = 2,
    ACTOR_BUFF_SHRINK = 3,
    ACTOR_BUFF_AFRAID = 4,
    ACTOR_BUFF_STONED = 5,
    ACTOR_BUFF_PARALYZED = 6,
    ACTOR_BUFF_SLOWED = 7,
    ACTOR_BUFF_SOMETHING_THAT_HALVES_AC = 8,  // suspect gm axe effect??
    ACTOR_BUFF_BERSERK = 9,
    ACTOR_BUFF_MASS_DISTORTION = 10, // Fake debuff, was used in vanilla to animate the momentary shrinkage of a monster
                                     // when mass distortion is cast. Not used in OE, see Actor::massDistortionTime.
    ACTOR_BUFF_FATE = 11,
    ACTOR_BUFF_ENSLAVED = 12,
    ACTOR_BUFF_DAY_OF_PROTECTION = 13,
    ACTOR_BUFF_HOUR_OF_POWER = 14,
    ACTOR_BUFF_SHIELD = 15,
    ACTOR_BUFF_STONESKIN = 16,
    ACTOR_BUFF_BLESS = 17,
    ACTOR_BUFF_HEROISM = 18,
    ACTOR_BUFF_HASTE = 19,
    ACTOR_BUFF_PAIN_REFLECTION = 20,
    ACTOR_BUFF_HAMMERHANDS = 21,

    ACTOR_BUFF_FIRST = ACTOR_BUFF_CHARM,
    ACTOR_BUFF_LAST = ACTOR_BUFF_HAMMERHANDS
};
using enum ActorBuff;

// TODO(captainurist): #enum need to do some renamings here
enum class AIState : uint16_t {
    Standing = 0x0,
    Tethered = 0x1,
    AttackingMelee = 0x2,
    AttackingRanged1 = 0x3,
    Dying = 0x4,
    Dead = 0x5,
    Pursuing = 0x6,
    Fleeing = 0x7,
    Stunned = 0x8,
    Fidgeting = 0x9,
    Interacting = 10,
    Removed = 11,
    AttackingRanged2 = 0xC,
    AttackingRanged3 = 0xD,
    Stoned = 0xE,
    Paralyzed = 0xF,
    Resurrected = 16,
    Summoned = 17,
    AttackingRanged4 = 18,
    Disabled = 19, // TODO(captainurist): what is this? Document properly.
};
using enum AIState;

enum class ActorAnimation {
    ANIM_Standing = 0x0,
    ANIM_Walking = 0x1,
    ANIM_AtkMelee = 0x2,
    ANIM_AtkRanged = 0x3,
    ANIM_GotHit = 0x4,
    ANIM_Dying = 0x5,
    ANIM_Dead = 0x6,
    ANIM_Bored = 0x7,

    ANIM_First = ANIM_Standing,
    ANIM_Last = ANIM_Bored
};
using enum ActorAnimation;

enum class ActorAttribute : uint32_t {
    ACTOR_UNKNOWN_4 = 0x00000004,
    ACTOR_VISIBLE = 0x00000008,
    ACTOR_STAND_IN_QUEUE = 0x00000080,

    // nearest 30 actors to the players are put into this state - where the ai will attack/ flee/ cast spells/ wander
    // out of this state actors will idle on the spot
    ACTOR_FULL_AI_STATE = 0x00000400,

    ACTOR_ACTIVE = 0x00004000,  // actor ai has been activated
    ACTOR_NEARBY = 0x00008000,  // detected player / monster
    ACTOR_UNKNOW11 = 0x00010000,  // aidisabled?
    ACTOR_FLEEING = 0x00020000,
    ACTOR_LAST_SPELL_MISSED = 0x00040000,
    ACTOR_AGGRESSOR = 0x00080000,
    ACTOR_UNKNOW7 = 0x00100000,  // alertstatus ?
    ACTOR_ANIMATION = 0x00200000,
    ACTOR_UNKNOW9 = 0x00400000,  // actor has job - never finished feature
    ACTOR_HAS_ITEM = 0x00800000,
    ACTOR_HOSTILE = 0x01000000,
};
using enum ActorAttribute;
MM_DECLARE_FLAGS(ActorAttributes, ActorAttribute)
MM_DECLARE_OPERATORS_FOR_FLAGS(ActorAttributes)

enum class ActorSound {
    ACTOR_ATTACK_SOUND = 0,
    ACTOR_DEATH_SOUND = 1,
    ACTOR_STUNNED_SOUND = 2,
    ACTOR_BORED_SOUND = 3,

    ACTOR_SOUND_FIRST = ACTOR_ATTACK_SOUND,
    ACTOR_SOUND_LAST = ACTOR_BORED_SOUND,
};
using enum ActorSound;
