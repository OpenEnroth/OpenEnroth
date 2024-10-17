#pragma once

#include "Utility/IndexedArray.h"

#include "Engine/Data/HouseData.h"

class Blob;

void initializeHouses(const Blob &houses);

extern IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;
