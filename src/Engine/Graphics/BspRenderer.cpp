#include "Engine/Graphics/BspRenderer.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Engine.h"

#include "Library/Logger/Logger.h"

// TODO(yoctozepto): we should not see it here
BspRenderer *pBspRenderer = new BspRenderer();


//----- (004B0EA8) --------------------------------------------------------
void BspRenderer::AddFace(const int node_id, const int uFaceID) {
    assert(uFaceID > -1 && uFaceID < pIndoor->faces.size() && "please report with a nearby save file");

    BLVFace *pFace = &pIndoor->faces[uFaceID];
    pFace->attributes |= FACE_SeenByParty;

    // NOTE(yoctozepto): the below happens, e.g., on various stairs
    // TODO(yoctozepto): might be nice to check if the vertices actually form a plane and not a line;
    //                   this could be done when loading the location and filtering out such broken faces
    if (pFace->numVertices < 3) {
        return;  // nothing to render
    }

    const BspRenderer_ViewportNode *currentNode = &nodes[node_id];

    // check if any triangle of the face can be seen

    static RenderVertexSoft originalFaceVertices[64];
    static RenderVertexSoft clippedFaceVertices[64];

    // TODO(yoctozepto): are face vertices consecutive? are face vertices shared/overlapping?
    for (unsigned k = 0; k < pFace->numVertices; ++k) {
        originalFaceVertices[k].vWorldPosition.x = pIndoor->vertices[pFace->vertexIds[k]].x;
        originalFaceVertices[k].vWorldPosition.y = pIndoor->vertices[pFace->vertexIds[k]].y;
        originalFaceVertices[k].vWorldPosition.z = pIndoor->vertices[pFace->vertexIds[k]].z;
    }

    unsigned int pNewNumVertices = pFace->numVertices;

    // TODO(yoctozepto): original vertices could have been just Vec3f
    // clip to current viewing node frustum
    pCamera3D->ClipFaceToFrustum(
        originalFaceVertices,
        &pNewNumVertices,
        clippedFaceVertices,
        currentNode->ViewportNodeFrustum.data());

    if (!pNewNumVertices) {
        return;  // no triangle of face in view
    }

    if (!pFace->isPortal()) {
        // NOTE(yoctozepto): the below happens, e.g., on various stairs
        if (pFace->Invisible())
            return;  // nothing to render

        // TODO(yoctozepto): does the below happen?
        // TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also OpenGLRenderer)
        if (!pFace->GetTexture()) {
            return;  // nothing to render
        }

        if (!pCamera3D->is_face_faced_to_cameraBLV(pFace)) {
            return;  // we don't see the face, no need to render
        }

        assert(num_faces < 1500 && "please report with a nearby save file");

        // add face and return
        auto newFace = &faces[num_faces];
        newFace->uFaceID = uFaceID;
        newFace->uNodeID = node_id;
        num_faces++;
        return;
    }

    // portals are invisible faces marking the transition between sectors

    // NOTE(yoctozepto): this is a quick test for a sure loop,
    //                   more cases are covered with the sector uniqueness check further on
    //                   logically, this would not be necessary to skip as the next test would cover that;
    //                   however, since we allow much about anything in the party node, it could happen that we allow a "mirror image"
    //                   through the portal the party is standing in - this will then filter it out
    // dont add the face we are looking through
    if (currentNode->uFaceID == uFaceID)
        return;

    const bool isPortalFlipped = currentNode->uSectorID != pFace->sectorId;

    // NOTE(yoctozepto): (2) ignore a portal if it would be processed backwards to the party
    //                   as this might result in infinite loops;
    //                   see issues #417, #1869, and the locations in the "Mercenary Guild" and "Tunnels to Eeofol";
    //                   (1) processing these portals is required in the same node as party because getting really close
    //                   to them might cause them to appear on the wrong side
    //                   (this is very similar to the exception below with the camera frustum);
    //                   still, because of seemingly unreliable frusta, this is not covering every possible loop
    //                   so there is another loop check further on;
    //                   another use of this relatively simple condition is beyond loop prevention - it can also
    //                   optimise away unnecessary nodes - this happens in the "Mercenary Guild" in the meeting room
    //                   where there is a ceiling portal and the room is split artificially into two sectors (just why?!)
    //                   - running without this causes the second half of the room to be "seen" both normally and via
    //                   the ceiling (because its normal is in Z and it has frustum reset to camera frustum)
    if (/*(1)*/ node_id != 0 && /*(2)*/ isPortalFlipped == pCamera3D->is_face_faced_to_cameraBLV(pFace)) {
        return;
    }

    assert(num_nodes < 150 && "please report with a nearby save file");

    // start to construct the new node

    auto newNode = &nodes[num_nodes];

    // TODO(yoctozepto): remove it from here
    static RenderVertexSoft pPortalBounding[4];

    // calculates the portal bounding and frustum
    bool isFrustumBuilt = CalcPortalShapePoly(
        pFace,
        clippedFaceVertices,
        &pNewNumVertices,
        newNode->ViewportNodeFrustum.data(),
        pPortalBounding);

    if (!isFrustumBuilt) {
        return;  // no way we can see through this portal
    }

    // new node should have new sector; use the back one if the front one is current
    newNode->uSectorID = isPortalFlipped ? pFace->sectorId : pFace->backSectorId;
    newNode->uFaceID = uFaceID;
    newNode->parentNodeId = node_id;

    // NOTE(yoctozepto): the final check for loops;
    //                   it happens that, despite the logic of previous checks being fine on paper, loops still happen;
    //                   there could be some subtle bug around the frusta construction or application that escapes the perception
    //                   of our small team of developers
    int nodeIdToCheck = currentNode->parentNodeId;
    while (nodeIdToCheck != -1) {
        if (nodes[nodeIdToCheck].uSectorID == newNode->uSectorID) {  // would mean we see some sector through the same sector
            return;
        }
        nodeIdToCheck = nodes[nodeIdToCheck].parentNodeId;
    }

    // assume it is a new sector and check it below
    bool isNewSector = true;

    // check if it is a new sector
    // NOTE(yoctozepto): having the same sector id in two nodes causes faces to be added twice
    //                   but this is expected because of how the view is constructed;
    //                   it is possible to have the same sector observed from two different portals
    //                   which give different views;
    //                   this is also recursive so we can have another portal in that duplicate sector
    //                   being observed differently from each portal that led to this sector;
    //                   an example is the open corridor in "Temple of Light";
    //                   see PR #1850 and issue #1704 for the discussion and save file
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i].uSectorID == newNode->uSectorID) {
            isNewSector = false;
            break;
        }
    }

    const int boundingslack = 128;

    // NOTE(yoctozepto): (1) when the party is standing near (in a `boundingslack`) a portal of the same node (`id == 0`), then we need to fix the frustum
    //                   because it is likely to clip the view too much and miss to render the faces behind it
    //                   (2) normal in z (1/-1) does not work too well - similar issue happens, e.g., in "Temple of Light" when looking at the indoor sky ceiling
    //                   behind a portal in the big room before the hidden stairs
    if (/* (1) */(node_id == 0 && pFace->boundingBox.intersectsCube(Vec3f(pCamera3D->vCameraPos.x, pCamera3D->vCameraPos.y, pCamera3D->vCameraPos.z), boundingslack))
        || /* (2) */(pFace->facePlane.normal.z == 1.0 || pFace->facePlane.normal.z == -1.0)) {
        newNode->SetFrustumToCamera();
    }

    // keep track of new sectors only
    if (isNewSector) {
        // NOTE(yoctozepto): the below happens, e.g., when looking back after entering the room behind hidden stairs in "Temple of Light";
        //                   there are lots of portals in sight from there
        // drop all sectors beyond config limit
        if (uNumVisibleNotEmptySectors >= engine->config->graphics.MaxVisibleSectors.value()) {
            logger->warning("Hit visible sector limit but needed to add new one!");
        } else {
            AddSector(newNode->uSectorID);
        }
    }

    AddNode();  // can recurse back to this function
}

