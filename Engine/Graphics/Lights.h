#pragma once

#include "../VectorTypes.h"

/*  257 */
#pragma pack(push, 1)
struct StationaryLight {
    Vec3_short_ vPosition;
    __int16 uRadius;
    unsigned __int8 uLightColorR;
    unsigned __int8 uLightColorG;
    unsigned __int8 uLightColorB;
    char uLightType;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MobileLight {
    Vec3_short_ vPosition;
    __int16 uRadius;
    unsigned __int8 uLightColorR;
    unsigned __int8 uLightColorG;
    unsigned __int8 uLightColorB;
    char uLightType;
    __int16 field_C;
    __int16 uSectorID;
    __int16 field_10;
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
    LightsStack_StationaryLight_() { uNumLightsActive = 0; }

    //----- (004AD395) --------------------------------------------------------
    virtual ~LightsStack_StationaryLight_() { uNumLightsActive = 0; }

    //----- (004AD39D) --------------------------------------------------------
    inline unsigned int GetNumLights() { return uNumLightsActive; }

    //----- (004AD3C8) --------------------------------------------------------
    bool AddLight(__int16 x, __int16 y, __int16 z, __int16 a5, unsigned char r,
                  unsigned char g, unsigned char b, char uLightType);

    // void ( ***vdestructor_ptr)(LightsStack_StationaryLight_ *, bool);
    StationaryLight pLights[400];
    unsigned int uNumLightsActive;
};
#pragma pack(pop)

/*  261 */
#pragma pack(push, 1)
struct LightsStack_MobileLight_ {
    //----- (00467D45) --------------------------------------------------------
    inline LightsStack_MobileLight_() {
        this->uNumLightsActive = 0;
        // this->vdestructor_ptr = &pLightsStack_MobileLight__pvdtor;
    }
    //----- (00467D55) --------------------------------------------------------
    virtual ~LightsStack_MobileLight_() { this->uNumLightsActive = 0; }

    bool AddLight(__int16 x, __int16 y, __int16 z, __int16 uSectorID,
                  int uRadius, unsigned __int8 r, unsigned __int8 g,
                  unsigned __int8 b, char a10);

    // void ( ***vdestructor_ptr)(LightsStack_MobileLight_ *, bool);
    MobileLight pLights[400];
    unsigned int uNumLightsActive;
};
#pragma pack(pop)
