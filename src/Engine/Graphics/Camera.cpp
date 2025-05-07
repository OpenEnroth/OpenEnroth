#include "Engine/Graphics/Camera.h"

#include "Engine/Engine.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Renderer/Renderer.h"

#include "Engine/Graphics/ClippingFunctions.h"

Camera3D *pCamera3D = new Camera3D;

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

// ViewTransformAndClipTest
bool Camera3D::ViewClip(int x, int y, int z, int *transformed_x,
                               int *transformed_y, int *transformed_z,
                               bool dont_show) {
    this->ViewTransform(x, y, z, transformed_x, transformed_y, transformed_z);

    if (dont_show) {
        return false;
    }
    return *transformed_x >= this->GetNearClip() &&
           *transformed_x <= this->GetFarClip();
}

void Camera3D::ViewTransform(int x, int y, int z, int *transformed_x, int *transformed_y, int *transformed_z) {
    RenderVertexSoft v;
    v.vWorldPosition.x = x;
    v.vWorldPosition.y = y;
    v.vWorldPosition.z = z;

    this->ViewTransform(&v, 1);
    if (transformed_x)
        *transformed_x = std::round(v.vWorldViewPosition.x + 0.5f);

    if (transformed_y)
        *transformed_y = std::round(v.vWorldViewPosition.y + 0.5f);

    if (transformed_z)
        *transformed_z = std::round(v.vWorldViewPosition.z + 0.5f);
}

//----- (00436523) --------------------------------------------------------
void Camera3D::ViewTransform(RenderVertexSoft *a1a, unsigned int uNumVertices) {
    for (unsigned i = 0; i < uNumVertices; ++i) {
        RenderVertexSoft *a1 = &a1a[i];

        double vCamToVertexX = (double)a1->vWorldPosition.x - (double)pCamera3D->vCameraPos.x;
        double vCamToVertexY = (double)a1->vWorldPosition.y - (double)pCamera3D->vCameraPos.y;
        double vCamToVertexZ = (double)a1->vWorldPosition.z - (double)pCamera3D->vCameraPos.z;

        glm::vec3 camtovert(vCamToVertexX, vCamToVertexY, vCamToVertexZ);
        camtovert = camtovert * ViewMatrix;
        a1->vWorldViewPosition.x = camtovert.x;
        a1->vWorldViewPosition.y = camtovert.y;
        a1->vWorldViewPosition.z = camtovert.z;
    }
}

//----- (00436932) --------------------------------------------------------
// TODO(captainurist): function belongs to stru314
void Camera3D::GetFacetOrientation(const Vec3f &normal, Vec3f *outU, Vec3f *outV) {
    if (fabsf(normal.z) < 1e-6f) {
        // Vertical wall.
        outV->x = -normal.y;
        outV->y = normal.x;
        outV->z = 0.0;

        outU->x = 0.0;
        outU->y = 0.0;
        outU->z = 1.0f;
    } else if (fabsf(normal.x) < 1e-6f && fabsf(normal.y) < 1e-6f) {
        // Floor.
        outV->x = 1.0;
        outV->y = 0.0;
        outV->z = 0.0;

        outU->x = 0.0;
        outU->y = 1.0;
        outU->z = 0.0;
    } else {
        // Other.
        if (fabs(normal.z) < 0.70811361) {
            outV->x = -normal.y;
            outV->y = normal.x;
            outV->z = 0.0;
            outV->normalize();

            outU->x = 0.0;
            outU->y = 0.0;
            outU->z = 1.0;
        } else {
            outV->x = 1.0;
            outV->y = 0.0;
            outV->z = 0.0;

            outU->x = 0.0;
            outU->y = 1.0;
            outU->z = 0.0;
        }
    }
}



//----- (00438258) --------------------------------------------------------
bool Camera3D::is_face_faced_to_cameraBLV(BLVFace *pFace) {
    return pFace->facePlane.dist +
        pCamera3D->vCameraPos.z * pFace->facePlane.normal.z +
        pCamera3D->vCameraPos.y * pFace->facePlane.normal.y +
        pCamera3D->vCameraPos.x * pFace->facePlane.normal.x >
        0.0f;
}

