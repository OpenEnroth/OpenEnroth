#pragma once

#include <string_view>

#include "Utility/IndexedArray.h"

#include "Engine/Data/HouseData.h"

void initializeHouses(std::string_view houses);

extern IndexedArray<HouseData, HOUSE_FIRST, HOUSE_LAST> houseTable;
