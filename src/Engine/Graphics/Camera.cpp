#include "Engine/Graphics/Camera.h"

#include <cmath>
#include <memory>

#include "Engine/Engine.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Renderer/Renderer.h"

#include "Engine/Graphics/ClippingFunctions.h"

std::unique_ptr<Camera3D> pCamera3D;

//----- (0043643E) --------------------------------------------------------
float Camera3D::GetMouseInfoDepth() {
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        return engine->config->gameplay.MouseInfoDepthOutdoor.value();
    else
        return engine->config->gameplay.MouseInfoDepthIndoor.value();
}

float Camera3D::GetNearClip() const {
    return engine->config->graphics.ClipNearDistance.value();
}

float Camera3D::GetFarClip() const {
    // The cave is one large area so we need to increase the clip distance so that the back of the cave is visible
    // TODO(pskelton): do we want to be overriding the config value here?
    if (engine->_currentLoadedMapId == MAP_WROMTHRAXS_CAVE)
        return 25000.0f;
    return engine->config->graphics.ClipFarDistance.value();
}

bool Camera3D::ViewClip(const Vec3f& pos, Vec3f* outPos) const {
    *outPos = ViewTransform(&pos);

    return outPos->x >= this->GetNearClip() && outPos->x <= this->GetFarClip();
}

//----- (00436523) --------------------------------------------------------
void Camera3D::ViewTransform(RenderVertexSoft *vertex, int uNumVertices) const {
    for (unsigned i = 0; i < uNumVertices; ++i) {
        RenderVertexSoft *a1 = &vertex[i];
        a1->vWorldViewPosition = ViewTransform(&a1->vWorldPosition);
    }
}

Vec3f Camera3D::ViewTransform(const Vec3f* pos) const {
    float vCamToVertexX = pos->x - pCamera3D->vCameraPos.x;
    float vCamToVertexY = pos->y - pCamera3D->vCameraPos.y;
    float vCamToVertexZ = pos->z - pCamera3D->vCameraPos.z;

    glm::vec3 camtovert(vCamToVertexX, vCamToVertexY, vCamToVertexZ);
    camtovert = camtovert * ViewMatrix;

    // TODO(pskelton): swap components to match expectation - eg x is depth make it z
    return Vec3f(camtovert.x, camtovert.y, camtovert.z);
}

//----- (00438258) --------------------------------------------------------
bool Camera3D::is_face_faced_to_camera(BLVFace *pFace) {
    return pFace->facePlane.dist +
        pCamera3D->vCameraPos.z * pFace->facePlane.normal.z +
        pCamera3D->vCameraPos.y * pFace->facePlane.normal.y +
        pCamera3D->vCameraPos.x * pFace->facePlane.normal.x >
        0.0f;
}

//----- (00437AB5) --------------------------------------------------------
void Camera3D::do_draw_debug_line_sw(RenderVertexSoft *pLineBegin,
                                            Color sStartDiffuse32,
                                            RenderVertexSoft *pLineEnd,
                                            Color sEndDiffuse32,
                                            float z_stuff) {
    RenderVertexSoft a1[2];

    a1[0].vWorldPosition = pLineBegin->vWorldPosition;
    a1[1].vWorldPosition = pLineEnd->vWorldPosition;
    if (IsFaceInCameraFrustum(a1, 2)) {
        ViewTransform(a1, 2);
        Project(a1, 2, 0);

        render->BeginLines2D();
        render->RasterLine2D(a1[0].vWorldViewProj.toInt(), a1[1].vWorldViewProj.toInt(),
                             sStartDiffuse32, sEndDiffuse32);
        render->EndLines2D();
    }
}

//----- (004379EE) --------------------------------------------------------
void Camera3D::debug_outline_sw(RenderVertexSoft *a2,
                                       unsigned int uNumVertices,
                                       Color uDiffuse32, float a5) {
    if (!uNumVertices) return;
    if ((signed int)(uNumVertices - 1) > 0) {
        for (unsigned i = 0; i < uNumVertices - 1; i++)
            do_draw_debug_line_sw(&a2[i], uDiffuse32, &a2[i + 1], uDiffuse32, a5);
    }
    do_draw_debug_line_sw(&a2[uNumVertices - 1], uDiffuse32, a2, uDiffuse32, a5);
}

