#pragma once

#include "Engine/MapEnums.h"

#include "LocationInfo.h"
#include "LocationTime.h"

// TODO(captainurist): move to Engine/ and drop the Location- prefix, should be MapSmth.

extern LevelType uCurrentlyLoadedLevelType;

bool GetAlertStatus();

LocationInfo &currentLocationInfo();
LocationTime &currentLocationTime();
