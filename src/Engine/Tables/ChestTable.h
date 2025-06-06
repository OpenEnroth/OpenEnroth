#pragma once

#include <array>

#include "Engine/Data/ChestData.h"

extern std::array<ChestData, 8> chestTable;

void initializeChests();
