#include "Engine/AttackList.h"
#include "Engine/Engine.h"

std::vector<AttackDescription> attackList;

// Original function was replaced
//----- (0040261D) --------------------------------------------------------
// void stru298::Add(int16_t uID, int16_t a3, int16_t x, int16_t y, int16_t z, ABILITY_INDEX a7, char a8);

void pushMeleeAttack(Pid pid, Vec3i pos, ActorAbility ability) {
    // Registered melee attack use the same distance as ranged attacks
    int effectDistance = engine->config->gameplay.RangedAttackDepth.value();
    attackList.push_back({pid, pos, effectDistance, true, ability});
}

void pushAoeAttack(Pid pid, int aoeDistance, Vec3i pos, ActorAbility ability) {
    attackList.push_back({pid, pos, aoeDistance, false, ability});
}
