#pragma once

#include "LocationEnums.h"
#include "LocationInfo.h"
#include "LocationTime.h"

enum class LevelType;
struct LocationInfo;
struct LocationTime;

extern LevelType uCurrentlyLoadedLevelType;

bool GetAlertStatus();

LocationInfo &currentLocationInfo();
LocationTime &currentLocationTime();
