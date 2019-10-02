#include "Engine/Graphics/Indoor.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/LOD.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/stru367.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/stru10.h"
#include "Engine/Graphics/stru9.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"

#include "../OurMath.h"
#include "../Party.h"
#include "../stru123.h"
#include "DecorationList.h"
#include "Level/Decoration.h"
#include "Lights.h"
#include "Outdoor.h"
#include "Overlays.h"
#include "ParticleEngine.h"
#include "Sprites.h"
#include "Viewport.h"

IndoorLocation *pIndoor = new IndoorLocation;
BLVRenderParams *pBLVRenderParams = new BLVRenderParams;

LEVEL_TYPE uCurrentlyLoadedLevelType = LEVEL_null;

LightsData Lights;  // stru_F8AD28
stru337 stru_F81018;
BspRenderer_PortalViewportData stru_F8A590;
BspRenderer *pBspRenderer = new BspRenderer;  // idb
stru141_actor_collision_object stru_721530;
std::array<stru352, 480> stru_F83B80;

unsigned __int16 pDoorSoundIDsByLocationID[78] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 404, 302, 306, 308, 304, 308, 302, 400, 302, 300,
    308, 308, 306, 308, 308, 304, 300, 404, 406, 300, 400, 406, 404,
    306, 302, 408, 304, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 404, 304,
    400, 300, 300, 404, 304, 400, 300, 300, 404, 304, 400, 300, 300};

std::array<const char *, 11> _4E6BDC_loc_names = {
    "mdt12.blv", "d18.blv",   "mdt14.blv", "d37.blv",
    "mdk01.blv", "mdt01.blv", "mdr01.blv", "mdt10.blv",
    "mdt09.blv", "mdt15.blv", "mdt11.blv"};

bool BLVFace::Deserialize(BLVFace_MM7 *data) {
    memcpy(&this->pFacePlane, &data->pFacePlane, sizeof(this->pFacePlane));
    memcpy(&this->pFacePlane_old, &data->pFacePlane_old,
           sizeof(this->pFacePlane_old));
    this->zCalc1 = data->zCalc1;
    this->zCalc2 = data->zCalc2;
    this->zCalc3 = data->zCalc3;
    this->uAttributes = data->uAttributes;
    this->pVertexIDs = data->pVertexIDs;
    this->pXInterceptDisplacements = data->pXInterceptDisplacements;
    this->pYInterceptDisplacements = data->pYInterceptDisplacements;
    this->pZInterceptDisplacements = data->pZInterceptDisplacements;
    this->pVertexUIDs = data->pVertexUIDs;
    this->pVertexVIDs = data->pVertexVIDs;
    this->uFaceExtraID = data->uFaceExtraID;
    // unsigned __int16  uBitmapID;
    this->uSectorID = data->uSectorID;
    this->uBackSectorID = data->uBackSectorID;
    memcpy(&this->pBounding, &data->pBounding, sizeof(this->pBounding));
    this->uPolygonType = (PolygonType)data->uPolygonType;
    this->uNumVertices = data->uNumVertices;
    this->field_5E = data->field_5E;
    this->field_5F = data->field_5F;

    return true;
}

//----- (0043F39E) --------------------------------------------------------
void PrepareDrawLists_BLV() {
    int TorchLightPower;           // eax@4
    unsigned int v7;  // ebx@8
    BLVSector *v8;    // esi@8

    pBLVRenderParams->Reset();
    pMobileLightsStack->uNumLightsActive = 0;
    // uNumMobileLightsApplied = 0;
    uNumDecorationsDrawnThisFrame = 0;
    uNumSpritesDrawnThisFrame = 0;
    uNumBillboardsToDraw = 0;

    if (!_4D864C_force_sw_render_rules || !engine->config->TorchlightEffect()) {  // lightspot around party
        TorchLightPower = 800;
        if (pParty->TorchlightActive()) {
            // max is 800 * torchlight
            // min is 800
            int MinTorch = TorchLightPower;
            int MaxTorch = TorchLightPower * pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;

            // TorchLightPower *= pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;  // 2,3,4
            int ran = rand();
            int mod = ((ran - (RAND_MAX * .4)) / 200);
            TorchLightPower = (pParty->TorchLightLastIntensity + mod);

            // clamp
            if (TorchLightPower < MinTorch)
                TorchLightPower = MinTorch;
            if (TorchLightPower > MaxTorch)
                TorchLightPower = MaxTorch;
        }

        pParty->TorchLightLastIntensity = TorchLightPower;

        //       double nexLightIntensity(lastIntensity)
        //   return clamp(0, 1, lastIntensity + (rand() - .3) / 100)

        pMobileLightsStack->AddLight(
            pIndoorCameraD3D->vPartyPos.x, pIndoorCameraD3D->vPartyPos.y,
            pIndoorCameraD3D->vPartyPos.z, pBLVRenderParams->uPartySectorID, TorchLightPower,
            floorf(pParty->flt_TorchlightColorR + 0.5f),
            floorf(pParty->flt_TorchlightColorG + 0.5f),
            floorf(pParty->flt_TorchlightColorB + 0.5f), _4E94D0_light_type);
    }

    PrepareBspRenderList_BLV();
    pIndoor->PrepareItemsRenderList_BLV();
    pIndoor->PrepareActorRenderList_BLV();

    for (uint i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
        v7 = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
        v8 = &pIndoor->pSectors[pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i]];

        for (uint j = 0; j < v8->uNumDecorations; ++j)
            pIndoor->PrepareDecorationsRenderList_BLV(v8->pDecorationIDs[j], v7);
    }
    FindBillboardsLightLevels_BLV();
    // engine->PrepareBloodsplats();
}

//----- (004407D9) --------------------------------------------------------
void BLVRenderParams::Reset() {
    this->field_0_timer_ = pEventTimer->uTotalGameTimeElapsed;

    pIndoorCameraD3D->debug_flags = 0;
    if (viewparams->draw_sw_outlines)
        pIndoorCameraD3D->debug_flags |= BLV_RENDER_DRAW_SW_OUTLINES;
    if (viewparams->draw_d3d_outlines)
        pIndoorCameraD3D->debug_flags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    // v2 = a2;
    // this->field_0_timer_ = a2->field_0_timer;
    // this->uFlags = a2->uFlags;
    // this->vPartyPos.x = a2->vPosition.x;
    // this->vPartyPos.y = a2->vPosition.y;
    // this->vPartyPos.z = a2->vPosition.z;
    // v4 = this->vPartyPos.z;
    // v5 = this->vPartyPos.y;
    // this->sPartyRotY = a2->sRotationY;
    // v6 = this->vPartyPos.x;
    // this->sPartyRotX = a2->sRotationX;
    int v7 = pIndoor->GetSector(pIndoorCameraD3D->vPartyPos.x,
                            pIndoorCameraD3D->vPartyPos.y,
                            pIndoorCameraD3D->vPartyPos.z);
    this->uPartySectorID = v7;
    if (!v7) {
        __debugbreak();  // shouldnt happen, please provide savegame
        /*v8 = this->vPartyPos.z;
        this->vPartyPos.x = pParty->vPosition.x;
        v9 = pParty->vPosition.y;
        v10 = this->vPartyPos.x;
        this->vPartyPos.y = pParty->vPosition.y;*/
        // this->uPartySectorID = pIndoor->GetSector(
        //    pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
    }
    // if ( render->pRenderD3D )
    {
        this->fov = pViewport->field_of_view;

        this->uViewportX = pViewport->uScreen_TL_X;
        this->uViewportY = pViewport->uScreen_TL_Y;
        this->uViewportZ = pViewport->uScreen_BR_X;
        this->uViewportW = pViewport->uScreen_BR_Y;

        this->uViewportWidth = uViewportZ - uViewportX + 1;
        this->uViewportHeight = uViewportW - uViewportY + 1;
        this->uViewportCenterX = (uViewportZ + uViewportX) / 2;
        this->uViewportCenterY = (uViewportY + uViewportW) / 2;
    }
    /*else
    {
      __debugbreak(); // no sw
   
    }*/
    // v27 = (unsigned int)(signed __int64)((double)this->uViewportWidth * 0.5
    //                                             / tan((double)(v2->fov_deg >>
    //                                             1) * 0.01745329)
    //                                             + 0.5) << 16;
    extern float _calc_fov(int viewport_width, int angle_degree);
    this->bsp_fov_rad = fixpoint_from_int(_calc_fov(uViewportWidth, 65), 0);
    this->bsp_fov_rad_inv = fixpoint_div(1 << 16, this->bsp_fov_rad);
    this->uTargetWidth = window->GetWidth();
    this->uTargetHeight = window->GetHeight();
    this->pTargetZBuffer = render->pActiveZBuffer;
    this->field_8C = 0;
    this->field_84 = 0;
    this->uNumFacesRenderedThisFrame = 0;
    this->field_88 = 0;
    pBLVRenderParams->field_90 = 64;
    pBLVRenderParams->field_94 = 6;
}

//----- (00440B44) --------------------------------------------------------
void IndoorLocation::ExecDraw(bool bD3D) {
    if (bD3D) {
        // pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);

        for (uint i = 0; i < pBspRenderer->num_faces; ++i) {
            if (pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
                    .viewing_portal_id == -1)
                IndoorLocation::ExecDraw_d3d(pBspRenderer->faces[i].uFaceID,
                                             nullptr, 4, nullptr);
            else
                IndoorLocation::ExecDraw_d3d(
                    pBspRenderer->faces[i].uFaceID,
                    pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
                        .std__vector_0007AC,
                    4,
                    pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
                        .pPortalBounding);
        }
    } else {
        for (uint j = 0; j < pBspRenderer->num_faces; ++j) {
            __debugbreak();  // no SW
            // pBLVRenderParams->field_7C =
            // &pBspRenderer->nodes[pBspRenderer->faces[j].uNodeID].PortalScreenData;
            // IndoorLocation::ExecDraw_sw(pBspRenderer->faces[j].uFaceID);
        }
    }
}

/*
//----- (00440BED) --------------------------------------------------------
void sub_440BED(IndoorLocation_drawstru *_this)
{
  unsigned __int16 *v1; // edi@7
  BspRenderer_stru0 *v2; // esi@8
  int v3; // ecx@9
  unsigned int v4; // edx@9
  short *v5; // eax@10
  signed int v6; // [sp+8h] [bp-8h]@7
  int v7; // [sp+Ch] [bp-4h]@8
  short *v8;

  PrepareDrawLists_BLV(_this);
  if (pBLVRenderParams->uPartySectorID)
    IndoorLocation::ExecDraw(render->pRenderD3D != 0);
  render->DrawBillboardList_BLV();

  if ( !render->pRenderD3D )
  {
    if (pBLVRenderParams->uFlags & INDOOR_CAMERA_DRAW_D3D_OUTLINES)
      pBspRenderer->DrawFaceOutlines();
    if (pBLVRenderParams->uFlags & INDOOR_CAMERA_DRAW_SW_OUTLINES)
    {
      v1 = pBLVRenderParams->pRenderTarget;
          v7 = 0;
          for(int i=0; i < pBspRenderer->num_nodes; i++)
          {
                  BspRenderer_stru0 *pNode = &pBspRenderer->nodes[i];
                  v4 = render->uTargetSurfacePitch *
pNode->PortalScreenData._viewport_space_y; if (
pNode->PortalScreenData._viewport_space_y <=
pNode->PortalScreenData._viewport_space_w )
                  {
                        //v5 = (char
*)&pBspRenderer->nodes[0].field_C.array_3D8[pNode->field_C._viewport_space_y +
v7]; v5 =
&pNode->PortalScreenData.viewport_right_side[pNode->PortalScreenData._viewport_space_y];
                        v8 =
&pNode->PortalScreenData.viewport_left_side[pNode->PortalScreenData._viewport_space_y];
                        do
                        {
                                v1[v4 + *v8] = 255;
                                ++pNode->PortalScreenData._viewport_space_y;
                                v1[v4 + *v5] = 255;
                                v4 += render->uTargetSurfacePitch;
                                ++v5;
                                ++v8;
                        }
                        while ( pNode->PortalScreenData._viewport_space_y <=
pNode->PortalScreenData._viewport_space_w );
                  }
          }
    }
  }
}
*/

//----- (00441BD4) --------------------------------------------------------
void IndoorLocation::Draw() {
    // int v0; // eax@1
    // IndoorLocation_drawstru _this; // [sp+0h] [bp-4Ch]@5
    //  int v2; // [sp+44h] [bp-8h]@5
    //  int v3; // [sp+48h] [bp-4h]@5

    /*_this.uFlags = 0;
    if (viewparams->draw_sw_outlines)
      _this.uFlags |= BLV_RENDER_DRAW_SW_OUTLINES;
    if (viewparams->draw_d3d_outlines)
      _this.uFlags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    _this.uFlags |= BLV_RENDER_DRAW_SW_OUTLINES;
    _this.uFlags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    _this.field_0_timer = pEventTimer->uTotalGameTimeElapsed;
    //_this.fov_deg = 65;
    //_this.vPosition.x = pParty->vPosition.x -
    fixpoint_mul(stru_5C6E00->Cos(pParty->sRotationY),
    pParty->y_rotation_granularity);
    //_this.vPosition.y = pParty->vPosition.y -
    fixpoint_mul(stru_5C6E00->Sin(pParty->sRotationY),
    pParty->y_rotation_granularity);
    //_this.vPosition.z = pParty->vPosition.z + pParty->sEyelevel;
    //_this.sRotationX = pParty->sRotationX;
    //_this.sRotationY = pParty->sRotationY;
    _this.uViewportX = pViewport->uScreen_TL_X;
    _this.uViewportY = pViewport->uScreen_TL_Y;
    _this.uViewportZ = pViewport->uScreen_BR_X;
    _this.uViewportW = pViewport->uScreen_BR_Y;
    _this.field_3C = pViewport->field_30;

    _this.uTargetWidth = 640;
    _this.uTargetHeight = 480;
    _this.pTargetZ = render->pActiveZBuffer;*/

    // sub_440BED(&_this); -- inlined
    //{
    PrepareDrawLists_BLV();
    if (pBLVRenderParams->uPartySectorID)
        ExecDraw(true /*render->pRenderD3D != 0*/);
    render->DrawBillboardList_BLV();
    //}

    pParty->uFlags &= ~2;
    engine->DrawParticles();
    trail_particle_generator.UpdateParticles();
}

//----- (004C0EF2) --------------------------------------------------------
void BLVFace::FromODM(ODMFace *face) {
    this->pFacePlane_old.vNormal.x = face->pFacePlane.vNormal.x;
    this->pFacePlane_old.vNormal.y = face->pFacePlane.vNormal.y;
    this->pFacePlane_old.vNormal.z = face->pFacePlane.vNormal.z;
    this->pFacePlane_old.dist = face->pFacePlane.dist;
    this->pFacePlane.vNormal.x =
        (double)(face->pFacePlane.vNormal.x & 0xFFFF) * 0.000015259022 +
        (double)(face->pFacePlane.vNormal.x >> 16);
    this->pFacePlane.vNormal.y =
        (double)(face->pFacePlane.vNormal.y & 0xFFFF) * 0.000015259022 +
        (double)(face->pFacePlane.vNormal.y >> 16);
    this->pFacePlane.vNormal.z =
        (double)(face->pFacePlane.vNormal.z & 0xFFFF) * 0.000015259022 +
        (double)(face->pFacePlane.vNormal.z >> 16);
    this->pFacePlane.dist =
        (double)(face->pFacePlane.dist & 0xFFFF) * 0.000015259022 +
        (double)(face->pFacePlane.dist >> 16);
    this->uAttributes = face->uAttributes;
    this->pBounding.x1 = face->pBoundingBox.x1;
    this->pBounding.y1 = face->pBoundingBox.y1;
    this->pBounding.z1 = face->pBoundingBox.z1;
    this->pBounding.x2 = face->pBoundingBox.x2;
    this->pBounding.y2 = face->pBoundingBox.y2;
    this->pBounding.z2 = face->pBoundingBox.z2;
    this->zCalc1 = face->zCalc1;
    this->zCalc2 = face->zCalc2;
    this->zCalc3 = face->zCalc3;
    this->pXInterceptDisplacements = face->pXInterceptDisplacements;
    this->pYInterceptDisplacements = face->pYInterceptDisplacements;
    this->pZInterceptDisplacements = face->pZInterceptDisplacements;
    this->uPolygonType = (PolygonType)face->uPolygonType;
    this->uNumVertices = face->uNumVertices;
    this->resource = face->resource;
    this->pVertexIDs = face->pVertexIDs;
}

//----- (004B0A25) --------------------------------------------------------
void IndoorLocation::ExecDraw_d3d(unsigned int uFaceID,
                                  IndoorCameraD3D_Vec4 *pVertices,
                                  unsigned int uNumVertices,
                                  RenderVertexSoft *pPortalBounding) {
    int ColourMask;  // ebx@25
    // IDirect3DTexture2 *v27; // eax@42
    unsigned int uNumVerticesa;  // [sp+24h] [bp-4h]@17
    int LightLevel;                     // [sp+34h] [bp+Ch]@25

    if (uFaceID >= pIndoor->uNumFaces)
        return;

    static RenderVertexSoft static_vertices_buff_in[64];  // buff in
    static RenderVertexSoft static_vertices_calc_out[64];  // buff out - calc portal shape

    static stru154 FacePlaneHolder;  // idb


    BLVFace *pFace = &pIndoor->pFaces[uFaceID];
    if (pFace->uNumVertices < 3) return;

    if (pFace->Invisible()) {
        return;
    }

    ++pBLVRenderParams->uNumFacesRenderedThisFrame;
    pFace->uAttributes |= FACE_RENDERED;

    if (!pFace->GetTexture()) {
        return;
    }

    if (!pIndoorCameraD3D->IsCulled(pFace)) {
        uNumVerticesa = pFace->uNumVertices;

        // copy to buff in
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            static_vertices_buff_in[i].vWorldPosition.x =
                pIndoor->pVertices[pFace->pVertexIDs[i]].x;
            static_vertices_buff_in[i].vWorldPosition.y =
                pIndoor->pVertices[pFace->pVertexIDs[i]].y;
            static_vertices_buff_in[i].vWorldPosition.z =
                pIndoor->pVertices[pFace->pVertexIDs[i]].z;
            static_vertices_buff_in[i].u = (signed short)pFace->pVertexUIDs[i];
            static_vertices_buff_in[i].v = (signed short)pFace->pVertexVIDs[i];
        }

        // 498377 always true - appears to be anothe function to clip vertices to portal planes??
        if (!pVertices || (engine->pStru9Instance->_498377(
                               pPortalBounding, 4, pVertices,
                               static_vertices_buff_in, &uNumVerticesa),
                           uNumVerticesa)) {
            if (pIndoorCameraD3D->CalcPortalShape(  // clips vertices to the frustum planes
                    static_vertices_buff_in, &uNumVerticesa,
                    static_vertices_calc_out,
                    pIndoorCameraD3D->std__vector_000034_prolly_frustrum, 4,
                    false, 0) != 1 || uNumVerticesa) {
                LightLevel = HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel);
                ColourMask =
                    (248 -
                     (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel) << 3)) |
                    (((248 - (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel)
                              << 3)) |
                      ((248 - (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel)
                               << 3))
                       << 8))
                     << 8);

                FaceFlowTextureOffset(uFaceID);


                // if (uFaceID == 2215 && pFace->uAttributes & FACE_OUTLINED) __debugbreak();
                // ToDo: restore this
                // if (pFace->uAttributes & FACE_OUTLINED) {
                    lightmap_builder->ApplyLights_IndoorFace(uFaceID);
                //}

                // if (Lights.uNumLightsApplied > 0) __debugbreak();

                decal_builder->ApplyBloodsplatDecals_IndoorFace(uFaceID);

                pIndoorCameraD3D->ViewTransfrom_OffsetUV(static_vertices_calc_out, uNumVerticesa, array_507D30, &Lights);

                pIndoorCameraD3D->Project(array_507D30, uNumVerticesa, 0);

                lightmap_builder->StationaryLightsCount = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumDecals > 0) {
                    FacePlaneHolder.face_plane.vNormal.x =
                        pFace->pFacePlane.vNormal.x;
                    FacePlaneHolder.polygonType = pFace->uPolygonType;
                    FacePlaneHolder.face_plane.vNormal.y =
                        pFace->pFacePlane.vNormal.y;
                    FacePlaneHolder.face_plane.vNormal.z =
                        pFace->pFacePlane.vNormal.z;
                    FacePlaneHolder.face_plane.dist = pFace->pFacePlane.dist;
                }

                if (Lights.uNumLightsApplied > 0 && !pFace->Indoor_sky())  // for torchlight(для света факелов)
                    // if (pFace->uAttributes & FACE_OUTLINED) {
                        lightmap_builder->ApplyLights(&Lights, &FacePlaneHolder, uNumVerticesa, array_507D30, pVertices, 0);
                    //}

                // bool LightmapBuilder::ApplyLights(LightsData *pLights, stru154 *a3, unsigned int uNumVertices,
               // RenderVertexSoft *VertexRenderList, IndoorCameraD3D_Vec4 *a6, char uClipFlag) {

                if (decal_builder->uNumDecals > 0)  // blood draw
                    decal_builder->ApplyDecals(LightLevel, 1, &FacePlaneHolder,
                                               uNumVerticesa, array_507D30,
                                               pVertices, 0, pFace->uSectorID);

                Texture *face_texture = pFace->GetTexture();
                if (pFace->Fluid()) {
                    // if (pFace->uBitmapID == render->hd_water_tile_id)
                    //    v27 =
                    //    pBitmaps_LOD->pHardwareTextures[render->pHDWaterBitmapIDs[render->hd_water_current_frame]];
                    // else
                    {
                        face_texture = (Texture *)pFace->resource;
                        // auto v24 = GetTickCount() / 4;
                        // auto v25 = v24 - stru_5C6E00->uIntegerHalfPi;
                        uint eightSeconds = OS_GetTime() % 8000;
                        float angle = (eightSeconds / 8000.0f) * 2 * 3.1415f;

                        // animte lava back and forth
                        for (uint i = 0; i < uNumVerticesa; ++i)
                            // array_507D30[i].v +=
                            // (double)(pBitmaps_LOD->pTextures[pFace->uBitmapID].uHeightMinus1
                            // & (unsigned int)(stru_5C6E00->SinCos(v25) >> 8));
                            array_507D30[i].v +=
                                (face_texture->GetHeight() - 1) * cosf(angle);
                    }
                } else if (pFace->IsTextureFrameTable()) {
                    face_texture = pTextureFrameTable->GetFrameTexture(
                        (int)pFace->resource, pBLVRenderParams->field_0_timer_);
                } else {
                    ColourMask = 0xFF808080;
                    // v27 = pBitmaps_LOD->pHardwareTextures[pFace->uBitmapID];
                }

                if (pFace->Indoor_sky()) {
                    render->DrawIndoorSky(uNumVerticesa, uFaceID);
                } else {
                   // if (pFace->uAttributes & FACE_OUTLINED) {
                        render->DrawIndoorPolygon(uNumVerticesa, pFace, PID(OBJECT_BModel, uFaceID), ColourMask, 0);
                    //}
                }
                return;
            }
        }
    }
}

//----- (004B0E07) --------------------------------------------------------
unsigned int FaceFlowTextureOffset(unsigned int uFaceID) {  // time texture offset
    Lights.pDeltaUV[0] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaU;
    Lights.pDeltaUV[1] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaV;

    unsigned int offset = OS_GetTime() >> 3;

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_DIAGONAL) {
        Lights.pDeltaUV[1] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);  // pBitmaps_LOD->GetTexture(pIndoor->pFaces[uFaceID].uBitmapID)->uHeightMinus1;
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_VERTICAL) {
        Lights.pDeltaUV[1] += offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);  // pBitmaps_LOD->GetTexture(pIndoor->pFaces[uFaceID].uBitmapID)->uHeightMinus1;
    }

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_HORIZONTAL) {
        Lights.pDeltaUV[0] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetWidth() - 1);  // pBitmaps_LOD->GetTexture(pIndoor->pFaces[uFaceID].uBitmapID)->uWidthMinus1;
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_DONT_CACHE_TEXTURE) {
        Lights.pDeltaUV[0] += offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetWidth() - 1);  // pBitmaps_LOD->GetTexture(pIndoor->pFaces[uFaceID].uBitmapID)->uWidthMinus1;
    }

    return offset;
}

//----- (004B0EA8) --------------------------------------------------------
void BspRenderer::AddFaceToRenderList_d3d(unsigned int node_id,
                                          unsigned int uFaceID) {
    unsigned __int16 pTransitionSector;  // ax@11
    int v9;                              // edx@15
    char v29;                            // al@48

    nodes[num_nodes].viewing_portal_id = -1;
    // v39 = &pIndoor->pFaces[uFaceID];

    BLVFace *pFace = &pIndoor->pFaces[uFaceID];

    if (!pFace->Portal()) {
        if (num_faces < 1000) {
            faces[num_faces].uFaceID = uFaceID;
            faces[num_faces++].uNodeID = node_id;
        }
        return;
    }

    // portals are invisible faces marking the transition between sectors

    if (nodes[node_id].uFaceID == uFaceID) return;
    if (!node_id &&
        pIndoorCameraD3D->vPartyPos.x >=
            pFace->pBounding.x1 -
                16 &&  // we are probably standing at the portal plane
        pIndoorCameraD3D->vPartyPos.x <= pFace->pBounding.x2 + 16 &&
        pIndoorCameraD3D->vPartyPos.y >= pFace->pBounding.y1 - 16 &&
        pIndoorCameraD3D->vPartyPos.y <= pFace->pBounding.y2 + 16 &&
        pIndoorCameraD3D->vPartyPos.z >= pFace->pBounding.z1 - 16 &&
        pIndoorCameraD3D->vPartyPos.z <= pFace->pBounding.z2 + 16) {
        if (abs(pFace->pFacePlane_old.dist +
                pIndoorCameraD3D->vPartyPos.x *
                    pFace->pFacePlane_old.vNormal.x +
                pIndoorCameraD3D->vPartyPos.y *
                    pFace->pFacePlane_old.vNormal.y +
                pIndoorCameraD3D->vPartyPos.z *
                    pFace->pFacePlane_old.vNormal.z) <=
            589824) {  // we sure are standing at the portal plane
            pTransitionSector = pFace->uSectorID;
            if (nodes[0].uSectorID == pTransitionSector)  // draw back sector
                pTransitionSector = pFace->uBackSectorID;
            nodes[num_nodes].uSectorID = pTransitionSector;
            nodes[num_nodes].uFaceID = uFaceID;
            nodes[num_nodes].uViewportX = pBLVRenderParams->uViewportX;
            nodes[num_nodes].uViewportZ = pBLVRenderParams->uViewportZ;
            nodes[num_nodes].uViewportY = pBLVRenderParams->uViewportY;
            nodes[num_nodes].uViewportW = pBLVRenderParams->uViewportW;
            nodes[num_nodes].PortalScreenData.GetViewportData(
                pBLVRenderParams->uViewportX, pBLVRenderParams->uViewportY,
                pBLVRenderParams->uViewportZ, pBLVRenderParams->uViewportW);
            AddBspNodeToRenderList(++num_nodes - 1);
            return;
        }
    }

    v9 = pFace->pFacePlane_old.vNormal.x *
             (pIndoor->pVertices[pFace->pVertexIDs[0]].x -
              pIndoorCameraD3D->vPartyPos.x) +
         pFace->pFacePlane_old.vNormal.y *
             (pIndoor->pVertices[pFace->pVertexIDs[0]].y -
              pIndoorCameraD3D->vPartyPos.y) +
         pFace->pFacePlane_old.vNormal.z *
             (pIndoor->pVertices[pFace->pVertexIDs[0]].z -
              pIndoorCameraD3D->vPartyPos.z);
    if (nodes[node_id].uSectorID != pFace->uSectorID) v9 = -v9;
    if (v9 >= 0) return;

    // check number of verts of portal is seen by camera
    int num_vertices = GetPortalScreenCoord(uFaceID);
    if (num_vertices < 2) return;

    int face_min_screenspace_x = PortalFace._screen_space_x[0],
        face_max_screenspace_x = PortalFace._screen_space_x[0];
    int face_min_screenspace_y = PortalFace._screen_space_y[0],
        face_max_screenspace_y = PortalFace._screen_space_y[0];
    for (uint i = 1; i < num_vertices; ++i) {
        if (face_min_screenspace_x > PortalFace._screen_space_x[i])
            face_min_screenspace_x = PortalFace._screen_space_x[i];
        if (face_max_screenspace_x < PortalFace._screen_space_x[i])
            face_max_screenspace_x = PortalFace._screen_space_x[i];

        if (face_min_screenspace_y > PortalFace._screen_space_y[i])
            face_min_screenspace_y = PortalFace._screen_space_y[i];
        if (face_max_screenspace_y < PortalFace._screen_space_y[i])
            face_max_screenspace_y = PortalFace._screen_space_y[i];
    }
    // _screen_space_x = 719, 568, 493
    // savegame: qw , 0Bh and 0x1D4h
    // problem here when standing near/on portal, condition is false because of
    // face_min_screenspace_x > p->uViewportZ
    if (face_max_screenspace_x >= nodes[node_id].uViewportX &&
        face_min_screenspace_x <= nodes[node_id].uViewportZ &&
        face_max_screenspace_y >= nodes[node_id].uViewportY &&
        face_min_screenspace_y <= nodes[node_id].uViewportW &&
        PortalFrustrum(num_vertices, &nodes[num_nodes].PortalScreenData,
                       &nodes[node_id].PortalScreenData, uFaceID)) {
        // current portal visible through previous

        pTransitionSector = pFace->uSectorID;
        if (nodes[node_id].uSectorID == pTransitionSector)
            pTransitionSector = pFace->uBackSectorID;
        nodes[num_nodes].uSectorID = pTransitionSector;
        nodes[num_nodes].uFaceID = uFaceID;
        nodes[num_nodes].uViewportX = pBLVRenderParams->uViewportX;
        nodes[num_nodes].uViewportZ = pBLVRenderParams->uViewportZ;
        nodes[num_nodes].uViewportY = pBLVRenderParams->uViewportY;
        nodes[num_nodes].uViewportW = pBLVRenderParams->uViewportW;
        v29 = false;
        if (nodes[node_id].viewing_portal_id == -1) {  // for first portal
            v29 = engine->pStru10Instance->CalcPortalShape(
                pFace, nodes[num_nodes].std__vector_0007AC,
                nodes[num_nodes].pPortalBounding);
        } else {  // for next portals
            static RenderVertexSoft static_subAddFaceToRenderList_d3d_stru_F7AA08[64];
            static RenderVertexSoft static_subAddFaceToRenderList_d3d_stru_F79E08[64];

            for (uint k = 0; k < pFace->uNumVertices; ++k) {
                static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.x =
                    pIndoor->pVertices[pFace->pVertexIDs[k]].x;
                static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.y =
                    pIndoor->pVertices[pFace->pVertexIDs[k]].y;
                static_subAddFaceToRenderList_d3d_stru_F7AA08[k].vWorldPosition.z =
                    pIndoor->pVertices[pFace->pVertexIDs[k]].z;
            }

            unsigned int pNewNumVertices = pFace->uNumVertices;
            pIndoorCameraD3D->CalcPortalShape(
                static_subAddFaceToRenderList_d3d_stru_F7AA08, &pNewNumVertices,
                static_subAddFaceToRenderList_d3d_stru_F79E08,
                nodes[node_id].std__vector_0007AC, 4, 0, 0);

            v29 = engine->pStru10Instance->_49C5DA(
                pFace, static_subAddFaceToRenderList_d3d_stru_F79E08,
                &pNewNumVertices, nodes[num_nodes].std__vector_0007AC,
                nodes[num_nodes].pPortalBounding);
        }

        if (1) {
            assert(num_nodes < 150);

            // add portal sector to drawing list

            nodes[num_nodes].viewing_portal_id = uFaceID;
            AddBspNodeToRenderList(++num_nodes - 1);
        }

        if (pIndoorCameraD3D->debug_flags & BLV_RENDER_DRAW_SW_OUTLINES)
            pIndoorCameraD3D->PrepareAndDrawDebugOutline(pFace, 0x1E1EFF);
        // pIndoorCameraD3D->DebugDrawPortal(pFace);
    }
}

