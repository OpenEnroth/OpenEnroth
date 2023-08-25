#pragma once

#include <array>
#include "Utility/Geometry/Plane.h"
#include "Engine/Graphics/RenderEntities.h"
#include "Utility/Geometry/Vec.h"

struct Planef;
struct RenderVertexSoft;

struct VertexBuffer {
    std::array<RenderVertexSoft, 64> pVertices = {{}};
    int uNumVertices;
};

struct ClippingFunctions {
    static bool ClipVertsToFace(RenderVertexSoft *a1, unsigned int uNumVertices, float a3, float a4, float a5,
                                RenderVertexSoft *pOutVertices, signed int *pOutNumVertices);

    static bool ClipVertsToPortal(RenderVertexSoft *pPortalBounding, unsigned int uNumVertices, Planef *pVertices,
                                  RenderVertexSoft *pVertices2, unsigned int *pOutNumVertices);

    static bool ClipVertsToFrustumPlane(RenderVertexSoft *pInVertices, signed int pInNumVertices, RenderVertexSoft *pOutVertices,
                                        unsigned int *pOutNumVertices, Vec3f *CamFrustumNormal, float CamDotDistance, char *VertsAdjusted, int unused);

    static void AddVertex(VertexBuffer *pVertexBuffer, RenderVertexSoft *pVertex);

    static bool AdjustVertToClipEdge(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, Vec3f *a4, RenderVertexSoft *a5);

    static bool AreVectorsCollinear(RenderVertexSoft *a1, RenderVertexSoft *a2, Vec3f *a3);

    static bool DoDecalVertsNeedClipping(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, Vec3f *a4);
};
