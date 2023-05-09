#pragma once

#include "LocationEnums.h"
#include "LocationInfo.h"
#include "LocationTime.h"

extern LEVEL_TYPE uCurrentlyLoadedLevelType;

bool GetAlertStatus();

LocationInfo &currentLocationInfo();
LocationTime &currentLocationTime();
