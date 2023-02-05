#pragma once

// TODO(pskelton): rename - lighting functions

#include <vector>

#include "Engine/Graphics/IRender.h"

struct LightsStack_StationaryLight_;
struct LightsStack_MobileLight_;

#define LIGHTMAP_FLAGS_USE_SPECULAR 0x01

extern LightsStack_StationaryLight_ *pStationaryLightsStack;
// extern StationaryLight pStationaryLights[400];
// extern int uNumStationaryLightsApplied;
extern LightsStack_MobileLight_ *pMobileLightsStack;
// extern MobileLight pMobileLights[400];
// extern int uNumMobileLightsApplied;

void DrawLightsDebugOutlines(char bit_one_for_list1__bit_two_for_list2);
