#include "Engine/Graphics/PortalFunctions.h"

#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Indoor.h"

int _49CE9E_sub0_x(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                   float test_val) {
    float max_val = FLT_MAX;
    int idx = -1;

    float temp_val;
    for (uint i = 0; i < uNumVertices; ++i) {
        if (pVertices[i].vWorldPosition.x <= test_val)
            temp_val = test_val - pVertices[i].vWorldPosition.x;
        else
            temp_val = pVertices[i].vWorldPosition.x - test_val;

        if (temp_val < max_val) {
            max_val = temp_val;
            idx = i;
        }
    }
    return idx;
}

int _49CE9E_sub0_y(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                   float test_val) {
    float max_val = FLT_MAX;
    int idx = -1;

    float temp_val;
    for (uint i = 0; i < uNumVertices; ++i) {
        if (pVertices[i].vWorldPosition.y <= test_val)
            temp_val = test_val - pVertices[i].vWorldPosition.y;
        else
            temp_val = pVertices[i].vWorldPosition.y - test_val;

        if (temp_val < max_val) {
            max_val = temp_val;
            idx = i;
        }
    }
    return idx;
}

int _49CE9E_sub0_z(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                   float test_val) {
    float max_val = FLT_MAX;
    int idx = -1;

    float temp_val;
    for (uint i = 0; i < uNumVertices; ++i) {
        if (pVertices[i].vWorldPosition.z <= test_val)
            temp_val = test_val - pVertices[i].vWorldPosition.z;
        else
            temp_val = pVertices[i].vWorldPosition.z - test_val;

        if (temp_val < max_val) {
            max_val = temp_val;
            idx = i;
        }
    }
    return idx;
}

//----- (0049CE9E) --------------------------------------------------------
void stru10::_49CE9E(BLVFace *pFace, RenderVertexSoft *pVertices,
                     unsigned int uNumVertices, RenderVertexSoft *pOutLimits) {
    Assert(sizeof(RenderVertexSoft) == 0x30);

    RenderVertexSoft pLimits[64];
    stru10::CalcPolygonLimits(pFace, pLimits);

    if (pFace->uAttributes & FACE_XY_PLANE) {
        memcpy(&pOutLimits[0],
               &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices,
                                         pLimits[0].vWorldPosition.x)],
               0x30);
        memcpy(&pOutLimits[2],
               &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices,
                                         pLimits[2].vWorldPosition.x)],
               0x30);
        memcpy(&pOutLimits[1],
               &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices,
                                         pLimits[1].vWorldPosition.y)],
               0x30);
        memcpy(&pOutLimits[3],
               &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices,
                                         pLimits[3].vWorldPosition.y)],
               0x30);
    } else if (pFace->uAttributes & FACE_XZ_PLANE) {
        memcpy(&pOutLimits[0],
               &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices,
                                         pLimits[0].vWorldPosition.x)],
               0x30);
        memcpy(&pOutLimits[2],
               &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices,
                                         pLimits[2].vWorldPosition.x)],
               0x30);
        memcpy(&pOutLimits[1],
               &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices,
                                         pLimits[1].vWorldPosition.z)],
               0x30);
        memcpy(&pOutLimits[3],
               &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices,
                                         pLimits[3].vWorldPosition.z)],
               0x30);
    } else if (pFace->uAttributes & FACE_YZ_PLANE) {
        memcpy(&pOutLimits[0],
               &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices,
                                         pLimits[0].vWorldPosition.y)],
               0x30);
        memcpy(&pOutLimits[2],
               &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices,
                                         pLimits[2].vWorldPosition.y)],
               0x30);
        memcpy(&pOutLimits[1],
               &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices,
                                         pLimits[1].vWorldPosition.z)],
               0x30);
        memcpy(&pOutLimits[3],
               &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices,
                                         pLimits[3].vWorldPosition.z)],
               0x30);
    }
}