//----- (004AE5BA) --------------------------------------------------------
Texture *BLVFace::GetTexture() {
    if (this->IsTextureFrameTable())
        return pTextureFrameTable->GetFrameTexture(
            (int)this->resource, pBLVRenderParams->field_0_timer_);
    else
        return (Texture *)this->resource;
}

void BLVFace::SetTexture(const String &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource =
            (void *)pTextureFrameTable->FindTextureByName(filename.c_str());
        if (this->resource != (void *)-1) {
            return;
        }

        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->GetBitmap(filename);
}

//----- (00498B15) --------------------------------------------------------
void IndoorLocation::Release() {
    free(this->ptr_0002B4_doors_ddata);
    this->ptr_0002B4_doors_ddata = NULL;

    free(this->ptr_0002B0_sector_rdata);
    this->ptr_0002B0_sector_rdata = NULL;

    free(this->ptr_0002B8_sector_lrdata);
    this->ptr_0002B8_sector_lrdata = NULL;

    free(this->pLFaces);
    this->pLFaces = NULL;

    free(this->pSpawnPoints);
    this->pSpawnPoints = NULL;

    this->uNumSectors = 0;
    this->uNumFaces = 0;
    this->uNumVertices = 0;
    this->uNumNodes = 0;
    this->uNumDoors = 0;
    this->uNumLights = 0;

    free(this->pVertices);
    this->pVertices = NULL;

    free(this->pFaces);
    this->pFaces = NULL;

    free(this->pFaceExtras);
    this->pFaceExtras = NULL;

    free(this->pSectors);
    this->pSectors = NULL;

    free(this->pLights);
    this->pLights = NULL;

    free(this->pDoors);
    this->pDoors = NULL;

    free(this->pNodes);
    this->pNodes = NULL;

    free(this->pMapOutlines);
    this->pMapOutlines = NULL;

    this->bLoaded = 0;
}

//----- (00498C45) --------------------------------------------------------
bool IndoorLocation::Alloc() {
    pVertices = (Vec3_short_ *)malloc(15000 * sizeof(Vec3_short_));  // 0x15F90u
    pFaces = (BLVFace *)malloc(10000 * sizeof(BLVFace));             // 0xEA600u
    pFaceExtras =
        (BLVFaceExtra *)malloc(5000 * sizeof(BLVFaceExtra));     // 0x2BF20u
    pSectors = (BLVSector *)malloc(512 * sizeof(BLVSector));     // 0xE800u
    pLights = (BLVLightMM7 *)malloc(400 * sizeof(BLVLightMM7));  // 0x1900u
    pDoors = (BLVDoor *)malloc(200 * sizeof(BLVDoor));           // 0x3E80u
    pNodes = (BSPNode *)malloc(5000 * sizeof(BSPNode));          // 0x9C40u
    pMapOutlines = (BLVMapOutlines *)malloc(sizeof(BLVMapOutlines));  // 0x14824u
    if (pVertices && pFaces && pFaceExtras && pSectors && pLights && pDoors &&
        pNodes && pMapOutlines) {
        memset(pVertices, 0, 15000 * sizeof(Vec3_short_));
        memset(pFaces, 0, 10000 * sizeof(BLVFace));
        memset(pFaceExtras, 0, 5000 * sizeof(BLVFaceExtra));
        memset(pSectors, 0, 512 * sizeof(BLVSector));
        memset(pLights, 0, 400 * sizeof(BLVLightMM7));
        memset(pDoors, 0, 200 * sizeof(BLVDoor));
        memset(pNodes, 0, 5000 * sizeof(BSPNode));
        memset(pMapOutlines, 0, sizeof(BLVMapOutlines));
        return true;
    } else {
        return false;
    }
}

//----- (00444810) --------------------------------------------------------
unsigned int IndoorLocation::GetLocationIndex(const char *Str1) {
    for (uint i = 0; i < 11; ++i)
        if (!_stricmp(Str1, _4E6BDC_loc_names[i])) return i + 1;
    return 0;
}

//----- (004488F7) --------------------------------------------------------
void IndoorLocation::ToggleLight(signed int sLightID, unsigned int bToggle) {
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor &&
        (sLightID <= pIndoor->uNumLights - 1) && (sLightID >= 0)) {
        if (bToggle)
            pIndoor->pLights[sLightID].uAtributes &= 0xFFFFFFF7;
        else
            pIndoor->pLights[sLightID].uAtributes |= 8;
        pParty->uFlags |= 2;
    }
}

//----- (00498E0A) --------------------------------------------------------
bool IndoorLocation::Load(const String &filename, int num_days_played,
                          int respawn_interval_days, char *pDest) {
    decal_builder->Reset(0);

    _6807E0_num_decorations_with_sounds_6807B8 = 0;

    if (bLoaded) {
        log->Warning(L"BLV is already loaded");
        return true;
    }

    auto blv_filename = String(filename);
    blv_filename.replace(blv_filename.length() - 4, 4, ".blv");

    this->filename = String(filename);
    if (!pGames_LOD->DoesContainerExist(blv_filename)) {
        Error("Unable to find %s in Games.LOD", blv_filename.c_str());
    }

    Release();
    if (!Alloc())
        return false;

    size_t blv_size = 0;
    void *rawData = pGames_LOD->LoadCompressed(blv_filename, &blv_size);
    char *pData = (char*)rawData;

    bLoaded = true;

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&blv, pData, sizeof(BLVHeader));
    pData += sizeof(BLVHeader);
    memcpy(&uNumVertices, pData, 4);
    pData += 4;
    memcpy(pVertices, pData, uNumVertices * sizeof(Vec3_short_));

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumFaces, pData += uNumVertices * sizeof(Vec3_short_), 4);
    pData += 4;

    pGameLoadingUI_ProgressBar->Progress();

    // memcpy(pFaces, pData, uNumFaces * sizeof(BLVFace));
    auto face_data = (BLVFace_MM7 *)pData;
    pFaces = new BLVFace[uNumFaces];
    for (unsigned int i = 0; i < uNumFaces; ++i) {
        pFaces[i].Deserialize(face_data);
        face_data++;
    }

    pLFaces = (unsigned __int16 *)malloc(blv.uFaces_fdata_Size);
    memcpy(pLFaces, pData += uNumFaces * sizeof(BLVFace_MM7),
           blv.uFaces_fdata_Size);

    for (uint i = 0, j = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];

        pFace->pVertexIDs = &pLFaces[j];

        j += pFace->uNumVertices + 1;
        pFace->pXInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pYInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pZInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexUIDs = (__int16 *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexVIDs = (__int16 *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
    }

    pGameLoadingUI_ProgressBar->Progress();

    pData += blv.uFaces_fdata_Size;

    for (uint i = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];

        char pTexName[16];
        strncpy(pTexName, pData, 10);
        pData += 10;

        pFace->SetTexture(String(pTexName));
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumFaceExtras, pData, 4);
    memcpy(pFaceExtras, pData += 4, uNumFaceExtras * sizeof(BLVFaceExtra));
    pData += uNumFaceExtras * sizeof(BLVFaceExtra);

    pGameLoadingUI_ProgressBar->Progress();

    // v108 = (char *)v107 + 36 * uNumFaceExtras;
    // v245 = 0;
    // *(int *)((char *)&uSourceLen + 1) = 0;
    for (uint i = 0; i < uNumFaceExtras; ++i) {
        char pTexName[32];
        strncpy(pTexName, pData, 10);
        pData += 10;

        if (!strcmp(pTexName, ""))
            pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            pFaceExtras[i].uAdditionalBitmapID =
                pBitmaps_LOD->LoadTexture(pTexName);
    }

    for (uint i = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];
        BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSectors, pData, 4);
    memcpy(pSectors, pData + 4, uNumSectors * sizeof(BLVSector));
    pData += 4 + uNumSectors * sizeof(BLVSector);

    pGameLoadingUI_ProgressBar->Progress();

    ptr_0002B0_sector_rdata =
        (unsigned short *)malloc(blv.uSector_rdata_Size);  //, "L.RData");
    memcpy(ptr_0002B0_sector_rdata, pData, blv.uSector_rdata_Size);
    pData += blv.uSector_rdata_Size;

    for (uint i = 0, j = 0; i < uNumSectors; ++i) {
        BLVSector *pSector = &pSectors[i];

        pSector->pFloors = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFloors;

        pSector->pWalls = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumWalls;

        pSector->pCeilings = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumCeilings;

        pSector->pFluids = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFluids;

        pSector->pPortals = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumPortals;

        pSector->pFaceIDs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFaces;

        pSector->pCogs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumCogs;

        pSector->pDecorationIDs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumDecorations;

        pSector->pMarkers = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumMarkers;
    }

    ptr_0002B8_sector_lrdata =
        (unsigned __int16 *)malloc(blv.uSector_lrdata_Size);  //, "L.RLData");
    memcpy(ptr_0002B8_sector_lrdata, pData, blv.uSector_lrdata_Size);
    pData += blv.uSector_lrdata_Size;

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0, j = 0; i < uNumSectors; ++i) {
        pSectors[i].pLights = ptr_0002B8_sector_lrdata + j;
        j += pSectors[i].uNumLights;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumDoors, pData, 4);
    pData += 4;

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumLevelDecorations, pData, 4);
    memcpy(pLevelDecorations.data(), pData + 4,
           uNumLevelDecorations * sizeof(LevelDecoration));
    pData += 4 + uNumLevelDecorations * sizeof(LevelDecoration);

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        pLevelDecorations[i].uDecorationDescID =
            pDecorationList->GetDecorIdByName(pData);

        pData += 32;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumLights, pData, 4);
    memcpy(pLights, pData + 4, uNumLights * sizeof(BLVLightMM7));
    pData += 4 + uNumLights * sizeof(BLVLightMM7);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumNodes, pData, 4);
    memcpy(pNodes, pData + 4, uNumNodes * sizeof(BSPNode));
    pData += 4 + uNumNodes * sizeof(BSPNode);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSpawnPoints, pData, 4);
    pSpawnPoints = (SpawnPointMM7 *)malloc(uNumSpawnPoints * sizeof(SpawnPointMM7));
    memcpy(pSpawnPoints, pData + 4, uNumSpawnPoints * sizeof(SpawnPointMM7));
    pData += 4 + uNumSpawnPoints * sizeof(SpawnPointMM7);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&pMapOutlines->uNumOutlines, pData, 4);
    memcpy(pMapOutlines->pOutlines, pData + 4, pMapOutlines->uNumOutlines * sizeof(BLVMapOutline));
    free(rawData);

    String dlv_filename = String(filename);
    dlv_filename.replace(dlv_filename.length() - 4, 4, ".dlv");

    bool _v244 = false;
    size_t dlv_size = 0;
    rawData = pGames_LOD->LoadCompressed(dlv_filename, &dlv_size);
    if (rawData != nullptr) {
        pData = (char*)rawData;
        memcpy(&dlv, pData, sizeof(DDM_DLV_Header));
        pData += sizeof(DDM_DLV_Header);
    } else {
        _v244 = true;
    }

    if (dlv.uNumFacesInBModels > 0) {
        if (dlv.uNumDecorations > 0) {
            if (dlv.uNumFacesInBModels != uNumFaces ||
                dlv.uNumDecorations != uNumLevelDecorations)
                _v244 = true;
        }
    }

    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_2000) {
        respawn_interval_days = 0x1BAF800;
    }

    bool _a = false;
    if (num_days_played - dlv.uLastRepawnDay >= respawn_interval_days &&
        (pCurrentMapName != "d29.dlv")) {
        _a = true;
    }

    char v203[875];
    if (_v244 || (_a || !dlv.uLastRepawnDay)) {
        if (_v244) {
            memset(v203, 0, 875);
        } else if (_a || !dlv.uLastRepawnDay) {
            memcpy(v203, pData, 875);
        }

        dlv.uLastRepawnDay = num_days_played;
        if (_v244) ++dlv.uNumRespawns;
        *(int *)pDest = 1;

        pData = (char*)pGames_LOD->LoadCompressed(dlv_filename);
        pData += sizeof(DDM_DLV_Header);
    } else {
        *(int*)pDest = 0;
    }

    memcpy(_visible_outlines, pData, 875);
    pData += 875;

    if (*(int *)pDest) memcpy(_visible_outlines, v203, 875);

    for (uint i = 0; i < pMapOutlines->uNumOutlines; ++i) {
        BLVMapOutline *pVertex = &pMapOutlines->pOutlines[i];
        if ((unsigned __int8)(1 << (7 - i % 8)) & _visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    for (uint i = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];
        BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

        memcpy(&pFace->uAttributes, pData, 4);
        pData += 4;

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        memcpy(&pLevelDecorations[i].uFlags, pData, 2);
        pData += 2;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumActors, pData, 4);
    memcpy(&pActors, pData + 4, uNumActors * sizeof(Actor));
    pData += 4 + uNumActors * sizeof(Actor);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSpriteObjects, pData, 4);
    memcpy(pSpriteObjects.data(), pData + 4,
           uNumSpriteObjects * sizeof(SpriteObject));
    pData += 4 + uNumSpriteObjects * sizeof(SpriteObject);

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].containing_item.uItemID && !(pSpriteObjects[i].uAttributes & 0x0100)) {
            pSpriteObjects[i].uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[pSpriteObjects[i].containing_item.uItemID].uSpriteID;
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    pData = ChestsDeserialize(pData);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(pDoors, pData, 0x3E80);
    pData += 0x3E80;

    // v201 = (const char *)blv.uDoors_ddata_Size;
    // v200 = (size_t)ptr_0002B4_doors_ddata;
    // v170 = malloc(ptr_0002B4_doors_ddata, blv.uDoors_ddata_Size, "L.DData");
    // v171 = blv.uDoors_ddata_Size;
    ptr_0002B4_doors_ddata = (uint16_t*)malloc(blv.uDoors_ddata_Size);  //, "L.DData");
    memcpy(ptr_0002B4_doors_ddata, pData, blv.uDoors_ddata_Size);
    pData += blv.uDoors_ddata_Size;

    // Src = (BLVFace *)((char *)Src + v171);
    // v172 = 0;
    // v245 = 0;
    // if (uNumDoors > 0)
    for (uint i = 0, j = 0; i < uNumDoors; ++i) {
        BLVDoor *pDoor = &pDoors[i];

        pDoor->pVertexIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumVertices;

        pDoor->pFaceIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumFaces;

        pDoor->pSectorIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->field_48;

        pDoor->pDeltaUs = (short *)(&ptr_0002B4_doors_ddata[j]);
        j += pDoor->uNumFaces;

        pDoor->pDeltaVs = (short *)(&ptr_0002B4_doors_ddata[j]);
        j += pDoor->uNumFaces;

        pDoor->pXOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;

        pDoor->pYOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;

        pDoor->pZOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;
    }
    // v190 = 0;
    // v245 = 0;
    for (uint i = 0; i < uNumDoors; ++i) {
        BLVDoor *pDoor = &pDoors[i];

        for (uint j = 0; j < pDoor->uNumFaces; ++j) {
            BLVFace *pFace = &pFaces[pDoor->pFaceIDs[j]];
            BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

            pDoor->pDeltaUs[j] = pFaceExtra->sTextureDeltaU;
            pDoor->pDeltaVs[j] = pFaceExtra->sTextureDeltaV;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&stru_5E4C90_MapPersistVars, pData, 0xC8);
    pData += 0xC8;

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&stru1, pData, 0x38u);
    pData += 0x38;

    return 0;
}

