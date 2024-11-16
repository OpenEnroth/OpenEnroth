#pragma once

#include <array>

#include <glm/glm.hpp>

#include "Engine/Graphics/RenderEntities.h"

#include "Library/Geometry/Plane.h"

struct ODMFace;
struct BLVFace;

struct Camera3D {
    void ViewTransform(int x, int y, int z, int *transformed_x, int *transformed_y, int *transformed_z);
    void ViewTransform(RenderVertexSoft *a1a, unsigned int uNumVertices);

    bool ViewClip(int x, int y, int z, int *transformed_x, int *transformed_y,
                  int *transformed_z, bool dont_show = false);

    void Project(int x, int y, int z, int *screenspace_x, int *screenspace_y);
    void Project(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                 bool fit_into_viewport = false);

    bool CullFaceToCameraFrustum(RenderVertexSoft *pInVertices,
        unsigned int *pOutNumVertices,
        RenderVertexSoft *pVertices,
        signed int NumFrustumPlanes);

    void LightmapNeerClip(RenderVertexSoft *pInVertices,
                          int uNumInVertices,
                          RenderVertexSoft *pOutVertices,
                          unsigned int *pOutNumVertices);
    void LightmapFarClip(RenderVertexSoft *pInVertices,
                         int uNumInVertices,
                         RenderVertexSoft *pOutVertices,
                         unsigned int *pOutNumVertices);
    void LightmapProject(unsigned int uNumInVertices,
                 RenderVertexSoft *pOutVertices,
                 RenderVertexSoft *pInVertices,
                 signed int *pOutNumVertices);
    bool CullFaceToFrustum(RenderVertexSoft *a1,
                         unsigned int *pOutNumVertices,
                         RenderVertexSoft *pVertices, signed int uNumVertices);

    bool CullFaceToFrustum(RenderVertexSoft *inVerts,
        unsigned int *pOutNumVertices,
        RenderVertexSoft *pOutVertices, Planef *frustum, signed int uNumPlanes);

    bool ClipFaceToFrustum(RenderVertexSoft *pInVertices,
                           unsigned int *pOutNumVertices,
                           RenderVertexSoft *pVertices,
                           const Planef *CameraFrustrum);

    void BuildViewFrustum();
    void CreateViewMatrixAndProjectionScale();

    void debug_outline_sw(RenderVertexSoft *a2,
                          unsigned int uNumVertices, Color uDiffuse32,
                          float a5);

    void do_draw_debug_line_sw(RenderVertexSoft *pLineBegin,
                               Color sStartDiffuse32,
                               RenderVertexSoft *pLineEnd,
                               Color sEndDiffuse32,
                               unsigned int uOutNumVertices, float z_stuff);
    bool is_face_faced_to_cameraBLV(BLVFace *pFace);
    bool is_face_faced_to_cameraODM(ODMFace *pFace, RenderVertexSoft *a2);
    static void GetFacetOrientation(const Vec3f &normal, Vec3f *outU, Vec3f *outV);

    void CullByNearClip(RenderVertexSoft *pverts, unsigned int *unumverts);
    void CullByFarClip(RenderVertexSoft *pverts, unsigned int *unumverts);

    float GetMouseInfoDepth();

    glm::mat3x3 ViewMatrix = {};
    // using w comp of vec4 for dotdist
    std::array<glm::vec4, 6> FrustumPlanes = {{}};

    // field of view in vertical direction in degrees for GL
    float fov_y_deg = 0;
    // centre of the game viewport
    float screenCenterX = 0;
    float screenCenterY = 0;
    // these are the effective focal distances of the camera in screen space pixels.
    float ViewPlaneDistPixels = 0;

    // Camera field of view angles in degrees and radians
    int odm_fov_deg = 75;
    float odm_fov_rad = odm_fov_deg * M_PI / 180.0f;
    int blv_fov_deg = 60;
    float blv_fov_rad = blv_fov_deg * M_PI / 180.0f;

    // game viewport aspect ratio
    float aspect = 0;

    // camera cos + sin values in both forms to avoid repeated calculation
    void CalculateRotations(int cameraYaw, int cameraPitch);
    int _viewYaw = 0;
    int _viewPitch = 0; // TODO(captainurist): up is negative? wtf???
    float _yawRotationSine = 0;
    float _yawRotationCosine = 0;
    float _pitchRotationSine = 0;
    float _pitchRotationCosine = 0;

    glm::vec3 vCameraPos = {};


    float GetNearClip() const;
    float GetFarClip() const;
};

extern Camera3D *pCamera3D;
