#include "Engine/Graphics/Camera.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Graphics/ClippingFunctions.h"

Camera3D *pCamera3D = new Camera3D;

//----- (004361EF) --------------------------------------------------------
Camera3D::Camera3D() {
    // debug_flags = 0;
    fRotationYCosine = 0;
    fRotationYSine = 0;
    fRotationZCosine = 0;
    fRotationZSine = 0;
}

//----- (0043643E) --------------------------------------------------------
float Camera3D::GetMouseInfoDepth() {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        return engine->config->gameplay.MouseInfoDepthOutdoor.Get();
    else
        return engine->config->gameplay.MouseInfoDepthIndoor.Get();
}

//----- (004364C5) --------------------------------------------------------
void Camera3D::ViewTransfrom_OffsetUV(RenderVertexSoft *pVertices,
                                             unsigned int uNumVertices,
                                             RenderVertexSoft *pOutVertices,
                                             LightsData *a5) {
    for (uint i = 0; i < uNumVertices; ++i) {
        pOutVertices[i].vWorldPosition.x = pVertices[i].vWorldPosition.x;
        pOutVertices[i].vWorldPosition.y = pVertices[i].vWorldPosition.y;
        pOutVertices[i].vWorldPosition.z = pVertices[i].vWorldPosition.z;

        pOutVertices[i].u = pVertices[i].u + a5->pDeltaUV[0];
        pOutVertices[i].v = pVertices[i].v + a5->pDeltaUV[1];
    }
    ViewTransform(pOutVertices, uNumVertices);
}

float Camera3D::GetNearClip() const {
    return engine->config->graphics.ClipNearDistance.Get();
}

