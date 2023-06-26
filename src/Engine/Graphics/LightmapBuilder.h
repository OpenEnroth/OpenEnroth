#pragma once

// TODO(pskelton): rename - lighting functions

struct LightsStack_StationaryLight_;
struct LightsStack_MobileLight_;
struct RenderBillboard;

#define LIGHTMAP_FLAGS_USE_SPECULAR 0x01

extern LightsStack_StationaryLight_ *pStationaryLightsStack;
extern LightsStack_MobileLight_ *pMobileLightsStack;

void DrawLightsDebugOutlines(char bit_one_for_list1__bit_two_for_list2);
int _43F55F_get_billboard_light_level(const RenderBillboard *a1, int uBaseLightLevel);
int GetLightLevelAtPoint(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z);