bool Camera3D::is_face_faced_to_cameraODM(ODMFace *pFace, RenderVertexSoft *a2) {
    // if (pFace->Portal()) return false;

    if ((a2->vWorldPosition.z - pCamera3D->vCameraPos.z) * pFace->facePlane.normal.z +
        (a2->vWorldPosition.y - pCamera3D->vCameraPos.y) * pFace->facePlane.normal.y +
        (a2->vWorldPosition.x - pCamera3D->vCameraPos.x) * pFace->facePlane.normal.x <
        0.0f)
        return true;

    return false;
}

//----- (00437AB5) --------------------------------------------------------
void Camera3D::do_draw_debug_line_sw(RenderVertexSoft *pLineBegin,
                                            Color sStartDiffuse32,
                                            RenderVertexSoft *pLineEnd,
                                            Color sEndDiffuse32,
                                            unsigned int uOutNumVertices,
                                            float z_stuff) {
    RenderVertexSoft a1[20];         // [sp+8h] [bp-7C4h]@6
    RenderVertexSoft pVertices[20];  // [sp+3C8h] [bp-404h]@2
    RenderVertexD3D3 v24[2];         // [sp+788h] [bp-44h]@11

    // if ( render->pRenderD3D )
    //{
    for (unsigned i = 0; i < 20; i++) pVertices[i].flt_2C = 0.0;
    if ((char)uOutNumVertices) {
        pVertices[0].vWorldViewProjX = pLineBegin->vWorldViewProjX;
        pVertices[0].vWorldViewProjY = pLineBegin->vWorldViewProjY;

        pVertices[1].vWorldViewProjX = pLineEnd->vWorldViewProjX;
        pVertices[1].vWorldViewProjY = pLineEnd->vWorldViewProjY;
        v24[0].specular = Color();
        v24[0].pos.x = pVertices[0].vWorldViewProjX;
        v24[0].pos.y = pVertices[0].vWorldViewProjY;
        v24[0].pos.z = 0.001 - z_stuff;
        v24[0].diffuse = sStartDiffuse32;
        v24[0].rhw = 0.001f;
        v24[0].texcoord.x = 0.0f;
        v24[0].texcoord.y = 0.0f;

        v24[1].pos.x = pVertices[1].vWorldViewProjX;
        v24[1].pos.y = pVertices[1].vWorldViewProjY;
        v24[1].diffuse = sEndDiffuse32;
        v24[1].pos.z = 0.001 - z_stuff;
        v24[1].specular = Color();
        v24[1].rhw = 0.001f;
        v24[1].texcoord.x = 0.0f;
        v24[1].texcoord.y = 0.0f;
        // v19 = render->pRenderD3D->pDevice;
        render->DrawLines(v24, 2);
        return;
    }
    for (unsigned i = 0; i < 20; i++) a1[i].flt_2C = 0.0;
    uOutNumVertices = 2;
    a1[0].vWorldPosition.x = pLineBegin->vWorldPosition.x;
    a1[0].vWorldPosition.y = pLineBegin->vWorldPosition.y;
    a1[0].vWorldPosition.z = pLineBegin->vWorldPosition.z;
    a1[1].vWorldPosition.x = pLineEnd->vWorldPosition.x;
    a1[1].vWorldPosition.y = pLineEnd->vWorldPosition.y;
    a1[1].vWorldPosition.z = pLineEnd->vWorldPosition.z;
    if (CullFaceToCameraFrustum(a1, &uOutNumVertices, pVertices, 4) != 1 ||
        (signed int)uOutNumVertices >= 2) {
        ViewTransform(pVertices, 2);
        Project(pVertices, 2, 0);
        v24[0].specular = Color();
        v24[0].pos.x = pVertices[0].vWorldViewProjX;
        v24[0].pos.y = pVertices[0].vWorldViewProjY;
        v24[0].pos.z = 0.001 - z_stuff;
        v24[0].diffuse = sStartDiffuse32;
        v24[0].rhw = 0.001f;
        v24[0].texcoord.x = 0.0;
        v24[0].texcoord.y = 0.0;

        v24[1].pos.x = pVertices[1].vWorldViewProjX;
        v24[1].pos.y = pVertices[1].vWorldViewProjY;
        v24[1].diffuse = sEndDiffuse32;
        v24[1].pos.z = 0.001 - z_stuff;
        v24[1].specular = Color();
        v24[1].rhw = 0.001f;
        v24[1].texcoord.x = 0.0;
        v24[1].texcoord.y = 0.0;
        // v19 = render->pRenderD3D->pDevice;
        render->DrawLines(v24, 2);
        return;
    }
    //}
}

