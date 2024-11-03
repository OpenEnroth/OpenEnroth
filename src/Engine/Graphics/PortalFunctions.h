#pragma once

#include "Library/Geometry/Vec.h"
#include "Library/Geometry/Plane.h"

struct BLVFace;
struct RenderVertexSoft;

bool CalcPortalShapePoly(BLVFace *pFace, RenderVertexSoft *pVertices,
                         unsigned int *pNumVertices, Planef *a5,
                         RenderVertexSoft *pOutBounding);
bool CalcPortalFrustum(RenderVertexSoft *pFaceBounding, Planef *pPortalDataFrustum);
bool CalcPortalFrustumPlane(RenderVertexSoft *pFaceBounding1,
                            RenderVertexSoft *pFaceBounding2,
                            Vec3f *pRayStart,
                            Planef *pPortalDataFrustum);
bool CalcFaceBounding(BLVFace *pFace,
                      RenderVertexSoft *pFaceLimits,
                      unsigned int uNumVertices,
                      RenderVertexSoft *pOutBounding);
void CalcPolygonLimits(BLVFace *pFace,
                       RenderVertexSoft pOutVertices[4]);
void _49CE9E(BLVFace *pFace, RenderVertexSoft *pVertices,
             unsigned int uNumVertices, RenderVertexSoft *pOutLimits);
