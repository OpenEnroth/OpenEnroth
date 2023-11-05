#pragma once

#include "Library/Geometry/Vec.h"
#include "Library/Geometry/Plane.h"

struct stru10 {
    stru10();
    virtual ~stru10();
    bool CalcPortalShapePoly(struct BLVFace *pFace, struct RenderVertexSoft *pVertices,
                 unsigned int *pNumVertices, Planef *a5,
                 struct RenderVertexSoft *pOutBounding);
    bool CalcPortalFrustum(struct RenderVertexSoft *pFaceBounding, Planef *pPortalDataFrustum);
    bool CalcPortalFrustumPlane(struct RenderVertexSoft *pFaceBounding1,
                                struct RenderVertexSoft *pFaceBounding2,
                                Vec3f *pRayStart,
                                Planef *pPortalDataFrustum);
    bool CalcFaceBounding(struct BLVFace *pFace,
                          struct RenderVertexSoft *pFaceLimits,
                          unsigned int uNumVertices,
                          struct RenderVertexSoft *pOutBounding);
    void CalcPolygonLimits(struct BLVFace *pFace,
                           struct RenderVertexSoft pOutVertices[4]);
    void _49CE9E(struct BLVFace *pFace, struct RenderVertexSoft *pVertices,
                 unsigned int uNumVertices, RenderVertexSoft *pOutLimits);
};
