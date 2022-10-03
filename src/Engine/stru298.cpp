#include "Engine/stru298.h"

//----- (0040261D) --------------------------------------------------------
void stru298::Add(int16_t uID, int16_t a3, int16_t x, int16_t y, int16_t z,
                  ABILITY_INDEX a7, char a8) {
    if (count < 100) {
        pIDs[count] = uID;
        pXs[count] = x;
        pYs[count] = y;
        pZs[count] = z;
        attack_range[count] = a3;  // a3 = 5120 for melee attack (attack range?)
        attack_type[count] = a8;  // a8 = 1 for melee / 0 for spells
        attack_special[count++] = a7;  // a7 = special ability
    }
}
