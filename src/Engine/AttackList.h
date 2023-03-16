#pragma once

#include <cstdint>
#include <vector>

#include "Engine/Objects/Actor.h"

struct AttackDescription {
    int16_t pid;
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
 * @param   pid            PID of an AOE spell
 * @param   aoeDistance    Damaging distance of spell relative to point of impact
 * @param   pos            Point of impact
 * @param   ability        ???
 * @offset 0x40261D
 */
extern void pushAoeAttack(int16_t pid, int aoeDistance, Vec3i pos, ABILITY_INDEX ability);

/**
 * Register melee attack performed by an actor.
 *
 * @param   pid            PID of attacker
 * @param   pos            Position of an attacker
 * @param   ability        ???
 */
extern void pushMeleeAttack(int16_t pid, Vec3i pos, ABILITY_INDEX ability);
