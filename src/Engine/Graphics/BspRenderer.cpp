#include "Engine/Graphics/BspRenderer.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Engine.h"

#include "Library/Logger/Logger.h"

BspRenderer *pBspRenderer = new BspRenderer();

//----- (004B0EA8) --------------------------------------------------------
void BspRenderer::AddFaceToRenderList_d3d(int node_id, int uFaceID) {
    int pTransitionSector;  // ax@11
    // int dotdist;                              // edx@15

    nodes[num_nodes].viewing_portal_id = -1;

    if (uFaceID >= pIndoor->pFaces.size()) return;
    BLVFace *pFace = &pIndoor->pFaces[uFaceID];

    if (!pFace->isPortal()) {
        if (num_faces < 1500) {
            // add face and return
            faces[num_faces].uFaceID = uFaceID;
            faces[num_faces++].uNodeID = node_id;
        } else {
            logger->info("Too many faces in BLV render");
        }
        return;
    }

    // portals are invisible faces marking the transition between sectors
    // dont add the face we are looking through
    if (nodes[node_id].uFaceID == uFaceID) return;

    // if node_id 0 and bounding box check with portal - ie party stood next to portal
    int boundingslack = 128;

    if (!node_id &&
        pFace->pBounding.intersectsCube(Vec3f(pCamera3D->vCameraPos.x, pCamera3D->vCameraPos.y, pCamera3D->vCameraPos.z), boundingslack)) {
        // we are standing at the portal plane
        pTransitionSector = pFace->uSectorID;
        // draw back sector if we are already doing this sector
        if (nodes[0].uSectorID == pTransitionSector)
            pTransitionSector = pFace->uBackSectorID;
        nodes[num_nodes].uSectorID = pTransitionSector;
        nodes[num_nodes].uFaceID = uFaceID;

        // set furstum to cam frustum
        for (int loop = 0; loop < 4; loop++) {
            nodes[num_nodes].ViewportNodeFrustum[loop].normal.x = pCamera3D->FrustumPlanes[loop].x;
            nodes[num_nodes].ViewportNodeFrustum[loop].normal.y = pCamera3D->FrustumPlanes[loop].y;
            nodes[num_nodes].ViewportNodeFrustum[loop].normal.z = pCamera3D->FrustumPlanes[loop].z;
            nodes[num_nodes].ViewportNodeFrustum[loop].dist = -pCamera3D->FrustumPlanes[loop].w;
        }

        AddBspNodeToRenderList(++num_nodes - 1);
        return;
    }
    // check if portal is visible on screen

    static RenderVertexSoft static_subAddFaceToRenderList_d3d_stru_F7AA08[64];
    static RenderVertexSoft static_subAddFaceToRenderList_d3d_stru_F79E08[64];

    for (unsigned k = 0; k < pFace->uNumVertices; ++k) {
        static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[k]].x;
        static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[k]].y;
        static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[k]].z;
    }

    unsigned int pNewNumVertices = pFace->uNumVertices;

    // accurate clip to current viewing nodes frustum
    bool vertadj = pCamera3D->ClipFaceToFrustum(
            static_subAddFaceToRenderList_d3d_stru_F7AA08, &pNewNumVertices,
            static_subAddFaceToRenderList_d3d_stru_F79E08,
            nodes[node_id].ViewportNodeFrustum.data(), 4, 0, 0);

    if (pNewNumVertices) {
        // current portal visible through previous
        pTransitionSector = pFace->uSectorID;
        if (nodes[node_id].uSectorID == pTransitionSector)
            pTransitionSector = pFace->uBackSectorID;
        nodes[num_nodes].uSectorID = pTransitionSector;
        nodes[num_nodes].uFaceID = uFaceID;

        // calculates the portal bounding and frustum
        bool bFrustumbuilt = engine->pStru10Instance->CalcPortalShapePoly(
                pFace, static_subAddFaceToRenderList_d3d_stru_F79E08,
                &pNewNumVertices, nodes[num_nodes].ViewportNodeFrustum.data(),
                nodes[num_nodes].pPortalBounding.data());

        auto boundingMatches = [](const std::array<RenderVertexSoft, 4> &l, const std::array<RenderVertexSoft, 4> &r) {
            for (int i = 0; i < l.size(); i++) {
                if (l[i].vWorldPosition != r[i].vWorldPosition)
                    return false;
            }
            return true;
        };

        // avoid circular loops in portals
        for (int test = 0; test < num_nodes; test++) {
            if (nodes[test].uSectorID == nodes[num_nodes].uSectorID &&
                nodes[test].uFaceID == nodes[num_nodes].uFaceID &&
                boundingMatches(nodes[test].pPortalBounding, nodes[num_nodes].pPortalBounding)) {
                return;
            }
        }

        if (bFrustumbuilt) {
            // add portal sector to drawing list
            assert(num_nodes < 150);
            nodes[num_nodes].viewing_portal_id = uFaceID;
            AddBspNodeToRenderList(++num_nodes - 1);
        }
    }
}