//----- (0049D379) --------------------------------------------------------
void stru10::CalcPolygonLimits(BLVFace *pFace, RenderVertexSoft *pOutVertices) {
    struct {
        float x;
        float y;
        int c;
    } v46[40]{};  // [sp+0C];

    if (pFace->uAttributes & FACE_XY_PLANE) {
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].x +
                       pFace->pXInterceptDisplacements[i];
            v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].y +
                       pFace->pYInterceptDisplacements[i];
            v46[i].c = i;
        }
    }
    if (pFace->uAttributes & FACE_XZ_PLANE) {
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].x +
                       pFace->pXInterceptDisplacements[i];
            v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].z +
                       pFace->pZInterceptDisplacements[i];
            v46[i].c = i;
        }
    }
    if (pFace->uAttributes & FACE_YZ_PLANE) {
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].y +
                       pFace->pYInterceptDisplacements[i];
            v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].z +
                       pFace->pZInterceptDisplacements[i];
            v46[i].c = i;
        }
    }

    float x_min = v46[0].x;
    uint x_min_idx = 0;

    float x_max = v46[0].x;
    uint x_max_idx = 0;

    float y_min = v46[0].y;
    uint y_min_idx = 0;

    float y_max = v46[0].y;
    uint y_max_idx = 0;

    for (uint i = 0; i < pFace->uNumVertices; ++i) {
        if (v46[i].x < x_min) {
            x_min = v46[i].x;
            x_min_idx = v46[i].c;
        }
        if (v46[i].x > x_max) {
            x_max = v46[i].x;
            x_max_idx = v46[i].c;
        }

        if (v46[i].y < y_min) {
            y_min = v46[i].y;
            y_min_idx = v46[i].c;
        }
        if (v46[i].y > y_max) {
            y_max = v46[i].y;
            y_max_idx = v46[i].c;
        }
    }

    RenderVertexSoft v1;  // [sp+30Ch] [bp-54h]@24
    v1.vWorldPosition.x =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].x;
    v1.vWorldPosition.y =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].y;
    v1.vWorldPosition.z =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].z;
    memcpy(&pOutVertices[0], &v1, sizeof(RenderVertexSoft));

    RenderVertexSoft v2;  // [sp+30Ch] [bp-54h]@24
    v2.vWorldPosition.x =
        (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].x;
    v2.vWorldPosition.y =
        (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].y;
    v2.vWorldPosition.z =
        (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].z;
    memcpy(&pOutVertices[1], &v2, sizeof(RenderVertexSoft));

    RenderVertexSoft v3;  // [sp+30Ch] [bp-54h]@24
    v3.vWorldPosition.x =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].x;
    v3.vWorldPosition.y =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].y;
    v3.vWorldPosition.z =
        (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].z;
    memcpy(&pOutVertices[2], &v3, sizeof(RenderVertexSoft));

    RenderVertexSoft v4;  // [sp+30Ch] [bp-54h]@24
    v4.vWorldPosition.x =
        (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].x;
    v4.vWorldPosition.y =
        (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].y;
    v4.vWorldPosition.z =
        (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].z;
    memcpy(&pOutVertices[3], &v4, sizeof(RenderVertexSoft));
}

