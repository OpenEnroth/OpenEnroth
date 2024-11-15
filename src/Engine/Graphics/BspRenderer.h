#pragma once

#include <array>

#include "Engine/Graphics/Camera.h"

#include "Library/Geometry/Plane.h"

struct BspRenderer;

struct BspRenderer_ViewportNode {
    int uSectorID = 0;  // sector that this node shows
    int uFaceID = 0;  // face id of the portal through which we're seeing this node
    int parentNodeId = 0;
    std::array<Planef, 4> ViewportNodeFrustum = {{}};  // frustum planes of portal

 private:
    void SetFrustumToCamera();

    friend BspRenderer;
};

struct BspFace {
    int uFaceID = 0;
    int uNodeID = 0;
};

struct BspRenderer {
 public:
    void Clear();
    void Render();

    // TODO(yoctozepto): hide these
    unsigned int num_faces = 0;
    std::array<BspFace, 1500> faces = { {} };

    unsigned int num_nodes = 0;
    std::array<BspRenderer_ViewportNode, 150> nodes = { {} };

    unsigned int uNumVisibleNotEmptySectors = 0;
    std::array<int, 150> pVisibleSectorIDs_toDrawDecorsActorsEtcFrom = { {} };

 private:
    void AddFace(const int node_id, const int uFaceID);
    void AddNode();
    void AddBSPFaces(const int node_id, const int bspNodeId);
    void AddSector(int sectorId);
};

extern BspRenderer *pBspRenderer;
