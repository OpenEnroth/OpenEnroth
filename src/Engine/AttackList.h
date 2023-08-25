#pragma once

#include <cstdint>
#include <vector>
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Pid.h"
#include "Utility/Geometry/Vec.h"

enum class ABILITY_INDEX : char;

struct AttackDescription {
    Pid pid;
    Vec3i pos;
    int attackRange;
    bool isMelee; // Melee attack or magic AOE
    ABILITY_INDEX attackSpecial; // special ability
    //Vec3i attackVector; // no point to have it inside struct
};

extern std::vector<AttackDescription> attackList;  // for area of effect damage

/**
 * Register damaging AOE spell impact.
 *
 * @param pid                           `Pid` of an AOE spell.
 * @param aoeDistance                   Damaging distance of spell relative to point of impact.
 * @param pos                           Point of impact.
 * @param ability                       ???
 * @offset 0x40261D
 */
extern void pushAoeAttack(Pid pid, int aoeDistance, Vec3i pos, ABILITY_INDEX ability);

/**
 * Register melee attack performed by an actor.
 *
 * @param pid                           `Pid` of attacker.
 * @param pos                           Position of an attacker.
 * @param ability                       ???
 */
extern void pushMeleeAttack(Pid pid, Vec3i pos, ABILITY_INDEX ability);