void BspRenderer::AddSector(int sectorId) {
    pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[uNumVisibleNotEmptySectors] = sectorId;
    uNumVisibleNotEmptySectors++;
}


void BspRenderer::Clear() {
    // reset lists
    num_faces = 0;
    num_nodes = 0;
    uNumVisibleNotEmptySectors = 0;
}


//----- (0043F953) --------------------------------------------------------
void BspRenderer::Render() {
    Clear();

    if (pBLVRenderParams->uPartySectorID) {
        // set to current sector - using eye sector here because feet can be in other sector on horizontal portal
        nodes[0].uSectorID = pBLVRenderParams->uPartyEyeSectorID;
        // this node is being observed directly, not through another face
        nodes[0].uFaceID = -1;
        nodes[0].parentNodeId = -1;
        nodes[0].SetFrustumToCamera();
        AddSector(nodes[0].uSectorID);

        AddNode();
    }
}


//----- (00440639) --------------------------------------------------------
void BspRenderer::AddNode() {
    const int node_id = num_nodes;
    num_nodes++;

    BLVSector *pSector = &pIndoor->sectors[nodes[node_id].uSectorID];

    for (uint16_t faceId : pSector->nonBspFaceIds)
        AddFace(node_id, faceId);  // can recurse back to this function

    if (pSector->flags & 0x10) {
        AddBSPFaces(node_id, pSector->firstBspNode);  // can recurse back to this function through AddFace
    }
}


