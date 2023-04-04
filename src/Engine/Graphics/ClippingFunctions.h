#pragma once
#include "Engine/Graphics/IRender.h"

/*  341 */
#pragma pack(push, 1)
struct VertexBuffer {
    //----- (0049835A) --------------------------------------------------------
    inline VertexBuffer() {
        for (unsigned int i = 0; i < 64; ++i) pVertices[i].flt_2C = 0.0f;

        uNumVertices = 0;
    }

    RenderVertexSoft pVertices[64];
    int uNumVertices;
};
#pragma pack(pop)

/*  342 */
#pragma pack(push, 1)
struct stru312 {
    //----- (004BE6D1) --------------------------------------------------------
    inline stru312() {}

    float x = 0;
    float y = 0;
    float z = 0;
    char field_C = 0;
    char pad[3]{};
};
#pragma pack(pop)

/*  126 */
#pragma pack(push, 1)
struct stru9 {
    //----- (00498093) --------------------------------------------------------
    inline stru9() {}

    bool ClipVertsToFace(struct RenderVertexSoft *a1, unsigned int uNumVertices,
                 float a3, float a4, float a5,
                 struct RenderVertexSoft *pOutVertices,
                 signed int *pOutNumVertices);
    bool ClipVertsToPortal(struct RenderVertexSoft *pPortalBounding,
                 unsigned int uNumVertices,
                 struct IndoorCameraD3D_Vec4 *pVertices,
                 struct RenderVertexSoft *pVertices2,
                 unsigned int *pOutNumVertices);
    bool ClipVertsToFrustumPlane(RenderVertexSoft *pInVertices, signed int pInNumVertices,
                                 RenderVertexSoft *pOutVertices,
                                 unsigned int *pOutNumVertices,
                                 Vec3f *CamFrustumNormal, float CamDotDistance, char *VertsAdjusted,
                                 int unused);
    void AddVertex(struct VertexBuffer *pVertexBuffer,
                   struct RenderVertexSoft *pVertex);
    bool AdjustVertToClipEdge(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                 struct RenderVertexSoft *a3, struct stru312 *a4,
                 struct RenderVertexSoft *a5);
    bool AreVectorsCollinear(struct RenderVertexSoft *a1,
                             struct RenderVertexSoft *a2, struct stru312 *a3);
    bool DoDecalVertsNeedClipping(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                 struct RenderVertexSoft *a3, struct stru312 *a4);
};
#pragma pack(pop)
