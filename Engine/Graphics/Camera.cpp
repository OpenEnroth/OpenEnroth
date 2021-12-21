#include "Engine/Graphics/Camera.h"

#include <cstdlib>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Log.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Graphics/stru9.h"

Camera3D *pCamera3D = new Camera3D;

//----- (004361EF) --------------------------------------------------------
Camera3D::Camera3D() {
    debug_flags = 0;
    fRotationXCosine = 0;
    fRotationXSine = 0;
    fRotationZCosine = 0;
    fRotationZSine = 0;
}

//----- (0043643E) --------------------------------------------------------
float Camera3D::GetPickDepth() {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        return pODMRenderParams->uPickDepth;
    else
        return 16192.0;
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

//----- (00436455) --------------------------------------------------------
bool Camera3D::IsCulled(BLVFace *pFace) {
    RenderVertexSoft v;  // [sp+8h] [bp-30h]@1

    //----- (0043648F) --------------------------------------------------------
    auto Vec3_short__to_RenderVertexSoft = [](RenderVertexSoft *_this,
                                              Vec3_short_ *a2) -> void {
        _this->flt_2C = 0.0;

        _this->vWorldPosition.x = a2->x;
        _this->vWorldPosition.y = a2->y;
        _this->vWorldPosition.z = a2->z;
    };

    Vec3_short__to_RenderVertexSoft(&v,
                                    &pIndoor->pVertices[*pFace->pVertexIDs]);
    return is_face_faced_to_camera(pFace, &v);
}

float Camera3D::GetNearClip() const {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        if (pODMRenderParams) {
            return pODMRenderParams->near_clip;
        }

        return 4.0f;
    } else {
        // return (double)pBLVRenderParams->fov_rad_fixpoint / 65536.0f;
        return 4.0f;
    }
}

float Camera3D::GetFarClip() const {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        if (pODMRenderParams) {
            return pODMRenderParams->far_clip;
        }

        return (float)0x2000;
    } else {
        return 16192.0f;
    }
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

        if (pCamera3D->sRotationX) {
            double v5 = vCamToVertexY * (double)fRotationZSine + (double)fRotationZCosine * vCamToVertexX;

            a1->vWorldViewPosition.x = v5 * (double)fRotationXCosine /*+*/ - (double)fRotationXSine * vCamToVertexZ;
            a1->vWorldViewPosition.y = (double)fRotationZCosine * vCamToVertexY - (double)fRotationZSine * vCamToVertexX;
            a1->vWorldViewPosition.z = (double)fRotationXCosine * vCamToVertexZ /*-*/ + v5 * (double)fRotationXSine;
        } else {
            a1->vWorldViewPosition.x = (double)fRotationZSine * vCamToVertexY + (double)fRotationZCosine * vCamToVertexX;
            a1->vWorldViewPosition.y = (double)fRotationZCosine * vCamToVertexY - (double)fRotationZSine * vCamToVertexX;
            a1->vWorldViewPosition.z = vCamToVertexZ;
        }

        //camtovert = camtovert * camrotation;
        //int y = 7;
    }

}

//----- (00436932) --------------------------------------------------------
bool Camera3D::GetFacetOrientation(char polyType, Vec3_float_ *a2,
                                          Vec3_float_ *a3, Vec3_float_ *a4) {
    switch ((PolygonType)polyType) {
        case POLYGON_VerticalWall: {
            a4->x = -a2->y;
            a4->y = a2->x;
            a4->z = 0.0;

            a3->x = 0.0;
            a3->y = 0.0;
            a3->z = 1.0f;
        }
            return true;

        case POLYGON_Floor:
        case POLYGON_Ceiling: {
            a4->x = 1.0;
            a4->y = 0.0;
            a4->z = 0.0;

            a3->x = 0.0;
            a3->y = 1.0;
            a3->z = 0.0;
        }
            return true;

        case POLYGON_InBetweenFloorAndWall:
        case POLYGON_InBetweenCeilingAndWall: {
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
        }
            return true;

        default:
            return false;
    }
}

