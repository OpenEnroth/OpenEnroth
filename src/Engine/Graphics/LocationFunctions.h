#pragma once

#include "LocationEnums.h"
#include "LocationInfo.h"
#include "LocationTime.h"

extern LevelType uCurrentlyLoadedLevelType;

bool GetAlertStatus();

LocationInfo &currentLocationInfo();
LocationTime &currentLocationTime();
