#pragma once

#include <cstdint>

enum class ABILITY_INDEX : char {
    ABILITY_ATTACK1 = 0,
    ABILITY_ATTACK2 = 1,
    ABILITY_SPELL1 = 2,
    ABILITY_SPELL2 = 3,
    ABILITY_SPECIAL = 4, // E.g. gogs exploding on death
};
using enum ABILITY_INDEX;

/*  361 */
enum class ACTOR_BUFF_INDEX {
    ACTOR_BUFF_CHARM = 1,
    ACTOR_BUFF_SUMMONED = 2,
    ACTOR_BUFF_SHRINK = 3,
    ACTOR_BUFF_AFRAID = 4,
    ACTOR_BUFF_STONED = 5,
    ACTOR_BUFF_PARALYZED = 6,
    ACTOR_BUFF_SLOWED = 7,
    ACTOR_BUFF_SOMETHING_THAT_HALVES_AC = 8,  // suspect gm axe effect??
    ACTOR_BUFF_BERSERK = 9,
    ACTOR_BUFF_MASS_DISTORTION = 10,
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
    ACTOR_BUFF_PAIN_HAMMERHANDS = 21,
    ACTOR_BUFF_COUNT
};
using enum ACTOR_BUFF_INDEX;

/*  295 */
enum class ObjectType {
    OBJECT_Any = 0x0,
    OBJECT_BLVDoor = 0x1,
    OBJECT_Item = 0x2,
    OBJECT_Actor = 0x3,
    OBJECT_Player = 0x4,
    OBJECT_Decoration = 0x5,
    OBJECT_BModel = 0x6,
};
using enum ObjectType;

/*  264 */
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
    Disabled = 19,
};
using enum AIState;

/*  265 */
enum class ActorAnimation {
    ANIM_Standing = 0x0,
    ANIM_Walking = 0x1,
    ANIM_AtkMelee = 0x2,
    ANIM_AtkRanged = 0x3,
    ANIM_GotHit = 0x4,
    ANIM_Dying = 0x5,
    ANIM_Dead = 0x6,
    ANIM_Bored = 0x7,
    ANIM_Count
};
using enum ActorAnimation;

#define ACTOR_VISIBLE 0x00000008
#define ACTOR_STAND_IN_QUEUE 0x00000080

// nearest 30 actors to the players are put into this state - where the ai will attack/ flee/ cast spells/ wander
// out of this state actors will idle on the spot
#define ACTOR_FULL_AI_STATE 0x00000400

#define ACTOR_ACTIVE 0x00004000  // actor ai has been activated
#define ACTOR_NEARBY 0x00008000  // detected player / monster
#define ACTOR_UNKNOW11 0x00010000  // aidisabled?
#define ACTOR_FLEEING 0x00020000
#define ACTOR_UNKNOW5 0x00040000  // casting??
#define ACTOR_AGGRESSOR 0x00080000
#define ACTOR_UNKNOW7 0x00100000  // alertstatus ?
#define ACTOR_ANIMATION 0x00200000
#define ACTOR_UNKNOW9 0x00400000  // actor has job - never finished feature
#define ACTOR_HAS_ITEM 0x00800000
#define ACTOR_HOSTILE 0x01000000

