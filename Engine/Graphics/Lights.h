#pragma once

#include "Engine/IocContainer.h"
#include "Engine/VectorTypes.h"

using EngineIoc = Engine_::IocContainer;

/*  257 */
#pragma pack(push, 1)
struct StationaryLight {
    Vec3_short_ vPosition {};
    int16_t uRadius = 0;
    uint8_t uLightColorR = 0;
    uint8_t uLightColorG = 0;
    uint8_t uLightColorB = 0;
    char uLightType = 0;
    int16_t uSectorID = 0;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MobileLight {
    Vec3_short_ vPosition {};
    int16_t uRadius = 0;
    uint8_t uLightColorR = 0;
    uint8_t uLightColorG = 0;
    uint8_t uLightColorB = 0;
    char uLightType = 0;
    int16_t field_C = 0;
    int16_t uSectorID = 0;
    int16_t field_10 = 0;
};
#pragma pack(pop)

/*
#pragma pack(push, 1)
struct LightStack<T>
{
  int field_0;
  char T[400];
  unsigned int uNumLightsActive;
};
#pragma pack(pop)*/

/*  260 */
#pragma pack(push, 1)
struct LightsStack_StationaryLight_ {
    //----- (004AD385) --------------------------------------------------------
    LightsStack_StationaryLight_() {
        this->log = EngineIoc::ResolveLogger();
        this->uNumLightsActive = 0;
    }

    //----- (004AD395) --------------------------------------------------------
    virtual ~LightsStack_StationaryLight_() { uNumLightsActive = 0; }

    //----- (004AD39D) --------------------------------------------------------
    inline unsigned int GetNumLights() { return uNumLightsActive; }

    //----- (004AD3C8) --------------------------------------------------------
    bool AddLight(int16_t x, int16_t y, int16_t z, int16_t a5, unsigned char r,
                  unsigned char g, unsigned char b, char uLightType);

    // void ( ***vdestructor_ptr)(LightsStack_StationaryLight_ *, bool);
    StationaryLight pLights[400];
    unsigned int uNumLightsActive;

    Log *log;
};
#pragma pack(pop)

/*  261 */
#pragma pack(push, 1)
struct LightsStack_MobileLight_ {
    //----- (00467D45) --------------------------------------------------------
    inline LightsStack_MobileLight_() {
        this->log = EngineIoc::ResolveLogger();
        this->uNumLightsActive = 0;
    }
    //----- (00467D55) --------------------------------------------------------
    virtual ~LightsStack_MobileLight_() { this->uNumLightsActive = 0; }

    bool AddLight(int16_t x, int16_t y, int16_t z, int16_t uSectorID,
                  int uRadius, uint8_t r, uint8_t g,
                  uint8_t b, char a10);

    // void ( ***vdestructor_ptr)(LightsStack_MobileLight_ *, bool);
    MobileLight pLights[400];
    unsigned int uNumLightsActive;
    Log *log;
};
#pragma pack(pop)
