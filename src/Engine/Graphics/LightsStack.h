#pragma once

#include <array>

#include "Engine/EngineIocContainer.h"

#include "Utility/Geometry/Vec.h"

struct StationaryLight {
    Vec3f vPosition {};
    int16_t uRadius = 0;
    uint8_t uLightColorR = 0;
    uint8_t uLightColorG = 0;
    uint8_t uLightColorB = 0;
    char uLightType = 0;
    int16_t uSectorID = 0;
};

struct MobileLight {
    Vec3f vPosition {};
    int16_t uRadius = 0;
    uint8_t uLightColorR = 0;
    uint8_t uLightColorG = 0;
    uint8_t uLightColorB = 0;
    char uLightType = 0;
    int16_t field_C = 0;
    int16_t uSectorID = 0;
    int16_t field_10 = 0;
};

/*
struct LightStack<T>
{
  int field_0;
  char T[400];
  unsigned int uNumLightsActive;
};
*/

struct LightsStack_StationaryLight_ {
    //----- (004AD385) --------------------------------------------------------
    LightsStack_StationaryLight_() {
        this->log = EngineIocContainer::ResolveLogger();
        this->uNumLightsActive = 0;
    }

    //----- (004AD395) --------------------------------------------------------
    virtual ~LightsStack_StationaryLight_() { uNumLightsActive = 0; }

    //----- (004AD39D) --------------------------------------------------------
    inline unsigned int GetNumLights() { return uNumLightsActive; }

    //----- (004AD3C8) --------------------------------------------------------
    bool AddLight(const Vec3f &pos, int16_t a5, unsigned char r,
                  unsigned char g, unsigned char b, char uLightType);

    std::array<StationaryLight, 400> pLights;
    unsigned int uNumLightsActive;

    Logger *log;
};

struct LightsStack_MobileLight_ {
    //----- (00467D45) --------------------------------------------------------
    inline LightsStack_MobileLight_() {
        this->log = EngineIocContainer::ResolveLogger();
        this->uNumLightsActive = 0;
    }
    //----- (00467D55) --------------------------------------------------------
    virtual ~LightsStack_MobileLight_() { this->uNumLightsActive = 0; }

    bool AddLight(const Vec3f &pos, int16_t uSectorID,
                  int uRadius, uint8_t r, uint8_t g,
                  uint8_t b, char uLightType);

    std::array<MobileLight, 400> pLights;
    unsigned int uNumLightsActive;
    Logger *log;
};
