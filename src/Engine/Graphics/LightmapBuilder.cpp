#include "LightmapBuilder.h"

// TODO(pskelton): rename - lighting functions

#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/stru314.h"

#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ClippingFunctions.h"
#include "Engine/Graphics/Weather.h"

LightsStack_StationaryLight_ *pStationaryLightsStack = new LightsStack_StationaryLight_;
// StationaryLight pStationaryLights[400];
// int uNumStationaryLightsApplied;
LightsStack_MobileLight_ *pMobileLightsStack = new LightsStack_MobileLight_;
// MobileLight pMobileLights[400];
// int uNumMobileLightsApplied;


// TODO(pskelton): this needs reworking if we want lights to be outlined
//----- (0045D698) --------------------------------------------------------
void DrawLightsDebugOutlines(
    char bit_one_for_list1__bit_two_for_list2) {
    //if (bit_one_for_list1__bit_two_for_list2 & 1) {
    //    for (int i = 0; i < this->StationaryLightsCount; ++i)
    //        pCamera3D->debug_outline_sw(
    //            this->StationaryLights[i].pVertices,
    //            this->StationaryLights[i].NumVertices, colorTable.Red.C32(), 0.0f);
    //}
    //if (bit_one_for_list1__bit_two_for_list2 & 2) {
    //    for (uint i = 0; i < this->MobileLightsCount; ++i)
    //        pCamera3D->debug_outline_sw(
    //            this->MobileLights[i].pVertices,
    //            this->MobileLights[i].NumVertices, colorTable.Mahogany.C32(), 0.00019999999f);
    //}
}

/**
 *
 * @param max_dimm                      Maximum dimming level allowed (0-31). 31 * 8 ~ 255.
 * @param min_dimm                      Minimum dimming level allowed (0-31).
 * @param distance                      Depth distance to billboard.
 * @param bNoLight                      No light dimming.
 * @param pBillboard                    Pointer to the billboard to test against or Nullptr.
 *
 * @return                              Return 32bit colour ARGB.
 */
int GetActorTintColor(int max_dimm, int min_dimm, float distance, int bNoLight, RenderBillboard *pBillboard) {
    int dimminglevel = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        return 8 * (31 - max_dimm) | ((8 * (31 - max_dimm) | ((31 - max_dimm) << 11)) << 8);

    if (pParty->armageddon_timer) return 0xFF0000FF;

    bool isNight = pWeather->bNight;
    if (engine->IsUnderwater())
        isNight = false;

    if (isNight) {
        dimminglevel = 216;
        if (pBillboard) dimminglevel = 8 * _43F55F_get_billboard_light_level(pBillboard, dimminglevel >> 3);
        dimminglevel = std::clamp(dimminglevel, 0, 216);
        return (255 - dimminglevel) | ((255 - dimminglevel) << 16) | ((255 - dimminglevel) << 8);
    }

    // daytime
    if (fabsf(distance) < 1.0e-6f) return 0xFFF8F8F8;

    // dim in measured in 8-steps
    int rangewidth = 8 * (max_dimm - min_dimm);
    rangewidth = std::clamp(rangewidth, 0, 216);

    float fog_density_mult = 216.0f;
    if (bNoLight)
        fog_density_mult += distance / pODMRenderParams->shading_dist_shade * 32.0f;

    dimminglevel = static_cast<int>(rangewidth + floorf(pOutdoor->fFogDensity * fog_density_mult + 0.5f));

    if (pBillboard) dimminglevel = 8 * _43F55F_get_billboard_light_level(pBillboard, dimminglevel >> 3);
    dimminglevel = std::clamp(dimminglevel, rangewidth, 216);
    if (dimminglevel > 8 * pOutdoor->max_terrain_dimming_level)
        dimminglevel = 8 * pOutdoor->max_terrain_dimming_level;

    if (!engine->IsUnderwater()) {
        return (255 - dimminglevel) | ((255 - dimminglevel) << 16) | ((255 - dimminglevel) << 8);
    } else {
        // underwater
        float col = (255 - dimminglevel) * 0.0039215689f;
        int red = static_cast<int>(floorf(col * 16.0f + 0.5f));
        int grn = static_cast<int>(floorf(col * 194.0f + 0.5f));
        int blue = static_cast<int>(floorf(col * 153.0f + 0.5f));
        return red | (grn << 8) | (blue << 16);
    }
}

/**
 * @offset 0x0043F5C8.
 *
 * @param uBaseLightLevel               Starting dimming level at point (0-31).
 * @param uSectorID                     Sector ID if indoors or 0.
 * @param x, y, z                       Co-ords of point.
 *
 * @return                              Dimming level (0-31) with lights effect added.
 */
int GetLightLevelAtPoint(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z) {
    int lightlevel = uBaseLightLevel;
    float light_radius{};
    float distX{};
    float distY{};
    float distZ{};
    unsigned int approx_distance;

    // mobile lights
    for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        MobileLight *p = &pMobileLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = abs(p->vPosition.z - z);
                if (distZ <= light_radius) {
                    approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                    if (approx_distance < light_radius)
                        //* ORIGONAL */lightlevel += ((uint64_t)(30i64 *(signed int)(approx_distance << 16) / light_radius) >> 16) - 30;
                        lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                }
            }
        }
    }

    // sector lights
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        BLVSector *pSector = &pIndoor->pSectors[uSectorID];

        for (uint i = 0; i < pSector->uNumLights; ++i) {
            BLVLightMM7 *this_light = &pIndoor->pLights[pSector->pLights[i]];
            light_radius = this_light->uRadius;

            if (~this_light->uAtributes & 8) {
                distX = abs(this_light->vPosition.x - x);
                if (distX <= light_radius) {
                    distY = abs(this_light->vPosition.y - y);
                    if (distY <= light_radius) {
                        distZ = abs(this_light->vPosition.z - z);
                        if (distZ <= light_radius) {
                            approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                            if (approx_distance < light_radius)
                                lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                        }
                    }
                }
            }
        }
    }

    // stationary lights
    for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        StationaryLight *p = &pStationaryLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = abs(p->vPosition.z - z);
                if (distZ <= light_radius) {
                    approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                    if (approx_distance < light_radius)
                        lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                }
            }
        }
    }

    lightlevel = std::clamp(lightlevel, 0, 31);
    return lightlevel;
}

/**
 * @offset 0x0043F55F.
 *
 * @param *a1                           Pointer to billboard to check.
 * @param uBaseLightLevel               Base light dimming value (0-31).
 *
 * @return                              Billboard dimming value (0-31) with lights applied.
 */
int _43F55F_get_billboard_light_level(RenderBillboard *a1, int uBaseLightLevel) {
    int v3 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        v3 = pIndoor->pSectors[a1->uIndoorSectorID].uMinAmbientLightLevel;
    } else {
        if (uBaseLightLevel == -1) {
            v3 = a1->dimming_level;
        } else {
            v3 = uBaseLightLevel;
        }
    }

    return GetLightLevelAtPoint(v3, a1->uIndoorSectorID, a1->world_x, a1->world_y, a1->world_z);
}
