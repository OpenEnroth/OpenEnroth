#pragma once
#include "Engine/OurMath.h"
#include "Engine/VectorTypes.h"

#include "Engine/Graphics/IRender.h"

/*  124 */
#pragma pack(push, 1)
struct IndoorCameraD3D_Vec3 {
    //----- (004C0376) --------------------------------------------------------
    inline IndoorCameraD3D_Vec3() {}
    //----- (004C037F) --------------------------------------------------------
    virtual ~IndoorCameraD3D_Vec3() {}
    //----- (004C039C) --------------------------------------------------------
    // void ~IndoorCameraD3D_Vec3() {}

    // void ( ***vdestructor_ptr)(IndoorCameraD3D_Vec3 *, bool);

    union {
        struct {
            float x;
            float y;
            float z;
        };
        float v[3] {};
    };
};
#pragma pack(pop)

/*  125 */
#pragma pack(push, 1)
struct IndoorCameraD3D_Vec4 : public IndoorCameraD3D_Vec3 {
    //----- (00498038) --------------------------------------------------------
    inline IndoorCameraD3D_Vec4() : IndoorCameraD3D_Vec3() {}

    //----- (00498069) --------------------------------------------------------
    virtual ~IndoorCameraD3D_Vec4() {}

    float dot = 0;
    int _wtf = 0;  // sizeof vec4 is 18 and first member is vdtor, but vdtor is
               // already included in vec3 so very weird
};
#pragma pack(pop)

/*  199 */
#pragma pack(push, 1)
struct IndoorCameraD3D_stru3 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    float flt_30;
    int field_34;
};
#pragma pack(pop)

/*  197 */
#pragma pack(push, 1)
struct IndoorCameraD3D_stru1 {
    //----- (004363A2) --------------------------------------------------------
    IndoorCameraD3D_stru1() { this->flt_2C = 0.0; }

    int field_0 = 0;
    int field_4 = 0;
    int field_8 = 0;
    int field_C = 0;
    int field_10 = 0;
    int field_14 = 0;
    int field_18 = 0;
    int field_1C = 0;
    int field_20 = 0;
    int field_24 = 0;
    int field_28 = 0;
    float flt_2C = 0;
};
#pragma pack(pop)

/*  198 */
#pragma pack(push, 1)
struct IndoorCameraD3D_stru2 {
    unsigned int mm7__vector_000004_size = 0;
    IndoorCameraD3D_stru1 mm7__vector_000004[64] {};
    int field_C04 = 0;
    int field_C08 = 0;
    int field_C0C = 0;
};
#pragma pack(pop)

#define BLV_RENDER_DRAW_SW_OUTLINES (1 << 0)       // 1
#define BLV_RENDER_DRAW_D3D_OUTLINES (1 << 1)      // 2
#define ODM_RENDER_DRAW_D3D_OUTLINES (1 << 2)      // 4
#define ODM_RENDER_DRAW_TERRAIN_OUTLINES (1 << 3)  // 8

/*  123 */
#pragma pack(push, 1)
struct IndoorCameraD3D {
    IndoorCameraD3D();

    //----- (004363C6) --------------------------------------------------------
    virtual ~IndoorCameraD3D() {}

    void ViewTransform(int x, int y, int z, int *transformed_x,
                       int *transformed_y, int *transformed_z);
    void ViewTransform(struct RenderVertexSoft *a1a, unsigned int uNumVertices);

    bool ViewClip(int x, int y, int z, int *transformed_x, int *transformed_y,
                  int *transformed_z, bool dont_show = false);

    void Project(int x, int y, int z, int *screenspace_x, int *screenspace_y);
    void Project(struct RenderVertexSoft *pVertices, unsigned int uNumVertices,
                 bool fit_into_viewport = false);

    double GetPolygonMaxZ(struct RenderVertexSoft *pVertex,
                          unsigned int uStripType);
    double GetPolygonMinZ(struct RenderVertexSoft *pVertices,
                          unsigned int uStripType);