//----- (0049C9E3) --------------------------------------------------------
bool stru10::CalcFaceBounding(BLVFace *pFace, RenderVertexSoft *pFaceLimits,
                              unsigned int uNumVertices,
                              RenderVertexSoft *pOutBounding) {
    Vec3_float_ a1;
    a1.x = 0.0f;
    a1.y = 0.0f;
    a1.z = 0.0f;

    float var_28;
    float var_24;
    switch (pFace->uPolygonType) {
        case POLYGON_VerticalWall:
            a1.x = -pFace->pFacePlane.vNormal.y;  // направление полигона
            a1.y = pFace->pFacePlane.vNormal.x;
            a1.z = 0.0f;
            a1.Normalize();

            var_28 = 0;
            var_24 = 1;
            break;

        case POLYGON_Floor:
        case POLYGON_Ceiling:
            a1.x = 1;
            a1.y = 0;
            a1.z = 0.0f;

            var_28 = 1;
            var_24 = 0;
            break;

        default:
            Error("Invalid polygon type (%u)", pFace->uPolygonType);
    }

    float face_center_x = 0;
    float face_center_y = 0;
    float face_center_z = 0;
    float a3 = 0;
    float var_8 = 0;

    if (pFace->uAttributes & FACE_XY_PLANE) {
        face_center_x = (pFaceLimits[0].vWorldPosition.x +
                         pFaceLimits[2].vWorldPosition.x) /
                        2;
        face_center_y = (pFaceLimits[3].vWorldPosition.y +
                         pFaceLimits[1].vWorldPosition.y) /
                        2;
        face_center_z = (pFaceLimits[0].vWorldPosition.z +
                         pFaceLimits[2].vWorldPosition.z) /
                        2;

        a3 = face_center_x - pFaceLimits[0].vWorldPosition.x;
        var_8 = face_center_y - pFaceLimits[1].vWorldPosition.y;
    }
    if (pFace->uAttributes & FACE_XZ_PLANE) {
        face_center_x = (pFaceLimits[0].vWorldPosition.x +
                         pFaceLimits[2].vWorldPosition.x) /
                        2;  // центр полигона
        face_center_y = (pFaceLimits[0].vWorldPosition.y +
                         pFaceLimits[2].vWorldPosition.y) /
                        2;
        face_center_z = (pFaceLimits[1].vWorldPosition.z +
                         pFaceLimits[3].vWorldPosition.z) /
                        2;

        a3 = face_center_x -
             pFaceLimits[0].vWorldPosition.x;  //от центра до верхнего края
        var_8 = face_center_z -
                pFaceLimits[1].vWorldPosition.z;  // высота от центра

        if (pFace->uPolygonType == POLYGON_VerticalWall) a3 /= a1.x;
    }
    if (pFace->uAttributes & FACE_YZ_PLANE) {
        face_center_x = (pFaceLimits[0].vWorldPosition.x +
                         pFaceLimits[2].vWorldPosition.x) /
                        2;
        face_center_y = (pFaceLimits[0].vWorldPosition.y +
                         pFaceLimits[2].vWorldPosition.y) /
                        2;
        face_center_z = (pFaceLimits[1].vWorldPosition.z +
                         pFaceLimits[3].vWorldPosition.z) /
                        2;

        a3 = face_center_y - pFaceLimits[0].vWorldPosition.y;
        var_8 = face_center_z - pFaceLimits[1].vWorldPosition.z;
        // [0.5]
        if (pFace->uPolygonType == POLYGON_VerticalWall) {
            if (a1.x != 1.0f) a3 /= a1.y;
        }
    }

    // _EBX = arg0;
    // v15 = v31;
    // v16 = arg0;
    // float var_20 = var_8 * var_24;
    // var_8 = a3 * a1.z;
    // float arg_0 = var_8 + var_4;



    pOutBounding[0].vWorldPosition.x =
        face_center_x - a3 * a1.x + var_8 * flt_4D84E8;
    pOutBounding[0].vWorldPosition.y =
        face_center_y - a3 * a1.y + var_8 * var_28;
    pOutBounding[0].vWorldPosition.z =
        face_center_z - a3 * a1.z + var_8 * var_24;

    pOutBounding[1].vWorldPosition.x =
        face_center_x - a3 * a1.x - var_8 * flt_4D84E8;
    pOutBounding[1].vWorldPosition.y =
        face_center_y - a3 * a1.y - var_8 * var_28;
    pOutBounding[1].vWorldPosition.z =
        face_center_z - a3 * a1.z - var_8 * var_24;

    pOutBounding[2].vWorldPosition.x =
        face_center_x + a3 * a1.x - var_8 * flt_4D84E8;
    pOutBounding[2].vWorldPosition.y =
        face_center_y + a3 * a1.y - var_8 * var_28;
    pOutBounding[2].vWorldPosition.z =
        face_center_z + a3 * a1.z - var_8 * var_24;

    pOutBounding[3].vWorldPosition.x =
        face_center_x + a3 * a1.x + var_8 * flt_4D84E8;
    pOutBounding[3].vWorldPosition.y =
        face_center_y + a3 * a1.y + var_8 * var_28;
    pOutBounding[3].vWorldPosition.z =
        face_center_z + a3 * a1.z + var_8 * var_24;

    a1.x = 0.0f;
    a1.y = 0.0f;
    a1.z = 0.0f;
    a3 = face_center_x + a3 * a1.x;

    // if (!FindFacePlaneNormalAndDot(pOutBounding, &a1, &a3)) return false;

    // testing
    a1.x = pFace->pFacePlane.vNormal.x;
    a1.y = pFace->pFacePlane.vNormal.y;
    a1.z = pFace->pFacePlane.vNormal.z;


    RenderVertexSoft v25;  // [sp+10h] [bp-90h]@20
    memcpy(&v25, pOutBounding, sizeof(RenderVertexSoft));

    float _dp = (v25.vWorldPosition.x - pCamera3D->vCameraPos.x) * a1.x +
                (v25.vWorldPosition.y - pCamera3D->vCameraPos.y) * a1.y +
                (v25.vWorldPosition.z - pCamera3D->vCameraPos.z) * a1.z;
    if (fabs(_dp) < 1e-6f) {
        logger->Info("Epsilon check");
        memcpy(&v25, &pOutBounding[1], sizeof(RenderVertexSoft));
        memcpy(&pOutBounding[1], &pOutBounding[3], sizeof(RenderVertexSoft));
        memcpy(&pOutBounding[3], &v25, sizeof(RenderVertexSoft));
    }


    RenderVertexSoft v26;

    // show portal outlines debug
    if (engine->config->debug_portal_outlines) {
        // portal frustums
        v26.vWorldPosition.x = pCamera3D->vCameraPos.x;
        v26.vWorldPosition.y = pCamera3D->vCameraPos.y;
        v26.vWorldPosition.z = pCamera3D->vCameraPos.z;

        /*pCamera3D->do_draw_debug_line_sw(&v26, 0xFF0000u, &pOutBounding[0], 0xFF0000u, 0, 0);
        pCamera3D->do_draw_debug_line_sw(&v26, 0xFF00u, &pOutBounding[1], 0xFF00u, 0, 0);
        pCamera3D->do_draw_debug_line_sw(&v26, 0xFFu, &pOutBounding[2], 0xFFu, 0, 0);
        pCamera3D->do_draw_debug_line_sw(&v26, 0xFFFFFFu, &pOutBounding[3], 0xFFFFFFu, 0, 0);*/

        // portal bounding
        pCamera3D->debug_outline_sw(pOutBounding, uNumVertices, 0x1EFF1Eu, 0.00019999999);

        // portal normal
        v25.vWorldPosition.x = face_center_x;
        v25.vWorldPosition.y = face_center_y;
        v25.vWorldPosition.z = face_center_z;

        v26.vWorldPosition.x = face_center_x + a1.x * 400.0f;
        v26.vWorldPosition.y = face_center_y + a1.y * 400.0f;
        v26.vWorldPosition.z = face_center_z + a1.z * 400.0f;

        pCamera3D->do_draw_debug_line_sw(&v25, -1, &v26, 0xFFFF00u, 0, 0);
    }


    return true;
}