//----- (004376E7) --------------------------------------------------------
void Camera3D::CreateViewMatrixAndProjectionScale() {
    // set up view transform matrix (NB orig game has y rotation direction reversed)
    float cos_y1 = _pitchRotationCosine, sin_y1 = _pitchRotationSine;
    float cos_z1 = _yawRotationCosine, sin_z1 = _yawRotationSine;

    // right handed co-ord system
    // world +ve x east, +ve y north, +ve z up
    // view +ve x in, +ve y left, +ve z up

    glm::mat3x3 newpitch(cos_y1, 0, -sin_y1, 0, 1, 0, sin_y1, 0, cos_y1);
    glm::mat3x3 newyaw(cos_z1, sin_z1, 0, -sin_z1, cos_z1, 0, 0, 0, 1);
    glm::mat3x3 newroll(1);

    ViewMatrix = newyaw * newpitch * newroll;

    // TODO(pskelton): fov calcs only need recalculating on level change or if we add config option
    // fov projection calcs
    float halfFovTan = std::tan(odm_fov_rad / 2.0);
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        halfFovTan = std::tan(blv_fov_rad / 2.0);

    ViewPlaneDistPixels = (double)pViewport.w * 0.5 / halfFovTan;

    // calculate vertical FOV in degrees for GL rendering
    fov_y_deg = (180.0 / M_PI) * 2.0 * std::atan((pViewport.h / 2.0) / pCamera3D->ViewPlaneDistPixels);

    screenCenterX = (double)pViewport.center().x;
    screenCenterY = (double)pViewport.center().y - pViewport.y;

    aspect = float(pViewport.w / float(pViewport.h));
}

//----- (004374E8) --------------------------------------------------------
void Camera3D::BuildViewFrustum() {
    float HalfAngleX = (M_PI / 2.0) - (odm_fov_rad / 2.0);
    float HalfAngleY = (M_PI / 2.0) - (std::atan((pViewport.h / 2.0) / pCamera3D->ViewPlaneDistPixels));

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        HalfAngleX = (M_PI / 2.0) - (blv_fov_rad / 2.0);
    }

    glm::vec3 PlaneVec(0);

    // rotate (1,0,0) around z
    PlaneVec.y = -std::sin(HalfAngleX);
    PlaneVec.x = std::cos(HalfAngleX);
    PlaneVec.z = 0.0;
    FrustumPlanes[0] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[0].w = glm::dot(glm::vec3(FrustumPlanes[0]), vCameraPos);

    PlaneVec.y = std::sin(HalfAngleX);
    FrustumPlanes[1] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[1].w = glm::dot(glm::vec3(FrustumPlanes[1]), vCameraPos);

    // rotate (1,0,0) around y
    PlaneVec.z = -std::sin(HalfAngleY);
    PlaneVec.y = 0.0;
    PlaneVec.x = std::cos(HalfAngleY);
    FrustumPlanes[2] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[2].w = glm::dot(glm::vec3(FrustumPlanes[2]), vCameraPos);

    PlaneVec.z = std::sin(HalfAngleY);
    FrustumPlanes[3] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[3].w = glm::dot(glm::vec3(FrustumPlanes[3]), vCameraPos);
}

bool Camera3D::IsFaceInCameraFrustum(const RenderVertexSoft *pInVertices, int uNumVertices) const {
    if (uNumVertices <= 0) return false;

    bool inside = false;
    for (int p = 0; p < 4; p++) {
        inside = false;
        for (int v = 0; v < uNumVertices; v++) {
            float pLinelength1 = pInVertices[v].vWorldPosition.x * FrustumPlanes[p].x +
                                  pInVertices[v].vWorldPosition.y * FrustumPlanes[p].y +
                                  pInVertices[v].vWorldPosition.z * FrustumPlanes[p].z;

            inside = pLinelength1 >= FrustumPlanes[p].w;
            // break early when one passing vert is found for this plane
            if (inside == true) break;
        }
        // reject poly if not a single point is inside this plane
        if (inside == false) break;
    }

    return inside;
}

// used for culling to supplied portal frustums
// very sloppy check when using early break - different points could be passing plane checks
// NB only reliable where size of face is small in relation to size frustum
bool Camera3D::CullFaceToFrustum(RenderVertexSoft *a1, unsigned int *pOutNumVertices,
                    RenderVertexSoft *pVertices, Planef *frustum,
                    signed int NumFrustumPlanes) {
    if (NumFrustumPlanes <= 0)
        return false;
    if (*pOutNumVertices <= 0)
        return false;
    if (frustum == NULL)
        return true;

    bool inside = false;
    for (int p = 0; p < NumFrustumPlanes; p++) {
        inside = false;
        for (int v = 0; v < *pOutNumVertices; v++) {
            float pLinelength1 = dot(a1[v].vWorldPosition, frustum[p].normal);

            inside = (pLinelength1 + 5.0) >= -frustum[p].dist;  // added 5 for a bit of epsilon
            // break early when one passing vert is found for this plane
            if (inside == true) break;  // true for early break -  false for all points must be in
        }
        // reject poly if not a single point is inside this plane
        if (inside == false) break;
    }

    if (inside) {
        // copy in vcerts
        memcpy(pVertices, a1, sizeof(RenderVertexSoft) * *pOutNumVertices);
    } else {
        *pOutNumVertices = 0;
    }

    return inside;
}

