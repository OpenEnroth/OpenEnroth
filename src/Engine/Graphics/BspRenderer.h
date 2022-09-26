#pragma once

#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/IRender.h"

/*  164 */
#pragma pack(push, 1)
struct BspRenderer_ViewportNode {
    //----- (0043F2BF) --------------------------------------------------------
    inline BspRenderer_ViewportNode() {}

    //----- (0043F2A9) --------------------------------------------------------
    ~BspRenderer_ViewportNode() {}

    uint16_t uSectorID = 0;  // sector that this node shows
    uint16_t uFaceID;
    unsigned int viewing_portal_id;  // portal/ node through which we're seeing this node
    IndoorCameraD3D_Vec4 ViewportNodeFrustum[4];  // frustum planes of portal
    RenderVertexSoft pPortalBounding[4];  // extents of portal
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BspFace {
    uint16_t uFaceID;
    uint16_t uNodeID;
};
#pragma pack(pop)

/*  163 */
#pragma pack(push, 1)
struct BspRenderer {  // stru170
    //----- (0043F282) --------------------------------------------------------
    inline BspRenderer() {
        num_faces = 0;
        num_nodes = 0;
        uNumVisibleNotEmptySectors = 0;
    }

    void AddFaceToRenderList_d3d(unsigned int node_id, unsigned int uFaceID);
    void MakeVisibleSectorList();

    unsigned int num_faces;
    BspFace faces[1000]{};

    unsigned int num_nodes;
    BspRenderer_ViewportNode nodes[150];

    unsigned int uNumVisibleNotEmptySectors;
    uint16_t pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[150]{};
};
#pragma pack(pop)

extern BspRenderer *pBspRenderer;

void PrepareBspRenderList_BLV();
void AddBspNodeToRenderList(unsigned int node_id);
void AddNodeBSPFaces(unsigned int node_id, unsigned int uFirstNode);  // idb

