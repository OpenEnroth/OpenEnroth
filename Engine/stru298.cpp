#include "Engine/stru298.h"

//----- (0040261D) --------------------------------------------------------
void stru298::Add(int16_t uID, int16_t a3, int16_t x, int16_t y, int16_t z,
                  char a7, char a8) {
    if (count < 100) {
        pIDs[count] = uID;
        pXs[count] = x;
        pYs[count] = y;
        pZs[count] = z;
        field_324[count] = a3;
        field_3EC[count] = a8;
        field_450[count++] = a7;
    }
}
