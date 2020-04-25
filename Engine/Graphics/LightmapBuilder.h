#pragma once

#include <vector>

#include "Engine/Graphics/IRender.h"

struct LightsStack_StationaryLight_;
struct LightsStack_MobileLight_;

/*  115 */
#pragma pack(push, 1)
struct Lightmap {  // карта света, текстура для наложения теней
    Lightmap();
    virtual ~Lightmap() {}

    // void ( ***vdestructor_ptr)(Lightmap *, bool);
    signed int NumVertices;
    RenderVertexSoft pVertices[64];
    __int16 position_x;  //позиция источника света
    __int16 position_y;
    __int16 position_z;
    __int16 field_C0E;  //нигде не используется
    unsigned int uColorMask;
    float fBrightness;  //яркость
    int field_C18;      //нигде не используется
};
#pragma pack(pop)

#define LIGHTMAP_FLAGS_USE_SPECULAR 0x01

/*  114 */
#pragma pack(push, 1)
class LightmapBuilder {
 public:
    LightmapBuilder();
    virtual ~LightmapBuilder()  //----- (0045BBAA)
    {}

    void Draw_183808_Lightmaps();  //используется для мобильного света
    // bool DrawLightmap(Lightmap *a1, Vec3_float_ *pColorMult, float z_bias);
    bool DoDraw_183808_Lightmaps(float a2);  //используется для мобильного света
    void DrawLightmaps(int indices);
    void DrawLightmapsType(int type);
    void DrawDebugOutlines(char bit_one_for_list1__bit_two_for_list2);
    double _45D643_sw(struct Edge *a1, float a2);
    // int _45D426_sw(struct Span *a1, struct Edge **a2, unsigned int a3, struct
    // Edge *a4, int a5); bool _45D3C7_sw(struct Polygon *a1);
    bool StackLight_TerrainFace(struct StationaryLight *pLight,
                                struct Vec3_float_ *pNormal, float *a3,
                                struct RenderVertexSoft *a1,
                                unsigned int uStripType, int X,
                                unsigned int *pSlot);
    bool StackLights_TerrainFace(struct Vec3_float_ *pNormal, float *a3,
                                 struct RenderVertexSoft *a1,
                                 unsigned int uStripType, bool bLightBackfaces);
    bool ApplyLight_ODM(struct StationaryLight *pLight, struct ODMFace *pFace,
                        unsigned int *pSlot, bool bLightBackfaces);
    bool ApplyLights_OutdoorFace(struct ODMFace *pFace);
    double _45CC0C_light(struct Vec3_float_ a1, float a2, float a3,
                         struct Vec3_float_ *pNormal, float a5, int uLightType);
    int *_45CBD4(struct RenderVertexSoft *a2, int a3, int *a4, int *a5);
    int _45CB89(struct RenderVertexSoft *a1, int a2);
    int *_45CA88(struct LightsData *a2, struct RenderVertexSoft *a3, int a4,
                struct Vec3_float_ *pNormal);
    bool ApplyLight_BLV(struct StationaryLight *pLight, struct BLVFace *a2,
                        unsigned int *pSlot, bool bLightBackfaces, char *a5);
    bool ApplyLights_IndoorFace(unsigned int uFaceID);
    int _45C6D6(int a2, struct RenderVertexSoft *a3, Lightmap *pLightmap);
    int _45C4B9(int a2, struct RenderVertexSoft *a3, Lightmap *pLightmap);
    bool _45BE86_build_light_polygon(Vec3_int_ *pos, float radius,
                                     unsigned int uColorMask, float dot_dist,
                                     int uLightType, struct stru314 *a7,
                                     unsigned int uNumVertices,
                                     RenderVertexSoft *a9, char uClipFlag);
    bool ApplyLights(struct LightsData *a2, struct stru154 *a3,
                     unsigned int uNumVertices, struct RenderVertexSoft *a5,
                     struct IndoorCameraD3D_Vec4 *, char uClipFlag);

    // void ( ***vdestructor_ptr)(LightmapBuilder *, bool);
    // std::vector<Lightmap> std__vector_000004;
    // std::vector<Lightmap> std__vector_183808;
    Lightmap StationaryLights[512];      // std__vector_000004
    unsigned int StationaryLightsCount;  // std__vector_000004_size
    Lightmap MobileLights[768];          // std__vector_183808
    unsigned int MobileLightsCount;      // std__vector_183808_size
    float light_length_x;
    float light_length_y;
    float light_length_z;
    float light_length_x2;
    float light_length_y2;
    float light_length_z2;
    float tex_light_radius;  // flt_3C8C24;
    float flt_3C8C28;
    float flt_3C8C2C_lightmaps_brightness;
    float light_radius;
    RenderVertexSoft field_3C8C34[256];
    int uFlags;  // LIGHTMAP_FLAGS_*

    Log *log = nullptr;
};
#pragma pack(pop)

extern LightsStack_StationaryLight_ *pStationaryLightsStack;
// extern StationaryLight pStationaryLights[400];
// extern int uNumStationaryLightsApplied;
extern LightsStack_MobileLight_ *pMobileLightsStack;
// extern MobileLight pMobileLights[400];
// extern int uNumMobileLightsApplied;
