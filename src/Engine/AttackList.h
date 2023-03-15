#pragma once

#include <cstdint>
#include <vector>

#include "Engine/Objects/Actor.h"

#pragma pack(push, 1)
struct AttackDescription {
    int16_t id;
    Vec3i pos;
    int attackRange;
    bool attackType; // melee = 1 / spells = 0
    ABILITY_INDEX attackSpecial; // special ability
    //Vec3i attackVector; // no point to have it inside struct
};
#pragma pack(pop)

extern std::vector<AttackDescription> attackList;  // for area of effect damage

extern void pushMeleeAttack(int16_t uID, Vec3i pos, ABILITY_INDEX ability);
extern void pushAoeAttack(int16_t uID, int aoeDistance, Vec3i pos, ABILITY_INDEX ability);
