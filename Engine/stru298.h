#pragma once

#include <cstdint>

#include "VectorTypes.h"

/*  303 */
#pragma pack(push, 1)
struct stru298 {
    void Add(int16_t uID, int16_t a3, int16_t x, int16_t y, int16_t z, char a7, char a8);

    int count;
    int16_t pIDs[100];
    int16_t pXs[100];
    int16_t pYs[100];
    int16_t pZs[100];
    int16_t field_324[100];  // range?
    char field_3EC[100];
    char field_450[100];
    Vec3_int_ vec_4B4[100];
};
#pragma pack(pop)

extern stru298 AttackerInfo;  // for area of effect damage
