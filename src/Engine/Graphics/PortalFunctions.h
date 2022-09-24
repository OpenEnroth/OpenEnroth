#pragma once

#include <Engine/VectorTypes.h>

/*  127 */
#pragma pack(push, 1)
struct stru10 {
    stru10();
    virtual ~stru10();
    bool CalcPortalShapePoly(struct BLVFace *pFace, struct RenderVertexSoft *pVertices,
                 unsigned int *pNumVertices, struct IndoorCameraD3D_Vec4 *a5,
                 struct RenderVertexSoft *pOutBounding);
    bool CalcPortalFrustum(struct RenderVertexSoft *pFaceBounding, struct IndoorCameraD3D_Vec4 *pPortalDataFrustum);
    bool CalcPortalFrustumPlane(struct RenderVertexSoft *pFaceBounding1,
                                struct RenderVertexSoft *pFaceBounding2,
                                Vec3f *pRayStart,
                                struct IndoorCameraD3D_Vec4 *pPortalDataFrustum);
    bool CalcFaceBounding(struct BLVFace *pFace,
                          struct RenderVertexSoft *pFaceLimits,
                          unsigned int uNumVertices,
                          struct RenderVertexSoft *pOutBounding);
    void CalcPolygonLimits(struct BLVFace *pFace,
                           struct RenderVertexSoft pOutVertices[4]);
    void _49CE9E(struct BLVFace *pFace, struct RenderVertexSoft *pVertices,
                 unsigned int uNumVertices, RenderVertexSoft *pOutLimits);
};
#pragma pack(pop)
