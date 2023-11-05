#pragma once

#include <array>

#include "Engine/Graphics/Camera.h"

#include "Library/Geometry/Plane.h"

struct BspRenderer_ViewportNode {
    int uSectorID = 0;  // sector that this node shows
    int uFaceID = 0;
    int viewing_portal_id = 0;  // face id of the portal through which we're seeing this node
    std::array<Planef, 4> ViewportNodeFrustum = {{}};  // frustum planes of portal
    std::array<RenderVertexSoft, 4> pPortalBounding = {{}};  // extents of portal
};

struct BspFace {
    int uFaceID = 0;
    int uNodeID = 0;
};

struct BspRenderer {
    void AddFaceToRenderList_d3d(int node_id, int uFaceID);
    void MakeVisibleSectorList();

    unsigned int num_faces = 0;
    std::array<BspFace, 1500> faces = {{}};

    unsigned int num_nodes = 0;
    std::array<BspRenderer_ViewportNode, 150> nodes = {{}};

    unsigned int uNumVisibleNotEmptySectors = 0;
    std::array<int, 150> pVisibleSectorIDs_toDrawDecorsActorsEtcFrom = {{}};
};

extern BspRenderer *pBspRenderer;

void PrepareBspRenderList_BLV();
void AddBspNodeToRenderList(int node_id);
void AddNodeBSPFaces(int node_id, int uFirstNode);  // idb