int IndoorLocation::GetSector(int sX, int sY, int sZ) {
    // sx = partyx..

    int v25;        // edx@21
    int v26;        // ebx@23

    int pSectorID;  // ebx@40
    int v39;        // eax@41
    int FoundFaceStore[50];    // [sp+Ch] [bp-108h]@1
    bool Vert2AboveParty;       // [sp+ECh] [bp-28h]@19

    int v53;        // [sp+F8h] [bp-1Ch]@10
    int VertsPassChecks;        // [sp+FCh] [bp-18h]@16


    FoundFaceStore[0] = 0;

    int NumFoundFaceStore = 0;

    if (uNumSectors < 2) return 0;

    for (uint i = 1; i < uNumSectors; ++i) {
        BLVSector *pSector = &pSectors[i];

        if (pSector->pBounding.x1 > sX || pSector->pBounding.x2 < sX ||
            pSector->pBounding.y1 > sY || pSector->pBounding.y2 < sY ||
            pSector->pBounding.z1 - 64 > sZ || pSector->pBounding.z2 + 64 < sZ)
            continue;  // outside sector

        // logger->Warning(L"Sector[%u]", i);
         int FloorsAndPortals = pSector->uNumFloors + pSector->uNumPortals;
        if (!FloorsAndPortals) continue;

        for (uint j = 0; j < FloorsAndPortals; ++j) {
            uint uFaceID;
            if (j < pSector->uNumFloors)
                uFaceID = pSector->pFloors[j];
            else
                uFaceID = pSector->pPortals[j - pSector->uNumFloors];

            BLVFace *pFace = &pFaces[uFaceID];
            if (pFace->uPolygonType != POLYGON_Floor &&
                pFace->uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            VertsPassChecks = 0;
            Vert2AboveParty = pVertices[pFace->pVertexIDs[0]].y >= sY;

            for (uint k = 1; k <= pFace->uNumVertices; k++) {
                bool Vert1AboveParty = Vert2AboveParty;

                if (VertsPassChecks >= 2) break;

                Vec3<int16_t> *v2 = &pVertices[pFace->pVertexIDs[k]];
                Vert2AboveParty = v2->y >= sY;

                if (Vert1AboveParty == Vert2AboveParty) continue;

                Vec3<int16_t> *v1 = &pVertices[pFace->pVertexIDs[k - 1]];
                v25 = v2->x >= sX ? 0 : 2;
                v26 = v25 | (v1->x < sX);

                if (v26 == 3) continue;  // v1 and v2 less than x

                if (!v26) {
                    ++VertsPassChecks;  // both greater than x
                } else {
                    if (v1->x >= v2->x) {
                        /*int _a58;
                        int _a59;

                        v32 = v1->x - v2->x;
                        LODWORD(v33) = v32 << 16;
                        HIDWORD(v33) = v32 >> 16;*/
                        // fixpoint_div(v1->x - v2->x, v1->y - v2->y);
                        // _a58 = v33 / (v1->y - v2->y);
                        // _a59 = fixpoint_mul(_a58, sY - v2->y);
                        long long x_div_y =
                            fixpoint_div(v1->x - v2->x, v1->y - v2->y);
                        long long res = fixpoint_mul(
                            x_div_y,
                            sY - v2->y);  // a / b * c  - looks like projection
                        if (res + v2->x > sX) ++VertsPassChecks;
                    } else {
                        long long x_div_y =
                            fixpoint_div(v2->x - v1->x, v2->y - v1->y);
                        long long res = fixpoint_mul(x_div_y, sY - v1->y);

                        if (res + v1->x > sX) ++VertsPassChecks;

                        /*int _a58;
                        int _a59;
                        auto v32 = v2->x - v1->x;
                        LODWORD(v33) = v32 << 16;
                        HIDWORD(v33) = v32 >> 16;
                        _a58 = v33 / (v2->y - v1->y);
                        _a59 = fixpoint_mul(_a58, sY - v1->y);

                        if (_a59 + pVertices[k].x > sX)
                          ++VertsPassChecks;*/
                    }
                }
            }

            if (pFace->uNumVertices && VertsPassChecks == 1) FoundFaceStore[NumFoundFaceStore++] = uFaceID;
        }
    }

    // v4 = FoundFaceStore[0];
    if (NumFoundFaceStore == 1) return this->pFaces[/*v4*/FoundFaceStore[0]].uSectorID;

    if (!NumFoundFaceStore) return 0;

    // when multiple possibilities are found - cycle through and use the closer one to party
    pSectorID = 0;
    v53 = 0xFFFFFFu;
    if (NumFoundFaceStore > 0) {
        v39 = sY;
        for (int v37 = 0; v37 < NumFoundFaceStore; ++v37) {
            if (this->pFaces[FoundFaceStore[v37]].uPolygonType == POLYGON_Floor)
                v39 =
                    sZ - this->pVertices[*this->pFaces[FoundFaceStore[v37]].pVertexIDs].z;
            if (this->pFaces[FoundFaceStore[v37]].uPolygonType ==
                POLYGON_InBetweenFloorAndWall) {
                v39 =
                    sZ -
                    ((fixpoint_mul(this->pFaces[FoundFaceStore[v37]].zCalc1, (sX << 16)) +
                      fixpoint_mul(this->pFaces[FoundFaceStore[v37]].zCalc2, (sY << 16)) +
                      this->pFaces[FoundFaceStore[v37]].zCalc3 + 0x8000) >>
                     16);
            }
            if (v39 >= 0) {
                if (v39 < v53) {
                    pSectorID = this->pFaces[FoundFaceStore[v37]].uSectorID;
                    v53 = v39;
                }
            }
        }
    }
    return pSectorID;
}

//----- (00498A41) --------------------------------------------------------
void BLVFace::_get_normals(Vec3_int_ *a2, Vec3_int_ *a3) {
    if (this->uPolygonType == POLYGON_VerticalWall) {
        a2->x = -this->pFacePlane_old.vNormal.y;
        a2->y = this->pFacePlane_old.vNormal.x;
        a2->z = 0;

        a3->x = 0;
        a3->y = 0;
        a3->z = 0xFFFF0000u;

    } else if (this->uPolygonType == POLYGON_Floor ||
               this->uPolygonType == POLYGON_Ceiling) {
        a2->x = 0x10000u;
        a2->y = 0;
        a2->z = 0;

        a3->x = 0;
        a3->y = 0xFFFF0000u;
        a3->z = 0;

    } else if (this->uPolygonType == POLYGON_InBetweenFloorAndWall ||
               this->uPolygonType == POLYGON_InBetweenCeilingAndWall) {
        if (abs(this->pFacePlane_old.vNormal.z) < 46441) {
            Vec3_float_ a1;
            a1.x = (double)-this->pFacePlane_old.vNormal.y;
            a1.y = (double)this->pFacePlane_old.vNormal.x;
            a1.z = 0.0;
            a1.Normalize();

            a2->x = (signed __int64)(a1.x * 65536.0);
            a2->y = (signed __int64)(a1.y * 65536.0);
            a2->z = 0;

            a3->y = 0;
            a3->z = 0xFFFF0000u;
            a3->x = 0;

        } else {
            a2->x = 0x10000u;
            a2->y = 0;
            a2->z = 0;

            a3->x = 0;
            a3->y = 0xFFFF0000u;
            a3->z = 0;
        }
    }
    // LABEL_12:
    if (this->uAttributes & FACE_UNKNOW3) {
        a2->x = -a2->x;
        a2->y = -a2->y;
        a2->z = -a2->z;
    }
    if (this->uAttributes & FACE_UNKNOW4) {
        a3->x = -a3->x;
        a3->y = -a3->y;
        a3->z = -a3->z;
    }
    return;
}

bool BLVFaceExtra::HasEventint() {
    int event_index = 0;
    if ((uLevelEVT_NumEvents - 1) <= 0) {
        return false;
    }
    while (pLevelEVT_Index[event_index].uEventID != this->uEventID) {
        ++event_index;
        if (event_index >= (signed int)(uLevelEVT_NumEvents - 1)) return false;
    }
    _evt_raw *end_evt =
        (_evt_raw
             *)&pLevelEVT[pLevelEVT_Index[event_index + 1].uEventOffsetInEVT];
    _evt_raw *start_evt =
        (_evt_raw *)&pLevelEVT[pLevelEVT_Index[event_index].uEventOffsetInEVT];
    if ((end_evt->_e_type != EVENT_Exit) ||
        (start_evt->_e_type != EVENT_MouseOver)) {
        return false;
    } else {
        return true;
    }
}

//----- (0046F228) --------------------------------------------------------
void BLV_UpdateDoors() {
    BLVFace *face;       // ebx@24
    Vec3_short_ *v17;    // esi@24
    int v18;             // eax@24
    int v19;             // edx@24
    signed int v20;      // eax@24
    int v24;             // esi@25
    int v25;             // eax@25
    signed __int64 v27;  // qtt@27
    BLVFaceExtra *v28;   // esi@32
    int v32;             // eax@34
    Vec3_short_ *v34;    // eax@35
    int v35;             // ecx@35
    int v36;             // edx@35
    signed int v37;      // eax@35
    signed int v38;      // edx@35
    int v39;             // eax@35
    int v40;             // edx@35
    Vec3_short_ *v43;    // edi@36
    int v57;             // eax@58
    Vec3_int_ v67;
    Vec3_int_ v70;
    int v73;               // [sp+20h] [bp-44h]@24
    int v75;               // [sp+28h] [bp-3Ch]@36
    int v76;               // [sp+2Ch] [bp-38h]@36
    int v77;               // [sp+30h] [bp-34h]@36
    int v82;               // [sp+44h] [bp-20h]@35
    int v83;               // [sp+48h] [bp-1Ch]@34
    int v84;               // [sp+4Ch] [bp-18h]@34
    SoundID eDoorSoundID;  // [sp+54h] [bp-10h]@1
    int v88;               // [sp+5Ch] [bp-8h]@18
    int v89;               // [sp+60h] [bp-4h]@6

    eDoorSoundID = (SoundID)
        pDoorSoundIDsByLocationID[dword_6BE13C_uCurrentlyLoadedLocationID];
    for (uint i = 0; i < pIndoor->uNumDoors; ++i) {
        BLVDoor *door = &pIndoor->pDoors[i];
        if (door->uState == BLVDoor::Closed || door->uState == BLVDoor::Open) {
            door->uAttributes &= 0xFFFFFFFDu;  // ~0x2
            continue;
        }
        door->uTimeSinceTriggered += pEventTimer->uTimeElapsed;
        if (door->uState == BLVDoor::Opening) {
            v89 = (signed int)(door->uTimeSinceTriggered * door->uCloseSpeed) /
                  128;
            if (v89 >= door->uMoveLength) {
                v89 = door->uMoveLength;
                door->uState = BLVDoor::Open;
                if (!(door->uAttributes & FACE_UNKNOW5) &&
                    door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1), PID(OBJECT_BLVDoor, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else if (!(door->uAttributes & FACE_UNKNOW5) &&
                door->uNumVertices) {
                pAudioPlayer->PlaySound(eDoorSoundID, PID(OBJECT_BLVDoor, i), 1,
                    -1, 0, 0);
            }
        } else {
            signed int v5 =
                (signed int)(door->uTimeSinceTriggered * door->uOpenSpeed) /
                128;
            if (v5 >= door->uMoveLength) {
                v89 = 0;
                door->uState = BLVDoor::Closed;
                if (!(door->uAttributes & FACE_UNKNOW5) &&
                    door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1),
                                            PID(OBJECT_BLVDoor, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else {
                v89 = door->uMoveLength - v5;
                if (!(door->uAttributes & FACE_UNKNOW5) && door->uNumVertices)
                    pAudioPlayer->PlaySound(eDoorSoundID,
                                            PID(OBJECT_BLVDoor, i), 1, -1, 0, 0);
            }
        }

        // LABEL_18:
        for (uint j = 0; j < door->uNumVertices; ++j) {
            pIndoor->pVertices[door->pVertexIDs[j]].x =
                fixpoint_mul(door->vDirection.x, v89) + door->pXOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].y =
                fixpoint_mul(door->vDirection.y, v89) + door->pYOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].z =
                fixpoint_mul(door->vDirection.z, v89) + door->pZOffsets[j];
        }
        for (v88 = 0; v88 < door->uNumFaces; ++v88) {
            face = &pIndoor->pFaces[door->pFaceIDs[v88]];
            v17 = &pIndoor->pVertices[face->pVertexIDs[0]];
            v18 = face->pFacePlane_old.vNormal.y;
            v73 = *(int *)&v17->x;
            v19 = face->pFacePlane_old.vNormal.z;
            v20 = -(v19 * (int)v17->z +
                    (signed __int16)v73 * face->pFacePlane_old.vNormal.x +
                    HEXRAYS_SHIWORD(v73) * v18);
            face->pFacePlane_old.dist = v20;
            face->pFacePlane.dist =
                -((double)v17->z * face->pFacePlane.vNormal.z +
                  (double)v17->y * face->pFacePlane.vNormal.y +
                  (double)v17->x * face->pFacePlane.vNormal.x);
            if (v19) {
                v24 = abs(v20 >> 15);
                v25 = abs(face->pFacePlane_old.vNormal.z);
                if (v24 > v25)
                    Error(
                        "Door Error\ndoor id: %i\nfacet no: %i\n\nOverflow "
                        "dividing facet->d [%i] by facet->nz [%i]",
                        door->uDoorID, door->pFaceIDs[v88],
                        face->pFacePlane_old.dist,
                        face->pFacePlane_old.vNormal.z);
                HEXRAYS_LODWORD(v27) = face->pFacePlane_old.dist << 16;
                HEXRAYS_HIDWORD(v27) = face->pFacePlane_old.dist >> 16;
                face->zCalc3 = -v27 / face->pFacePlane_old.vNormal.z;
            }
            // if ( face->uAttributes & FACE_TEXTURE_FLOW || render->pRenderD3D
            // )
            face->_get_normals(&v70, &v67);
            v28 = &pIndoor->pFaceExtras[face->uFaceExtraID];
            /*if ( !render->pRenderD3D )
            {
            if ( !(face->uAttributes & FACE_TEXTURE_FLOW) )
            continue;
            v83 = (unsigned __int64)(door->vDirection.x * (signed __int64)v70.x)
            >> 16; v85 = (unsigned __int64)(door->vDirection.y * (signed
            __int64)v70.y) >> 16; v84 = (unsigned __int64)(door->vDirection.z *
            (signed __int64)v70.z) >> 16; v29 = v89; v28->sTextureDeltaU =
            -((v83 + v85 + v84) * (signed __int64)v89) >> 16; v85 = (unsigned
            __int64)(door->vDirection.x * (signed __int64)v67.x) >> 16; v83 =
            (unsigned __int64)(door->vDirection.y * (signed __int64)v67.y) >>
            16; v84 = (unsigned __int64)(door->vDirection.z * (signed
            __int64)v67.z) >> 16; v31 = (v85 + v83 + v84) * (signed __int64)v29;
            v32 = v31 >> 16;
            v57 = -v32;
            v28->sTextureDeltaV = v57;
            v28->sTextureDeltaU += door->pDeltaUs[v88];
            v28->sTextureDeltaV = v57 + door->pDeltaVs[v88];
            continue;
            }*/
            v28->sTextureDeltaU = 0;
            v28->sTextureDeltaV = 0;
            v34 = &pIndoor->pVertices[face->pVertexIDs[0]];
            v35 = v34->z;
            v36 = v34->y;
            v82 = v34->x;
            v37 = v70.x * v82 + v70.y * v36 + v70.z * v35;
            v38 = v67.x * v82 + v67.y * v36 + v67.z * v35;
            v39 = v37 >> 16;
            *face->pVertexUIDs = v39;
            v40 = v38 >> 16;
            *face->pVertexVIDs = v40;
            v84 = v39;
            v82 = v40;
            for (uint j = 1; j < face->uNumVertices; ++j) {
                v43 = &pIndoor->pVertices[face->pVertexIDs[j]];
                v76 = ((__int64)v70.z * v43->z + (__int64)v70.x * v43->x +
                       (__int64)v70.y * v43->y) >>
                      16;
                v77 = ((__int64)v67.x * v43->x + (__int64)v67.y * v43->y +
                       (__int64)v43->z * v67.z) >>
                      16;
                if (v76 < v39) v39 = v76;
                if (v77 < v40) v40 = v77;
                if (v76 > v84) v84 = v76;
                if (v77 > v82) v82 = v77;
                face->pVertexUIDs[j] = v76;
                face->pVertexVIDs[j] = v77;
            }
            if (face->uAttributes & 0x00001000) {
                v28->sTextureDeltaU -= v39;
            } else {
                if (face->uAttributes & 0x8000) {
                    if (face->resource) {
                        // v28->sTextureDeltaU -= v84 +
                        // pBitmaps_LOD->pTextures[face->uBitmapID].uTextureWidth;
                        v28->sTextureDeltaU -=
                            v84 + ((Texture *)face->resource)->GetWidth();
                    }
                }
            }
            if (face->uAttributes & FACE_UNKNOW6) {
                v28->sTextureDeltaV -= v40;
            } else {
                if (face->uAttributes & FACE_INDOOR_DOOR) {
                    v28->sTextureDeltaV -=
                        v84 + ((Texture *)face->resource)->GetHeight();
                    // if (face->uBitmapID != -1)
                    //    v28->sTextureDeltaV -= v82 +
                    //    pBitmaps_LOD->GetTexture(face->uBitmapID)->uTextureHeight;
                }
            }
            if (face->uAttributes & FACE_TEXTURE_FLOW) {
                v84 = fixpoint_mul(door->vDirection.x, v70.x);
                v82 = fixpoint_mul(door->vDirection.y, v70.y);
                v83 = fixpoint_mul(door->vDirection.z, v70.z);
                v75 = v84 + v82 + v83;
                v82 = fixpoint_mul(v75, v89);
                v28->sTextureDeltaU = -v82;
                v84 = fixpoint_mul(door->vDirection.x, v67.x);
                v82 = fixpoint_mul(door->vDirection.y, v67.y);
                v83 = fixpoint_mul(door->vDirection.z, v67.z);
                v75 = v84 + v82 + v83;
                v32 = fixpoint_mul(v75, v89);
                v57 = -v32;
                v28->sTextureDeltaV = v57;
                v28->sTextureDeltaU += door->pDeltaUs[v88];
                v28->sTextureDeltaV = v57 + door->pDeltaVs[v88];
            }
        }
    }
}

//----- (0046F90C) --------------------------------------------------------
void UpdateActors_BLV() {
    int v2;                  // edi@6
    int v3;                  // eax@6
    int v4;                  // eax@8
    __int16 v5;              // ax@11
    signed int v6;           // ebx@14
    signed __int64 v10;      // qax@18
    int v22;                 // edi@46
    unsigned int v24;        // eax@51
    int v27;                 // ST08_4@54
    int v28;                 // edi@54
    int v29;                 // eax@54
    int v30;                 // ecx@62
    int v31;                 // ebx@62
    int v32;                 // eax@62
    int v33;                 // eax@64
    int v37;          // ebx@85
    int v44;                 // ecx@96
    int v45;                 // edi@101
    AIDirection v52;         // [sp+0h] [bp-60h]@75
    AIDirection v53;         // [sp+1Ch] [bp-44h]@116
    unsigned int uSectorID;  // [sp+3Ch] [bp-24h]@6
    int v56;                 // [sp+40h] [bp-20h]@6
    unsigned int _this;      // [sp+44h] [bp-1Ch]@51
    int v58;                 // [sp+48h] [bp-18h]@51
    int v59;                 // [sp+4Ch] [bp-14h]@8
    unsigned int uFaceID;    // [sp+50h] [bp-10h]@6
    int v61;                 // [sp+54h] [bp-Ch]@14
    int v62;                 // [sp+58h] [bp-8h]@6
    unsigned int actor_id;   // [sp+5Ch] [bp-4h]@1

    if (engine->config->no_actors)
        return;  // uNumActors = 0;

    for (actor_id = 0; actor_id < uNumActors; actor_id++) {
        if (pActors[actor_id].uAIState == Removed ||
            pActors[actor_id].uAIState == Disabled ||
            pActors[actor_id].uAIState == Summoned ||
            !pActors[actor_id].uMovementSpeed)
            continue;
        uSectorID = pActors[actor_id].uSectorID;
        v2 = collide_against_floor(
            pActors[actor_id].vPosition.x, pActors[actor_id].vPosition.y,
            pActors[actor_id].vPosition.z, &uSectorID, &uFaceID);
        pActors[actor_id].uSectorID = uSectorID;
        v3 = pActors[actor_id].pMonsterInfo.uFlying;
        v56 = v2;
        v62 = v3;
        if (!pActors[actor_id].CanAct()) v62 = 0;
        v4 = pActors[actor_id].vPosition.z;
        v59 = 0;
        if (pActors[actor_id].vPosition.z > v2 + 1) v59 = 1;
        if (v2 <= -30000) {
            v5 = pIndoor->GetSector(pActors[actor_id].vPosition.x,
                                    pActors[actor_id].vPosition.y, v4);
            pActors[actor_id].uSectorID = v5;
            v56 = BLV_GetFloorLevel(
                pActors[actor_id].vPosition.x, pActors[actor_id].vPosition.y,
                pActors[actor_id].vPosition.z, v5, &uFaceID);
            if (!v5 || v56 == -30000) continue;
        }
        if (pActors[actor_id].uCurrentActionAnimation ==
            ANIM_Walking) {  //монстр двигается
            v6 = pActors[actor_id].uMovementSpeed;
            if (pActors[actor_id].pActorBuffs[ACTOR_BUFF_SLOWED].Active()) {
                if (pActors[actor_id].pActorBuffs[ACTOR_BUFF_SLOWED].uPower)
                    HEXRAYS_LODWORD(v10) = pActors[actor_id].uMovementSpeed /
                                           (unsigned __int16)pActors[actor_id]
                                               .pActorBuffs[ACTOR_BUFF_SLOWED]
                                               .uPower;
                else
                    v10 = (signed __int64)((double)pActors[actor_id]
                                               .uMovementSpeed *
                                           0.5);
                v6 = v10;
            }
            if (pActors[actor_id].uAIState == Pursuing ||
                pActors[actor_id].uAIState == Fleeing)
                v6 *= 2;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT)
                v6 = (signed __int64)((double)v6 *
                                      flt_6BE3AC_debug_recmod1_x_1_6);
            if (v6 > 1000) v6 = 1000;
            pActors[actor_id].vVelocity.x =
                fixpoint_mul(stru_5C6E00->Cos(pActors[actor_id].uYawAngle), v6);
            pActors[actor_id].vVelocity.y =
                fixpoint_mul(stru_5C6E00->Sin(pActors[actor_id].uYawAngle), v6);
            if (v62)
                pActors[actor_id].vVelocity.z = fixpoint_mul(
                    stru_5C6E00->Sin(pActors[actor_id].uPitchAngle), v6);
        } else {  // actor is not moving(актор не двигается)
            pActors[actor_id].vVelocity.x =
                fixpoint_mul(55000, pActors[actor_id].vVelocity.x);
            pActors[actor_id].vVelocity.y =
                fixpoint_mul(55000, pActors[actor_id].vVelocity.y);
            if (v62)
                pActors[actor_id].vVelocity.z =
                    fixpoint_mul(55000, pActors[actor_id].vVelocity.z);
        }
        if (pActors[actor_id].vPosition.z <= v56) {
            pActors[actor_id].vPosition.z = v56 + 1;
            if (pIndoor->pFaces[uFaceID].uPolygonType == 3) {
                if (pActors[actor_id].vVelocity.z < 0)
                    pActors[actor_id].vVelocity.z = 0;
            } else {
                if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 45000)
                    pActors[actor_id].vVelocity.z -=
                        (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            }
        } else {
            if (v59 && !v62)
                pActors[actor_id].vVelocity.z +=
                    -8 * (short)pEventTimer->uTimeElapsed *
                    GetGravityStrength();
        }
        if (pActors[actor_id].vVelocity.x * pActors[actor_id].vVelocity.x +
                pActors[actor_id].vVelocity.y * pActors[actor_id].vVelocity.y +
                pActors[actor_id].vVelocity.z * pActors[actor_id].vVelocity.z >=
            400) {
            stru_721530.field_84 = -1;
            stru_721530.field_70 = 0;
            stru_721530.field_0 = 1;
            stru_721530.field_8_radius = pActors[actor_id].uActorRadius;
            stru_721530.prolly_normal_d = pActors[actor_id].uActorRadius;
            stru_721530.height = pActors[actor_id].uActorHeight;
            v22 = 0;
            for (uSectorID = 0; uSectorID < 100; uSectorID++) {
                stru_721530.position.x = pActors[actor_id].vPosition.x;
                stru_721530.normal.x = stru_721530.position.x;
                stru_721530.position.y = pActors[actor_id].vPosition.y;
                stru_721530.normal.y = stru_721530.position.y;
                stru_721530.normal.z = pActors[actor_id].vPosition.z +
                                       pActors[actor_id].uActorRadius + 1;
                stru_721530.position.z = pActors[actor_id].vPosition.z -
                                         pActors[actor_id].uActorRadius +
                                         stru_721530.height - 1;
                if (stru_721530.position.z < stru_721530.normal.z)
                    stru_721530.position.z = pActors[actor_id].vPosition.z +
                                             pActors[actor_id].uActorRadius + 1;
                stru_721530.velocity.x = pActors[actor_id].vVelocity.x;
                stru_721530.velocity.y = pActors[actor_id].vVelocity.y;
                stru_721530.velocity.z = pActors[actor_id].vVelocity.z;
                stru_721530.uSectorID = pActors[actor_id].uSectorID;
                if (!stru_721530._47050A(v22)) {
                    v58 = 0;
                    v24 = 8 * actor_id;
                    HEXRAYS_LOBYTE(v24) = PID(OBJECT_Actor, actor_id);
                    for (v61 = 0; v61 < 100; ++v61) {
                        _46E44E_collide_against_faces_and_portals(1);
                        _46E0B2_collide_against_decorations();
                        _46EF01_collision_chech_player(0);
                        _46ED8A_collide_against_sprite_objects(v24);
                        for (uint j = 0; j < ai_arrays_size; j++) {
                            if (ai_near_actors_ids[j] != actor_id) {
                                v27 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.z -
                                    pActors[actor_id].vPosition.z);
                                v28 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.y -
                                    pActors[actor_id].vPosition.y);
                                v29 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.x -
                                    pActors[actor_id].vPosition.x);
                                if (int_get_vector_length(v29, v28, v27) >=
                                        pActors[actor_id].uActorRadius +
                                            (signed int)
                                                pActors[ai_near_actors_ids[j]]
                                                    .uActorRadius &&
                                    Actor::_46DF1A_collide_against_actor(
                                        ai_near_actors_ids[j], 40))
                                    ++v58;
                            }
                        }
                        if (_46F04E_collide_against_portals()) break;
                    }
                    v56 = v58 > 1;
                    if (stru_721530.field_7C >= stru_721530.field_6C) {
                        v30 = stru_721530.normal2.x;
                        v31 = stru_721530.normal2.y;
                        v32 = stru_721530.normal2.z -
                              stru_721530.prolly_normal_d - 1;
                    } else {
                        v30 = pActors[actor_id].vPosition.x +
                              fixpoint_mul(stru_721530.field_7C,
                                           stru_721530.direction.x);
                        v31 = pActors[actor_id].vPosition.y +
                              fixpoint_mul(stru_721530.field_7C,
                                           stru_721530.direction.y);
                        v32 = pActors[actor_id].vPosition.z +
                              fixpoint_mul(stru_721530.field_7C,
                                           stru_721530.direction.z);
                    }
                    v33 = collide_against_floor(
                        v30, v31, v32, &stru_721530.uSectorID, &uFaceID);
                    if (pIndoor->pFaces[uFaceID].uAttributes &
                            FACE_INDOOR_SKY &&
                        pActors[actor_id].uAIState == Dead) {
                        pActors[actor_id].uAIState = Removed;
                        continue;
                    }
                    if (v59 || v62 ||
                        !(pIndoor->pFaces[uFaceID].uAttributes &
                          FACE_INDOOR_SKY)) {
                        if (v33 == -30000) continue;
                        if (pActors[actor_id].uCurrentActionAnimation != 1 ||
                            v33 >= pActors[actor_id].vPosition.z - 100 || v59 ||
                            v62) {
                            if (stru_721530.field_7C < stru_721530.field_6C) {
                                pActors[actor_id].vPosition.x +=
                                    fixpoint_mul(stru_721530.field_7C,
                                                 stru_721530.direction.x);
                                pActors[actor_id].vPosition.y +=
                                    fixpoint_mul(stru_721530.field_7C,
                                                 stru_721530.direction.y);
                                pActors[actor_id].vPosition.z +=
                                    fixpoint_mul(stru_721530.field_7C,
                                                 stru_721530.direction.z);
                                pActors[actor_id].uSectorID =
                                    (short)stru_721530.uSectorID;
                                stru_721530.field_70 += stru_721530.field_7C;
                                v37 = PID_ID(stru_721530.pid);
                                if (PID_TYPE(stru_721530.pid) == OBJECT_Actor) {
                                    if (pParty->bTurnBasedModeOn &&
                                        (pTurnEngine->turn_stage == TE_ATTACK ||
                                         pTurnEngine->turn_stage == TE_MOVEMENT)) {
                                        pActors[actor_id].vVelocity.x =
                                            fixpoint_mul(58500, pActors[actor_id].vVelocity.x);
                                        pActors[actor_id].vVelocity.y =
                                            fixpoint_mul(58500, pActors[actor_id].vVelocity.y);
                                        pActors[actor_id].vVelocity.z =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                        v22 = 0;
                                        continue;
                                    }
                                    if (pActors[actor_id]
                                            .pMonsterInfo.uHostilityType) {
                                        if (!v56) {
                                            Actor::AI_Flee(actor_id,
                                                           stru_721530.pid, v22,
                                                           (AIDirection *)v22);
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    } else {
                                        if (!v56) {
                                            if (!pActors[v37]
                                                     .pMonsterInfo
                                                     .uHostilityType) {
                                                Actor::AI_FaceObject(
                                                    actor_id, stru_721530.pid,
                                                    v22, (AIDirection *)v22);
                                                pActors[actor_id].vVelocity.x =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.x);
                                                pActors[actor_id].vVelocity.y =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.y);
                                                pActors[actor_id].vVelocity.z =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.z);
                                                v22 = 0;
                                                continue;
                                            }
                                            Actor::AI_Flee(actor_id,
                                                           stru_721530.pid, v22,
                                                           (AIDirection *)v22);
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    }
                                    Actor::AI_StandOrBored(actor_id, 4, v22,
                                                           &v53);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(stru_721530.pid) ==
                                    OBJECT_Player) {
                                    if (pActors[actor_id].GetActorsRelation(
                                            0)) {
                                        // v51 =
                                        // __OFSUB__(HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime),
                                        // v22); v49 =
                                        // HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime)
                                        // == v22; v50 =
                                        // HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime)
                                        // - v22 < 0;
                                        pActors[actor_id].vVelocity.y = 0;
                                        pActors[actor_id].vVelocity.x = 0;
                                        if (pParty
                                                ->pPartyBuffs
                                                    [PARTY_BUFF_INVISIBILITY]
                                                .Active()) {
                                            pParty
                                                ->pPartyBuffs
                                                    [PARTY_BUFF_INVISIBILITY]
                                                .Reset();
                                        }

                                        viewparams->bRedrawGameUI = 1;
                                        pActors[actor_id].vVelocity.x =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                        pActors[actor_id].vVelocity.y =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                        pActors[actor_id].vVelocity.z =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                        continue;
                                    }
                                    Actor::AI_FaceObject(actor_id,
                                                         stru_721530.pid, v22,
                                                         (AIDirection *)v22);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(stru_721530.pid) ==
                                    OBJECT_Decoration) {
                                    _this = integer_sqrt(
                                        pActors[actor_id].vVelocity.x *
                                            pActors[actor_id].vVelocity.x +
                                        pActors[actor_id].vVelocity.y *
                                            pActors[actor_id].vVelocity.y);
                                    v45 = stru_5C6E00->Atan2(
                                        pActors[actor_id].vPosition.x -
                                            pLevelDecorations[v37].vPosition.x,
                                        pActors[actor_id].vPosition.y -
                                            pLevelDecorations[v37].vPosition.y);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(stru_5C6E00->Cos(v45),
                                                     _this);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(stru_5C6E00->Sin(v45),
                                                     _this);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(stru_721530.pid) ==
                                    OBJECT_BModel) {
                                    stru_721530.field_84 = stru_721530.pid >> 3;
                                    if (pIndoor->pFaces[v37].uPolygonType ==
                                        3) {
                                        pActors[actor_id].vVelocity.z = 0;
                                        pActors[actor_id].vPosition.z =
                                            pIndoor
                                                ->pVertices[*pIndoor
                                                                 ->pFaces[v37]
                                                                 .pVertexIDs]
                                                .z +
                                            1;
                                        if (pActors[actor_id].vVelocity.x *
                                                    pActors[actor_id]
                                                        .vVelocity.x +
                                                pActors[actor_id].vVelocity.y *
                                                    pActors[actor_id]
                                                        .vVelocity.y <
                                            400) {
                                            pActors[actor_id].vVelocity.y = 0;
                                            pActors[actor_id].vVelocity.x = 0;
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    } else {
                                        v61 = abs(pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.x *
                                                      pActors[actor_id]
                                                          .vVelocity.x +
                                                  pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.y *
                                                      pActors[actor_id]
                                                          .vVelocity.y +
                                                  pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.z *
                                                      pActors[actor_id]
                                                          .vVelocity.z) >>
                                              16;
                                        if ((stru_721530.speed >> 3) > v61)
                                            v61 = stru_721530.speed >> 3;
                                        pActors[actor_id].vVelocity.x +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.x);
                                        pActors[actor_id].vVelocity.y +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.y);
                                        pActors[actor_id].vVelocity.z +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.z);
                                        if (pIndoor->pFaces[v37].uPolygonType !=
                                                4 &&
                                            pIndoor->pFaces[v37].uPolygonType !=
                                                3) {
                                            v44 = stru_721530.prolly_normal_d -
                                                  ((pIndoor->pFaces[v37]
                                                        .pFacePlane_old.dist +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.z *
                                                        pActors[actor_id]
                                                            .vPosition.z +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.y *
                                                        pActors[actor_id]
                                                            .vPosition.y +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.x *
                                                        pActors[actor_id]
                                                            .vPosition.x) >>
                                                   16);
                                            if (v44 > 0) {
                                                pActors[actor_id].vPosition.x +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.x);
                                                pActors[actor_id].vPosition.y +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.y);
                                                pActors[actor_id].vPosition.z +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.z);
                                            }
                                            pActors[actor_id]
                                                .uYawAngle = stru_5C6E00->Atan2(
                                                pActors[actor_id].vVelocity.x,
                                                pActors[actor_id].vVelocity.y);
                                        }
                                    }
                                    if (pIndoor->pFaces[v37].uAttributes &
                                        FACE_UNKNOW1)
                                        EventProcessor(
                                            pIndoor
                                                ->pFaceExtras[pIndoor
                                                                  ->pFaces[v37]
                                                                  .uFaceExtraID]
                                                .uEventID,
                                            0, 1);
                                }
                                pActors[actor_id].vVelocity.x = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.x);
                                pActors[actor_id].vVelocity.y = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.y);
                                pActors[actor_id].vVelocity.z = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.z);
                                v22 = 0;
                                continue;
                            } else {
                                pActors[actor_id].vPosition.x =
                                    (short)stru_721530.normal2.x;
                                pActors[actor_id].vPosition.y =
                                    (short)stru_721530.normal2.y;
                                pActors[actor_id].vPosition.z =
                                    (short)stru_721530.normal2.z -
                                    (short)stru_721530.prolly_normal_d - 1;
                                pActors[actor_id].uSectorID =
                                    (short)stru_721530.uSectorID;
                                // goto LABEL_123;
                                break;
                            }

                        } else if (pActors[actor_id].vPosition.x & 1) {
                            pActors[actor_id].uYawAngle += 100;
                        } else {
                            pActors[actor_id].uYawAngle -= 100;
                        }
                    } else {
                        if (pParty->bTurnBasedModeOn &&
                            (pTurnEngine->turn_stage == TE_ATTACK ||
                             pTurnEngine->turn_stage == TE_MOVEMENT))
                            continue;
                        if (!pActors[actor_id].pMonsterInfo.uHostilityType ||
                            v56 != v22) {
                            Actor::AI_StandOrBored(actor_id, 4, v22, &v52);
                            continue;
                        }
                    }
                }
            }
        } else {
            pActors[actor_id].vVelocity.z = 0;
            pActors[actor_id].vVelocity.y = 0;
            pActors[actor_id].vVelocity.x = 0;
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_SKY) {
                if (pActors[actor_id].uAIState == Dead)
                    pActors[actor_id].uAIState = Removed;
            }
        }
        // LABEL_123:
    }
}