//----- (0049C5B0) --------------------------------------------------------
stru10::stru10() { /*this->bDoNotDrawPortalFrustum = false;*/ }

//----- (0049C5BD) --------------------------------------------------------
stru10::~stru10() {}

//----- (0049C5DA) --------------------------------------------------------
bool stru10::CalcPortalShapePoly(BLVFace *pFace, RenderVertexSoft *pVertices,
                     unsigned int *pNumVertices, IndoorCameraD3D_Vec4 *pOutFrustum,
                     RenderVertexSoft *pOutBounding) {
    // calc poly limits
    RenderVertexSoft pLimits[4];
    _49CE9E(pFace, pVertices, *pNumVertices, pLimits);

    if (CalcFaceBounding(pFace, pLimits, 4, pOutBounding)) {
        pCamera3D->ViewTransform(pOutBounding, 4);
        pCamera3D->Project(pOutBounding, 4);
        RenderVertexSoft temp[4];
        memcpy(temp, pOutBounding, sizeof(RenderVertexSoft) * 4);

        // make sure frustum planes will be on correct side
        if (pOutBounding[0].vWorldViewProjX > pOutBounding[3].vWorldViewProjX) {
            memcpy(&pOutBounding[0], &temp[3], sizeof(RenderVertexSoft));
            memcpy(&pOutBounding[2], &temp[1], sizeof(RenderVertexSoft));
            memcpy(&pOutBounding[3], &temp[0], sizeof(RenderVertexSoft));
            memcpy(&pOutBounding[1], &temp[2], sizeof(RenderVertexSoft));
        }

        // calculate the new frustum for this portal
        bool test = CalcPortalFrustum(pOutBounding, pOutFrustum);

        // if normal in z - portals dont work out so use camera
        if (pFace->pFacePlane.vNormal.z == 1.0 || pFace->pFacePlane.vNormal.z == -1.0) {
            logger->Info("Portal z normal");
            for (int i = 0; i < 4; i++) {
                pOutFrustum[i].x = pCamera3D->FrustumPlanes[i].x;
                pOutFrustum[i].y = pCamera3D->FrustumPlanes[i].y;
                pOutFrustum[i].z = pCamera3D->FrustumPlanes[i].z;
                pOutFrustum[i].dot = pCamera3D->FrustumPlanes[i].w;
            }
        }

        return test;
    }

    return false;
}

