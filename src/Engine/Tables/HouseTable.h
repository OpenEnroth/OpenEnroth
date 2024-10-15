#pragma once

#include "Utility/IndexedArray.h"

#include "Engine/Data/HouseData.h"

class Blob;

void initializeHouses(const Blob &houses);

// TODO(captainurist): move HOUSE_* to Engine/Data
extern IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;