//----- (00438258) --------------------------------------------------------
bool Camera3D::is_face_faced_to_camera(BLVFace *pFace,
                                              RenderVertexSoft *a2) {
    if (pFace->Portal()) return false;

    // really strange cull; dot(to_cam, normal) < 0 means we see the BACK face,
    // not font %_%
    if ((a2->vWorldPosition.z - (double)pCamera3D->vCameraPos.z) *
                (double)pFace->pFacePlane_old.vNormal.z +
            (a2->vWorldPosition.y - (double)pCamera3D->vCameraPos.y) *
                (double)pFace->pFacePlane_old.vNormal.y +
            (a2->vWorldPosition.x - (double)pCamera3D->vCameraPos.x) *
                (double)pFace->pFacePlane_old.vNormal.x <
        0.0)
        return false;

    return true;
}

//----- (00437AB5) --------------------------------------------------------
void Camera3D::do_draw_debug_line_sw(RenderVertexSoft *pLineBegin,
                                            signed int sStartDiffuse,
                                            RenderVertexSoft *pLineEnd,
                                            signed int sEndDiffuse,
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
        v24[0].diffuse = sStartDiffuse;
        v24[0].rhw = 0.001;
        v24[0].texcoord.x = 0.0;
        v24[0].texcoord.y = 0.0;

        v24[1].pos.x = pVertices[1].vWorldViewProjX;
        v24[1].pos.y = pVertices[1].vWorldViewProjY;
        v24[1].diffuse = sEndDiffuse;
        v24[1].pos.z = 0.001 - z_stuff;
        v24[1].specular = 0;
        v24[1].rhw = 0.001;
        v24[1].texcoord.x = 0.0;
        v24[1].texcoord.y = 0.0;
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
    if (CullFaceToFrustum(a1, &uOutNumVertices, pVertices, 4, 1,
                        0) != 1 ||
        (signed int)uOutNumVertices >= 2) {
        ViewTransform(pVertices, 2);
        Project(pVertices, 2, 0);
        v24[0].specular = 0;
        v24[0].pos.x = pVertices[0].vWorldViewProjX;
        v24[0].pos.y = pVertices[0].vWorldViewProjY;
        v24[0].pos.z = 0.001 - z_stuff;
        v24[0].diffuse = sStartDiffuse;
        v24[0].rhw = 0.001;
        v24[0].texcoord.x = 0.0;
        v24[0].texcoord.y = 0.0;

        v24[1].pos.x = pVertices[1].vWorldViewProjX;
        v24[1].pos.y = pVertices[1].vWorldViewProjY;
        v24[1].diffuse = sEndDiffuse;
        v24[1].pos.z = 0.001 - z_stuff;
        v24[1].specular = 0;
        v24[1].rhw = 0.001;
        v24[1].texcoord.x = 0.0;
        v24[1].texcoord.y = 0.0;
        // v19 = render->pRenderD3D->pDevice;
        render->DrawLines(v24, 2);
        return;
    }
    //}
}

//----- (00437A55) --------------------------------------------------------
void Camera3D::debug_outline_d3d(const RenderVertexD3D3 *pLineVertices,
                                        unsigned int uNumLines, int uDiffuse,
                                        float z_stuff) {
    for (int i = 0; i < uNumLines - 1; ++i)
        render->do_draw_debug_line_d3d(&pLineVertices[i], uDiffuse,
                                       &pLineVertices[i + 1], uDiffuse,
                                       z_stuff);
    render->do_draw_debug_line_d3d(&pLineVertices[uNumLines - 1], uDiffuse,
                                   pLineVertices, uDiffuse, z_stuff);
}

//----- (004379EE) --------------------------------------------------------
void Camera3D::debug_outline_sw(RenderVertexSoft *a2,
                                       unsigned int uNumVertices,
                                       unsigned int uDiffuse, float a5) {
    if (!uNumVertices) return;
    if ((signed int)(uNumVertices - 1) > 0) {
        for (uint i = 0; i < uNumVertices - 1; i++)
            do_draw_debug_line_sw(&a2[i], uDiffuse, &a2[i + 1], uDiffuse, 0,
                                  a5);
    }
    do_draw_debug_line_sw(&a2[uNumVertices - 1], uDiffuse, a2, uDiffuse, 0, a5);
}