//----- (00460A78) --------------------------------------------------------
void PrepareToLoadBLV(unsigned int bLoading) {
    unsigned int respawn_interval;  // ebx@1
    unsigned int map_id;            // eax@8
    MapInfo *map_info;              // edi@9
    int v4;                         // eax@11
    char v28;                       // zf@81
    signed int v30;                 // edi@94
    int v34[4];                     // [sp+3E8h] [bp-2Ch]@96
    int v35;                        // [sp+3F8h] [bp-1Ch]@1
    int v38;                        // [sp+404h] [bp-10h]@1
    int pDest;                      // [sp+40Ch] [bp-8h]@1

    respawn_interval = 0;
    pGameLoadingUI_ProgressBar->Reset(0x20u);
    bNoNPCHiring = false;
    pDest = 1;
    uCurrentlyLoadedLevelType = LEVEL_Indoor;

    engine->SetUnderwater(
        Is_out15odm_underwater());

    if ((pCurrentMapName == "out15.odm") || (pCurrentMapName == "d23.blv")) {
        bNoNPCHiring = true;
    }
    pPaletteManager->pPalette_tintColor[0] = 0;
    pPaletteManager->pPalette_tintColor[1] = 0;
    pPaletteManager->pPalette_tintColor[2] = 0;
    pPaletteManager->RecalculateAll();
    if (_A750D8_player_speech_timer) _A750D8_player_speech_timer = 0;
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        map_info = &pMapStats->pInfos[map_id];
        respawn_interval = pMapStats->pInfos[map_id].uRespawnIntervalDays;
        v38 = GetAlertStatus();
    } else {
        map_info = (MapInfo *)bLoading;
    }
    dword_6BE13C_uCurrentlyLoadedLocationID = map_id;

    pStationaryLightsStack->uNumLightsActive = 0;
    v4 = pIndoor->Load(pCurrentMapName, pParty->GetPlayingTime().GetDays() + 1,
                       respawn_interval, (char *)&pDest) -
         1;
    if (!v4) Error("Unable to open %s", pCurrentMapName.c_str());

    if (v4 == 1) Error("File %s is not a BLV File", pCurrentMapName.c_str());

    if (v4 == 2) Error("Attempt to open new level before clearing old");
    if (v4 == 3) Error("Out of memory loading indoor level");
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_2000)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_2000;
    if (!map_id) pDest = 0;
    if (pDest == 1) {
        for (uint i = 0; i < pIndoor->uNumSpawnPoints; ++i) {
            auto spawn = pIndoor->pSpawnPoints + i;
            if (spawn->IsMonsterSpawn())
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                map_info->SpawnRandomTreasure(spawn);
        }
        RespawnGlobalDecorations();
    }

    for (uint i = 0; i < pIndoor->uNumDoors; ++i) {
        if (pIndoor->pDoors[i].uAttributes & 0x01) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
        }

        if (pIndoor->pDoors[i].uState == BLVDoor::Closed) {
            pIndoor->pDoors[i].uState = BLVDoor::Closing;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
        } else if (pIndoor->pDoors[i].uState == BLVDoor::Open) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
        }
    }

    /*for (uint i = 0; i < pIndoor->uNumFaces; ++i)
    {
        if (pIndoor->pFaces[i].uBitmapID != -1)
            pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id2 =
    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id1);
    }*/

    pGameLoadingUI_ProgressBar->Progress();

    v35 = 0;
    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);

        DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);

        if (decoration->uSoundID && _6807E0_num_decorations_with_sounds_6807B8 < 9) {
            // pSoundList->LoadSound(decoration->uSoundID, 0);
            _6807B8_level_decorations_ids[_6807E0_num_decorations_with_sounds_6807B8++] = i;
        }

        if (!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            if (!decoration->DontDraw()) {
                if (decoration->uLightRadius) {
                    unsigned char r = 255, g = 255, b = 255;
                    if (/*render->pRenderD3D*/ true &&
                        render->config->is_using_colored_lights) {
                        r = decoration->uColoredLightRed;
                        g = decoration->uColoredLightGreen;
                        b = decoration->uColoredLightBlue;
                    }
                    pStationaryLightsStack->AddLight(
                        pLevelDecorations[i].vPosition.x,
                        pLevelDecorations[i].vPosition.y,
                        pLevelDecorations[i].vPosition.z +
                            decoration->uDecorationHeight,
                        decoration->uLightRadius, r, g, b, _4E94D0_light_type);
                }
            }
        }

        if (!pLevelDecorations[i].uEventID) {
            if (pLevelDecorations[i].IsInteractive()) {
                if (v35 < 124) {
                    pLevelDecorations[i]._idx_in_stru123 = v35 + 75;
                    if (!stru_5E4C90_MapPersistVars._decor_events[v35])
                        pLevelDecorations[i].uFlags |=
                            LEVEL_DECORATION_INVISIBLE;
                    v35++;
                }
            }
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].containing_item.uItemID) {
                if (pSpriteObjects[i].containing_item.uItemID != 220 &&
                    pItemsTable
                            ->pItems[pSpriteObjects[i].containing_item.uItemID]
                            .uEquipType == EQUIP_POTION &&
                    !pSpriteObjects[i].containing_item.uEnchantmentType)
                    pSpriteObjects[i].containing_item.uEnchantmentType =
                        rand() % 15 + 5;
                pItemsTable->SetSpecialBonus(
                    &pSpriteObjects[i].containing_item);
            }
        }
    }

    // INDOOR initialize actors
    v38 = 0;

    for (uint i = 0; i < uNumActors; ++i) {
        if (pActors[i].uAttributes & ACTOR_UNKNOW7) {
            if (!map_id) {
                pActors[i].pMonsterInfo.field_3E = 19;
                pActors[i].uAttributes |= ACTOR_UNKNOW11;
                continue;
            }
            v28 = v38 == 0;
        } else {
            v28 = v38 == 1;
        }

        if (!v28) {
            pActors[i].PrepareSprites(0);
            pActors[i].pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
            if (pActors[i].pMonsterInfo.field_3E != 11 &&
                pActors[i].pMonsterInfo.field_3E != 19 &&
                (!pActors[i].sCurrentHP || !pActors[i].pMonsterInfo.uHP)) {
                pActors[i].pMonsterInfo.field_3E = 5;
                pActors[i].UpdateAnimation();
            }
        } else {
            pActors[i].pMonsterInfo.field_3E = 19;
            pActors[i].uAttributes |= ACTOR_UNKNOW11;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    // Party to start position
    Actor this_;
    this_.pMonsterInfo.uID = 45;
    this_.PrepareSprites(0);
    if (!bLoading) {
        pParty->sRotationX = 0;
        pParty->sRotationY = 0;
        pParty->vPosition.z = 0;
        pParty->vPosition.y = 0;
        pParty->vPosition.x = 0;
        pParty->uFallStartY = 0;
        pParty->uFallSpeed = 0;
        TeleportToStartingPoint(uLevel_StartingPointType);
    }
    viewparams->_443365();
    PlayLevelMusic();
    if (!bLoading) {
        v30 = 0;
        for (uint pl_id = 1; pl_id <= 4; ++pl_id) {
            if (pPlayers[pl_id]->CanAct()) v34[v30++] = pl_id;
        }
        if (v30) {
            if (pDest) {
                _A750D8_player_speech_timer = 256;
                PlayerSpeechID = SPEECH_46;
                uSpeakingCharacter = v34[rand() % v30];
            }
        }
    }
}

//----- (0046CEC3) --------------------------------------------------------
int BLV_GetFloorLevel(int x, int y, int z, unsigned int uSectorID,
                      unsigned int *pFaceID) {
    int v13;                  // ecx@13
    signed int v14;           // ebx@14
    int v15;                  // eax@16
    int v21;                  // eax@27
    signed int v28;           // eax@45
    int v29;                  // ebx@47
    int v38;                  // edx@62
    bool v47;                 // [sp+24h] [bp-1Ch]@43
    bool current_vertices_Y;  // [sp+28h] [bp-18h]@10
    bool v49;                 // [sp+28h] [bp-18h]@41
    bool next_vertices_Y;     // [sp+2Ch] [bp-14h]@12
    int number_hits;          // [sp+30h] [bp-10h]@10
    int v54;                  // [sp+30h] [bp-10h]@41
    int v55;                  // [sp+34h] [bp-Ch]@1

    static int blv_floor_id[50];     // 00721200
    static int blv_floor_level[50];  // 007212C8

    static __int16 blv_floor_face_vert_coord_Y[104];  // word_721390_ys
    static __int16 blv_floor_face_vert_coord_X[104];  // word_721460_xs

    BLVSector *pSector = &pIndoor->pSectors[uSectorID];
    v55 = 0;
    for (uint i = 0; i < pSector->uNumFloors; ++i) {
        BLVFace *pFloor = &pIndoor->pFaces[pSector->pFloors[i]];
        if (pFloor->Ethereal()) continue;

        assert(pFloor->uNumVertices);
        if (x <= pFloor->pBounding.x2 && x >= pFloor->pBounding.x1 &&
            y <= pFloor->pBounding.y2 && y >= pFloor->pBounding.y1) {
            for (uint j = 0; j < pFloor->uNumVertices; ++j) {
                blv_floor_face_vert_coord_X[2 * j] =
                    pFloor->pXInterceptDisplacements[j] +
                    pIndoor->pVertices[pFloor->pVertexIDs[j]].x;
                blv_floor_face_vert_coord_X[2 * j + 1] =
                    pFloor->pXInterceptDisplacements[j] +
                    pIndoor->pVertices[pFloor->pVertexIDs[j + 1]].x;
                blv_floor_face_vert_coord_Y[2 * j] =
                    pFloor->pYInterceptDisplacements[j] +
                    pIndoor->pVertices[pFloor->pVertexIDs[j]].y;
                blv_floor_face_vert_coord_Y[2 * j + 1] =
                    pFloor->pYInterceptDisplacements[j] +
                    pIndoor->pVertices[pFloor->pVertexIDs[j + 1]].y;
            }
            blv_floor_face_vert_coord_X[2 * pFloor->uNumVertices] =
                blv_floor_face_vert_coord_X[0];
            blv_floor_face_vert_coord_Y[2 * pFloor->uNumVertices] =
                blv_floor_face_vert_coord_Y[0];

            next_vertices_Y = blv_floor_face_vert_coord_Y[0] >= y;
            number_hits = 0;

            for (uint j = 0; j < 2 * pFloor->uNumVertices; ++j) {
                if (number_hits >= 2) break;

                current_vertices_Y = next_vertices_Y;
                next_vertices_Y = blv_floor_face_vert_coord_Y[j + 1] >= y;

                v13 = i;
                if (current_vertices_Y == next_vertices_Y) continue;

                v14 = blv_floor_face_vert_coord_X[j + 1] >= x ? 0 : 2;
                v15 = v14 | (blv_floor_face_vert_coord_X[j] < x);

                if (v15 == 3) {
                    continue;
                } else if (!v15) {
                    ++number_hits;
                } else {
                    long long a_div_b =
                        fixpoint_div(y - blv_floor_face_vert_coord_Y[j],
                                     blv_floor_face_vert_coord_Y[j + 1] -
                                         blv_floor_face_vert_coord_Y[j]);
                    long long res = fixpoint_mul(
                        (signed int)blv_floor_face_vert_coord_X[j + 1] -
                            (signed int)blv_floor_face_vert_coord_X[j],
                        a_div_b);

                    if (res + blv_floor_face_vert_coord_X[j] >= x)
                        ++number_hits;
                }
            }

            if (number_hits == 1) {
                if (v55 >= 50) break;
                if (pFloor->uPolygonType == POLYGON_Floor ||
                    pFloor->uPolygonType == POLYGON_Ceiling)
                    v21 = pIndoor->pVertices[pFloor->pVertexIDs[0]].z;
                else
                    v21 = fixpoint_mul(pFloor->zCalc1, x) +
                          fixpoint_mul(pFloor->zCalc2, y) +
                          (short)(pFloor->zCalc3 >> 16);
                blv_floor_level[v55] = v21;
                blv_floor_id[v55] = pSector->pFloors[i];
                v55++;
            }
        }
    }

    if (pSector->field_0 & 8) {
        for (uint i = 0; i < pSector->uNumPortals; ++i) {
            BLVFace *portal = &pIndoor->pFaces[pSector->pPortals[i]];
            if (portal->uPolygonType != POLYGON_Floor) continue;

            if (!portal->uNumVertices) continue;

            if (x <= portal->pBounding.x2 && x >= portal->pBounding.x1 &&
                y <= portal->pBounding.y2 && y >= portal->pBounding.y1) {
                for (uint j = 0; j < portal->uNumVertices; ++j) {
                    blv_floor_face_vert_coord_X[2 * j] =
                        portal->pXInterceptDisplacements[j] +
                        pIndoor->pVertices[portal->pVertexIDs[j]].x;
                    blv_floor_face_vert_coord_X[2 * j + 1] =
                        portal->pXInterceptDisplacements[j + 1] +
                        pIndoor->pVertices[portal->pVertexIDs[j + 1]].x;
                    blv_floor_face_vert_coord_Y[2 * j] =
                        portal->pYInterceptDisplacements[j] +
                        pIndoor->pVertices[portal->pVertexIDs[j]].y;
                    blv_floor_face_vert_coord_Y[2 * j + 1] =
                        portal->pYInterceptDisplacements[j + 1] +
                        pIndoor->pVertices[portal->pVertexIDs[j + 1]].y;
                }
                blv_floor_face_vert_coord_X[2 * portal->uNumVertices] =
                    blv_floor_face_vert_coord_X[0];
                blv_floor_face_vert_coord_Y[2 * portal->uNumVertices] =
                    blv_floor_face_vert_coord_Y[0];
                v54 = 0;
                v47 = blv_floor_face_vert_coord_Y[0] >= y;

                for (uint j = 0; j < 2 * portal->uNumVertices; ++j) {
                    v49 = v47;
                    if (v54 >= 2) break;
                    v47 = blv_floor_face_vert_coord_Y[j + 1] >= y;
                    if (v49 != v47) {
                        v28 = blv_floor_face_vert_coord_X[j + 1] >= x ? 0 : 2;
                        v29 = v28 | (blv_floor_face_vert_coord_X[j] < x);
                        if (v29 != 3) {
                            if (!v29) {
                                ++v54;
                            } else {
                                long long a_div_b = fixpoint_div(
                                    y - blv_floor_face_vert_coord_Y[j],
                                    blv_floor_face_vert_coord_Y[j + 1] -
                                        blv_floor_face_vert_coord_Y[j]);
                                long long res = fixpoint_mul(
                                    blv_floor_face_vert_coord_X[j + 1] -
                                        blv_floor_face_vert_coord_X[j],
                                    a_div_b);
                                if (res + blv_floor_face_vert_coord_X[j] >= x)
                                    ++v54;
                            }
                        }
                    }
                }
                if (v54 == 1) {
                    if (v55 >= 50) break;
                    blv_floor_level[v55] = -29000;
                    blv_floor_id[v55] = pSector->pPortals[i];
                    v55++;
                }
            }
        }
    }
    if (v55 == 1) {
        *pFaceID = blv_floor_id[0];
        if (blv_floor_level[0] <= -29000) __debugbreak();
        return blv_floor_level[0];
    }
    if (!v55) return -30000;
    *pFaceID = blv_floor_id[0];
    // result = blv_floor_level[0];

    /*for ( v35 = 1; v35 < v55; ++v35 )
    {
      if ( blv_floor_level[0] <= z + 5 )
      {
        if ( blv_floor_level[v35] >= blv_floor_level[0] || blv_floor_level[v35]
    > z + 5 ) continue; blv_floor_level[0] = blv_floor_level[v35]; *pFaceID =
    blv_floor_id[v35]; continue;
      }
      if ( blv_floor_level[v35] < blv_floor_level[0] )
      {
        blv_floor_level[0] = blv_floor_level[v35];
        *pFaceID = blv_floor_id[v35];
      }
    }*/

    int result = blv_floor_level[0];
    for (uint i = 1; i < v55; ++i) {
        v38 = blv_floor_level[i];
        if (result <= z + 5) {
            if (v38 > result && v38 <= z + 5) {
                result = blv_floor_level[i];
                if (blv_floor_level[i] <= -29000) __debugbreak();
                *pFaceID = blv_floor_id[i];
            }
        } else if (v38 < result) {
            result = blv_floor_level[i];
            if (blv_floor_level[i] < -29000) __debugbreak();  // crashes here when <=
            *pFaceID = blv_floor_id[i];
        }
    }

    return result;
}

//----- (0043FDED) --------------------------------------------------------
void IndoorLocation::PrepareActorRenderList_BLV() {  // combines this with outdoorlocation ??
    unsigned int v4;  // eax@5
    int v6;           // esi@5
    int v8;           // eax@10
    SpriteFrame *v9;  // eax@16
    int v12;          // ecx@28
    __int16 v41;      // [sp+3Ch] [bp-18h]@18
    // int z; // [sp+48h] [bp-Ch]@32
    // signed int y; // [sp+4Ch] [bp-8h]@32
    // int x; // [sp+50h] [bp-4h]@32

    for (uint i = 0; i < uNumActors; ++i) {
        if (pActors[i].uAIState == Removed || pActors[i].uAIState == Disabled)
            continue;

        v4 = stru_5C6E00->Atan2(
            pActors[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
            pActors[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);
        v6 = ((signed int)(pActors[i].uYawAngle +
                           ((signed int)stru_5C6E00->uIntegerPi >> 3) - v4 +
                           stru_5C6E00->uIntegerPi) >>
              8) &
             7;
        v8 = pActors[i].uCurrentActionTime;
        if (pParty->bTurnBasedModeOn) {
            if (pActors[i].uCurrentActionAnimation == 1)
                v8 = i * 32 + pMiscTimer->uTotalGameTimeElapsed;
        } else {
            if (pActors[i].uCurrentActionAnimation == 1)
                v8 = i * 32 + pBLVRenderParams->field_0_timer_;
        }
        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].pActorBuffs[ACTOR_BUFF_PARALYZED].Active())
            v8 = 0;

        if (pActors[i].uAIState == Resurrected)
            v9 = pSpriteFrameTable->GetFrameBy_x(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], v8);
        else
            v9 = pSpriteFrameTable->GetFrame(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], v8);

        v41 = 0;
        if (v9->uFlags & 2) v41 = 2;
        if (v9->uFlags & 0x40000) v41 |= 0x40;
        if (v9->uFlags & 0x20000) v41 |= 0x80;
        if ((256 << v6) & v9->uFlags) v41 |= 4;
        if (v9->uGlowRadius) {
            pMobileLightsStack->AddLight(
                pActors[i].vPosition.x, pActors[i].vPosition.y,
                pActors[i].vPosition.z, pActors[i].uSectorID, v9->uGlowRadius,
                0xFFu, 0xFFu, 0xFFu, _4E94D3_light_type);
        }

        for (v12 = 0; v12 < pBspRenderer->uNumVisibleNotEmptySectors; ++v12) {
            if (pBspRenderer
                    ->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[v12] ==
                pActors[i].uSectorID) {
                int view_x = 0;
                int view_y = 0;
                int view_z = 0;
                bool visible = pIndoorCameraD3D->ViewClip(
                    pActors[i].vPosition.x, pActors[i].vPosition.y,
                    pActors[i].vPosition.z, &view_x, &view_y, &view_z);
                if (visible) {
                    if (abs(view_x) >= abs(view_y)) {
                        int projected_x = 0;
                        int projected_y = 0;
                        pIndoorCameraD3D->Project(view_x, view_y, view_z,
                                                  &projected_x, &projected_y);

                        if (uNumBillboardsToDraw >= 500) break;
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pActors[i].uAttributes |= ACTOR_VISIBLE;
                        pBillboardRenderList[uNumBillboardsToDraw - 1]
                            .hwsprite = v9->hw_sprites[v6];

                        // error catching
                        if (v9->hw_sprites[v6]->texture->GetHeight() == 0 || v9->hw_sprites[v6]->texture->GetWidth() == 0)
                            __debugbreak();

                        pBillboardRenderList[uNumBillboardsToDraw - 1]
                            .uPalette = v9->uPaletteIndex;
                        pBillboardRenderList[uNumBillboardsToDraw - 1] .uIndoorSectorID = pActors[i].uSectorID;

                        pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x = pIndoorCameraD3D->fov_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y = pIndoorCameraD3D->fov_y;

                        auto _v18_over_x =
                            fixed::FromInt(
                                floorf(pIndoorCameraD3D->fov_x + 0.5f)) /
                            fixed::FromInt(view_x);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = v9->scale.GetFloat() * _v18_over_x.GetFloat();
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = v9->scale.GetFloat() * _v18_over_x.GetFloat();

                        if (pActors[i].pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = spell_fx_renderer->_4A806F_get_mass_distortion_value(&pActors[i]) *
                                pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y;
                        } else if (pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].Active() &&
                                   pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower > 0) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y =
                                    1.0f / pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower *
                                    pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y;
                        }

                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_x = pActors[i].vPosition.x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_y = pActors[i].vPosition.y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_z = pActors[i].vPosition.z;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z = view_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Actor, i);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v41 & 0xFF;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v9;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = pMonsterList
                                ->pMonsters[pActors[i].pMonsterInfo.uID - 1].sTintColor;

                        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .field_1E |= 0x100;
                        }
                    }
                }
            }
        }
    }
}

void IndoorLocation::PrepareItemsRenderList_BLV() {
    unsigned int v6;     // eax@12

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].HasSprite()) {
                if ((pSpriteObjects[i].uType < 1000 ||
                     pSpriteObjects[i].uType >= 10000) &&
                        (pSpriteObjects[i].uType < 500 ||
                         pSpriteObjects[i].uType >= 600) &&
                        (pSpriteObjects[i].uType < 811 ||
                         pSpriteObjects[i].uType >= 815) ||
                    spell_fx_renderer->RenderAsSprite(&pSpriteObjects[i])) {
                    SpriteFrame *v4 = pSpriteObjects[i].GetSpriteFrame();
                    int a6 = v4->uGlowRadius * pSpriteObjects[i].field_22_glow_radius_multiplier;
                    v6 = stru_5C6E00->Atan2(pSpriteObjects[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
                                            pSpriteObjects[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);
                    int v7 = pSpriteObjects[i].uFacing;
                    int v9 = ((int)(stru_5C6E00->uIntegerPi + ((int)stru_5C6E00->uIntegerPi >> 3) + v7 - v6) >> 8) & 7;

                    pBillboardRenderList[uNumBillboardsToDraw].hwsprite = v4->hw_sprites[v9];
                    // error catching
                    if (v4->hw_sprites[v9]->texture->GetHeight() == 0 || v4->hw_sprites[v9]->texture->GetWidth() == 0)
                        __debugbreak();

                    if (v4->uFlags & 0x20)
                        pSpriteObjects[i].vPosition.z -= (int)(fixpoint_mul(v4->scale._internal, v4->hw_sprites[v9]->uBufferHeight) / 2);

                    int16_t v34 = 0;
                    if (v4->uFlags & 2) v34 = 2;
                    if (v4->uFlags & 0x40000) v34 |= 0x40;
                    if (v4->uFlags & 0x20000) v34 |= 0x80;
                    // v11 = (int *)(256 << v9);
                    if ((256 << v9) & v4->uFlags) v34 |= 4;
                    if (a6) {
                        pMobileLightsStack->AddLight(
                            pSpriteObjects[i].vPosition.x,
                            pSpriteObjects[i].vPosition.y,
                            pSpriteObjects[i].vPosition.z,
                            pSpriteObjects[i].uSectorID, a6,
                            pSpriteObjects[i].GetParticleTrailColorR(),
                            pSpriteObjects[i].GetParticleTrailColorG(),
                            pSpriteObjects[i].GetParticleTrailColorB(),
                            _4E94D3_light_type);
                    }

                    int view_x = 0;
                    int view_y = 0;
                    int view_z = 0;

                    bool visible = pIndoorCameraD3D->ViewClip(pSpriteObjects[i].vPosition.x,
                                                              pSpriteObjects[i].vPosition.y,
                                                              pSpriteObjects[i].vPosition.z,
                                                              &view_x, &view_y, &view_z);

                    view_x -= 0.005;


                    if (visible) {
                        int projected_x = 0;
                        int projected_y = 0;
                        pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

                        assert(uNumBillboardsToDraw < 500);
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pSpriteObjects[i].uAttributes |= 1;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uPalette = v4->uPaletteIndex;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID = pSpriteObjects[i].uSectorID;
                        // if ( render->pRenderD3D )
                        {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x = pIndoorCameraD3D->fov_x;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y = pIndoorCameraD3D->fov_y;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x =
                                v4->scale.GetFloat() * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y =
                                v4->scale.GetFloat() * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
                        }

                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v34;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_x = pSpriteObjects[i].vPosition.x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_y = pSpriteObjects[i].vPosition.y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_z = pSpriteObjects[i].vPosition.z;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = 0;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v4;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z = view_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Item, i);
                    }
                }
            }
        }
    }
}

void AddBspNodeToRenderList(unsigned int node_id) {
    BLVSector *pSector = &pIndoor->pSectors[pBspRenderer->nodes[node_id].uSectorID];
    // if ( render->pRenderD3D )
    {
        for (uint i = 0; i < pSector->uNumNonBSPFaces; ++i)
            // logger->Warning(L"Non-BSP face: %X", v3->pFaceIDs[v2]);
            pBspRenderer->AddFaceToRenderList_d3d(node_id, pSector->pFaceIDs[i]);  // рекурсия\recursion
    }
    /*else
    {
      for (uint i = 0; i < pSector->uNumNonBSPFaces; ++i)
        pBspRenderer->AddFaceToRenderList_sw(node_id, pSector->pFaceIDs[i]);
    }*/
    if (pSector->field_0 & 0x10) sub_4406BC(node_id, pSector->uFirstBSPNode);
}

//----- (004406BC) --------------------------------------------------------
void sub_4406BC(unsigned int node_id, unsigned int uFirstNode) {
    BLVSector *pSector;       // esi@2
    BSPNode *pNode;           // edi@2
    BLVFace *pFace;           // eax@2
    int v5;                   // ecx@2
    __int16 v6;               // ax@6
    int v7;                   // ebp@10
    int v8;                   // ebx@10
    __int16 v9;               // di@18
    BspRenderer_stru0 *node;  // [sp+18h] [bp-4h]@1

    // logger->Warning(L"sub_4406BC(%u, %u)", a1, uFirstNode);

    // v10 = a1;
    node = &pBspRenderer->nodes[node_id];
    while (1) {
        pSector = &pIndoor->pSectors[node->uSectorID];
        pNode = &pIndoor->pNodes[uFirstNode];
        pFace = &pIndoor->pFaces[pSector->pFaceIDs[pNode->uCoplanarOffset]];
        v5 = pFace->pFacePlane_old.dist +
             pIndoorCameraD3D->vPartyPos.x * pFace->pFacePlane_old.vNormal.x +
             pIndoorCameraD3D->vPartyPos.y * pFace->pFacePlane_old.vNormal.y +
             pIndoorCameraD3D->vPartyPos.z *
                 pFace->pFacePlane_old.vNormal.z;  // plane equation
        if (pFace->Portal() && pFace->uSectorID != node->uSectorID) v5 = -v5;
        // v11 = v5 > 0;
        if (v5 <= 0)
            v6 = pNode->uFront;
        else
            v6 = pNode->uBack;
        if (v6 != -1) sub_4406BC(node_id, v6);
        v7 = pNode->uCoplanarOffset;
        v8 = v7 + pNode->uCoplanarSize;

        // logger->Warning(L"Node %u: %X to %X (%hX)", uFirstNode, v7, v8,
        // v2->pFaceIDs[v7]);

        // if ( render->pRenderD3D )
        {
            while (v7 < v8)
                pBspRenderer->AddFaceToRenderList_d3d(node_id,
                                                      pSector->pFaceIDs[v7++]);
        }
        /*else
        {
          while ( v7 < v8 )
            pBspRenderer->AddFaceToRenderList_sw(node_id,
        pSector->pFaceIDs[v7++]);
        }*/
        v9 = v5 > 0 ? pNode->uFront : pNode->uBack;
        if (v9 == -1) break;
        uFirstNode = v9;
    }
}