//----- (0043F333) --------------------------------------------------------
void BspRenderer::MakeVisibleSectorList() {
    bool onlist = false;
    uNumVisibleNotEmptySectors = 0;

    // TODO: this is actually n^2, might make sense to rewrite properly.

    for (unsigned i = 0; i < num_nodes; ++i) {
        onlist = false;
        for (unsigned j = 0; j < uNumVisibleNotEmptySectors; j++) {
            if (pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] == nodes[i].uSectorID) {
                onlist = true;
                break;
            }
        }

        if (!onlist)
            pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[uNumVisibleNotEmptySectors++] = nodes[i].uSectorID;

        // drop all sectors beyond config limit
        if (uNumVisibleNotEmptySectors >= engine->config->graphics.MaxVisibleSectors.value()) {
            logger->warning("Hit visible sector limit!");
            break;
        }
    }
}


//----- (0043F953) --------------------------------------------------------
void PrepareBspRenderList_BLV() {
    // reset faces list
    pBspRenderer->num_faces = 0;

    if (pBLVRenderParams->uPartySectorID) {
        // set node 0 to current sector - using eye sector here because feet can be in other sector on horizontal portal
        pBspRenderer->nodes[0].uSectorID = pBLVRenderParams->uPartyEyeSectorID;
        // set furstum to cam frustum
        for (int loop = 0; loop < 4; loop++) {
            pBspRenderer->nodes[0].ViewportNodeFrustum[loop].normal.x = pCamera3D->FrustumPlanes[loop].x;
            pBspRenderer->nodes[0].ViewportNodeFrustum[loop].normal.y = pCamera3D->FrustumPlanes[loop].y;
            pBspRenderer->nodes[0].ViewportNodeFrustum[loop].normal.z = pCamera3D->FrustumPlanes[loop].z;
            pBspRenderer->nodes[0].ViewportNodeFrustum[loop].dist = -pCamera3D->FrustumPlanes[loop].w;
        }

        // blank viewing node
        pBspRenderer->nodes[0].uFaceID = -1;
        pBspRenderer->nodes[0].viewing_portal_id = -1;
        pBspRenderer->num_nodes = 1;
        AddBspNodeToRenderList(0);
    }

    pBspRenderer->MakeVisibleSectorList();
}


//----- (00440639) --------------------------------------------------------
void AddBspNodeToRenderList(int node_id) {
    BLVSector *pSector = &pIndoor->pSectors[pBspRenderer->nodes[node_id].uSectorID];

    for (unsigned i = 0; i < pSector->uNumNonBSPFaces; ++i)
        pBspRenderer->AddFaceToRenderList_d3d(node_id, pSector->pFaceIDs[i]);  // рекурсия\recursion

    if (pSector->field_0 & 0x10) {
        AddNodeBSPFaces(node_id, pSector->uFirstBSPNode);
    }
}


//----- (004406BC) --------------------------------------------------------
void AddNodeBSPFaces(int node_id, int uFirstNode) {
    BLVSector *pSector;       // esi@2
    BSPNode *pNode;           // edi@2
    BLVFace *pFace;           // eax@2
    int v6;               // ax@6
    int v7;                   // ebp@10
    int v8;                   // ebx@10
    int v9;               // di@18

    BspRenderer_ViewportNode *node = &pBspRenderer->nodes[node_id];

    while (1) {
        pSector = &pIndoor->pSectors[node->uSectorID];
        pNode = &pIndoor->pNodes[uFirstNode];
        pFace = &pIndoor->pFaces[pSector->pFaceIDs[pNode->uBSPFaceIDOffset]];
        // check if we are in front or behind face
        float v5 = pFace->facePlane.dist +
             pCamera3D->vCameraPos.x * pFace->facePlane.normal.x +
             pCamera3D->vCameraPos.y * pFace->facePlane.normal.y +
             pCamera3D->vCameraPos.z * pFace->facePlane.normal.z;  // plane equation
        if (pFace->isPortal() && pFace->uSectorID != node->uSectorID) v5 = -v5;

        if (v5 <= 0)
            v6 = pNode->uFront;
        else
            v6 = pNode->uBack;

        if (v6 != -1) AddNodeBSPFaces(node_id, v6);

        v7 = pNode->uBSPFaceIDOffset;
        v8 = v7 + pNode->uNumBSPFaces;

        while (v7 < v8) {
            pBspRenderer->AddFaceToRenderList_d3d(node_id, pSector->pFaceIDs[v7++]);
        }

        v9 = v5 > 0 ? pNode->uFront : pNode->uBack;
        if (v9 == -1) break;
        uFirstNode = v9;
    }
}
