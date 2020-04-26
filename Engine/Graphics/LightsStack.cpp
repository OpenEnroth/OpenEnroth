#include "Engine/Log.h"

#include "Engine/Graphics/Lights.h"

//----- (00467D88) --------------------------------------------------------
bool LightsStack_MobileLight_::AddLight(int16_t x, int16_t y, int16_t z,
                                        int16_t uSectorID, int uRadius,
                                        uint8_t r, uint8_t g,
                                        uint8_t b, char uLightType) {
    if (uNumLightsActive >= 400) {
        log->Warning("Too many mobile lights!");
        return false;
    }

    pLights[uNumLightsActive].vPosition.x = x;
    pLights[uNumLightsActive].vPosition.y = y;
    pLights[uNumLightsActive].vPosition.z = z;
    pLights[uNumLightsActive].uRadius = uRadius;
    pLights[uNumLightsActive].field_C = (((uRadius < 0) - 1) & 0x3E) - 31;
    pLights[uNumLightsActive].uSectorID = uSectorID;
    pLights[uNumLightsActive].field_10 = uRadius * uRadius >> 5;
    pLights[uNumLightsActive].uLightColorR = r;
    pLights[uNumLightsActive].uLightColorG = g;
    pLights[uNumLightsActive].uLightColorB = b;
    pLights[uNumLightsActive++].uLightType = uLightType;

    return true;
}

bool LightsStack_StationaryLight_::AddLight(int16_t x, int16_t y, int16_t z,
                                            int16_t a5, unsigned char r,
                                            unsigned char g, unsigned char b,
                                            char uLightType) {
    if (uNumLightsActive >= 400) {
        log->Warning("Too many stationary lights!");
        return false;
    }

    StationaryLight* pLight = &pLights[uNumLightsActive++];
    pLight->vPosition.x = x;
    pLight->vPosition.y = y;
    pLight->vPosition.z = z;
    pLight->uRadius = a5;
    pLight->uLightColorR = r;
    pLight->uLightColorG = g;
    pLight->uLightColorB = b;
    pLight->uLightType = uLightType;
    return true;
}
