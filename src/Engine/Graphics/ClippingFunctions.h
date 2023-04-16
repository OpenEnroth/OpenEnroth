#pragma once

#include "Engine/Graphics/IRender.h"

struct IndoorCameraD3D_Vec4;

struct VertexBuffer {
    std::array<RenderVertexSoft, 64> pVertices = {{}};
    int uNumVertices;
};

// TODO(captainurist): this is just Vec3f
struct stru312 {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct ClippingFunctions {
    static bool ClipVertsToFace(RenderVertexSoft *a1, unsigned int uNumVertices, float a3, float a4, float a5,
                                RenderVertexSoft *pOutVertices, signed int *pOutNumVertices);

    static bool ClipVertsToPortal(RenderVertexSoft *pPortalBounding, unsigned int uNumVertices, IndoorCameraD3D_Vec4 *pVertices,
                                  RenderVertexSoft *pVertices2, unsigned int *pOutNumVertices);

    static bool ClipVertsToFrustumPlane(RenderVertexSoft *pInVertices, signed int pInNumVertices, RenderVertexSoft *pOutVertices,
                                        unsigned int *pOutNumVertices, Vec3f *CamFrustumNormal, float CamDotDistance, char *VertsAdjusted, int unused);

    static void AddVertex(VertexBuffer *pVertexBuffer, RenderVertexSoft *pVertex);

    static bool AdjustVertToClipEdge(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, stru312 *a4, RenderVertexSoft *a5);

    static bool AreVectorsCollinear(RenderVertexSoft *a1, RenderVertexSoft *a2, stru312 *a3);

    static bool DoDecalVertsNeedClipping(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, stru312 *a4);
};