    void LightmapNeerClip(struct RenderVertexSoft *pInVertices,
                          int uNumInVertices,
                          struct RenderVertexSoft *pOutVertices,
                          unsigned int *pOutNumVertices);
    void LightmapFarClip(struct RenderVertexSoft *pInVertices,
                         int uNumInVertices,
                         struct RenderVertexSoft *pOutVertices,
                         unsigned int *pOutNumVertices);
    void LightmapProject(unsigned int uNumInVertices,
                 struct RenderVertexSoft *pOutVertices,
                 struct RenderVertexSoft *pInVertices,
                 signed int *pOutNumVertices);
    bool _4371C3(struct RenderVertexSoft *pVertices,
                 unsigned int *pOutNumVertices, int _unused);
    bool CullFaceToFrustum(struct RenderVertexSoft *a1,
                         unsigned int *pOutNumVertices,
                         struct RenderVertexSoft *pVertices,
                         IndoorCameraD3D_Vec4 *a4, signed int uNumVertices,
                         char a6, int _unused);
    char _437376(struct stru154 *thisa, struct RenderVertexSoft *a2,
                 unsigned int *pOutNumVertices);
    void BuildViewFrustum();
    void BuildFrustumPlane(IndoorCameraD3D_Vec3 *a1, IndoorCameraD3D_Vec4 *a2);
    void Vec3Transform(const IndoorCameraD3D_Vec3 *pVector,
                       IndoorCameraD3D_Vec3 *pOut);
    void CreateWorldMatrixAndSomeStuff();
    void MatrixMultiply(struct Matrix3x3_float_ *a1,
                        struct Matrix3x3_float_ *a2,
                        struct Matrix3x3_float_ *a3);
    void PrepareAndDrawDebugOutline(struct BLVFace *pFace,
                                    unsigned int uDiffuse);
    void debug_outline_sw(struct RenderVertexSoft *a2,
                          unsigned int uNumVertices, unsigned int uDiffuse,
                          float a5);
    void debug_outline_d3d(const struct RenderVertexD3D3 *pLineVertices,
                           unsigned int uNumLines, int uDiffuse, float z_stuff);
    void do_draw_debug_line_sw(struct RenderVertexSoft *pLineBegin,
                               signed int sStartDiffuse,
                               struct RenderVertexSoft *pLineEnd,
                               signed int sEndDiffuse,
                               unsigned int uOutNumVertices, float z_stuff);
    bool is_face_faced_to_camera(struct BLVFace *pFace,
                                 struct RenderVertexSoft *a2);
    bool GetFacetOrientation(char polyType, struct Vec3_float_ *a2,
                             struct Vec3_float_ *a3, struct Vec3_float_ *a4);
    bool IsCulled(struct BLVFace *pFace);
    void ViewTransfrom_OffsetUV(struct RenderVertexSoft *pVertices,
                                unsigned int uNumVertices,
                                struct RenderVertexSoft *pOutVertices,
                                struct LightsData *a5);

    float GetPickDepth();

    void DebugDrawPortal(struct BLVFace *pFace);

    // void ( ***vdestructor_ptr)(IndoorCameraD3D *, bool);
    IndoorCameraD3D_Vec3 field_4[3] {};  // matrix for camera rotation transform
    // IndoorCameraD3D_Vec3 field_14;
    // IndoorCameraD3D_Vec3 field_24;
    IndoorCameraD3D_Vec4 FrustumPlanes[6] {};
    float fov = 0;
    float screenCenterX = 0;
    float screenCenterY = 0;
    float fov_x = 0;
    float fov_y = 0;
    float inv_fov = 0;
    // float _unused_blv_party_x;
    // float _unused_blv_party_y;
    // float _unused_blv_party_z;
    char field_E8[32] {};
    float field_108 = 0;
    // float _unused_blv_party_x_2;
    // float _unused_blv_party_y_2;
    // float _unused_blv_party_z_2;
    char field_118[32] {};
    float field_138 = 0;
    char field_13C[44] {};
    float field_168 = 0;
    char field_16C[44] {};
    float field_198 = 0;
    char field_19C[44] {};
    float field_1C8 = 0;
    char field_1CC[44] {};
    float field_1F8 = 0;
    char field_1FC[44] {};
    float field_228 = 0;
    char field_22C[44] {};
    float field_258 = 0;
    char field_25C[44] {};
    float field_288 = 0;
    char field_28C[44] {};
    float field_2B8 = 0;
    float field_2BC = 0;
    float field_2C0 = 0;
    float field_2C4 = 0;
    char field_2C8[32] {};
    float field_2E8 = 0;
    float field_2EC = 0;
    float field_2F0 = 0;
    float field_2F4 = 0;
    char field_2F8[32] {};
    float field_318 = 0;
    float field_31C = 0;
    float field_320 = 0;
    float field_324 = 0;
    char field_328[32] {};
    float field_348 = 0;
    float field_34C = 0;
    float field_350 = 0;
    float field_354 = 0;
    char field_358[32] {};
    float field_378 = 0;
    IndoorCameraD3D_stru3 list_0037C[16384] {};
    unsigned int list_0037C_size = 0;
    IndoorCameraD3D_stru2 list_E0380[256] {};
    int list_E0380_size = 0;

    void CalculateRotations(int camera_rot_x, int camera_rot_z);
    int sRotationZ = 0;        // z rotation   // moved  from 157 struct IndoorCamera::18
    int sRotationX = 0;          // moved  from 157 struct IndoorCamera::14
    float fRotationZSine = 0;  // z rotation  // moved  from 157 struct IndoorCamera::2C
    float fRotationZCosine = 0;  // z rotatio  // moved  from 157 struct IndoorCamera::30
    float fRotationXSine = 0;    // moved  from 157 struct IndoorCamera::34
    float fRotationXCosine = 0;  // moved  from 157 struct IndoorCamera::38
    Vec3<int> vPartyPos {};     // moved  from 157 struct IndoorCamera::00
                             // merged from 162 struct BLVRenderParams::08
    int debug_flags = 0;         // moved  from 157 struct IndoorCamera::4C
                             // merged from 162 struct BLVRenderParams::04
    int int_sine_Z = 0;          // moved  from 157 struct ODMRenderParams::1C
                             // merged from 162 struct BLVRenderParams::24
    int int_cosine_Z = 0;        // moved  from 157 struct ODMRenderParams::20
                             // merged from 162 struct BLVRenderParams::20
    int int_sine_x = 0;          // moved  from 157 struct ODMRenderParams::24
                             // merged from 162 struct BLVRenderParams::2C
    int int_cosine_x = 0;        // moved  from 157 struct ODMRenderParams::28
                             // merged from 162 struct BLVRenderParams::28

    float GetNearClip() const;
    float GetFarClip() const;
};
#pragma pack(pop)

extern IndoorCameraD3D *pIndoorCameraD3D;
