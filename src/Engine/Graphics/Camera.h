#pragma once

#include <cmath>
#include <array>
#include <memory>

#include <glm/glm.hpp>

#include "Engine/Graphics/RenderEntities.h"

#include "Library/Geometry/Plane.h"

struct BLVFace;

constexpr int FRUSTUM_PLANE_COUNT = 4;  // Left, right, top and bottom. Near and far are handled separately.

struct Camera3D {
    Vec3f ViewTransform(const Vec3f* pos) const;
    void ViewTransform(RenderVertexSoft *vertex, int uNumVertices) const;

    bool ViewClip(const Vec3f& pos, Vec3f* outPos) const;

    Vec2f Project(const Vec3f& pos) const;
    void Project(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                 bool fit_into_viewport = false);
    Vec2f FitToViewport(const Vec2f& projPos) const;

    /**
     * @param pInVertices               Face vertices in world space.
     * @param uNumVertices              Vertex count.
     * @return                          Whether the face is approximately inside the camera frustum. Each plane is
     *                                  tested independently, so a face outside a frustum corner can still pass.
     * @offset 0x437285
     */
    bool IsFaceInCameraFrustum(const RenderVertexSoft *pInVertices, int uNumVertices) const;

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
                               Color sEndDiffuse32, float z_stuff);
    bool is_face_faced_to_camera(BLVFace *pFace);

    void CullByNearClip(RenderVertexSoft *pverts, unsigned int *unumverts);
    void CullByFarClip(RenderVertexSoft *pverts, unsigned int *unumverts);

    float GetMouseInfoDepth();

    glm::mat3x3 ViewMatrix = {};
    // using w comp of vec4 for dotdist
    std::array<glm::vec4, FRUSTUM_PLANE_COUNT> FrustumPlanes = {{}};

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

extern std::unique_ptr<Camera3D> pCamera3D;
