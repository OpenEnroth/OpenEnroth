#include "Engine/stru298.h"
#include "Engine/Engine.h"

std::vector<AttackDescription> attackList;

// Original function was replaced
//----- (0040261D) --------------------------------------------------------
// void stru298::Add(int16_t uID, int16_t a3, int16_t x, int16_t y, int16_t z, ABILITY_INDEX a7, char a8);

void pushMeleeAttack(int16_t uID, Vec3i pos, ABILITY_INDEX ability) {
    // Registered melee attack use the same distance as ranged attacks
    attackList.push_back({uID, pos, engine->config->gameplay.RangedAttackDepth.Get(), true, ability});
}

void pushAoeAttack(int16_t uID, int aoeDistance, Vec3i pos, ABILITY_INDEX ability) {
    attackList.push_back({uID, pos, aoeDistance, false, ability});
}