void IndoorLocation::PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, unsigned int uSectorID) {
    unsigned int v8;       // edi@5
    int v9;                // edi@5
    int v10;               // eax@7
    SpriteFrame *v11;      // eax@7
    Particle_sw particle;  // [sp+Ch] [bp-A0h]@3
    int v30;               // [sp+8Ch] [bp-20h]@7

    if (pLevelDecorations[uDecorationID].uFlags & LEVEL_DECORATION_INVISIBLE)
        return;

    DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[uDecorationID].uDecorationDescID);

    if (decoration->uFlags & DECORATION_DESC_EMITS_FIRE) {
        memset(&particle, 0, sizeof(particle));  // fire,  like at the Pit's tavern
        particle.type =
            ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
        particle.uDiffuse = 0xFF3C1E;
        particle.x = (double)pLevelDecorations[uDecorationID].vPosition.x;
        particle.y = (double)pLevelDecorations[uDecorationID].vPosition.y;
        particle.z = (double)pLevelDecorations[uDecorationID].vPosition.z;
        particle.r = 0.0;
        particle.g = 0.0;
        particle.b = 0.0;
        particle.particle_size = 1.0;
        particle.timeToLive = (rand() & 0x80) + 128;
        particle.texture = spell_fx_renderer->effpar01;
        particle_engine->AddParticle(&particle);
        return;
    }

    if (decoration->uFlags & DECORATION_DESC_DONT_DRAW) {
        return;
    }

    v8 = pLevelDecorations[uDecorationID].field_10_y_rot +
         ((signed int)stru_5C6E00->uIntegerPi >> 3) -
         stru_5C6E00->Atan2(pLevelDecorations[uDecorationID].vPosition.x -
                                pIndoorCameraD3D->vPartyPos.x,
                            pLevelDecorations[uDecorationID].vPosition.y -
                                pIndoorCameraD3D->vPartyPos.y);
    v9 = ((signed int)(stru_5C6E00->uIntegerPi + v8) >> 8) & 7;
    int v37 = pBLVRenderParams->field_0_timer_;
    if (pParty->bTurnBasedModeOn) v37 = pMiscTimer->uTotalGameTimeElapsed;
    v10 = abs(pLevelDecorations[uDecorationID].vPosition.x +
              pLevelDecorations[uDecorationID].vPosition.y);
    v11 = pSpriteFrameTable->GetFrame(decoration->uSpriteID, v37 + v10);

    // error catching
    if (v11->icon_name == "null") __debugbreak();

    v30 = 0;
    if (v11->uFlags & 2) v30 = 2;
    if (v11->uFlags & 0x40000) v30 |= 0x40;
    if (v11->uFlags & 0x20000) v30 |= 0x80;
    if ((256 << v9) & v11->uFlags) v30 |= 4;

    int view_x = 0;
    int view_y = 0;
    int view_z = 0;
    bool visible =
        pIndoorCameraD3D->ViewClip(pLevelDecorations[uDecorationID].vPosition.x,
                                   pLevelDecorations[uDecorationID].vPosition.y,
                                   pLevelDecorations[uDecorationID].vPosition.z,
                                   &view_x, &view_y, &view_z);

    if (visible) {
        if (abs(view_x) >= abs(view_y)) {
            int projected_x = 0;
            int projected_y = 0;
            pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x,
                                      &projected_y);

            assert(uNumBillboardsToDraw < 500);
            ++uNumBillboardsToDraw;
            ++uNumDecorationsDrawnThisFrame;

            pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite =
                v11->hw_sprites[v9];

            if (v11->hw_sprites[v9]->texture->GetHeight() == 0 || v11->hw_sprites[v9]->texture->GetWidth() == 0)
                __debugbreak();

            pBillboardRenderList[uNumBillboardsToDraw - 1].uPalette =
                v11->uPaletteIndex;
            pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID =
                uSectorID;

            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x =
                pIndoorCameraD3D->fov_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y =
                pIndoorCameraD3D->fov_y;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = v11->scale.GetFloat() * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = v11->scale.GetFloat() * (int)floorf(pIndoorCameraD3D->fov_y + 0.5f) / view_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v30;
            pBillboardRenderList[uNumBillboardsToDraw - 1].world_x =
                pLevelDecorations[uDecorationID].vPosition.x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].world_y =
                pLevelDecorations[uDecorationID].vPosition.y;
            pBillboardRenderList[uNumBillboardsToDraw - 1].world_z =
                pLevelDecorations[uDecorationID].vPosition.z;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x =
                projected_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y =
                projected_y;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z =
                view_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid =
                PID(OBJECT_Decoration, uDecorationID);

            pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = 0;
            pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v11;
        }
    }
}

//----- (0043F953) --------------------------------------------------------
void PrepareBspRenderList_BLV() {
    pBspRenderer->num_faces = 0;

    if (pBLVRenderParams->uPartySectorID) {
        pBspRenderer->nodes[0].uSectorID = pBLVRenderParams->uPartySectorID;
        pBspRenderer->nodes[0].uViewportX = pBLVRenderParams->uViewportX;
        pBspRenderer->nodes[0].uViewportY = pBLVRenderParams->uViewportY;
        pBspRenderer->nodes[0].uViewportZ = pBLVRenderParams->uViewportZ;
        pBspRenderer->nodes[0].uViewportW = pBLVRenderParams->uViewportW;
        pBspRenderer->nodes[0].PortalScreenData.GetViewportData(
            pBLVRenderParams->uViewportX, pBLVRenderParams->uViewportY,
            pBLVRenderParams->uViewportZ, pBLVRenderParams->uViewportW);
        pBspRenderer->nodes[0].uFaceID = -1;
        pBspRenderer->nodes[0].viewing_portal_id = -1;
        pBspRenderer->num_nodes = 1;
        AddBspNodeToRenderList(0);
    }

    pBspRenderer->MakeVisibleSectorList();
}

//----- (0043F9E1) --------------------------------------------------------
void BspRenderer_PortalViewportData::GetViewportData(__int16 x, int y,
                                                     __int16 z, int w) {
    _viewport_space_y = y;
    _viewport_space_w = w;

    for (uint i = 0; i < window->GetHeight(); ++i) {
        if (i < y || i > w) {
            viewport_left_side[i] = window->GetWidth();
            viewport_right_side[i] = -1;
        } else {
            viewport_left_side[i] = x;
            viewport_right_side[i] = z;
        }
    }
}
//----- (0048653D) --------------------------------------------------------


