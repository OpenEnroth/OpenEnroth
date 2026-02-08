#include "LightmapBuilder.h"

// TODO(pskelton): rename - lighting functions

#include "Engine/Engine.h"

#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/ClippingFunctions.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"

#include "Library/Color/ColorTable.h"

LightsStack_StationaryLight_ *pStationaryLightsStack = nullptr;
LightsStack_MobileLight_ *pMobileLightsStack = nullptr;


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
    //    for (unsigned i = 0; i < this->MobileLightsCount; ++i)
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
Color GetActorTintColor(int max_dimm, int min_dimm, float distance, int bNoLight, const RenderBillboard *pBillboard) {
    int dimminglevel = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        return Color(8 * (31 - max_dimm), 8 * (31 - max_dimm), 8 * (31 - max_dimm));

    if (pParty->armageddon_timer) return colorTable.Red;

    bool isNight = pWeather->bNight;
    if (engine->IsUnderwater())
        isNight = false;

    if (isNight) {
        dimminglevel = 216;
        if (pBillboard) dimminglevel = 8 * _43F55F_get_billboard_light_level(pBillboard, dimminglevel >> 3);
        dimminglevel = std::clamp(dimminglevel, 0, 216);
        return Color(255 - dimminglevel, 255 - dimminglevel, 255 - dimminglevel);
    }

    // daytime
    if (fabsf(distance) < 1.0e-6f) return colorTable.White;

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
        return Color(255 - dimminglevel, 255 - dimminglevel, 255 - dimminglevel);
    } else {
        Color sea = colorTable.Topaz;
        float col = (255 - dimminglevel) * 0.0039215689f;
        int red = static_cast<int>(floorf(col * sea.r + 0.5f));
        int grn = static_cast<int>(floorf(col * sea.g + 0.5f));
        int blue = static_cast<int>(floorf(col * sea.b + 0.5f));
        return Color(red, grn, blue);
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
    for (unsigned i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        MobileLight *p = &pMobileLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = std::abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = std::abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = std::abs(p->vPosition.z - z);
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
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        BLVSector *pSector = &pIndoor->sectors[uSectorID];

        for (uint16_t lightId : pSector->lightIds) {
            BLVLight *this_light = &pIndoor->lights[lightId];
            light_radius = this_light->uRadius;

            if (~this_light->uAtributes & 8) {
                distX = std::abs(this_light->vPosition.x - x);
                if (distX <= light_radius) {
                    distY = std::abs(this_light->vPosition.y - y);
                    if (distY <= light_radius) {
                        distZ = std::abs(this_light->vPosition.z - z);
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
    for (unsigned i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        StationaryLight *p = &pStationaryLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = std::abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = std::abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = std::abs(p->vPosition.z - z);
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
int _43F55F_get_billboard_light_level(const RenderBillboard *a1, int uBaseLightLevel) {
    int v3 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        v3 = pIndoor->sectors[a1->uIndoorSectorID].minAmbientLightLevel;
    } else {
        if (uBaseLightLevel == -1) {
            v3 = a1->dimming_level;
        } else {
            v3 = uBaseLightLevel;
        }
    }

    return GetLightLevelAtPoint(v3, a1->uIndoorSectorID, a1->worldPos.x, a1->worldPos.y, a1->worldPos.z);
}
