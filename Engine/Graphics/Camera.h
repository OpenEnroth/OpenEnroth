#pragma once
#include "Engine/OurMath.h"
#include "Engine/VectorTypes.h"

#include "Engine/Graphics/IRender.h"

#include <glm.hpp>

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

#define BLV_RENDER_DRAW_SW_OUTLINES (1 << 0)       // 1
#define BLV_RENDER_DRAW_D3D_OUTLINES (1 << 1)      // 2
#define ODM_RENDER_DRAW_D3D_OUTLINES (1 << 2)      // 4
#define ODM_RENDER_DRAW_TERRAIN_OUTLINES (1 << 3)  // 8

/*  123 */
#pragma pack(push, 1)
struct Camera3D {
    Camera3D();

    //----- (004363C6) --------------------------------------------------------
    virtual ~Camera3D() {}

    void ViewTransform(int x, int y, int z, int *transformed_x, int *transformed_y, int *transformed_z);
    void ViewTransform(struct RenderVertexSoft *a1a, unsigned int uNumVertices);

    bool ViewClip(int x, int y, int z, int *transformed_x, int *transformed_y,
                  int *transformed_z, bool dont_show = false);

    void Project(int x, int y, int z, int *screenspace_x, int *screenspace_y);
    void Project(struct RenderVertexSoft *pVertices, unsigned int uNumVertices,
                 bool fit_into_viewport = false);

    float GetPolygonMaxZ(struct RenderVertexSoft *pVertex,
                          unsigned int uStripType);
    float GetPolygonMinZ(struct RenderVertexSoft *pVertices,
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
    bool CullFaceToFrustum(struct RenderVertexSoft *a1,
                         unsigned int *pOutNumVertices,
                         struct RenderVertexSoft *pVertices, signed int uNumVertices,
                         char a6, int _unused);
    char CullVertsToPlane(struct stru154 *thisa, struct RenderVertexSoft *a2,
                 unsigned int *pOutNumVertices);
    void BuildViewFrustum();
    void CreateViewMatrixAndProjectionScale();

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
    bool is_face_faced_to_camera(struct BLVFace *pFace, struct RenderVertexSoft *a2);
    bool is_face_faced_to_cameraODM(struct ODMFace* pFace, struct RenderVertexSoft* a2);
    bool GetFacetOrientation(char polyType, struct Vec3_float_ *a2,
                             struct Vec3_float_ *a3, struct Vec3_float_ *a4);
    bool IsCulled(struct BLVFace *pFace);
    void ViewTransfrom_OffsetUV(struct RenderVertexSoft *pVertices,
                                unsigned int uNumVertices,
                                struct RenderVertexSoft *pOutVertices,
                                struct LightsData *a5);

    void CullByNearClip(struct RenderVertexSoft* pverts, uint* unumverts);
    void CullByFarClip(struct RenderVertexSoft* pverts, uint* unumverts);

    float GetPickDepth();

    void DebugDrawPortal(struct BLVFace *pFace);

    glm::mat3x3 ViewMatrix;
    // using w comp of vec4 for dotdist
    glm::vec4 FrustumPlanes[6] {};

    float fov = 0;
    float inv_fov = 0;
    float screenCenterX = 0;
    float screenCenterY = 0;
    float ViewPlaneDist_X = 0;
    float ViewPlaneDist_Y = 0;  // doesnt quite make sense

    int odm_fov_deg = 75;
    float odm_fov_rad = (float)odm_fov_deg * pi / 180.0;
    int blv_fov_deg = 60;
    float blv_fov_rad = (float)blv_fov_deg * pi / 180.0;
    
    void CalculateRotations(int camera_rot_y, int camera_rot_z);
    int sRotationZ = 0;
    int sRotationY = 0;
    float fRotationZSine = 0;
    float fRotationZCosine = 0;
    float fRotationYSine = 0;
    float fRotationYCosine = 0;
    int int_sine_Z = 0;
    int int_cosine_Z = 0;
    int int_sine_y = 0;
    int int_cosine_y = 0;

    glm::vec3 vCameraPos {};

    int debug_flags = 0;

    float GetNearClip() const;
    float GetFarClip() const;
};
#pragma pack(pop)

extern Camera3D *pCamera3D;