bool sub_407A1C(int x, int y, int z, Vec3_int_ v) {
    unsigned int v4;  // esi@1
    int dist_y;       // edi@2
    int dist_z;       // ebx@2
    int dist_x;       // esi@2
    int v9;           // ecx@2
    int v10;          // eax@2
    int v12;          // eax@4
    int v17;          // ST34_4@25
    int v18;          // ST38_4@25
    int v19;          // eax@25
    char v20;         // zf@25
    int v21;          // ebx@25
    int v23;          // edi@26
    int v24;          // ST34_4@30
    int v32;          // ecx@37
    int v33;          // eax@37
    int v35;          // eax@39
    int v40;          // ebx@60
    int v42;          // edi@61
    int v49;          // ecx@73
    int v50;          // eax@73
    int v51;          // edx@75
    int v52;          // ecx@75
    int v53;          // eax@75
    int v59;          // eax@90
    BLVFace *face;    // esi@91
    int v63;          // ST34_4@98
    int v64;          // ST30_4@98
    int v65;          // eax@98
    int v66;          // ebx@98
    int v68;          // edi@99
    int v69;          // ST2C_4@103
    int v77;          // ecx@111
    int v78;          // eax@111
    int v79;          // edx@113
    int v80;          // ecx@113
    int v81;          // eax@113
    int v87;          // ecx@128
    int v91;          // ebx@136
    int v93;          // edi@137
    Vec3_int_ v97;    // [sp-18h] [bp-94h]@1
    int v107;         // [sp+10h] [bp-6Ch]@98
    int v108;         // [sp+10h] [bp-6Ch]@104
    int v109;         // [sp+18h] [bp-64h]@25
    int v110;         // [sp+18h] [bp-64h]@31
    int v113;         // [sp+20h] [bp-5Ch]@1
    int v114;         // [sp+24h] [bp-58h]@1
    int v119;         // [sp+34h] [bp-48h]@75
    int v120;         // [sp+34h] [bp-48h]@113
    int v121;         // [sp+38h] [bp-44h]@4
    int v122;         // [sp+38h] [bp-44h]@39
    int v123;         // [sp+38h] [bp-44h]@76
    int v124;         // [sp+38h] [bp-44h]@114
    int v125;         // [sp+3Ch] [bp-40h]@4
    int v126;         // [sp+3Ch] [bp-40h]@39
    int v127;         // [sp+3Ch] [bp-40h]@77
    int v128;         // [sp+3Ch] [bp-40h]@115
    int v129;         // [sp+40h] [bp-3Ch]@11
    int v130;         // [sp+40h] [bp-3Ch]@46
    int v131;         // [sp+40h] [bp-3Ch]@78
    int v132;         // [sp+40h] [bp-3Ch]@116
    int v133;         // [sp+44h] [bp-38h]@10
    int v134;         // [sp+44h] [bp-38h]@45
    int v135;         // [sp+44h] [bp-38h]@81
    int v136;         // [sp+44h] [bp-38h]@119
    int v137;         // [sp+48h] [bp-34h]@7
    int v138;         // [sp+48h] [bp-34h]@42
    int v139;         // [sp+48h] [bp-34h]@82
    int v140;         // [sp+48h] [bp-34h]@120
    int v141;         // [sp+4Ch] [bp-30h]@6
    int v142;         // [sp+4Ch] [bp-30h]@41
    int v143;         // [sp+4Ch] [bp-30h]@75
    int v144;         // [sp+4Ch] [bp-30h]@113
    int v145;         // [sp+50h] [bp-2Ch]@5
    int v146;         // [sp+50h] [bp-2Ch]@40
    int v149;         // [sp+54h] [bp-28h]@4
    int v150;         // [sp+54h] [bp-28h]@39
    int sDepthb;      // [sp+58h] [bp-24h]@90
    int a5b;          // [sp+5Ch] [bp-20h]@83
    int a5c;          // [sp+5Ch] [bp-20h]@121
    int v162;         // [sp+60h] [bp-1Ch]@128
    int outz;         // [sp+64h] [bp-18h]@2
    int outx;         // [sp+68h] [bp-14h]@2
    int outy;         // [sp+6Ch] [bp-10h]@2
    int sZ;           // [sp+70h] [bp-Ch]@2
    int sX;           // [sp+74h] [bp-8h]@2
    int sY;           // [sp+78h] [bp-4h]@2
                      // 8bytes unused
    int ya;           // [sp+84h] [bp+8h]@60
    int yb;           // [sp+84h] [bp+8h]@136
    int ve;           // [sp+88h] [bp+Ch]@60
    int va;           // [sp+88h] [bp+Ch]@60
    int vb;           // [sp+88h] [bp+Ch]@66
    int vf;           // [sp+88h] [bp+Ch]@136
    int vc;           // [sp+88h] [bp+Ch]@136
    int vd;           // [sp+88h] [bp+Ch]@142
    int v_4;          // [sp+8Ch] [bp+10h]@60
    int v_4a;         // [sp+8Ch] [bp+10h]@65
    int v_4b;         // [sp+8Ch] [bp+10h]@136
    int v_4c;         // [sp+8Ch] [bp+10h]@141

    // __debugbreak();срабатывает при стрельбе огненным шаром

    v4 = stru_5C6E00->Atan2(v.x - x, v.y - y);

    v113 = 0;
    v114 = 0;

    v97.z = z;
    v97.x = x;
    v97.y = y;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        Vec3_int_::Rotate(32, stru_5C6E00->uIntegerHalfPi + v4, 0, v97, &sX,
                          &sY, &sZ);
        Vec3_int_::Rotate(32, stru_5C6E00->uIntegerHalfPi + v4, 0, v, &outx,
                          &outy, &outz);
        dist_y = outy - sY;
        dist_z = outz - sZ;
        dist_x = outx - sX;
        v49 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v50 = 65536;
        if (v49) v50 = 65536 / v49;
        v51 = outx;
        v143 = dist_x * v50;
        v52 = dist_y * v50;
        v53 = dist_z * v50;

        v123 = max(outx, sX);
        v119 = min(outx, sX);

        v131 = max(outy, sY);
        v127 = min(outy, sY);

        v139 = max(outz, sZ);
        v135 = min(outz, sZ);

        for (a5b = 0; a5b < 2; a5b++) {
            if (a5b)
                v59 = pIndoor->GetSector(sX, sY, sZ);
            else
                v59 = pIndoor->GetSector(outx, outy, outz);
            // v60 = pIndoor->pSectors;
            // v61 = 116 * v59;
            // i = 116 * v59;
            // for (sDepthb = 0; *(__int16 *)((char
            // *)&pIndoor->pSectors->uNumWalls + v61)
            // + 2 * *(__int16 *)((char *)&pIndoor->pSectors->uNumFloors + v61);
            // ++sDepthb)
            for (sDepthb = 0; sDepthb < pIndoor->pSectors[v59].uNumFaces;
                 ++sDepthb) {
                face =
                    &pIndoor
                         ->pFaces[pIndoor->pSectors[v59]
                                      .pFaceIDs[sDepthb]];  // face =
                                                            // &pIndoor->pFaces[*(__int16
                                                            // *)((char
                                                            // *)&pIndoor->pSectors->pWalls
                                                            // + v61)[sDepthb]]
                v63 = fixpoint_mul(v143, face->pFacePlane_old.vNormal.x);
                v64 = fixpoint_mul(v53, face->pFacePlane_old.vNormal.z);
                v65 = fixpoint_mul(v52, face->pFacePlane_old.vNormal.y);
                v20 = v63 + v64 + v65 == 0;
                v66 = v63 + v64 + v65;
                v107 = v63 + v64 + v65;
                if (face->Portal() || v119 > face->pBounding.x2 ||
                    v123 < face->pBounding.x1 || v127 > face->pBounding.y2 ||
                    v131 < face->pBounding.y1 || v135 > face->pBounding.z2 ||
                    v139 < face->pBounding.z1 || v20)
                    continue;
                v68 = -(face->pFacePlane_old.dist +
                        sX * face->pFacePlane_old.vNormal.x +
                        sY * face->pFacePlane_old.vNormal.y +
                        sZ * face->pFacePlane_old.vNormal.z);
                if (v66 <= 0) {
                    if (face->pFacePlane_old.dist +
                            sX * face->pFacePlane_old.vNormal.x +
                            sY * face->pFacePlane_old.vNormal.y +
                            sZ * face->pFacePlane_old.vNormal.z <
                        0)
                        continue;
                } else {
                    if (face->pFacePlane_old.dist +
                            sX * face->pFacePlane_old.vNormal.x +
                            sY * face->pFacePlane_old.vNormal.y +
                            sZ * face->pFacePlane_old.vNormal.z >
                        0)
                        continue;
                }
                v69 = abs(-(face->pFacePlane_old.dist +
                            sX * face->pFacePlane_old.vNormal.x +
                            sY * face->pFacePlane_old.vNormal.y +
                            sZ * face->pFacePlane_old.vNormal.z)) >>
                      14;
                if (v69 <= abs(v66)) {
                    // LODWORD(v70) = v68 << 16;
                    // HIDWORD(v70) = v68 >> 16;
                    // v71 = v70 / v107;
                    // v108 = v70 / v107;
                    v108 = fixpoint_div(v68, v107);
                    if (v108 >= 0) {
                        if (sub_4075DB(
                                sX + ((signed int)(fixpoint_mul(v108, v143) +
                                                   0x8000) >>
                                      16),
                                sY + ((signed int)(fixpoint_mul(v108, v52) +
                                                   0x8000) >>
                                      16),
                                sZ + ((signed int)(fixpoint_mul(v108, v53) +
                                                   0x8000) >>
                                      16),
                                face)) {
                            v114 = 1;
                            break;
                        }
                    }
                }
            }
        }

        Vec3_int_::Rotate(32, v4 - stru_5C6E00->uIntegerHalfPi, 0, v97, &sX,
                          &sY, &sZ);
        Vec3_int_::Rotate(32, v4 - stru_5C6E00->uIntegerHalfPi, 0, v, &outx,
                          &outy, &outz);
        dist_y = outy - sY;
        dist_z = outz - sZ;
        dist_x = outx - sX;
        v77 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v78 = 65536;
        if (v77) v78 = 65536 / v77;
        v79 = outx;
        v144 = dist_x * v78;
        v80 = dist_y * v78;
        v81 = dist_z * v78;

        v120 = max(outx, sX);
        v124 = min(outx, sX);

        v132 = max(outy, sY);
        v128 = min(outy, sY);

        v140 = max(outz, sZ);
        v136 = min(outz, sZ);

        for (a5c = 0; a5c < 2; a5c++) {
            if (v113) return !v114 || !v113;
            if (a5c) {
                v87 = pIndoor->GetSector(sX, sY, sZ);
            } else {
                v87 = pIndoor->GetSector(outx, outy, outz);
            }
            for (v162 = 0; v162 < pIndoor->pSectors[v87].uNumFaces; v162++) {
                face = &pIndoor->pFaces[pIndoor->pSectors[v87].pFaceIDs[v162]];
                yb = fixpoint_mul(v144, face->pFacePlane_old.vNormal.x);
                v_4b = fixpoint_mul(v80, face->pFacePlane_old.vNormal.y);
                vf = fixpoint_mul(v81, face->pFacePlane_old.vNormal.z);
                v20 = yb + vf + v_4b == 0;
                v91 = yb + vf + v_4b;
                vc = yb + vf + v_4b;
                if (face->Portal() || v120 > face->pBounding.x2 ||
                    v124 < face->pBounding.x1 || v128 > face->pBounding.y2 ||
                    v132 < face->pBounding.y1 || v136 > face->pBounding.z2 ||
                    v140 < face->pBounding.z1 || v20)
                    continue;
                v93 = -(face->pFacePlane_old.dist +
                        sX * face->pFacePlane_old.vNormal.x +
                        sY * face->pFacePlane_old.vNormal.y +
                        sZ * face->pFacePlane_old.vNormal.z);
                if (v91 <= 0) {
                    if (face->pFacePlane_old.dist +
                            sX * face->pFacePlane_old.vNormal.x +
                            sY * face->pFacePlane_old.vNormal.y +
                            sZ * face->pFacePlane_old.vNormal.z <
                        0)
                        continue;
                } else {
                    if (face->pFacePlane_old.dist +
                            sX * face->pFacePlane_old.vNormal.x +
                            sY * face->pFacePlane_old.vNormal.y +
                            sZ * face->pFacePlane_old.vNormal.z >
                        0)
                        continue;
                }
                v_4c = abs(-(face->pFacePlane_old.dist +
                             sX * face->pFacePlane_old.vNormal.x +
                             sY * face->pFacePlane_old.vNormal.y +
                             sZ * face->pFacePlane_old.vNormal.z)) >>
                       14;
                if (v_4c <= abs(v91)) {
                    vd = fixpoint_div(v93, vc);
                    if (vd >= 0) {
                        if (sub_4075DB(
                                sX + ((signed int)(fixpoint_mul(vd, v144) +
                                                   0x8000) >>
                                      16),
                                sY + ((signed int)(fixpoint_mul(vd, v80) +
                                                   0x8000) >>
                                      16),
                                sZ + ((signed int)(fixpoint_mul(vd, v81) +
                                                   0x8000) >>
                                      16),
                                face)) {
                            v113 = 1;
                            break;
                        }
                    }
                }
            }
        }
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        Vec3_int_::Rotate(32, stru_5C6E00->uIntegerHalfPi + v4, 0, v97, &sX,
                          &sY, &sZ);
        Vec3_int_::Rotate(32, stru_5C6E00->uIntegerHalfPi + v4, 0, v, &outx,
                          &outy, &outz);
        dist_y = outy - sY;
        dist_z = outz - sZ;
        dist_x = outx - sX;
        v9 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v10 = 65536;
        if (v9) v10 = 65536 / v9;
        v125 = dist_x * v10;
        v12 = dist_z * v10;
        v121 = dist_y * v10;

        v145 = max(outx, sX);
        v149 = min(outx, sX);

        v137 = max(outy, sY);
        v141 = min(outy, sY);

        v129 = max(outz, sZ);
        v133 = min(outz, sZ);

        for (BSPModel &model : pOutdoor->pBModels) {
            if (sub_4088E9(sX, sY, outx, outy, model.vPosition.x,
                           model.vPosition.y) <= model.sBoundingRadius + 128) {
                for (ODMFace &face : model.pFaces) {
                    v17 = fixpoint_mul(v125, face.pFacePlane.vNormal.x);
                    v18 = fixpoint_mul(v121, face.pFacePlane.vNormal.y);
                    v19 = fixpoint_mul(v12, face.pFacePlane.vNormal.z);
                    v20 = v17 + v18 + v19 == 0;
                    v21 = v17 + v18 + v19;
                    v109 = v17 + v18 + v19;
                    if (v149 > face.pBoundingBox.x2 ||
                        v145 < face.pBoundingBox.x1 ||
                        v141 > face.pBoundingBox.y2 ||
                        v137 < face.pBoundingBox.y1 ||
                        v133 > face.pBoundingBox.z2 ||
                        v129 < face.pBoundingBox.z1 || v20)
                        continue;
                    v23 = -(face.pFacePlane.dist +
                            sX * face.pFacePlane.vNormal.x +
                            sY * face.pFacePlane.vNormal.y +
                            sZ * face.pFacePlane.vNormal.z);
                    if (v21 <= 0) {
                        if (face.pFacePlane.dist +
                                sX * face.pFacePlane.vNormal.x +
                                sY * face.pFacePlane.vNormal.y +
                                sZ * face.pFacePlane.vNormal.z <
                            0)
                            continue;
                    } else {
                        if (face.pFacePlane.dist +
                                sX * face.pFacePlane.vNormal.x +
                                sY * face.pFacePlane.vNormal.y +
                                sZ * face.pFacePlane.vNormal.z >
                            0)
                            continue;
                    }
                    v24 = abs(-(face.pFacePlane.dist +
                                sX * face.pFacePlane.vNormal.x +
                                sY * face.pFacePlane.vNormal.y +
                                sZ * face.pFacePlane.vNormal.z)) >>
                          14;
                    if (v24 <= abs(v21)) {
                        v110 = fixpoint_div(v23, v109);
                        if (v110 >= 0) {
                            if (sub_4077F1(
                                    sX +
                                        ((signed int)(fixpoint_mul(v110, v125) +
                                                      0x8000) >>
                                         16),
                                    sY +
                                        ((signed int)(fixpoint_mul(v110, v121) +
                                                      0x8000) >>
                                         16),
                                    sZ + ((signed int)(fixpoint_mul(v110, v12) +
                                                       0x8000) >>
                                          16),
                                    &face, &model.pVertices)) {
                                v114 = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }

        Vec3_int_::Rotate(32, v4 - stru_5C6E00->uIntegerHalfPi, 0, v97, &sX,
                          &sY, &sZ);
        Vec3_int_::Rotate(32, v4 - stru_5C6E00->uIntegerHalfPi, 0, v, &outx,
                          &outy, &outz);
        dist_y = outy - sY;
        dist_z = outz - sZ;
        dist_x = outx - sX;
        v32 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v33 = 65536;
        if (v32) v33 = 65536 / v32;
        v126 = dist_x * v33;
        v35 = dist_z * v33;
        v122 = dist_y * v33;

        v146 = max(outx, sX);
        v150 = min(outx, sX);

        v138 = max(outy, sY);
        v142 = min(outy, sY);

        v130 = max(outz, sZ);
        v134 = min(outz, sZ);

        for (BSPModel &model : pOutdoor->pBModels) {
            if (sub_4088E9(sX, sY, outx, outy, model.vPosition.x,
                           model.vPosition.y) <= model.sBoundingRadius + 128) {
                for (ODMFace &face : model.pFaces) {
                    ya = fixpoint_mul(v126, face.pFacePlane.vNormal.x);
                    ve = fixpoint_mul(v122, face.pFacePlane.vNormal.y);
                    v_4 = fixpoint_mul(v35, face.pFacePlane.vNormal.z);
                    v20 = ya + ve + v_4 == 0;
                    v40 = ya + ve + v_4;
                    va = ya + ve + v_4;
                    if (v150 > face.pBoundingBox.x2 ||
                        v146 < face.pBoundingBox.x1 ||
                        v142 > face.pBoundingBox.y2 ||
                        v138 < face.pBoundingBox.y1 ||
                        v134 > face.pBoundingBox.z2 ||
                        v130 < face.pBoundingBox.z1 || v20)
                        continue;
                    v42 = -(face.pFacePlane.dist +
                            sX * face.pFacePlane.vNormal.x +
                            sY * face.pFacePlane.vNormal.y +
                            sZ * face.pFacePlane.vNormal.z);
                    if (v40 <= 0) {
                        if (face.pFacePlane.dist +
                                sX * face.pFacePlane.vNormal.x +
                                sY * face.pFacePlane.vNormal.y +
                                sZ * face.pFacePlane.vNormal.z <
                            0)
                            continue;
                    } else {
                        if (face.pFacePlane.dist +
                                sX * face.pFacePlane.vNormal.x +
                                sY * face.pFacePlane.vNormal.y +
                                sZ * face.pFacePlane.vNormal.z >
                            0)
                            continue;
                    }
                    v_4a = abs(-(face.pFacePlane.dist +
                                 sX * face.pFacePlane.vNormal.x +
                                 sY * face.pFacePlane.vNormal.y +
                                 sZ * face.pFacePlane.vNormal.z)) >>
                           14;
                    if (v_4a <= abs(v40)) {
                        // LODWORD(v43) = v42 << 16;
                        // HIDWORD(v43) = v42 >> 16;
                        // vb = v43 / va;
                        vb = fixpoint_div(v42, va);
                        if (vb >= 0) {
                            if (sub_4077F1(sX + ((int)(fixpoint_mul(vb, v126) +
                                                       0x8000) >>
                                                 16),
                                           sY + ((int)(fixpoint_mul(vb, v122) +
                                                       0x8000) >>
                                                 16),
                                           sZ + ((int)(fixpoint_mul(vb, v35) +
                                                       0x8000) >>
                                                 16),
                                           &face, &model.pVertices)) {
                                v113 = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return !v114 || !v113;
}

void BspRenderer::MakeVisibleSectorList() {
    //  int v6; // ebx@3

    uNumVisibleNotEmptySectors = 0;
    for (uint i = 0; i < num_nodes; ++i) {
        // if (!uNumVisibleNotEmptySectors)
        //{
        // pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[uNumVisibleNotEmptySectors++]
        // = nodes[i].uSectorID; continue;
        //}
        // v6 = 0;
        // while (pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[v6] !=
        // nodes[i].uSectorID )
        for (uint j = 0; j < uNumVisibleNotEmptySectors; j++) {
            // ++v6;
            if (pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] ==
                nodes[i].uSectorID)
                break;
        }
        pVisibleSectorIDs_toDrawDecorsActorsEtcFrom
            [uNumVisibleNotEmptySectors++] = nodes[i].uSectorID;
    }
}

char DoInteractionWithTopmostZObject(int a1, int a2) {
    uint32_t v17 = PID_ID(a1);
    switch (PID_TYPE(a1)) {
        case OBJECT_Item: {  // take the item
            if (pSpriteObjects[v17].IsUnpickable() || v17 >= 1000 || !pSpriteObjects[v17].uObjectDescID) {
                return 1;
            }

            extern void ItemInteraction(unsigned int item_id);
            ItemInteraction(v17);
            break;
        }

        case OBJECT_Actor:
            if (pActors[v17].uAIState == Dying || pActors[v17].uAIState == Summoned)
                return 1;
            if (pActors[v17].uAIState == Dead) {
                pActors[v17].LootActor();
            } else {
                extern bool ActorInteraction(unsigned int id);
                ActorInteraction(v17);
            }
            break;

        case OBJECT_Decoration:
            extern void DecorationInteraction(unsigned int id, unsigned int pid);
            DecorationInteraction(v17, a1);
            break;

        default:
            logger->Warning(L"Warning: Invalid ID reached!");
            return 1;

        case OBJECT_BModel:
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                int bmodel_id = a1 >> 9;
                int face_id = v17 & 0x3F;
                if (bmodel_id >= pOutdoor->pBModels.size()) {
                    return 1;
                }
                if (pOutdoor->pBModels[bmodel_id].pFaces[face_id].uAttributes & FACE_HAS_EVENT ||
                    pOutdoor->pBModels[bmodel_id].pFaces[face_id].sCogTriggeredID == 0)
                    return 1;
                EventProcessor((int16_t)pOutdoor->pBModels[bmodel_id].pFaces[face_id].sCogTriggeredID,
                               a1, 1);
            } else {
                if (!(pIndoor->pFaces[v17].uAttributes & FACE_CLICKABLE)) {
                    GameUI_StatusBar_NothingHere();
                    return 1;
                }
                if (pIndoor->pFaces[v17].uAttributes & FACE_HAS_EVENT ||
                    !pIndoor->pFaceExtras[pIndoor->pFaces[v17].uFaceExtraID].uEventID)
                    return 1;
                if (current_screen_type != SCREEN_BRANCHLESS_NPC_DIALOG)
                    EventProcessor((int16_t)pIndoor->pFaceExtras[pIndoor->pFaces[v17].uFaceExtraID].uEventID,
                                   a1, 1);
            }
            return 0;
            break;
    }
    return 0;
}
//----- (0046BDF1) --------------------------------------------------------
void BLV_UpdateUserInputAndOther() {
    UpdateObjects();
    BLV_ProcessPartyActions();
    UpdateActors_BLV();
    BLV_UpdateDoors();
    check_event_triggers();
}
//----- (00424829) --------------------------------------------------------
// Finds out if current portal can be seen through the previous portal
bool PortalFrustrum(int pNumVertices,
                    BspRenderer_PortalViewportData *far_portal,
                    BspRenderer_PortalViewportData *near_portal, int uFaceID) {
    int min_y;              // esi@5
    int max_y;              // edx@5
    int current_ID;         // eax@12
    int v13;                // eax@22
    int v15;                // ecx@29
    int v18;                // eax@39
    int v19;                // eax@44
    int v20;                // ecx@44
    int v22;                // edi@46
    int v24;                // edx@48
    int v26;                // eax@55
    int v29;                // edx@57
    int v31;                // eax@64
    __int16 v36;            // dx@67
    __int16 v38;            // dx@67
    int v46;                // edx@87
    int v49;                // esi@93
    int v53;                // [sp+Ch] [bp-34h]@44
    int v54;                // [sp+10h] [bp-30h]@0
    int min_y_ID2;          // [sp+14h] [bp-2Ch]@12
    int v59;                // [sp+14h] [bp-2Ch]@87
    int v61;                // [sp+1Ch] [bp-24h]@29
    int v62;                // [sp+20h] [bp-20h]@0
    signed int direction1;  // [sp+24h] [bp-1Ch]@3
    signed int direction2;  // [sp+28h] [bp-18h]@3
    int min_y_ID;           // [sp+2Ch] [bp-14h]@5
    int v69;                // [sp+34h] [bp-Ch]@29
    int v70;                // [sp+34h] [bp-Ch]@46

    if (pNumVertices <= 1) return false;
    min_y = PortalFace._screen_space_y[0];
    min_y_ID = 0;
    max_y = PortalFace._screen_space_y[0];
    // face direction(направление фейса)
    if (!PortalFace.direction) {
        direction1 = 1;
        direction2 = -1;
    } else {
        direction1 = -1;
        direction2 = 1;
    }

    // get min and max y for portal(дать минимальное и максимальное значение y
    // для портала)
    for (uint i = 1; i < pNumVertices; ++i) {
        if (PortalFace._screen_space_y[i] < min_y) {
            min_y_ID = i;
            min_y = PortalFace._screen_space_y[i];
        } else if (PortalFace._screen_space_y[i] > max_y) {
            max_y = PortalFace._screen_space_y[i];
        }
    }
    if (max_y == min_y) return false;

    //*****************************************************************************************************************************
    far_portal->_viewport_space_y = min_y;
    far_portal->_viewport_space_w = max_y;
    current_ID = min_y_ID;
    min_y_ID2 = min_y_ID;

    for (uint i = 0; i < pNumVertices; ++i) {
        current_ID += direction2;
        if (current_ID < pNumVertices) {
            if (current_ID < 0) current_ID += pNumVertices;
        } else {
            current_ID -= pNumVertices;
        }
        if (PortalFace._screen_space_y[current_ID] <=
            PortalFace._screen_space_y[min_y_ID]) {  // определение минимальной у
            min_y_ID2 = current_ID;
            min_y_ID = current_ID;
        }
        if (PortalFace._screen_space_y[current_ID] == max_y) break;
    }

    v13 = min_y_ID2 + direction2;
    if (v13 < pNumVertices) {
        if (v13 < 0) v13 += pNumVertices;
    } else {
        v13 -= pNumVertices;
    }
    if (PortalFace._screen_space_y[v13] !=
        PortalFace._screen_space_y[min_y_ID2]) {
        v62 = PortalFace._screen_space_x[min_y_ID2] << 16;
        v54 = ((PortalFace._screen_space_x[v13] -
                PortalFace._screen_space_x[min_y_ID2])
               << 16) /
              (PortalFace._screen_space_y[v13] -
               PortalFace._screen_space_y[min_y_ID2]);
        far_portal->viewport_left_side[min_y] =
            (short)PortalFace._screen_space_x[min_y_ID2];
    }
    //****************************************************************************************************************************************
    //
    v15 = min_y_ID;
    v61 = min_y_ID;
    for (v69 = 0; v69 < pNumVertices; ++v69) {
        v15 += direction1;
        if (v15 < pNumVertices) {
            if (v15 < 0) v15 += pNumVertices;
        } else {
            v15 -= pNumVertices;
        }
        if (PortalFace._screen_space_y[v15] <=
            PortalFace._screen_space_y[min_y_ID]) {
            v61 = v15;
            min_y_ID = v15;
        }
        if (PortalFace._screen_space_y[v15] == max_y) break;
    }
    v18 = direction1 + v61;
    if (v18 < pNumVertices) {
        if (v18 < 0) v18 += pNumVertices;
    } else {
        v18 -= pNumVertices;
    }
    v19 = v18;
    v20 = v61;
    if (PortalFace._screen_space_y[v19] != PortalFace._screen_space_y[v61]) {
        v61 = PortalFace._screen_space_x[v20] << 16;
        v53 =
            ((PortalFace._screen_space_x[v19] - PortalFace._screen_space_x[v20])
             << 16) /
            (PortalFace._screen_space_y[v19] - PortalFace._screen_space_y[v20]);
        far_portal->viewport_right_side[max_y] =
            (short)PortalFace._screen_space_x[v20];
    }
    //****************************************************************************************************************************************
    v22 = min_y;
    if (min_y <= max_y) {
        for (v70 = min_y; v70 <= max_y; ++v70) {
            v24 = v13;
            if (v22 >= PortalFace._screen_space_y[v13] && v22 != max_y) {
                v13 = direction2 + v13;
                if (v13 < pNumVertices) {
                    if (v13 < 0) v13 += pNumVertices;
                } else {
                    v13 -= pNumVertices;
                }
                v26 = v13;
                if (PortalFace._screen_space_y[v26] -
                        PortalFace._screen_space_y[v24] >
                    0) {
                    v54 = ((PortalFace._screen_space_x[v26] -
                            PortalFace._screen_space_x[v24])
                           << 16) /
                          (PortalFace._screen_space_y[v26] -
                           PortalFace._screen_space_y[v24]);
                    v62 = PortalFace._screen_space_x[v24] << 16;
                }
            }
            v29 = v18;
            if (v70 >= PortalFace._screen_space_y[v18] && v70 != max_y) {
                v18 += direction1;
                if (v18 < pNumVertices) {
                    if (v18 < 0) v18 += pNumVertices;
                } else {
                    v18 -= pNumVertices;
                }
                v31 = v18;
                if (PortalFace._screen_space_y[v31] -
                        PortalFace._screen_space_y[v29] >
                    0) {
                    v53 = ((PortalFace._screen_space_x[v31] -
                            PortalFace._screen_space_x[v29])
                           << 16) /
                          (PortalFace._screen_space_y[v31] -
                           PortalFace._screen_space_y[v29]);
                    v61 = PortalFace._screen_space_x[v29] << 16;
                }
            }
            far_portal->viewport_left_side[v70] = HEXRAYS_HIWORD(v62);
            far_portal->viewport_right_side[v70] = HEXRAYS_HIWORD(v61);
            if (far_portal->viewport_left_side[v70] >
                far_portal->viewport_right_side[v70]) {
                v36 = far_portal->viewport_left_side[v70] ^
                      far_portal->viewport_right_side[v70];
                // v37 = far_portal->viewport_right_side[v70];
                far_portal->viewport_left_side[v70] = v36;
                v38 = far_portal->viewport_right_side[v70] ^ v36;
                far_portal->viewport_left_side[v70] ^= v38;
                far_portal->viewport_right_side[v70] = v38;
            }
            v62 += v54;
            v22 = v70 + 1;
            v61 += v53;
        }
    }
    //*****************************************************************************************************************************
    // check portals coordinates and determine max, min(проверка координат
    // порталов и определение макс, мин-ой у)
    if (max_y < near_portal->_viewport_space_y) return false;
    if (min_y > near_portal->_viewport_space_w) return false;
    if (min_y < near_portal->_viewport_space_y)
        min_y = near_portal->_viewport_space_y;
    if (max_y > near_portal->_viewport_space_w)
        max_y = near_portal->_viewport_space_w;
    if (min_y <= max_y) {
        for (min_y; min_y <= max_y; ++min_y) {
            if (far_portal->viewport_right_side[min_y] >=
                    near_portal->viewport_left_side[min_y] &&
                far_portal->viewport_left_side[min_y] <=
                    near_portal->viewport_right_side[min_y])
                break;
        }
    }
    if (max_y < min_y) return false;
    for (max_y; max_y >= min_y; --max_y) {
        if (far_portal->viewport_right_side[max_y] >=
                near_portal->viewport_left_side[max_y] &&
            far_portal->viewport_left_side[max_y] <=
                near_portal->viewport_right_side[max_y])
            break;
    }
    if (min_y >= max_y) return false;
    //*************************************************************************************************************************************
    v59 = min_y;
    for (v46 = max_y - min_y + 1; v46; --v46) {
        if (far_portal->viewport_left_side[v59] <
            near_portal->viewport_left_side[v59])
            far_portal->viewport_left_side[v59] =
                near_portal->viewport_left_side[v59];
        if (far_portal->viewport_right_side[v59] >
            near_portal->viewport_right_side[v59])
            far_portal->viewport_right_side[v59] =
                near_portal->viewport_right_side[v59];
        ++v59;
    }
    far_portal->_viewport_space_y = min_y;
    far_portal->_viewport_space_w = max_y;
    far_portal->_viewport_space_x = far_portal->viewport_left_side[min_y];
    far_portal->_viewport_space_z = far_portal->viewport_right_side[min_y];
    far_portal->_viewport_x_minID = min_y;
    far_portal->_viewport_z_maxID = min_y;
    v49 = min_y + 1;
    if (v49 <= max_y) {
        for (v49; v49 <= max_y; ++v49) {
            if (far_portal->viewport_left_side[v49] <
                far_portal->_viewport_space_x) {
                far_portal->_viewport_space_x =
                    far_portal->viewport_left_side[v49];
                far_portal->_viewport_x_minID = v49;
            }
            if (far_portal->viewport_right_side[v49] >
                far_portal->_viewport_space_z) {
                far_portal->_viewport_space_z =
                    far_portal->viewport_right_side[v49];
                far_portal->_viewport_z_maxID = v49;
            }
        }
    }
    return true;
}

int GetPortalScreenCoord(unsigned int uFaceID) {
    BLVFace *pFace;                 // ebx@1
    int pNextVertices;              // edx@11
    int t;                          // ST28_4@12
    int pScreenX;                   // eax@22
    int pScreenY;                   // eax@27
    int left_num_vertices;   // edi@31
    int right_num_vertices;  // ebx@41
    int top_num_vertices;    // edi@51
    int bottom_num_vertices;        // ebx@61
    bool current_vertices_flag;     // [sp+18h] [bp-10h]@9
    int depth_num_vertices;  // [sp+1Ch] [bp-Ch]@9
    bool next_vertices_flag;        // [sp+20h] [bp-8h]@10

    //Доп инфо "Программирование трёхмерных игр для windows" Ламот стр 910

    pFace = &pIndoor->pFaces[uFaceID];
    memset(&PortalFace, 0, sizeof(stru367));

    // get direction the face(определение направленности
    // фейса)*********************************************************************************
    if (pFace->pFacePlane_old.vNormal.x *
                (pIndoor->pVertices[pIndoor->pFaces[uFaceID].pVertexIDs[0]].x -
                 pIndoorCameraD3D->vPartyPos.x) +
            pFace->pFacePlane_old.vNormal.y *
                (pIndoor->pVertices[pIndoor->pFaces[uFaceID].pVertexIDs[0]].y -
                 pIndoorCameraD3D->vPartyPos.y) +
            pFace->pFacePlane_old.vNormal.z *
                (pIndoor->pVertices[pIndoor->pFaces[uFaceID].pVertexIDs[0]].z -
                 pIndoorCameraD3D->vPartyPos.z) <
        0) {
        PortalFace.direction = true;
    } else {
        PortalFace.direction = false;
        if (!(pFace->Portal())) return 0;
    }
    //*****************************************************************************************************************************************
    // transform to camera coordinates (генерация/конвертирование в координаты
    // пространства камеры)

    if ((signed int)pFace->uNumVertices > 0) {
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            pIndoorCameraD3D->ApplyViewTransform_TrueIfStillVisible_BLV(
                pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                pIndoor->pVertices[pFace->pVertexIDs[i]].y,
                pIndoor->pVertices[pFace->pVertexIDs[i]].z,
                (fixed *)&PortalFace._view_transformed_z[i + 3],
                (fixed *)&PortalFace._view_transformed_x[i + 3],
                (fixed *)&PortalFace._view_transformed_y[i + 3], false);  // xyz wrong order?

            /*pIndoorCameraD3D->ViewClip(pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                pIndoor->pVertices[pFace->pVertexIDs[i]].y,
                pIndoor->pVertices[pFace->pVertexIDs[i]].z,
                &PortalFace._view_transformed_z[i + 3],
                &PortalFace._view_transformed_x[i + 3],
                &PortalFace._view_transformed_y[i + 3], true);*/
        }
    }

    //*****************************************************************************************************************************************
    // check vertices for the nearest plane(проверка вершин есть ли в области за
    // ближайшей плоскостью)
    if (pFace->uNumVertices <= 0) return 0;
    bool bFound = false;
    for (uint i = 0; i < pFace->uNumVertices; ++i) {
        if (PortalFace._view_transformed_z[i + 3] >=
            0x80000) {  // 8.0(0x80000) 0x196A9FF >=0x80000
            bFound = true;
            break;
        }
    }
    if (!bFound) return 0;
    //*****************************************************************************************************************************************
    // check for near clip plane(проверка по ближней границе)
    //
    //     v0                 v1
    //      ._________________.
    //     /                   \
    //    /                     \
    // v5.                       . v2
    //   |                       |
    //   |                       |
    //   |                       |
    //  ---------------------------- 8.0(near_clip)
    //   |                       |
    //   ._______________________.
    //  v4                        v3
    depth_num_vertices = 0;
    PortalFace._view_transformed_z[pFace->uNumVertices + 3] =
        PortalFace._view_transformed_z[3];
    PortalFace._view_transformed_x[pFace->uNumVertices + 3] =
        PortalFace._view_transformed_x[3];
    PortalFace._view_transformed_y[pFace->uNumVertices + 3] =
        PortalFace._view_transformed_y[3];
    current_vertices_flag =
        PortalFace._view_transformed_z[3] >= 0x80000;  // 524288
    if (pFace->uNumVertices >= 1) {
        for (uint i = 1; i <= pFace->uNumVertices; ++i) {
            next_vertices_flag = PortalFace._view_transformed_z[i + 3] >=
                                 0x80000;  // 524288;// 8.0
            if (current_vertices_flag ^
                next_vertices_flag) {  // current or next vertex is near-clipped /
                                       // или текущая или следующая вершина за
                                       // ближней границей - v5
                if (next_vertices_flag) {  // next vertex is near-clipped /
                                           // следующая вершина за ближней
                                           // границей
                    // t = near_clip - v4.z / v5.z - v4.z
                    t = fixpoint_div(
                        0x80000 - PortalFace._view_transformed_z[i + 2],
                        PortalFace._view_transformed_z[i + 3] -
                            PortalFace._view_transformed_z[i + 2]);
                    // New_x = (v5.x - v4.x)*t + v4.x
                    PortalFace._view_transformed_x[depth_num_vertices] =
                        PortalFace._view_transformed_x[i + 2] +
                        fixpoint_mul(t,
                                     (PortalFace._view_transformed_x[i + 3] -
                                      PortalFace._view_transformed_x[i + 2]));
                    // New_y = (v5.y - v4.y)*t + v4.y
                    PortalFace._view_transformed_y[depth_num_vertices] =
                        PortalFace._view_transformed_y[i + 2] +
                        fixpoint_mul(t,
                                     (PortalFace._view_transformed_y[i + 3] -
                                      PortalFace._view_transformed_y[i + 2]));
                    // New_z = 8.0(0x80000)
                    PortalFace._view_transformed_z[depth_num_vertices] =
                        0x80000;  // 524288

                    // test new code
                    auto _t =
                        (fixed::FromInt(8) -
                         fixed::Raw(PortalFace._view_transformed_z[i + 2])) /
                        (fixed::Raw(PortalFace._view_transformed_z[i + 3]) -
                         fixed::Raw(PortalFace._view_transformed_z[i + 2]));
                    auto _x =
                        fixed::Raw(PortalFace._view_transformed_x[i + 2]) +
                        _t *
                            (fixed::Raw(PortalFace._view_transformed_x[i + 3]) -
                             fixed::Raw(PortalFace._view_transformed_x[i + 2]));
                    auto _y =
                        fixed::Raw(PortalFace._view_transformed_y[i + 2]) +
                        _t *
                            (fixed::Raw(PortalFace._view_transformed_y[i + 3]) -
                             fixed::Raw(PortalFace._view_transformed_y[i + 2]));
                    auto _z = fixed::FromInt(8);

                    assert(_t._internal == t);
                    assert(_x._internal ==
                           PortalFace._view_transformed_x[depth_num_vertices]);
                    assert(_y._internal ==
                           PortalFace._view_transformed_y[depth_num_vertices]);
                    assert(_z._internal ==
                           PortalFace._view_transformed_z[depth_num_vertices]);
                } else {  // current vertex is near-clipped / текущая вершина за
                          // ближней границей
                    // t = near_clip - v1.z / v0.z - v1.z
                    t = fixpoint_div(
                        524288 - PortalFace._view_transformed_z[i + 3],
                        PortalFace._view_transformed_z[i + 2] -
                            PortalFace._view_transformed_z[i + 3]);
                    // New_x = (v0.x - v1.x)*t + v1.x
                    PortalFace._view_transformed_x[depth_num_vertices] =
                        PortalFace._view_transformed_x[i + 3] +
                        fixpoint_mul(t,
                                     (PortalFace._view_transformed_x[i + 2] -
                                      PortalFace._view_transformed_x[i + 3]));
                    // New_y = (v0.x - v1.y)*t + v1.y
                    PortalFace._view_transformed_y[depth_num_vertices] =
                        PortalFace._view_transformed_y[i + 3] +
                        fixpoint_mul(t,
                                     (PortalFace._view_transformed_y[i + 2] -
                                      PortalFace._view_transformed_y[i + 3]));
                    // New_z = 8.0(0x80000)
                    PortalFace._view_transformed_z[depth_num_vertices] =
                        0x80000;  // 524288

                    // test new code
                    auto _t =
                        (fixed::FromInt(8) -
                         fixed::Raw(PortalFace._view_transformed_z[i + 3])) /
                        (fixed::Raw(PortalFace._view_transformed_z[i + 2]) -
                         fixed::Raw(PortalFace._view_transformed_z[i + 3]));
                    auto _x =
                        fixed::Raw(PortalFace._view_transformed_x[i + 3]) +
                        _t *
                            (fixed::Raw(PortalFace._view_transformed_x[i + 2]) -
                             fixed::Raw(PortalFace._view_transformed_x[i + 3]));
                    auto _y =
                        fixed::Raw(PortalFace._view_transformed_y[i + 3]) +
                        _t *
                            (fixed::Raw(PortalFace._view_transformed_y[i + 2]) -
                             fixed::Raw(PortalFace._view_transformed_y[i + 3]));
                    auto _z = fixed::FromInt(8);

                    // test new projection against old
                    // assert(_t._internal == t);
                    // assert(_x._internal ==
                    // PortalFace._view_transformed_x[depth_num_vertices]);
                    // assert(_y._internal ==
                    // PortalFace._view_transformed_y[depth_num_vertices]);
                    // assert(_z._internal ==
                    // PortalFace._view_transformed_z[depth_num_vertices]);
                }
                depth_num_vertices++;
            }
            if (next_vertices_flag) {  //если следующая вершина за ближней
                                       //границей
                pNextVertices = depth_num_vertices++;
                PortalFace._view_transformed_z[pNextVertices] =
                    PortalFace._view_transformed_z[i + 3];
                PortalFace._view_transformed_x[pNextVertices] =
                    PortalFace._view_transformed_x[i + 3];
                PortalFace._view_transformed_y[pNextVertices] =
                    PortalFace._view_transformed_y[i + 3];
            }
            current_vertices_flag = next_vertices_flag;
        }
    }

    //результат: нет моргания на границе портала(когда проходим сквозь портал)
    //************************************************************************************************************************************
    // convertion in screen coordinates(конвертирование в координаты экрана)
    PortalFace._view_transformed_z[depth_num_vertices] =
        PortalFace._view_transformed_z[0];
    PortalFace._view_transformed_x[depth_num_vertices] =
        PortalFace._view_transformed_x[0];
    PortalFace._view_transformed_y[depth_num_vertices] =
        PortalFace._view_transformed_y[0];
    for (uint i = 0; i < depth_num_vertices; ++i) {
        if ((abs(PortalFace._view_transformed_x[i]) >> 13) <=
            abs(PortalFace._view_transformed_z[i])) {
            pScreenX = fixpoint_div(PortalFace._view_transformed_x[i],
                PortalFace._view_transformed_z[i]);
        } else {
            if (PortalFace._view_transformed_x[i] >= 0) {
                if (PortalFace._view_transformed_z[i] >= 0)
                    pScreenX = 0x400000;  // 64.0
                else
                    pScreenX = 0xFFC00000;  // -63.0
            } else {
                if (PortalFace._view_transformed_z[i] >= 0)
                    pScreenX = 0xFFC00000;  // -63.0
                else
                    pScreenX = 0x400000;  // 64.0
            }
        }

        if ((abs(PortalFace._view_transformed_y[i]) >> 13) <=
            abs(PortalFace._view_transformed_z[i])) {
            pScreenY = fixpoint_div(PortalFace._view_transformed_y[i],
                PortalFace._view_transformed_z[i]);
        } else {
            if (PortalFace._view_transformed_y[i] >= 0) {
                if (PortalFace._view_transformed_z[i] >= 0)
                    pScreenY = 0x400000;  // 64.0
                else
                    pScreenY = 0xFFC00000;  // -63.0
            } else {
                if (PortalFace._view_transformed_z[i] >= 0)
                    pScreenY = 0xFFC00000;  // -63.0
                else
                    pScreenY = 0x400000;  // 64.0
            }
        }
        PortalFace._screen_space_x[i + 12] =
            pBLVRenderParams->uViewportCenterX -
            fixpoint_mul(HEXRAYS_SHIWORD(pBLVRenderParams->bsp_fov_rad),
                         pScreenX);
        PortalFace._screen_space_y[i + 12] =
            pBLVRenderParams->uViewportCenterY -
            fixpoint_mul(HEXRAYS_SHIWORD(pBLVRenderParams->bsp_fov_rad),
                         pScreenY);

        // test new projection against old
        auto _x =
            pBLVRenderParams->uViewportCenterX -
            (fixed::Raw(pBLVRenderParams->bsp_fov_rad) * fixed::Raw(pScreenX))
                .GetInt();
        auto _y =
            pBLVRenderParams->uViewportCenterY -
            (fixed::Raw(pBLVRenderParams->bsp_fov_rad) * fixed::Raw(pScreenY))
                .GetInt();
        // assert(PortalFace._screen_space_x[i + 12] == _x);
        // assert(PortalFace._screen_space_y[i + 12] == _y);
    }
    // результат: при повороте камеры, когда граница портала сдвигается к краю
    // экрана, портал остается прозрачным(видимым)

    //******************************************************************************************************************************************
    // координаты как в Ида-базе игры так и в данном проекте перевёрнутые,т.е.
    // портал который в правой части экрана имеет экранные координаты которые для
    // левой части экрана. Например, x(оригинал) = 8, у нас х =
    // 468(противоположный край экрана), точно также и с у.
    // coordinates (original and here) are flipped horizontaly, e.g. portal on
    // right side of the screen x(original) = 8 becomes x = 468 (opposite side
    // of the screen). the same holds true for y
    //
    // check for left_clip plane(порверка по левой границе)
    left_num_vertices = 0;
    PortalFace._screen_space_x[depth_num_vertices + 12] =
        PortalFace._screen_space_x[12];
    PortalFace._screen_space_y[depth_num_vertices + 12] =
        PortalFace._screen_space_y[12];
    current_vertices_flag = PortalFace._screen_space_x[12] >=
                            (signed int)pBLVRenderParams->uViewportX;  // 8.0
    if (depth_num_vertices < 1) return 0;
    for (uint i = 1; i <= depth_num_vertices; ++i) {
        next_vertices_flag = PortalFace._screen_space_x[i + 12] >=
                             (signed int)pBLVRenderParams->uViewportX;
        if (current_vertices_flag ^ next_vertices_flag) {
            if (next_vertices_flag) {
                // t = left_clip - v0.x / v1.x - v0.x
                t = fixpoint_div(pBLVRenderParams->uViewportX -
                                     PortalFace._screen_space_x[i + 11],
                                 PortalFace._screen_space_x[i + 12] -
                                     PortalFace._screen_space_x[i + 11]);
                // New_y = (v1.y - v0.y)*t + v0.y
                PortalFace._screen_space_y[left_num_vertices + 9] =
                    PortalFace._screen_space_y[i + 11] +
                    fixpoint_mul(t, (PortalFace._screen_space_y[i + 12] -
                                     PortalFace._screen_space_y[i + 11]));
                // New_x = left_clip
                PortalFace._screen_space_x[left_num_vertices + 9] =
                    pBLVRenderParams->uViewportX;

                auto _t = (fixed::FromInt(pBLVRenderParams->uViewportX) -
                           fixed::FromInt(PortalFace._screen_space_x[i + 11])) /
                          (fixed::FromInt(PortalFace._screen_space_x[i + 12]) -
                           fixed::FromInt(PortalFace._screen_space_x[i + 11]));
                auto _x = fixed::FromInt(pBLVRenderParams->uViewportX);
                auto _y =
                    fixed::FromInt(PortalFace._screen_space_y[i + 11]) +
                    _t * (fixed::FromInt(PortalFace._screen_space_y[i + 12]) -
                          fixed::FromInt(PortalFace._screen_space_y[i + 11]));

                // assert(_t._internal == t);
                // assert(_x.GetInt() ==
                // PortalFace._screen_space_x[left_num_vertices + 9]);
                // assert(_y.GetInt() ==
                // PortalFace._screen_space_y[left_num_vertices + 9]);
            } else {
                // t = left_clip - v1.x / v0.x - v1.x
                t = fixpoint_div(pBLVRenderParams->uViewportX -
                                     PortalFace._screen_space_x[i + 12],
                                 PortalFace._screen_space_x[i + 11] -
                                     PortalFace._screen_space_x[i + 12]);
                // New_y = (v0.y - v1.y)*t + v1.y
                PortalFace._screen_space_y[left_num_vertices + 9] =
                    PortalFace._screen_space_y[i + 12] +
                    fixpoint_mul(t, (PortalFace._screen_space_y[i + 11] -
                                     PortalFace._screen_space_y[i + 12]));
                // New_x = left_clip
                PortalFace._screen_space_x[left_num_vertices + 9] =
                    pBLVRenderParams->uViewportX;

                auto _t = (fixed::FromInt(pBLVRenderParams->uViewportX) -
                           fixed::FromInt(PortalFace._screen_space_x[i + 12])) /
                          (fixed::FromInt(PortalFace._screen_space_x[i + 11]) -
                           fixed::FromInt(PortalFace._screen_space_x[i + 12]));
                auto _x = fixed::FromInt(pBLVRenderParams->uViewportX);
                auto _y =
                    fixed::FromInt(PortalFace._screen_space_y[i + 12]) +
                    _t * (fixed::FromInt(PortalFace._screen_space_y[i + 11]) -
                          fixed::FromInt(PortalFace._screen_space_y[i + 12]));

                // test new projection against old
                // assert(_t._internal == t);
                // assert(_x.GetInt() ==
                // PortalFace._screen_space_x[left_num_vertices + 9]);
                // assert(_y.GetInt() ==
                // PortalFace._screen_space_y[left_num_vertices + 9]);
            }
            left_num_vertices++;
        }
        if (next_vertices_flag) {
            pNextVertices = left_num_vertices++;
            PortalFace._screen_space_x[pNextVertices + 9] =
                PortalFace._screen_space_x[i + 12];
            PortalFace._screen_space_y[pNextVertices + 9] =
                PortalFace._screen_space_y[i + 12];
        }
        current_vertices_flag = next_vertices_flag;
    }
    //*********************************************************************************************************************************
    // for right_clip plane(проверка по правой плоскости)
    right_num_vertices = 0;
    PortalFace._screen_space_x[left_num_vertices + 9] =
        PortalFace._screen_space_x[9];
    PortalFace._screen_space_y[left_num_vertices + 9] =
        PortalFace._screen_space_y[9];
    current_vertices_flag = PortalFace._screen_space_x[9] <=
                            (signed int)pBLVRenderParams->uViewportZ;  // 468.0
    if (left_num_vertices < 1) return 0;
    for (uint i = 1; i <= left_num_vertices; ++i) {
        next_vertices_flag = PortalFace._screen_space_x[i + 9] <=
                             (signed int)pBLVRenderParams->uViewportZ;
        if (current_vertices_flag ^ next_vertices_flag) {
            if (next_vertices_flag) {
                // t = right_clip - v1.x / v0.x - v1.x
                t = fixpoint_div(pBLVRenderParams->uViewportZ -
                                     PortalFace._screen_space_x[i + 8],
                                 PortalFace._screen_space_x[i + 9] -
                                     PortalFace._screen_space_x[i + 8]);
                // New_y = (v0.y - v1.y)*t + v1.y
                PortalFace._screen_space_y[right_num_vertices + 6] =
                    fixpoint_mul((PortalFace._screen_space_y[i + 9] -
                                  PortalFace._screen_space_y[i + 8]),
                                 t) +
                    PortalFace._screen_space_y[i + 8];
                // New_x = right_clip
                PortalFace._screen_space_x[right_num_vertices + 6] =
                    pBLVRenderParams->uViewportZ;
            } else {
                // t = right_clip - v0.x / v1.x - v0.x
                t = fixpoint_div(pBLVRenderParams->uViewportZ -
                                     PortalFace._screen_space_x[i + 9],
                                 PortalFace._screen_space_x[i + 8] -
                                     PortalFace._screen_space_x[i + 9]);
                // New_y = (v1.y - v0.y)*t + v0.y
                PortalFace._screen_space_y[right_num_vertices + 6] =
                    fixpoint_mul((PortalFace._screen_space_y[i + 8] -
                                  PortalFace._screen_space_y[i + 9]),
                                 t) +
                    PortalFace._screen_space_y[i + 9];
                // New_x = right_clip
                PortalFace._screen_space_x[right_num_vertices + 6] =
                    pBLVRenderParams->uViewportZ;
            }
            right_num_vertices++;
        }
        if (next_vertices_flag) {
            pNextVertices = right_num_vertices++;
            PortalFace._screen_space_x[pNextVertices + 6] =
                PortalFace._screen_space_x[i + 9];
            PortalFace._screen_space_y[pNextVertices + 6] =
                PortalFace._screen_space_y[i + 9];
        }
        current_vertices_flag = next_vertices_flag;
    }
    //************************************************************************************************************************************
    // for top clip plane
    top_num_vertices = 0;
    PortalFace._screen_space_x[right_num_vertices + 6] =
        PortalFace._screen_space_x[6];
    PortalFace._screen_space_y[right_num_vertices + 6] =
        PortalFace._screen_space_y[6];

    current_vertices_flag = PortalFace._screen_space_y[6] >=
                            (signed int)pBLVRenderParams->uViewportY;  // 8.0
    if (right_num_vertices < 1) return 0;
    for (uint i = 1; i <= right_num_vertices; ++i) {
        next_vertices_flag = PortalFace._screen_space_y[i + 6] >=
                             (signed int)pBLVRenderParams->uViewportY;
        if (current_vertices_flag ^ next_vertices_flag) {
            if (next_vertices_flag) {
                t = fixpoint_div(pBLVRenderParams->uViewportY -
                                     PortalFace._screen_space_y[i + 5],
                                 PortalFace._screen_space_y[i + 6] -
                                     PortalFace._screen_space_y[i + 5]);
                PortalFace._screen_space_x[top_num_vertices + 3] =
                    ((signed int)((PortalFace._screen_space_x[i + 6] -
                                   PortalFace._screen_space_x[i + 5]) *
                                  t) >>
                     16) +
                    PortalFace._screen_space_x[i + 5];
                PortalFace._screen_space_y[top_num_vertices + 3] =
                    pBLVRenderParams->uViewportY;
            } else {
                t = fixpoint_div(pBLVRenderParams->uViewportY -
                                     PortalFace._screen_space_y[i + 6],
                                 PortalFace._screen_space_y[i + 5] -
                                     PortalFace._screen_space_y[i + 6]);
                PortalFace._screen_space_x[top_num_vertices + 3] =
                    fixpoint_mul((PortalFace._screen_space_x[i + 5] -
                                  PortalFace._screen_space_x[i + 6]),
                                 t) +
                    PortalFace._screen_space_x[i + 6];
                PortalFace._screen_space_y[top_num_vertices + 3] =
                    pBLVRenderParams->uViewportY;
            }
            top_num_vertices++;
        }
        current_vertices_flag = next_vertices_flag;
        if (next_vertices_flag) {
            pNextVertices = top_num_vertices++;
            PortalFace._screen_space_x[pNextVertices + 3] =
                PortalFace._screen_space_x[i + 6];
            PortalFace._screen_space_y[pNextVertices + 3] =
                PortalFace._screen_space_y[i + 6];
        }
    }
    //**********************************************************************************************************************************
    // for bottom_clip plane(проверка по нижней плоскости)
    bottom_num_vertices = 0;
    PortalFace._screen_space_x[top_num_vertices + 3] =
        PortalFace._screen_space_x[3];
    PortalFace._screen_space_y[top_num_vertices + 3] =
        PortalFace._screen_space_y[3];
    current_vertices_flag = PortalFace._screen_space_y[3] <=
                            (signed int)pBLVRenderParams->uViewportW;  // 351.0
    if (top_num_vertices < 1) return 0;
    for (uint i = 1; i <= top_num_vertices; ++i) {
        next_vertices_flag = PortalFace._screen_space_y[i + 3] <=
                             (signed int)pBLVRenderParams->uViewportW;
        if (current_vertices_flag ^ next_vertices_flag) {
            if (next_vertices_flag) {
                t = fixpoint_div(pBLVRenderParams->uViewportW -
                                     PortalFace._screen_space_y[i + 2],
                                 PortalFace._screen_space_y[i + 3] -
                                     PortalFace._screen_space_y[i + 2]);
                PortalFace._screen_space_x[bottom_num_vertices] =
                    fixpoint_mul((PortalFace._screen_space_x[i + 3] -
                                  PortalFace._screen_space_x[i + 2]),
                                 t) +
                    PortalFace._screen_space_x[i + 2];
                PortalFace._screen_space_y[bottom_num_vertices] =
                    pBLVRenderParams->uViewportW;
            } else {
                t = fixpoint_div(pBLVRenderParams->uViewportW -
                                     PortalFace._screen_space_y[i + 3],
                                 PortalFace._screen_space_y[i + 2] -
                                     PortalFace._screen_space_y[i + 3]);
                PortalFace._screen_space_x[bottom_num_vertices] =
                    fixpoint_mul((PortalFace._screen_space_x[i + 2] -
                                  PortalFace._screen_space_x[i + 3]),
                                 t) +
                    PortalFace._screen_space_x[i + 3];
                PortalFace._screen_space_y[bottom_num_vertices] =
                    pBLVRenderParams->uViewportW;
            }
            bottom_num_vertices++;
        }
        if (next_vertices_flag) {
            pNextVertices = bottom_num_vertices++;
            PortalFace._screen_space_x[pNextVertices] =
                PortalFace._screen_space_x[i + 3];
            PortalFace._screen_space_y[pNextVertices] =
                PortalFace._screen_space_y[i + 3];
        }
        current_vertices_flag = next_vertices_flag;
    }
    //***************************************************************************************************************************************

    if (!bottom_num_vertices) return 0;
    PortalFace._screen_space_x[bottom_num_vertices] =
        PortalFace._screen_space_x[0];
    PortalFace._screen_space_y[bottom_num_vertices] =
        PortalFace._screen_space_y[0];
    // check for software(проверка для софтвар)
    /*if ( !render->pRenderD3D && bottom_num_vertices > 3 )
    {
      PortalFace._screen_space_x[bottom_num_vertices + 1] =
    PortalFace._screen_space_x[1];
      PortalFace._screen_space_y[bottom_num_vertices + 1] =
    PortalFace._screen_space_y[1]; thisf = PortalFace.direction == true ? 1 : -
    1; if ( bottom_num_vertices > 0 )
      {
        v62 = 1;
        v71 = 1;
        do
        {
          v63 = v62 - 1;
          v64 = v62 + 1;
          v80 = v62 + 1;
          if ( v62 - 1 >= bottom_num_vertices )
            v63 -= bottom_num_vertices;
          if ( v62 >= bottom_num_vertices )
            v62 -= bottom_num_vertices;
          if ( v64 >= bottom_num_vertices )
            v64 -= bottom_num_vertices;
          if ( thisf * ((PortalFace._screen_space_y[v64] -
    PortalFace._screen_space_y[v63])
                     * (PortalFace._screen_space_x[v62] -
    PortalFace._screen_space_x[v63])
                     - (PortalFace._screen_space_y[v62] -
    PortalFace._screen_space_y[v63])
                     * (PortalFace._screen_space_x[v64] -
    PortalFace._screen_space_x[v63])) < 0 )
          {
            v62 = v80;
            v71 = v80;
          }
          else
          {
            v62 = v71;
            v65 = v71;
            if ( v71 < bottom_num_vertices || (v65 = v71 - bottom_num_vertices,
    v71 - bottom_num_vertices < bottom_num_vertices) )
            {
              memcpy(&PortalFace._screen_space_y[v65],
    &PortalFace._screen_space_y[v65 + 1], 4 * ((unsigned int)(4 *
    (bottom_num_vertices - v65)) >> 2));
              memcpy(&PortalFace._screen_space_x[v65],
    &PortalFace._screen_space_x[v65 + 1], 4 * ((unsigned int)(4 *
    (bottom_num_vertices - v65)) >> 2));
            }
            --bottom_num_vertices;
          }
        }
        while ( v62 - 1 < bottom_num_vertices );
      }
      PortalFace._screen_space_x[bottom_num_vertices] =
    PortalFace._screen_space_x[0];
      PortalFace._screen_space_y[bottom_num_vertices] =
    PortalFace._screen_space_y[0];
    }*/
    return bottom_num_vertices;
}

//----- (004AAEA6) --------------------------------------------------------
int sub_4AAEA6_transform(RenderVertexSoft *a1) {
    double v4;  // st5@2
    double v5;  // st4@3
    float v11;  // [sp+8h] [bp-8h]@2
    float v12;  // [sp+8h] [bp-8h]@6
    float v13;  // [sp+Ch] [bp-4h]@2
    float v14;  // [sp+Ch] [bp-4h]@6

    if (pIndoorCameraD3D->sRotationX) {
        v13 = a1->vWorldPosition.x - (double)pParty->vPosition.x;
        v11 = a1->vWorldPosition.y - (double)pParty->vPosition.y;
        v4 = a1->vWorldPosition.z - (double)pParty->vPosition.z;
        // if ( render->pRenderD3D )
        //{
        v5 = v11 * pIndoorCameraD3D->fRotationYSine +
             v13 * pIndoorCameraD3D->fRotationYCosine;
        a1->vWorldViewPosition.y = v13 * pIndoorCameraD3D->fRotationYSine -
                                   v11 * pIndoorCameraD3D->fRotationYCosine;
        /*}
        else
        {
          v5 = v13 * pBLVRenderParams->fCosineY - v11 *
        pBLVRenderParams->fSineY; a1->vWorldViewPosition.y = v13 *
        pBLVRenderParams->fSineY + v11 * pBLVRenderParams->fCosineY;
        }*/
        a1->vWorldViewPosition.x = v5 * pIndoorCameraD3D->fRotationXCosine -
                                   v4 * pIndoorCameraD3D->fRotationXSine;
        a1->vWorldViewPosition.z = v5 * pIndoorCameraD3D->fRotationXSine +
                                   v4 * pIndoorCameraD3D->fRotationXCosine;
    } else {
        v14 = a1->vWorldPosition.x - (double)pParty->vPosition.x;
        v12 = a1->vWorldPosition.y - (double)pParty->vPosition.y;
        a1->vWorldViewPosition.z =
            a1->vWorldPosition.z - (double)pParty->vPosition.z;
        // if ( render->pRenderD3D )
        //{
        a1->vWorldViewPosition.x = v12 * pIndoorCameraD3D->fRotationYSine +
                                   v14 * pIndoorCameraD3D->fRotationYCosine;
        a1->vWorldViewPosition.y = v14 * pIndoorCameraD3D->fRotationYSine -
                                   v12 * pIndoorCameraD3D->fRotationYCosine;
        /*}
        else
        {
          a1->vWorldViewPosition.x = v14 * pBLVRenderParams->fCosineY - v12 *
        pBLVRenderParams->fSineY; a1->vWorldViewPosition.y = v14 *
        pBLVRenderParams->fSineY + v12 * pBLVRenderParams->fCosineY;
        }*/
    }
    return 0;
}
//----- (00472866) --------------------------------------------------------
void BLV_ProcessPartyActions() {
    int v1;                   // ebx@1
    int v2;                   // edi@1
    double v10;               // st7@27
    int new_party_z;          // esi@96
    int v38;                  // eax@96
    int v39;                  // ecx@106
    int v40;                  // eax@106
    int v42;                  // eax@120
    BLVFace *pFace;           // esi@126
    int v46;                  // ecx@133
    int v52;                  // eax@140
    int v54;                  // ebx@146
    unsigned int uFaceEvent;  // [sp+14h] [bp-4Ch]@1
    bool party_running_flag;  // [sp+1Ch] [bp-44h]@1
    bool bFeatherFall;        // [sp+24h] [bp-3Ch]@15
    unsigned int uSectorID;   // [sp+28h] [bp-38h]@1
    bool party_walking_flag;  // [sp+2Ch] [bp-34h]@1
    unsigned int uFaceID;     // [sp+30h] [bp-30h]@1
    int v80;                  // [sp+34h] [bp-2Ch]@1
    int v82;                  // [sp+3Ch] [bp-24h]@47
    int _view_angle;          // [sp+40h] [bp-20h]@47
    bool hovering;            // [sp+44h] [bp-1Ch]@1
    int new_party_y;          // [sp+48h] [bp-18h]@1
    int new_party_x;          // [sp+4Ch] [bp-14h]@1
    int party_z;              // [sp+50h] [bp-10h]@1
    int angle;                // [sp+5Ch] [bp-4h]@47

    uFaceEvent = 0;
    // v89 = pParty->uFallSpeed;
    v1 = 0;
    v2 = 0;
    new_party_x = pParty->vPosition.x;
    new_party_y = pParty->vPosition.y;
    party_z = pParty->vPosition.z;
    uSectorID = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                   pParty->vPosition.z);
    party_running_flag = false;
    party_walking_flag = false;
    hovering = false;

    uFaceID = -1;
    int floor_level =
        collide_against_floor(new_party_x, new_party_y, party_z + 40,
                              &uSectorID, &uFaceID);  //получить высоту пола

    if (pParty->bFlying) {  // отключить полёт
        pParty->bFlying = false;
        if (pParty->FlyActive())
            pOtherOverlayList
                ->pOverlays[pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID - 1]
                .field_E |= 1;
    }

    if (floor_level == -30000 || uFaceID == -1) {
        floor_level = collide_against_floor_approximate(
            new_party_x, new_party_y, party_z + 40, &uSectorID, &uFaceID);
        if (floor_level == -30000 || uFaceID == -1) {
            __debugbreak();  // level built with errors
            pParty->vPosition.x = blv_prev_party_x;
            pParty->vPosition.y = blv_prev_party_z;
            pParty->vPosition.z = blv_prev_party_y;
            pParty->uFallStartY = blv_prev_party_y;
            return;
        }
    }

    blv_prev_party_x = pParty->vPosition.x;
    blv_prev_party_z = pParty->vPosition.y;
    blv_prev_party_y = pParty->vPosition.z;
    if (!pParty->bTurnBasedModeOn) {
        static int dword_720CDC = 0;

        int v67 = OS_GetTime() / 500;
        if (dword_720CDC != v67) {
            dword_4F8580[3 * dword_4F8580[1]] = pParty->vPosition.x;
            dword_4F8580[3 * dword_4F8580[2]] = pParty->vPosition.y;
            dword_4F8580[3 * dword_4F8580[3]] = pParty->vPosition.z;
            if (dword_4F8580[0] > 60) dword_4F8580[0] = 1;

            dword_720CDC = v67;
        }
    }

    int fall_start;
    /*
    if (!pParty->FeatherFallActive())// не активно падение пера
    {
      bFeatherFall = false;
      if (!pParty->pPlayers[0].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
    // grants feather fall
          !pParty->pPlayers[1].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
          !pParty->pPlayers[2].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
          !pParty->pPlayers[3].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT))
      {
        fall_start = pParty->uFallStartY;
      }
      else// was missing
      {
          fall_start = floor_level;
          bFeatherFall = true;
          pParty->uFallStartY = floor_level;
      }
    }
    else// активно падение пера
    {
      fall_start = floor_level;
      bFeatherFall = true;
      pParty->uFallStartY = floor_level;
    }

    Reworked condition below
    */
    if (pParty->FeatherFallActive() ||
        pParty->pPlayers[0].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[1].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[2].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[3].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT)) {
        fall_start = floor_level;
        bFeatherFall = true;
        pParty->uFallStartY = floor_level;
    } else {
        bFeatherFall = false;
        fall_start = pParty->uFallStartY;
    }

    if (fall_start - party_z > 512 && !bFeatherFall &&
        party_z <= floor_level + 1) {  // повреждение от падения с высоты
        assert(~pParty->uFlags & PARTY_FLAGS_1_LANDING);  // why land in indoor?
        if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
            pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
        } else {
            for (uint i = 0; i < 4; ++i) {  // receive falling damage
                if (!pParty->pPlayers[i].HasEnchantedItemEquipped(72) &&
                    !pParty->pPlayers[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS,
                        EQUIP_BOOTS)) {
                    pParty->pPlayers[i].ReceiveDamage(
                        (pParty->uFallStartY - party_z) *
                        (0.1f * pParty->pPlayers[i].GetMaxHealth()) / 256,
                        DMGT_PHISYCAL);
                    v10 = (double)(20 - pParty->pPlayers[i].GetParameterBonus(
                        pParty->pPlayers[i]
                        .GetActualEndurance())) *
                        flt_6BE3A4_debug_recmod1 * 2.133333333333333;
                    pParty->pPlayers[i].SetRecoveryTime((signed __int64)v10);
                }
            }
        }
    }

    if (party_z > floor_level + 1) hovering = true;

    bool not_high_fall = false;

    if (party_z - floor_level <= 32) {
        pParty->uFallStartY = party_z;
        not_high_fall = true;
    }

    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer) {  //таймеры для звуков передвижения
        if (pParty->walk_sound_timer > pEventTimer->uTimeElapsed)
            pParty->walk_sound_timer -= pEventTimer->uTimeElapsed;
        else
            pParty->walk_sound_timer = 0;
    }

    if (party_z <= floor_level + 1) {  // группа ниже уровня пола
        party_z = floor_level + 1;
        pParty->uFallStartY = floor_level + 1;

        if (!hovering && pParty->floor_face_pid != uFaceID) {  // не парящие и
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_PRESSURE_PLATE)
                uFaceEvent =
                    pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID]
                        .uEventID;
        }
    }
    if (!hovering) pParty->floor_face_pid = uFaceID;

    bool on_water = false;
    if (pIndoor->pFaces[uFaceID].Fluid())  // на воде
        on_water = true;

    // v81 = pParty->uWalkSpeed;
    angle = pParty->sRotationY;
    _view_angle = pParty->sRotationX;
    v82 =
        (unsigned __int64)(pEventTimer->dt_in_some_format *
                           (signed __int64)((signed int)(pParty
                                                             ->y_rotation_speed *
                                                         stru_5C6E00
                                                             ->uIntegerPi) /
                                            180)) >>
        16;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_TurnLeft:
                if (engine->config->turn_speed > 0)
                    angle = stru_5C6E00->uDoublePiMask & (angle + engine->config->turn_speed);
                else
                    angle = stru_5C6E00->uDoublePiMask & (angle + (int)(v82 * fTurnSpeedMultiplier));
                break;
            case PARTY_TurnRight:
                if (engine->config->turn_speed > 0)
                    angle = stru_5C6E00->uDoublePiMask & (angle - engine->config->turn_speed);
                else
                    angle = stru_5C6E00->uDoublePiMask & (angle - (int)(v82 * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->turn_speed > 0)
                    angle = stru_5C6E00->uDoublePiMask & (angle + engine->config->turn_speed);
                else
                    angle = stru_5C6E00->uDoublePiMask & (angle + (int)(2.0f * fTurnSpeedMultiplier * (double)v82));
                break;

            case PARTY_FastTurnRight:
                if (engine->config->turn_speed > 0)
                    angle = stru_5C6E00->uDoublePiMask & (angle - engine->config->turn_speed);
                else
                    angle = stru_5C6E00->uDoublePiMask & (angle - (int)(2.0f * fTurnSpeedMultiplier * (double)v82));
                break;

            case PARTY_StrafeLeft:
                v2 -= fixpoint_mul(stru_5C6E00->Sin(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                v1 += fixpoint_mul(stru_5C6E00->Cos(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                party_walking_flag = true;
                break;
            case PARTY_StrafeRight:
                v2 += fixpoint_mul(stru_5C6E00->Sin(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                v1 -= fixpoint_mul(stru_5C6E00->Cos(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                party_walking_flag = true;
                break;
            case PARTY_WalkForward:
                v2 += fixpoint_mul(stru_5C6E00->Cos(angle), 2 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                v1 += fixpoint_mul(stru_5C6E00->Sin(angle), 2 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                party_walking_flag = true;
                break;
            case PARTY_WalkBackward:
                v2 -= fixpoint_mul(stru_5C6E00->Cos(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                v1 -= fixpoint_mul(stru_5C6E00->Sin(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                party_walking_flag = true;
                break;
            case PARTY_RunForward:  //Бег вперёд
                v2 += fixpoint_mul(stru_5C6E00->Cos(angle), 5 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                v1 += fixpoint_mul(stru_5C6E00->Sin(angle), 5 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                party_running_flag = true;
                break;
            case PARTY_RunBackward:
                v2 -= fixpoint_mul(stru_5C6E00->Cos(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                v1 -= fixpoint_mul(stru_5C6E00->Sin(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                party_running_flag = true;
                break;
            case PARTY_LookUp:
                _view_angle += (signed __int64)(flt_6BE150_look_up_down_dangle * 25.0);
                if (_view_angle > 128)
                    _view_angle = 128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_63, 0);
                break;
            case PARTY_LookDown:
                _view_angle += (signed __int64)(flt_6BE150_look_up_down_dangle * -25.0);
                if (_view_angle < -128)
                    _view_angle = -128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_64, 0);
                break;
            case PARTY_CenterView:
                _view_angle = 0;
                break;
            case PARTY_Jump:
                if ((!hovering ||
                     party_z <= floor_level + 6 && pParty->uFallSpeed <= 0) &&
                    pParty->field_24) {
                    hovering = true;
                    pParty->uFallSpeed =
                        (signed __int64)((double)(pParty->field_24 << 6) * 1.5 +
                                         (double)pParty->uFallSpeed);
                }
                break;
            default:
                break;
        }
    }
    pParty->sRotationY = angle;
    pParty->sRotationX = _view_angle;
    if (hovering) {  // парящие
        pParty->uFallSpeed += -2 * pEventTimer->uTimeElapsed *
                              GetGravityStrength();  // расчёт скорости падения
        if (hovering && pParty->uFallSpeed <= 0) {
            if (pParty->uFallSpeed < -500 && !pParty->bFlying) {
                for (uint pl = 1; pl <= 4; pl++) {
                    if (!pPlayers[pl]->HasEnchantedItemEquipped(72) &&
                        !pPlayers[pl]->WearsItem(ITEM_ARTIFACT_HERMES_SANDALS,
                                                 EQUIP_BOOTS))  // was 8
                        pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_SCARED,
                                                  0);
                }
            }
        } else {
            pParty->uFallStartY = party_z;
        }
    } else {  // не парящие
        if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 0x8000) {
            pParty->uFallSpeed -=
                pEventTimer->uTimeElapsed * GetGravityStrength();
            pParty->uFallStartY = party_z;
        } else {
            if (!(pParty->uFlags & PARTY_FLAGS_1_LANDING))
                pParty->uFallSpeed = 0;
            pParty->uFallStartY = party_z;
        }
    }
    if (v2 * v2 + v1 * v1 < 400) {
        v1 = 0;
        v2 = 0;
    }

    stru_721530.field_84 = -1;
    stru_721530.field_70 = 0;
    stru_721530.prolly_normal_d = pParty->field_14_radius;
    stru_721530.field_8_radius = pParty->field_14_radius / 2;
    stru_721530.field_0 = 1;
    stru_721530.height = pParty->uPartyHeight - 32;
    for (uint i = 0; i < 100; i++) {
        new_party_z = party_z;
        stru_721530.position.x = new_party_x;
        stru_721530.position.y = new_party_y;
        stru_721530.position.z = stru_721530.height + party_z + 1;

        stru_721530.normal.x = new_party_x;
        stru_721530.normal.y = new_party_y;
        stru_721530.normal.z = stru_721530.prolly_normal_d + party_z + 1;

        stru_721530.velocity.x = v2;
        stru_721530.velocity.y = v1;
        stru_721530.velocity.z = pParty->uFallSpeed;

        stru_721530.uSectorID = uSectorID;
        v38 = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
            v38 = 13312;
        }
        if (stru_721530._47050A(v38)) break;
        for (uint j = 0; j < 100; ++j) {
            _46E44E_collide_against_faces_and_portals(1);
            _46E0B2_collide_against_decorations();  //столкновения с декором
            for (v80 = 0; v80 < (signed int)uNumActors; ++v80)
                Actor::_46DF1A_collide_against_actor(
                    v80, 0);  //столкновения с монстрами
            if (_46F04E_collide_against_portals())  //столкновения с порталами
                break;
        }
        if (stru_721530.field_7C >= stru_721530.field_6C) {
            v39 = stru_721530.normal2.x;
            uSectorID = stru_721530.normal2.y;
            v40 = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
        } else {
            v39 = new_party_x +
                  fixpoint_mul(stru_721530.field_7C, stru_721530.direction.x);
            uSectorID = new_party_y + fixpoint_mul(stru_721530.field_7C,
                                                   stru_721530.direction.y);
            v40 = new_party_z +
                  fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
        }
        v42 = collide_against_floor(v39, uSectorID, v40 + 40,
                                    &stru_721530.uSectorID, &uFaceID);
        if (v42 == -30000 || v42 - new_party_z > 128) return;
        if (stru_721530.field_7C >= stru_721530.field_6C) {  // ???
            new_party_x = stru_721530.normal2.x;
            new_party_y = stru_721530.normal2.y;
            new_party_z =
                stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
            break;
        }
        new_party_x +=
            fixpoint_mul(stru_721530.field_7C, stru_721530.direction.x);
        new_party_y +=
            fixpoint_mul(stru_721530.field_7C, stru_721530.direction.y);
        uSectorID = stru_721530.uSectorID;
        stru_721530.field_70 += stru_721530.field_7C;
        unsigned long long v87 =
            new_party_z +
            fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
        if (PID_TYPE(stru_721530.pid) ==
            OBJECT_Actor) {  // invis break on actor collision    /    при
                             // столкновении с монстром
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
            }
            viewparams->bRedrawGameUI = true;
        } else if (PID_TYPE(stru_721530.pid) ==
                   OBJECT_Decoration) {  // decoration collision   /   при
                                         // столкновении с декорацией
            v54 = stru_5C6E00->Atan2(
                new_party_x -
                    pLevelDecorations[stru_721530.pid >> 3].vPosition.x,
                new_party_y -
                    pLevelDecorations[stru_721530.pid >> 3].vPosition.y);
            v2 = fixpoint_mul(stru_5C6E00->Cos(v54),
                              integer_sqrt(v2 * v2 + v1 * v1));
            v1 = fixpoint_mul(stru_5C6E00->Sin(v54),
                              integer_sqrt(v2 * v2 + v1 * v1));
        } else if (PID_TYPE(stru_721530.pid) ==
                   OBJECT_BModel) {  // при столкновении с bmodel
            pFace = &pIndoor->pFaces[(signed int)stru_721530.pid >> 3];
            if (pFace->uPolygonType == POLYGON_Floor) {  // если bmodel - пол
                if (pParty->uFallSpeed < 0) pParty->uFallSpeed = 0;
                v87 = pIndoor->pVertices[*pFace->pVertexIDs].z + 1;
                if (pParty->uFallStartY - v87 < 512) pParty->uFallStartY = v87;
                if (v2 * v2 + v1 * v1 < 400) {
                    v1 = 0;
                    v2 = 0;
                }
                if (pParty->floor_face_pid != PID_ID(stru_721530.pid) &&
                    pFace->Pressure_Plate())
                    uFaceEvent =
                        pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
            } else {  // если не пол
                v46 = pParty->uFallSpeed * pFace->pFacePlane_old.vNormal.z;
                if (pFace->uPolygonType !=
                    POLYGON_InBetweenFloorAndWall) {  // полез на холм
                    v80 = abs(v1 * pFace->pFacePlane_old.vNormal.y + v46 +
                              v2 * pFace->pFacePlane_old.vNormal.x) >>
                          16;
                    if ((stru_721530.speed >> 3) > v80)
                        v80 = stru_721530.speed >> 3;
                    v2 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.x);
                    v1 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.y);
                    pParty->uFallSpeed +=
                        fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.z);
                    // v80 = pFace->pFacePlane_old.vNormal.y;
                    v52 = stru_721530.prolly_normal_d -
                          ((pFace->pFacePlane_old.dist +
                            v87 * pFace->pFacePlane_old.vNormal.z +
                            new_party_y * pFace->pFacePlane_old.vNormal.y +
                            new_party_x * pFace->pFacePlane_old.vNormal.x) >>
                           16);
                    if (v52 > 0) {
                        new_party_x +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.x);
                        new_party_y +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.y);
                        v87 +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.z);
                    }
                    if (pParty->floor_face_pid != PID_ID(stru_721530.pid) &&
                        pFace->Pressure_Plate())
                        uFaceEvent =
                            pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                }
                if (pFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                    v80 = abs(v1 * pFace->pFacePlane_old.vNormal.y + v46 +
                              v2 * pFace->pFacePlane_old.vNormal.x) >>
                          16;
                    if ((stru_721530.speed >> 3) > v80)
                        v80 = stru_721530.speed >> 3;
                    v2 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.x);
                    v1 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.y);
                    pParty->uFallSpeed +=
                        fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.z);
                    if (v2 * v2 + v1 * v1 >= 400) {
                        if (pParty->floor_face_pid != PID_ID(stru_721530.pid) &&
                            pFace->Pressure_Plate())
                            uFaceEvent =
                                pIndoor->pFaceExtras[pFace->uFaceExtraID]
                                    .uEventID;
                    } else {
                        v2 = 0;
                        v1 = 0;
                        pParty->uFallSpeed = 0;
                    }
                }
            }
        }
        v2 = fixpoint_mul(58500, v2);
        v1 = fixpoint_mul(58500, v1);
        pParty->uFallSpeed = fixpoint_mul(58500, pParty->uFallSpeed);
    }

    //  //Воспроизведение звуков ходьбы/бега-------------------------
    uint pX_ = abs(pParty->vPosition.x - new_party_x);
    uint pY_ = abs(pParty->vPosition.y - new_party_y);
    uint pZ_ = abs(pParty->vPosition.z - new_party_z);
    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);  // stop sound
        if (party_running_flag &&
            (!hovering || not_high_fall)) {  // Бег и (не прыжок или не высокое падение )
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 16) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_RunWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_RunCarpet, -1 /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_RunWood, -1 /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 96;  // 64
            }
        } else if (party_walking_flag &&
                   (!hovering || not_high_fall)) {  // Ходьба и (не прыжок или не
                                                    // высокое падение)
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 8) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_WalkWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_WalkCarpet, -1 /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_WalkWood, -1 /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 144;  // 64
            }
        }
    }
    if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) <
        8)  //отключить  звук ходьбы при остановке
        pAudioPlayer->StopAll(804);
    //-------------------------------------------------------------
    if (!hovering || !not_high_fall)
        pParty->uFlags &= ~PARTY_FLAGS_1_FALLING;
    else
        pParty->uFlags |= PARTY_FLAGS_1_FALLING;
    pParty->uFlags &= ~PARTY_FLAGS_1_BURNING;
    pParty->vPosition.x = new_party_x;
    pParty->vPosition.z = new_party_z;
    pParty->vPosition.y = new_party_y;
    // pParty->uFallSpeed = v89;
    if (!hovering && pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_LAVA)
        pParty->uFlags |= PARTY_FLAGS_1_BURNING;  // 0x200
    if (uFaceEvent) EventProcessor(uFaceEvent, 0, 1);
}

