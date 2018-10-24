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

    float x;
    float y;
    float z;
    char field_C;
    char pad[3];
};
#pragma pack(pop)

/*  126 */
#pragma pack(push, 1)
struct stru9 {
    //----- (00498093) --------------------------------------------------------
    inline stru9() {}

    bool _4980B9(struct RenderVertexSoft *a1, unsigned int uNumVertices,
                 float a3, float a4, float a5,
                 struct RenderVertexSoft *pOutVertices,
                 signed int *pOutNumVertices);
    bool _498377(struct RenderVertexSoft *pPortalBounding,
                 unsigned int uNumVertices,
                 struct IndoorCameraD3D_Vec4 *pVertices,
                 struct RenderVertexSoft *pVertices2,
                 unsigned int *pOutNumVertices);
    bool AdjustVerticesToFrustumPlane(RenderVertexSoft *pInVertices, signed int pInNumVertices,
        RenderVertexSoft *pOutVertices,
        unsigned int *pOutNumVertices,
        struct Vec3_float_ *CamFrustumNormal, float CamDotDistance, char *VertsAdjusted,
        int unused);
    void AddVertex(struct VertexBuffer *pVertexBuffer,
                   struct RenderVertexSoft *pVertex);
    bool ClipDecalVertsToFace(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                 struct RenderVertexSoft *a3, struct stru312 *a4,
                 struct RenderVertexSoft *a5);
    bool AreVectorsCollinear(struct RenderVertexSoft *a1,
                             struct RenderVertexSoft *a2, struct stru312 *a3);
    bool DoDecalVertsNeedClipping(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                 struct RenderVertexSoft *a3, struct stru312 *a4);

    void (***vdestructor_ptr)(stru9 *, bool);
};
#pragma pack(pop)