bool Camera3D::ClipFaceToFrustum(RenderVertexSoft *pInVertices,
    unsigned int *pOutNumVertices,
    RenderVertexSoft *pVertices,
    const Planef *CameraFrustrum) {
    // DebugLines 0 or 1 - 1 when debug lines

    RenderVertexSoft *v14;  // eax@8
    RenderVertexSoft *v15;  // edx@8
    // float v17; // [sp+44h] [bp-10h]@1
    // int v18; // [sp+48h] [bp-Ch]@5
    bool VertsAdjusted = false;  // [sp+53h] [bp-1h]@5
    // bool a6a; // [sp+70h] [bp+1Ch]@5

    // v17 = 0.0;
    // thisa = engine->pStru9Instance;

    static RenderVertexSoft sr_vertices_50D9D8[64];

    // result = 0;
    // VertsAdjusted = 0;
    const int MinVertsAllowed = 3;

    // v12 = *pOutNumVertices;
    // v13 = (char *)&a4->y;

    // while ( 1 )
    for (unsigned i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {  // cycle through left,right, top, bottom planes
        if (i % 2) {
            v14 = pInVertices;
            v15 = sr_vertices_50D9D8;
        }  else {
            v15 = pInVertices;
            v14 = sr_vertices_50D9D8;
        }

        if (i == FRUSTUM_PLANE_COUNT - 1) v14 = pVertices;

        ClippingFunctions::ClipVertsToFrustumPlane(
            v15, *pOutNumVertices, v14, pOutNumVertices, &CameraFrustrum[i].normal, -CameraFrustrum[i].dist,
            &VertsAdjusted);

        // v12 = *pOutNumVertices;
        if (*pOutNumVertices < MinVertsAllowed) {
            *pOutNumVertices = 0;
            return true;
        }
        // result = a6a;
        // v13 += 24;
    }
    return VertsAdjusted;
}

Vec2f Camera3D::Project(const Vec3f& pos) const {
    float viewscalefactor = ViewPlaneDistPixels / pos.x;
    return { pViewport.center().x - viewscalefactor * pos.y , pViewport.center().y - viewscalefactor * pos.z };
}

Vec2f Camera3D::FitToViewport(const Vec2f& projPos) const {
    float minX = (float)pViewport.x;
    float maxX = (float)(pViewport.x + pViewport.w - 1);
    float minY = (float)pViewport.y;
    float maxY = (float)(pViewport.y + pViewport.h - 1);

    return { std::clamp(projPos.x, minX, maxX), std::clamp(projPos.y, minY, maxY) };
}

//----- (00436BB7) --------------------------------------------------------
void Camera3D::Project(RenderVertexSoft *pVertices, unsigned int uNumVertices, bool fit_into_viewport) {
    for (unsigned i = 0; i < uNumVertices; ++i) {
        auto v = pVertices + i;

        v->vWorldViewProj = Project(v->vWorldViewPosition);
        if (fit_into_viewport) {
            v->vWorldViewProj = FitToViewport(v->vWorldViewProj);
        }
    }
}

void Camera3D::CalculateRotations(int cameraYaw, int cameraPitch) {
    // NB original game inverts rotation direction of y axis
    _viewPitch = -cameraPitch;  // pitch
    _viewYaw = cameraYaw;  // yaw

    _yawRotationSine = std::sin((2 * M_PI) * _viewYaw / 2048.0);
    _yawRotationCosine = std::cos((2 * M_PI) * _viewYaw / 2048.0);

    _pitchRotationSine = std::sin((2 * M_PI) * _viewPitch / 2048.0);
    _pitchRotationCosine = std::cos((2 * M_PI) * _viewPitch / 2048.0);
}

void Camera3D::CullByNearClip(RenderVertexSoft *pverts, unsigned *unumverts) {
    float near = GetNearClip();

    if (!unumverts) return;
    for (unsigned i = 0; i < *unumverts; ++i) {
        if (pverts[i].vWorldViewPosition.x > near) {
            return;
        }
    }

    *unumverts = 0;
    return;
}

void Camera3D::CullByFarClip(RenderVertexSoft *pverts, unsigned *unumverts) {
    float far = GetFarClip();

    if (!unumverts) return;
    for (unsigned i = 0; i < *unumverts; ++i) {
        if (pverts[i].vWorldViewPosition.x < far) {
            return;
        }
    }

    *unumverts = 0;
    return;
}
