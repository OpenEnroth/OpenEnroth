#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Utility/IndexedArray.h"

void TrainingDialog(const char *s);

// TODO(Nik-RE-dev): contain in cpp file
extern IndexedArray<int, HOUSE_TRAINING_HALL_EMERALD_ISLE, HOUSE_TRAINING_HALL_STONE_CITY> trainingHallMaxLevels;
extern std::vector<int> charactersTrainedLevels;