float Camera3D::GetFarClip() const {
    return engine->config->graphics.ClipFarDistance.Get();
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
    for (uint i = 0; i < uNumVertices; ++i) {
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
bool Camera3D::GetFacetOrientation(PolygonType polyType, Vec3f *a2,
                                   Vec3f *a3, Vec3f *a4) {
    switch ((PolygonType)polyType) {
        case POLYGON_VerticalWall:
            a4->x = -a2->y;
            a4->y = a2->x;
            a4->z = 0.0;

            a3->x = 0.0;
            a3->y = 0.0;
            a3->z = 1.0f;

            return true;

        case POLYGON_Floor:
        case POLYGON_Ceiling:
            a4->x = 1.0;
            a4->y = 0.0;
            a4->z = 0.0;

            a3->x = 0.0;
            a3->y = 1.0;
            a3->z = 0.0;

            return true;

        case POLYGON_InBetweenFloorAndWall:
        case POLYGON_InBetweenCeilingAndWall:
            if (fabs(a2->z) < 0.70811361) {
                a4->x = -a2->y;
                a4->y = a2->x;
                a4->z = 0.0;
                a4->Normalize();

                a3->x = 0.0;
                a3->y = 0.0;
                a3->z = 1.0;
            } else {
                a4->x = 1.0;
                a4->y = 0.0;
                a4->z = 0.0;

                a3->x = 0.0;
                a3->y = 1.0;
                a3->z = 0.0;
            }

            return true;

        default:
            return false;
    }
}



//----- (00438258) --------------------------------------------------------
bool Camera3D::is_face_faced_to_cameraBLV(BLVFace *pFace) {
    // if (pFace->Portal()) return false;

    if (pFace->uNumVertices == 0)
        return false; // TODO(captainurist): would be great to just filter these our on load & assert instead.

    float x = pIndoor->pVertices[pFace->pVertexIDs[0]].x;
    float y = pIndoor->pVertices[pFace->pVertexIDs[0]].y;
    float z = pIndoor->pVertices[pFace->pVertexIDs[0]].z;

    if ((z - pCamera3D->vCameraPos.z) * pFace->pFacePlane.vNormal.z +
        (y - pCamera3D->vCameraPos.y) * pFace->pFacePlane.vNormal.y +
        (x - pCamera3D->vCameraPos.x) * pFace->pFacePlane.vNormal.x <
        0.0f)
        return true;

    return false;
}

bool Camera3D::is_face_faced_to_cameraODM(ODMFace* pFace, RenderVertexSoft* a2) {
    // if (pFace->Portal()) return false;

    if ((a2->vWorldPosition.z - pCamera3D->vCameraPos.z) * pFace->pFacePlane.vNormal.z +
        (a2->vWorldPosition.y - pCamera3D->vCameraPos.y) * pFace->pFacePlane.vNormal.y +
        (a2->vWorldPosition.x - pCamera3D->vCameraPos.x) * pFace->pFacePlane.vNormal.x <
        0.0f)
        return true;

    return false;
}

//----- (00437AB5) --------------------------------------------------------
void Camera3D::do_draw_debug_line_sw(RenderVertexSoft *pLineBegin,
                                            signed int sStartDiffuse32,
                                            RenderVertexSoft *pLineEnd,
                                            signed int sEndDiffuse32,
                                            unsigned int uOutNumVertices,
                                            float z_stuff) {
    RenderVertexSoft a1[20];         // [sp+8h] [bp-7C4h]@6
    RenderVertexSoft pVertices[20];  // [sp+3C8h] [bp-404h]@2
    RenderVertexD3D3 v24[2];         // [sp+788h] [bp-44h]@11

    // if ( render->pRenderD3D )
    //{
    for (uint i = 0; i < 20; i++) pVertices[i].flt_2C = 0.0;
    if ((char)uOutNumVertices) {
        pVertices[0].vWorldViewProjX = pLineBegin->vWorldViewProjX;
        pVertices[0].vWorldViewProjY = pLineBegin->vWorldViewProjY;

        pVertices[1].vWorldViewProjX = pLineEnd->vWorldViewProjX;
        pVertices[1].vWorldViewProjY = pLineEnd->vWorldViewProjY;
        v24[0].specular = 0;
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
        v24[1].specular = 0;
        v24[1].rhw = 0.001f;
        v24[1].texcoord.x = 0.0f;
        v24[1].texcoord.y = 0.0f;
        // v19 = render->pRenderD3D->pDevice;
        render->DrawLines(v24, 2);
        return;
    }
    for (uint i = 0; i < 20; i++) a1[i].flt_2C = 0.0;
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
        v24[0].specular = 0;
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
        v24[1].specular = 0;
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
                                       unsigned int uDiffuse32, float a5) {
    if (!uNumVertices) return;
    if ((signed int)(uNumVertices - 1) > 0) {
        for (uint i = 0; i < uNumVertices - 1; i++)
            do_draw_debug_line_sw(&a2[i], uDiffuse32, &a2[i + 1], uDiffuse32, 0,
                                  a5);
    }
    do_draw_debug_line_sw(&a2[uNumVertices - 1], uDiffuse32, a2, uDiffuse32, 0, a5);
}

//----- (004376E7) --------------------------------------------------------
void Camera3D::CreateViewMatrixAndProjectionScale() {
    // set up view transform matrix (NB orig game has y rotation direction reversed)
    float cos_y1 = fRotationYCosine, sin_y1 = fRotationYSine;
    float cos_z1 = fRotationZCosine, sin_z1 = fRotationZSine;

    // right handed co-ord system
    // world +ve x east, +ve y north, +ve z up
    // view +ve x in, +ve y left, +ve z up

    glm::mat3x3 newpitch(cos_y1, 0, -sin_y1, 0, 1, 0, sin_y1, 0, cos_y1);
    glm::mat3x3 newyaw(cos_z1, sin_z1, 0, -sin_z1, cos_z1, 0, 0, 0, 1);
    glm::mat3x3 newroll(1);

    ViewMatrix = newyaw * newpitch * newroll;

    // TODO(pskelton): fov calcs only need recalculating on level change or if we add config option
    // fov projection calcs
    unit_fov = 0.5 / tan(odm_fov_rad / 2.0);
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        unit_fov = 0.5 / tan(blv_fov_rad / 2.0);

    ViewPlaneDist_X = (double)pViewport->uScreenWidth * unit_fov;
    ViewPlaneDist_Y = (double)pViewport->uScreenHeight * unit_fov;

    // calculate vertical FOV in degrees for GL rendering
    fov_y_deg = (180.0 / pi) * 2.0 * atan((game_viewport_height / 2.0) / pCamera3D->ViewPlaneDist_X);

    screenCenterX = (double)pViewport->uScreenCenterX;
    screenCenterY = (double)pViewport->uScreenCenterY - pViewport->uScreen_TL_Y;

    aspect = float(game_viewport_width / float(game_viewport_height));
}

//----- (004374E8) --------------------------------------------------------
void Camera3D::BuildViewFrustum() {
    float HalfAngleX = (pi / 2.0) - (odm_fov_rad / 2.0);
    float HalfAngleY = (pi / 2.0) - (atan((game_viewport_height / 2.0) / pCamera3D->ViewPlaneDist_X));

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        HalfAngleX = (pi / 2.0) - (blv_fov_rad / 2.0);
    }

    glm::vec3 PlaneVec(0);

    // rotate (1,0,0) around z
    PlaneVec.y = -sin(HalfAngleX);
    PlaneVec.x = cos(HalfAngleX);
    PlaneVec.z = 0.0;
    FrustumPlanes[0] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[0].w = glm::dot(glm::vec3(FrustumPlanes[0]), vCameraPos);

    PlaneVec.y = sin(HalfAngleX);
    FrustumPlanes[1] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[1].w = glm::dot(glm::vec3(FrustumPlanes[1]), vCameraPos);

    // rotate (1,0,0) around y
    PlaneVec.z = -sin(HalfAngleY);
    PlaneVec.y = 0.0;
    PlaneVec.x = cos(HalfAngleY);
    FrustumPlanes[2] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[2].w = glm::dot(glm::vec3(FrustumPlanes[2]), vCameraPos);

    PlaneVec.z = sin(HalfAngleY);
    FrustumPlanes[3] = glm::vec4(ViewMatrix * PlaneVec, 1.0);
    FrustumPlanes[3].w = glm::dot(glm::vec3(FrustumPlanes[3]), vCameraPos);
}


//----- (00437376) --------------------------------------------------------
// culls vertices to face plane
bool Camera3D::CullVertsToPlane(stru154 *faceplane, RenderVertexSoft *vertices,
                              unsigned int *pOutNumVertices) {
    double v6;             // st7@3
    int previous;          // esi@6
    int current;           // ebx@8
    int next;              // eax@8
    int temp_t;               // eax@15
    signed int v14;        // ebx@17
    RenderVertexSoft v18;  // [sp+Ch] [bp-34h]@2
                           //  signed int thisb; // [sp+48h] [bp+8h]@6
    bool result;           // [sp+4Fh] [bp+Fh]@5

    memcpy(&v18, vertices, sizeof(v18));
    result = false;
    memcpy(&vertices[*pOutNumVertices], vertices, sizeof(vertices[*pOutNumVertices]));
    memcpy(&vertices[*pOutNumVertices + 1], &vertices[1], sizeof(vertices[*pOutNumVertices + 1]));

    if ((signed int)*pOutNumVertices <= 3 ||
        (((v18.vWorldPosition.z - (double)pCamera3D->vCameraPos.z) *
            faceplane->face_plane.vNormal.z +
              (v18.vWorldPosition.y - (double)pCamera3D->vCameraPos.y) *
            faceplane->face_plane.vNormal.y +
              (v18.vWorldPosition.x - (double)pCamera3D->vCameraPos.x) *
            faceplane->face_plane.vNormal.x <
          0.0)
             ? (v6 = 1.0)
             : (v6 = -1.0),
         (signed int)*pOutNumVertices <= 0))
        return 0;

    for (int i = 1; i - 1 < (signed int)*pOutNumVertices; i++) {
        current = i;
        next = i + 1;

        previous = i - 1;
        if (previous >= (signed int)*pOutNumVertices)
            previous -= *pOutNumVertices;

        if (current >= (signed int)*pOutNumVertices)
            current -= *pOutNumVertices;

        if (next >= (signed int)*pOutNumVertices) next -= *pOutNumVertices;

        // nearest approximation of float 0.01
        if (-0.009999999776482582 <
            ((vertices[current].vWorldViewProjX - vertices[previous].vWorldViewProjX) *
                 (vertices[next].vWorldViewProjY - vertices[previous].vWorldViewProjY) -
             (vertices[current].vWorldViewProjY - vertices[previous].vWorldViewProjY) *
                 (vertices[next].vWorldViewProjX - vertices[previous].vWorldViewProjX)) *
                v6) {
            temp_t = next;
            if (next >= (signed int)*pOutNumVertices)
                temp_t = next - *pOutNumVertices;

            if (temp_t < (signed int)*pOutNumVertices) {
                for (v14 = temp_t; v14 < (signed int)*pOutNumVertices; ++v14)
                    memcpy(&vertices[v14], &vertices[v14 + 1], sizeof(vertices[v14]));
            }
            result = true;
            --*pOutNumVertices;
        }
    }
    if (result)
        return true;
    else
        return false;
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

    __debugbreak();

    return false;
}

// used for culling to supplied portal frustums
// very sloppy check when using early break - different points could be passing plane checks
// NB only reliable where size of face is small in relation to size frustum
bool Camera3D::CullFaceToFrustum(struct RenderVertexSoft* a1, unsigned int* pOutNumVertices,
                    struct RenderVertexSoft* pVertices, struct IndoorCameraD3D_Vec4* frustum,
                    signed int NumFrustumPlanes) {
    if (NumFrustumPlanes <= 0) return false;
    if (*pOutNumVertices <= 0) return false;
    if (frustum == NULL) return true;

    bool inside = false;
    for (int p = 0; p < NumFrustumPlanes; p++) {
        inside = false;
        for (int v = 0; v < *pOutNumVertices; v++) {
            float pLinelength1 = a1[v].vWorldPosition.x * frustum[p].x +
                a1[v].vWorldPosition.y * frustum[p].y +
                a1[v].vWorldPosition.z * frustum[p].z;

            inside = (pLinelength1 + 5.0) >= frustum[p].dot;  // added 5 for a bit of epsilon
            // break early when one passing vert is found for this plane
            if (inside == true) break;  // true for early break -  false for all points must be in
        }
        // reject poly if not a single point is inside this plane
        if (inside == false) break;
    }

    if (inside == false) {
        *pOutNumVertices = 0;
        return false;
    } else {
        // copy in vcerts
        memcpy(pVertices, a1, sizeof(RenderVertexSoft) * *pOutNumVertices);
        // return true
        return true;
    }

    __debugbreak();

    return false;
}

bool Camera3D::ClipFaceToFrustum(RenderVertexSoft* pInVertices,
    unsigned int* pOutNumVertices,
    RenderVertexSoft* pVertices,
    IndoorCameraD3D_Vec4* CameraFrustrum,
    signed int NumFrustumPlanes, char DebugLines,
    int _unused) {
    // NumFrustumPlanes usually 4 - top, bottom, left, right - near and far done elsewhere
    // DebugLines 0 or 1 - 1 when debug lines

    RenderVertexSoft* v14;  // eax@8
    RenderVertexSoft* v15;  // edx@8
    Vec3f FrustumPlaneVec {};         // [sp+18h] [bp-3Ch]@12
    // float v17; // [sp+44h] [bp-10h]@1
    // int v18; // [sp+48h] [bp-Ch]@5
    // stru9 *thisa; // [sp+4Ch] [bp-8h]@1
    int VertsAdjusted = 0;  // [sp+53h] [bp-1h]@5
    // bool a6a; // [sp+70h] [bp+1Ch]@5

    // v17 = 0.0;
    // thisa = engine->pStru9Instance;

    static RenderVertexSoft sr_vertices_50D9D8[64];

    // result = 0;
    // VertsAdjusted = 0;
    int MinVertsAllowed = 2 * (DebugLines == 0) + 1;  // 3 normally 1 for debuglines
    // a6a = 0;
    // v18 = MinVertsAllowed;
    if (NumFrustumPlanes <= 0) return false;

    // v12 = *pOutNumVertices;
    // v13 = (char *)&a4->y;

    // while ( 1 )
    for (uint i = 0; i < NumFrustumPlanes; ++i) {  // cycle through left,right, top, bottom planes
        if (i % 2) {
            v14 = pInVertices;
            v15 = sr_vertices_50D9D8;
        }  else {
            v15 = pInVertices;
            v14 = sr_vertices_50D9D8;
        }

        if (i == NumFrustumPlanes - 1) v14 = pVertices;
        FrustumPlaneVec.x = CameraFrustrum[i].x;
        FrustumPlaneVec.y = CameraFrustrum[i].y;
        FrustumPlaneVec.z = CameraFrustrum[i].z;

        engine->pStru9Instance->ClipVertsToFrustumPlane(
            v15, *pOutNumVertices, v14, pOutNumVertices, &FrustumPlaneVec, CameraFrustrum[i].dot,
            (char*)&VertsAdjusted, _unused);

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

    for (uint i = 0; i < uNumVertices; ++i) {
        auto v = pVertices + i;

        RHW = 1.0 / (v->vWorldViewPosition.x + 0.0000001);
        v->_rhw = RHW;
        viewscalefactor = RHW * ViewPlaneDist_X;

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

void Camera3D::CalculateRotations(int camera_rot_y, int camera_rot_z) {
    // NB original game inverts rotation direction of y axis
    sRotationY = -camera_rot_y;  // pitch
    sRotationZ = camera_rot_z;  // yaw

    fRotationZSine = sin((pi_double + pi_double) * (double)sRotationZ / 2048.0);
    fRotationZCosine = cos((pi_double + pi_double) * (double)sRotationZ / 2048.0);

    fRotationYSine = sin((pi_double + pi_double) * (double)sRotationY / 2048.0);
    fRotationYCosine = cos((pi_double + pi_double) * (double)sRotationY / 2048.0);
}

//----- (00436A6D) --------------------------------------------------------
float Camera3D::GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int uStripType) {
    float result = FLT_MAX;
    for (uint i = 0; i < uStripType; i++) {
        if (pVertices[i].vWorldPosition.z < result) {
            result = pVertices[i].vWorldPosition.z;
        }
    }
    return result;
}

//----- (00436A40) --------------------------------------------------------
float Camera3D::GetPolygonMaxZ(RenderVertexSoft* pVertex, unsigned int uStripType) {
    float result = FLT_MIN;
    for (uint i = 0; i < uStripType; i++) {
        if (pVertex[i].vWorldPosition.z > result)
            result = pVertex[i].vWorldPosition.z;
    }
    return result;
}

void Camera3D::CullByNearClip(RenderVertexSoft* pverts, uint* unumverts) {
    float near = GetNearClip();

    if (!unumverts) return;
    for (uint i = 0; i < *unumverts; ++i) {
        if (pverts[i].vWorldViewPosition.x > near) {
            return;
        }
    }

    *unumverts = 0;
    return;
}

void Camera3D::CullByFarClip(RenderVertexSoft* pverts, uint* unumverts) {
    float far = GetFarClip();

    if (!unumverts) return;
    for (uint i = 0; i < *unumverts; ++i) {
        if (pverts[i].vWorldViewPosition.x < far) {
            return;
        }
    }

    *unumverts = 0;
    return;
}
