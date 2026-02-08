#include "Engine/Graphics/PortalFunctions.h"

#include "Engine/Engine.h"
#include "Engine/Graphics/Camera.h"
#include "Indoor.h"

#include "Library/Color/ColorTable.h"
#include "Library/Logger/Logger.h"

int _49CE9E_sub0_x(RenderVertexSoft *pVertices, unsigned int uNumVertices,
                   float test_val) {
    float max_val = FLT_MAX;
    int idx = -1;

    float temp_val;
    for (unsigned i = 0; i < uNumVertices; ++i) {
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
    for (unsigned i = 0; i < uNumVertices; ++i) {
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
    for (unsigned i = 0; i < uNumVertices; ++i) {
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
void _49CE9E(const BLVFace *pFace, RenderVertexSoft *pVertices,
                     unsigned int uNumVertices, RenderVertexSoft *pOutLimits) {
    assert(sizeof(RenderVertexSoft) == 0x30);

    RenderVertexSoft pLimits[64];
    CalcPolygonLimits(pFace, pLimits);

    if (pFace->attributes & FACE_XY_PLANE) {
        pOutLimits[0] = pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[0].vWorldPosition.x)];
        pOutLimits[2] = pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[2].vWorldPosition.x)];
        pOutLimits[1] = pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[1].vWorldPosition.y)];
        pOutLimits[3] = pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[3].vWorldPosition.y)];
    } else if (pFace->attributes & FACE_XZ_PLANE) {
        pOutLimits[0] = pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[0].vWorldPosition.x)];
        pOutLimits[2] = pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[2].vWorldPosition.x)];
        pOutLimits[1] = pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[1].vWorldPosition.z)];
        pOutLimits[3] = pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[3].vWorldPosition.z)];
    } else if (pFace->attributes & FACE_YZ_PLANE) {
        pOutLimits[0] = pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[0].vWorldPosition.y)];
        pOutLimits[2] = pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[2].vWorldPosition.y)];
        pOutLimits[1] = pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[1].vWorldPosition.z)];
        pOutLimits[3] = pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[3].vWorldPosition.z)];
    }
}

//----- (0049D379) --------------------------------------------------------
void CalcPolygonLimits(const BLVFace *pFace, RenderVertexSoft *pOutVertices) {
    FlatFace points;
    pFace->Flatten(&points, MODEL_INDOOR);

    float x_min = points.u[0];
    unsigned x_min_idx = 0;

    float x_max = points.u[0];
    unsigned x_max_idx = 0;

    float y_min = points.v[0];
    unsigned y_min_idx = 0;

    float y_max = points.v[0];
    unsigned y_max_idx = 0;

    for (unsigned i = 0; i < pFace->numVertices; ++i) {
        if (points.u[i] < x_min) {
            x_min = points.u[i];
            x_min_idx = i;
        }
        if (points.u[i] > x_max) {
            x_max = points.u[i];
            x_max_idx = i;
        }

        if (points.v[i] < y_min) {
            y_min = points.v[i];
            y_min_idx = i;
        }
        if (points.v[i] > y_max) {
            y_max = points.v[i];
            y_max_idx = i;
        }
    }

    RenderVertexSoft v1;  // [sp+30Ch] [bp-54h]@24
    v1.vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[x_min_idx]].x;
    v1.vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[x_min_idx]].y;
    v1.vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[x_min_idx]].z;
    pOutVertices[0] = v1;

    RenderVertexSoft v2;  // [sp+30Ch] [bp-54h]@24
    v2.vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[y_min_idx]].x;
    v2.vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[y_min_idx]].y;
    v2.vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[y_min_idx]].z;
    pOutVertices[1] = v2;

    RenderVertexSoft v3;  // [sp+30Ch] [bp-54h]@24
    v3.vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[x_max_idx]].x;
    v3.vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[x_max_idx]].y;
    v3.vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[x_max_idx]].z;
    pOutVertices[2] = v3;

    RenderVertexSoft v4;  // [sp+30Ch] [bp-54h]@24
    v4.vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[y_max_idx]].x;
    v4.vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[y_max_idx]].y;
    v4.vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[y_max_idx]].z;
    pOutVertices[3] = v4;
}

