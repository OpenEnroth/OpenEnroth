#pragma once

#include "Library/Geometry/Vec.h"
#include "Library/Geometry/Plane.h"

struct BLVFace;
struct RenderVertexSoft;

bool CalcPortalShapePoly(const BLVFace *pFace, RenderVertexSoft *pVertices,
                         unsigned int *pNumVertices, Planef *pOutFrustum, RenderVertexSoft* pOutBounding);
bool CalcPortalFrustum(RenderVertexSoft *pFaceBounding, Planef *pPortalDataFrustum);
bool CalcPortalFrustumPlane(RenderVertexSoft *pFaceBounding1,
                            RenderVertexSoft *pFaceBounding2,
                            Vec3f *pRayStart,
                            Planef *pPortalDataFrustum);
bool CalcFaceBounding(const BLVFace *pFace,
                      RenderVertexSoft *pFaceLimits,
                      unsigned int uNumVertices,
                      RenderVertexSoft *pOutBounding);
void CalcPolygonLimits(const BLVFace *pFace,
                       RenderVertexSoft pOutVertices[4]);
void _49CE9E(const BLVFace *pFace, RenderVertexSoft *pVertices,
             unsigned int uNumVertices, RenderVertexSoft *pOutLimits);