void Camera3D::DebugDrawPortal(BLVFace *pFace) {
    Assert(pFace->uNumVertices <= 32);

    RenderVertexSoft sw[32];
    for (uint i = 0; i < pFace->uNumVertices; ++i) {
        sw[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
        sw[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
        sw[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
    }
    ViewTransform(sw, pFace->uNumVertices);
    Project(sw, pFace->uNumVertices, 0);

    RenderVertexD3D3 v[32];
    for (uint i = 0; i < pFace->uNumVertices; ++i) {
        v[i].pos.x = sw[i].vWorldViewProjX;
        v[i].pos.y = sw[i].vWorldViewProjY;
        v[i].pos.z = 1.0 - 1.0 / (sw[i].vWorldViewPosition.x * 0.061758894);
        v[i].rhw = 1.0 / sw[i].vWorldViewPosition.x;
        v[i].diffuse = 0x80F020F0;
        v[i].specular = 0;
        // v[i].texcoord.x = pFace->pVertexUIDs[i] /
        // (double)pTex->uTextureWidth; v[i].texcoord.y = pFace->pVertexUIDs[i] /
        // (double)pTex->uTextureHeight;
        v[i].texcoord.x = 0;
        v[i].texcoord.y = 0;
    }

    render->DrawFansTransparent(v, pFace->uNumVertices);
}

//----- (00437906) --------------------------------------------------------
void Camera3D::PrepareAndDrawDebugOutline(BLVFace *pFace,
                                                 unsigned int uDiffuse) {
    static RenderVertexSoft static_sub_437906_array_50CDD0[64];
    static bool __init_flag1 = false;
    if (!__init_flag1) {
        __init_flag1 = true;

        for (uint i = 0; i < 64; ++i)
            static_sub_437906_array_50CDD0[i].flt_2C = 0.0f;
    }
    if (pFace->uNumVertices) {
        for (uint i = 0; i < pFace->uNumVertices; i++) {
            static_sub_437906_array_50CDD0[i].vWorldPosition.x =
                (double)pIndoor->pVertices[pFace->pVertexIDs[i]].x;
            static_sub_437906_array_50CDD0[i].vWorldPosition.y =
                (double)pIndoor->pVertices[pFace->pVertexIDs[i]].y;
            static_sub_437906_array_50CDD0[i].vWorldPosition.z =
                (double)pIndoor->pVertices[pFace->pVertexIDs[i]].z;
            static_sub_437906_array_50CDD0[i].u = (double)pFace->pVertexUIDs[i];
            static_sub_437906_array_50CDD0[i].v = (double)pFace->pVertexVIDs[i];
        }
    }
    if (engine->config->debug_portal_outlines)
        debug_outline_sw(static_sub_437906_array_50CDD0, pFace->uNumVertices,
                         uDiffuse, 0.0);
}
// 50D9D0: using guessed type char static_sub_437906_byte_50D9D0_init_flag;

//----- (004378BA) --------------------------------------------------------
void Camera3D::MatrixMultiply(Matrix3x3_float_ *ma, Matrix3x3_float_ *mb,
                                     Matrix3x3_float_ *m_out) {
    float sum;

    for (uint row = 0; row < 3; row++) {
        for (uint col = 0; col < 3; col++) {
            sum = 0;
            for (int index = 0; index < 3; index++)
                sum += ma->v[row][index] * mb->v[index][col];
            m_out->v[row][col] = sum;
        }
    }
}

//----- (004376E7) --------------------------------------------------------
void Camera3D::CreateWorldMatrixAndSomeStuff() {
    Matrix3x3_float_ m1;  // [sp+10h] [bp-B8h]@1
    Matrix3x3_float_ m2;  // [sp+34h] [bp-94h]@1
    Matrix3x3_float_ m3;  // [sp+58h] [bp-70h]@1
    Matrix3x3_float_ m4;  // [sp+7Ch] [bp-4Ch]@1
    Matrix3x3_float_ m5;  // [sp+A0h] [bp-28h]@1

    // axis on view transform not typical - matrices joggled round to reflect this
    // x -> z, y -> x, z -> y 

    // if axis typical
    // glm::mat3x3 roll(cosf(0), sinf(0), 0, -sinf(0), cosf(0), 0, 0, 0, 1);
    // axis womm - no roll - identity
    glm::mat3x3 roll = glm::mat3(1);

    // RotationZ(0) - roll
    m5._11 = cosf(0);
    m5._12 = sinf(0);
    m5._13 = 0;
    m5._21 = -sinf(0);
    m5._22 = cosf(0);
    m5._23 = 0;
    m5._31 = 0;
    m5._32 = 0;
    m5._33 = 1;

    

    float cos_x1 = fRotationXCosine, sin_x1 = fRotationXSine;
    float cos_y1 = fRotationZCosine, sin_y1 = fRotationZSine;

    // if axis typical
    // glm::mat3x3 pitch(1, 0, 0, 0 , cos_x1, sin_x1, 0, -sin_x1, cos_x1);
    // axis WOMM
    glm::mat3x3 pitch(0, -sin_x1, cos_x1, 1, 0, 0, 0, cos_x1, sin_x1);

    // RotationX(x) - pitch
    m4._11 = 1;
    m4._12 = 0;
    m4._13 = 0;
    m4._21 = 0;
    m4._22 = cos_x1;
    m4._23 = sin_x1;
    m4._31 = 0;
    m4._32 = -sin_x1;
    m4._33 = cos_x1;

    


    // if axis typical
    // glm::mat3x3 yaw(cos_y1, 0, -sin_y1, 0, 1, 0, sin_y1, 0, cos_y1);
    // axis womm
    glm::mat3x3 yaw(-sin_y1, cos_y1, 0, 0, 0, 1, cos_y1, sin_y1, 0);

    // RotationY(some_angle) - yaw
    m3._11 = cos_y1;
    m3._12 = 0;
    m3._13 = -sin_y1;
    m3._21 = 0;
    m3._22 = 1;
    m3._23 = 0;
    m3._31 = sin_y1;
    m3._32 = 0;
    m3._33 = cos_y1;

    MatrixMultiply(&m5, &m3, &m1);
    MatrixMultiply(&m4, &m1, &m2);

    camrotation = yaw * roll * pitch;
    //camrotation[0] = finmat[2];
    //amrotation[1] = finmat[0];
    //camrotation[2] = finmat[1];

    // this flips the axis around to keep wierd transform
    for (uint i = 0; i < 3; ++i) {
        m3x3_cam_rotation[0].v[i] = m2.v[1][i];
        m3x3_cam_rotation[1].v[i] = m2.v[0][i];
        m3x3_cam_rotation[2].v[i] = m2.v[2][i];

        //camrotation[0][i] = m2.v[1][i];
        //camrotation[1][i] = m2.v[0][i];
        //camrotation[2][i] = m2.v[2][i];
    }

    // fov projection calcs
    fov = 0.5 / tan(odm_fov_rad / 2.0);
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        fov = 0.5 / tan(blv_fov_rad / 2.0);

    inv_fov = 1.0 / fov;

    ViewPlaneDist_X = (double)pViewport->uScreenWidth * fov;
    ViewPlaneDist_Y = (double)pViewport->uScreenHeight * fov;

    screenCenterX = (double)pViewport->uScreenCenterX;
    screenCenterY = (double)pViewport->uScreenCenterY - pViewport->uScreen_TL_Y;  //- pViewport->uScreen_TL_Y);
}

//----- (00437691) --------------------------------------------------------
void Camera3D::Vec3Transform(const glm::vec3 *pVector, glm::vec3 *pOut) {
    pOut->y = m3x3_cam_rotation[1].x * pVector->x + m3x3_cam_rotation[0].x * pVector->y +
        m3x3_cam_rotation[2].x * pVector->z;
    pOut->z = m3x3_cam_rotation[1].y * pVector->x + m3x3_cam_rotation[0].y * pVector->y +
        m3x3_cam_rotation[2].y * pVector->z;
    pOut->x = m3x3_cam_rotation[1].z * pVector->x + m3x3_cam_rotation[0].z * pVector->y +
        m3x3_cam_rotation[2].z * pVector->z;


    // need normals in correct XYZ so swizzle from odd cam view matrix
    pOut[0] = *pVector * camrotation;
    
    int t = 7;
}

//----- (00437607) --------------------------------------------------------
void Camera3D::BuildFrustumPlane(glm::vec3 *a1, glm::vec4 *a2) {
    glm::vec3 a23(*a2);
    Vec3Transform(a1, &a23);
    a2->x = a23.x;
    a2->y = a23.y;
    a2->z = a23.z;
    a2->w = glm::dot(vCameraPos, a23) + 0.000099999997;
}

//----- (004374E8) --------------------------------------------------------
void Camera3D::BuildViewFrustum() {
    float HalfAngleX = odm_fov_rad / 2.0;
    float HalfAngleY = atan((game_viewport_height / 2.0) / pCamera3D->ViewPlaneDist_X);
    
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        HalfAngleX = blv_fov_rad / 2.0;
    }

    glm::vec3 PlaneVec;

    PlaneVec.x = -sin(HalfAngleX);
    PlaneVec.y = 0.0;
    PlaneVec.z = cos(HalfAngleX);
    BuildFrustumPlane(&PlaneVec, &FrustumPlanes[0]);

    PlaneVec.x = sin(HalfAngleX);
    BuildFrustumPlane(&PlaneVec, &FrustumPlanes[1]);

    PlaneVec.y = sin(HalfAngleY);
    PlaneVec.x = 0.0;
    PlaneVec.z = cos(HalfAngleY);
    BuildFrustumPlane(&PlaneVec, &FrustumPlanes[2]);

    PlaneVec.y = -sin(HalfAngleY);
    BuildFrustumPlane(&PlaneVec, &FrustumPlanes[3]);
}

//----- (00437376) --------------------------------------------------------
// culls vertices to face plane
char Camera3D::CullVertsToPlane(stru154 *faceplane, RenderVertexSoft *vertices,
                              unsigned int *pOutNumVertices) {
    double v6;             // st7@3
    int previous;          // esi@6
    int current;           // ebx@8
    int next;              // eax@8
    int v13;               // eax@15
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

        if (-0.009999999776482582 <
            ((vertices[current].vWorldViewProjX - vertices[previous].vWorldViewProjX) *
                 (vertices[next].vWorldViewProjY - vertices[previous].vWorldViewProjY) -
             (vertices[current].vWorldViewProjY - vertices[previous].vWorldViewProjY) *
                 (vertices[next].vWorldViewProjX - vertices[previous].vWorldViewProjX)) *
                v6) {
            v13 = next;
            if (next >= (signed int)*pOutNumVertices)
                v13 = next - *pOutNumVertices;

            if (v13 < (signed int)*pOutNumVertices) {
                for (v14 = v13; v14 < (signed int)*pOutNumVertices; ++v14)
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

//----- (00437285) --------------------------------------------------------
bool Camera3D::CullFaceToFrustum(RenderVertexSoft *pInVertices,
                                      unsigned int *pOutNumVertices,
                                      RenderVertexSoft *pVertices,
                                      signed int NumFrustumPlanes, char DebugLines,
                                      int _unused) {
    // NumFrustumPlanes usually 4 - top, bottom, left, right - near and far done elsewhere
    // DebugLines 0 or 1 - 1 when debug lines

    RenderVertexSoft *v14;  // eax@8
    RenderVertexSoft *v15;  // edx@8
    Vec3_float_ FrustumPlaneVec;         // [sp+18h] [bp-3Ch]@12
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
        } else {
            v15 = pInVertices;
            v14 = sr_vertices_50D9D8;
        }
        if (i == NumFrustumPlanes - 1) v14 = pVertices;
        FrustumPlaneVec.x = FrustumPlanes[i].x;
        FrustumPlaneVec.y = FrustumPlanes[i].y;
        FrustumPlaneVec.z = FrustumPlanes[i].z;

        engine->pStru9Instance->ClipVertsToFrustumPlane(
            v15, *pOutNumVertices, v14, pOutNumVertices, &FrustumPlaneVec, FrustumPlanes[i].w,
            (char *)&VertsAdjusted, _unused);

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

//----- (00437143) --------------------------------------------------------
void Camera3D::LightmapProject(unsigned int uNumInVertices,  // lightmap project
                              RenderVertexSoft *pOutVertices,
                              RenderVertexSoft *pInVertices,
                              signed int *pOutNumVertices) {
    double v9;  // st7@3

    uint i = 0;

    for (; i < uNumInVertices; ++i) {
        pInVertices[i]._rhw =
            1.0 / (pInVertices[i].vWorldViewPosition.x + 0.0000001);
        memcpy(&pOutVertices[i], &pInVertices[i], sizeof(pOutVertices[i]));
        v9 = (double)pCamera3D->ViewPlaneDist_X * pInVertices[i]._rhw;
        pOutVertices[i].vWorldViewProjX =
            (double)pViewport->uScreenCenterX -
            v9 * pInVertices[i].vWorldViewPosition.y;
        pOutVertices[i].vWorldViewProjY =
            (double)pViewport->uScreenCenterY -
            v9 * pInVertices[i].vWorldViewPosition.z;
    }
    *pOutNumVertices = i;
    return;
}

//----- (00436F09) --------------------------------------------------------
void Camera3D::LightmapFarClip(RenderVertexSoft *pInVertices,
                                      int uNumInVertices,
                                      RenderVertexSoft *pOutVertices,
                                      unsigned int *pOutNumVertices) {
    bool current_vertices_flag;
    bool next_vertices_flag;
    double t;
    signed int depth_num_vertices;

    memcpy(&pInVertices[uNumInVertices], &pInVertices[0],
           sizeof(pInVertices[uNumInVertices]));
    depth_num_vertices = 0;
    current_vertices_flag = false;
    if (pInVertices[0].vWorldViewPosition.x >= pCamera3D->GetFarClip())
        current_vertices_flag =
            true;  //настоящая вершина больше границы видимости
    if ((signed int)uNumInVertices <= 0) return;
    // check for far clip plane(проверка по дальней границе)
    //
    // v3.__________________. v0
    //   |                  |
    //   |                  |
    //   |                  |
    //  ----------------------- 8192.0(far_clip - 0x2000)
    //   |                  |
    //   .__________________.
    //  v2                     v1

    for (uint i = 0; i < uNumInVertices; ++i) {
        next_vertices_flag = pInVertices[i + 1].vWorldViewPosition.x >=
                             pCamera3D->GetFarClip();
        if (current_vertices_flag ^
            next_vertices_flag) {  // одна из граней за границей видимости
            if (next_vertices_flag) {  // следующая вершина больше границы
                                       // видимости(настоящая вершина меньше
                                       // границы видимости) - v3
                // t = far_clip - v2.x / v3.x - v2.x (формула получения точки
                // пересечения отрезка с плоскостью)
                t = (pCamera3D->GetFarClip() -
                     pInVertices[i].vWorldViewPosition.x) /
                    (pInVertices[i].vWorldViewPosition.x -
                     pInVertices[i + 1].vWorldViewPosition.x);
                pOutVertices[depth_num_vertices].vWorldViewPosition.x =
                    pCamera3D->GetFarClip();
                // New_y = v2.y + (v3.y - v2.y)*t
                pOutVertices[depth_num_vertices].vWorldViewPosition.y =
                    pInVertices[i].vWorldViewPosition.y +
                    (pInVertices[i].vWorldViewPosition.y -
                     pInVertices[i + 1].vWorldViewPosition.y) *
                        t;
                // New_z = v2.z + (v3.z - v2.z)*t
                pOutVertices[depth_num_vertices].vWorldViewPosition.z =
                    pInVertices[i].vWorldViewPosition.z +
                    (pInVertices[i].vWorldViewPosition.z -
                     pInVertices[i + 1].vWorldViewPosition.z) *
                        t;
                pOutVertices[depth_num_vertices].u =
                    pInVertices[i].u +
                    (pInVertices[i].u - pInVertices[i + 1].u) * t;
                pOutVertices[depth_num_vertices].v =
                    pInVertices[i].v +
                    (pInVertices[i].v - pInVertices[i + 1].v) * t;
                pOutVertices[depth_num_vertices]._rhw =
                    1.0 / pCamera3D->GetFarClip();
            } else {  // настоящая вершина больше границы видимости(следующая
                      // вершина меньше границы видимости) - v0
                // t = far_clip - v1.x / v0.x - v1.x
                t = (pCamera3D->GetFarClip() -
                     pInVertices[i].vWorldViewPosition.x) /
                    (pInVertices[i + 1].vWorldViewPosition.x -
                     pInVertices[i].vWorldViewPosition.x);
                pOutVertices[depth_num_vertices].vWorldViewPosition.x =
                    pCamera3D->GetFarClip();
                // New_y = (v0.y - v1.y)*t + v1.y
                pOutVertices[depth_num_vertices].vWorldViewPosition.y =
                    pInVertices[i].vWorldViewPosition.y +
                    (pInVertices[i + 1].vWorldViewPosition.y -
                     pInVertices[i].vWorldViewPosition.y) *
                        t;
                // New_z = (v0.z - v1.z)*t + v1.z
                pOutVertices[depth_num_vertices].vWorldViewPosition.z =
                    pInVertices[i].vWorldViewPosition.z +
                    (pInVertices[i + 1].vWorldViewPosition.z -
                     pInVertices[i].vWorldViewPosition.z) *
                        t;
                pOutVertices[depth_num_vertices].u =
                    pInVertices[i].u +
                    (pInVertices[i + 1].u - pInVertices[i].u) * t;
                pOutVertices[depth_num_vertices].v =
                    pInVertices[i].v +
                    (pInVertices[i + 1].v - pInVertices[i].v) * t;
                pOutVertices[depth_num_vertices]._rhw =
                    1.0 / pCamera3D->GetFarClip();
            }
            ++depth_num_vertices;
        }
        if (!next_vertices_flag) {  // оба в границе видимости
            memcpy(&pOutVertices[depth_num_vertices], &pInVertices[i + 1],
                   sizeof(pOutVertices[depth_num_vertices]));
            depth_num_vertices++;
        }
        current_vertices_flag = next_vertices_flag;
    }
    if (depth_num_vertices >= 3)
        *pOutNumVertices = depth_num_vertices;
    else
        *pOutNumVertices = 0;
}

//----- (00436CDC) --------------------------------------------------------
void Camera3D::LightmapNeerClip(RenderVertexSoft *pInVertices,
                                       int uNumInVertices,
                                       RenderVertexSoft *pOutVertices,
                                       unsigned int *pOutNumVertices) {
    float nearclip = pCamera3D->GetNearClip();

    double t;                    // st6@11
    bool current_vertices_flag;  // esi@2
    bool next_vertices_flag;     // [sp+Ch] [bp+8h]@7

    // check for near clip plane(проверка по ближней границе)
    //
    // v3.__________________. v0
    //   |                  |
    //   |                  |
    //   |                  |
    //  ----------------------- 8.0(near_clip - 8.0)
    //   |                  |
    //   .__________________.
    //  v2                     v1

    int out_num_vertices = 0;

    if (uNumInVertices) {
        memcpy(&pInVertices[uNumInVertices], &pInVertices[0],
               sizeof(pInVertices[0]));
        next_vertices_flag = false;
        current_vertices_flag = false;
        if (pInVertices[0].vWorldViewPosition.x <= nearclip)
            current_vertices_flag = true;
        for (uint i = 0; i < uNumInVertices; ++i) {
            next_vertices_flag =
                pInVertices[i + 1].vWorldViewPosition.x <= nearclip;  //
            if (current_vertices_flag ^ next_vertices_flag) {  // XOR
                if (next_vertices_flag) {  // следующая вершина за ближней границей
                    // t = near_clip - v0.x / v1.x - v0.x    (формула получения
                    // точки пересечения отрезка с плоскостью)
                    t = (nearclip - pInVertices[i].vWorldViewPosition.x) /
                        (pInVertices[i + 1].vWorldViewPosition.x -
                         pInVertices[i].vWorldViewPosition.x);
                    pOutVertices[out_num_vertices].vWorldViewPosition.x = nearclip;
                    pOutVertices[out_num_vertices].vWorldViewPosition.y =
                        pInVertices[i].vWorldViewPosition.y +
                        (pInVertices[i + 1].vWorldViewPosition.y -
                         pInVertices[i].vWorldViewPosition.y) *
                            t;
                    pOutVertices[out_num_vertices].vWorldViewPosition.z =
                        pInVertices[i].vWorldViewPosition.z +
                        (pInVertices[i + 1].vWorldViewPosition.z -
                         pInVertices[i].vWorldViewPosition.z) *
                            t;
                    pOutVertices[out_num_vertices].u =
                        pInVertices[i].u +
                        (pInVertices[i + 1].u - pInVertices[i].u) * t;
                    pOutVertices[out_num_vertices].v =
                        pInVertices[i].v +
                        (pInVertices[i + 1].v - pInVertices[i].v) * t;
                    pOutVertices[out_num_vertices]._rhw = 1.0 / 8.0;
                    // pOutVertices[*pOutNumVertices]._rhw = 0.125;
                } else {  // текущая вершина за ближней границей
                    t = (nearclip - pInVertices[i].vWorldViewPosition.x) /
                        (pInVertices[i].vWorldViewPosition.x -
                         pInVertices[i + 1].vWorldViewPosition.x);
                    pOutVertices[out_num_vertices].vWorldViewPosition.x = nearclip;
                    pOutVertices[out_num_vertices].vWorldViewPosition.y =
                        pInVertices[i].vWorldViewPosition.y +
                        (pInVertices[i].vWorldViewPosition.y -
                         pInVertices[i + 1].vWorldViewPosition.y) *
                            t;
                    pOutVertices[out_num_vertices].vWorldViewPosition.z =
                        pInVertices[i].vWorldViewPosition.z +
                        (pInVertices[i].vWorldViewPosition.z -
                         pInVertices[i + 1].vWorldViewPosition.z) *
                            t;
                    pOutVertices[out_num_vertices].u =
                        pInVertices[i].u +
                        (pInVertices[i].u - pInVertices[i + 1].u) * t;
                    pOutVertices[out_num_vertices].v =
                        pInVertices[i].v +
                        (pInVertices[i].v - pInVertices[i + 1].v) * t;
                    pOutVertices[out_num_vertices]._rhw = 1.0 / nearclip;
                }
                ++out_num_vertices;
            }
            if (!next_vertices_flag) {
                memcpy(&pOutVertices[out_num_vertices], &pInVertices[i + 1],
                       sizeof(pInVertices[i + 1]));
                // pOutVertices[out_num_vertices]._rhw = 1.0 /
                // (pInVertices[i].vWorldViewPosition.x + 0.0000001);
                out_num_vertices++;
            }
            current_vertices_flag = next_vertices_flag;
        }
        if (out_num_vertices >= 3)
            *pOutNumVertices = out_num_vertices;
        else
            *pOutNumVertices = 0;
    }
}

//----- (00436BB7) --------------------------------------------------------
void Camera3D::Project(RenderVertexSoft *pVertices,
                              unsigned int uNumVertices,
                              bool fit_into_viewport) {
    // double v7;   // st7@7
    double v8;   // st7@9
    double v9;   // st6@10
    double v10;  // st5@12
    double v11;  // st7@16
    double v12;  // st6@17
    double v13;  // st5@19
    double v1;
    double v2;

    for (uint i = 0; i < uNumVertices; ++i) {

            auto v = pVertices + i;


            v1 = 1.0 / (v->vWorldViewPosition.x + 0.0000001);
            v->_rhw = v1;
            v2 = v1 * ViewPlaneDist_X;

            v->vWorldViewProjX = (double)pViewport->uScreenCenterX -
                                 v2 * (double)v->vWorldViewPosition.y;
            v->vWorldViewProjY = (double)pViewport->uScreenCenterY -
                                 v2 * (double)v->vWorldViewPosition.z;

        if (fit_into_viewport) {
            // __debugbreak();

            v8 = (double)(signed int)pViewport->uViewportBR_X;
            if (v8 >= pVertices[i].vWorldViewProjX)
                v9 = pVertices[i].vWorldViewProjX;
            else
                v9 = v8;
            v10 = (double)(signed int)pViewport->uViewportTL_X;
            if (v10 <= v9) {
                if (v8 >= pVertices[i].vWorldViewProjX)
                    v8 = pVertices[i].vWorldViewProjX;
            } else {
                v8 = v10;
            }
            pVertices[i].vWorldViewProjX = v8;
            v11 = (double)(signed int)pViewport->uViewportBR_Y;
            if (v11 >= pVertices[i].vWorldViewProjY)
                v12 = pVertices[i].vWorldViewProjY;
            else
                v12 = v11;
            v13 = (double)(signed int)pViewport->uViewportTL_Y;
            if (v13 <= v12) {
                if (v11 >= pVertices[i].vWorldViewProjY)
                    v11 = pVertices[i].vWorldViewProjY;
            } else {
                v11 = v13;
            }
            pVertices[i].vWorldViewProjY = v11;
        }
    }
}

void Camera3D::Project(int x, int y, int z, int *screenspace_x,
                              int *screenspace_y) {
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
float Camera3D::GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int uStripType) {
    float result; = FLT_MIN;
    for (uint i = 0; i < uStripType; i++) {
        if (pVertex[i].vWorldPosition.z > result)
            result = pVertex[i].vWorldPosition.z;
    }
    return result;
}

// -- new
// merged from IndoorCamera::Initialize2
//         and ODMRenderParams::RotationToInts
//         and BLVRenderParams::Reset
void Camera3D::CalculateRotations(int camera_rot_x, int camera_rot_z) {
    sRotationX = camera_rot_x + 20;  // pitch
    sRotationZ = camera_rot_z;  // yaw

    fRotationZSine = sin((pi_double + pi_double) * (double)sRotationZ / 2048.0);
    fRotationZCosine = cos((pi_double + pi_double) * (double)sRotationZ / 2048.0);

    fRotationXSine = sin((pi_double + pi_double) * (double)sRotationX / 2048.0);
    fRotationXCosine = cos((pi_double + pi_double) * (double)sRotationX / 2048.0);

    int_sine_Z = TrigLUT->Sin(sRotationZ);
    int_cosine_Z = TrigLUT->Cos(sRotationZ);
    int_sine_x = TrigLUT->Sin(sRotationX);
    int_cosine_x = TrigLUT->Cos(sRotationX);
}