//----- (00449A49) --------------------------------------------------------
void Door_switch_animation(unsigned int uDoorID, int a2) {
    int old_state;       // eax@1
    signed int door_id;  // esi@2

    if (!pIndoor->pDoors) return;
    for (door_id = 0; door_id < 200; ++door_id) {
        if (pIndoor->pDoors[door_id].uDoorID == uDoorID) break;
    }
    if (door_id >= 200) {
        Error("Unable to find Door ID: %i!", uDoorID);
    }
    old_state = pIndoor->pDoors[door_id].uState;
    // old_state: 0 - в нижнем положении/закрыто
    //           2 - в верхнем положении/открыто,
    // a2: 1 - открыть
    //    2 - опустить/поднять
    if (a2 == 2) {
        if (pIndoor->pDoors[door_id].uState == BLVDoor::Closing ||
            pIndoor->pDoors[door_id].uState == BLVDoor::Opening)
            return;
        if (pIndoor->pDoors[door_id].uState) {
            if (pIndoor->pDoors[door_id].uState != BLVDoor::Closed &&
                pIndoor->pDoors[door_id].uState != BLVDoor::Closing) {
                pIndoor->pDoors[door_id].uState = BLVDoor::Closing;
                if (old_state == BLVDoor::Open) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
                    return;
                }
                if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered =
                        (pIndoor->pDoors[door_id].uMoveLength << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed -
                        ((signed int)(pIndoor->pDoors[door_id]
                                          .uTimeSinceTriggered *
                                      pIndoor->pDoors[door_id].uCloseSpeed) /
                             128
                         << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed;
                    return;
                }
                pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
            }
            return;
        }
    } else {
        if (a2 == 0) {
            if (pIndoor->pDoors[door_id].uState != BLVDoor::Closed &&
                pIndoor->pDoors[door_id].uState != BLVDoor::Closing) {
                pIndoor->pDoors[door_id].uState = BLVDoor::Closing;
                if (old_state == BLVDoor::Open) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
                    return;
                }
                if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered =
                        (pIndoor->pDoors[door_id].uMoveLength << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed -
                        ((signed int)(pIndoor->pDoors[door_id]
                                          .uTimeSinceTriggered *
                                      pIndoor->pDoors[door_id].uCloseSpeed) /
                             128
                         << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed;
                    return;
                }
                pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
            }
            return;
        }
        if (a2 != 1) return;
    }
    if (old_state != BLVDoor::Open && old_state != BLVDoor::Opening) {
        pIndoor->pDoors[door_id].uState = BLVDoor::Opening;
        if (old_state == BLVDoor::Closed) {
            pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
            return;
        }
        if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
            pIndoor->pDoors[door_id].uTimeSinceTriggered =
                (pIndoor->pDoors[door_id].uMoveLength << 7) /
                    pIndoor->pDoors[door_id].uCloseSpeed -
                ((signed int)(pIndoor->pDoors[door_id].uTimeSinceTriggered *
                              pIndoor->pDoors[door_id].uOpenSpeed) /
                     128
                 << 7) /
                    pIndoor->pDoors[door_id].uCloseSpeed;
            return;
        }
        pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
    }
    return;
}

