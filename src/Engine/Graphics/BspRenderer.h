#pragma once

#include <array>

#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/IRender.h"

struct BspRenderer_ViewportNode {
    uint16_t uSectorID = 0;  // sector that this node shows
    uint16_t uFaceID = 0;
    unsigned int viewing_portal_id = 0;  // portal/ node through which we're seeing this node
    std::array<IndoorCameraD3D_Vec4, 4> ViewportNodeFrustum = {{}};  // frustum planes of portal
    std::array<RenderVertexSoft, 4> pPortalBounding = {{}};  // extents of portal
};

struct BspFace {
    uint16_t uFaceID = 0;
    uint16_t uNodeID = 0;
};

struct BspRenderer {
    void AddFaceToRenderList_d3d(unsigned int node_id, unsigned int uFaceID);
    void MakeVisibleSectorList();

    unsigned int num_faces = 0;
    std::array<BspFace, 1500> faces = {{}};

    unsigned int num_nodes = 0;
    std::array<BspRenderer_ViewportNode, 150> nodes = {{}};

    unsigned int uNumVisibleNotEmptySectors = 0;
    std::array<uint16_t, 150> pVisibleSectorIDs_toDrawDecorsActorsEtcFrom = {{}};
};

extern BspRenderer *pBspRenderer;

void PrepareBspRenderList_BLV();
void AddBspNodeToRenderList(unsigned int node_id);
void AddNodeBSPFaces(unsigned int node_id, unsigned int uFirstNode);  // idb