//----- (004406BC) --------------------------------------------------------
void BspRenderer::AddBSPFaces(const int node_id, const int initialBSPNodeId) {
    BLVSector *pSector;       // esi@2
    BSPNode *bspNode;         // edi@2
    BLVFace *pFace;           // eax@2
    int bspNodeId = initialBSPNodeId;  // for tail recursion optimisation, see below

    BspRenderer_ViewportNode *node = &nodes[node_id];

    // NOTE(yoctozepto): tail recursion optimisation;
    //                   normally, this BSP node exploration is recursive on two branches but the second recursion can be optimised
    //                   because it's in the tail call position - in here, it has been optimised explicitly through the following loop
    do {
        pSector = &pIndoor->sectors[node->uSectorID];
        bspNode = &pIndoor->nodes[bspNodeId];
        pFace = &pIndoor->faces[pSector->faceIds[bspNode->uBSPFaceIDOffset]];

        bool isFaceFront = pCamera3D->is_face_faced_to_cameraBLV(pFace);
        // NOTE(yoctozepto): if the face is a portal going from a different sector, then its normal is inverted, so invert the computed value
        if (pFace->isPortal() && pFace->sectorId != node->uSectorID)
            isFaceFront = !isFaceFront;

        int otherBSPNodeId = isFaceFront ? bspNode->uBack : bspNode->uFront;

        if (otherBSPNodeId != -1)
            AddBSPFaces(node_id, otherBSPNodeId);

        for (int i = 0; i < bspNode->uNumBSPFaces; i++) {
            AddFace(node_id, pSector->faceIds[bspNode->uBSPFaceIDOffset + i]);  // can recurse back to this function through AddNode
        }

        // tail recursion optimised call
        bspNodeId = isFaceFront ? bspNode->uFront : bspNode->uBack;
    } while (bspNodeId != -1);
}


void BspRenderer_ViewportNode::SetFrustumToCamera() {
    for (int i = 0; i < 4; i++) {
        ViewportNodeFrustum[i].normal.x = pCamera3D->FrustumPlanes[i].x;
        ViewportNodeFrustum[i].normal.y = pCamera3D->FrustumPlanes[i].y;
        ViewportNodeFrustum[i].normal.z = pCamera3D->FrustumPlanes[i].z;
        ViewportNodeFrustum[i].dist = -pCamera3D->FrustumPlanes[i].w;
    }
}
