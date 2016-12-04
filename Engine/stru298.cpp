#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "stru298.h"


//----- (0040261D) --------------------------------------------------------
void stru298::Add(__int16 uID, __int16 a3, __int16 x, __int16 y, __int16 z, char a7, char a8)
{
	if (count < 100)
	{
		pIDs[count] = uID;
		pXs[count] = x;
		pYs[count] = y;
		pZs[count] = z;
		field_324[count] = a3;
		field_3EC[count] = a8;
		field_450[count++] = a7;
	}
}