//----- (004088E9) --------------------------------------------------------
int sub_4088E9(int x1, int y1, int x2, int y2, int x3, int y3) {
    signed int result;  // eax@1

    result =
        integer_sqrt(abs(x2 - x1) * abs(x2 - x1) + abs(y2 - y1) * abs(y2 - y1));
    if (result)
        result = abs(((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / result);
    return result;
}

int GetAlertStatus() {
    int result;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        result = pOutdoor->ddm.field_C_alert;
    else
        result = uCurrentlyLoadedLevelType == LEVEL_Outdoor ? pIndoor->dlv.field_C_alert : 0;

    return result;
}

int _45063B_spawn_some_monster(MapInfo *a1, int a2) {
    int result;            // eax@8
    int v6;                // edi@11
    int v7;                // ebx@11
    int v9;                // ebx@12
    int v10;               // eax@12
    char v11;              // zf@16
    int v12;               // edi@20
    int v13;               // eax@20
    int v14;               // ebx@20
    int v15;               // eax@20
    int v16;               // eax@20
    int v17;               // eax@20
    int v18;               // eax@21
    SpawnPointMM7 v19;     // [sp+Ch] [bp-38h]@1
    int v22;               // [sp+2Ch] [bp-18h]@3
    unsigned int uFaceID;  // [sp+38h] [bp-Ch]@10
    int v26;               // [sp+3Ch] [bp-8h]@11
    int v27;               // [sp+40h] [bp-4h]@11

    if (!uNumActors) return 0;

    for (uint mon_id = 0; mon_id < uNumActors; ++mon_id) {
        if ((pActors[mon_id].pMonsterInfo.uID < 121 ||
             pActors[mon_id].pMonsterInfo.uID > 123) &&  // Dwarf FemaleC A-C
            (pActors[mon_id].pMonsterInfo.uID < 124 ||
             pActors[mon_id].pMonsterInfo.uID > 126) &&  // Dwarf MaleA A-C
            (pActors[mon_id].pMonsterInfo.uID < 133 ||
             pActors[mon_id].pMonsterInfo.uID > 135) &&  // Peasant Elf FemaleA A-C
            pActors[mon_id].CanAct()) {
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                v22 = 0;
                uint face_id = 0;
                for (face_id; face_id < 100; ++face_id) {
                    v6 = rand() % 1024 + 512;
                    v7 = rand() % (signed int)stru_5C6E00->uIntegerDoublePi;
                    v19.vPosition.x = pParty->vPosition.x + fixpoint_mul(stru_5C6E00->Cos(v7), v6);
                    v19.uIndex = a2;
                    v19.vPosition.y = fixpoint_mul(stru_5C6E00->Sin(v7), v6) + pParty->vPosition.y;
                    v19.vPosition.z = pParty->vPosition.z;
                    bool bInWater = false;
                    v27 = 0;
                    v19.vPosition.z = ODM_GetFloorLevel(
                        v19.vPosition.x, v19.vPosition.y, pParty->vPosition.z,
                        0, &bInWater, &v27, 0);
                    for (BSPModel &model : pOutdoor->pBModels) {
                        v9 = abs(v19.vPosition.y - model.vBoundingCenter.y);
                        v10 = abs(v19.vPosition.x - model.vBoundingCenter.x);
                        if (int_get_vector_length(v10, v9, 0) <
                            model.sBoundingRadius + 256) {
                            v22 = 1;
                            break;
                        }
                    }
                    if (v22) {
                        v11 = face_id == 100;
                        break;
                    }
                }
                v11 = face_id == 100;
            } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                v22 =
                    pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                       pParty->vPosition.z);
                for (uint i = 0; i < 100; ++i) {
                    v12 = rand() % 512 + 256;
                    v13 = rand();
                    v14 = v13 % (int)stru_5C6E00->uIntegerDoublePi;
                    v15 = stru_5C6E00->Cos(v13 % (int)stru_5C6E00->uIntegerDoublePi);
                    v19.vPosition.x = pParty->vPosition.x + fixpoint_mul(v15, v12);
                    v16 = stru_5C6E00->Sin(v13 % (int)stru_5C6E00->uIntegerDoublePi);
                    v19.vPosition.y = fixpoint_mul(v16, v12) + pParty->vPosition.y;
                    v19.vPosition.z = pParty->vPosition.z;
                    v19.uIndex = a2;
                    v17 = pIndoor->GetSector(v19.vPosition.x, v19.vPosition.y, pParty->vPosition.z);
                    if (v17 == v22) {
                        v18 = BLV_GetFloorLevel(v19.vPosition.x, v19.vPosition.y,
                                                v19.vPosition.z, v17, &uFaceID);
                        v19.vPosition.z = v18;
                        if (v18 != -30000) {
                            if (abs(v18 - pParty->vPosition.z) <= 1024) break;
                        }
                    }
                }
                v11 = v26 == 100;
            }

            if (v11) {
                result = 0;
            } else {
                SpawnEncounter(a1, &v19, 0, 0, 1);
                result = a2;
            }
        }

        // break;
        // v22 = v3->pMonsterInfo.uID - 1;
        // v4 = (signed __int64)((double)v22 * 0.3333333333333333);
        // if ( (int)v4 != 40 )
        //{
        //  if ( (int)v4 != 41 && (int)v4 != 44 && v3->CanAct() )
        //    break;
        //}
        // ++v2;
        // ++v3;
        // if ( v2 >= (signed int)uNumActors )
        //  goto LABEL_8;
    }
    return result;
}

int sub_450521_ProllyDropItemAt(int ecx0, int a2, int a3, int a4, int a5, uint16_t a6) {
    SpriteObject a1;
    pItemsTable->GenerateItem(ecx0, a2, &a1.containing_item);
    a1.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1.containing_item.uItemID].uSpriteID;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.vPosition.x = a3;
    a1.vPosition.y = a4;
    a1.vPosition.z = a5;
    a1.uFacing = a6;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a3, a4, a5);
    a1.uSpriteFrameID = 0;
    return a1.Create(0, 0, 0, 0);
}

bool sub_4075DB(int x, int y, int z, BLVFace *face) {
    int v8;          // edi@2
    int v25;  // eax@22
    bool result;     // eax@25
    int a3a;  // [sp+24h] [bp+8h]@14
    int a4a;         // [sp+28h] [bp+Ch]@2

    std::array<int, 52> dword_4F5CC8_ys;  // idb
    std::array<int, 52> dword_4F5D98_xs;  // idb

    if (face->uAttributes & FACE_XY_PLANE) {
        a4a = x;
        v8 = y;
        for (int i = 0; i < face->uNumVertices; i++) {
            dword_4F5D98_xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].x;
            dword_4F5CC8_ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].y;
        }
    } else {
        v8 = z;
        if (face->uAttributes & FACE_XZ_PLANE) {
            a4a = x;
            for (int i = 0; i < face->uNumVertices; i++) {
                dword_4F5D98_xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].x;
                dword_4F5CC8_ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].z;
            }
        } else {
            a4a = y;
            for (int i = 0; i < face->uNumVertices; i++) {
                dword_4F5D98_xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].y;
                dword_4F5CC8_ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].z;
            }
        }
    }
    a3a = 0;
    dword_4F5D98_xs[face->uNumVertices] = dword_4F5D98_xs[0];
    dword_4F5CC8_ys[face->uNumVertices] = dword_4F5CC8_ys[0];
    for (int i = 0; i < face->uNumVertices && a3a < 2; i++) {
        if (dword_4F5CC8_ys[i] >= v8 ^ (dword_4F5CC8_ys[i + 1] >= v8)) {
            // if( dword_4F5D98_xs[i + 1] >= a4a || dword_4F5D98_xs[i] >= a4a)
            if (!(dword_4F5D98_xs[i + 1] >= a4a && dword_4F5D98_xs[i] < a4a)) {
                if ((dword_4F5D98_xs[i + 1] < a4a && dword_4F5D98_xs[i] >= a4a)) {
                    ++a3a;
                    // || (v25 = dword_4F5D98_xs[i + 1] -
                    // dword_4F5D98_xs[i],LODWORD(v26) = v25 << 16, HIDWORD(v26) =
                    // v25 >> 16, dword_4F5D98_xs[i] + ((signed int)(((unsigned
                    // __int64)(v26 / (dword_4F5CC4_ys[i + 2] - dword_4F5CC4_ys[i +
                    // 1])* ((v8 - dword_4F5CC4_ys[i + 1]) << 16)) >> 16)
                    //                + 32768) >> 16) >= a4a) )
                } else {
                    v25 = fixpoint_div(dword_4F5D98_xs[i + 1] - dword_4F5D98_xs[i],
                                       dword_4F5CC8_ys[i + 1] - dword_4F5CC8_ys[i]);
                    if (dword_4F5D98_xs[i] + (fixpoint_mul(v25, (v8 - dword_4F5CC8_ys[i]) << 16) + 0x8000 >> 16) >= a4a)
                        ++a3a;
                }
            }
        }
    }
    result = 1;
    if (a3a != 1) result = 0;
    return result;
}

bool sub_4077F1(int a1, int a2, int a3, ODMFace *face, BSPVertexBuffer *a5) {
    int a4a;         // [sp+28h] [bp+Ch]@2
    int a5a;  // [sp+2Ch] [bp+10h]@14

    std::array<int, 52> dword_4F5B24_ys;  // idb
    std::array<int, 52> dword_4F5BF4_xs;  // idb

    // __debugbreak(); // срабатывает при нападении стрекозавров с огнём

    if (face->uAttributes & FACE_XY_PLANE) {
        a4a = a1;
        a3 = a2;
        for (int i = 0; i < face->uNumVertices; i++) {
            dword_4F5BF4_xs[i + 1] = a5->pVertices[face->pVertexIDs[i]].x;
            dword_4F5B24_ys[i + 1] = a5->pVertices[face->pVertexIDs[i]].y;
        }
    } else {
        if (face->uAttributes & FACE_XY_PLANE) {
            a4a = a1;
            for (int i = 0; i < face->uNumVertices; i++) {
                dword_4F5BF4_xs[i + 1] = a5->pVertices[face->pVertexIDs[i]].x;
                dword_4F5B24_ys[i + 1] = a5->pVertices[face->pVertexIDs[i]].z;
            }
        } else {
            a4a = a2;
            for (int i = 0; i < face->uNumVertices; i++) {
                dword_4F5BF4_xs[i + 1] = a5->pVertices[face->pVertexIDs[i]].y;
                dword_4F5B24_ys[i + 1] = a5->pVertices[face->pVertexIDs[i]].z;
            }
        }
    }
    a5a = 0;
    dword_4F5BF4_xs[face->uNumVertices + 1] = dword_4F5BF4_xs[1];
    dword_4F5B24_ys[face->uNumVertices + 1] = dword_4F5B24_ys[1];
    for (int i = 0; i < face->uNumVertices; i++) {
        if (a5a >= 2) break;
        if (dword_4F5B24_ys[i + 1] >= a3 ^ (dword_4F5B24_ys[i + 2] >= a3)) {
            if (dword_4F5BF4_xs[i + 2] >= a4a || dword_4F5BF4_xs[i] >= a4a) {
                if (dword_4F5BF4_xs[i + 2] >= a4a &&
                    dword_4F5BF4_xs[i + 1] >= a4a) {
                    ++a5a;
                } else {
                    // v23 = (__int64)(dword_4F5BF4_xs[i + 2] -
                    // dword_4F5BF4_xs[i + 1]) << 16;
                    __int64 _a =
                        dword_4F5B24_ys[i + 2] - dword_4F5B24_ys[i + 1];
                    __int64 _b = (__int64)(a3 - dword_4F5B24_ys[i + 1]) << 16;

                    if (dword_4F5BF4_xs[i + 1] +
                            ((((((__int64)(dword_4F5BF4_xs[i + 2] -
                                           dword_4F5BF4_xs[i + 1])
                                 << 16) /
                                _a * _b) >>
                               16) +
                              0x8000) >>
                             16) >=
                        a4a)
                        ++a5a;
                }
            }
        }
    }

    if (a5a != 1) return false;
    return true;
}

//----- (0049B04D) --------------------------------------------------------
void stru154::GetFacePlaneAndClassify(ODMFace *a2, BSPVertexBuffer *a3) {
    Vec3_float_ OutPlaneNorm;
    float OutPlaneDist;

    OutPlaneNorm.x = 0.0;
    OutPlaneNorm.y = 0.0;
    OutPlaneNorm.z = 0.0;
    GetFacePlane(a2, a3, &OutPlaneNorm, &OutPlaneDist);

    if (fabsf(a2->pFacePlane.vNormal.z) < 1e-6f)
        polygonType = POLYGON_VerticalWall;
    else if (fabsf(a2->pFacePlane.vNormal.x) < 1e-6f &&
             fabsf(a2->pFacePlane.vNormal.y) < 1e-6f)
        polygonType = POLYGON_Floor;
    else
        polygonType = POLYGON_InBetweenFloorAndWall;

    face_plane.vNormal.x = OutPlaneNorm.x;
    face_plane.vNormal.y = OutPlaneNorm.y;
    face_plane.vNormal.z = OutPlaneNorm.z;
    face_plane.dist = OutPlaneDist;
}

//----- (0049B0C9) --------------------------------------------------------
void stru154::ClassifyPolygon(Vec3_float_ *pNormal, float dist) {
    if (fabsf(pNormal->z) < 1e-6f)
        polygonType = POLYGON_VerticalWall;
    else if (fabsf(pNormal->x) < 1e-6f && fabsf(pNormal->y) < 1e-6f)
        polygonType = POLYGON_Floor;
    else
        polygonType = POLYGON_InBetweenFloorAndWall;

    face_plane.vNormal.x = pNormal->x;
    face_plane.dist = dist;
    face_plane.vNormal.y = pNormal->y;
    face_plane.vNormal.z = pNormal->z;
}

//----- (0049B13D) --------------------------------------------------------
void stru154::GetFacePlane(ODMFace *pFace, BSPVertexBuffer *pVertices,
                           Vec3_float_ *pOutNormal, float *pOutDist) {
    Vec3_float_ *v19;  // eax@3
    Vec3_float_ v2;    // [sp+4h] [bp-64h]@3
    float v26;         // [sp+1Ch] [bp-4Ch]@3
    float v27;         // [sp+20h] [bp-48h]@3
    float v28;         // [sp+24h] [bp-44h]@3
    Vec3_float_ v1;    // [sp+40h] [bp-28h]@1
    Vec3_float_ v38;   // [sp+58h] [bp-10h]@3

    v1.x = 0.0;
    v1.y = 0.0;
    v1.z = 0.0;

    if (pFace->uNumVertices >= 2) {
        for (int i = 0; i < pFace->uNumVertices - 2; i++) {
            v1.x = pVertices->pVertices[pFace->pVertexIDs[i + 1]].x -
                   pVertices->pVertices[pFace->pVertexIDs[i]].x;
            v1.y = pVertices->pVertices[pFace->pVertexIDs[i + 1]].y -
                   pVertices->pVertices[pFace->pVertexIDs[i]].y;
            v1.z = pVertices->pVertices[pFace->pVertexIDs[i + 1]].z -
                   pVertices->pVertices[pFace->pVertexIDs[i]].z;

            v26 = pVertices->pVertices[pFace->pVertexIDs[i + 2]].x -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].x;
            v27 = pVertices->pVertices[pFace->pVertexIDs[i + 2]].y -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].y;
            v28 = pVertices->pVertices[pFace->pVertexIDs[i + 2]].z -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].z;

            v19 = Vec3_float_::Cross(&v1, &v2, v26, v27, v28);
            v38.x = v19->x;
            v38.y = v19->y;
            v38.z = v19->z;
            if (v38.x != 0.0 || v38.y != 0.0 || v38.z != 0.0) {
                v38.Normalize();

                pOutNormal->x = v38.x;
                pOutNormal->y = v38.y;
                pOutNormal->z = v38.z;

                *pOutDist =
                    -(pVertices->pVertices[pFace->pVertexIDs[i]].x * v38.x +
                      pVertices->pVertices[pFace->pVertexIDs[i]].y * v38.y +
                      pVertices->pVertices[pFace->pVertexIDs[i]].z * v38.z);
                return;
            }
        }
    }

    pOutNormal->x = (double)(pFace->pFacePlane.vNormal.x & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.x >> 16);
    pOutNormal->y = (double)(pFace->pFacePlane.vNormal.y & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.y >> 16);
    pOutNormal->z = (double)(pFace->pFacePlane.vNormal.z & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.z >> 16);
    *pOutDist = (double)(pFace->pFacePlane.dist & 0xFFFF) / 65535.0f +
                (double)(pFace->pFacePlane.dist >> 16);
}

//----- (0043F515) --------------------------------------------------------
void FindBillboardsLightLevels_BLV() {
    for (uint i = 0; i < uNumBillboardsToDraw; ++i) {
        if (pBillboardRenderList[i].field_1E & 2 ||
            uCurrentlyLoadedLevelType == LEVEL_Indoor &&
                !pBillboardRenderList[i].uIndoorSectorID)
            pBillboardRenderList[i].dimming_level = 0;
        else
            pBillboardRenderList[i].dimming_level =
                _43F55F_get_billboard_light_level(&pBillboardRenderList[i], -1);
    }
}

//----- (0047272C) --------------------------------------------------------
int collide_against_floor_approximate(int x, int y, int z,
                                      unsigned int *pSectorID,
                                      unsigned int *pFaceID) {
    int result;  // eax@1

    *pSectorID = pIndoor->GetSector(x - 2, y, z + 40);
    result = collide_against_floor(x - 2, y, z + 40, pSectorID, pFaceID);
    if (result == -30000 || !*pSectorID) {
        *pSectorID = pIndoor->GetSector(x + 2, y, z + 40);
        result = collide_against_floor(x + 2, y, z + 40, pSectorID, pFaceID);
        if (result == -30000 || !*pSectorID) {
            *pSectorID = pIndoor->GetSector(x, y - 2, z + 40);
            result =
                collide_against_floor(x, y - 2, z + 40, pSectorID, pFaceID);
            if (result == -30000 || !*pSectorID) {
                *pSectorID = pIndoor->GetSector(x, y + 2, z + 40);
                result =
                    collide_against_floor(x, y + 2, z + 40, pSectorID, pFaceID);
                if (result == -30000 || !*pSectorID) {
                    *pSectorID = pIndoor->GetSector(x, y, z + 140);
                    result = collide_against_floor(x, y, z + 140, pSectorID,
                                                   pFaceID);
                }
            }
        }
    }
    return result;
}

//----- (0047050A) --------------------------------------------------------
int stru141_actor_collision_object::_47050A(int dt) {
    int v7;             // eax@1
    signed int result;  // eax@4
    int v17;            // eax@5
    int v18;            // eax@7
    int v21;            // eax@9
    int v22;            // eax@11

    int speed = 1 | integer_sqrt(this->velocity.z * this->velocity.z +
                                 this->velocity.y * this->velocity.y +
                                 this->velocity.x * this->velocity.x);

    this->direction.x = 65536 / speed * this->velocity.x;
    this->direction.y = 65536 / speed * this->velocity.y;
    this->direction.z = 65536 / speed * this->velocity.z;

    this->speed = speed;
    this->inv_speed = 65536 / speed;

    if (dt)
        v7 = dt;
    else
        v7 = pEventTimer->dt_in_some_format;

    // v8 = fixpoint_mul(v7, speed) - this->field_70; // speed * dt - something
    this->field_6C = fixpoint_mul(v7, speed) - this->field_70;
    if (this->field_6C > 0) {
        // v10 = fixpoint_mul(v8, this->direction.x) + this->normal.x;
        this->field_4C =
            fixpoint_mul(this->field_6C, this->direction.x) + this->normal.x;
        this->normal2.x =
            fixpoint_mul(this->field_6C, this->direction.x) + this->normal.x;
        // v11 = fixpoint_mul(this->field_6C, this->direction.y) +
        // this->normal.y;
        this->field_50 =
            fixpoint_mul(this->field_6C, this->direction.y) + this->normal.y;
        this->normal2.y =
            fixpoint_mul(this->field_6C, this->direction.y) + this->normal.y;
        this->normal2.z =
            fixpoint_mul(this->field_6C, this->direction.z) + this->normal.z;
        // v12 = this->position.z;
        // v13 = this->normal.x;
        // v14 = this->normal2.x;
        // v15 = this->prolly_normal_d;
        // v16 = this->position.z + fixpoint_mul(this->field_6C,
        // this->direction.z); v28 = this->position.z +
        // fixpoint_mul(this->field_6C, this->direction.z);
        this->field_54 =
            this->position.z + fixpoint_mul(this->field_6C, this->direction.z);
        v17 = this->normal.x;
        if (v17 >= this->normal2.x) v17 = this->normal2.x;
        this->sMaxX = v17 - this->prolly_normal_d;
        v18 = this->prolly_normal_d + this->normal.x;
        if (this->normal.x <= this->normal2.x)
            v18 = this->prolly_normal_d + this->normal2.x;
        // v19 = this->normal.y;
        // v20 = this->normal2.y;
        this->sMinX = v18;
        v21 = this->normal.y;
        if (v21 >= this->normal2.y) v21 = this->normal2.y;
        this->sMaxY = v21 - this->prolly_normal_d;
        v22 = this->prolly_normal_d + this->normal.y;
        if (this->normal.y <= this->normal2.y)
            v22 = this->normal2.y + this->prolly_normal_d;
        // v23 = this->normal2.z;
        this->sMinY = v22;
        // v24 = this->normal.z;
        if (this->normal.z >= this->normal2.z)
            this->sMaxZ = this->normal2.z - this->prolly_normal_d;
        else
            this->sMaxZ = this->normal.z - this->prolly_normal_d;
        // this->sMaxZ = v25;
        // v26 = this->field_8_radius;
        if (this->position.z <=
            this->position.z + fixpoint_mul(this->field_6C, this->direction.z))
            this->sMinZ = (this->position.z +
                           fixpoint_mul(this->field_6C, this->direction.z)) +
                          this->field_8_radius;
        else
            this->sMinZ = this->position.z + this->field_8_radius;
        this->pid = 0;
        this->field_80 = -1;
        this->field_88 = -1;
        // this->sMinZ = v27;
        this->field_7C = 0xFFFFFFu;
        result = 0;
    } else {
        result = 1;
    }
    return result;
}
