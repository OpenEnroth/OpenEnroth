#pragma once

#include "Utility/IndexedArray.h"

#include "Engine/Data/HouseData.h"

class Blob;

void initializeHouses(std::string_view houses);

extern IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;