//----- (004379EE) --------------------------------------------------------
void Camera3D::debug_outline_sw(RenderVertexSoft *a2,
                                       unsigned int uNumVertices,
                                       Color uDiffuse32, float a5) {
    if (!uNumVertices) return;
    if ((signed int)(uNumVertices - 1) > 0) {
        for (unsigned i = 0; i < uNumVertices - 1; i++)
            do_draw_debug_line_sw(&a2[i], uDiffuse32, &a2[i + 1], uDiffuse32, 0,
                                  a5);
    }
    do_draw_debug_line_sw(&a2[uNumVertices - 1], uDiffuse32, a2, uDiffuse32, 0, a5);
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

    ViewPlaneDistPixels = (double)pViewport->uScreenWidth * 0.5 / halfFovTan;

    // calculate vertical FOV in degrees for GL rendering
    fov_y_deg = (180.0 / pi) * 2.0 * std::atan((game_viewport_height / 2.0) / pCamera3D->ViewPlaneDistPixels);

    screenCenterX = (double)pViewport->uScreenCenterX;
    screenCenterY = (double)pViewport->uScreenCenterY - pViewport->uScreen_TL_Y;

    aspect = float(game_viewport_width / float(game_viewport_height));
}

//----- (004374E8) --------------------------------------------------------
void Camera3D::BuildViewFrustum() {
    float HalfAngleX = (pi / 2.0) - (odm_fov_rad / 2.0);
    float HalfAngleY = (pi / 2.0) - (std::atan((game_viewport_height / 2.0) / pCamera3D->ViewPlaneDistPixels));

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        HalfAngleX = (pi / 2.0) - (blv_fov_rad / 2.0);
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

// TODO(pskelton): does this func need to copy verts or could it be eliminated
//----- (00437285) --------------------------------------------------------
bool Camera3D::CullFaceToCameraFrustum(RenderVertexSoft *pInVertices,
                                      unsigned int *pOutNumVertices,
                                      RenderVertexSoft *pVertices,
                                      signed int NumFrustumPlanes) {
    if (NumFrustumPlanes <= 0) return false;
    if (*pOutNumVertices <= 0) return false;

    bool inside = false;
    for (int p = 0; p < NumFrustumPlanes; p++) {
        inside = false;
        for (int v = 0; v < *pOutNumVertices; v++) {
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

    if (inside == false) {
        *pOutNumVertices = 0;
        return false;
    } else {
        // copy in vcerts
        memcpy(pVertices, pInVertices, sizeof(RenderVertexSoft) * *pOutNumVertices);
        // return true
        return true;
    }

    assert(false);

    return false;
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
    // NumFrustumPlanes usually 4 - top, bottom, left, right - near and far done elsewhere
    // DebugLines 0 or 1 - 1 when debug lines

    RenderVertexSoft *v14;  // eax@8
    RenderVertexSoft *v15;  // edx@8
    // float v17; // [sp+44h] [bp-10h]@1
    // int v18; // [sp+48h] [bp-Ch]@5
    bool VertsAdjusted = false;  // [sp+53h] [bp-1h]@5
    // bool a6a; // [sp+70h] [bp+1Ch]@5

    // TODO(yoctozepto): just have this as a global constant instead of random vars/4 around
    const int NumFrustumPlanes = 4;

    // v17 = 0.0;
    // thisa = engine->pStru9Instance;

    static RenderVertexSoft sr_vertices_50D9D8[64];

    // result = 0;
    // VertsAdjusted = 0;
    const int MinVertsAllowed = 3;

    // v12 = *pOutNumVertices;
    // v13 = (char *)&a4->y;

    // while ( 1 )
    for (unsigned i = 0; i < NumFrustumPlanes; ++i) {  // cycle through left,right, top, bottom planes
        if (i % 2) {
            v14 = pInVertices;
            v15 = sr_vertices_50D9D8;
        }  else {
            v15 = pInVertices;
            v14 = sr_vertices_50D9D8;
        }

        if (i == NumFrustumPlanes - 1) v14 = pVertices;

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
        // if (++i >= FrustumPlanes)
        //
    }
    return VertsAdjusted;
}


//----- (00436BB7) --------------------------------------------------------
void Camera3D::Project(RenderVertexSoft *pVertices, unsigned int uNumVertices, bool fit_into_viewport) {
    double fitted_x;
    double temp_r;
    double temp_l;
    double fitted_y;
    double temp_b;
    double temp_t;
    double RHW;
    double viewscalefactor;

    for (unsigned i = 0; i < uNumVertices; ++i) {
        auto v = pVertices + i;

        RHW = 1.0 / (v->vWorldViewPosition.x + 0.0000001);
        v->_rhw = RHW;
        viewscalefactor = RHW * ViewPlaneDistPixels;

        v->vWorldViewProjX = (double)pViewport->uScreenCenterX -
                             viewscalefactor * (double)v->vWorldViewPosition.y;
        v->vWorldViewProjY = (double)pViewport->uScreenCenterY -
                             viewscalefactor * (double)v->vWorldViewPosition.z;

        if (fit_into_viewport) {
            fitted_x = (double)(signed int)pViewport->uViewportBR_X;
            if (fitted_x >= pVertices[i].vWorldViewProjX)
                temp_r = pVertices[i].vWorldViewProjX;
            else
                temp_r = fitted_x;
            temp_l = (double)(signed int)pViewport->uViewportTL_X;
            if (temp_l <= temp_r) {
                if (fitted_x >= pVertices[i].vWorldViewProjX)
                    fitted_x = pVertices[i].vWorldViewProjX;
            } else {
                fitted_x = temp_l;
            }
            pVertices[i].vWorldViewProjX = fitted_x;

            fitted_y = (double)(signed int)pViewport->uViewportBR_Y;
            if (fitted_y >= pVertices[i].vWorldViewProjY)
                temp_b = pVertices[i].vWorldViewProjY;
            else
                temp_b = fitted_y;
            temp_t = (double)(signed int)pViewport->uViewportTL_Y;
            if (temp_t <= temp_b) {
                if (fitted_y >= pVertices[i].vWorldViewProjY)
                    fitted_y = pVertices[i].vWorldViewProjY;
            } else {
                fitted_y = temp_t;
            }
            pVertices[i].vWorldViewProjY = fitted_y;
        }
    }
}

void Camera3D::Project(int x, int y, int z, int *screenspace_x, int *screenspace_y) {
    RenderVertexSoft v;
    v.vWorldViewPosition.x = x;
    v.vWorldViewPosition.y = y;
    v.vWorldViewPosition.z = z;

    this->Project(&v, 1, false);

    if (screenspace_x) {
        *screenspace_x = floorf(v.vWorldViewProjX + 0.5f);
    }

    if (screenspace_y) {
        *screenspace_y = floorf(v.vWorldViewProjY + 0.5f);
    }
}

void Camera3D::CalculateRotations(int cameraYaw, int cameraPitch) {
    // NB original game inverts rotation direction of y axis
    _viewPitch = -cameraPitch;  // pitch
    _viewYaw = cameraYaw;  // yaw

    _yawRotationSine = std::sin((pi_double + pi_double) * _viewYaw / 2048.0);
    _yawRotationCosine = std::cos((pi_double + pi_double) * _viewYaw / 2048.0);

    _pitchRotationSine = std::sin((pi_double + pi_double) * _viewPitch / 2048.0);
    _pitchRotationCosine = std::cos((pi_double + pi_double) * _viewPitch / 2048.0);
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