//----- (0049C720) --------------------------------------------------------
bool stru10::CalcPortalFrustum(RenderVertexSoft *pFaceBounding, IndoorCameraD3D_Vec4 *pPortalDataFrustum) {
    Vec3_float_ pRayStart;
    pRayStart.x = (double)pCamera3D->vCameraPos.x;
    pRayStart.y = (double)pCamera3D->vCameraPos.y;
    pRayStart.z = (double)pCamera3D->vCameraPos.z;

    if (CalcPortalFrustumPlane(&pFaceBounding[1], &pFaceBounding[0], &pRayStart, &pPortalDataFrustum[0]) &&
        CalcPortalFrustumPlane(&pFaceBounding[3], &pFaceBounding[2], &pRayStart, &pPortalDataFrustum[1]) &&
        CalcPortalFrustumPlane(&pFaceBounding[0], &pFaceBounding[3], &pRayStart, &pPortalDataFrustum[2]) &&
        CalcPortalFrustumPlane(&pFaceBounding[2], &pFaceBounding[1], &pRayStart, &pPortalDataFrustum[3])) {
        return true;
    }

    return false;
}

//----- (0049C7C5) --------------------------------------------------------
bool stru10::CalcPortalFrustumPlane(RenderVertexSoft *pFaceBounding1,
                            RenderVertexSoft *pFaceBounding2,
                            Vec3_float_ *pRayStart,
                            IndoorCameraD3D_Vec4 *pPortalDataFrustum) {
    Vec3_float_ ray_dir;
    Vec3_float_ pRay2;

    ray_dir.x = pFaceBounding1->vWorldPosition.x - pRayStart->x;  // get ray for cmera to bounding1
    ray_dir.y = pFaceBounding1->vWorldPosition.y - pRayStart->y;
    ray_dir.z = pFaceBounding1->vWorldPosition.z - pRayStart->z;
    Vec3_float_::Cross(
        &ray_dir, &pRay2,
        pFaceBounding2->vWorldPosition.x - pFaceBounding1->vWorldPosition.x,
        pFaceBounding2->vWorldPosition.y - pFaceBounding1->vWorldPosition.y,
        pFaceBounding2->vWorldPosition.z - pFaceBounding1->vWorldPosition.z);

    float sqr_mag = pRay2.x * pRay2.x + pRay2.y * pRay2.y + pRay2.z * pRay2.z;
    if (fabsf(sqr_mag) > 1e-6f) {
        float inv_mag = 1.0f / sqrtf(sqr_mag);
        pRay2.x *= inv_mag;
        pRay2.y *= inv_mag;
        pRay2.z *= inv_mag;
        pRay2.Normalize();

        pPortalDataFrustum->x = pRay2.x;
        pPortalDataFrustum->y = pRay2.y;
        pPortalDataFrustum->z = pRay2.z;
        pPortalDataFrustum->dot = pRayStart->z * pRay2.z +
                                  pRayStart->y * pRay2.y +
                                  pRayStart->x * pRay2.x;
        return true;
    }
    return false;
}