//----- (0049C9E3) --------------------------------------------------------
bool CalcFaceBounding(const BLVFace *pFace, RenderVertexSoft *pFaceLimits,
                              unsigned int uNumVertices,
                              RenderVertexSoft *pOutBounding) {
    Vec3f a1;
    a1.x = 0.0f;
    a1.y = 0.0f;
    a1.z = 0.0f;

    float var_28;
    float var_24;
    // TODO(captainurist): code looks very similar to Camera3D::GetFacetOrientation
    switch (pFace->polygonType) {
        case POLYGON_VerticalWall:
            a1.x = -pFace->facePlane.normal.y;  // направление полигона
            a1.y = pFace->facePlane.normal.x;
            a1.z = 0.0f;
            a1.normalize();

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
            assert(false);
            return false;
    }

    float face_center_x = 0;
    float face_center_y = 0;
    float face_center_z = 0;
    float a3 = 0;
    float var_8 = 0;

    if (pFace->attributes & FACE_XY_PLANE) {
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
    if (pFace->attributes & FACE_XZ_PLANE) {
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

        if (pFace->polygonType == POLYGON_VerticalWall) a3 /= a1.x;
    }
    if (pFace->attributes & FACE_YZ_PLANE) {
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
        if (pFace->polygonType == POLYGON_VerticalWall) {
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
    a1.x = pFace->facePlane.normal.x;
    a1.y = pFace->facePlane.normal.y;
    a1.z = pFace->facePlane.normal.z;


    RenderVertexSoft v25 = pOutBounding[0];  // [sp+10h] [bp-90h]@20
    float _dp = (v25.vWorldPosition.x - pCamera3D->vCameraPos.x) * a1.x +
                (v25.vWorldPosition.y - pCamera3D->vCameraPos.y) * a1.y +
                (v25.vWorldPosition.z - pCamera3D->vCameraPos.z) * a1.z;
    if (fabs(_dp) < 1e-6f) {
        logger->info("Epsilon check");
        v25 = pOutBounding[1];
        pOutBounding[1] = pOutBounding[3];
        pOutBounding[3] = v25;
    }


    RenderVertexSoft v26;

    // show portal outlines debug
    if (engine->config->debug.PortalOutlines.value()) {
        // portal frustums
        v26.vWorldPosition.x = pCamera3D->vCameraPos.x;
        v26.vWorldPosition.y = pCamera3D->vCameraPos.y;
        v26.vWorldPosition.z = pCamera3D->vCameraPos.z;

        // portal bounding
        pCamera3D->debug_outline_sw(pOutBounding, uNumVertices, colorTable.NeonGreen, 0.00019999999);

        // portal normal
        v25.vWorldPosition.x = face_center_x;
        v25.vWorldPosition.y = face_center_y;
        v25.vWorldPosition.z = face_center_z;

        v26.vWorldPosition.x = face_center_x + a1.x * 400.0f;
        v26.vWorldPosition.y = face_center_y + a1.y * 400.0f;
        v26.vWorldPosition.z = face_center_z + a1.z * 400.0f;

        pCamera3D->do_draw_debug_line_sw(&v25, colorTable.White, &v26, colorTable.Yellow, 0);
    }


    return true;
}

//----- (0049C5DA) --------------------------------------------------------
bool CalcPortalShapePoly(const BLVFace *pFace, RenderVertexSoft *pVertices,
                     unsigned int *pNumVertices, Planef *pOutFrustum, RenderVertexSoft *pOutBounding) {
    // calc poly limits
    RenderVertexSoft pLimits[4];
    _49CE9E(pFace, pVertices, *pNumVertices, pLimits);

    if (!CalcFaceBounding(pFace, pLimits, 4, pOutBounding)) {
        return false;
    }

    pCamera3D->ViewTransform(pOutBounding, 4);
    pCamera3D->Project(pOutBounding, 4);

    // make sure frustum planes will be on correct side
    if (pOutBounding[0].vWorldViewProj.x > pOutBounding[3].vWorldViewProj.x) {
        RenderVertexSoft temp[4];
        memcpy(temp, pOutBounding, sizeof(RenderVertexSoft) * 4);
        pOutBounding[0] = temp[3];
        pOutBounding[2] = temp[1];
        pOutBounding[3] = temp[0];
        pOutBounding[1] = temp[2];
    }

    // calculate the new frustum for this portal
    return CalcPortalFrustum(pOutBounding, pOutFrustum);
}

//----- (0049C720) --------------------------------------------------------
bool CalcPortalFrustum(RenderVertexSoft *pFaceBounding, Planef *pPortalDataFrustum) {
    Vec3f pRayStart;
    pRayStart.x = pCamera3D->vCameraPos.x;
    pRayStart.y = pCamera3D->vCameraPos.y;
    pRayStart.z = pCamera3D->vCameraPos.z;

    if (CalcPortalFrustumPlane(&pFaceBounding[1], &pFaceBounding[0], &pRayStart, &pPortalDataFrustum[0]) &&
        CalcPortalFrustumPlane(&pFaceBounding[3], &pFaceBounding[2], &pRayStart, &pPortalDataFrustum[1]) &&
        CalcPortalFrustumPlane(&pFaceBounding[0], &pFaceBounding[3], &pRayStart, &pPortalDataFrustum[2]) &&
        CalcPortalFrustumPlane(&pFaceBounding[2], &pFaceBounding[1], &pRayStart, &pPortalDataFrustum[3])) {
        return true;
    }

    return false;
}

//----- (0049C7C5) --------------------------------------------------------
bool CalcPortalFrustumPlane(RenderVertexSoft *pFaceBounding1,
                                    RenderVertexSoft *pFaceBounding2,
                                    Vec3f *pRayStart,
                                    Planef *pPortalDataFrustum) {
    Vec3f ray_dir = pFaceBounding1->vWorldPosition - *pRayStart; // get ray for cmera to bounding1
    Vec3f pRay2 = cross(ray_dir, pFaceBounding2->vWorldPosition - pFaceBounding1->vWorldPosition);

    float sqr_mag = pRay2.x * pRay2.x + pRay2.y * pRay2.y + pRay2.z * pRay2.z;
    if (fabsf(sqr_mag) > 1e-6f) {
        float inv_mag = 1.0f / sqrtf(sqr_mag);
        pRay2.x *= inv_mag;
        pRay2.y *= inv_mag;
        pRay2.z *= inv_mag;
        pRay2.normalize();

        pPortalDataFrustum->normal.x = pRay2.x;
        pPortalDataFrustum->normal.y = pRay2.y;
        pPortalDataFrustum->normal.z = pRay2.z;
        pPortalDataFrustum->dist = -dot(*pRayStart, pRay2);
        return true;
    }
    return false;
}
