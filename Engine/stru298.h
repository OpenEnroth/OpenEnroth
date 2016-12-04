#pragma once
#include "VectorTypes.h"

/*  303 */
#pragma pack(push, 1)
struct stru298
{
  void Add(__int16 uID, __int16 a3, __int16 x, __int16 y, __int16 z, char a7, char a8);

  int count;
  __int16 pIDs[100];
  __int16 pXs[100];
  __int16 pYs[100];
  __int16 pZs[100];
  __int16 field_324[100];
  char field_3EC[100];
  char field_450[100];
  Vec3_int_ vec_4B4[100];
};
#pragma pack(pop)

extern stru298 AttackerInfo; // weak