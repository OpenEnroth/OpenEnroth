#pragma once

#include "Engine/MapEnums.h"
#include "Engine/LocationInfo.h"

// TODO(captainurist): move to Engine/ and drop the Location- prefix, should be MapSmth.

extern LevelType uCurrentlyLoadedLevelType;

bool GetAlertStatus();

LocationInfo &currentLocationInfo();
