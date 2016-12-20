#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Engine/OurMath.h"
#include "Engine/stru6.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/MMT.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

#include "Level/Decoration.h"

#include "Media/MediaPlayer.h"

#include "GUI/GUIWindow.h"

#include "IO/Mouse.h"

#include "Render.h"
#include "Sprites.h"
#include "GammaControl.h"
#include "DecalBuilder.h"
#include "ParticleEngine.h"
#include "Outdoor.h"
#include "Viewport.h"
#include "PaletteManager.h"
#include "LightmapBuilder.h"
#include "DecorationList.h"
#include "Lights.h"
#include "Vis.h"
#include "Weather.h"


//#pragma comment(lib, "lib\\legacy_dx\\lib\\ddraw.lib")
//#pragma comment(lib, "lib\\legacy_dx\\lib\\dxguid.lib")

struct IDirectDrawClipper *pDDrawClipper;
class IRender *render; // idb
struct RenderVertexD3D3  pVertices[50];
int uNumDecorationsDrawnThisFrame;
RenderBillboard pBillboardRenderList[500];
unsigned int uNumBillboardsToDraw;
int uNumSpritesDrawnThisFrame;

RenderVertexSoft array_507D30[50];
RenderVertexSoft VertexRenderList[50];//array_50AC10
RenderVertexSoft array_73D150[20];

RenderVertexD3D3 d3d_vertex_buffer[50];

/*  384 */
#pragma pack(push, 1)
struct PCXHeader_1
{
  char manufacturer;
  char version;
  char encoding;
  char bpp;
  __int16 left;
  __int16 up;
  __int16 right;
  __int16 bottom;
  __int16 hdpi;
  __int16 vdpi;
};
#pragma pack(pop)

/*  385 */
#pragma pack(push, 1)
struct PCXHeader_2
{
  char reserved;
  char planes;
  __int16 pitch;
  __int16 palette_info;
};
#pragma pack(pop)

HRESULT __stdcall D3DZBufferFormatEnumerator(DDPIXELFORMAT *Src, DDPIXELFORMAT *Dst);
HRESULT __stdcall DDrawDisplayModesEnumerator(DDSURFACEDESC2 *pSurfaceDesc, __int16 *a2);
HRESULT __stdcall D3DDeviceEnumerator(const GUID *lpGUID, const char *lpDeviceDesc, const char *lpDeviceName, D3DDEVICEDESC *pHWDesc, D3DDEVICEDESC *pSWDesc, struct RenderD3D_aux *a6);
signed int __stdcall RenderD3D__DeviceEnumerator(GUID *lpGUID, const char *lpDevDesc, const char *lpDriverName, RenderD3D__DevInfo *pOut); // idb

//----- (0049E79F) --------------------------------------------------------
bool Render::CheckTextureStages()
{
  bool v0; // edi@1
  IDirectDrawSurface4 *pSurface2; // [sp+Ch] [bp-14h]@1
  IDirectDrawSurface4 *pSurface1; // [sp+10h] [bp-10h]@1
  DWORD v4; // [sp+14h] [bp-Ch]@1
  IDirect3DTexture2 *pTexture2; // [sp+18h] [bp-8h]@1
  IDirect3DTexture2 *pTexture1; // [sp+1Ch] [bp-4h]@1

  v0 = false;
  pRenderD3D->CreateTexture(64, 64, &pSurface1, &pTexture1, true, false, 32);
  pRenderD3D->CreateTexture(64, 64, &pSurface2, &pTexture2, true, false, 32);

  ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture1));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 1));

  ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 7));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, 1));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MINFILTER, 2));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, 1));

  if ( !pRenderD3D->pDevice->ValidateDevice(&v4) && v4 == 1 )
    v0 = true;
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1));
  pTexture1->Release();
  pTexture2->Release();
  pSurface1->Release();
  pSurface2->Release();
  return v0;
}


//----- (00440CB8) --------------------------------------------------------
void Render::DrawBillboardList_BLV()
{
  RenderBillboardTransform_local0 soft_billboard; // [sp+4h] [bp-50h]@1

  soft_billboard.sParentBillboardID = -1;
  soft_billboard.pTarget = pBLVRenderParams->pRenderTarget;
  soft_billboard.pTargetZ = pBLVRenderParams->pTargetZBuffer;
  soft_billboard.uTargetPitch = uTargetSurfacePitch;
  soft_billboard.uViewportX = pBLVRenderParams->uViewportX;
  soft_billboard.uViewportY = pBLVRenderParams->uViewportY;
  soft_billboard.uViewportZ = pBLVRenderParams->uViewportZ - 1;
  soft_billboard.uViewportW = pBLVRenderParams->uViewportW;

  pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;
  for (uint i = 0; i < ::uNumBillboardsToDraw; ++i)
  {
    RenderBillboard* p = &pBillboardRenderList[i];

      soft_billboard.uScreenSpaceX = p->uScreenSpaceX;
      soft_billboard.sParentBillboardID = i;
      soft_billboard.uScreenSpaceY = p->uScreenSpaceY;
      soft_billboard._screenspace_x_scaler_packedfloat = p->_screenspace_x_scaler_packedfloat;
      soft_billboard._screenspace_y_scaler_packedfloat = p->_screenspace_y_scaler_packedfloat;
      soft_billboard.sZValue = p->sZValue;
      soft_billboard.uFlags = p->field_1E;
      soft_billboard.sTintColor = p->sTintColor;
      if ( p->HwSpriteID != -1 )
      {
        if ( pRenderD3D )
          DrawBillboard_Indoor(&soft_billboard, &pSprites_LOD->pHardwareSprites[p->HwSpriteID], p->dimming_level);
        else
        {
          soft_billboard.pPalette = PaletteManager::Get_Dark_or_Red_LUT(p->uPalette, p->dimming_level, 1);
          if (p->field_1E & 0x0100)
            soft_billboard.pPalette = pPaletteManager->field_261600[p->uPalette];
          if ( !(soft_billboard.uFlags & 0x40) && soft_billboard.uFlags & 0x80 )
            soft_billboard.pPalette2 = PaletteManager::Get_Dark_or_Red_LUT(p->uPalette, 0, 1);
          if ( p->HwSpriteID >= 0 )
            pSprites_LOD->pSpriteHeaders[p->HwSpriteID].DrawSprite_sw(&soft_billboard, 1);
        }
      }
  }
}

//----- (004A16A5) --------------------------------------------------------
bool Render::AreRenderSurfacesOk()
{
  return pFrontBuffer4 && pBackBuffer4;
}


//----- (004A19D8) --------------------------------------------------------
unsigned int BlendColors(unsigned int a1, unsigned int a2)
{
  /*signed __int64 v2; // ST10_8@1
  double v3; // st7@1
  float v4; // ST24_4@1
  double v5; // ST10_8@1
  int v6; // ST1C_4@1
  float v7; // ST24_4@1
  double v8; // ST10_8@1
  unsigned __int8 v9; // ST20_1@1
  float v10; // ST24_4@1
  double v11; // ST10_8@1
  float v12; // ST24_4@1
  double v13; // ST08_8@1*/

  uint alpha = (uint)floorf(0.5f + (a1 >> 24) / 255.0f *
                                   (a2 >> 24) / 255.0f * 255.0f),
       red = (uint)floorf(0.5f + ((a1 >> 16) & 0xFF) / 255.0f *
                                 ((a2 >> 16) & 0xFF) / 255.0f * 255.0f),
       green = (uint)floorf(0.5f + ((a1 >> 8) & 0xFF) / 255.0f *
                                   ((a2 >> 8) & 0xFF) / 255.0f * 255.0f),
       blue = (uint)floorf(0.5f + ((a1 >> 0) & 0xFF) / 255.0f *
                                   ((a2 >> 0) & 0xFF) / 255.0f * 255.0f);
  return (alpha << 24) | (red << 16) | (green << 8) | blue;
  /*v2 = a1 >> 24;
  v3 = (double)v2 / 255.0f;
  HIDWORD(v2) = 0;
  LODWORD(v2) = a2 >> 24;
  v4 = v3 * (double)v2 / 255.0f * 255.0;
  v5 = v4 + 6.7553994e15;
  v6 = LODWORD(v5);
  v7 = (double)((a1 >> 16) & 0xFFi64) / 255.0f * (double)((a2 >> 16) & 0xFF) * 0.0039215689 * 255.0;
  v8 = v7 + 6.7553994e15;
  v9 = LOBYTE(v8);
  v10 = (double)((unsigned __int16)a1 >> 8) / 255.0f * (double)((unsigned __int16)a2 >> 8) / 255.0f * 255.0;
  v11 = v10 + 6.7553994e15;
  v12 = (double)(a1 & 0xFFi64) / 255.0f * (double)(unsigned __int8)a2 / 255.0f * 255.0;
  v13 = v12 + 6.7553994e15;
  return LOBYTE(v13) | ((LOBYTE(v11) | (((v6 << 8) | v9) << 8)) << 8);*/
}

void Render::RenderTerrainD3D() // New function
{
  int v6; // ecx@8
  struct Polygon *pTilePolygon; // ebx@8
  float Light_tile_dist;

  //warning: the game uses CW culling by default, ccw is incosistent
  pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);

  static RenderVertexSoft pTerrainVertices[128 * 128];//vertexCountX and vertexCountZ

  //Генерация местоположения вершин-------------------------------------------------------------------------
  //решётка вершин делится на две части от -64 до 0 и от 0 до 64
  //
  // -64  X  0     64
  //  --------------- 64
  //  |      |      |
  //  |      |      |
  //  |      |      |
  // 0|------+------| Z
  //  |      |      |
  //  |      |      |
  //  |      |      |
  //  ---------------
  //                -64

  int blockScale = 512;
  int heightScale = 32;
  for (unsigned int z = 0; z < 128; ++z)
  {
    for (unsigned int x = 0; x < 128; ++x)
    {
      pTerrainVertices[z * 128 + x].vWorldPosition.x = (-64 + (signed)x) * blockScale;
      pTerrainVertices[z * 128 + x].vWorldPosition.y = (64 - (signed)z) * blockScale;
      pTerrainVertices[z * 128 + x].vWorldPosition.z = heightScale * pOutdoor->pTerrain.pHeightmap[z * 128 + x];
      pIndoorCameraD3D->ViewTransform(&pTerrainVertices[z * 128 + x], 1);
      pIndoorCameraD3D->Project(&pTerrainVertices[z * 128 + x], 1, 0);
    }
  }
//-------(Отсечение невидимой части карты)------------------------------------------------------------------------------------------
  float direction = (float)(pIndoorCameraD3D->sRotationY / 256);//direction of the camera(напрвление камеры)
  //0-East(B)
  //1-NorthEast(CB)
  //2-North(C)
  //3-WestNorth(CЗ)
  //4-West(З)
  //5-SouthWest(ЮЗ)
  //6-South(Ю)
  //7-SouthEast(ЮВ)
  unsigned int Start_X, End_X, Start_Z, End_Z;
  if ( direction >= 0 && direction < 1.0 )//East(B) - NorthEast(CB)
  {
    Start_X = pODMRenderParams->uMapGridCellX - 2, End_X = 128;
    Start_Z = 0, End_Z = 128;
  }
  else if (direction >= 1.0 && direction < 3.0)//NorthEast(CB) - WestNorth(CЗ)
  {
      Start_X = 0, End_X = 128;
      Start_Z = 0, End_Z = pODMRenderParams->uMapGridCellZ + 1;
  }
  else if (direction >= 3.0 && direction < 5.0)//WestNorth(CЗ) - SouthWest(ЮЗ)
  {
    Start_X = 0, End_X = pODMRenderParams->uMapGridCellX + 2;
    Start_Z = 0, End_Z = 128;
  }
  else if ( direction >= 5.0 && direction < 7.0 )//SouthWest(ЮЗ) - //SouthEast(ЮВ)
  {
    Start_X = 0, End_X = 128;
    Start_Z = pODMRenderParams->uMapGridCellZ - 2, End_Z = 128;
  }
  else//SouthEast(ЮВ) - East(B)
  {
    Start_X = pODMRenderParams->uMapGridCellX - 2, End_X = 128;
    Start_Z = 0, End_Z = 128;
  }
  for (unsigned int z = Start_Z; z < End_Z; ++z)
  {
    for (unsigned int x = Start_X; x < End_X; ++x)
    {
      pTilePolygon = &array_77EC08[pODMRenderParams->uNumPolygons];
      pTilePolygon->flags = 0;
      pTilePolygon->field_32 = 0;
      pTilePolygon->uTileBitmapID = pOutdoor->DoGetTileTexture(x, z);
      pTilePolygon->pTexture = (Texture_MM7 *)&pBitmaps_LOD->pHardwareTextures[pTilePolygon->uTileBitmapID];
      if (pTilePolygon->uTileBitmapID == 0xFFFF)
        continue;

      //pTile->flags = 0x8010 |pOutdoor->GetSomeOtherTileInfo(x, z);
      pTilePolygon->flags = pOutdoor->GetSomeOtherTileInfo(x, z);
      pTilePolygon->field_32 = 0;
      pTilePolygon->field_59 = 1;
      pTilePolygon->sTextureDeltaU = 0;
      pTilePolygon->sTextureDeltaV = 0;
//  x,z         x+1,z
//  .____________.
//  |            |
//  |            |
//  |            |
//  |            |
//  |            |
//  .____________.
//  x,z+1       x+1,z+1
      memcpy(&array_73D150[0], &pTerrainVertices[z * 128 + x], sizeof(RenderVertexSoft));//x, z
      array_73D150[0].u = 0;
      array_73D150[0].v = 0;
      memcpy(&array_73D150[1], &pTerrainVertices[z * 128 + x + 1], sizeof(RenderVertexSoft));//x + 1, z
      array_73D150[1].u = 1;
      array_73D150[1].v = 0;
      memcpy(&array_73D150[2], &pTerrainVertices[(z + 1) * 128 + x + 1], sizeof(RenderVertexSoft));//x + 1, z + 1
      array_73D150[2].u = 1;
      array_73D150[2].v = 1;
      memcpy(&array_73D150[3], &pTerrainVertices[(z + 1) * 128 + x], sizeof(RenderVertexSoft));//x, z + 1
      array_73D150[3].u = 0;
      array_73D150[3].v = 1;
      //v58 = 0;
      //if (v58 == 4) // if all y == first y;  primitive in xz plane 
        //pTile->field_32 |= 0x0001;
      pTilePolygon->pODMFace = nullptr;
      pTilePolygon->uNumVertices = 4;
      pTilePolygon->field_59 = 5;

      if ( array_73D150[0].vWorldViewPosition.x < 8.0
        && array_73D150[1].vWorldViewPosition.x < 8.0
        && array_73D150[2].vWorldViewPosition.x < 8.0
        && array_73D150[3].vWorldViewPosition.x < 8.0 )
        continue;
      if ( (double)pODMRenderParams->shading_dist_mist < array_73D150[0].vWorldViewPosition.x
        && (double)pODMRenderParams->shading_dist_mist < array_73D150[1].vWorldViewPosition.x
        && (double)pODMRenderParams->shading_dist_mist < array_73D150[2].vWorldViewPosition.x
        && (double)pODMRenderParams->shading_dist_mist < array_73D150[3].vWorldViewPosition.x )
        continue;
 //----------------------------------------------------------------------------

      ++pODMRenderParams->uNumPolygons;
      ++pODMRenderParams->field_44;
      assert(pODMRenderParams->uNumPolygons < 20000);

      pTilePolygon->uBModelID = 0;
      pTilePolygon->uBModelFaceID = 0;
      pTilePolygon->field_50 = (8 * (0 | (0 << 6))) | 6;
      for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k)
      {
        memcpy(&VertexRenderList[k], &array_73D150[k], sizeof(struct RenderVertexSoft));
        VertexRenderList[k]._rhw = 1.0 / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097);
      }

//shading (затенение)----------------------------------------------------------------------------
      //uint norm_idx = pTerrainNormalIndices[2 * (z * 128 + x) + 1];
      uint norm_idx = pTerrainNormalIndices[2 * (x * 128 + z) + 1];
      assert(norm_idx < uNumTerrainNormals);

      Vec3_float_* norm = &pTerrainNormals[norm_idx];
      float _f = ((norm->x * (float)pOutdoor->vSunlight.x / 65536.0) -
                  (norm->y * (float)pOutdoor->vSunlight.y / 65536.0) -
                  (norm->z * (float)pOutdoor->vSunlight.z / 65536.0));
      pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);
      if ( norm_idx < 0 || norm_idx > uNumTerrainNormals - 1 )
        norm = 0;
      else
        norm = &pTerrainNormals[norm_idx];
	  if (lights_flag)
	  {
        //MessageBoxA(nullptr, "Ritor1: function StackLights_TerrainFace needed refactoring and result - slows", "", 0);
        //__debugbreak();

		pEngine->pLightmapBuilder->StackLights_TerrainFace(norm, &Light_tile_dist, VertexRenderList, 4, 1);//Ritor1: slows
      //pDecalBuilder->_49BE8A(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, 4, 1);
      }
      unsigned int a5 = 4;

//---------Draw distance(Дальность отрисовки)-------------------------------
      int temp =  pODMRenderParams->shading_dist_mist;
      if ( draw_terrain_dist_mist )
        pODMRenderParams->shading_dist_mist = 0x5000;
      bool neer_clip = array_73D150[0].vWorldViewPosition.x < 8.0
                    || array_73D150[1].vWorldViewPosition.x < 8.0
                    || array_73D150[2].vWorldViewPosition.x < 8.0
                    || array_73D150[3].vWorldViewPosition.x < 8.0;
      bool far_clip = (double)pODMRenderParams->shading_dist_mist < array_73D150[0].vWorldViewPosition.x
                   || (double)pODMRenderParams->shading_dist_mist < array_73D150[1].vWorldViewPosition.x
                   || (double)pODMRenderParams->shading_dist_mist < array_73D150[2].vWorldViewPosition.x
                   || (double)pODMRenderParams->shading_dist_mist < array_73D150[3].vWorldViewPosition.x;

      int uClipFlag = 0;
      static stru154 static_sub_0048034E_stru_154;
      pEngine->pLightmapBuilder->StationaryLightsCount = 0;
      if ( Lights.uNumLightsApplied > 0 || pDecalBuilder->uNumDecals > 0 )
      {
        if ( neer_clip )
          uClipFlag = 3;
        else
          uClipFlag = far_clip != 0 ? 5 : 0;
        static_sub_0048034E_stru_154.ClassifyPolygon(norm, Light_tile_dist);
        if ( pDecalBuilder->uNumDecals > 0 )
          pDecalBuilder->ApplyDecals(31 - pTilePolygon->dimming_level, 4, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, *(float *)&uClipFlag, -1);
        if ( Lights.uNumLightsApplied > 0 )
          pEngine->pLightmapBuilder->ApplyLights(&Lights, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, uClipFlag);
      }

      if ( !byte_4D864C || ~pEngine->uFlags & 0x80 )
      {
        //if ( neer_clip ) //Ritor1: Даёт искажения на подъёме, возможно требуется ф-ция Безье
        //{
         // pTilePolygon->uNumVertices = ODM_NearClip(pTilePolygon->uNumVertices);
         // ODM_Project(pTilePolygon->uNumVertices);
        //}
        if ( far_clip )
        {
          pTilePolygon->uNumVertices = ODM_FarClip(pTilePolygon->uNumVertices);
          ODM_Project(pTilePolygon->uNumVertices);
        }
      }
      pODMRenderParams->shading_dist_mist = temp;

// check the transparency and texture (tiles) mapping (проверка прозрачности и наложение текстур (тайлов))----------------------
      bool transparent = false;
      if ( !( pTilePolygon->flags & 1 ) ) // не поддерживается TextureFrameTable
      {
        if ( /*pTile->flags & 2 && */pTilePolygon->uTileBitmapID == render->hd_water_tile_id)
        {
          //transparent = false;
          v6 = render->pHDWaterBitmapIDs[render->hd_water_current_frame];
        }
        else
        {
          v6 = pTilePolygon->uTileBitmapID;
          if ( !_strnicmp(pBitmaps_LOD->pTextures[pTilePolygon->uTileBitmapID].pName, "wtrdr", 5) )
            transparent = true;
        }

        assert(v6 < 1000); // many random crashes here

        // for all shore tiles - draw a tile water under them since they're half-empty
        if (!_strnicmp(pBitmaps_LOD->pTextures[pTilePolygon->uTileBitmapID].pName, "wtrdr", 5))  // all shore tile filenames are wtrdrXXX
          DrawBorderTiles(pTilePolygon);

        render->DrawTerrainPolygon(pTilePolygon->uNumVertices, pTilePolygon, pBitmaps_LOD->pHardwareTextures[v6], transparent, true);
      }
      //else //здесь уже пограничные тайлы воды
        //pTile->DrawBorderTiles();
//--------------------------------------------------------------------------------------------------------------------------------

      //--pODMRenderParams->uNumPolygons;
      //--pODMRenderParams->field_44;
    }
  }
}

//----- (004811A3) --------------------------------------------------------
void Render::DrawBorderTiles(struct Polygon *poly)
{
  pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
  DrawTerrainPolygon(poly->uNumVertices, poly,
                     pBitmaps_LOD->pHardwareTextures[pHDWaterBitmapIDs[hd_water_current_frame]], false, true);

  pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, true);
  //DrawTerrainPolygon(poly->uNumVertices, poly, pBitmaps_LOD->pHardwareTextures[poly->uTileBitmapID], true, true);
}


//----- (0047BACF) --------------------------------------------------------
void Render::TransformBillboardsAndSetPalettesODM()
{
    //int v0; // edi@1
    //char *v1; // esi@2
    //unsigned int v2; // edx@3
    //int v3; // eax@3
    //int v4; // edi@3
    //int v5; // eax@3
    //__int16 v6; // di@3
    //int v7; // eax@3
    //int v8; // ebx@4
  //  unsigned __int16 *v9; // eax@7
  //  char v10; // zf@9
    //DWORD v11; // eax@13
  //  int v12; // eax@13
  //  int v13; // eax@14
    RenderBillboardTransform_local0 billboard; // [sp+4h] [bp-60h]@1
  //  int v15; // [sp+54h] [bp-10h]@13
    //int v16; // [sp+58h] [bp-Ch]@1
    //int v17; // [sp+5Ch] [bp-8h]@2
  //  int v18; // [sp+60h] [bp-4h]@13

    billboard.sParentBillboardID = -1;
    billboard.pTarget = render->pTargetSurface;
    billboard.pTargetZ = render->pActiveZBuffer;
    billboard.uTargetPitch = render->uTargetSurfacePitch;
    billboard.uViewportX = pViewport->uViewportTL_X;
    billboard.uViewportY = pViewport->uViewportTL_Y;
    billboard.uViewportZ = pViewport->uViewportBR_X - 1;
    billboard.uViewportW = pViewport->uViewportBR_Y;
    pODMRenderParams->uNumBillboards = uNumBillboardsToDraw;

    for (unsigned int i = 0; i < ::uNumBillboardsToDraw; ++i)
    {
        billboard.uScreenSpaceX = pBillboardRenderList[i].uScreenSpaceX;
        billboard.uScreenSpaceY = pBillboardRenderList[i].uScreenSpaceY;
        billboard.sParentBillboardID = i;
        billboard._screenspace_x_scaler_packedfloat = pBillboardRenderList[i]._screenspace_x_scaler_packedfloat;
        billboard.sTintColor = pBillboardRenderList[i].sTintColor;
        billboard._screenspace_y_scaler_packedfloat = pBillboardRenderList[i]._screenspace_y_scaler_packedfloat;
        billboard.sZValue = pBillboardRenderList[i].sZValue;
        billboard.uFlags = pBillboardRenderList[i].field_1E;
        if (pBillboardRenderList[i].HwSpriteID != -1)
        {
            if (!pRenderD3D) __debugbreak(); // no sw rendering
            TransformBillboard(
                &billboard,
                &pSprites_LOD->pHardwareSprites[pBillboardRenderList[i].HwSpriteID],
                pBillboardRenderList[i].dimming_level, &pBillboardRenderList[i]
            );
        }
    }
}

//----- (0047AF11) --------------------------------------------------------
void Render::DrawSpriteObjects_ODM()
{
    SpriteFrame *frame; // eax@10
    unsigned int v6; // eax@10
    int v9; // ecx@10
    int v17; // ecx@25
    int v18; // eax@25
  //  int v22; // ST3C_4@29
    signed __int64 v23; // qtt@30
    int v26; // eax@31
  //  char v27; // zf@31
    int v30; // [sp+14h] [bp-2Ch]@23
    int v37; // [sp+1Ch] [bp-24h]@23
    int a6; // [sp+20h] [bp-20h]@10
    int v42; // [sp+2Ch] [bp-14h]@23
    int y; // [sp+30h] [bp-10h]@10
    int x; // [sp+34h] [bp-Ch]@10
    int z; // [sp+38h] [bp-8h]@10
    signed __int16 v46; // [sp+3Ch] [bp-4h]@12

    //v41 = 0;
    for (unsigned int i = 0; i < uNumSpriteObjects; ++i)
    {
        SpriteObject* object = &pSpriteObjects[i];
        //auto v0 = (char *)&pSpriteObjects[i].uSectorID;
        //v0 = (char *)&pSpriteObjects[0].uSectorID;
        //do
        //{
        if (!object->uObjectDescID)  // item probably pciked up
            continue;

        assert(object->uObjectDescID < pObjectList->uNumObjects);
        ObjectDesc* object_desc = &pObjectList->pObjects[object->uObjectDescID];
        if (object_desc->NoSprite())
            continue;

        //v1 = &pObjectList->pObjects[*((short *)v0 - 13)];
        //if ( !(v1->uFlags & 1) )
        //{
          //v2 = *((short *)v0 - 14)
    //v2 = object->uType;
        if ((object->uType < 1000 || object->uType >= 10000) && (object->uType < 500 || object->uType >= 600)
            || pEngine->pStru6Instance->RenderAsSprite(object))
        {
            //a5 = *(short *)v0;
            x = object->vPosition.x;
            y = object->vPosition.y;
            z = object->vPosition.z;
            frame = pSpriteFrameTable->GetFrame(object_desc->uSpriteID, object->uSpriteFrameID);
            a6 = frame->uGlowRadius * object->field_22_glow_radius_multiplier;
            v6 = stru_5C6E00->Atan2(object->vPosition.x - pIndoorCameraD3D->vPartyPos.x, object->vPosition.y - pIndoorCameraD3D->vPartyPos.y);
            //LOWORD(v7) = object->uFacing;
            //v8 = v36;
            v9 = ((signed int)(stru_5C6E00->uIntegerPi + ((signed int)stru_5C6E00->uIntegerPi >> 3) + object->uFacing - v6) >> 8) & 7;
            pBillboardRenderList[::uNumBillboardsToDraw].HwSpriteID = frame->pHwSpriteIDs[v9];
            if (frame->uFlags & 0x20)
            {
                //v8 = v36;
                z -= fixpoint_mul(frame->scale, pSprites_LOD->pSpriteHeaders[(signed __int16)frame->pHwSpriteIDs[v9]].uHeight) / 2;
            }
            v46 = 0;
            if (frame->uFlags & 2)
                v46 = 2;
            //v11 = (int *)(256 << v9);
            if ((256 << v9) & frame->uFlags)
                v46 |= 4;
            if (frame->uFlags & 0x40000)
                v46 |= 0x40;
            if (frame->uFlags & 0x20000)
                LOBYTE(v46) = v46 | 0x80;
            if (a6)
            {
                //LOBYTE(v11) = _4E94D3_light_type;
                pMobileLightsStack->AddLight(x, y, z, object->uSectorID, a6, 0xFF, 0xFF, 0xFF, _4E94D3_light_type);
            }
            if (pIndoorCameraD3D->sRotationX)
            {
                v30 = fixpoint_mul((x - pIndoorCameraD3D->vPartyPos.x) << 16, pIndoorCameraD3D->int_cosine_y)
                    + fixpoint_mul((y - pIndoorCameraD3D->vPartyPos.y) << 16, pIndoorCameraD3D->int_sine_y);
                v37 = fixpoint_mul((x - pIndoorCameraD3D->vPartyPos.x) << 16, pIndoorCameraD3D->int_sine_y);
                v42 = fixpoint_mul((z - pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_sine_x)
                    + fixpoint_mul(v30, pIndoorCameraD3D->int_cosine_x);
                if (v42 >= 0x40000 && v42 <= pODMRenderParams->shading_dist_mist << 16)
                {
                    v17 = fixpoint_mul((y - pIndoorCameraD3D->vPartyPos.y) << 16, pIndoorCameraD3D->int_cosine_y) - v37;
                    v18 = fixpoint_mul((z - pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_cosine_x)
                        - fixpoint_mul(v30, pIndoorCameraD3D->int_sine_x);
                    if (abs(v42) >= abs(v17))
                    {
                        LODWORD(v23) = 0;
                        HIDWORD(v23) = SLOWORD(pODMRenderParams->int_fov_rad);

                        object->uAttributes |= 1;
                        pBillboardRenderList[::uNumBillboardsToDraw].uPalette = frame->uPaletteIndex;
                        pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = object->uSectorID;
                        pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_x_scaler_packedfloat = fixpoint_mul(frame->scale, v23 / v42);
                        pBillboardRenderList[::uNumBillboardsToDraw].pSpriteFrame = frame;
                        pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_y_scaler_packedfloat = fixpoint_mul(frame->scale, v23 / v42);
                        pBillboardRenderList[::uNumBillboardsToDraw].field_1E = v46;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_x = x;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_y = y;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_z = z;
                        pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceX = pViewport->uScreenCenterX - ((signed int)(fixpoint_mul(v23 / v42, v17) + 0x8000) >> 16);
                        pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceY = pViewport->uScreenCenterY - (((unsigned int)fixpoint_mul(v23 / v42, v18) + 0x8000) >> 16);
                        HIWORD(v26) = HIWORD(v42);
                        LOWORD(v26) = 0;
                        pBillboardRenderList[::uNumBillboardsToDraw].sZValue = v26 + (PID(OBJECT_Item, i));
                        pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
                        pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = 0;
                        if (!(object->uAttributes & 0x20))
                        {
                            if (!pRenderD3D)
                            {
                                __debugbreak();
                                pBillboardRenderList[::uNumBillboardsToDraw].sZValue = 0;
                            }
                        }
                        //if (::uNumBillboardsToDraw >= 500)
                        //  return;
                        assert(::uNumBillboardsToDraw < 500);
                        ++::uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;
                    }
                }
            }
            else
            {
                v42 = fixpoint_mul((y - pIndoorCameraD3D->vPartyPos.y) << 16, pIndoorCameraD3D->int_sine_y)
                    + fixpoint_mul((x - pIndoorCameraD3D->vPartyPos.x) << 16, pIndoorCameraD3D->int_cosine_y);
                if (v42 >= 0x40000 && v42 <= pODMRenderParams->shading_dist_mist << 16)
                {
                    v17 = fixpoint_mul((y - pIndoorCameraD3D->vPartyPos.y) << 16, pIndoorCameraD3D->int_cosine_y)
                        - fixpoint_mul(((x - pIndoorCameraD3D->vPartyPos.x) << 16), pIndoorCameraD3D->int_sine_y);
                    v18 = (z - pIndoorCameraD3D->vPartyPos.z) << 16;
                    if (abs(v42) >= abs(v17))
                    {
                        LODWORD(v23) = 0;
                        HIDWORD(v23) = SLOWORD(pODMRenderParams->int_fov_rad);

                        object->uAttributes |= 1;
                        pBillboardRenderList[::uNumBillboardsToDraw].uPalette = frame->uPaletteIndex;
                        pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = object->uSectorID;
                        pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_x_scaler_packedfloat = fixpoint_mul(frame->scale, v23 / v42);
                        pBillboardRenderList[::uNumBillboardsToDraw].pSpriteFrame = frame;
                        pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_y_scaler_packedfloat = fixpoint_mul(frame->scale, v23 / v42);
                        pBillboardRenderList[::uNumBillboardsToDraw].field_1E = v46;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_x = x;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_y = y;
                        pBillboardRenderList[::uNumBillboardsToDraw].world_z = z;
                        pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceX = pViewport->uScreenCenterX - ((signed int)(fixpoint_mul(v23 / v42, v17) + 0x8000) >> 16);
                        pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceY = pViewport->uScreenCenterY - (((unsigned int)fixpoint_mul(v23 / v42, v18) + 0x8000) >> 16);
                        HIWORD(v26) = HIWORD(v42);
                        LOWORD(v26) = 0;
                        pBillboardRenderList[::uNumBillboardsToDraw].sZValue = v26 + (PID(OBJECT_Item, i));
                        pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
                        pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = 0;
                        if (!(object->uAttributes & 0x20))
                        {
                            if (!pRenderD3D)
                            {
                                __debugbreak();
                                pBillboardRenderList[::uNumBillboardsToDraw].sZValue = 0;
                            }
                        }
                        //if (::uNumBillboardsToDraw >= 500)
                        //  return;
                        assert(::uNumBillboardsToDraw < 500);
                        ++::uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;
                    }
                }
            }
        }
    }
}

//----- (0049D9BC) --------------------------------------------------------
signed int __stdcall RenderD3D__DeviceEnumerator(GUID *lpGUID, const char *lpDevDesc, const char *lpDriverName, RenderD3D__DevInfo *pOut)
{
  size_t v4; // eax@1
  size_t v5; // eax@1
  size_t v7; // eax@13
  DDDEVICEIDENTIFIER ddDevId; // [sp+4h] [bp-4F8h]@11
  DDSURFACEDESC2 v10;/*int v10; // [sp+42Ch] [bp-D0h]@16*/
  DDSCAPS2 ddsCaps; // [sp+4A8h] [bp-54h]@14
  unsigned int uFreeVideoMem; // [sp+4B8h] [bp-44h]@14
  RenderD3D_aux aux; // [sp+4BCh] [bp-40h]@19
  IDirect3D3 *pDirect3D3; // [sp+4C4h] [bp-38h]@18
  int is_there_a_compatible_screen_mode; // [sp+4C8h] [bp-34h]@16
  RenderD3D_D3DDevDesc v20; // [sp+4CCh] [bp-30h]@1
  LPDIRECTDRAW pDirectDraw = nullptr; // [sp+4F4h] [bp-8h]@4
  IDirectDraw4 *pDirectDraw4; // [sp+4F8h] [bp-4h]@7

  v4 = strlen(lpDriverName);
  v20.pDriverName = new char[v4 + 1];
  v5 = strlen(lpDevDesc);
  v20.pDeviceDesc = new char[v5 + 1];
  strcpy(v20.pDriverName, lpDriverName);
  strcpy(v20.pDeviceDesc, lpDevDesc);
  if ( lpGUID )
  {
    v20.pGUID = new GUID;
    memcpy(v20.pGUID, lpGUID, 0x10);
  }
  else
    v20.pGUID = 0;

  if (FAILED(DirectDrawCreate(v20.pGUID, &pDirectDraw, 0)))
  {
    delete [] v20.pDriverName;
    delete [] v20.pDeviceDesc;
    delete v20.pGUID;
  }
  else
  {
    if (FAILED(pDirectDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pDirectDraw4)))
    {
      delete [] v20.pDriverName;
      delete [] v20.pDeviceDesc;
      delete v20.pGUID;
      pDirectDraw->Release();
    }
    else
    {
      pDirectDraw->Release();
      if (FAILED( pDirectDraw4->GetDeviceIdentifier(&ddDevId, 1)))
        v20.pDDraw4DevDesc = 0;
      else
      {
        v7 = strlen(ddDevId.szDescription);
        v20.pDDraw4DevDesc = new char[v7 + 1];
        strcpy(v20.pDDraw4DevDesc, ddDevId.szDescription);
      }
      memset(&ddsCaps, 0, sizeof(ddsCaps));
      if (FAILED(pDirectDraw4->GetAvailableVidMem(&ddsCaps, (LPDWORD)&v20.uVideoMem, (LPDWORD)&uFreeVideoMem)))
        v20.uVideoMem = 0;
      memset(&v10, 0, sizeof(v10));
      v10.dwSize = 124;
      v10.dwFlags = 6;
      v10.dwHeight = window->GetWidth();
      v10.dwWidth = window->GetHeight();
      v10.ddpfPixelFormat.dwSize = 32;

      is_there_a_compatible_screen_mode = false;
      if ( FAILED(pDirectDraw4->EnumDisplayModes(0, 0, &is_there_a_compatible_screen_mode, (LPDDENUMMODESCALLBACK2)DDrawDisplayModesEnumerator))
        || !is_there_a_compatible_screen_mode
        || FAILED(pDirectDraw4->QueryInterface(IID_IDirect3D3, (LPVOID *)&pDirect3D3)))
      {
        delete [] v20.pDriverName;
        delete [] v20.pDeviceDesc;
        //free(v20.pDDraw4DevDesc);
		delete [] v20.pDDraw4DevDesc;
        delete v20.pGUID;
        pDirectDraw4->Release();
      }
      else
      {
        aux.pInfo = pOut;
        aux.ptr_4 = &v20;
        pDirect3D3->EnumDevices((LPD3DENUMDEVICESCALLBACK)D3DDeviceEnumerator, &aux);
        delete [] v20.pDriverName;
        delete [] v20.pDeviceDesc;
		delete [] v20.pDDraw4DevDesc;
        delete v20.pGUID;
        pDirectDraw4->Release();
        pDirectDraw4 = 0;
        pDirect3D3->Release();
      }
    }
  }
  return 1;
}

//----- (0049D784) --------------------------------------------------------
HRESULT __stdcall D3DDeviceEnumerator(const GUID *lpGUID, const char *lpDeviceDesc, const char *lpDeviceName, D3DDEVICEDESC *pHWDesc, D3DDEVICEDESC *pSWDesc, RenderD3D_aux *a6)
{
  signed int v7; // edi@1

  v7 = -1;
  if ( pHWDesc->dwFlags )
  {
    if ( !a6->ptr_4->pGUID )
      v7 = 0;
    if ( pHWDesc->dwFlags && a6->ptr_4->pGUID )
      v7 = 1;
  }
  if ( !strcmp(lpDeviceName, "RGB Emulation") && !a6->ptr_4->pGUID )
    v7 = 2;
  if ( !strcmp(lpDeviceName, "Reference Rasterizer") && !a6->ptr_4->pGUID )
    v7 = 3;
  if ( v7 != -1 )
  {
    a6->pInfo[v7].bIsDeviceCompatible = 1;
    a6->pInfo[v7].uCaps = 0;
    if ( !(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 0x10) )
      a6->pInfo[v7].uCaps |= 2;
    if ( !(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 2) )
      a6->pInfo[v7].uCaps |= 4;
    if ( !(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 1) )
      a6->pInfo[v7].uCaps |= 8;
    if ( !(pHWDesc->dpcTriCaps.dwDestBlendCaps & 0x20) )
      a6->pInfo[v7].uCaps |= 16;
    if ( !(pHWDesc->dpcTriCaps.dwDestBlendCaps & 2) )
      a6->pInfo[v7].uCaps |= 32;
    if ( !(pHWDesc->dpcTriCaps.dwDestBlendCaps & 4) )
      a6->pInfo[v7].uCaps |= 64;
    if ( !(BYTE1(pHWDesc->dwDevCaps) & 0x10) )
      BYTE1(a6->pInfo[v7].uCaps) |= 1;
    if ( pHWDesc->dpcTriCaps.dwTextureCaps & 0x20 )
      LOBYTE(a6->pInfo[v7].uCaps) |= 0x80;

    a6->pInfo[v7].pName = new char[strlen(lpDeviceName) + 1];
    strcpy(a6->pInfo[v7].pName, lpDeviceName);

    a6->pInfo[v7].pDescription = new char[strlen(lpDeviceDesc) + 1];
    strcpy(a6->pInfo[v7].pDescription, lpDeviceDesc);

    a6->pInfo[v7].pGUID = new GUID;
    memcpy(a6->pInfo[v7].pGUID, lpGUID, 0x10);

    a6->pInfo[v7].pDriverName = new char[strlen(a6->ptr_4->pDriverName) + 1];
    strcpy(a6->pInfo[v7].pDriverName, a6->ptr_4->pDriverName);

    a6->pInfo[v7].pDeviceDesc = new char[strlen(a6->ptr_4->pDeviceDesc) + 1];
    strcpy(a6->pInfo[v7].pDeviceDesc, a6->ptr_4->pDeviceDesc);

    a6->pInfo[v7].pDDraw4DevDesc = new char[strlen(a6->ptr_4->pDDraw4DevDesc) + 1];
    strcpy(a6->pInfo[v7].pDDraw4DevDesc, a6->ptr_4->pDDraw4DevDesc);

    if ( a6->ptr_4->pGUID )
    {
      a6->pInfo[v7].pDirectDrawGUID = new GUID;
      memcpy(a6->pInfo[v7].pDirectDrawGUID, a6->ptr_4->pGUID, 0x10);
    }
    else
      a6->pInfo[v7].pDirectDrawGUID = 0;
    a6->pInfo[v7].uVideoMem = a6->ptr_4->uVideoMem;
  }
  return 1;
}

//----- (0049D75C) --------------------------------------------------------
HRESULT __stdcall DDrawDisplayModesEnumerator(DDSURFACEDESC2 *pSurfaceDesc, __int16 *found_compatible_mode)
{
  if ( pSurfaceDesc->ddsCaps.dwCaps | DDSCAPS_3DDEVICE /*&& pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16*/ )
  {
    *found_compatible_mode = 1;
	return S_OK;
  }
  return 1;
}

//----- (0047A95E) --------------------------------------------------------
void Render::PrepareDecorationsRenderList_ODM()
{
    unsigned int v6; // edi@9
    int v7; // eax@9
    SpriteFrame *frame; // eax@9
    unsigned __int16 *v10; // eax@9
    int v13; // ecx@9
    char r; // ecx@20
    char g; // dl@20
    char b_; // eax@20
    int v17; // eax@23
    int v18; // ecx@24
    int v19; // eax@24
    int v20; // ecx@24
    int v21; // ebx@26
    int v22; // eax@26
    signed __int64 v24; // qtt@31
    int v25; // ebx@31
    __int16 v29; // cx@37
    int v30; // ecx@37
    int v31; // ebx@37
    Particle_sw local_0; // [sp+Ch] [bp-98h]@7
    unsigned __int16 *v37; // [sp+84h] [bp-20h]@9
    int v38; // [sp+88h] [bp-1Ch]@9
    int v40; // [sp+90h] [bp-14h]@24
    int v41; // [sp+94h] [bp-10h]@24
    int v42; // [sp+98h] [bp-Ch]@9
    int b; // [sp+A0h] [bp-4h]@22

    for (unsigned int i = 0; i < uNumLevelDecorations; ++i)
    {
        //LevelDecoration* decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST)
            || pLevelDecorations[i].IsObeliskChestActive()) && !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE))
        {
            DecorationDesc* decor_desc = &pDecorationList->pDecorations[pLevelDecorations[i].uDecorationDescID];
            if ((char)decor_desc->uFlags >= 0)
            {
                if (!(decor_desc->uFlags & 0x22))
                {
                    v6 = pMiscTimer->uTotalGameTimeElapsed;
                    v7 = abs(pLevelDecorations[i].vPosition.x + pLevelDecorations[i].vPosition.y);

#pragma region "New: seasons change"
                    extern bool change_seasons;
                    if (change_seasons)
                        switch (pParty->uCurrentMonth)
                        {
                            // case 531 (tree60), 536 (tree65), 537 (tree66) have no autumn/winter sprites
                        case 11: case 0: case 1: // winter
                            switch (decor_desc->uSpriteID)
                            {
                                //case 468: //bush02    grows on swamps, which are evergreeen actually
                            case 548:             // flower10
                            case 547:             // flower09
                            case 541:             // flower03
                            case 539: continue;   // flower01

                            case 483:             // tree01
                            case 486:             // tree04
                            case 492:             // tree10
                                pSpriteFrameTable->InitializeSprite(decor_desc->uSpriteID + 2);
                                frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID + 2, v6 + v7);
                                break;

                            default:
                                frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);
                            }
                            break;

                        case 2: case 3: case 4: // spring
                            switch (decor_desc->uSpriteID)
                            {
                            }
                            frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);
                            break;

                        case 8: case 9: case 10: // autumn
                            switch (decor_desc->uSpriteID)
                            {
                                //case 468: //bush02    grows on swamps, which are evergreeen actually
                            case 548:             // flower10
                            case 547:             // flower09
                            case 541:             // flower03
                            case 539: continue;   // flower01

                            case 483:             // tree01
                            case 486:             // tree04
                            case 492:             // tree10
                                pSpriteFrameTable->InitializeSprite(decor_desc->uSpriteID + 1);
                                frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID + 1, v6 + v7);
                                break;

                            default:
                                frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);
                            }
                            break;

                        case 5: case 6: case 7: // summer
                          //all green by default
                            frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);
                            break;

                        default: assert(pParty->uCurrentMonth >= 0 && pParty->uCurrentMonth < 12);
                        }
                    else
                        frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);
#pragma endregion
                    //v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID, v6 + v7);

                    v10 = (unsigned __int16 *)stru_5C6E00->Atan2(pLevelDecorations[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
                        pLevelDecorations[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);
                    v38 = 0;
                    v13 = ((signed int)(stru_5C6E00->uIntegerPi + ((signed int)stru_5C6E00->uIntegerPi >> 3) + pLevelDecorations[i].field_10_y_rot - (signed int)v10) >> 8) & 7;
                    v37 = (unsigned __int16 *)v13;
                    if (frame->uFlags & 2)
                        v38 = 2;
                    if ((256 << v13) & frame->uFlags)
                        v38 |= 4;
                    if (frame->uFlags & 0x40000)
                        v38 |= 0x40;
                    if (frame->uFlags & 0x20000)
                        LOBYTE(v38) = v38 | 0x80;

                    //for light
                    if (frame->uGlowRadius)
                    {
                        r = 255;
                        g = 255;
                        b_ = 255;
                        if ( /*pRenderD3D &&*/ bUseColoredLights)
                        {
                            r = /*255;//*/decor_desc->uColoredLightRed;
                            g = /*255;//*/decor_desc->uColoredLightGreen;
                            b_ = /*255;//*/decor_desc->uColoredLightBlue;
                        }
                        pStationaryLightsStack->AddLight(pLevelDecorations[i].vPosition.x,
                            pLevelDecorations[i].vPosition.y,
                            pLevelDecorations[i].vPosition.z + decor_desc->uDecorationHeight / 2,
                            frame->uGlowRadius, r, g, b_, _4E94D0_light_type);
                    }//for light

                    v17 = (pLevelDecorations[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x) << 16;
                    if (pIndoorCameraD3D->sRotationX)
                    {
                        v40 = (pLevelDecorations[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y) << 16;
                        v18 = fixpoint_mul(v17, pIndoorCameraD3D->int_cosine_y) + fixpoint_mul(v40, pIndoorCameraD3D->int_sine_y);
                        v41 = fixpoint_mul((pLevelDecorations[i].vPosition.z - pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_sine_x);
                        v19 = fixpoint_mul(v18, pIndoorCameraD3D->int_cosine_x);
                        v20 = v19 + fixpoint_mul((pLevelDecorations[i].vPosition.z - pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_sine_x);
                        if (v20 >= 0x40000 && v20 <= pODMRenderParams->shading_dist_mist << 16)
                        {
                            v21 = fixpoint_mul(v40, pIndoorCameraD3D->int_cosine_y) - fixpoint_mul(v17, pIndoorCameraD3D->int_sine_y);
                            v22 = fixpoint_mul((pLevelDecorations[i].vPosition.z - pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_cosine_x) - fixpoint_mul(v18, pIndoorCameraD3D->int_sine_x);
                            if (2 * abs(v20) >= abs(v21))
                            {
                                LODWORD(v24) = 0;
                                HIDWORD(v24) = SLOWORD(pODMRenderParams->int_fov_rad);
                                v25 = pViewport->uScreenCenterX - ((signed int)(fixpoint_mul(v24 / v20, v21) + 0x8000) >> 16);
                                v40 = pViewport->uScreenCenterY - ((signed int)(fixpoint_mul(v24 / v20, v22) + 0x8000) >> 16);
                                v41 = fixpoint_mul(frame->scale, v24 / v20);
                                if (pRenderD3D)
                                    b = fixpoint_mul(pSprites_LOD->pHardwareSprites[frame->pHwSpriteIDs[(int)v37]].uBufferWidth / 2, v41);
                                else
                                {
                                    __debugbreak();
                                    b = fixpoint_mul(pSprites_LOD->pSpriteHeaders[frame->pHwSpriteIDs[(int)v37]].uWidth / 2, v41);
                                }
                                if (b + v25 >= (signed int)pViewport->uViewportTL_X && v25 - b <= (signed int)pViewport->uViewportBR_X)
                                {
                                    if (::uNumBillboardsToDraw >= 500)
                                        return;
                                    pBillboardRenderList[::uNumBillboardsToDraw].HwSpriteID = frame->pHwSpriteIDs[(int)v37];
                                    pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_x_scaler_packedfloat = v41;
                                    pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_y_scaler_packedfloat = v41;
                                    v29 = v38;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceX = v25;
                                    HIBYTE(v29) |= 2;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uPalette = frame->uPaletteIndex;
                                    pBillboardRenderList[::uNumBillboardsToDraw].field_1E = v29;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_x = pLevelDecorations[i].vPosition.x;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_y = pLevelDecorations[i].vPosition.y;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_z = pLevelDecorations[i].vPosition.z;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceY = v40;
                                    HIWORD(v30) = HIWORD(v20);
                                    v31 = PID(OBJECT_Decoration, i);
                                    LOWORD(v30) = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].sZValue = v30 + v31;
                                    pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].pSpriteFrame = frame;
                                    pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = 0;
                                    ::uNumBillboardsToDraw++;
                                    ++uNumDecorationsDrawnThisFrame;
                                }
                            }
                            continue;
                        }
                    }
                    else
                    {
                        v42 = (pLevelDecorations[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x) << 16;
                        v40 = (pLevelDecorations[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y) << 16;
                        v20 = fixpoint_mul(v17, pIndoorCameraD3D->int_cosine_y) + fixpoint_mul(v40, pIndoorCameraD3D->int_sine_y);
                        if (v20 >= 0x40000 && v20 <= pODMRenderParams->shading_dist_mist << 16)
                        {
                            v21 = fixpoint_mul(v40, pIndoorCameraD3D->int_cosine_y) - fixpoint_mul(v42, pIndoorCameraD3D->int_sine_y);
                            v22 = (pLevelDecorations[i].vPosition.z - pIndoorCameraD3D->vPartyPos.z) << 16;
                            v42 = v22;
                            if (2 * abs(v20) >= abs(v21))
                            {
                                LODWORD(v24) = 0;
                                HIDWORD(v24) = SLOWORD(pODMRenderParams->int_fov_rad);
                                v25 = pViewport->uScreenCenterX - ((signed int)(fixpoint_mul(v24 / v20, v21) + 0x8000) >> 16);
                                v40 = pViewport->uScreenCenterY - ((signed int)(fixpoint_mul(v24 / v20, v42) + 0x8000) >> 16);
                                v41 = fixpoint_mul(frame->scale, v24 / v20);
                                if (pRenderD3D)
                                    b = fixpoint_mul(pSprites_LOD->pHardwareSprites[frame->pHwSpriteIDs[(int)v37]].uBufferWidth / 2, v41);
                                else
                                {
                                    __debugbreak();
                                    b = fixpoint_mul(pSprites_LOD->pSpriteHeaders[frame->pHwSpriteIDs[(int)v37]].uWidth / 2, v41);
                                }
                                if (b + v25 >= (signed int)pViewport->uViewportTL_X && v25 - b <= (signed int)pViewport->uViewportBR_X)
                                {
                                    if (::uNumBillboardsToDraw >= 500)
                                        return;
                                    pBillboardRenderList[::uNumBillboardsToDraw].HwSpriteID = frame->pHwSpriteIDs[(int)v37];
                                    pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_x_scaler_packedfloat = v41;
                                    pBillboardRenderList[::uNumBillboardsToDraw]._screenspace_y_scaler_packedfloat = v41;
                                    v29 = v38;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceX = v25;
                                    HIBYTE(v29) |= 2;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uPalette = frame->uPaletteIndex;
                                    pBillboardRenderList[::uNumBillboardsToDraw].field_1E = v29;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_x = pLevelDecorations[i].vPosition.x;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_y = pLevelDecorations[i].vPosition.y;
                                    pBillboardRenderList[::uNumBillboardsToDraw].world_z = pLevelDecorations[i].vPosition.z;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uScreenSpaceY = v40;
                                    HIWORD(v30) = HIWORD(v20);
                                    v31 = PID(OBJECT_Decoration, i);
                                    LOWORD(v30) = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].sZValue = v30 + v31;
                                    pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
                                    pBillboardRenderList[::uNumBillboardsToDraw].pSpriteFrame = frame;
                                    pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = 0;
                                    ::uNumBillboardsToDraw++;
                                    ++uNumDecorationsDrawnThisFrame;
                                }
                            }
                            continue;
                        }
                    }
                }
            }
            else
            {
                memset(&local_0, 0, 0x68);
                local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
                local_0.uDiffuse = 0xFF3C1E;
                local_0.x = (double)pLevelDecorations[i].vPosition.x;
                local_0.y = (double)pLevelDecorations[i].vPosition.y;
                local_0.z = (double)pLevelDecorations[i].vPosition.z;
                local_0.r = 0.0;
                local_0.g = 0.0;
                local_0.b = 0.0;
                local_0.flt_28 = 1.0;
                local_0.timeToLive = (rand() & 0x80) + 128;
                local_0.resource_id = pBitmaps_LOD->LoadTexture("effpar01");
                pEngine->pParticleEngine->AddParticle(&local_0);
            }
        }
    }
}

//----- (0049D717) --------------------------------------------------------
HRESULT __stdcall D3DZBufferFormatEnumerator(DDPIXELFORMAT *Src, DDPIXELFORMAT *Dst)
{
  if ( Src->dwFlags & (0x400 | 0x2000))
  {
    if ( Src->dwRGBBitCount == 16 && !Src->dwRBitMask )
    {
      memcpy(Dst, Src, sizeof(DDPIXELFORMAT));
      return 0;
    }
    if ( !Dst->dwSize )
    {
      memcpy(Dst, Src, sizeof(DDPIXELFORMAT));
      return 1;
    }
  }
  return 1;
}

//----- (0049DC28) --------------------------------------------------------
void RenderD3D::GetAvailableDevices(RenderD3D__DevInfo **pOutDevices)
{
  RenderD3D__DevInfo *v2; // eax@1

  v2 = new RenderD3D__DevInfo[4];// 4 items
  *pOutDevices = v2;
  memset(v2, 0, sizeof(v2));
  DirectDrawEnumerateExA((LPDDENUMCALLBACKEXA)RenderD3D__DeviceEnumerator, *pOutDevices, DDENUM_ATTACHEDSECONDARYDEVICES);
}

//----- (0049DC58) --------------------------------------------------------
RenderD3D::RenderD3D()
{
  this->pHost = nullptr;
  this->pDirect3D = nullptr;
  this->pUnk = nullptr;
  this->pBackBuffer = nullptr;
  this->pFrontBuffer = nullptr;
  this->pZBuffer = nullptr;
  this->pDevice = nullptr;
  this->pViewport = nullptr;
  this->field_40 = 1;
  this->field_44 = 10;
  GetAvailableDevices(&this->pAvailableDevices);
}

//----- (0049DC90) --------------------------------------------------------
void RenderD3D::Release()
{
  if ( !this->bWindowed )
  {
    if ( this->pHost )
    {
      this->pHost->RestoreDisplayMode();
      this->pHost->SetCooperativeLevel(this->hWindow, DDSCL_NORMAL);
      this->pHost->FlipToGDISurface();
    }
  }

    for (int i = 0; i < 4; i++)
    {
        if (this->pAvailableDevices[i].pDriverName)
        {
            delete[] this->pAvailableDevices[i].pDriverName;
            this->pAvailableDevices[i].pDriverName = nullptr;
        }

        if (this->pAvailableDevices[i].pDeviceDesc)
        {
            delete[] this->pAvailableDevices[i].pDeviceDesc;
            this->pAvailableDevices[i].pDeviceDesc = nullptr;
        }

        if (this->pAvailableDevices[i].pDDraw4DevDesc)
        {
            delete[] this->pAvailableDevices[i].pDDraw4DevDesc;
            this->pAvailableDevices[i].pDDraw4DevDesc = nullptr;
        }

        if (this->pAvailableDevices[i].pDirectDrawGUID)
        {
            delete this->pAvailableDevices[i].pDirectDrawGUID;
            this->pAvailableDevices[i].pDirectDrawGUID = nullptr;
        }

        if (this->pAvailableDevices[i].pName)
        {
            delete[] this->pAvailableDevices[i].pName;
            this->pAvailableDevices[i].pName = nullptr;
        }

        if (this->pAvailableDevices[i].pDescription)
        {
            delete[] this->pAvailableDevices[i].pDescription;
            this->pAvailableDevices[i].pDescription = nullptr;
        }

        if (this->pAvailableDevices[i].pGUID)
        {
            delete this->pAvailableDevices[i].pGUID;
            this->pAvailableDevices[i].pGUID = nullptr;
        }
  }

  delete[] this->pAvailableDevices;
  this->pAvailableDevices = NULL;

  if ( this->pViewport )
  {
    this->pViewport->Release();
    this->pViewport = NULL;
  }

  if ( this->pUnk )
  {
    this->pUnk->Release();
    this->pUnk = NULL;
  }

  if ( this->pZBuffer )
  {
    this->pZBuffer->Release();
    this->pZBuffer = NULL;
  }

  if ( this->pDevice )
  {
    this->pDevice->Release();
    this->pDevice = NULL;
  }

  if ( this->pDirect3D )
  {
    this->pDirect3D->Release();
    this->pDirect3D = NULL;
  }

  if ( this->pBackBuffer )
  {
    this->pBackBuffer->Release();
    this->pBackBuffer = NULL;
  }

  if ( this->pFrontBuffer )
  {
    this->pFrontBuffer->Release();
    this->pFrontBuffer = NULL;
  }

  if ( this->pHost )
  {
    this->pHost->Release();
    this->pHost = NULL;
  }
}

//----- (0049DE14) --------------------------------------------------------
bool RenderD3D::CreateDevice(unsigned int uDeviceID, int bWindowed, OSWindow *window)
{
  DWORD v26; // [sp-4h] [bp-DCh]@30
  DDSCAPS2 v27; // [sp+Ch] [bp-CCh]@37
  DDSURFACEDESC2 ddsd2; // [sp+1Ch] [bp-BCh]@11
  D3DVIEWPORT2 d3dvp2; // [sp+98h] [bp-40h]@28
  IDirectDrawClipper *lpddclipper; // [sp+C4h] [bp-14h]@18
  LPDIRECTDRAW lpDD; // [sp+C8h] [bp-10h]@1

  auto hWnd = window->GetApiHandle();
  int game_width = window->GetWidth();
  int game_height = window->GetHeight();

  this->bWindowed = bWindowed;
  this->hWindow = hWnd;

  //Создание объекта DirectDraw
  if (FAILED(DirectDrawCreate(pAvailableDevices[uDeviceID].pDirectDrawGUID, &lpDD, NULL)))
  {
    sprintf(pErrorMessage, "Init - Failed to create DirectDraw interface.\n");
    return 0;
  }

  //Запрос интерфейса IDirectDraw4
  if (FAILED(lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pHost)))
  {
    sprintf(pErrorMessage, "Init - Failed to create DirectDraw4 interface.\n");
    if (lpDD)
      lpDD->Release();
    return 0;
  }
  lpDD->Release();
  lpDD = NULL;

  //Задаём уровень совместного доступа для приложения DirectDraw в оконном режиме
  if (bWindowed && !pAvailableDevices[uDeviceID].pDirectDrawGUID)
  {
    if (FAILED(pHost->SetCooperativeLevel(hWnd, DDSCL_MULTITHREADED | DDSCL_NORMAL)))
    {
      sprintf(pErrorMessage, "Init - Failed to set cooperative level.\n");
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
    }

	//
    memset(&ddsd2, 0, sizeof(DDSURFACEDESC2));
    ddsd2.dwSize = sizeof(DDSURFACEDESC2);
    ddsd2.dwFlags = DDSD_CAPS;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	//Создаём первичную поверхность
    if ( FAILED(pHost->CreateSurface(&ddsd2, &pFrontBuffer, NULL)) )
    {
      sprintf(pErrorMessage, "Init - Failed to create front buffer.\n");
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
	}
    ddsd2.dwSize = sizeof(DDSURFACEDESC2);
    pHost->GetDisplayMode(&ddsd2);
    if ( FORCE_16_BITS && ddsd2.ddpfPixelFormat.dwRGBBitCount != 16 )
    {
      sprintf(pErrorMessage, "Init - Desktop isn't in 16 bit mode.\n");
      if (pFrontBuffer)
      {
        pFrontBuffer->Release();
        pFrontBuffer = NULL;
      }
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
    }

    ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    ddsd2.dwWidth = game_width;
    ddsd2.dwHeight = game_height;
    if (pHost->CreateSurface(&ddsd2, &pBackBuffer, NULL) )
    {
      sprintf(pErrorMessage, "Init - Failed to create back buffer.\n");
      if (pFrontBuffer)
      {
        pFrontBuffer->Release();
        pFrontBuffer = NULL;
      }
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
    }
    //Создание отсекателя DirectDraw
    if ( pHost->CreateClipper(0, &lpddclipper, NULL) )
    {
      sprintf(pErrorMessage, "Init - Failed to create clipper.\n");
      if (pBackBuffer)
      {
        pBackBuffer->Release();
        pBackBuffer = NULL;
      }
      if (pFrontBuffer)
      {
        pFrontBuffer->Release();
        pFrontBuffer= NULL;
      }
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
    }
    lpddclipper->SetHWnd(0, hWnd);
    pFrontBuffer->SetClipper(lpddclipper);

    lpddclipper->Release();
    lpddclipper = NULL;
	//

    pHost->QueryInterface(IID_IDirect3D3, (LPVOID *)&pDirect3D);

    ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
    ddsd2.dwWidth = game_width;
    ddsd2.dwHeight = game_height;

    if ( pDirect3D->EnumZBufferFormats(*pAvailableDevices[uDeviceID].pGUID,
           (HRESULT (__stdcall *)(DDPIXELFORMAT *, void *))D3DZBufferFormatEnumerator,
           &ddsd2.ddpfPixelFormat) )
    {
      sprintf(pErrorMessage, "Init - Failed to enumerate Z buffer formats.\n");
      if (pBackBuffer)
      {
        pBackBuffer->Release();
        pBackBuffer = NULL;
      }
      if (pFrontBuffer)
      {
        pFrontBuffer->Release();
        pFrontBuffer= NULL;
      }
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;	  
    }
    if ( uDeviceID == 2 || uDeviceID == 3 )
      ddsd2.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    if ( !pHost->CreateSurface(&ddsd2, &pZBuffer, NULL) )
    {
      if ( !pBackBuffer->AddAttachedSurface(pZBuffer) )
      {
        if ( !pDirect3D->CreateDevice(*pAvailableDevices[uDeviceID].pGUID, pBackBuffer, &pDevice, 0) )
        {
          memset(&d3dvp2, 0, sizeof(D3DVIEWPORT2));
          d3dvp2.dvClipWidth = 2.0;
          d3dvp2.dvClipY = 1.0;
          d3dvp2.dvClipHeight = 2.0;
          d3dvp2.dvMaxZ = 1.0;
          d3dvp2.dvMinZ = 0.0;
          goto LABEL_54;
        }
        sprintf(pErrorMessage, "Init - Failed to create D3D device.\n");
        if (pDirect3D)
        {
          pDirect3D->Release();
          pDirect3D = NULL;
        }
        if (pZBuffer)
        {
          pZBuffer->Release();
          pZBuffer = NULL;
        }
        if (pBackBuffer)
        {
          pBackBuffer->Release();
          pBackBuffer = NULL;
        }
        if (pFrontBuffer)
        {
          pFrontBuffer->Release();
          pFrontBuffer= NULL;
        }
        if (pHost)
        {
          pHost->Release();
          pHost = NULL;
        }
        return 0;
      }
      sprintf(pErrorMessage, "Init - Failed to attach z-buffer to back buffer.\n");
      if (pZBuffer)
      {
        pZBuffer->Release();
        pZBuffer = NULL;
      }
      if (pBackBuffer)
      {
        pBackBuffer->Release();
        pBackBuffer = NULL;
      }
      if (pFrontBuffer)
      {
        pFrontBuffer->Release();
        pFrontBuffer= NULL;
      }
      if (pHost)
      {
        pHost->Release();
        pHost = NULL;
      }
      return 0;
    }
    sprintf(pErrorMessage, "Init - Failed to create z-buffer.\n");
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = NULL;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= NULL;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = NULL;
    }
    return 0;
  }
  if ( uDeviceID == 1 )
    v26 = 1045;
  else
    v26 = 1041;
  if (pHost->SetCooperativeLevel(hWnd, v26) )
  {
    sprintf(pErrorMessage, "Init - Failed to set cooperative level.\n");
    if (pHost)
    {
      pHost->Release();
      pHost = NULL;
    }
    return 0;
  }
  if (pHost->SetDisplayMode(window->GetWidth(), window->GetHeight(), 16, 0, 0) )
  {
    sprintf(pErrorMessage, "Init - Failed to set display mode.\n");
    if (pHost)
    {
      pHost->Release();
      pHost = NULL;
    }
    return 0;
  }

  memset(&ddsd2, 0, sizeof(DDSURFACEDESC2));
  ddsd2.dwSize = sizeof(DDSURFACEDESC2);
  //Подключение полей с достоверными данными
  ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  //Запрос сложной структуры с возможностью переключения
  ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  //Присвоение полю счётчика задних буферов значения 1
  ddsd2.dwBackBufferCount = 1;
  if ( pHost->CreateSurface(&ddsd2, &pFrontBuffer, NULL) )
  {
    sprintf(pErrorMessage, "Init - Failed to create front buffer.\n");
    if (pHost)
    {
      pHost->Release();
      pHost = NULL;
    }
    return 0;  
  }
  //a3a = &pBackBuffer;
  //v14 = *v34;
  memset(&v27, 0, sizeof(DDSCAPS2));

  v27.dwCaps = DDSCAPS_BACKBUFFER;
  //v33 = (IDirect3DDevice3 **)v14->GetAttachedSurface(&v27, &pBackBuffer);
  //hWnda = &pDirect3D;
  pHost->QueryInterface(IID_IDirect3D3, (LPVOID *)&pDirect3D);

  if (FAILED(pFrontBuffer->GetAttachedSurface(&v27, &pBackBuffer)))
  {
    sprintf(pErrorMessage, "Init - Failed to get D3D interface.\n");
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = NULL;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= NULL;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = NULL;
    }
    return 0;
  }

  ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
  ddsd2.dwWidth = 640;
  ddsd2.dwHeight = 480;
  if ( pDirect3D->EnumZBufferFormats(*pAvailableDevices[uDeviceID].pGUID,
         (HRESULT (__stdcall *)(DDPIXELFORMAT *, void *))D3DZBufferFormatEnumerator,
         &ddsd2.ddpfPixelFormat) )
  {
    sprintf(pErrorMessage, "Init - Failed to enumerate Z buffer formats.\n");
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = 0;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= 0;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = 0;
    }
    return 0;
  }
  if ( uDeviceID == 2 || uDeviceID == 3 )
    BYTE1(ddsd2.ddsCaps.dwCaps) |= 8;
  //uDeviceIDa = &pZBuffer;
  if (pHost->CreateSurface(&ddsd2, &pZBuffer, NULL) )
  {
    sprintf(pErrorMessage, "Init - Failed to create z-buffer.\n");
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = 0;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= 0;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = 0;
    }
    return 0;
  }
  if (pBackBuffer->AddAttachedSurface(pZBuffer))
  {
    sprintf(pErrorMessage, "Init - Failed to attach z-buffer to back buffer.\n");
    if (pZBuffer)
    {
      pZBuffer->Release();
      pZBuffer = 0;
    }
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = 0;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= 0;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = 0;
    }
    return 0;  
  }
  //v33 = &pDevice;
  if (pDirect3D->CreateDevice(*pAvailableDevices[uDeviceID].pGUID, pBackBuffer, &pDevice, 0) )
  {
    sprintf(pErrorMessage, "Init - Failed to create D3D device.\n");
    if (pDirect3D)
    {
      pDirect3D->Release();
      pDirect3D = 0;
    }
    if (pZBuffer)
    {
      pZBuffer->Release();
      pZBuffer = 0;
    }
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = 0;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= 0;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = 0;
    }
    return 0;  
  }
  memset(&d3dvp2, 0, sizeof(D3DVIEWPORT2));
  d3dvp2.dvClipWidth = 2.0;
  d3dvp2.dvClipY = 1.0;
  d3dvp2.dvClipHeight = 2.0;
  d3dvp2.dvMaxZ = 1.0;

LABEL_54:
  d3dvp2.dwSize = sizeof(D3DVIEWPORT2);
  //v17 = *hWnda;
  d3dvp2.dwWidth = game_width;
  d3dvp2.dwHeight = game_height;
  d3dvp2.dvClipX = -1.0;
  //v18 = v17->lpVtbl;
  //v32 = &v4->pViewport;
  if (pDirect3D->CreateViewport(&pViewport, 0))
  {
    sprintf(pErrorMessage, "Init - Failed to create viewport.\n");
    if (pDevice)
    {
      pDevice->Release();
      pDevice = 0;
    }
    if (pDirect3D)
    {
      pDirect3D->Release();
      pDirect3D = 0;
    }
    if (pZBuffer)
    {
      pZBuffer->Release();
      pZBuffer = 0;
    }
    if (pBackBuffer)
    {
      pBackBuffer->Release();
      pBackBuffer = 0;
    }
    if (pFrontBuffer)
    {
      pFrontBuffer->Release();
      pFrontBuffer= 0;
    }
    if (pHost)
    {
      pHost->Release();
      pHost = 0;
    }
    return 0;
  }

  pDevice->AddViewport(pViewport);
  pViewport->SetViewport2(&d3dvp2);
  pDevice->SetCurrentViewport(pViewport);
  return 1;
}

//----- (0049E444) --------------------------------------------------------
unsigned int RenderD3D::GetDeviceCaps()
{
  unsigned int v1; // ebx@1
  unsigned int result; // eax@2
  D3DDEVICEDESC refCaps; // [sp+Ch] [bp-1F8h]@1
  D3DDEVICEDESC halCaps; // [sp+108h] [bp-FCh]@1

  v1 = 0;

  memset(&halCaps, 0, sizeof(halCaps));
  halCaps.dwSize = sizeof(halCaps);

  memset(&refCaps, 0, sizeof(refCaps));
  refCaps.dwSize = sizeof(refCaps);

  if ( this->pDevice->GetCaps(&halCaps, &refCaps) )
    result = 1;
  else
  {
    if ( !(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) )
      v1 = 2;
    if ( !(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ONE) )
      v1 |= 4;
    if ( !(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO) )
      v1 |= 8;
    if ( !(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) )
      v1 |= 16;
    if ( !(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE) )
      v1 |= 32;
    if ( !(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR) )
      v1 |= 64;
    if ( halCaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
      v1 |= 128;
    result = v1;
  }
  return result;
}

//----- (0049E4FC) --------------------------------------------------------
void RenderD3D::ClearTarget(unsigned int bClearColor, unsigned int uClearColor, unsigned int bClearDepth, float z_clear)
{
  uint uClearFlags = 0;

  if (bClearColor)
    uClearFlags |= D3DCLEAR_TARGET;
  if (bClearDepth)
    uClearFlags |= D3DCLEAR_ZBUFFER;
  
  D3DRECT rects[] = {{0, 0, window->GetWidth(), window->GetHeight()}};
  if (uClearFlags)
    pViewport->Clear2(1, rects, uClearFlags, uClearColor, z_clear, 0);
}

//----- (0049E54D) --------------------------------------------------------
void RenderD3D::Present(bool bForceBlit)
{
  RECT source_rect; // [sp+18h] [bp-18h]@1
  struct tagPOINT Point; // [sp+28h] [bp-8h]@4

  source_rect.left = 0;
  source_rect.top = 0;
  source_rect.bottom = 480;//window->GetHeight(); //Ritor1: проблема с кнопкой "развернуть"
  source_rect.right = 640; //window->GetWidth();

  if (bWindowed || bForceBlit)
  {
    RECT dest_rect;
    GetClientRect(hWindow, &dest_rect);
    Point.y = 0;
    Point.x = 0;
    ClientToScreen(hWindow, &Point);
    OffsetRect(&dest_rect, Point.x, Point.y);
    pFrontBuffer->Blt(&dest_rect, pBackBuffer, &source_rect, DDBLT_WAIT, NULL);
  }
  else
    pFrontBuffer->Flip(NULL, DDFLIP_WAIT);
}

//----- (0049E5D4) --------------------------------------------------------
bool RenderD3D::CreateTexture(unsigned int uTextureWidth, unsigned int uTextureHeight, IDirectDrawSurface4 **pOutSurface, IDirect3DTexture2 **pOutTexture, bool bAlphaChannel, bool bMipmaps, unsigned int uMinDeviceTexDim)
{
  unsigned int v9; // ebx@5
  unsigned int v10; // eax@5
  DWORD v11; // edx@5
  DDSURFACEDESC2 ddsd2; // [sp+Ch] [bp-80h]@1

  memset(&ddsd2, 0, sizeof(ddsd2));
  ddsd2.dwSize = sizeof(ddsd2);
  ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
  ddsd2.dwHeight = uTextureHeight;
  ddsd2.dwWidth = uTextureWidth;
  if ( bMipmaps )
  {
    if ( (signed int)uTextureHeight <= (signed int)uTextureWidth )
    {
      ddsd2.dwMipMapCount = GetMaxMipLevels(uTextureHeight) - GetMaxMipLevels(uMinDeviceTexDim);
      if ( ddsd2.dwMipMapCount )
      {
        ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
        ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
      }
      goto LABEL_12;
    }
    if ( (signed int)uTextureWidth < (signed int)uMinDeviceTexDim )
    {
      ddsd2.dwMipMapCount = GetMaxMipLevels(uMinDeviceTexDim);
      if ( ddsd2.dwMipMapCount )
      {
        ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
        ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
      }
      goto LABEL_12;
    }
    v9 = GetMaxMipLevels(uTextureWidth);
    v10 = GetMaxMipLevels(uMinDeviceTexDim);
    ddsd2.dwMipMapCount = v9 - v10;
    if ( v9 == v10 )
    {
      ddsd2.dwFlags = 0x1007;
      __debugbreak(); // warning C4700: uninitialized local variable 'v11' used
      ddsd2.ddsCaps.dwCaps = v11;
      goto LABEL_12;
    }
  }
  else
    ddsd2.dwMipMapCount = 1;
  ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
  ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
LABEL_12:
  ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;
  ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
  if (bAlphaChannel)
  {
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwRBitMask = 0x7C00;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x03E0;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
  }
  else
  {
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd2.ddpfPixelFormat.dwRBitMask = 0xF800;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x07E0;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0;
  }
  if (FAILED(pHost->CreateSurface(&ddsd2, pOutSurface, NULL)))
    return false;
  if (FAILED((*pOutSurface)->QueryInterface(IID_IDirect3DTexture2, (void **)pOutTexture)))
  {
    (*pOutSurface)->Release();
    *pOutSurface = 0;
    return false;
  }
  return true;
}

//----- (004A5190) --------------------------------------------------------
void RenderD3D::HandleLostResources()
{
  pBitmaps_LOD->ReleaseLostHardwareTextures();
  pBitmaps_LOD->_410423_move_textures_to_device();
  pSprites_LOD->ReleaseLostHardwareSprites();
}

//----- (004A2050) --------------------------------------------------------
void Render::DrawPolygon(unsigned int uNumVertices, struct Polygon *a3, ODMFace *a4, IDirect3DTexture2 *pTexture)
{
  unsigned int v6; // ebx@1
  int v8; // eax@7
  unsigned int v41; // eax@29
  //unsigned int v54; // [sp+5Ch] [bp-Ch]@3
  signed int a2; // [sp+64h] [bp-4h]@4

  v6 = 0;
  if ( this->uNumD3DSceneBegins && (signed int)uNumVertices >= 3 )
  {
    //v54 = pEngine->pLightmapBuilder->StationaryLightsCount;
    if ( pEngine->pLightmapBuilder->StationaryLightsCount)
      a2 = -1;
    pEngine->AlterGamma_ODM(a4, &a2);
    if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_01_lightmap_related)
    {
      v8 = ::GetActorTintColor(a3->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 0, 0);
      pEngine->pLightmapBuilder->DrawLightmaps(v8/*, 0*/);
    }
    else
    {
      if ( !pEngine->pLightmapBuilder->StationaryLightsCount || byte_4D864C && pEngine->uFlags & 2 )
      {
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));
        if (bUsingSpecular)
        {
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
        }
		for (uint i = 0; i < uNumVertices; ++i)
		{
		
		  d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
		  d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
		  d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
		  d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
		  d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(a3->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
          pEngine->AlterGamma_ODM(a4, &d3d_vertex_buffer[i].diffuse);

		  if ( this->bUsingSpecular )
            d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);
          else
            d3d_vertex_buffer[i].specular = 0;
		  d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
		  d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
		  
		}

        if (a4->uAttributes & FACE_OUTLINED)
        {
          int color;
          if (GetTickCount() % 300 >= 150)
            color = 0xFFFF2020;
          else color = 0xFF901010;

          for (uint i = 0; i < uNumVertices; ++i)
            d3d_vertex_buffer[i].diffuse = color;
        }

        pRenderD3D->pDevice->SetTexture(0, pTexture);
        pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                           D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                           d3d_vertex_buffer,
                                           uNumVertices,
                                           D3DDP_DONOTLIGHT);
      }
      else
      {
		for (uint i = 0; i < uNumVertices; ++i)
		{
		
		  d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
		  d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
		  d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
		  d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
		  d3d_vertex_buffer[i].diffuse = GetActorTintColor(a3->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
          if ( this->bUsingSpecular )
            d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);
          else
            d3d_vertex_buffer[i].specular = 0;
		  d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
		  d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
		  
		}
	  
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        if (bUsingSpecular)
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

        ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                  D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                                  d3d_vertex_buffer,
                                                  uNumVertices,
                                                  D3DDP_DONOTLIGHT));
        //v50 = (const char *)v5->pRenderD3D->pDevice;
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
        //(*(void (**)(void))(*(int *)v50 + 88))();
        pEngine->pLightmapBuilder->DrawLightmaps(-1/*, 0*/);
	    for (uint i = 0; i < uNumVertices; ++i)
		{
		  d3d_vertex_buffer[i].diffuse = a2;
		}
        ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture));
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        if ( !render->bUsingSpecular )
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                  D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                                  d3d_vertex_buffer,
                                                  uNumVertices,
                                                  D3DDP_DONOTLIGHT));
        if (bUsingSpecular)
        {
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

		  for (uint i = 0; i < uNumVertices; ++i)
		  {
		    d3d_vertex_buffer[i].diffuse = render->uFogColor | d3d_vertex_buffer[i].specular & 0xFF000000;
		    d3d_vertex_buffer[i].specular = 0;
		  }

          ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA));
          ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                    D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                                    d3d_vertex_buffer,
                                                    uNumVertices,
                                                    D3DDP_DONOTLIGHT));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
          //v40 = render->pRenderD3D->pDevice->lpVtbl;
          v41 = GetLevelFogColor();
          pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF);
          v6 = 0;
          pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0);
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, v6));
      }
    }
  }
}
// 4D864C: using guessed type char byte_4D864C;

//----- (0049EB79) --------------------------------------------------------
Render::~Render()
{
  free(this->pDefaultZBuffer);
  this->pD3DBitmaps.Release();
  this->pD3DSprites.Release();
  Release();
  this->bWindowMode = 1;
  //nullsub_1();
  //nullsub_1();
}


//----- (0049E992) --------------------------------------------------------
Render::Render(): IRender()
{
  //Render *v1; // esi@1
//  int v2; // eax@1
//  char v3; // zf@1

  //v1 = this;
  this->pDirectDraw4 = nullptr;
  this->pFrontBuffer4 = nullptr;
  this->pBackBuffer4 = nullptr;
  //this->pColorKeySurface4 = 0;
  //this->pDirectDraw2 = 0;
  //this->pFrontBuffer2 = 0;
  //this->pBackBuffer2 = 0;
  //this->pSomeSurface2 = 0;
  //RenderHWLContainer::RenderHWLContainer(&this->pD3DBitmaps);
  //RenderHWLContainer::RenderHWLContainer(&v1->pD3DSprites);
  this->bWindowMode = 1;
  //this->field_40054 = 0;
  //this->field_10 = 640;
  //this->field_14 = 480;
  //this->field_40030 = 0;
  //this->field_4002C = 0;
  this->pActiveZBuffer = nullptr;
  this->pDefaultZBuffer = nullptr;
  this->raster_clip_y = 0;
  this->raster_clip_x = 0;
  this->raster_clip_z = 639;
  this->raster_clip_w = 479;
  //this->field_4003C = 0x004EED80;
  //this->field_40040 = dword_4EED78;
  this->uClipZ = 640;
  //this->field_40044 = 2;
  //this->field_40048 = 6;
  this->pFrontBuffer4 = nullptr;
  this->pBackBuffer4 = nullptr;
  //this->pColorKeySurface4 = 0;
  this->pDirectDraw4 = nullptr;
  this->pRenderD3D = 0;
  this->uNumSceneBegins = 0;
  this->uNumD3DSceneBegins = 0;
  this->using_software_screen_buffer = 0;
  this->pTargetSurface = nullptr;
  this->uTargetSurfacePitch = 0;
  this->uClipY = 0;
  this->uClipX = 0;
  this->uClipW = 480;
  this->bClip = 1;
  //this->bColorKeySupported = 0;
  this->bRequiredTextureStagesAvailable = 0;
  this->bTinting = 1;
  //LOBYTE(this->field_103668) = 0;
  uNumBillboardsToDraw = 0;
  bFogEnabled = false;

  hd_water_tile_id = -1;
  hd_water_current_frame = 0;
}

bool Render::Initialize(OSWindow *window/*, bool bColoredLights, uint32_t uDetailLevel, bool bTinting*/)
{
  //bUserDirect3D = true;//ReadWindowsRegistryInt("Use D3D", 0);

  this->window = window;
  //bStartInWindow = true;
  //windowed_mode_width = windowed_width;
  //windowed_mode_height = windowed_height;

  uDesiredDirect3DDevice = ReadWindowsRegistryInt("D3D Device", 0);

  bUseColoredLights = ReadWindowsRegistryInt("Colored Lights", false);
  uLevelOfDetail = ReadWindowsRegistryInt("Detail Level", 1);
  bTinting = ReadWindowsRegistryInt("Tinting", 1) != 0;

  bool r1 = pD3DBitmaps.Load(L"data\\d3dbitmap.hwl");
  bool r2 = pD3DSprites.Load(L"data\\d3dsprite.hwl");

  return r1 && r2;
}


//----- (0049ECC4) --------------------------------------------------------
void Render::ClearBlack()
{
  //if (pRenderD3D)
  {
    if (using_software_screen_buffer)
      pRenderD3D->ClearTarget(true, 0, false, 0.0);
  }
  //else
    //memset(render->pTargetSurface, 0, 4 * (field_10 * field_14 / 2));
}

//----- (0049ED18) --------------------------------------------------------
void Render::PresentBlackScreen()
{
  IDirectDrawSurface *lpddsback; // eax@3
  DDBLTFX lpDDBltFx; // [sp+4h] [bp-74h]@5
  RECT dest_rect; // [sp+68h] [bp-10h]@3

  memset(&lpDDBltFx, 0, sizeof(DDBLTFX));
  lpDDBltFx.dwSize = sizeof(DDBLTFX);

  GetWindowRect(window->GetApiHandle(), &dest_rect);
  lpddsback = (IDirectDrawSurface *)this->pBackBuffer4;

  lpDDBltFx.dwFillColor = 0;
  lpddsback->Blt(&dest_rect, NULL, NULL, DDBLT_COLORFILL, &lpDDBltFx);
  render->Present();
}

//----- (0049EDB6) --------------------------------------------------------
void Render::SavePCXScreenshot()
{
  int v5; // eax@8
  FILE *pOutFile; // edi@10
  unsigned short *v8; // eax@11
  signed int v12; // eax@18
  char v15[56]; // [sp+Ch] [bp-158h]@10
  DDSURFACEDESC2 Dst; // [sp+48h] [bp-11Ch]@7
  char color_map[48]; // [sp+C4h] [bp-A0h]@10
  char Filename[40]; // [sp+F4h] [bp-70h]@3
  char *lineB; // [sp+11Ch] [bp-48h]@14
  char *lineG; // [sp+120h] [bp-44h]@14
  FILE *File; // [sp+128h] [bp-3Ch]@3
  PCXHeader_1 header1; // [sp+130h] [bp-34h]@10
  PCXHeader_2 header2; // [sp+140h] [bp-24h]@10
  char *lineRGB; // [sp+148h] [bp-1Ch]@10
  void *surface; // [sp+14Ch] [bp-18h]@8
  unsigned int image_width; // [sp+150h] [bp-14h]@4
  int pitch; // [sp+154h] [bp-10h]@4
  char v31; // [sp+15Ah] [bp-Ah]@25
  unsigned char pict_byte; // [sp+15Bh] [bp-9h]@17
  unsigned short *line_picture_data; // [sp+15Ch] [bp-8h]@10
  byte test_byte; // [sp+163h] [bp-1h]@17

  int num_r_bits = 5;
  int num_g_bits = 6;
  int num_b_bits = 5;

  int r_mask = 0xF800;
  int g_mask = 0x7E0;
  int b_mask = 0x1F;

  if ( !this->pRenderD3D || this->using_software_screen_buffer )
  {
    sprintf(Filename, "screen%0.2i.pcx", ScreenshotFileNumber++ % 100);
    File = fopen(Filename, "wb");
    if ( File )
    {
      pitch = this->GetRenderWidth();
      if ( pitch & 1 )
        pitch = pitch + 1;
      if ( this->pRenderD3D )
      {
        memset(&Dst, 0, sizeof(Dst));
        Dst.dwSize = sizeof(Dst);
        if ( !render->LockSurface_DDraw4(render->pBackBuffer4, &Dst, DDLOCK_WAIT) )
          return;
        surface = Dst.lpSurface;
        v5 = Dst.lPitch / 2;
      }
      else
      {
        render->BeginScene();
        surface = render->pTargetSurface;
        v5 = render->uTargetSurfacePitch;
      }
      header1.right = GetRenderWidth() - 1;
      header1.left = 0;
      header1.bottom = this->GetRenderHeight() - 1;
      header1.up = 0;
      header2.pitch = pitch;
      memset(color_map, 0, sizeof(color_map));
      memset(v15, 0, sizeof(v15));
      header2.reserved = 0;
      header1.manufacturer = 10;
      pOutFile = File;
      header1.version = 5;
      header1.encoding = 1;
      header1.bpp = 8;
      header1.hdpi = 75;
      header1.vdpi = 75;
      header2.planes = 3;
      header2.palette_info = 1;
      fwrite(&header1, 1, 1, File);
      fwrite(&header1.version, 1, 1, pOutFile);
      fwrite(&header1.encoding, 1, 1, pOutFile);
      fwrite(&header1.bpp, 1, 1, pOutFile);
      fwrite(&header1.left, 2, 1, pOutFile);
      fwrite(&header1.up, 2, 1, pOutFile);
      fwrite(&header1.right, 2, 1, pOutFile);
      fwrite(&header1.bottom, 2, 1, pOutFile);
      fwrite(&header1.hdpi, 2, 1, pOutFile);
      fwrite(&header1.vdpi, 2, 1, pOutFile);
      fwrite(color_map, 0x30, 1, pOutFile);
      fwrite(&header2, 1, 1, pOutFile);
      fwrite(&header2.planes, 1, 1, pOutFile);
      fwrite(&header2.pitch, 2, 1, pOutFile);
      fwrite(&header2.palette_info, 2, 1, pOutFile);
      fwrite(v15, 0x3Au, 1, pOutFile);
      lineRGB = (char *)malloc(3 * GetRenderWidth() + 6);
      if ( this->GetRenderHeight() > 0 )
      {
        image_width = 3 * pitch;
        //v24 = 2 * v5;
        v8 = (unsigned short *)surface;
        for ( unsigned int y = 0; y < this->GetRenderHeight(); y++ )
        {
          line_picture_data = v8;
          if ( GetRenderWidth() > 0 )
          {
            lineG = (char *)lineRGB + pitch;
            lineB = (char *)lineRGB + 2 * pitch;
            for ( uint x = 0; x < this->GetRenderWidth(); x++ )
            {
			  //int p = *line_picture_data; //0x2818
              //int for_rad = (render->uTargetGBits + render->uTargetBBits );//16 = 8 + 8
			  //int value = (render->uTargetRMask & *line_picture_data);//0 = 0xFF0000 & 0x2818
			  //int result = (render->uTargetRMask & *line_picture_data) >> (render->uTargetGBits + render->uTargetBBits );
              lineRGB[x] = (uTargetRMask & *line_picture_data) >> (uTargetGBits + uTargetBBits );// + render->uTargetRBits - 8);
              lineG[x] = (uTargetGMask & *line_picture_data) >> (uTargetBBits);// + render->uTargetGBits - 8);
			  //int value2 = (render->uTargetGMask & *line_picture_data); //10240 = 0xFF00 & 0x2818
			  //int result2 = (render->uTargetGMask & *line_picture_data) >> (render->uTargetBBits);
              lineB[x] = (uTargetBMask & *line_picture_data);// << (8 - render->uTargetBBits);
		      //int value3 = (render->uTargetBMask & *line_picture_data);//24 = 0xFF & 0x2818
              line_picture_data += 2;
            }
          }
          for ( uint i = 0; i < image_width; i += test_byte )
          {
            pict_byte = lineRGB[i];
            for ( test_byte = 1; test_byte < 0x3F; ++test_byte )
            {
              v12 = i + test_byte;
              if ( lineRGB[v12] != pict_byte )
                break;
              if ( !(v12 % pitch) )
                break;
            }
            if ( i + test_byte > image_width )
              test_byte = 3 * pitch - i;
            if ( test_byte > 1 || pict_byte >= 0xC0 )
            {
              v31 = test_byte | 0xC0;
              fwrite(&v31, 1, 1, pOutFile);
            }
            fwrite(&pict_byte, 1, 1, pOutFile);
          }
          v8 += v5;
        }
      }
      if ( this->pRenderD3D )
        ErrD3D(render->pBackBuffer4->Unlock(NULL));
      else
        render->EndScene();

      free(lineRGB);
      fclose(pOutFile);
    }
  }
}

//----- (0049F1BC) --------------------------------------------------------
void Render::SaveWinnersCertificate(const char *a1)
{
  unsigned int v6; // eax@8
  //FILE *v7; // edi@10
//  int v8; // ecx@11
  unsigned short *v9; // eax@11
  int v10; // eax@13
  signed int v13; // eax@18
//  char v14; // zf@27
//  HRESULT v15; // eax@29
  char v16[56]; // [sp+Ch] [bp-12Ch]@10
  __int16 v17; // [sp+44h] [bp-F4h]@10
  DDSURFACEDESC2 Dst; // [sp+48h] [bp-F0h]@7
//  int v19; // [sp+58h] [bp-E0h]@8
//  unsigned __int16 *v20; // [sp+6Ch] [bp-CCh]@8
  char color_map[48]; // [sp+C4h] [bp-74h]@10
//  unsigned int v22; // [sp+F4h] [bp-44h]@11
  char *lineB; // [sp+F8h] [bp-40h]@14
  int image_width; // [sp+FCh] [bp-3Ch]@11
  int v25; // [sp+100h] [bp-38h]@4
  FILE *File; // [sp+104h] [bp-34h]@3
  char Str; // [sp+108h] [bp-30h]@10
  char v28; // [sp+109h] [bp-2Fh]@10
  char v29; // [sp+10Ah] [bp-2Eh]@10
  char v30; // [sp+10Bh] [bp-2Dh]@10
  __int16 v31; // [sp+10Ch] [bp-2Ch]@10
  __int16 v32; // [sp+10Eh] [bp-2Ah]@10
  __int16 v33; // [sp+110h] [bp-28h]@10
  __int16 v34; // [sp+112h] [bp-26h]@10
  __int16 v35; // [sp+114h] [bp-24h]@10
  __int16 v36; // [sp+116h] [bp-22h]@10
  char v37; // [sp+118h] [bp-20h]@10
  char v38; // [sp+119h] [bp-1Fh]@10
  __int16 v39; // [sp+11Ah] [bp-1Eh]@10
  __int16 v40; // [sp+11Ch] [bp-1Ch]@10
  char *lineRGB; // [sp+120h] [bp-18h]@10
  void *surface; // [sp+124h] [bp-14h]@8
  int pitch; // [sp+128h] [bp-10h]@4
  char v44; // [sp+12Fh] [bp-9h]@25
  char *lineG; // [sp+130h] [bp-8h]@10
  unsigned char pict_byte; // [sp+137h] [bp-1h]@17
  byte test_byte;

  int num_r_bits = 5;
  int num_g_bits = 6;
  int num_b_bits = 5;

  int r_mask = 0xF800;
  int g_mask = 0x7E0;
  int b_mask = 0x1F;

  if ( !this->pRenderD3D || this->using_software_screen_buffer )
  {
    static int _4EFA84_num_winners_certificates = 0;
    ++_4EFA84_num_winners_certificates;

    File = fopen(a1, "wb");
    if ( File )
    {
      v25 = this->GetRenderWidth();
      pitch = v25;
      if ( pitch & 1 )
        pitch = pitch + 1;
      if ( this->pRenderD3D )
      {
        memset(&Dst, 0, 0x7C);
        Dst.dwSize = 124;
        if ( !render->LockSurface_DDraw4(render->pBackBuffer4, (DDSURFACEDESC2 *)&Dst, DDLOCK_WAIT) )
          return;
        surface = Dst.lpSurface;
        v6 = Dst.lPitch / 2;
      }
      else
      {
        render->BeginScene();
        surface = render->pTargetSurface;
        v6 = render->uTargetSurfacePitch;
      }
      v33 = this->GetRenderWidth() - 1;
      v31 = 0;
      v34 = (short)this->GetRenderHeight() - 1;
      v32 = 0;
      v39 = pitch;
      memset(&color_map, 0, sizeof(color_map));
      memset(&v16, 0, sizeof(v16));
      v37 = 0;
      Str = 10;
      v17 = 0;
      v28 = 5;
      v29 = 1;
      v30 = 8;
      v35 = 75;
      v36 = 75;
      v38 = 3;
      v40 = 1;
      fwrite(&Str, 1, 1, File);
      fwrite(&v28, 1, 1, File);
      fwrite(&v29, 1, 1, File);
      fwrite(&v30, 1, 1, File);
      fwrite(&v31, 2, 1, File);
      fwrite(&v32, 2, 1, File);
      fwrite(&v33, 2, 1, File);
      fwrite(&v34, 2, 1, File);
      fwrite(&v35, 2, 1, File);
      fwrite(&v36, 2, 1, File);
      fwrite(&color_map, 0x30, 1, File);
      fwrite(&v37, 1, 1, File);
      fwrite(&v38, 1, 1, File);
      fwrite(&v39, 2, 1, File);
      fwrite(&v40, 2, 1, File);
      fwrite(&v16, 0x3A, 1, File);
      lineRGB = (char *)malloc(3 * (v25 + 2));
      if ( (signed int)this->GetRenderHeight() > 0 )
      {
        image_width = 3 * pitch;
        v9 = (unsigned short *)surface;
        for ( uint j = 0; j < this->GetRenderHeight(); j++)
        {
          a1 = (const char *)v9;
          if ( v25 > 0 )
          {
            lineG = (char *)lineRGB + pitch;
            lineB = (char *)lineRGB + 2 * pitch;
            for ( v10 = 0; v10 < v25; v10++ )
            {
              lineRGB[v10] = (signed int)(r_mask & *(short *)a1) >> (num_g_bits + num_b_bits + num_r_bits - 8);
              lineG[v10] = (signed int)(g_mask & *(short *)a1) >> (num_b_bits + num_g_bits - 8);
              lineB[v10] = (b_mask & *(short *)a1) << (8 - num_b_bits);
              a1 += 2;
            }
          }
          for ( uint i = 0; i < image_width; i += test_byte )
          {
            pict_byte = lineRGB[i];
            for ( test_byte = 1; test_byte < 0x3F; test_byte )
            {
              v13 = i + test_byte;
              if ( lineRGB[v13] != pict_byte )
                break;
              if ( !(v13 % pitch) )
                break;
            }
            if ( i + test_byte > image_width )
              test_byte = 3 * pitch - i;
            if ( test_byte > 1 || pict_byte >= 0xC0 )
            {
              v44 = test_byte | 0xC0;
              fwrite(&v44, 1, 1, File);
            }
            fwrite(&pict_byte, 1, 1, File);
          }
          v9 += pitch;
        }
      }
      if ( this->pRenderD3D )
        ErrD3D(render->pBackBuffer4->Unlock(NULL));
      else
        render->EndScene();
      free(lineRGB);
      fclose(File);
    }
  }
}

//----- (0049F5A2) --------------------------------------------------------
void Render::PackPCXpicture( unsigned short* picture_data, int wight, int heidth, void *data_buff, int max_buff_size,unsigned int* packed_size )
{
  void *v8; // esi@3
  void *v9; // esi@3
  unsigned short* v11; // eax@4
//  int v13; // eax@8
//  int v14; // ecx@8
  signed int v15; // eax@11
//  char v16; // zf@20
//  int result; // eax@21
  char v18[58]; // [sp+Ch] [bp-ACh]@3
  char v20[48]; // [sp+48h] [bp-70h]@3
  char *lineG; // [sp+78h] [bp-40h]@7
  char *lineB; // [sp+7Ch] [bp-3Ch]@7
  int v23; // [sp+80h] [bp-38h]@4
  int v24; // [sp+84h] [bp-34h]@4
  int v25; // [sp+88h] [bp-30h]@4
  int v26; // [sp+8Ch] [bp-2Ch]@4
  PCXHeader_1 Src; // [sp+90h] [bp-28h]@3
  PCXHeader_2 v27; // [sp+A0h] [bp-18h]@3
  char *lineRGB; // [sp+A8h] [bp-10h]@3
  int pitch; // [sp+ACh] [bp-Ch]@1
  char v43; // [sp+B3h] [bp-5h]@18
  int i; // [sp+B4h] [bp-4h]@6
  unsigned short* line_picture_data;
  byte test_byte;
  unsigned char pict_byte;

  int num_r_bits = 5;
  int num_g_bits = 6;
  int num_b_bits = 5;

  int r_mask = 0xF800;
  int g_mask = 0x7E0;
  int b_mask = 0x1F;

  pitch = wight;
  if ( wight & 1 )
      pitch = wight + 1;
  Src.left = 0;
  Src.up = 0;
  Src.right = wight - 1;
  Src.bottom = heidth - 1;
  v27.pitch = pitch;
  memset(&v20, 0, 0x30u);
  memset(&v18, 0, 0x38u);
  v8 = data_buff;
  v27.reserved = 0;
  *(_WORD *)&v18[56] = 0;
  Src.manufacturer = 10;
  Src.version = 5;
  Src.encoding = 1;
  Src.bpp = 8;
  Src.hdpi = 75;
  Src.vdpi = 75;
  v27.planes = 3;
  v27.palette_info = 1;
  memcpy(data_buff, &Src, 1);
  v8 = (char *)v8 + 1;
  memcpy(v8, &Src.version, 1);
  v8 = (char *)v8 + 1;
  memcpy(v8, &Src.encoding, 1);
  v8 = (char *)v8 + 1;
  memcpy(v8, &Src.bpp, 1);
  v8 = (char *)v8 + 1;
  memcpy(v8, &Src.left, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &Src.up, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &Src.right, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &Src.bottom, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &Src.hdpi, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &Src.vdpi, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &v20, 0x30u);
  v8 = (char *)v8 + 48;
  memcpy(v8, &v27, 1u);
  v8 = (char *)v8 + 1;
  memcpy(v8, &v27.planes, 1);
  v8 = (char *)v8 + 1;
  memcpy(v8, &v27.pitch, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &v27.palette_info, 2);
  v8 = (char *)v8 + 2;
  memcpy(v8, &v18, 0x3Au);
  v9 = (char *)v8 + 58;

  lineRGB = (char*)malloc(3 * (wight + 2));
  if ( heidth > 0 )
  {
    v26 = 3 * pitch;
    v23 = 2 * wight;
    v11 = picture_data;
    v24 = (int)picture_data;
    for ( v25 = heidth; v25; v25-- )
    {
      line_picture_data = v11;
      if ( wight > 0 )
      {
        lineG = (char *)lineRGB + pitch;
        lineB = (char *)lineRGB + 2 * pitch;
        for ( uint i = 0; i < wight; i++ )
        {
          lineRGB[i] = (signed int)(r_mask & *line_picture_data) >> (num_g_bits  + num_b_bits  + num_r_bits  - 8);
          lineG[i] = (signed int)(g_mask & *line_picture_data) >> ( num_b_bits + num_g_bits- 8);
          lineB[i] = (b_mask & *line_picture_data) << (8 - num_b_bits);
          line_picture_data += 1;
        }
      }
      for ( i = 0; i < v26; v9 = (char *)v9 + 1 )
      {
        pict_byte = lineRGB[i];
        for ( test_byte = 1; test_byte < 63; ++test_byte )
        {
          v15 = i + test_byte;
          if ( lineRGB[v15] != pict_byte )//Uninitialized memory access
            break;
          if ( !(v15 % pitch) )
            break;
        }
        if ( i + test_byte > v26 )
          test_byte = 3 * pitch - i;
        if ( test_byte > 1 || pict_byte >= 192 )
        {
          v43 = test_byte | 0xC0;
          memcpy(v9, &v43, 1);
          v9 = (char *)v9 + 1;
        }
        memcpy(v9, &pict_byte, 1);
        i += test_byte;
      }
      v11 += wight;
    }
  }
  free(lineRGB);
  *(int *)packed_size = (char *)v9 - data_buff;
}

//----- (0049F8B5) --------------------------------------------------------
void Render::SavePCXImage(const String &filename, unsigned short* picture_data, int width, int height)
{
    FILE *result; // eax@1
    FILE *pOutFile; // edi@4
    unsigned short* v9; // eax@5
  //  int v10; // eax@7
    signed int v12; // eax@12
  //  char v13; // zf@21
    char v14[56]; // [sp+4h] [bp-A0h]@4
    __int16 v15; // [sp+3Ch] [bp-68h]@4
    char color_map[48]; // [sp+40h] [bp-64h]@4
    int v18; // [sp+74h] [bp-30h]@5
  //  char *v19; // [sp+78h] [bp-2Ch]@5
    int image_width; // [sp+7Ch] [bp-28h]@5
    PCXHeader_1 header1; // [sp+80h] [bp-24h]@4
    PCXHeader_2 header2; // [sp+90h] [bp-14h]@4
    char *lineRGB; // [sp+98h] [bp-Ch]@4
    int pitch; // [sp+9Ch] [bp-8h]@2
    char *lineB; // [sp+A0h] [bp-4h]@8
    char *lineG;
    unsigned short* line_pictute_data;
    byte test_byte;
    char v43;

    int num_r_bits = 5;
    int num_g_bits = 6;
    int num_b_bits = 5;

    int r_mask = 0xF800;
    int g_mask = 0x7E0;
    int b_mask = 0x1F;

    result = fopen(filename.c_str(), "wb");
    if (result)
    {
        pitch = width;
        if (width & 1)
            pitch = width + 1;
        header1.left = 0;
        header1.up = 0;
        header1.right = width - 1;
        header1.bottom = height - 1;
        header2.pitch = pitch;
        memset(color_map, 0, sizeof(color_map));
        header2.reserved = 0;
        memset(v14, 0, sizeof(v14));
        v15 = 0;
        header1.manufacturer = 10;
        header1.version = 5;
        header1.encoding = 1;
        header1.bpp = 8;
        header1.hdpi = 75;
        header1.vdpi = 75;
        header2.planes = 3;
        header2.palette_info = 1;
        fwrite(&header1, 1, 1, result);
        pOutFile = result;
        fwrite(&header1.version, 1, 1, result);
        fwrite(&header1.encoding, 1, 1, pOutFile);
        fwrite(&header1.bpp, 1, 1, pOutFile);
        fwrite(&header1.left, 2, 1, pOutFile);
        fwrite(&header1.up, 2, 1, pOutFile);
        fwrite(&header1.right, 2, 1, pOutFile);
        fwrite(&header1.bottom, 2, 1, pOutFile);
        fwrite(&header1.hdpi, 2, 1, pOutFile);
        fwrite(&header1.vdpi, 2, 1, pOutFile);
        fwrite(color_map, 0x30u, 1, pOutFile);
        fwrite(&header2, 1, 1, pOutFile);
        fwrite(&header2.planes, 1, 1, pOutFile);
        fwrite(&header2.pitch, 2, 1, pOutFile);
        fwrite(&header2.palette_info, 2, 1, pOutFile);
        fwrite(v14, 0x3Au, 1, pOutFile);

        lineRGB = (char *)malloc(3 * (width + 2));
        //При сохранении изображения подряд идущие пиксели одинакового цвета объединяются и вместо указания цвета для каждого пикселя
        //указывается цвет группы пикселей и их количество.
        image_width = 3 * pitch;
        v9 = picture_data;
        for (v18 = 0; v18 < height; v18++)//столбец
        {
            line_pictute_data = v9;
            lineG = (char *)lineRGB + pitch;
            lineB = (char *)lineRGB + 2 * pitch;

            for (int i = 0; i < width; i++)//строка
            {
                lineRGB[i] = (signed int)(r_mask & *line_pictute_data) >> (num_g_bits + num_b_bits + num_r_bits - 8);
                lineG[i] = (signed int)(g_mask & *line_pictute_data) >> (num_b_bits + num_g_bits - 8);
                lineB[i] = (b_mask & *line_pictute_data) << (8 - num_b_bits);
                line_pictute_data += 1;
            }
            test_byte = 1;
            for (int i = 0; (signed int)i < image_width; i += test_byte)
            {
                unsigned char pic_byte = lineRGB[i];
                for (test_byte; test_byte < 63; ++test_byte)// расчёт количества одинаковых цветов
                {
                    v12 = i + test_byte;
                    if (lineRGB[v12] != pic_byte)
                        break;
                    if (!(v12 % pitch))
                        break;
                }
                if (i + test_byte > image_width)
                    test_byte = 3 * pitch - i;
                if (test_byte > 1 || pic_byte >= 0xC0)
                {
                    v43 = test_byte | 0xC0;//тест-байт объединения
                    fwrite(&v43, 1, 1, pOutFile);
                }
                fwrite(&pic_byte, 1, 1, pOutFile);
            }
            v9 += width;
        }
        free(lineRGB);
        fclose(pOutFile);
    }
}

//----- (0049FBCD) --------------------------------------------------------
void Render::ClearTarget(unsigned int uColor)
{
  //if (pRenderD3D)
  {
    if (using_software_screen_buffer)
      pRenderD3D->ClearTarget(true, uColor, false, 0.0);
  }
  //else
    //memset32(pTargetSurface, uColor, field_10 * field_14 / 2);
}


//----- (0049FC37) --------------------------------------------------------
void Render::Present()
{
  //struct tagRECT Rect; // [sp+8h] [bp-28h]@11
  //RECT a4; // [sp+18h] [bp-18h]@11
  //struct tagPOINT Point; // [sp+28h] [bp-8h]@11

  if ( !pRenderD3D || this->using_software_screen_buffer )
  {
    this->pBeforePresentFunction();
    if ( this->pRenderD3D )
    {
      if ( this->using_software_screen_buffer )
        pRenderD3D->Present(false);
    }
    else
      __debugbreak(); // no sr
    /*{
      if ( this->bWindowMode )
      {
        RestoreFrontBuffer();
        GetClientRect(this->hWnd, &Rect);
        Point.y = 0;
        Point.x = 0;
        ClientToScreen(this->hWnd, &Point);
        OffsetRect(&Rect, Point.x, Point.y);
        a4.top = 0;
        a4.bottom = 480;
        a4.left = 0;
        a4.right = 640;
        PresentRect(&Rect, &a4);
      }
      else
      {
        RestoreFrontBuffer();
        a4.top = 0;
        a4.bottom = 480;
        a4.left = 0;
        a4.right = 640;
        BltBackToFontFast(0, 0, &a4);
      }
    }*/
  }
}

//----- (0049FD3A) --------------------------------------------------------
void Render::_49FD3A_fullscreen()
{
  RECT src_rect; // [sp+8h] [bp-10h]@6

  if ( this->pRenderD3D )
  {
    if (pFrontBuffer4->IsLost() == DDERR_SURFACELOST)
      pFrontBuffer4->Restore();
    if (pBackBuffer4->IsLost() == DDERR_SURFACELOST)
      pBackBuffer4->Restore();
    src_rect.top = 0;
    src_rect.bottom = window->GetHeight();
    src_rect.left = 0;
    src_rect.right = window->GetWidth();
    this->pBackBuffer4->BltFast(NULL, NULL, this->pFrontBuffer4, &src_rect, DDBLTFAST_WAIT);
  }
}

//----- (0049FDBF) --------------------------------------------------------
void Render::CreateZBuffer()
{
  if (!pDefaultZBuffer)
  {
    pDefaultZBuffer = pActiveZBuffer = (int *)malloc(0x12C000);
    memset32(pActiveZBuffer, 0xFFFF0000, 0x4B000u); //    // inlined Render::ClearActiveZBuffer  (mm8::004A085B)
  }
}

//----- (0049FE05) --------------------------------------------------------
void Render::Release()
{
  //Render *v1; // esi@1
  //RenderD3D *v2; // ecx@1
  //char v3; // zf@4
  //void *v4; // ebx@6
//  IDirectDraw *v5; // eax@10
//  IDirectDrawSurface2 *v6; // eax@11
//  IDirectDrawSurface2 *v7; // eax@13
//  IDirectDrawSurface2 *v8; // eax@15
//  IDirectDraw2 *v9; // eax@17
//  IDirectDraw4 *v10; // eax@19
//  IDirectDrawSurface4 *v11; // eax@20
//  IDirectDrawSurface4 *v12; // eax@22
//  IDirectDrawSurface4 *v13; // eax@24
//  IDirectDraw4 *v14; // eax@26
//  unsigned __int16 **v15; // ebx@28
//  void **v16; // esi@29

 // v1 = this;
  if (pRenderD3D)
  {
    if ( this->using_software_screen_buffer )
    {
      pRenderD3D->ClearTarget(true, 0, false, 1.0);
      pRenderD3D->Present(0);
      pRenderD3D->ClearTarget(true, 0, false, 1.0);
    }
    //this->pColorKeySurface4 = 0;
    this->pBackBuffer4 = nullptr;
    this->pFrontBuffer4 = nullptr;
    this->pDirectDraw4 = nullptr;
    delete [] this->pTargetSurface_unaligned;
    this->pTargetSurface = nullptr;
    this->pTargetSurface_unaligned = nullptr;
    if (pRenderD3D)
    {
      pRenderD3D->Release();
      delete pRenderD3D;
    }
    pRenderD3D = nullptr;
  }
  else
    ;//__debugbreak(); // no sr
  /*{
    if ( bWinNT4_0 == 1 )
    {
      v5 = (IDirectDraw *)this->pDirectDraw2;
      if ( !v5 )
        return;
      v5->SetCooperativeLevel(this->hWnd, 8u);
      this->pDirectDraw2->FlipToGDISurface();
      v6 = this->pSomeSurface2;
      if ( v6 )
      {
        v6->Release();
        this->pSomeSurface2 = 0;
      }
      v7 = this->pBackBuffer2;
      if ( v7 )
      {
        v7->Release();
        this->pBackBuffer2 = 0;
      }
      v8 = this->pFrontBuffer2;
      if ( v8 )
      {
        v8->Release();
        this->pFrontBuffer2 = 0;
      }
      v9 = this->pDirectDraw2;
      if ( v9 )
      {
        v9->Release();
        this->pDirectDraw2 = 0;
      }
    }
    else
    {
      v10 = this->pDirectDraw4;
      if ( !v10 )
        return;
      v10->SetCooperativeLevel(this->hWnd, 1032u);
      this->pDirectDraw4->FlipToGDISurface();
      v11 = this->pColorKeySurface4;
      if ( v11 )
      {
        v11->Release();
        this->pColorKeySurface4 = 0;
      }
      v12 = this->pBackBuffer4;
      if ( v12 )
      {
        v12->Release();
        this->pBackBuffer4 = 0;
      }
      v13 = this->pFrontBuffer4;
      if ( v13 )
      {
        v13->Release();
        this->pFrontBuffer4 = 0;
      }
      v14 = this->pDirectDraw4;
      if ( v14 )
      {
        v14->Release();
        this->pDirectDraw4 = 0;
      }
    }
    v15 = &this->pTargetSurface;
    if ( this->pTargetSurface )
    {
      v16 = (void **)&this->ptr_400E8;
      free(*v16);
      *v15 = 0;
      *v16 = 0;
    }
  }*/
}

void Present32(unsigned __int32 *src, unsigned int src_pitch,
               unsigned __int32 *dst, unsigned int dst_pitch)
{
        for (uint y = 0; y < 8; ++y)
          memcpy(dst + y * dst_pitch,
                 src + y * src_pitch, src_pitch * sizeof(__int32));

        for (uint y = 8; y < 352; ++y)
        {
          memcpy(dst + y * dst_pitch,
                 src + y * src_pitch, 8 * sizeof(__int32));
          memcpy(dst + 8 + game_viewport_width + y * dst_pitch,
                 src + 8 + game_viewport_width + y * src_pitch, 174/*172*/ * sizeof(__int32));
        }

        for (uint y = 352; y < 480; ++y)
          memcpy(dst + y * dst_pitch,
                 src + y * src_pitch, src_pitch * sizeof(__int32));

        for (uint y = pViewport->uViewportTL_Y; y < pViewport->uViewportBR_Y + 1; ++y)
        {
          for (uint x = pViewport->uViewportTL_X; x < pViewport->uViewportBR_X; ++x)
          {
            //if (src[x + y * src_pitch] != (render->uTargetGMask | render->uTargetBMask))
            if (src[x + y * src_pitch] != 0xFF00FCF8)  // FFF8FCF8 =  Color32(Color16(g_mask | b_mask))
              dst[x + y * dst_pitch] = src[x + y * src_pitch];
          }
        }
}

//----- (004A597D) --------------------------------------------------------
void Present_NoColorKey()
{
  void *v2; // edi@4
  int v9; // eax@10
  unsigned int v10; // esi@10
  unsigned __int32 v11; // edi@10
  unsigned int v13; // ebx@10
  DDSURFACEDESC2 Dst; // [sp+Ch] [bp-98h]@3
  int v21; // [sp+8Ch] [bp-18h]@10
  __int32 v22; // [sp+90h] [bp-14h]@10
  unsigned int v24; // [sp+98h] [bp-Ch]@4

  int r_mask = 0xF800;
  int g_mask = 0x7E0;
  int b_mask = 0x1F;

  //if ( !render->uNumSceneBegins )
  {
    //if ( render->using_software_screen_buffer )
    //{
      memset(&Dst, 0, sizeof(Dst));
      Dst.dwSize = sizeof(Dst);
      if ( render->LockSurface_DDraw4(render->pBackBuffer4, &Dst, DDLOCK_WAIT) )
      {
        //v26 = Dst.lpSurface;
        //render->pCurrentlyLockedSurfaceDataPtr = (unsigned __int16 *)Dst.lpSurface;
        v24 = g_mask | b_mask | ((g_mask | b_mask) << 16);
        //render->pCurrentlyLockedSoftSurface = render->pTargetSurface;
        //render->uCurrentlyLockedSurfacePitch = Dst.lPitch;
        //v1 = render->pTargetSurface;
        v2 = Dst.lpSurface;


        /*for (uint y = 0; y < 480; ++y)
        {
          auto pDst = (unsigned short *)((char *)Dst.lpSurface + y * Dst.lPitch);
          for (uint x = 0; x < 640; ++x)
            pDst[x] = render->uTargetRMask | render->uTargetBMask;
        }*/

        if (!FORCE_16_BITS)
          Present32((unsigned __int32 *)render->pTargetSurface, render->uTargetSurfacePitch, (unsigned __int32 *)Dst.lpSurface, Dst.lPitch / 4);
        else
        {        
        ushort* pSrc = (unsigned short *)render->pTargetSurface;
        short* pDst = (__int16 *)Dst.lpSurface;

        for (uint y = 0; y < 8; ++y)
          memcpy(pDst + y * Dst.lPitch / 2,

		  pSrc + y * window->GetWidth(), window->GetWidth() * sizeof(__int16));

        for (uint y = 8; y < 352; ++y)
        {
          memcpy(pDst + y * Dst.lPitch / 2,
                 pSrc + y * window->GetWidth(), 8 * sizeof(__int16));
          memcpy(pDst + 8 + game_viewport_width/*462*/ + y * Dst.lPitch / 2,
                 pSrc + 8 + game_viewport_width/*462*/ + y * window->GetWidth(), 174/*172*/ * sizeof(__int16));
        }

        for (uint y = 352; y < window->GetHeight(); ++y)
          memcpy(pDst + y * Dst.lPitch / 2,
                 pSrc + y * window->GetWidth(), window->GetWidth() * sizeof(__int16));


        ushort* pSrc_x1y1 = pSrc + window->GetWidth() * pViewport->uViewportTL_Y + pViewport->uViewportTL_X;
        //_this = (unsigned int)&pSrc[2 * (((signed int)pViewport->uViewportX >> 1) + 320 * pViewport->uViewportY)];
        short* pDst_x1y1 = pDst + Dst.lPitch * pViewport->uViewportTL_Y + pViewport->uViewportTL_X;
        //v23 = (unsigned __int32)((char *)v26 + 4 * (((signed int)pViewport->uViewportX >> 1) + (Dst.lPitch >> 2) * pViewport->uViewportY));
        v9 = ((signed int)pViewport->uViewportTL_X >> 1) - ((signed int)pViewport->uViewportBR_X >> 1);
        //v20 = ((signed int)pViewport->uViewportZ >> 1) - ((signed int)pViewport->uViewportX >> 1);
        v22 = 4 * ((Dst.lPitch / 4) + v9);
        v21 = 4 * v9 + 1280;

        //auto uNumLines = pViewport->uViewportW - pViewport->uViewportY + 1;
        //v26 = (LPVOID)(pViewport->uViewportW - pViewport->uViewportY + 1);
        v10 = (int)pSrc_x1y1;
        v11 = (int)pDst_x1y1;
        int uHalfWidth = (pViewport->uViewportBR_X - pViewport->uViewportTL_X) / 2;
        v13 = v24;

        for (uint y = pViewport->uViewportTL_Y; y < pViewport->uViewportBR_Y + 1; ++y)
        {
          //memcpy(pDst + pViewport->uViewportX + y * Dst.lPitch / 2,
          //       pSrc + pViewport->uViewportX + y * 640, (pViewport->uViewportZ - pViewport->uViewportX) * sizeof(__int16));
          for (uint x = pViewport->uViewportTL_X; x < pViewport->uViewportBR_X; ++x)
          {
            if (pSrc[y * window->GetWidth() + x] != (g_mask | b_mask))
              pDst[y * Dst.lPitch / 2 + x] = pSrc[y * window->GetWidth() + x];
          }
        }
        }

              ErrD3D(render->pBackBuffer4->Unlock(NULL));

       /* while ( 1 )
        {
          while ( 1 )
          {
            v14 = *(int *)v10;
            v10 += 4;
            if ( v14 == v13 )
              break;
            if ( (short)v14 == (short)v13 )
            {
              *(int *)v11 = *(int *)v11 & 0xFFFF | v14 & 0xFFFF0000;
              v11 += 4;
              --uHalfWidth;
              if ( !uHalfWidth )
                goto LABEL_21;
            }
            else
            {
              v15 = __ROL__(v14, 16);
              if ( (short)v15 == (short)v13 )
              {
                v17 = __ROR__(v15, 16);
                *(int *)v11 = *(int *)v11 & 0xFFFF0000 | (unsigned __int16)v17;
                v11 += 4;
                --uHalfWidth;
                if ( !uHalfWidth )
                  goto LABEL_21;
              }
              else
              {
                v16 = __ROR__(v15, 16);
                *(int *)v11 = v16;
                v11 += 4;
                --uHalfWidth;
                if ( !uHalfWidth )
                  goto LABEL_21;
              }
            }
          }
          v11 += 4;
          --uHalfWidth;
          if ( !uHalfWidth )
          {
LABEL_21:
            v10 += v21;
            v11 += v22;
            uHalfWidth = v20;
            if ( !--uNumLines )
            {
              ErrD3D(render->pBackBuffer4->Unlock(NULL));
              return;
            }
          }
        }*/
      }
    //}
  }
}


//----- (0049FFFB) --------------------------------------------------------
bool Render::InitializeFullscreen()
{
  RenderD3D__DevInfo *v7; // ecx@5
  bool v8; // eax@6
  unsigned int v10; // eax@13
  signed int v15; // ebx@31
  int *v22; // eax@42
  int v23; // ecx@42
  D3DDEVICEDESC refCaps; // [sp+Ch] [bp-300h]@25
  DDSURFACEDESC2 pDesc; // [sp+108h] [bp-204h]@40
  D3DDEVICEDESC halCaps; // [sp+184h] [bp-188h]@25
  int v29; // [sp+308h] [bp-4h]@2

  //__debugbreak(); // Nomad

  this->using_software_screen_buffer = 0;
  //this->pColorKeySurface4 = 0;
  this->pBackBuffer4 = nullptr;
  this->pFrontBuffer4 = nullptr;
  this->pDirectDraw4 = nullptr;
  //this->bColorKeySupported = 0;
  Release();
  //v3 = hWnd;
  this->window = window;
  CreateZBuffer();

  /*if (!bUserDirect3D)
  {
    CreateDirectDraw();
    SetDirectDrawCooperationMode(hWnd, 1);
    SetDirectDrawDisplayMode(640u, 480u, 16u);
    CreateDirectDrawPrimarySurface();
    v15 = 1;
  }
  else
  {*/
    pRenderD3D = new RenderD3D;
    //v28 = pRenderD3D;
    //v6 = uDesiredDirect3DDevice;
    v29 = -1;
    v7 = pRenderD3D->pAvailableDevices;
    if ( pRenderD3D->pAvailableDevices[uDesiredDirect3DDevice].bIsDeviceCompatible )
      v8 = pRenderD3D->CreateDevice(uDesiredDirect3DDevice, /*0*/true, window);
    else
    {
      if ( v7[1].bIsDeviceCompatible )
        v8 = pRenderD3D->CreateDevice(1, /*0*/true, window);
      else
      {
        if ( !v7->bIsDeviceCompatible )
          Error("There aren't any D3D devices to create.");

        v8 = pRenderD3D->CreateDevice(0, /*0*/true, window);
      }
    }
    if ( !v8 )
      Error("D3Drend->Init failed.");

    //v9 = pRenderD3D;
    pBackBuffer4 = pRenderD3D->pBackBuffer;
    pFrontBuffer4 = pRenderD3D->pFrontBuffer;
    pDirectDraw4 = pRenderD3D->pHost;
    v10 = pRenderD3D->GetDeviceCaps();
    if ( v10 & 1 )
    {
      if ( pRenderD3D )
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device failed to return capabilities.");
    }
    if ( v10 & 0x3E )
    {
      if ( pRenderD3D )
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      //pColorKeySurface4 = 0;
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device doesn't support the necessary alpha blending modes.");
    }
    if ( (v10 & 0x80) != 0 )
    {
      if ( pRenderD3D )
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device doesn't support non-square textures.");
    }
    //LOBYTE(field_10365C) = ~(unsigned __int8)(v10 >> 6) & 1;
    bRequiredTextureStagesAvailable = CheckTextureStages();

    memset(&halCaps, 0, sizeof(halCaps));
    halCaps.dwSize = sizeof(halCaps);

    memset(&refCaps, 0, sizeof(refCaps));
    refCaps.dwSize = sizeof(refCaps);

    ErrD3D(pRenderD3D->pDevice->GetCaps(&halCaps, &refCaps));

    uMinDeviceTextureDim = halCaps.dwMinTextureWidth;
    if ( (unsigned int)halCaps.dwMinTextureWidth >= halCaps.dwMinTextureHeight )
      uMinDeviceTextureDim = halCaps.dwMinTextureHeight;
    uMinDeviceTextureDim = halCaps.dwMaxTextureWidth;
    if ( (unsigned int)halCaps.dwMaxTextureWidth < halCaps.dwMaxTextureHeight )
      uMinDeviceTextureDim = halCaps.dwMaxTextureHeight;
    if ( (unsigned int)uMinDeviceTextureDim < 4 )
      uMinDeviceTextureDim = 4;
    v15 = 1;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, true));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, true));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, 2));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, false));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, false));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, 1));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 3));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4));
  //}
  ddpfPrimarySuface.dwSize = 32;
  GetTargetPixelFormat(&ddpfPrimarySuface);
  ParseTargetPixelFormat();

  if (!pRenderD3D)
  {
    __debugbreak();
    pBeforePresentFunction = 0;//nullsub_1;
  }
  //else
  //{
    /*v16 = IsColorKeySupported(pDirectDraw4);
    v17 = uAcquiredDirect3DDevice == v15;
    bColorKeySupported = v16;
    if ( !v17 )
      bColorKeySupported = 0;
    if ( bColorKeySupported )
    {
      memset(&ddsd2, 0, sizeof(ddsd2));
      ddsd2.dwSize = sizeof(ddsd2);
      ddsd2.ddckCKSrcBlt.dwColorSpaceLowValue = uTargetGMask | uTargetBMask;
      ddsd2.ddckCKSrcBlt.dwColorSpaceHighValue = ddsd2.ddckCKSrcBlt.dwColorSpaceLowValue;
      ddsd2.dwFlags = 65543;
      ddsd2.ddsCaps.dwCaps = 2112;
      ddsd2.dwWidth = 640;
      ddsd2.dwHeight = 480;
      ErrD3D(pDirectDraw4->CreateSurface(&ddsd2, &pColorKeySurface4, NULL));
      pBeforePresentFunction = Present_ColorKey;
    }
    else*/
    {
      pTargetSurface = nullptr;
      pTargetSurface_unaligned = (unsigned int *)malloc(window->GetWidth() * window->GetHeight() * 2 + 32);
      if ( !pTargetSurface_unaligned )
		  return 0;
      memset(&pDesc, 0, sizeof(pDesc));
      pDesc.dwSize = sizeof(pDesc);
      if ( !render->LockSurface_DDraw4(render->pBackBuffer4, &pDesc, v15) )
        return 0;
      pBackBuffer4->Unlock(NULL);
      v22 = (int *)pTargetSurface_unaligned + 4;
      v23 = (unsigned int)pDesc.lpSurface & 7;
      LOBYTE(v22) = (unsigned __int8)v22 & 0xF8;
      uTargetSurfacePitch = window->GetWidth();
      pBeforePresentFunction = Present_NoColorKey;
      v15 = 1;
      pTargetSurface = (unsigned __int32 *)((char *)v22 + 2 * v23);
    }
    using_software_screen_buffer = v15;
  //}
  bWindowMode = 0;
  pParty->uFlags |= 2;
  pViewport->SetFOV(flt_6BE3A0 * 65536.0f);
  return v15 != 0;
}

//----- (004A05F3) --------------------------------------------------------
bool Render::SwitchToWindow()
{
  bool v7; // eax@7
  unsigned int v9; // eax@12
  int v12; // eax@24
  int v13; // eax@26
  D3DDEVICEDESC refCaps; // [sp+Ch] [bp-300h]@24
  DDSURFACEDESC2 pDesc; // [sp+108h] [bp-204h]@37
  D3DDEVICEDESC halCaps; // [sp+184h] [bp-188h]@24
  int v29; // [sp+308h] [bp-4h]@2

  pParty->uFlags |= PARTY_FLAGS_1_0002;
  pViewport->SetFOV(flt_6BE3A0 * 65536.0f);
  using_software_screen_buffer = 0;
  Release();
  //pColorKeySurface4 = 0;
  pBackBuffer4 = nullptr;
  pFrontBuffer4 = nullptr;
  pDirectDraw4 = nullptr;
  //bColorKeySupported = 0;
  CreateZBuffer();
  /*if (!bUserDirect3D)
  {
    CreateDirectDraw();
    SetDirectDrawCooperationMode(hWnd, 0);
    field_4004C = 1;
    CreateFrontBuffer();
    CreateClipper(hWnd);
    CreateBackBuffer();
    field_40030 = 0;
    field_18_locked_pitch = 0;
  }
  else
  {*/
    /*v3 = malloc(0x148u);
    thisa = (RenderD3D *)v3;
    v29 = 0;
    if ( v3 )
      v4 = RenderD3D::RenderD3D((RenderD3D *)v3);
    else
      v4 = 0;*/
    pRenderD3D = new RenderD3D;
    //v4 = pRenderD3D;
    //v5 = uDesiredDirect3DDevice;
    v29 = -1;
    //v6 = pRenderD3D->pAvailableDevices;
    if (pRenderD3D->pAvailableDevices[uDesiredDirect3DDevice].bIsDeviceCompatible &&
        uDesiredDirect3DDevice != 1 )
    {
      v7 = pRenderD3D->CreateDevice(uDesiredDirect3DDevice, true, window);
    }
    else
    {
      if ( !pRenderD3D->pAvailableDevices[0].bIsDeviceCompatible )
        Error("There aren't any D3D devices to init.");

      v7 = pRenderD3D->CreateDevice(0, true, window);
    }
    if ( !v7 )
      Error("D3Drend->Init failed.");

    //v8 = pRenderD3D;
    //pColorKeySurface4 = 0;
    pBackBuffer4 = pRenderD3D->pBackBuffer;
    pFrontBuffer4 = pRenderD3D->pFrontBuffer;
    pDirectDraw4 = pRenderD3D->pHost;
    v9 = pRenderD3D->GetDeviceCaps();
    if ( v9 & 1 )
    {
      if (pRenderD3D)
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device failed to return capabilities.");
    }
    if ( v9 & 0x3E )
    {
      if (pRenderD3D)
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      //pColorKeySurface4 = 0;
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device doesn't support the necessary alpha blending modes.");
    }
    if (v9 & 0x80)
    {
      if (pRenderD3D)
      {
        pRenderD3D->Release();
        delete pRenderD3D;
      }
      pRenderD3D = nullptr;
      pBackBuffer4 = nullptr;
      pFrontBuffer4 = nullptr;
      pDirectDraw4 = nullptr;
      Error("Direct3D renderer:  The device doesn't support non-square textures.");
    }
    //LOBYTE(field_10365C) = ~(unsigned __int8)(v9 >> 6) & 1;
    bRequiredTextureStagesAvailable = CheckTextureStages();

    memset(&halCaps, 0, sizeof(halCaps));
    halCaps.dwSize = sizeof(halCaps);

    memset(&refCaps, 0, sizeof(refCaps));
    refCaps.dwSize = sizeof(refCaps);

    ErrD3D(pRenderD3D->pDevice->GetCaps(&halCaps, &refCaps));
    v12 = halCaps.dwMinTextureWidth;
    if ( (unsigned int)halCaps.dwMinTextureWidth > halCaps.dwMinTextureHeight )
      v12 = halCaps.dwMinTextureHeight;
    uMinDeviceTextureDim = v12;
    v13 = halCaps.dwMaxTextureWidth;
    if ( (unsigned int)halCaps.dwMaxTextureWidth < halCaps.dwMaxTextureHeight )
      v13 = halCaps.dwMaxTextureHeight;
    uMaxDeviceTextureDim = v13;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 1));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 1));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, 2));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, 0));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 0));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, 1));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 3));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4));
  //}

  ddpfPrimarySuface.dwSize = 32;
  GetTargetPixelFormat(&ddpfPrimarySuface);
  ParseTargetPixelFormat();

  if ( !pRenderD3D )
  {
    __debugbreak();
    //pBeforePresentFunction = 0;//nullsub_1;
    //goto LABEL_47;
  }
  /*v14 = IsColorKeySupported(pDirectDraw4);
  v15 = uAcquiredDirect3DDevice == 1;
  bColorKeySupported = v14;
  if ( !v15 )
    bColorKeySupported = 0;*/
  //if ( bColorKeySupported )
  if (false)
  {
    /*memset(&ddsd2, 0, 0x7Cu);
    ddsd2.ddckCKSrcBlt.dwColorSpaceLowValue = uTargetGMask | uTargetBMask;
    ddsd2.ddckCKSrcBlt.dwColorSpaceHighValue = ddsd2.ddckCKSrcBlt.dwColorSpaceLowValue;
    v16 = pDirectDraw4;
    ddsd2.dwSize = 124;
    ddsd2.dwFlags = 65543;
    ddsd2.ddsCaps.dwCaps = 2112;
    ddsd2.dwWidth = 640;
    ddsd2.dwHeight = 480;
    ErrD3D(v16->CreateSurface(&ddsd2, &pColorKeySurface4, NULL));
    pBeforePresentFunction = Present_ColorKey;*/
    using_software_screen_buffer = 1;
//LABEL_47:
    bWindowMode = 1;
    //hWnd = hWnd;
    return 0;
  }
  pTargetSurface = 0;
  pTargetSurface_unaligned = 0;

  uint num_pixels = window->GetWidth() * window->GetHeight();
  pTargetSurface_unaligned = new unsigned int[num_pixels];

  if (!pTargetSurface_unaligned)
    return false;

  memset(&pDesc, 0, sizeof(pDesc));
  pDesc.dwSize = sizeof(pDesc);
  if (!render->LockSurface_DDraw4(render->pBackBuffer4, &pDesc, DDLOCK_WAIT))
  {
    delete [] pTargetSurface_unaligned;
    return false;
  }

  memset32(pTargetSurface_unaligned, -1, num_pixels);

  render->pBackBuffer4->Unlock(NULL);
  /*v19 = pTargetSurface_unaligned;
  v20 = (unsigned int)pDesc.lpSurface & 7;
  v21 = (unsigned int)ptr_400E8 & 7;
  if ( v21 == v20 )
    pTargetSurface = (unsigned __int16 *)v19;
  else
  {
    if ( (signed int)v21 >= v20 )
      v22 = (int)((char *)v19 + 2 * (v21 - v20) + 16);
    else
      v22 = (int)((char *)v19 + 2 * (v20 - v21) + 16);
    pTargetSurface = (unsigned __int16 *)v22;
  }*/
  pTargetSurface = pTargetSurface_unaligned;
  uTargetSurfacePitch = window->GetWidth();
  pBeforePresentFunction = Present_NoColorKey;
  using_software_screen_buffer = 1;
  bWindowMode = 1;
  return 0;
}


//----- (0044F2B2) --------------------------------------------------------
bool Render::IsGammaSupported()
{
//  bool result; // eax@3
//  HRESULT v1; // eax@4

  //if ( pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT || pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
    DDCAPS halCaps; // [sp+0h] [bp-180h]@4
    memset(&halCaps, 0, sizeof(DDCAPS));
    halCaps.dwSize = sizeof(DDCAPS);

    ErrD3D(pDirectDraw4->GetCaps(&halCaps, 0));
    return (halCaps.dwCaps2 >> 17) & 1;
  }
  /*else
    return false;*/
}

//----- (004A0BEE) --------------------------------------------------------
void Render::RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, unsigned __int16 uColor)
{
  signed int lower_bound; // eax@17
//  signed int left_bound;
  unsigned int v21; // edi@46
  int v22; // esi@47
  int v23; // ebx@47
  signed int v24; // edx@50
  signed int v25; // esi@52
  unsigned __int16 *v26; // ecx@52
  int v27; // ebx@54
  int v28; // edi@55
  int v29; // edx@55
  int v30; // ebx@60
  int v31; // edx@61
  int v32; // edi@61
  signed int upper_bound; // [sp+18h] [bp-4h]@28
  unsigned int uXa; // [sp+24h] [bp+8h]@49
  int uYb; // [sp+28h] [bp+Ch]@47
  bool left_border_x = false;
  bool right_border_x = false;
  bool left_border_z = false;
  bool right_border_z = false;
  bool upper_border_y = false;
  bool bottom_border_y = false;
  bool upper_border_w = false;
  bool bottom_border_w = false;

  if ( uX < this->raster_clip_x )// x выходит за рамки левой границы
    left_border_x = true;
  if ( uX > this->raster_clip_z )// x выходит за рамки правой границы
    right_border_x = true;

  if ( uZ < this->raster_clip_x )// z выходит за рамки левой границы
    left_border_z = true;
  if ( uZ > this->raster_clip_z )// z выходит за рамки правой границы
    right_border_z = true;

  if ( uY < this->raster_clip_y )// y выходит за рамки верхней границы
    upper_border_y = true;
  if ( uY > this->raster_clip_w )// y выходит за рамки нижней границы
    bottom_border_y = true;

  if ( uW < this->raster_clip_y )// w выходит за рамки верхней границы
    upper_border_w = true;
  if ( uW > this->raster_clip_w )// w выходит за рамки нижней границы
    bottom_border_w = true;

  if ( (left_border_x && left_border_z) || (right_border_x && right_border_z )
    || (upper_border_y && upper_border_w) || (bottom_border_y && bottom_border_w))
    return;

  if ( left_border_x || left_border_z || right_border_x || right_border_z
    || upper_border_y || upper_border_w || bottom_border_y || bottom_border_w)
  {
    if ( left_border_x || left_border_z )//if ( (BYTE4(v36) ^ (unsigned __int8)v36) & 8 )//for left (левая граница)
    {
      if ( left_border_x )//left_border = true; х меньше левой границы
      {
        uY += (uW - uY) * ((this->raster_clip_x - uX) / (uZ - uX));//t = near_clip - v0.x / v1.x - v0.x  (формула получения точки пересечения отрезка с плоскостью)
        uX = this->raster_clip_x;
      }
      else if ( left_border_z )//z меньше левой границы
      {
        uZ = this->raster_clip_x;
        uW += (uY - uW) * ((this->raster_clip_x - uZ) / (uX - uZ));
      }
    }

    if ( right_border_x || right_border_z )//if ( (BYTE4(v36) ^ (unsigned __int8)v36) & 4 )//for right (правая граница)
    {
      if ( right_border_x ) //right_border = true; х больше правой границы
      {
        uY += (uY - uW) * ((this->raster_clip_z - uX) / (uZ - uX));
        uX = this->raster_clip_z;
      }
      else if ( right_border_z )//z больше правой границы
      {
        uW += (uW - uY) * ((this->raster_clip_z - uZ) / (uX - uZ));
        uZ = this->raster_clip_z;
      }
    }

    upper_bound = 0;
    if ( uY < this->raster_clip_y )
      upper_bound = 2;
    if ( uY > this->raster_clip_w )
      upper_bound |= 1;

    lower_bound = 0;
    if ( uW < this->raster_clip_y )
      lower_bound = 2;
    if ( uW > this->raster_clip_w )
      lower_bound |= 1;

    if ( !(lower_bound & upper_bound) )//for up and down(для верха и низа)
    {
      lower_bound ^= upper_bound;
      if ( lower_bound & 2 )
      {
        if ( upper_bound & 2 )
        {
          uX += (uZ - uX) * ((this->raster_clip_y - uY) / (uW - uY));
          uY = this->raster_clip_y;
        }
        else
        {
          uZ += (uX - uZ) * ((this->raster_clip_y - uW) / (uY - uW));
          uW = this->raster_clip_y;
        }
      }
      if ( lower_bound & 1 )
      {
        if ( upper_bound & 1 )
        {
          uX += (uZ - uX) * ((this->raster_clip_w - uY) / (uW - uY));
          uY = this->raster_clip_w;
        }
        else
        {
          uZ += (uX - uZ) * ((this->raster_clip_w - uW) / (uY - uW));
          uW = this->raster_clip_w;
        }
      }
    }
  }
  v21 = render->uTargetSurfacePitch;
  if ( render->uTargetSurfacePitch )
  {
    //v12 = uX + uY * render->uTargetSurfacePitch;
    v22 = uW - uY;
    v23 = v22;
    uYb = v22;
    if ( v22 < 0 )
    {
      v23 = -v22;
      uYb = -v22;
      v21 = -render->uTargetSurfacePitch;
    }
    uXa = uZ - uX;
    if ((signed)(uZ - uX) >= 0)
      v24 = 1;
    else
    {
      uXa = -uXa;
      v24 = -1;
    }
    v25 = 0;

    v26 = (unsigned __int16 *)this->pTargetSurface;
    if ( v26 )
    {
      if ( (signed int)uXa <= v23 )//рисуем вертикальную линию
      {
        v30 = v23 + 1;
        if ( v30 > 0 )
        {
          v31 = 2 * v24;
          v32 = 2 * v21;
          //v12 = (int)&v26[v12];
          int y = 0;
          int x = 0;
          for ( v30; v30; --v30 )
          {
            v25 += uXa;
            //*(short *)v12 = uColor;
            //v12 += v32;
            WritePixel16(uX + x, uY + y, uColor);
            if ( v32 >= 0 )
              y += 1;
            else
              y -= 1;
            if ( v25 > 0 )
            {
              v25 -= uYb;
              //v12 += v31;
              if ( v31 >= 0 )
                x += 1;
              else
                x -= 1;
            }
          }
        }
      }
      else//рисуем горизонтальную линию
      {
        v27 = uXa + 1;
        if ( (signed int)(uXa + 1) > 0 )
        {
          v28 = 2 * v21;
          v29 = 2 * v24;
          int y = 0;
          int x = 0;
          //v12 = (int)&v26[v12];
          for ( v27; v27; --v27 )
          {
            v25 += uYb;
            //*(short *)v12 = uColor;
            //v12 += v29;
            WritePixel16(uX + x, uY + y, uColor);
            if ( v29 >= 0 )
              x += 1;
            else
              x -= 1;
            if ( v25 > (signed int)uXa )
            {
              v25 -= uXa;
              //v12 += v28;
              if ( v28 >= 0 )
                y += 1;
              else
                y -= 1;
            }
          }
        }
      }
    }
  }
  return;
}

//----- (004A0E80) --------------------------------------------------------
void Render::ClearZBuffer(int a2, int a3)
{
  memset32(this->pActiveZBuffer, -65536, 0x4B000);
}

//----- (004A0E97) --------------------------------------------------------
void Render::SetRasterClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW)
{
  this->raster_clip_x = uX;
  this->raster_clip_y = uY;
  this->raster_clip_z = uZ;
  this->raster_clip_w = uW;
}

//----- (004A0EB6) --------------------------------------------------------
void Render::ParseTargetPixelFormat()
{
  signed int v2; // ecx@1
  DWORD uRedMask; // edx@1
  unsigned int uGreenMask; // esi@5
  signed int v5; // ecx@5
  unsigned int uBlueMask; // edx@9
  signed int v7; // ecx@9
  //unsigned int v8; // ecx@13

  v2 = 0;
  uRedMask = this->ddpfPrimarySuface.dwRBitMask;
  this->uTargetBBits = 0;
  this->uTargetGBits = 0;
  this->uTargetRBits = 0;
  do
  {
    if ( (1 << v2) & uRedMask )
      ++this->uTargetRBits;
    ++v2;
  }
  while ( v2 < 32 );
  uGreenMask = this->ddpfPrimarySuface.dwGBitMask;
  v5 = 0;
  do
  {
    if ( (1 << v5) & uGreenMask )
      ++this->uTargetGBits;
    ++v5;
  }
  while ( v5 < 32 );
  uBlueMask = this->ddpfPrimarySuface.dwBBitMask;
  v7 = 0;
  do
  {
    if ( (1 << v7) & uBlueMask )
      ++this->uTargetBBits;
    ++v7;
  }
  while ( v7 < 32 );
  this->uTargetGMask = uGreenMask;
  this->uTargetRMask = this->ddpfPrimarySuface.dwRBitMask;
  this->uTargetBMask = uBlueMask;
}

//----- (004A0F40) --------------------------------------------------------
bool Render::LockSurface_DDraw4(IDirectDrawSurface4 *pSurface, DDSURFACEDESC2 *pDesc, unsigned int uLockFlags)
{
  HRESULT result; // eax@1
  HRESULT v6; // eax@4
  char v9; // [sp+Bh] [bp-1h]@1

  v9 = 1;
  result = pSurface->Lock(NULL, pDesc, uLockFlags, NULL);
  /*
  Когда объект DirectDrawSurface теряет поверхностную память, методы возвратят DDERR_SURFACELOST
  и не выполнят никакую другую функцию. Метод IDirectDrawSurface::Restore перераспределит поверхностную память
  и повторно присоединит ее к объекту DirectDrawSurface. 
  */
  if ( result == DDERR_SURFACELOST )
  {
    v6 = pSurface->Restore();//Восстанавливает потерянную поверхность. Это происходит, когда поверхностная память,
	                         //связанная с объектом DirectDrawSurface была освобождена. 
    if ( v6 )
    {
      if ( v6 != DDERR_IMPLICITLYCREATED )//DDERR_IMPLICITLYCREATED - Поверхность не может быть восстановлена,   
		                                  //потому что она - неявно созданная поверхность.										
      {
        v9 = 0;
        result = (bool)memset(pDesc, 0, 4);
        LOBYTE(result) = v9;
        return 0;
      }
      render->pFrontBuffer4->Restore();
      pSurface->Restore();
    }
    result = pSurface->Lock(NULL, pDesc, DDLOCK_WAIT, NULL);
    if ( result == DDERR_INVALIDRECT || result == DDERR_SURFACEBUSY )//DDERR_SURFACEBUSY - Доступ к этой поверхности отказан, 
		//потому что поверхность блокирована другой нитью. DDERR_INVALIDRECT - Обеспечиваемый прямоугольник недопустим.
	{
      v9 = 0;
      result = (bool)memset(pDesc, 0, 4);
      LOBYTE(result) = v9;
      return result;
	}
    ErrD3D(result);
    if ( result )
    {
      //v8 = 0;
      //v7 = 2161;
//LABEL_19:
      //CheckHRESULT((CheckHRESULT_stru0 *)&pSurface, result, "E:\\WORK\\MSDEV\\MM7\\MM7\\Code\\Screen16.cpp", v7, v8);
      v9 = 0;
      result = (bool)memset(pDesc, 0, 4);
      LOBYTE(result) = v9;
      return result;
    }
    if ( pRenderD3D )
      pRenderD3D->HandleLostResources();
    result = render->pDirectDraw4->RestoreAllSurfaces();
  }
  else
  {
    if ( result )
    {
      if ( result == DDERR_INVALIDRECT || result == DDERR_SURFACEBUSY )
	  {
        v9 = 0;
        result = (bool)memset(pDesc, 0, 4);
        LOBYTE(result) = v9;
        return result;
	  }
      ErrD3D(result);
      //v8 = 0;
      //v7 = 2199;
      //goto LABEL_19;
    }
  }
  return true;
}


//----- (004A10E4) --------------------------------------------------------
void Render::CreateDirectDraw()
{
  //Render *v1; // edi@1
//  HRESULT v2; // eax@1
//  HRESULT v3; // eax@5
//  int v6; // [sp-Ch] [bp-20h]@3
//  unsigned int v9; // [sp+0h] [bp-14h]@0
  IDirectDraw *lpDD; // [sp+10h] [bp-4h]@1

  //v1 = this;
  ErrD3D(DirectDrawCreate(0, &lpDD, 0));

  pDirectDraw4 = nullptr;

  ErrD3D(lpDD->QueryInterface(IID_IDirectDraw4, (void **)&pDirectDraw4));

  lpDD->Release();
  lpDD = nullptr;
}

//----- (004A1169) --------------------------------------------------------
void Render::SetDirectDrawCooperationMode(HWND hWnd, bool bFullscreen)
{
  DWORD flags; // eax@1

  //Установка взаимодействия для полноэкранного и оконного режимов
  flags = bFullscreen ? DDSCL_NORMAL | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN : DDSCL_NORMAL;

  ErrD3D(pDirectDraw4->SetCooperativeLevel(hWnd, flags | DDSCL_MULTITHREADED));
}

//----- (004A11C6) --------------------------------------------------------
void Render::SetDirectDrawDisplayMode(unsigned int uWidth, unsigned int uHeight, unsigned int uBPP)
{
  ErrD3D(pDirectDraw4->SetDisplayMode(uWidth, uHeight, uBPP, 0, 0));
}

//----- (004A121C) --------------------------------------------------------
void Render::CreateFrontBuffer()
{
  //Render *v1; // esi@1
  IDirectDraw *pDD; // eax@3
  IDirectDrawSurface **pOutSurf; // esi@3
  struct _DDSURFACEDESC *v4; // edx@3
////  HRESULT v5; // eax@5
  int v6; // [sp-8h] [bp-8Ch]@3
  DDSURFACEDESC2 a2; // [sp+4h] [bp-80h]@3

  //v1 = this;
  //if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
      //pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
    memset(&a2, 0, sizeof(a2));
    a2.dwSize = sizeof(a2);

    pDD = (IDirectDraw *)this->pDirectDraw4;
    a2.dwFlags = 1;
    a2.ddsCaps.dwCaps = 512;//DDSCAPS_PRIMARYSURFACE = 0x200

    v6 = 2357;
    pOutSurf = (IDirectDrawSurface **)&this->pFrontBuffer4;
    v4 = (struct _DDSURFACEDESC *)&a2;
  }
  /*else
  {
    memset(&a2.lPitch, 0, 0x6Cu);               // DDSURFACEDESC here
    pDD = (IDirectDraw *)v1->pDirectDraw2;
    a2.lPitch = 108;
    a2.dwBackBufferCount = 1;
    a2.dwTextureStage = 512;
    v6 = 2346;
    pOutSurf = (IDirectDrawSurface **)&v1->pFrontBuffer2;
    v4 = (struct _DDSURFACEDESC *)&a2.lPitch;
  }*/
  ErrD3D(pDD->CreateSurface(v4, pOutSurf, NULL));
}

//----- (004A12CD) --------------------------------------------------------
void Render::CreateBackBuffer()
{
  //Render *v1; // esi@1
  IDirectDraw *v2; // eax@3
  IDirectDrawSurface **ppBackBuffer; // esi@3
  struct _DDSURFACEDESC *v4; // edx@3
//  HRESULT v5; // eax@5
  int v6; // [sp-8h] [bp-8Ch]@3
  unsigned int v7; // [sp-4h] [bp-88h]@3
  DDSURFACEDESC2 a2; // [sp+4h] [bp-80h]@3

  //v1 = this;
  //if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
    //  pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
    memset(&a2, 0, sizeof(a2));
    a2.dwSize = sizeof(a2);

    v2 = (IDirectDraw *)this->pDirectDraw4;
    a2.dwFlags = 7;
    a2.ddsCaps.dwCaps = 2112;//0x840 = DDSCAPS_SYSTEMMEMORY	= 0x800 | DDSCAPS_OFFSCREENPLAIN = 0x40
    a2.dwWidth = window->GetWidth();
    a2.dwHeight = window->GetHeight();

    v7 = 0;
    v6 = 2387;
    ppBackBuffer = (IDirectDrawSurface **)&this->pBackBuffer4;
    v4 = (struct _DDSURFACEDESC *)&a2;
  }
  /*else
  {
    memset(&a2.lPitch, 0, 0x6Cu);
    v2 = (IDirectDraw *)v1->pDirectDraw2;
    a2.lPitch = 108;
    a2.dwBackBufferCount = 7;
    v7 = 0;
    a2.dwTextureStage = 2112;
    a2.dwAlphaBitDepth = 640;
    a2.dwMipMapCount = 480;
    v6 = 2374;
    ppBackBuffer = (IDirectDrawSurface **)&v1->pBackBuffer2;
    v4 = (struct _DDSURFACEDESC *)&a2.lPitch;   // //DDSURFACEDESC here fo ddraw2
  }*/
  ErrD3D(v2->CreateSurface(v4, ppBackBuffer, NULL));
}

//----- (004A139A) --------------------------------------------------------
void Render::CreateDirectDrawPrimarySurface()
{
  IDirectDrawSurface *pFrontBuffer; // eax@3
  DDSCAPS2 *v6; // edx@3
  IDirectDraw4 *v7; // eax@4
  int v9; // ST14_4@5
  IDirectDrawSurface *v10; // ST10_4@5
  IDirectDrawSurface **ppBackBuffer; // [sp-4h] [bp-A4h]@3
  DDSURFACEDESC2 ddsd2; // [sp+Ch] [bp-94h]@3
  DDSCAPS2 v17; // [sp+88h] [bp-18h]@4

  //v1 = this;
  //if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
      //pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
    //v2 = 0;
    //this->field_4004C = 1;
    memset(&ddsd2, 0, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);

    ddsd2.dwBackBufferCount = 1;
    ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE;
    ErrD3D(this->pDirectDraw4->CreateSurface(&ddsd2, &pFrontBuffer4, NULL));
    pFrontBuffer = (IDirectDrawSurface *)this->pFrontBuffer4;
    ppBackBuffer = (IDirectDrawSurface **)&this->pBackBuffer4;
  }
  /*else
  {
    v2 = 0;
    this->field_4004C = 1;

    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(DDSURFACEDESC));

    ddsd.lpSurface = (LPVOID)1;
    ddsd.lPitch = 108;
    ddsd.dwBackBufferCount = 33;
    ddsd.ddsCaps.dwCaps = 8728;
    ErrD3D(pDirectDraw2->CreateSurface(&ddsd, (IDirectDrawSurface **)&pFrontBuffer2, NULL));

    pFrontBuffer = (IDirectDrawSurface *)v1->pFrontBuffer2;
    ppBackBuffer = (IDirectDrawSurface **)&v1->pBackBuffer2;
  }*/
  __debugbreak(); // warning C4700: uninitialized local variable 'v6' used
  v9 = (int)v6;
  v10 = pFrontBuffer;                           // BUG

    v17.dwCaps = 4;
  ErrD3D(pFrontBuffer->GetAttachedSurface((DDSCAPS *)&v17, ppBackBuffer));//  hr = this->pFrontBuffer->GetAttachedSurface(&ddsCaps2, ppBackBuffer);
  //CheckHRESULT(&thisa, v11, (const char *)v10, v9, (unsigned int)ppBackBuffer);
  //v1->field_40030 = v2;
  //v1->field_18_locked_pitch = v2;
}

//----- (004A14F4) --------------------------------------------------------
void Render::CreateClipper(HWND a2)
{
  ErrD3D(pDirectDraw4->CreateClipper(0, &pDDrawClipper, NULL));
  ErrD3D(pDDrawClipper->SetHWnd(0, a2));
  ErrD3D(pFrontBuffer4->SetClipper(pDDrawClipper));
}

//----- (004A15D8) --------------------------------------------------------
void Render::GetTargetPixelFormat(DDPIXELFORMAT *pOut)
{
  pFrontBuffer4->GetPixelFormat(pOut);
}

//----- (004A1605) --------------------------------------------------------
void Render::LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow)
{
  signed int v4; // eax@3

  //if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
      //pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
  DDSURFACEDESC2 pDesc; // [sp+4h] [bp-7Ch]@3

    memset(&pDesc, 0, sizeof(pDesc));
    pDesc.dwSize = sizeof(pDesc);

    LockSurface_DDraw4(this->pBackBuffer4, &pDesc, DDLOCK_WAIT);
    *pOutSurfacePtr = pDesc.lpSurface;
    v4 = pDesc.lPitch;
  }
  /*else
  {
  DDSURFACEDESC pDesc; // [sp+4h] [bp-7Ch]@3
    memset(&pDesc.lPitch, 0, 0x6Cu);
    pDesc.lPitch = 108;
    LockSurface_DDraw2(this->pBackBuffer2, &pDesc, 1);
    *pOutSurfacePtr = (void *)pDesc.lpSurface;
    v4 = pDesc.dwReserved;
  }*/
  *pOutPixelsPerRow = v4 >> 1;
}

//----- (004A16E1) --------------------------------------------------------
void Render::UnlockBackBuffer()
{
  ErrD3D(pBackBuffer4->Unlock(NULL));
}

//----- (004A172E) --------------------------------------------------------
void Render::LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow)
{
  signed int v4; // eax@3

  //if ( pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT || pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
  DDSURFACEDESC2 pDesc; // [sp+4h] [bp-7Ch]@3

    memset(&pDesc, 0, sizeof(pDesc));
    pDesc.dwSize = sizeof(pDesc);

    LockSurface_DDraw4(this->pFrontBuffer4, &pDesc, DDLOCK_WAIT);
    *pOutSurface = pDesc.lpSurface;
    v4 = pDesc.lPitch;
  }
  /*else
  {
  DDSURFACEDESC pDesc; // [sp+4h] [bp-7Ch]@3
    memset(&pDesc.lPitch, 0, 0x6Cu);
    pDesc.lPitch = 108;
    LockSurface_DDraw2(this->pFrontBuffer2, &pDesc, 1);
    *pOutSurface = (void *)pDesc.lpSurface;
    v4 = pDesc.dwReserved;
  }*/
  *pOutPixelsPerRow = v4 >> 1;
}

//----- (004A17C7) --------------------------------------------------------
void Render::UnlockFrontBuffer()
{
  ErrD3D(pFrontBuffer4->Unlock(NULL));
}

//----- (004A1814) --------------------------------------------------------
void Render::RestoreFrontBuffer()
{
  if (pFrontBuffer4->IsLost() == DDERR_SURFACELOST )
    pFrontBuffer4->Restore();
}

//----- (004A184C) --------------------------------------------------------
void Render::RestoreBackBuffer()
{
  if ( pBackBuffer4->IsLost() == DDERR_SURFACELOST )
    pBackBuffer4->Restore();
}

//----- (004A18F5) --------------------------------------------------------
void Render::BltToFront(RECT *pDstRect, IDirectDrawSurface *pSrcSurface, RECT *pSrcRect, unsigned int uBltFlags)
{
  ErrD3D(pFrontBuffer4->Blt(pDstRect, (IDirectDrawSurface4 *)pSrcSurface, pSrcRect, uBltFlags, nullptr));
}

//----- (004A194A) --------------------------------------------------------
void Render::BltBackToFontFast(int a2, int a3, RECT *pSrcRect)
{
  IDirectDrawSurface *pFront; // eax@3
  IDirectDrawSurface *pBack; // [sp-Ch] [bp-Ch]@3

  //if ( pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT || pVersion->pVersionInfo.dwMajorVersion != 4 )
  {
    pFront = (IDirectDrawSurface *)this->pFrontBuffer4;
    pBack = (IDirectDrawSurface *)this->pBackBuffer4;
  }
  /*else
  {
    pFront = (IDirectDrawSurface *)this->pFrontBuffer2;
    pBack = (IDirectDrawSurface *)this->pBackBuffer2;
  }*/
  pFront->BltFast(NULL, NULL, pBack, pSrcRect, DDBLTFAST_WAIT);
}

//----- (004A1B22) --------------------------------------------------------
unsigned int Render::Billboard_ProbablyAddToListAndSortByZOrder(float z)
{
    unsigned int v7; // edx@6

    if (uNumBillboardsToDraw >= 999)
        return 0;
    if (!uNumBillboardsToDraw)
    {
        uNumBillboardsToDraw = 1;
        return 0;
    }

    for (int left = 0, right = uNumBillboardsToDraw; left < right; ) // binsearch
    {
        v7 = left + (right - left) / 2;
        if (z <= render->pBillboardRenderListD3D[v7].z_order)
            right = v7;
        else
            left = v7 + 1;
    }

    if (z > render->pBillboardRenderListD3D[v7].z_order)
    {
        if (v7 == render->uNumBillboardsToDraw - 1)
            v7 = render->uNumBillboardsToDraw;
        else
        {
            if ((signed int)render->uNumBillboardsToDraw > (signed int)v7)
            {
                for (uint i = 0; i < render->uNumBillboardsToDraw - v7; i++)
                {
                    memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
                        &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
                        sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
                }
            }
            ++v7;
        }
        uNumBillboardsToDraw++;
        return v7;
    }
    if (z <= render->pBillboardRenderListD3D[v7].z_order)
    {
        if ((signed int)render->uNumBillboardsToDraw > (signed int)v7)
        {
            for (uint i = 0; i < render->uNumBillboardsToDraw - v7; i++)
            {
                memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
                    &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
                    sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
            }
        }
        uNumBillboardsToDraw++;
        return v7;
    }
    return v7;
}

//----- (004A1E9D) --------------------------------------------------------
unsigned int Render::GetBillboardDrawListSize()
{
  return render->uNumBillboardsToDraw;
}

//----- (004A1EA3) --------------------------------------------------------
unsigned int Render::GetParentBillboardID(unsigned int uBillboardID)
{
  return render->pBillboardRenderListD3D[uBillboardID].sParentBillboardID;
}

//----- (004A1EB6) --------------------------------------------------------
void Render::BeginSceneD3D()
{
  if (!uNumD3DSceneBegins++)
  {
    //if (pRenderD3D)
    {
      pRenderD3D->ClearTarget(true, 0x00F08020, true, 1.0);
      render->uNumBillboardsToDraw = 0;
      pRenderD3D->pDevice->BeginScene();

      if ( uCurrentlyLoadedLevelType == LEVEL_Outdoor )
        uFogColor = GetLevelFogColor();
      else
        uFogColor = 0;

      if ( uFogColor & 0xFF000000 )
      {
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, 1);
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, uFogColor & 0xFFFFFF);
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0);
        bUsingSpecular = true;
      }
      else
      {
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, 0);
        bUsingSpecular = 0;
      }
    }
    /*else
    {
      LockRenderSurface((void **)&pTargetSurface, &uTargetSurfacePitch);
      if (pTargetSurface)
        field_18_locked_pitch = uTargetSurfacePitch;
      else
        --uNumD3DSceneBegins;
    }*/
  }
}

//----- (004A1FE1) --------------------------------------------------------
void Render::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene()
{
    --uNumD3DSceneBegins;
    if (uNumD3DSceneBegins)
        return;

    if (pRenderD3D)
    {
        pEngine->draw_debug_outlines();
        DoRenderBillboards_D3D();
        pEngine->pStru6Instance->RenderSpecialEffects();
        pRenderD3D->pDevice->EndScene();
    }
    else
        pEngine->pStru6Instance->RenderSpecialEffects();
}

//----- (004A2031) --------------------------------------------------------
unsigned int Render::GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6)
{
//  __debugbreak(); // should not fire outside decal builder
  return ::GetActorTintColor(tint, a4, a2, a5, a6);
}

/*void Render::DrawTerrainPolygon_new(Polygon *a3, IDirect3DTexture2 *pTexture)//new function
{
  int v5; // ebx@1
  int v6; // edi@1
  int v8; // eax@7
  float v9; // eax@12
  float *v10; // esi@12
  float v11; // ecx@14
  double v12; // st7@14
  double v13; // st7@14
  double v14; // st7@14
  signed int v15; // eax@14
  int v16; // eax@15
  float v17; // ST48_4@15
  char v18; // zf@17
  int v19; // eax@18
  int v20; // eax@18
  int v21; // edx@20
  signed int v22; // ecx@20
  int v23; // eax@20
  const char *v24; // ST4C_4@20
  unsigned int v25; // ST50_4@20
  int v26; // ST54_4@20
  int v27; // eax@20
  _UNKNOWN *v28; // eax@21
  int v29; // ecx@23
  int v30; // eax@23
  int v31; // eax@23
  int v32; // eax@24
  int v33; // eax@25
  int v34; // eax@25
  int v35; // eax@25
  int v36; // eax@25
  signed int v37; // ecx@26
  int v38; // eax@26
  _UNKNOWN *v39; // eax@27
  int v40; // edx@28
  int v41; // eax@29
  int v42; // eax@29
  int v43; // eax@29
  int v44; // eax@29
  unsigned int v46; // eax@29
  int v47; // eax@30
  int v48; // eax@30
  int v49; // eax@30
  double v52; // st6@35
  const char *v55; // [sp+4Ch] [bp-1Ch]@20
  int v57; // [sp+5Ch] [bp-Ch]@3
  signed int v59; // [sp+60h] [bp-8h]@12
  int v61; // [sp+64h] [bp-4h]@4
 int i;

  v6 = (int)this;
  v5 = 0;
 if (!this->uNumD3DSceneBegins)
   return;



 
     this->pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
     this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
     if (this->bUsingSpecular)
     {
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
     }

     pVertices[0].pos.x = VertexRenderList[0].vWorldViewProjX;
     pVertices[0].pos.y = VertexRenderList[0].vWorldViewProjY;
     pVertices[0].pos.z = 1.0 - 1.0 / (1000 * VertexRenderList[0].vWorldViewPosition.x / (double)pODMRenderParams->shading_dist_mist);
     pVertices[0].rhw = 1.0 / (VertexRenderList[0].vWorldViewPosition.x + 0.0000001000000011686097);
     pVertices[0].diffuse = GetActorTintColor(a3->field_58, 0, VertexRenderList[0].vWorldViewPosition.x, 0, 0);
     pVertices[0].specular = 0;
     pVertices[0].texcoord.x = VertexRenderList[0].u;
     pVertices[0].texcoord.y = VertexRenderList[0].v;
 
     pVertices[1].pos.x = VertexRenderList[3].vWorldViewProjX;
     pVertices[1].pos.y = VertexRenderList[3].vWorldViewProjY;
     pVertices[1].pos.z = 1.0 - 1.0 / (1000 * VertexRenderList[3].vWorldViewPosition.x / (double)pODMRenderParams->shading_dist_mist);
     pVertices[1].rhw = 1.0 / (VertexRenderList[3].vWorldViewPosition.x + 0.0000001000000011686097);
     pVertices[1].diffuse = GetActorTintColor(a3->field_58, 0, VertexRenderList[3].vWorldViewPosition.x, 0, 0);
     pVertices[1].specular = 0;
     pVertices[1].texcoord.x = VertexRenderList[3].u;
     pVertices[1].texcoord.y = VertexRenderList[3].v;

     pVertices[2].pos.x = VertexRenderList[1].vWorldViewProjX;
     pVertices[2].pos.y = VertexRenderList[1].vWorldViewProjY;
     pVertices[2].pos.z = 1.0 - 1.0 / (1000 * VertexRenderList[1].vWorldViewPosition.x / (double)pODMRenderParams->shading_dist_mist);
     pVertices[2].rhw = 1.0 / (VertexRenderList[1].vWorldViewPosition.x + 0.0000001000000011686097);
     pVertices[2].diffuse = GetActorTintColor(a3->field_58, 0, VertexRenderList[1].vWorldViewPosition.x, 0, 0);
     pVertices[2].specular = 0;
     pVertices[2].texcoord.x = VertexRenderList[1].u;
     pVertices[2].texcoord.y = VertexRenderList[1].v;

     memcpy(pVertices + 3, pVertices + 2, sizeof(RenderVertexD3D3));
     memcpy(pVertices + 4, pVertices + 1, sizeof(RenderVertexD3D3));

     pVertices[5].pos.x = VertexRenderList[2].vWorldViewProjX;
     pVertices[5].pos.y = VertexRenderList[2].vWorldViewProjY;
     pVertices[5].pos.z = 1.0 - 1.0 / (1000 * VertexRenderList[2].vWorldViewPosition.x / (double)pODMRenderParams->shading_dist_mist);
     pVertices[5].rhw = 1.0 / (VertexRenderList[2].vWorldViewPosition.x + 0.0000001000000011686097);
     pVertices[5].diffuse = GetActorTintColor(a3->field_58, 0, VertexRenderList[2].vWorldViewPosition.x, 0, 0);
     pVertices[5].specular = 0;
     pVertices[5].texcoord.x = VertexRenderList[2].u;
     pVertices[5].texcoord.y = VertexRenderList[2].v;


     this->pRenderD3D->pDevice->SetTexture(0, pTexture);
     this->pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, pVertices, 6, D3DDP_DONOTLIGHT);

}*/

//----- (004A26BC) --------------------------------------------------------
void Render::DrawTerrainPolygon(unsigned int uNumVertices, struct Polygon *a4, IDirect3DTexture2 *a5, bool transparent, bool clampAtTextureBorders)
{
  int v11; // eax@5
  int v20; // eax@14
  unsigned int v45; // eax@28

  if (!this->uNumD3DSceneBegins)
     return;
  if ( uNumVertices < 3)
     return;

    if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_01_lightmap_related)
    {
      v11 = ::GetActorTintColor(a4->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x, 0, 0);
      pEngine->pLightmapBuilder->DrawLightmaps(v11/*, 0*/);
    }
    else if (transparent || !pEngine->pLightmapBuilder->StationaryLightsCount || 
          byte_4D864C && pEngine->uFlags & GAME_FLAGS_2_SATURATE_LIGHTMAPS )
    {
        if (clampAtTextureBorders)
          this->pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
        else
          this->pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

        if (transparent || this->bUsingSpecular)
        {
          this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
          if (transparent)
          {
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
            //this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
            //this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
          }
          else
          {
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
          }
        }

		for (uint i = 0; i < uNumVertices; ++i)
		{
		
		  d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
		  d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
		  d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
		  d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
		  d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(a4->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
          d3d_vertex_buffer[i].specular = 0;          
		  if ( this->bUsingSpecular )
            d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);

		  d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
		  d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
		}

		this->pRenderD3D->pDevice->SetTexture(0, a5);
        this->pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT);
        if (transparent)
        {
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
        }
    }
    else if(pEngine->pLightmapBuilder->StationaryLightsCount)
    {
		for (uint i = 0; i < uNumVertices; ++i)
		{
		
		  d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
		  d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
		  d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) / (double)pODMRenderParams->shading_dist_mist);
		  d3d_vertex_buffer[i].rhw = 1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
		  d3d_vertex_buffer[i].diffuse = GetActorTintColor(a4->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
		  d3d_vertex_buffer[i].specular = 0;
          if ( this->bUsingSpecular )
            d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(0, 0, VertexRenderList[i].vWorldViewPosition.x);
 		  d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
		  d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
		}
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        if (render->bUsingSpecular)
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

        ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,//рисуется текстурка с светом
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer,
                uNumVertices,
                D3DDP_DONOTLIGHT));
        //ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
        pEngine->pLightmapBuilder->DrawLightmaps(-1/*, 0*/);
	    for (uint i = 0; i < uNumVertices; ++i)
	    {
		  d3d_vertex_buffer[i].diffuse = -1;
	    }
        ErrD3D(pRenderD3D->pDevice->SetTexture(0, a5));//текстурка 
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        if ( !render->bUsingSpecular )
        {
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer,
                uNumVertices,
                D3DDP_DONOTLIGHT));
        if ( render->bUsingSpecular )
        {
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
          ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
		  for (uint i = 0; i < uNumVertices; ++i)
		  {
			d3d_vertex_buffer[i].diffuse = render->uFogColor | d3d_vertex_buffer[i].specular & 0xFF000000;
			d3d_vertex_buffer[i].specular = 0;
		  }

          ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));//problem
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA));
          ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                  D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                  d3d_vertex_buffer,
                  uNumVertices,
                  D3DDP_DONOTLIGHT));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
          v45 = GetLevelFogColor();
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, v45 & 0xFFFFFF));
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, FALSE));
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
      //}
    }

    //if (pIndoorCamera->flags & INDOOR_CAMERA_DRAW_TERRAIN_OUTLINES || pBLVRenderParams->uFlags & INDOOR_CAMERA_DRAW_TERRAIN_OUTLINES)
    //if (pIndoorCameraD3D->debug_flags & ODM_RENDER_DRAW_TERRAIN_OUTLINES)
	if (debug_terrain_polygin)
      pIndoorCameraD3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x00FFFFFF, 0.0);
  }
// 4A26BC: could not find valid save-restore pair for esi
// 4D864C: using guessed type char byte_4D864C;

//----- (004A2DA3) --------------------------------------------------------
void Render::DrawOutdoorSkyPolygon(unsigned int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture)
{
  int v7; // eax@7

  if ( !this->uNumD3DSceneBegins )
    return;
  if ( uNumVertices >= 3 )
  {
    this->pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
    if ( this->bUsingSpecular )
    {
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
      this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    }
    for ( uint i = 0; i < uNumVertices; ++i )
    {
      pVertices[i].pos.x = VertexRenderList[i].vWorldViewProjX;
      pVertices[i].pos.y = VertexRenderList[i].vWorldViewProjY;
      pVertices[i].pos.z = 0.99989998;
      pVertices[i].rhw = VertexRenderList[i]._rhw;

      pVertices[i].diffuse = ::GetActorTintColor(31, 0, VertexRenderList[i].vWorldViewPosition.x, 1, 0);
      v7 = 0;
      if (this->bUsingSpecular)
        v7 = sub_47C3D7_get_fog_specular(0, 1, VertexRenderList[i].vWorldViewPosition.x);
      pVertices[i].specular = v7;
      pVertices[i].texcoord.x = VertexRenderList[i].u;
      pVertices[i].texcoord.y = VertexRenderList[i].v;
    }
    pRenderD3D->pDevice->SetTexture(0, pTexture);
    pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                       pVertices, uNumVertices, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
  }
}

//----- (004A2ED5) --------------------------------------------------------
void Render::DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon, IDirect3DTexture2 *pTexture)
{
  int v5; // eax@3

  if ( this->uNumD3DSceneBegins )
  {
    if ( uNumVertices >= 3 )
    {
      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
      v5 = 31 - (pSkyPolygon->dimming_level & 0x1F);
      if ( v5 < pOutdoor->max_terrain_dimming_level )
        v5 = pOutdoor->max_terrain_dimming_level;
      for (uint i = 0; i < (unsigned int)uNumVertices; ++i)
      {
        d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
        d3d_vertex_buffer[i].rhw = array_507D30[i]._rhw;
        d3d_vertex_buffer[i].diffuse = 8 * v5 | ((8 * v5 | (v5 << 11)) << 8);
        d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u;
        d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v;
      }
      ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture));
      ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        d3d_vertex_buffer, uNumVertices, 28));
    }
  }
}

//----- (00479A53) --------------------------------------------------------
void Render::DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID)
{
  BLVFace *pFace; // esi@1
  double v5; // st7@3
  signed __int64 v6; // qax@3
  int v12; // edx@7
  int v13; // eax@7
  int v17; // edi@9
  double v18; // st7@9
  signed int v19; // ebx@9
  void *v20; // ecx@9
  int v21; // ebx@11
  int v22; // eax@14
  signed __int64 v23; // qtt@16
  double v28; // st7@20
  double v33; // st6@23
  const void *v35; // ecx@31
  int v36; // eax@31
  const void *v37; // edi@31
  signed __int64 v38; // qax@31
  int v39; // ecx@31
  int v40; // ebx@33
  int v41; // eax@36
  signed __int64 v42; // qtt@39
  int v43; // eax@39
  double v48; // st7@41
  double v51; // st7@46
  struct Polygon pSkyPolygon; // [sp+14h] [bp-160h]@6
  unsigned int v63; // [sp+120h] [bp-54h]@7
  unsigned int v65; // [sp+128h] [bp-4Ch]@1
  unsigned int v66; // [sp+12Ch] [bp-48h]@7
  __int64 v69; // [sp+13Ch] [bp-38h]@3
  int v70; // [sp+144h] [bp-30h]@3
  int X; // [sp+148h] [bp-2Ch]@9
  int v72; // [sp+14Ch] [bp-28h]@7
  float v73; // [sp+150h] [bp-24h]@16
  unsigned int v74; // [sp+154h] [bp-20h]@3
  unsigned int v74_; // [sp+154h] [bp-20h]@3
  RenderVertexSoft *v75; // [sp+158h] [bp-1Ch]@3
  float v76; // [sp+15Ch] [bp-18h]@9
  int v77; // [sp+160h] [bp-14h]@9
  int v78; // [sp+164h] [bp-10h]@7
  void *v79; // [sp+168h] [bp-Ch]@9
  float v80; // [sp+16Ch] [bp-8h]@3
  const void *v81; // [sp+170h] [bp-4h]@7

  pFace = &pIndoor->pFaces[uFaceID];
  //for floor and wall(for example Selesta)-------------------
  if (pFace->uPolygonType == POLYGON_InBetweenFloorAndWall || pFace->uPolygonType == POLYGON_Floor)
  {
    int v69 = (GetTickCount() / 32) - pIndoorCameraD3D->vPartyPos.x;
    int v55 = (GetTickCount() / 32) + pIndoorCameraD3D->vPartyPos.y;
    for (uint i = 0; i < uNumVertices; ++i)
    {
      array_507D30[i].u = (v69 + array_507D30[i].u) * 0.25f;
      array_507D30[i].v = (v55 + array_507D30[i].v) * 0.25f;
    }
    render->DrawIndoorPolygon(uNumVertices, pFace, pBitmaps_LOD->pHardwareTextures[pFace->uBitmapID], pFace->GetTexture(), PID(OBJECT_BModel, uFaceID), -1, 0);
    return;
  }
//---------------------------------------
  v70 = (signed __int64)((double)(pBLVRenderParams->fov_rad_fixpoint * pIndoorCameraD3D->vPartyPos.z)//179
                       / (((double)pBLVRenderParams->fov_rad_fixpoint + 16192.0)
                        * 65536.0)
                       + (double)pBLVRenderParams->uViewportCenterY);
  v5 = (double)pIndoorCameraD3D->sRotationX * 0.0030664064;//0
  v6 = (signed __int64)((double)pBLVRenderParams->uViewportCenterY//183
                      - (double)pBLVRenderParams->fov_rad_fixpoint
                      / ((cos(v5) * 16192.0 + 0.0000001)
                       * 65535.0)
                      * (sin(v5) * -16192.0 - (double)pIndoorCameraD3D->vPartyPos.z));

  stru_8019C8._48653D_frustum_blv(65536, 0, 0, 0, 65536, 0);
  pSkyPolygon.Create_48607B(&stru_8019C8);
  pSkyPolygon.uTileBitmapID = pFace->uBitmapID;

  pSkyPolygon.pTexture = pBitmaps_LOD->GetTexture(pSkyPolygon.uTileBitmapID);
  if ( !pSkyPolygon.pTexture )
    return;

  pSkyPolygon.dimming_level = 0;
  pSkyPolygon.uNumVertices = uNumVertices;

  pSkyPolygon.v_18.x = -stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX + 16);
  pSkyPolygon.v_18.y = 0;
  pSkyPolygon.v_18.z = -stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX + 16);

  memcpy(&array_507D30[uNumVertices], array_507D30, sizeof(array_507D30[uNumVertices]));
  pSkyPolygon.field_24 = 0x2000000;

  extern float _calc_fov(int viewport_width, int angle_degree);
  //v64 = (double)(signed int)(pBLVRenderParams->uViewportZ - pBLVRenderParams->uViewportX) * 0.5;
  //v72 = 65536 / (signed int)(signed __int64)(v64 / tan(0.6457717418670654) + 0.5);
  v72 = 65536.0f / _calc_fov(pBLVRenderParams->uViewportZ - pBLVRenderParams->uViewportX, 74);
  v12 = pSkyPolygon.pTexture->uWidthMinus1;
  v13 = pSkyPolygon.pTexture->uHeightMinus1;
  //v67 = 1.0 / (double)pSkyPolygon.pTexture->uTextureWidth;
  v63 = 224 * pMiscTimer->uTotalGameTimeElapsed & v13;
  v66 = 224 * pMiscTimer->uTotalGameTimeElapsed & v12;
  v78 = 0;
  //v81 = 0;
  float v68 = 1.0 / (double)pSkyPolygon.pTexture->uTextureHeight;
  if ( (signed int)pSkyPolygon.uNumVertices <= 0 )
    return;

  int _507D30_idx = 0;
  for ( _507D30_idx; _507D30_idx < pSkyPolygon.uNumVertices; _507D30_idx++ )
  {
    //v15 = (void *)(v72 * (v70 - (int)array_507D30[_507D30_idx].vWorldViewProjY));
    v77 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, v72 * (v70 - array_507D30[_507D30_idx].vWorldViewProjY));
    v74 = v77 + pSkyPolygon.ptr_38->angle_from_north;

    v77 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_north_south, v72 * (v70 - array_507D30[_507D30_idx].vWorldViewProjY));
    v74_ = v77 + pSkyPolygon.ptr_38->angle_from_east;

    v79 = (void *)(fixpoint_mul(pSkyPolygon.v_18.z, v72 * (v70 - (int)array_507D30[_507D30_idx].vWorldViewProjY)));
    v17 = v72 * (pBLVRenderParams->uViewportCenterX - (int)array_507D30[_507D30_idx].vWorldViewProjX);
    v18 = array_507D30[_507D30_idx].vWorldViewProjY - 1.0;
    v19 = -pSkyPolygon.field_24;
    v77 = -pSkyPolygon.field_24;
    X = (int)((char *)v79 + pSkyPolygon.v_18.x);
    LODWORD(v76) = (signed __int64)v18;
    v20 = (void *)(v72 * (v70 - LODWORD(v76)));
    while ( 1 )
    {
      v79 = v20;
      if ( !X )
        goto LABEL_14;
      v21 = abs(v19 >> 14);
      if ( v21 <= abs(X) )//0x800 <= 0x28652
        break;
      if ( SLODWORD(v76) <= (signed int)pViewport->uViewportTL_Y )
        break;
      v19 = v77;
      v20 = v79;
LABEL_14:
      v79 = (void *)fixpoint_mul(pSkyPolygon.v_18.z, (int)v20);
      v22 = fixpoint_mul(pSkyPolygon.v_18.z, (int)v20);
      --LODWORD(v76);
      v20 = (char *)v20 + v72;
      X = v22 + pSkyPolygon.v_18.x;
      v78 = 1;
    }
    if ( !v78 )
    {
      LODWORD(v23) = v77 << 16;
      HIDWORD(v23) = v77 >> 16;//v23 = 0xfffffe0000000000
      v79 = (void *)(v23 / X);//X = FFFF9014(-28652)
      v77 = v17;
      signed __int64 s = v74 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_west, v17);// s = 0xFFFFFFFF FFFF3EE6
      LODWORD(v80) = v66 + ((signed int)fixpoint_mul(SLODWORD(s), v23 / X) >> 4);
      array_507D30[_507D30_idx].u = ((double)SLODWORD(v80) * 0.000015259022) * (1.0 / (double)pSkyPolygon.pTexture->uTextureWidth);

      signed __int64 s2 = v74_ + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_south, v17);
      LODWORD(v80) = v63 + ((signed int)fixpoint_mul(SLODWORD(s2), v23 / X) >> 4);
      array_507D30[_507D30_idx].v = ((double)SLODWORD(v80) * 0.000015259022) * v68;

      v77 = fixpoint_mul(SLODWORD(s), v23 / X);
      LODWORD(v73) = fixpoint_mul(SLODWORD(s2), v23 / X);
      array_507D30[_507D30_idx]._rhw = 65536.0 / (double)(signed int)v79;

      //if ( (int)v81 >= pSkyPolygon.uNumVertices )
      //{
      //  render->DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon,
      //     pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
      //  return;
      //}
      continue;
    }
    break;
  }
  if ( _507D30_idx >= pSkyPolygon.uNumVertices )
  {
    render->DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon,
       pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
    return;
  }
  LODWORD(v73) = 0;
  v80 = v76;
  if ( (signed int)pSkyPolygon.uNumVertices > 0 )
  {
    v28 = (double)SLODWORD(v76);
    LODWORD(v76) = (int)(char *)VertexRenderList + 28;
	uint i = 0;
    for ( v78 = pSkyPolygon.uNumVertices; v78; --v78 )
    {
      ++LODWORD(v73);
      memcpy(&VertexRenderList[i], &array_507D30[i], 0x30u);
      LODWORD(v76) += 48;
      if ( v28 < array_507D30[i].vWorldViewProjY | v28 == array_507D30[i].vWorldViewProjY
        || v28 >= array_507D30[i + 1].vWorldViewProjY )
      {
        if ( v28 >= array_507D30[i].vWorldViewProjY || v28 <= array_507D30[i + 1].vWorldViewProjY )
        {
          i++;
          continue;
        }
        v33 = (array_507D30[i + 1].vWorldViewProjX - array_507D30[i].vWorldViewProjX) * v28 / (array_507D30[i + 1].vWorldViewProjY - array_507D30[i].vWorldViewProjY)
            + array_507D30[i + 1].vWorldViewProjX;
      }
      else
      {
        v33 = (array_507D30[i].vWorldViewProjX - array_507D30[i + 1].vWorldViewProjX) * v28 / (array_507D30[i].vWorldViewProjY - array_507D30[i + 1].vWorldViewProjY)
            + array_507D30[i].vWorldViewProjX;
      }
      VertexRenderList[i + 1].vWorldViewProjX = v33;
      ++LODWORD(v73);
      *(unsigned int *)LODWORD(v76) = v28;
      LODWORD(v76) += 48;
      i++;
    }
  }
  if ( SLODWORD(v73) <= 0 )
    goto LABEL_40;
  //v34 = (char *)&VertexRenderList[0].vWorldViewProjY;
  uint j = 0;
  v65 = v77 >> 14;
  //HIDWORD(v69) = LODWORD(v73);
  for ( int t = (int)LODWORD(v73); t > 1; t-- )
  {
    v35 = (const void *)(v72 * (v70 - (unsigned __int64)(signed __int64)VertexRenderList[j].vWorldViewProjY));

    //v78 = pSkyPolygon.ptr_38->viewing_angle_from_west_east;
    //v81 = (const void *)fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, v35);
    v36 = (int)(fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, (int)v35) + pSkyPolygon.ptr_38->angle_from_north);

    v81 = v35;
    v74 = v36;
    //v78 = pSkyPolygon.ptr_38->viewing_angle_from_north_south;
    v81 = (const void *)fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_north_south, (int)v35);
    v78 = (int)v35;
    v75 = (RenderVertexSoft *)((char *)v81 + pSkyPolygon.ptr_38->angle_from_east);
    //v81 = (const void *)pSkyPolygon.v_18.z;
    v78 = fixpoint_mul(pSkyPolygon.v_18.z, (int)v35);
    v37 = (const void *)(v72 * (pBLVRenderParams->uViewportCenterX - (unsigned __int64)(signed __int64)VertexRenderList[j].vWorldViewProjX));
    v38 = (signed __int64)(VertexRenderList[j].vWorldViewProjY - 1.0);
    v81 = 0;
    LODWORD(v76) = v38;
    v39 = v72 * (v70 - v38);
    while ( 1 )
    {
      v78 = v39;
      if ( !X )
        goto LABEL_36;
      v40 = abs(X);
      if ( abs((signed __int64)v65) <= v40 )
        break;
      if ( SLODWORD(v76) <= (signed int)pViewport->uViewportTL_Y )
        break;
      v39 = v78;
LABEL_36:
      v78 = pSkyPolygon.v_18.z;
      v41 = fixpoint_mul(pSkyPolygon.v_18.z, v39);
      --LODWORD(v76);
      v39 += v72;
      X = v41 + pSkyPolygon.v_18.x;
      v81 = (const void *)1;
    }
    if ( v81 )
    {
      v79 = (void *)pSkyPolygon.v_18.z;
      v78 = 2 * LODWORD(v76);
      v81 = (const void *)fixpoint_mul(pSkyPolygon.v_18.z, (((double)v70 - ((double)(2 * LODWORD(v76)) - VertexRenderList[j].vWorldViewProjY))
                                                                                         * (double)v72));
      X = (int)((char *)v81 + pSkyPolygon.v_18.x);
    }
    LODWORD(v42) = v77 << 16;
    HIDWORD(v42) = v77 >> 16;
    v79 = (void *)(v42 / X);
    v81 = v37;

    //v78 = pSkyPolygon.ptr_38->angle_from_west;
    v81 = (const void *)fixpoint_mul(pSkyPolygon.ptr_38->angle_from_west, (int)v37);
    v43 = v74 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_west, (int)v37);
    v74 = (unsigned int)v37;
    LODWORD(v76) = v43;

    //v78 = pSkyPolygon.ptr_38->angle_from_south;
    v75 = (RenderVertexSoft *)((char *)v75 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_south, (int)v37));
    //v74 = fixpoint_mul(v43, v42 / X);
    v81 = (const void *)fixpoint_mul((int)v75, v42 / X);

    //v34 += 48;
    //v78 = v66 + ((signed int)fixpoint_mul(v43, v42 / X) >> 4);
    //v44 = HIDWORD(v69)-- == 1;
    //v45 = (double)(v66 + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) * 0.000015259022;
    //v78 = v63 + ((signed int)fixpoint_mul((int)v75, v42 / X) >> 4);
    VertexRenderList[j].u = ((double)(v66 + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) * 0.000015259022) * (1.0 / (double)pSkyPolygon.pTexture->uTextureWidth);
    VertexRenderList[j].v = ((double)(v66 + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) * 0.000015259022) * v68;
    //v46 = (double)(signed int)v79;
    VertexRenderList[j].vWorldViewPosition.x = 0.000015258789 * (double)(signed int)v79;
    VertexRenderList[j]._rhw = 65536.0 / (double)(signed int)v79;
	++j;
  }
  //while ( !v44 );
LABEL_40:
  uint i = 0;
  if ( SLODWORD(v73) > 0 )
  {
    v48 = (double)SLODWORD(v80);
    for ( HIDWORD(v69) = LODWORD(v73); HIDWORD(v69); --HIDWORD(v69) )
    {
      if ( v48 >= VertexRenderList[i].vWorldViewProjY )
      {
        ++i;
        memcpy(&array_507D30[i], &VertexRenderList[i], 0x30u);
      }
    }
  }
  pSkyPolygon.uNumVertices = i;
  render->DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon, pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
  int pNumVertices = 0;
  if ( SLODWORD(v73) > 0 )
  {
    v51 = (double)SLODWORD(v80);
    for ( v80 = v73; v80 != 0.0; --LODWORD(v80) )
    {
      if ( v51 <= VertexRenderList[pNumVertices].vWorldViewProjY )
      {
        ++pNumVertices;
        memcpy(&array_507D30[pNumVertices], &VertexRenderList[pNumVertices], 0x30u);
      }
    }
  }
  render->DrawIndoorSkyPolygon(pNumVertices, &pSkyPolygon, pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
}

//----- (004A2FC0) --------------------------------------------------------
void Render::DrawIndoorPolygon(unsigned int uNumVertices, BLVFace *pFace, IDirect3DTexture2 *pHwTex, Texture_MM7 *pTex, int uPackedID, unsigned int uColor, int a8)
{
  if (!uNumD3DSceneBegins || uNumVertices < 3)
    return;

  int sCorrectedColor = uColor;

  if (pEngine->pLightmapBuilder->StationaryLightsCount)
    sCorrectedColor = -1;
  pEngine->AlterGamma_BLV(pFace, &sCorrectedColor);

  if (pFace->uAttributes & FACE_OUTLINED)
  {
    if (GetTickCount() % 300 >= 150)
      uColor = sCorrectedColor = 0xFF20FF20;
    else 
	  uColor = sCorrectedColor = 0xFF109010;
  }

  if (byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_01_lightmap_related)
  {
      __debugbreak();
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false));
      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
	  for (uint i = 0; i < uNumVertices; ++i)
	  {
		d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
		d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
		d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
		d3d_vertex_buffer[i].rhw = 1.0 / array_507D30[i].vWorldViewPosition.x;
		d3d_vertex_buffer[i].diffuse = sCorrectedColor;
		d3d_vertex_buffer[i].specular = 0;
		d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u / (double)pTex->uTextureWidth;
		d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v / (double)pTex->uTextureHeight;
	  }

      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
      ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
      ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              d3d_vertex_buffer, uNumVertices, 28));
      pEngine->pLightmapBuilder->DrawLightmaps(-1/*, 0*/);
  }
  else
  {
    if (!pEngine->pLightmapBuilder->StationaryLightsCount || byte_4D864C && pEngine->uFlags & GAME_FLAGS_2_SATURATE_LIGHTMAPS)
    {
      for (uint i = 0; i < uNumVertices; ++i)
      {
        d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
        d3d_vertex_buffer[i].rhw = 1.0 / array_507D30[i].vWorldViewPosition.x;
        d3d_vertex_buffer[i].diffuse = sCorrectedColor;
        d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u / (double)pTex->uTextureWidth;
        d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v / (double)pTex->uTextureHeight;
      }

      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
      ErrD3D(pRenderD3D->pDevice->SetTexture(0, pHwTex));
      ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer, uNumVertices, 28));
    }
    else
    {
      for (uint i = 0; i < uNumVertices; ++i)
      {
        d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
        d3d_vertex_buffer[i].rhw = 1.0 / array_507D30[i].vWorldViewPosition.x;
        d3d_vertex_buffer[i].diffuse = uColor;
        d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u / (double)pTex->uTextureWidth;
        d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v / (double)pTex->uTextureHeight;
      }
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
      ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
      ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              d3d_vertex_buffer, uNumVertices, 28));

      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
      pEngine->pLightmapBuilder->DrawLightmaps(-1/*, 0*/);

      for (uint i = 0; i < uNumVertices; ++i)
        d3d_vertex_buffer[i].diffuse = sCorrectedColor;

      ErrD3D(pRenderD3D->pDevice->SetTexture(0, pHwTex));
      ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR));
      ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              d3d_vertex_buffer, uNumVertices, 28));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    }
  }
}
// 4D864C: using guessed type char byte_4D864C;

//----- (004A43B1) --------------------------------------------------------
void Render::DrawBillboard_Indoor(RenderBillboardTransform_local0 *pSoftBillboard, Sprite *pSprite, int dimming_level)
{
  unsigned int v7; // eax@2
  signed int v11; // eax@9
  signed int v12; // eax@9
  double v15; // st5@12
  double v16; // st4@12
  double v17; // st3@12
  double v18; // st2@12
  int v19; // ecx@14
  double v20; // st3@14
  int v21; // ecx@16
  double v22; // st3@16
  float v27; // [sp+24h] [bp-Ch]@5
  int v28; // [sp+28h] [bp-8h]@2
  float v29; // [sp+2Ch] [bp-4h]@5
  float v31; // [sp+3Ch] [bp+Ch]@5
  float a1; // [sp+40h] [bp+10h]@5

  if ( this->uNumD3DSceneBegins )
  {
    //v4 = pSoftBillboard;
    //v5 = (double)pSoftBillboard->zbuffer_depth;
    //pSoftBillboarda = pSoftBillboard->zbuffer_depth;
    //v6 = pSoftBillboard->zbuffer_depth;
    v7 = Billboard_ProbablyAddToListAndSortByZOrder(pSoftBillboard->zbuffer_depth);
    //v8 = dimming_level;
    //v9 = v7;
    v28 = dimming_level & 0xFF000000;
    if ( dimming_level & 0xFF000000 )
      pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Opaque_3;
    else
      pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Transparent;
    //v10 = a3;
    pBillboardRenderListD3D[v7].field_90 = pSoftBillboard->field_44;
    pBillboardRenderListD3D[v7].sZValue = pSoftBillboard->sZValue;
    pBillboardRenderListD3D[v7].sParentBillboardID = pSoftBillboard->sParentBillboardID;
    //v25 = pSoftBillboard->uScreenSpaceX;
    //v24 = pSoftBillboard->uScreenSpaceY;
    a1 = (pSoftBillboard->_screenspace_x_scaler_packedfloat & 0xFFFF) * 0.000015260186 + HIWORD(pSoftBillboard->_screenspace_x_scaler_packedfloat);
    v29 = (pSoftBillboard->_screenspace_y_scaler_packedfloat & 0xFFFF) * 0.000015260186 + HIWORD(pSoftBillboard->_screenspace_y_scaler_packedfloat);
    v31 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v27 = (double)(pSprite->uBufferHeight - pSprite->uAreaY);
    if ( pSoftBillboard->uFlags & 4 )
      v31 = v31 * -1.0;
    if ( pSoftBillboard->sTintColor && this->bTinting )
    {
      v11 = ::GetActorTintColor(dimming_level, 0, pSoftBillboard->zbuffer_depth, 0, 0);
      v12 = BlendColors(pSoftBillboard->sTintColor, v11);
      if ( v28 )
        v12 = (unsigned int)((char *)&array_77EC08[1852].pEdgeList1[17] + 3) & ((unsigned int)v12 >> 1);
    }
    else
    {
      v12 = ::GetActorTintColor(dimming_level, 0, pSoftBillboard->zbuffer_depth, 0, 0);
    }
    //v13 = (double)v25;
    pBillboardRenderListD3D[v7].pQuads[0].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[0].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[0].pos.x = pSoftBillboard->uScreenSpaceX - v31 * a1;
    //v14 = (double)v24;
    //v32 = v14;
    pBillboardRenderListD3D[v7].pQuads[0].pos.y = pSoftBillboard->uScreenSpaceY - v27 * v29;
    v15 = 1.0 - 1.0 / (pSoftBillboard->zbuffer_depth * 0.061758894);
    pBillboardRenderListD3D[v7].pQuads[0].pos.z = v15;
    v16 = 1.0 / pSoftBillboard->zbuffer_depth;
    pBillboardRenderListD3D[v7].pQuads[0].rhw = 1.0 / pSoftBillboard->zbuffer_depth;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.y = 0.0;
    v17 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v18 = (double)(pSprite->uBufferHeight - pSprite->uAreaY - pSprite->uAreaHeight);
    if ( pSoftBillboard->uFlags & 4 )
      v17 = v17 * -1.0;
    pBillboardRenderListD3D[v7].pQuads[1].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[1].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[1].pos.x = pSoftBillboard->uScreenSpaceX - v17 * a1;
    pBillboardRenderListD3D[v7].pQuads[1].pos.y = pSoftBillboard->uScreenSpaceY - v18 * v29;
    pBillboardRenderListD3D[v7].pQuads[1].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[1].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.y = 1.0;
    v19 = pSprite->uBufferHeight - pSprite->uAreaY - pSprite->uAreaHeight;
    v20 = (double)(pSprite->uAreaX + pSprite->uAreaWidth + (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if ( pSoftBillboard->uFlags & 4 )
      v20 = v20 * -1.0;
    pBillboardRenderListD3D[v7].pQuads[2].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[2].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[2].pos.x = v20 * a1 + pSoftBillboard->uScreenSpaceX;
    pBillboardRenderListD3D[v7].pQuads[2].pos.y = pSoftBillboard->uScreenSpaceY - (double)v19 * v29;
    pBillboardRenderListD3D[v7].pQuads[2].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[2].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.y = 1.0;
    v21 = pSprite->uBufferHeight - pSprite->uAreaY;
    v22 = (double)(pSprite->uAreaX + pSprite->uAreaWidth + (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if ( pSoftBillboard->uFlags & 4 )
      v22 = v22 * -1.0;
    pBillboardRenderListD3D[v7].pQuads[3].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[3].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[3].pos.x = v22 * a1 + pSoftBillboard->uScreenSpaceX;
    pBillboardRenderListD3D[v7].pQuads[3].pos.y = pSoftBillboard->uScreenSpaceY - (double)v21 * v29;
    pBillboardRenderListD3D[v7].pQuads[3].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[3].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.y = 0.0;
    //v23 = pSprite->pTexture;
    pBillboardRenderListD3D[v7].uNumVertices = 4;
    pBillboardRenderListD3D[v7].z_order = pSoftBillboard->zbuffer_depth;
    pBillboardRenderListD3D[v7].pTexture = pSprite->pTexture;
  }
}

//----- (004A354F) --------------------------------------------------------
void Render::MakeParticleBillboardAndPush_BLV(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle)
{
  unsigned int v8; // esi@3
  float v11; // ST28_4@3
  float v16; // ST2C_4@3
  float v17; // ST30_4@3
  signed int v18; // ST18_4@3
  signed int v19; // ST14_4@3
  signed int v20; // ST10_4@3
  signed int v21; // eax@3
  double v22; // st6@3
  float v23; // ST2C_4@3
  float v24; // ST30_4@3
  signed int v25; // ST10_4@3
  signed int v26; // ST14_4@3
  signed int v27; // ST18_4@3
  signed int v28; // eax@3
  double v29; // st6@3
  float v30; // ecx@3
  float v31; // ST2C_4@3
  float v32; // ST30_4@3
  signed int v33; // ST10_4@3
  signed int v34; // ST14_4@3
  signed int v35; // ST18_4@3
  signed int v36; // eax@3
  float v37; // ecx@3
  double v38; // st6@3
  float v39; // ST2C_4@3
  float v40; // ST30_4@3
  signed int v41; // ST10_4@3
  signed int v42; // ST14_4@3
  signed int v43; // ST18_4@3
  signed int v44; // eax@3
  double v45; // st6@3
  float v46; // eax@3

  if ( this->uNumD3DSceneBegins )
  {
    if (a2->zbuffer_depth)
    {
      //v5 = (double)a2->zbuffer_depth;
      //v6 = v5;
      //v7 = v5;
      v8 = Billboard_ProbablyAddToListAndSortByZOrder(a2->zbuffer_depth);
      pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Opaque_1;
      pBillboardRenderListD3D[v8].field_90 = a2->field_44;
      pBillboardRenderListD3D[v8].sZValue = a2->sZValue;
      pBillboardRenderListD3D[v8].sParentBillboardID = a2->sParentBillboardID;
      //v9 = a2->uScreenSpaceX;
      //v10 = a2->uScreenSpaceY;
      v11 = (a2->_screenspace_x_scaler_packedfloat & 0xFFFF) * 0.000015260186 + HIWORD(a2->_screenspace_x_scaler_packedfloat);
      //v12 = (double) a2->uScreenSpaceX;
      //v13 = v12;
      //v14 = (double)(a2->uScreenSpaceY - 12);
      //v15 = v14;
      v16 = (double)( a2->uScreenSpaceX - 12) - (double) a2->uScreenSpaceX;
      v17 = (double)(a2->uScreenSpaceY - 25) - (double)(a2->uScreenSpaceY - 12);
      v18 = stru_5C6E00->Cos(angle);
      v19 = stru_5C6E00->Sin(angle);
      v20 = stru_5C6E00->Sin(angle);
      v21 = stru_5C6E00->Cos(angle);
      pBillboardRenderListD3D[v8].pQuads[0].pos.x = (((double)(unsigned __int16)v18 * 0.000015259022
                                                       + (double)(v18 >> 16))
                                                       * v16
                                                       - ((double)(unsigned __int16)v19 * 0.000015259022
                                                       + (double)(v19 >> 16))
                                                       * v17)
                                                       * v11 + (double) a2->uScreenSpaceX;
      v22 = (((double)(unsigned __int16)v21 * 0.000015259022 + (double)(v21 >> 16)) * v17
           + ((double)(unsigned __int16)v20 * 0.000015259022 + (double)(v20 >> 16)) * v16
           - 12.0)
          * v11
          + (double)a2->uScreenSpaceY;
      pBillboardRenderListD3D[v8].pQuads[0].specular = 0;
      pBillboardRenderListD3D[v8].pQuads[0].diffuse = uDiffuse;
      pBillboardRenderListD3D[v8].pQuads[0].pos.y = v22;
      pBillboardRenderListD3D[v8].pQuads[0].pos.z = 1.0 - 1.0 / (a2->zbuffer_depth * 0.061758894);
      pBillboardRenderListD3D[v8].pQuads[0].rhw = 1.0 / a2->zbuffer_depth;
      pBillboardRenderListD3D[v8].pQuads[0].texcoord.x = 0.0;
      pBillboardRenderListD3D[v8].pQuads[0].texcoord.y = 0.0;
      v31 = (double)(a2->uScreenSpaceX + 12) - (double) a2->uScreenSpaceX;
      v32 = (double)a2->uScreenSpaceY - (double)(a2->uScreenSpaceY - 12);
      v25 = stru_5C6E00->Cos(angle);
      v26 = stru_5C6E00->Sin(angle);
      v27 = stru_5C6E00->Sin(angle);
      v28 = stru_5C6E00->Cos(angle);
      pBillboardRenderListD3D[v8].pQuads[1].pos.x = (((double)(unsigned __int16)v25 * 0.000015259022
                                                       + (double)(v25 >> 16))
                                                       * v31
                                                       - ((double)(unsigned __int16)v26 * 0.000015259022
                                                       + (double)(v26 >> 16))
                                                       * v32)
                                                       * v11 + (double) a2->uScreenSpaceX;
      v29 = (((double)(unsigned __int16)v28 * 0.000015259022 + (double)(v28 >> 16)) * v32
           + ((double)(unsigned __int16)v27 * 0.000015259022 + (double)(v27 >> 16)) * v31
           - 12.0)
          * v11
          + (double)a2->uScreenSpaceY;
      pBillboardRenderListD3D[v8].pQuads[1].pos.z = render->pBillboardRenderListD3D[v8].pQuads[0].pos.z;
      v30 = pBillboardRenderListD3D[v8].pQuads[0].rhw;
      pBillboardRenderListD3D[v8].pQuads[1].pos.y = v29;
      pBillboardRenderListD3D[v8].pQuads[1].specular = 0;
      pBillboardRenderListD3D[v8].pQuads[1].rhw = v30;
      pBillboardRenderListD3D[v8].pQuads[1].diffuse = uDiffuse;
      pBillboardRenderListD3D[v8].pQuads[1].texcoord.x = 0.0;
      pBillboardRenderListD3D[v8].pQuads[1].texcoord.y = 1.0;
      v23 = (double)(a2->uScreenSpaceX - 12) - (double) a2->uScreenSpaceX;
      v24 = (double)a2->uScreenSpaceY - (double)(a2->uScreenSpaceY - 12);
      v33 = stru_5C6E00->Cos(angle);
      v34 = stru_5C6E00->Sin(angle);
      v35 = stru_5C6E00->Sin(angle);
      v36 = stru_5C6E00->Cos(angle);
      pBillboardRenderListD3D[v8].pQuads[2].pos.x = (((double)(unsigned __int16)v33 * 0.000015259022
                                                        + (double)(v33 >> 16))
                                                        * v23
                                                        - ((double)(unsigned __int16)v34 * 0.000015259022
                                                        + (double)(v34 >> 16))
                                                        * v24)
                                                        * v11 + (double) a2->uScreenSpaceX;
      v37 = pBillboardRenderListD3D[v8].pQuads[0].pos.z;
      v38 = (((double)(unsigned __int16)v36 * 0.000015259022 + (double)(v36 >> 16)) * v24
           + ((double)(unsigned __int16)v35 * 0.000015259022 + (double)(v35 >> 16)) * v23
           - 12.0)
          * v11
          + (double)a2->uScreenSpaceY;
      pBillboardRenderListD3D[v8].pQuads[2].specular = 0;
      pBillboardRenderListD3D[v8].pQuads[2].pos.z = v37;
      pBillboardRenderListD3D[v8].pQuads[2].rhw = pBillboardRenderListD3D[v8].pQuads[0].rhw;
      pBillboardRenderListD3D[v8].pQuads[2].diffuse = uDiffuse;
      pBillboardRenderListD3D[v8].pQuads[2].pos.y = v38;
      pBillboardRenderListD3D[v8].pQuads[2].texcoord.x = 1.0;
      pBillboardRenderListD3D[v8].pQuads[2].texcoord.y = 1.0;
      v39 = (double)(a2->uScreenSpaceX + 12) - (double) a2->uScreenSpaceX;
      v40 = (double)(a2->uScreenSpaceY - 25) - (double)(a2->uScreenSpaceY - 12);
      v41 = stru_5C6E00->Cos(angle);
      v42 = stru_5C6E00->Sin(angle);
      v43 = stru_5C6E00->Sin(angle);
      v44 = stru_5C6E00->Cos(angle);
      pBillboardRenderListD3D[v8].pQuads[3].pos.x = (((double)(unsigned __int16)v41 * 0.000015259022
                                                        + (double)(v41 >> 16))
                                                        * v39
                                                        - ((double)(unsigned __int16)v42 * 0.000015259022
                                                        + (double)(v42 >> 16))
                                                        * v40)
                                                        * v11 + (double) a2->uScreenSpaceX;
      v45 = (((double)(unsigned __int16)v44 * 0.000015259022 + (double)(v44 >> 16)) * v40
           + ((double)(unsigned __int16)v43 * 0.000015259022 + (double)(v43 >> 16)) * v39
           - 12.0)
          * v11
          + (double)a2->uScreenSpaceY;
      v46 = pBillboardRenderListD3D[v8].pQuads[0].pos.z;
      pBillboardRenderListD3D[v8].pQuads[3].specular = 0;
      pBillboardRenderListD3D[v8].pQuads[3].pos.z = v46;
      pBillboardRenderListD3D[v8].pQuads[3].rhw = pBillboardRenderListD3D[v8].pQuads[0].rhw;
      pBillboardRenderListD3D[v8].pQuads[3].diffuse = uDiffuse;
      pBillboardRenderListD3D[v8].pTexture = a3;
      pBillboardRenderListD3D[v8].z_order = a2->zbuffer_depth;
      pBillboardRenderListD3D[v8].uNumVertices = 4;
      pBillboardRenderListD3D[v8].pQuads[3].pos.y = v45;
      pBillboardRenderListD3D[v8].pQuads[3].texcoord.x = 1.0;
      pBillboardRenderListD3D[v8].pQuads[3].texcoord.y = 0.0;
    }
  }
}

//----- (004A3AD9) --------------------------------------------------------
void Render::MakeParticleBillboardAndPush_ODM(RenderBillboardTransform_local0 *a2, IDirect3DTexture2 *a3, unsigned int uDiffuse, int angle)
{
  double v5; // st7@2
  float v6; // ST28_4@2
  float v7; // ST00_4@2
  unsigned int v8; // esi@2
  //int v9; // eax@2
  //int v10; // ebx@2
  float v11; // ST34_4@2
  double v12; // st7@2
  float v13; // ST2C_4@2
  double v14; // st6@2
  float v15; // ST24_4@2
  float v16; // ST38_4@2
  float v17; // ST3C_4@2
  signed int v18; // ST1C_4@2
  int v19; // ST30_4@2
  signed int v20; // ST20_4@2
  signed int v21; // ST18_4@2
  signed int v22; // eax@2
  double v23; // st6@2
  float v24; // ST20_4@2
  float v25; // ST1C_4@2
  float v26; // ST38_4@2
  float v27; // ST3C_4@2
  signed int v28; // ST18_4@2
  signed int v29; // ST14_4@2
  signed int v30; // ST10_4@2
  signed int v31; // eax@2
  double v32; // st6@2
  float v33; // ST38_4@2
  float v34; // ST3C_4@2
  signed int v35; // ST10_4@2
  signed int v36; // ST14_4@2
  signed int v37; // ST18_4@2
  signed int v38; // eax@2
  double v39; // st6@2
  float v40; // ST38_4@2
  float v41; // ST3C_4@2
  signed int v42; // ST10_4@2
  signed int v43; // ST14_4@2
  signed int v44; // ST18_4@2
  signed int v45; // eax@2
  double v46; // st6@2

  if ( this->uNumD3DSceneBegins )
  {
    v5 = (double)a2->zbuffer_depth;
    v6 = v5;
    v7 = v5;
    v8 = Billboard_ProbablyAddToListAndSortByZOrder(LODWORD(v7));
    pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Opaque_1;
    pBillboardRenderListD3D[v8].field_90 = a2->field_44;
    pBillboardRenderListD3D[v8].sZValue = a2->sZValue;
    pBillboardRenderListD3D[v8].sParentBillboardID = a2->sParentBillboardID;

    //v9 = a2->uScreenSpaceX;
    //v10 = a2->uScreenSpaceY;
    v11 = (a2->_screenspace_x_scaler_packedfloat & 0xFFFF) * 0.000015260186 + HIWORD(a2->_screenspace_x_scaler_packedfloat);
    v12 = (double)a2->uScreenSpaceX;
    v13 = (double)a2->uScreenSpaceX;
    v14 = (double)(a2->uScreenSpaceY - 12);
    v15 = v14;
    v16 = (double)(a2->uScreenSpaceX - 12) - v12;
    v17 = (double)(a2->uScreenSpaceY - 25) - v14;
    v18 = stru_5C6E00->Cos(angle);
    v19 = angle - stru_5C6E00->uIntegerHalfPi;
    v20 = stru_5C6E00->Sin(angle);
    v21 = stru_5C6E00->Sin(angle);
    v22 = stru_5C6E00->Cos(angle);
    pBillboardRenderListD3D[v8].pQuads[0].pos.x = (((double)(unsigned __int16)v18 * 0.000015259022
                                                    + (double)(v18 >> 16)) * v16
                                                    - ((double)(unsigned __int16)v20 * 0.000015259022
                                                    + (double)(v20 >> 16)) * v17)
                                                    * v11 + v13;
    v23 = (((double)(unsigned __int16)v22 * 0.000015259022 + (double)(v22 >> 16)) * v17
         + ((double)(unsigned __int16)v21 * 0.000015259022 + (double)(v21 >> 16)) * v16
         - 12.0)
        * v11
        + (double)a2->uScreenSpaceY;
    pBillboardRenderListD3D[v8].pQuads[0].specular = 0;
    pBillboardRenderListD3D[v8].pQuads[0].diffuse = uDiffuse;
    pBillboardRenderListD3D[v8].pQuads[0].pos.y = v23;
    v24 = 1.0 - 1.0 / (v6 * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[0].pos.z = v24;
    v25 = 1.0 / v6;
    pBillboardRenderListD3D[v8].pQuads[0].rhw = v25;
    pBillboardRenderListD3D[v8].pQuads[0].texcoord.x = 0.0;
    pBillboardRenderListD3D[v8].pQuads[0].texcoord.y = 0.0;

    v26 = (double)(a2->uScreenSpaceX - 12) - v13;
    v27 = (double)a2->uScreenSpaceY - v15;
    v28 = stru_5C6E00->Cos(angle);
    v29 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v30 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v31 = stru_5C6E00->Cos(angle);
    pBillboardRenderListD3D[v8].pQuads[1].pos.x = (((double)(unsigned __int16)v28 * 0.000015259022
                                                     + (double)(v28 >> 16)) * v26
                                                     - ((double)(unsigned __int16)v29 * 0.000015259022
                                                     + (double)(v29 >> 16)) * v27)
                                                     * v11 + v13;
    v32 = (((double)(unsigned __int16)v31 * 0.000015259022 + (double)(v31 >> 16)) * v27
         + ((double)(unsigned __int16)v30 * 0.000015259022 + (double)(v30 >> 16)) * v26
         - 12.0)
        * v11
        + (double)a2->uScreenSpaceY;
    pBillboardRenderListD3D[v8].pQuads[1].pos.z = v24;
    pBillboardRenderListD3D[v8].pQuads[1].pos.y = v32;
    pBillboardRenderListD3D[v8].pQuads[1].specular = 0;
    pBillboardRenderListD3D[v8].pQuads[1].rhw = v25;
    pBillboardRenderListD3D[v8].pQuads[1].diffuse = uDiffuse;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.y = 1.0;

    v33 = (double)(a2->uScreenSpaceX + 12) - v13;
    v34 = (double)a2->uScreenSpaceY - v15;
    v35 = stru_5C6E00->Cos(angle);
    v36 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v37 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v38 = stru_5C6E00->Cos(angle);
    pBillboardRenderListD3D[v8].pQuads[2].pos.x = (((double)(unsigned __int16)v35 * 0.000015259022
                                                     + (double)(v35 >> 16)) * v33
                                                     - ((double)(unsigned __int16)v36 * 0.000015259022
                                                     + (double)(v36 >> 16)) * v34)
                                                     * v11 + v13;
    v39 = (((double)(unsigned __int16)v38 * 0.000015259022 + (double)(v38 >> 16)) * v34
         + ((double)(unsigned __int16)v37 * 0.000015259022 + (double)(v37 >> 16)) * v33
         - 12.0)
        * v11
        + (double)a2->uScreenSpaceY;
    pBillboardRenderListD3D[v8].pQuads[2].specular = 0;
    pBillboardRenderListD3D[v8].pQuads[2].pos.z = v24;
    pBillboardRenderListD3D[v8].pQuads[2].rhw = v25;
    pBillboardRenderListD3D[v8].pQuads[2].diffuse = uDiffuse;
    pBillboardRenderListD3D[v8].pQuads[2].pos.y = v39;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.y = 1.0;

    v40 = (double)(a2->uScreenSpaceX + 12) - v13;
    v41 = (double)(a2->uScreenSpaceY - 25) - v15;
    v42 = stru_5C6E00->Cos(angle);
    v43 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v44 = stru_5C6E00->Sin(v19 + stru_5C6E00->uIntegerHalfPi);
    v45 = stru_5C6E00->Cos(angle);
    pBillboardRenderListD3D[v8].pQuads[3].pos.x = (((double)(unsigned __int16)v42 * 0.000015259022
                                                     + (double)(v42 >> 16)) * v40
                                                     - ((double)(unsigned __int16)v43 * 0.000015259022
                                                     + (double)(v43 >> 16)) * v41)
                                                     * v11 + v13;
    v46 = (((double)(unsigned __int16)v45 * 0.000015259022 + (double)(v45 >> 16)) * v41
         + ((double)(unsigned __int16)v44 * 0.000015259022 + (double)(v44 >> 16)) * v40
         - 12.0)
        * v11
        + (double)a2->uScreenSpaceY;
    pBillboardRenderListD3D[v8].pQuads[3].specular = 0;
    pBillboardRenderListD3D[v8].pQuads[3].pos.z = v24;
    pBillboardRenderListD3D[v8].pQuads[3].rhw = v25;
    pBillboardRenderListD3D[v8].pQuads[3].diffuse = uDiffuse;
    pBillboardRenderListD3D[v8].pTexture = a3;
    pBillboardRenderListD3D[v8].z_order = v6;
    pBillboardRenderListD3D[v8].uNumVertices = 4;
    pBillboardRenderListD3D[v8].pQuads[3].pos.y = v46;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.y = 0.0;
  }
}

//----- (004A4023) --------------------------------------------------------
void Render::TransformBillboard(RenderBillboardTransform_local0 *a2, Sprite *pSprite, int dimming_level, RenderBillboard *pBillboard)
{
  unsigned int v8; // esi@2
  double v14; // st6@14
  double v15; // st5@14
  float v29; // [sp+28h] [bp-8h]@5
  float v30; // [sp+2Ch] [bp-4h]@5

  if (!uNumD3DSceneBegins)
    return;

  v8 = Billboard_ProbablyAddToListAndSortByZOrder(a2->zbuffer_depth);

  v30 = (a2->_screenspace_x_scaler_packedfloat & 0xFFFF) / 65530.0 + HIWORD(a2->_screenspace_x_scaler_packedfloat);
  v29 = (a2->_screenspace_y_scaler_packedfloat & 0xFFFF) / 65530.0 + HIWORD(a2->_screenspace_y_scaler_packedfloat);

  unsigned int diffuse = ::GetActorTintColor(dimming_level, 0, a2->zbuffer_depth, 0, pBillboard);
  if (a2->sTintColor & 0x00FFFFFF && bTinting)
  {
    diffuse = BlendColors(a2->sTintColor, diffuse);
    if (a2->sTintColor & 0xFF000000)
      diffuse = 0x007F7F7F & ((unsigned int)diffuse >> 1);
  }

  unsigned int specular = 0;
  if (bUsingSpecular)
    specular = sub_47C3D7_get_fog_specular(0, 0, a2->zbuffer_depth);

  v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
  v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
  if (a2->uFlags & 4)
    v14 *= -1.0;
  pBillboardRenderListD3D[v8].pQuads[0].diffuse = diffuse;
  pBillboardRenderListD3D[v8].pQuads[0].pos.x = (double)a2->uScreenSpaceX - v14 * v30;
  pBillboardRenderListD3D[v8].pQuads[0].pos.y = (double)a2->uScreenSpaceY - v15 * v29;
  pBillboardRenderListD3D[v8].pQuads[0].pos.z = 1.0 - 1.0 / (a2->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
  pBillboardRenderListD3D[v8].pQuads[0].rhw = 1.0 / a2->zbuffer_depth;
  pBillboardRenderListD3D[v8].pQuads[0].specular = specular;
  pBillboardRenderListD3D[v8].pQuads[0].texcoord.x = 0.0;
  pBillboardRenderListD3D[v8].pQuads[0].texcoord.y = 0.0;

    v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
    if ( a2->uFlags & 4 )
      v14 = v14 * -1.0;
    pBillboardRenderListD3D[v8].pQuads[1].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[1].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[1].pos.x = (double)a2->uScreenSpaceX - v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[1].pos.y = (double)a2->uScreenSpaceY - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[1].pos.z = 1.0 - 1.0 / (a2->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[1].rhw = 1.0 / a2->zbuffer_depth;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v8].pQuads[1].texcoord.y = 1.0;

    v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
    if ( a2->uFlags & 4 )
      v14 *= -1.0;
    pBillboardRenderListD3D[v8].pQuads[2].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[2].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[2].pos.x = (double)a2->uScreenSpaceX + v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[2].pos.y = (double)a2->uScreenSpaceY - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[2].pos.z = 1.0 - 1.0 / (a2->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[2].rhw = 1.0 / a2->zbuffer_depth;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[2].texcoord.y = 1.0;

    v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
    v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
    if ( a2->uFlags & 4 )
      v14 *= -1.0;
    pBillboardRenderListD3D[v8].pQuads[3].diffuse = diffuse;
    pBillboardRenderListD3D[v8].pQuads[3].specular = specular;
    pBillboardRenderListD3D[v8].pQuads[3].pos.x = (double)a2->uScreenSpaceX + v14 * v30;
    pBillboardRenderListD3D[v8].pQuads[3].pos.y = (double)a2->uScreenSpaceY - v15 * v29;
    pBillboardRenderListD3D[v8].pQuads[3].pos.z = 1.0 - 1.0 / (a2->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
    pBillboardRenderListD3D[v8].pQuads[3].rhw = 1.0 / a2->zbuffer_depth;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v8].pQuads[3].texcoord.y = 0.0;

  pBillboardRenderListD3D[v8].uNumVertices = 4;
  pBillboardRenderListD3D[v8].pTexture = pSprite->pTexture;
  pBillboardRenderListD3D[v8].z_order = a2->zbuffer_depth;
  pBillboardRenderListD3D[v8].field_90 = a2->field_44;
  pBillboardRenderListD3D[v8].sZValue = a2->sZValue;
  pBillboardRenderListD3D[v8].sParentBillboardID = a2->sParentBillboardID;

  if (a2->sTintColor & 0xFF000000)
    pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Opaque_3;
  else
    pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Transparent;
}


//----- (004A49D0) --------------------------------------------------------
void Render::DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, IDirect3DTexture2 *a9)
{
  int absXDifference; // eax@1
  int absYDifference; // eax@1
  unsigned int smallerabsdiff; // ebx@1
  unsigned int largerabsdiff;
  double v16; // st7@7
  double v17; // st7@7
  double v18; // st6@7
  double v20; // st4@8
  double v21; // st4@10
  double v22; // st4@10
  double v23; // st4@10
  double v25; // st4@11
  double v26; // st4@13
  double v28; // st4@13
  RenderVertexD3D3 v29[4]; // [sp+0h] [bp-94h]@7
  int xDifference; // [sp+88h] [bp-Ch]@1
  signed int v32; // [sp+8Ch] [bp-8h]@1
  int yDifference; // [sp+90h] [bp-4h]@1

  xDifference = bankersRounding(dstX - srcX);
  yDifference = bankersRounding(dstY - srcY);
  absYDifference = abs(yDifference);
  absXDifference = abs(xDifference);
  smallerabsdiff = min(absXDifference, absYDifference);
  largerabsdiff = max(absXDifference, absYDifference);
  v32 = (11 * smallerabsdiff >> 5) + largerabsdiff;
  v16 = 1.0 / (double)v32;
  v17 = (double)yDifference * v16 * a4;
  v18 = (double)xDifference * v16 * a4;
  if ( uCurrentlyLoadedLevelType == LEVEL_Outdoor )
  {
    v20 = a3 * 1000.0 / (double)pODMRenderParams->shading_dist_mist;
    v25 = a7 * 1000.0 / (double)pODMRenderParams->shading_dist_mist;
  }
  else
  {
    v20 = a3 * 0.061758894;
    v25 = a7 * 0.061758894;
  }
  v21 = 1.0 / a3;
  v22 = (double)yDifference * v16 * a8;
  v23 = (double)xDifference * v16 * a8;
  v26 = 1.0 - 1.0 / v25;
  v28 = 1.0 / a7;
  v29[0].pos.x = srcX + v17;
  v29[0].pos.y = srcY - v18;
  v29[0].pos.z = 1.0 - 1.0 / v20;
  v29[0].rhw = v21;
  v29[0].diffuse = -1;
  v29[0].specular = 0;
  v29[0].texcoord.x = 1.0;
  v29[0].texcoord.y = 0.0;

  v29[1].pos.x = v22 + dstX;
  v29[1].pos.y = dstY - v23;
  v29[1].pos.z = v26;
  v29[1].rhw = v28;
  v29[1].diffuse = -16711936;
  v29[1].specular = 0;
  v29[1].texcoord.x = 1.0;
  v29[1].texcoord.y = 1.0;

  v29[2].pos.x = dstX - v22;
  v29[2].pos.y = v23 + dstY;
  v29[2].pos.z = v26;
  v29[2].rhw = v28;
  v29[2].diffuse = -1;
  v29[2].specular = 0;
  v29[2].texcoord.x = 0.0;
  v29[2].texcoord.y = 1.0;

  v29[3].pos.x = srcX - v17;
  v29[3].pos.y = v18 + srcY;
  v29[3].pos.z = v29[0].pos.z;
  v29[3].rhw = v21;
  v29[3].diffuse = -1;
  v29[3].specular = 0;
  v29[3].texcoord.x = 0.0;
  v29[3].texcoord.y = 0.0;
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, a9));
  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, v29, 4, 24));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));
}

//----- (004A4CC9) --------------------------------------------------------
void Render::_4A4CC9_AddSomeBillboard(stru6_stru1_indoor_sw_billboard *a1, int diffuse)
{
  unsigned int v5; // eax@7
  double v10; // st6@9
  double v11; // st6@10
  int v12; // ebx@13

  if (a1->uNumVertices < 3)
    return;

  float depth = 1000000.0;
  for (uint i = 0; i < (unsigned int)a1->uNumVertices; ++i)
  {
    if (a1->field_104[i].z < depth)
      depth = a1->field_104[i * 4].z;
  }

  v5 = Billboard_ProbablyAddToListAndSortByZOrder(depth);
  pBillboardRenderListD3D[v5].field_90 = 0;
  pBillboardRenderListD3D[v5].sParentBillboardID = -1;
  pBillboardRenderListD3D[v5].opacity = RenderBillboardD3D::Opaque_2;
  pBillboardRenderListD3D[v5].pTexture = 0;
  pBillboardRenderListD3D[v5].uNumVertices = a1->uNumVertices;
  pBillboardRenderListD3D[v5].z_order = depth;

  for (uint i = 0; i < (unsigned int)a1->uNumVertices; ++i)
  {
    pBillboardRenderListD3D[v5].pQuads[i].pos.x = a1->field_104[i].x;
    pBillboardRenderListD3D[v5].pQuads[i].pos.y = a1->field_104[i].y;

    v10 = a1->field_104[i].z;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
      v11 = v10 * 0.061758894;
    else
      v11 = v10 * 1000.0 / (double)pODMRenderParams->shading_dist_mist;
    pBillboardRenderListD3D[v5].pQuads[i].pos.z = 1.0 - 1.0 / v11;
    pBillboardRenderListD3D[v5].pQuads[i].rhw = 1.0 / a1->field_104[i].z;

    if (diffuse & 0xFF000000)
      v12 = a1->field_104[i].diffuse;
    else
      v12 = diffuse;
    pBillboardRenderListD3D[v5].pQuads[i].diffuse = v12;
    pBillboardRenderListD3D[v5].pQuads[i].specular = 0;

    pBillboardRenderListD3D[v5].pQuads[i].texcoord.x = 0.0;
    pBillboardRenderListD3D[v5].pQuads[i].texcoord.y = 0.0;
  }
}

//----- (004A4DE1) --------------------------------------------------------
bool Render::LoadTexture(const char *pName, unsigned int bMipMaps, IDirectDrawSurface4 **pOutSurface, IDirect3DTexture2 **pOutTexture)
{
  unsigned __int16 *v13; // ecx@19
  unsigned __int16 *v14; // eax@19
  DWORD v15; // edx@20
  stru350 Dst; // [sp+Ch] [bp-F8h]@12

  HWLTexture* pHWLTexture = pD3DBitmaps.LoadTexture(pName, bMipMaps);
  if (!pHWLTexture)
    return false;

    bMipMaps = !strncmp(pName, "HDWTR", 5);
    if ( !pRenderD3D->CreateTexture(pHWLTexture->uWidth, pHWLTexture->uHeight, pOutSurface, pOutTexture, true,
            bMipMaps, uMinDeviceTextureDim) )
      Error("HiScreen16::LoadTexture - D3Drend->CreateTexture() failed: %x", 0);
    if (bMipMaps)
    {
      Dst._450DDE();
      Dst._450DF1(&stru_4EFCBC, &stru_4EFCBC);

      IDirectDrawSurface4 *pNextSurf = *pOutSurface;
      while ( 1 )
      {
        DDSCAPS2 v19;
        memset(&v19, 0, sizeof(DDSCAPS2));
        v19.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

        DDSURFACEDESC2 desc;
        memset(&desc, 0, sizeof(DDSURFACEDESC2));
        desc.dwSize = sizeof(DDSURFACEDESC2);

        if ( LockSurface_DDraw4(pNextSurf, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY) )
        {
			// linear scaling
		  for (int s = 0; s < desc.dwHeight; ++s)
			  for (int t = 0; t < desc.dwWidth; ++t)
			  {
				  unsigned int resampled_x = t * pHWLTexture->uWidth / desc.dwWidth,
					           resampled_y = s * pHWLTexture->uHeight / desc.dwHeight;
				  unsigned short sample = pHWLTexture->pPixels[resampled_y * pHWLTexture->uWidth + resampled_x];

				  ((unsigned short *)desc.lpSurface)[s * (desc.lPitch >> 1) + t] = sample;
			  }

			
			  //bicubic sampling
          //Dst.sub_451007_scale_image_bicubic(pHWLTexture->pPixels, pHWLTexture->uWidth, pHWLTexture->uHeight, pHWLTexture->uWidth,
          //  (unsigned short *)desc.lpSurface, desc.dwWidth, desc.dwHeight, desc.lPitch >> 1, 0, 0);

          ErrD3D(pNextSurf->Unlock(NULL));
          //bMipMaps = 0x4D86ACu;
        }
        if (FAILED(pNextSurf->GetAttachedSurface(&v19, &pNextSurf)))
          break;
      }
      //v20 = -1;
      //nullsub_1();
    }
    else
    {
        DDSCAPS2 v19;
        memset(&v19, 0, sizeof(DDSCAPS2));
        v19.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

        DDSURFACEDESC2 desc;
        memset(&desc, 0, sizeof(DDSURFACEDESC2));
        desc.dwSize = sizeof(DDSURFACEDESC2);

      if ( LockSurface_DDraw4(*pOutSurface, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY) )
      {
        bMipMaps = 0;
        v13 = pHWLTexture->pPixels;
        v14 = (unsigned __int16 *)desc.lpSurface;
        for(uint bMipMaps = 0; bMipMaps < desc.dwHeight; bMipMaps++)
        {
          for (v15 = 0; v15 < desc.dwWidth; v15++)
          {
            *v14 = *v13;
            ++v14;
            ++v13;
          }
          v14 += (desc.lPitch >> 1) - desc.dwWidth;
        }
        ErrD3D((*pOutSurface)->Unlock(NULL));
      }
    }
    delete [] pHWLTexture->pPixels;
    delete pHWLTexture;
    return true;

}

//----- (004A5048) --------------------------------------------------------
bool Render::MoveSpriteToDevice( Sprite *pSprite )
{
  HWLTexture *sprite_texture; // eax@1
  unsigned __int16 *v9; // edx@5
  LPVOID v10; // eax@5
  DDSURFACEDESC2 Dst; // [sp+Ch] [bp-7Ch]@4

  sprite_texture = pD3DSprites.LoadTexture(pSprite->pName, pSprite->uPaletteID);
  if ( sprite_texture )
  {
    pSprite->uAreaX = sprite_texture->uAreaX;
    pSprite->uAreaY = sprite_texture->uAreaY;
    pSprite->uBufferWidth = sprite_texture->uBufferWidth;
    pSprite->uBufferHeight = sprite_texture->uBufferHeight;
    pSprite->uAreaWidth = sprite_texture->uAreaWidth;
    pSprite->uAreaHeight = sprite_texture->uAreaHeigth;
    if (!pRenderD3D->CreateTexture(sprite_texture->uWidth, sprite_texture->uHeight, &pSprite->pTextureSurface, &pSprite->pTexture, 1u, 0, uMinDeviceTextureDim))
      Error("HiScreen16::LoadTexture - D3Drend->CreateTexture() failed: %x", 0);
    memset(&Dst, 0, sizeof(DDSURFACEDESC2));
    Dst.dwSize = 124;
    if ( LockSurface_DDraw4((IDirectDrawSurface4 *)pSprite->pTextureSurface, &Dst, DDLOCK_WAIT | DDLOCK_WRITEONLY) )
    {
      v9 = sprite_texture->pPixels;
      v10 = Dst.lpSurface;
      for (uint i=0; i<sprite_texture->uHeight; ++i)
      {
        for (uint j=0; j<sprite_texture->uWidth/2; ++j)
        {
          *(int *)v10 = *(int *)v9;
          v9 += 2;
          v10 = (char *)v10 + 4;
        }
        v10 = (char *)v10 + Dst.lPitch-sprite_texture->uWidth*2;
      }
      ErrD3D(pSprite->pTextureSurface->Unlock(NULL));
    }
    delete [] sprite_texture->pPixels;
    delete sprite_texture;
	return true;
  }
  return false;
}

//----- (004A51CB) --------------------------------------------------------
void Render::BeginScene()
{
  //Render *v1; // esi@1
  unsigned int v2; // eax@1
/*int v3; // eax@5
  unsigned __int16 **v4; // edi@6
  char *v5; // ebx@7*/
//  DDSURFACEDESC2 Dst; // [sp+Ch] [bp-7Ch]@4

  //v1 = this;
  v2 = this->uNumSceneBegins;
  this->uNumSceneBegins = v2 + 1;
  if ( !v2 )
  {
    if ( this->pRenderD3D )
    {
      /*if ( this->bColorKeySupported )
      {
        memset(&Dst, 0, 0x7Cu);
        Dst.dwSize = 124;
        if ( LockSurface_DDraw4(this->pColorKeySurface4, &Dst, 0x800 | DDLOCK_WAIT) )
        {
          this->pTargetSurface = (unsigned __int16 *)Dst.lpSurface;
          this->uTargetSurfacePitch = Dst.lPitch >> 1;
          this->field_18_locked_pitch = Dst.lPitch >> 1;
        }
        --this->uNumSceneBegins;
      }*/
    }
    else
    {
      if ( !this->pTargetSurface )
      {
        LockRenderSurface((void **)&this->pTargetSurface, &this->uTargetSurfacePitch);
        /*if ( this->pTargetSurface )
        {
          this->field_18_locked_pitch = this->uTargetSurfacePitch;
        }*/
        --this->uNumSceneBegins;
      }
    }
    RestoreFrontBuffer();
  }
}

//----- (004A527D) --------------------------------------------------------
void Render::EndScene()
{
  if ( this->uNumSceneBegins )
  {
    this->uNumSceneBegins--;
    if ( !this->uNumSceneBegins )
    {
      if ( this->pRenderD3D )
      {
        /*if ( this->bColorKeySupported )
        {
          this->pTargetSurface = 0;
          this->uTargetSurfacePitch = 0;
          this->field_18_locked_pitch = 0;
          ErrD3D(this->pColorKeySurface4->Unlock(NULL));
        }*/
      }
      else
      {
        this->pTargetSurface = 0;
        this->uTargetSurfacePitch = 0;
        //this->field_18_locked_pitch = 0;
        UnlockBackBuffer();
      }
    }
  }
}

//----- (004A52F1) --------------------------------------------------------
void Render::ScreenFade(unsigned int color, float t)
{
  unsigned int v3; // esi@1
  unsigned int v7; // eax@6
  RenderVertexD3D3 v36[4]; // [sp+Ch] [bp-94h]@6
  int v40; // [sp+9Ch] [bp-4h]@6

  v3 = 0;

  //{
  if (t > 1.0f)
    t = 1.0f;
  else if (t < 0.0f)
    t = 0.0f;

    v40 = (char)floorf(t * 255.0f + 0.5f);
    
    v7 = color | (v40 << 24);

    v36[0].specular = 0;
	v36[0].pos.x = pViewport->uViewportTL_X;
    v36[0].pos.y = (double)pViewport->uViewportTL_Y;
    v36[0].pos.z = 0.0;
    v36[0].diffuse = v7;
    v36[0].rhw = 1.0;
    v36[0].texcoord.x = 0.0;
    v36[0].texcoord.y = 0.0;

    v36[1].specular = 0;
    v36[1].pos.x = pViewport->uViewportTL_X;
    v36[1].pos.y = (double)(pViewport->uViewportBR_Y + 1);
    v36[1].pos.z = 0.0;
    v36[1].diffuse = v7;
    v36[1].rhw = 1.0;
    v36[1].texcoord.x = 0.0;
    v36[1].texcoord.y = 0.0;

    v36[2].specular = 0;
    v36[2].pos.x = (double)pViewport->uViewportBR_X;
    v36[2].pos.y = (double)(pViewport->uViewportBR_Y + 1);
    v36[2].pos.z = 0.0;
    v36[2].diffuse = v7;
    v36[2].rhw = 1.0;
    v36[2].texcoord.x = 0.0;
    v36[2].texcoord.y = 0.0;

    v36[3].specular = 0;
    v36[3].pos.x = (double)pViewport->uViewportBR_X;
    v36[3].pos.y = (double)pViewport->uViewportTL_Y;
    v36[3].pos.z = 0.0;
    v36[3].diffuse = v7;
    v36[3].rhw = 1.0;
    v36[3].texcoord.x = 0.0;
    v36[3].texcoord.y = 0.0;

    ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
      D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, v36, 4, 28));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));
  /*}
  else
  {
    v40 = (1.0 - a3) * 65536.0;
    v39 = v40 + 6.7553994e15;
    LODWORD(a3) = LODWORD(v39);
    v38 = (signed int)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) >> 1;
    HIDWORD(v39) = pViewport->uViewportBR_Y - pViewport->uViewportTL_Y + 1;
    v13 = pViewport->uViewportTL_X + ecx0->uTargetSurfacePitch - pViewport->uViewportBR_X;
    v14 = &ecx0->pTargetSurface[pViewport->uViewportTL_X + pViewport->uViewportTL_Y * ecx0->uTargetSurfacePitch];
    v37 = 2 * v13;
    LODWORD(v40) = (int)v14;

    int __i = 0;
    v15 = dword_F1B430.data();
    do
    {
      v16 = v3;
      v3 += LODWORD(a3);
      dword_F1B430[__i++] = v16 >> 16;
    }
    //while ( (signed int)v15 < (signed int)&Aureal3D_SplashScreen );
    while (__i < 32);

    if ( render->uTargetGBits == 6 )
    {
      v17 = sr_42690D_colors_cvt(this_);
      v18 = (65536 - LODWORD(a3)) * (v17 & 0x1F);
      this_ = (((65536 - LODWORD(a3)) * (unsigned __int16)(v17 & 0xF800) & 0xF800FFFF | v18 & 0x1F0000 | (65536 - LODWORD(a3)) * (v17 & 0x7E0) & 0x7E00000u) >> 16 << 16) | (((65536 - LODWORD(a3)) * (unsigned __int16)(v17 & 0xF800) & 0xF800FFFF | v18 & 0x1F0000 | (65536 - LODWORD(a3)) * (v17 & 0x7E0) & 0x7E00000u) >> 16);
      v19 = v40;
      v20 = off_4EFDB0;
      v21 = HIDWORD(v39);
      do
      {
        v22 = v38;
        v31 = v21;
        do
        {
          v23 = (*(int *)((char *)v20
                           + ((((unsigned __int16)(*(short *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | *(unsigned int *)LODWORD(v19) & 0x7FF) & 0x7C0u) >> 4)) << 6) | (*(int *)((char *)v20 + ((((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0x7C00000u) >> 20)) << 22) | ((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0xF81FF81F;
          result = this_
                 + (*((int *)v20
                    + (((unsigned __int8)(*((char *)v20
                                          + ((((unsigned __int16)(*(short *)((char *)v20
                                                                           + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | *(unsigned int *)LODWORD(v19) & 0x7FF) & 0x7C0u) >> 4)) << 6) | *(unsigned int *)LODWORD(v19) & 0x1F) & 0x1F)) | (*(int *)((char *)v20 + ((v23 & 0x1F0000u) >> 14)) << 16) | ((*(int *)((char *)v20 + ((((unsigned __int16)(*(short *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | *(unsigned int *)LODWORD(v19) & 0x7FF) & 0x7C0u) >> 4)) << 6) | (*(int *)((char *)v20 + ((((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0x7C00000u) >> 20)) << 22) | ((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0xF81FF81F) & 0xFFE0FFE0);
          *(unsigned int *)LODWORD(v19) = result;
          LODWORD(v19) += 4;
          --v22;
        }
        while ( v22 );
        LODWORD(v19) += v37;
        v21 = v31 - 1;
      }
      while ( v31 != 1 );
    }
    else
    {
      v24 = sr_4268E3_smthn_to_a1r5g5b5(this_);
      v25 = (65536 - LODWORD(a3)) * (v24 & 0x1F);
      this_ = (((65536 - LODWORD(a3)) * (v24 & 0x7C00) & 0x7C000000 | v25 & 0x1F0000 | (65536 - LODWORD(a3))
                                                                                    * (v24 & 0x3E0) & 0x3E00000u) >> 16 << 16) | (((65536 - LODWORD(a3)) * (v24 & 0x7C00) & 0x7C000000 | v25 & 0x1F0000 | (65536 - LODWORD(a3)) * (v24 & 0x3E0) & 0x3E00000u) >> 16);
      v26 = v40;
      v27 = (char *)off_4EFDB0;
      v28 = HIDWORD(v39);
      do
      {
        v29 = v38;
        v32 = v28;
        do
        {
          v30 = 32
              * *(int *)&v27[(((unsigned __int16)(*(short *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | *(unsigned int *)LODWORD(v26) & 0x3FF) & 0x3E0u) >> 3] | (*(int *)&v27[(((*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x3E00000u) >> 19] << 21) | ((*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x7C1F7C1F;
          result = this_
                 + (*(int *)&v27[4
                                  * (((unsigned __int8)(32
                                                      * v27[(((unsigned __int16)(*(short *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | *(unsigned int *)LODWORD(v26) & 0x3FF) & 0x3E0u) >> 3]) | *(unsigned int *)LODWORD(v26) & 0x1F) & 0x1F)] | (*(int *)&v27[(v30 & 0x1F0000u) >> 14] << 16) | (32 * *(int *)&v27[(((unsigned __int16)(*(short *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | *(unsigned int *)LODWORD(v26) & 0x3FF) & 0x3E0u) >> 3] | (*(int *)&v27[(((*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x3E00000u) >> 19] << 21) | ((*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x7C1F7C1F) & 0xFFE0FFE0);
          *(unsigned int *)LODWORD(v26) = result;
          LODWORD(v26) += 4;
          --v29;
        }
        while ( v29 );
        LODWORD(v26) += v37;
        v28 = v32 - 1;
      }
      while ( v32 != 1 );
    }
  }*/
}

//----- (004A5B81) --------------------------------------------------------
void Render::SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW)
{
  this->bClip = 1;
  this->uClipX = uX;
  this->uClipY = uY;
  this->uClipZ = uZ;
  this->uClipW = uW;
}

//----- (004A5BB6) --------------------------------------------------------
void Render::ResetUIClipRect()
{
  this->bClip = 1;
  this->uClipX = 0;
  this->uClipY = 0;
  this->uClipZ = window->GetWidth();
  this->uClipW = 480;
}

unsigned __int32 Color32(unsigned __int16 color16)
{
  unsigned __int32 c = color16;
  unsigned int b = (c & 31) * 8;
  unsigned int g = ((c >> 5) & 63) * 4;
  unsigned int r = ((c >> 11) & 31) * 8;

  return (r << 16) | (g << 8) | b;
}

unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16)
{
  unsigned __int32 c = color16;
  unsigned int b = (c & 31) * 8;
  unsigned int g = ((c >> 5) & 63) * 4;
  unsigned int r = ((c >> 11) & 31) * 8;

  return (b << 16) | (g << 8) | r;
}

//----- (0040DEF3) --------------------------------------------------------
unsigned __int16 Color16(unsigned __int32 r, unsigned __int32 g, unsigned __int32 b)
{
  //return ((unsigned int)b >> (8 - LOBYTE(render->uTargetBBits))) | render->uTargetGMask & (g << (LOBYTE(render->uTargetGBits) + 
  //                         LOBYTE(render->uTargetBBits) - 8)) | render->uTargetRMask & (r << (LOBYTE(render->uTargetGBits) + 
  //                         LOBYTE(render->uTargetRBits) + LOBYTE(render->uTargetBBits) - 8));
  return (b >> (8 - 5)) |
         0x7E0 & (g << (6 + 5 - 8)) |
         0xF800 & (r << (6 + 5 + 5 - 8));
}



void Render::DrawTextureCustomHeight(float u, float v, class Image *img, int custom_height)
{
    unsigned __int16 *v6; // esi@3
    unsigned int v8; // eax@5
    unsigned int v11; // eax@7
    unsigned int v12; // ebx@8
    unsigned int v15; // eax@14
    int v19; // [sp+10h] [bp-8h]@3

    if (!img)
        return;

    unsigned int uOutX = 640 * u;
    unsigned int uOutY = 480 * v;

    int width = img->GetWidth();
    int height = min(img->GetHeight(), custom_height);
    v6 = (unsigned __int16 *)img->GetPixels(IMAGE_FORMAT_R5G6B5);

    //v5 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v19 = width;
    if (this->bClip)
    {
        if ((signed int)uOutX < (signed int)this->uClipX)
        {
            v8 = this->uClipX - uOutX;
            unsigned int v9 = uOutX - this->uClipX;
            v8 *= 2;
            width += v9;
            v6 = (unsigned __int16 *)((char *)v6 + v8);
            //v5 = (unsigned __int16 *)((char *)v5 + v8);
        }
        if ((signed int)uOutY < (signed int)this->uClipY)
        {
            v11 = this->uClipY - uOutY;
            v6 += v19 * v11;
            height += uOutY - this->uClipY;
            //v5 += this->uTargetSurfacePitch * v11;
        }
        v12 = max(this->uClipX, uOutX);
        if ((signed int)(width + v12) >(signed int)this->uClipZ)
        {
            width = this->uClipZ - max(this->uClipX, uOutX);
        }
        v15 = max(this->uClipY, uOutY);
        if ((signed int)(v15 + height) > (signed int)this->uClipW)
        {
            height = this->uClipW - max(this->uClipY, uOutY);
        }
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            WritePixel16(uOutX + x, uOutY + y, *v6);
            //*v5 = *v6;
            //++v5;
            ++v6;
        }
        v6 += v19 - width;
        //v5 += this->uTargetSurfacePitch - v4;
    }
}

void Render::DrawTextureNew(float u, float v, Image *bmp)
{
    DrawTextureCustomHeight(u, v, bmp, bmp->GetHeight());
}


//----- (004A5D33) --------------------------------------------------------
void Render::DrawTextureOffset(int out_x, int out_y, int offset_x, int offset_y, Image *pTexture)
{
    unsigned __int16 *pTexturea; // [sp+28h] [bp+18h]@3

    if (this->uNumSceneBegins && pTexture)
    {
        int draw_width = pTexture->GetWidth() - offset_x;
        int draw_height = pTexture->GetHeight() - offset_y;
        pTexturea = (unsigned __int16 *)pTexture->GetPixels(IMAGE_FORMAT_R5G6B5) + offset_x + offset_y * pTexture->GetWidth();
        if (this->bClip)
        {
            if (out_x < this->uClipX)//если кадр выходит за правую границу
            {
                pTexturea += (this->uClipX - out_x);
                draw_width += out_x - this->uClipX;
            }
            if (out_y < this->uClipY)//если кадр выходит за верхнюю границу
            {
                pTexturea += pTexture->GetWidth() * (this->uClipY - out_y);
                draw_height += out_y - this->uClipY;
            }
            if (this->uClipX < out_x)//если правая граница окна меньше х координаты кадра
                this->uClipX = out_x;
            if (this->uClipY < out_y)//если верхняя граница окна меньше y координаты кадра
                this->uClipY = out_y;
            if ((draw_width + this->uClipX) > this->uClipZ)//если ширина кадра выходит за правую границу
            {
                if (this->uClipX < out_x)
                    this->uClipX = out_x;
                draw_width = this->uClipZ - this->uClipX;
            }
            if ((draw_height + this->uClipY) > this->uClipW)//если высота кадра выходит за нижнюю границу
            {
                if (this->uClipY < out_y)
                    this->uClipY = out_y;
                draw_height = this->uClipW - this->uClipY;
            }
        }

        for (int y = 0; y < draw_height; ++y)
        {
            for (int x = 0; x < draw_width; ++x)
            {
                if (*pTexturea != Color16(0, 0xFF, 0xFF))
                {
                    WritePixel16(out_x + x, out_y + y, *pTexturea);
                }
                ++pTexturea;
            }
            pTexturea += (pTexture->GetWidth() - draw_width);
        }
    }
}

//----- (004A6E7E) --------------------------------------------------------
void Render::DrawTextureGrayShade(float u, float v, Image *img)
{
    DrawMasked(u, v, img, 1, 0x7BEF);
}

//----- (004A6DF5) --------------------------------------------------------
void Render::_4A6DF5(unsigned __int16 *pBitmap, unsigned int uBitmapPitch, Vec2_int_ *pBitmapXY, void *pTarget, unsigned int uTargetPitch, Vec4_int_ *a7)
{
    int width; // ecx@3
    unsigned __int16 *pixels; // ebx@4
    int height; // esi@4

    if (!pBitmap || !pTarget)
        return;

    width = a7->z - a7->x;
    height = a7->w - a7->y;
    pixels = (unsigned short *)pTarget + a7->x + uTargetPitch * a7->y;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            WritePixel16(a7->x + x, a7->y + y, *pixels);
            ++pixels;
        }
        pixels += uTargetPitch - width;
    }
}

//----- (004A6D87) --------------------------------------------------------
void Render::FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, unsigned int uColor16)
{
    if (!uNumSceneBegins)
        return;

    unsigned __int32 twoColors = (uColor16 << 16) | uColor16;
    for (uint y = 0; y < uHeight; ++y)
    {
        void *pDst = (char *)pTargetSurface + (FORCE_16_BITS ? 2 : 4) * (uX + (y + uY) * uTargetSurfacePitch);

        memset32(pDst,
            FORCE_16_BITS ? twoColors : 0xFF000000 | Color32(uColor16),  // two colors per int (16bit) or 1 (32bit)
            uWidth / (FORCE_16_BITS ? 2 : 1)                             // two pixels per int (16bit) or 1 (32bit)
        );

        if (FORCE_16_BITS && uWidth & 1) // we may miss one pixel for 16bit
        {
            ((unsigned __int16 *)pTargetSurface)[uX + uWidth - 1 + (y + uY) * uTargetSurfacePitch] = uColor16;
        }
    }
}

//----- (004A6C4F) --------------------------------------------------------
void Render::DrawText(signed int uOutX, signed int uOutY, unsigned __int8 *pFontPixels, unsigned int uCharWidth, 
                       unsigned int uCharHeight, unsigned __int16 *pFontPalette, 
                       unsigned __int16 uFaceColor, unsigned __int16 uShadowColor)
{
  unsigned int v9; // edi@2
  unsigned int v10; // esi@2
  unsigned int v12; // ebx@3
  //signed int v13; // edx@5
  int v14; // edx@6
  signed int v15; // ebx@7
  //unsigned int v16; // edx@9
  signed int v17; // edi@10
  signed int v18; // ebx@13
  unsigned int v19; // edx@15
  signed int v20; // esi@16
  unsigned __int16 v22; // dx@24
  unsigned __int8 *v24; // [sp+Ch] [bp-4h]@2

  if (!this->uNumSceneBegins)
    return;

    v9 = uCharWidth;
    v10 = uCharHeight;
    //v11 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v24 = pFontPixels;

    int clipped_out_x = uOutX, clipped_out_y = uOutY;
    if ( this->bClip )
    {
      v12 = this->uClipX;
      if ( uOutX < (signed int)v12 )
      {
        v24 = &pFontPixels[v12 - uOutX];
        //v11 += v12 - uOutX;
        clipped_out_x = uClipX;
        v9 = uCharWidth + uOutX - v12;
      }
      //v13 = this->uClipY;
      if ( uOutY < this->uClipY )
      {
        v14 = this->uClipY - uOutY;
        v24 += uCharWidth * v14;
        v10 = uCharHeight + uOutY - this->uClipY;
        //v11 += this->uTargetSurfacePitch * v14;
        clipped_out_y = uClipY;
      }
      v15 = this->uClipX;
      if ( this->uClipX < uOutX )
        v15 = uOutX;
      //v16 = this->uClipZ;
      if ( (signed int)(v9 + v15) > (signed int)this->uClipZ )
      {
        v17 = this->uClipX;
        if ( this->uClipX < uOutX )
          v17 = uOutX;
        v9 = this->uClipZ - v17;
      }
      v18 = this->uClipY;
      if ( this->uClipY < uOutY )
        v18 = uOutY;
      v19 = this->uClipW;
      if ( (signed int)(v10 + v18) > (signed int)v19 )
      {
        v20 = this->uClipY;
        if ( this->uClipY < uOutY )
          v20 = uOutY;
        v10 = v19 - v20;
      }
    }

    for (uint y = 0; y < v10; ++y)
    {
      for (uint x = 0; x < v9; ++x)
      {
        if (*v24)
        {
          v22 = uShadowColor;
          if ( *v24 != 1 )
            v22 = uFaceColor;
           WritePixel16(clipped_out_x + x, clipped_out_y + y, v22);
        }
        ++v24;
      } 
      v24 += uCharWidth - v9;
        //v23 = uOutXa-- == 1;
        //v11 += this->uTargetSurfacePitch - v9;
    }
}

/*
//----- (004A6A68) --------------------------------------------------------
void Render::DrawTextureCustomHeight(unsigned int x, unsigned int y, Texture_MM7 *texture, __int16 height)
{
  Texture_MM7 tex; // [sp+Ch] [bp-48h]@1

  memcpy(&tex, texture, sizeof(tex));
  tex.uTextureHeight = texture->uTextureHeight - height;
  if ( (signed __int16)tex.uTextureHeight > 0 )
      DrawTextureTransparentColorKey(x, y, &tex);
}*/

//----- (004A6AB1) --------------------------------------------------------
void Render::DrawTextAlpha( int x, int y, unsigned char* font_pixels, int a5, unsigned int uFontHeight, unsigned __int16 *pPalette, bool present_time_transparency )
    {
  int v8; // edi@2
  unsigned int v9; // esi@2
  unsigned char *v11; // edx@2
  int v14; // edx@6
  signed int v15; // ebx@7
  signed int v17; // edi@10
  signed int v18; // ebx@13
  signed int v20; // esi@16
  unsigned __int16 v24; // si@35
  int v25; // [sp+Ch] [bp-4h]@2
  unsigned int v28; // [sp+20h] [bp+10h]@30

  int a2 = x;
  int a3 = y;
  uint a6 = uFontHeight;
  if (!this->uNumSceneBegins)
    return;

    v8 = a5;
    v9 = a6;
    //v10 = &pTargetSurface[x + y * uTargetSurfacePitch];
    v11 = (unsigned char *)font_pixels;
    v25 = (int)font_pixels;
    int clipped_out_x = x;
    int clipped_out_y = y;
    if ( this->bClip )
    {
      if ( a2 < (signed int)this->uClipX )
      {
        v25 = this->uClipX - a2 + (int)font_pixels;
        //v10 += v12 - a2;
        v8 = a5 + a2 - this->uClipX;
        clipped_out_x = uClipX;
      }
      if ( a3 < this->uClipY )
      {
        v14 = this->uClipY - a3;
        v25 += a5 * v14;
        v9 = a6 + a3 - this->uClipY;
        //v10 += this->uTargetSurfacePitch * v14;
        clipped_out_y = uClipY;
      }
      v15 = this->uClipX;
      if ( this->uClipX < a2 )
        v15 = a2;
      if ( v8 + v15 > (signed int)this->uClipZ )
      {
        v17 = this->uClipX;
        if ( v17 < a2 )
          v17 = a2;
        v8 = this->uClipZ - v17;
      }
      v18 = this->uClipY;
      if ( this->uClipY < a3 )
        v18 = a3;
      if ( (signed int)(v9 + v18) > (signed int)this->uClipW )
      {
        v20 = this->uClipY;
        if ( this->uClipY < a3 )
          v20 = a3;
        v9 = this->uClipW - v20;
      }
      v11 = (unsigned char *)v25;
    }

    if ( present_time_transparency )
    {
      v28 = 0x7FF; // transparent color 16bit render->uTargetGMask | render->uTargetBMask;
      for (uint dy = 0; dy < v9; ++dy)
      {
        for (int dx = 0; dx < v8; ++dx)
        {
              if ( *v11 )
                v24 = pPalette[*v11];
              else
                v24 = v28;
              WritePixel16(clipped_out_x + dx, clipped_out_y + dy, v24);
              //*v10 = v24;
              //++v10;
              ++v11;
              //--v27;

        }
          v11 += a5 - v8;
      }
      /*if ( (signed int)v9 > 0 )
      {
        v23 = a5;
        v30 = v9;
        do
        {
          if ( v8 > 0 )
          {
            v27 = v8;
            do
            {
              if ( *v11 )
                v24 = pPalette[*v11];
              else
                v24 = v28;
              *v10 = v24;
              ++v10;
              ++v11;
              --v27;
            }
            while ( v27 );
          }
          v11 += v23 - v8;
          v22 = v30-- == 1;
          v10 += this->uTargetSurfacePitch - v8;
        }
        while ( !v22 );
      }*/
    }
    else
    {
      for (uint dy = 0; dy < v9; ++dy)
      {
        for (int dx = 0; dx < v8; ++dx)
        {
            if ( *v11 )       
              WritePixel16(clipped_out_x + dx, clipped_out_y + dy, pPalette[*v11]);
              //*v10 = v24;
              //++v10;
              ++v11;
              //--v27;
        }
          v11 += a5 - v8;
      }

      /*if ( (signed int)v9 > 0 )
      {
        v21 = a5;
        v29 = v9;
        do
        {
          if ( v8 > 0 )
          {
            v26 = v8;
            do
            {
              if ( *v11 )
                *v10 = pPalette[*v11];
              ++v10;
              ++v11;
              --v26;
            }
            while ( v26 );
          }
          v11 += v21 - v8;
          v22 = v29-- == 1;
          v10 += this->uTargetSurfacePitch - v8;
        }
        while ( !v22 );
      }*/
    }
}

//----- (004A68EF) --------------------------------------------------------
void Render::DrawTransparentGreenShade(float u, float v, Image *pTexture)
{
  DrawMasked(u, v, pTexture, 0, 0x07E0);
}


//----- (004A6776) --------------------------------------------------------
void Render::DrawTransparentRedShade(float u, float v, Image *a4)
{
  DrawMasked(u, v, a4, 0, 0xF800);
}

//----- (004A68EF) --------------------------------------------------------
void Render::DrawMasked(float u, float v, Image *pTexture, unsigned int color_dimming_level, unsigned __int16 mask)
{
  unsigned int v5; // ebx@4
  int v10; // edx@8
  signed int v11; // edx@9
  signed int v12; // esi@12
  signed int v13; // esi@15
  signed int v15; // esi@18
  //unsigned __int8 *v16; // ebx@22
  int v18; // [sp+10h] [bp-10h]@4
  //unsigned __int8 *v19; // [sp+18h] [bp-8h]@4
  int v20; // [sp+1Ch] [bp-4h]@4

  if (!uNumSceneBegins || !pTexture)
    return;

  //if ( pTexture->pPalette16 )
  {
    v5 = pTexture->GetHeight();
    //v6 = &this->pTargetSurface[a2 + a3 * this->uTargetSurfacePitch];
    //v19 = pTexture->paletted_pixels;
    auto pixels = (unsigned __int32 *)pTexture->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    v18 = v20 = pTexture->GetWidth();

    int a2 = u * window->GetWidth();
    int a3 = v * window->GetHeight();

    int clipped_out_x = a2;
    int clipped_out_y = a3;
    if ( this->bClip )
    {
      if ( a2 < this->uClipX )
      {
          pixels += this->uClipX - a2;
        v20 += a2 - this->uClipX;
        clipped_out_x = uClipX;
      }
      v5 = pTexture->GetHeight();
      if ( a3 < this->uClipY )
      {
        v10 = this->uClipY - a3;
        pixels += v18 * v10;
        v5 = a3 - this->uClipY + pTexture->GetHeight();
        clipped_out_y = uClipY;
      }
      v11 = this->uClipX;
      if ( this->uClipX < a2 )
        v11 = a2;
      if ( v11 + v20 > (signed int)this->uClipZ )
      {
        v12 = this->uClipX;
        if ( this->uClipX < a2 )
          v12 = a2;
        v20 = this->uClipZ - v12;
      }
      v13 = this->uClipY;
      if ( this->uClipY < a3 )
        v13 = a3;
      if ( (signed int)(v5 + v13) > (signed int)this->uClipW )
      {
        v15 = this->uClipY;
        if ( this->uClipY < a3 )
          v15 = a3;
        v5 = this->uClipW - v15;
      }
    }
        
    /*v16 = v19;
    for (uint y = 0; y < v5; ++y)
    {
      for (int x = 0; x < v20; ++x)
      {
        if ( *v16 )
          WritePixel16(clipped_out_x + x, clipped_out_y + y, pTexture->pPalette16[*v16] & mask);
        ++v16;
      }
      v16 += v18 - v20;
    }*/

    for (uint y = 0; y < v5; ++y)
    {
        for (int x = 0; x < v20; ++x)
        {
            if (*pixels & 0xFF000000)
                WritePixel16(
                    clipped_out_x + x,
                    clipped_out_y + y,
                    (Color16((*pixels >> 16) & 0xFF, (*pixels >> 8) & 0xFF, *pixels & 0xFF) >> color_dimming_level) & mask
                );
            ++pixels;
        }
        pixels += v18 - v20;
    }
  }
}


//----- (004A65CC) --------------------------------------------------------
void Render::_4A65CC(unsigned int x, unsigned int y, Image *a4, Image *a5, int a6, int a7, int a8)
{
    unsigned int uHeight; // edi@6
    unsigned int v14; // edx@11
    unsigned int v16; // edx@14
    unsigned int v17; // edx@17
    unsigned int v19; // edx@20
    int v20; // eax@27
    int v21; // edx@29
    unsigned __int16 *v24; // [sp+14h] [bp-4h]@6
    int Width; // [sp+2Ch] [bp+14h]@6

    if (this->uNumSceneBegins && a4 && a5)
    {
        v24 = (unsigned __int16 *)a4->GetPixels(IMAGE_FORMAT_R5G6B5);
        Width = a4->GetWidth();
        uHeight = a4->GetHeight();
        int clipped_out_x = x;
        int clipped_out_y = y;
        if (this->bClip)
        {
            if ((signed int)x < (signed int)this->uClipX)
            {
                v24 += this->uClipX - x;
                Width += x - this->uClipX;
                clipped_out_x = uClipX;
            }
            if ((signed int)y < (signed int)this->uClipY)
            {
                v24 += a4->GetWidth() * (this->uClipY - y);
                uHeight = y - this->uClipY + a4->GetHeight();
                clipped_out_y = uClipY;
            }
            v14 = this->uClipX;
            if ((signed int)this->uClipX < (signed int)x)
                v14 = x;
            if ((signed int)(Width + v14) >(signed int)this->uClipZ)
            {
                v16 = this->uClipX;
                if ((signed int)this->uClipX < (signed int)x)
                    v16 = x;
                Width = this->uClipZ - v16;
            }
            v17 = this->uClipY;
            if ((signed int)this->uClipY < (signed int)y)
                v17 = y;
            if ((signed int)(uHeight + v17) >(signed int)this->uClipW)
            {
                v19 = this->uClipY;
                if ((signed int)this->uClipY < (signed int)y)
                    v19 = y;
                uHeight = this->uClipW - v19;
            }
        }

        for (uint dy = 0; dy < uHeight; ++dy)
        {
            for (int dx = 0; dx < Width; ++dx)
            {
                /*v20 = *v24;
                if (v20 >= a7 && v20 <= a8)
                {
                    v21 = a7 + (a6 + v20) % (2 * (a8 - a7));
                    if ((a6 + v20) % (2 * (a8 - a7)) >= a8 - a7)
                        v21 = 2 * a8 - v21 - a7;
                    WritePixel16(clipped_out_x + dx, clipped_out_y + dy, a4->pPalette16[v21]);
                }
                ++v24;*/
                WritePixel16(clipped_out_x + dx, clipped_out_y + dy, *v24);
            }
            v24 += a4->GetWidth() - Width;
        }
    }
}

//----- (004A63E6) --------------------------------------------------------
void Render::BlendTextures(int a2, int a3, Image *a4, Image *a5, int t, int start_opacity, int end_opacity)
{
    unsigned int v14; // edx@11
    unsigned int v16; // edx@14
    unsigned int v17; // edx@17
    unsigned int v19; // edx@20
    int v20; // eax@27
    int v21; // edx@29
    int Height; // [sp+10h] [bp-8h]@6
    int Width; // [sp+14h] [bp-4h]@6
    int v27; // [sp+24h] [bp+Ch]@23
    unsigned __int16 *v28; // [sp+28h] [bp+10h]@6

    if (this->uNumSceneBegins)
    {
        if (a4 && a5)
        {
            //if ( a4->pPalette16 )
            {
                //if ( a5 )
                {
                    //if ( a5->pPalette16 )
                    {
                        v28 = (unsigned __int16 *)a4->GetPixels(IMAGE_FORMAT_R5G6B5);
                        Width = a4->GetWidth();
                        Height = a4->GetHeight();
                        int clipped_out_x = a2;
                        int clipped_out_y = a3;
                        if (this->bClip)
                        {
                            if ((signed int)a2 < (signed int)this->uClipX)
                            {
                                v28 += this->uClipX - a2;
                                Width += a2 - this->uClipX;
                                clipped_out_x = uClipX;
                            }

                            if ((signed int)a3 < (signed int)this->uClipY)
                            {
                                v28 += a4->GetWidth() * (this->uClipY - a3);
                                Height += a3 - this->uClipY;
                                clipped_out_y = uClipY;
                            }

                            v14 = this->uClipX;
                            if ((signed int)this->uClipX < (signed int)a2)
                                v14 = a2;
                            if ((signed int)(Width + v14) >(signed int)this->uClipZ)
                            {
                                v16 = this->uClipX;
                                if ((signed int)this->uClipX < (signed int)a2)
                                    v16 = a2;
                                Width = this->uClipZ - v16;
                            }

                            v17 = this->uClipY;
                            if ((signed int)this->uClipY < (signed int)a3)
                                v17 = a3;
                            if ((signed int)(Height + v17) >(signed int)this->uClipW)
                            {
                                v19 = this->uClipY;
                                if ((signed int)this->uClipY < (signed int)a3)
                                    v19 = a3;
                                Height = this->uClipW - v19;
                            }
                        }

                        v27 = 0;
                        for (int y = 0; y < Height; ++y)
                        {
                            for (int x = 0; x < Width; ++x)
                            {
__debugbreak(); // find out what's that spell
                                /*if (*v28)
                                {
                                    v20 = *((unsigned __int16 *)a5->GetPixels(IMAGE_FORMAT_R5G6B5) + x & (a5->GetWidth() - 1) + a5->GetWidth() * (v27 & (a5->GetHeight() - 1)));
                                    if (v20 >= start_opacity)
                                    {
                                        if (v20 <= end_opacity)
                                        {
                                            v21 = start_opacity + (t + v20) % (2 * (end_opacity - start_opacity));
                                            if ((t + v20) % (2 * (end_opacity - start_opacity)) >= end_opacity - start_opacity)
                                                v21 = 2 * end_opacity - v21 - start_opacity;

                                            WritePixel16(clipped_out_x + x, clipped_out_y + y, a5->pPalette16[v21]);
                                        }
                                    }
                                }*/
                                v28++;
                            }
                            v28 += a4->GetWidth() - Width;
                        }
                    }
                }
            }
        }
    }
}




void Render::DrawTextureAlphaNew(float u, float v, Image *img)
{
    int uHeight; // ebx@4
    unsigned int v11; // edx@9
    unsigned int v12; // esi@12
    unsigned int v13; // esi@15
    unsigned int v15; // esi@18
    //unsigned __int8 *v19; // [sp+18h] [bp-8h]@4
    int uWidth; // [sp+1Ch] [bp-4h]@4

    if (!uNumSceneBegins || !img)
        return;

    uHeight = img->GetHeight();
    ///v19 = pTexture->paletted_pixels;
    uWidth = img->GetWidth();

    auto pixels = (const unsigned __int32 *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    int uX = u * 640.0f;
    int uY = v * 480.0f;
    int clipped_out_x = uX;
    int clipped_out_y = uY;
    if (this->bClip)
    {
        if ((signed int)uX < (signed int)this->uClipX)
        {
            pixels += this->uClipX - uX;
            uWidth += uX - this->uClipX;
            clipped_out_x = uClipX;
        }

        uHeight = img->GetHeight();
        if ((signed int)uY < (signed int)this->uClipY)
        {
            pixels += img->GetWidth() * (this->uClipY - uY);
            uHeight = uY - this->uClipY + img->GetHeight();
            clipped_out_y = uClipY;
        }
        v11 = this->uClipX;
        if ((signed int)this->uClipX < (signed int)uX)
            v11 = uX;

        if ((signed int)(v11 + uWidth) >(signed int)this->uClipZ)
        {
            v12 = this->uClipX;
            if ((signed int)this->uClipX < (signed int)uX)
                v12 = uX;
            uWidth = this->uClipZ - v12;
        }
        v13 = this->uClipY;
        if ((signed int)this->uClipY < (signed int)uY)
            v13 = uY;

        if ((signed int)(uHeight + v13) >(signed int)this->uClipW)
        {
            v15 = this->uClipY;
            if ((signed int)this->uClipY < (signed int)uY)
                v15 = uY;
            uHeight = this->uClipW - v15;
        }
    }

    for (int y = 0; y < uHeight; ++y)
    {
        for (int x = 0; x < uWidth; ++x)
        {
            if (*pixels & 0xFF000000)
                WritePixel16(
                    clipped_out_x + x,
                    clipped_out_y + y,
                    Color16(
                        (*pixels >> 16) & 0xFF,
                        (*pixels >> 8) & 0xFF,
                        *pixels & 0xFF
                    )
                );
            ++pixels;
        }
        pixels  += img->GetWidth() - uWidth;
    }
}


/*
//----- (004A6274) --------------------------------------------------------
void Render::DrawTextureIndexedAlpha(unsigned int uX, unsigned int uY, Texture_MM7 *pTexture)
{
  int uHeight; // ebx@4
  unsigned int v11; // edx@9
  unsigned int v12; // esi@12
  unsigned int v13; // esi@15
  unsigned int v15; // esi@18
  unsigned __int8 *v19; // [sp+18h] [bp-8h]@4
  int uWidth; // [sp+1Ch] [bp-4h]@4

  if ( this->uNumSceneBegins )
  {
    if ( pTexture )
    {
      if ( pTexture->pPalette16 )
      {
        uHeight = pTexture->uTextureHeight;
        v19 = pTexture->paletted_pixels;
        uWidth = pTexture->uTextureWidth;

        int clipped_out_x = uX;
        int clipped_out_y = uY;
        if ( this->bClip )
        {
          if ( (signed int)uX < (signed int)this->uClipX )
          {
            v19 += this->uClipX - uX;
            uWidth += uX - this->uClipX;
            clipped_out_x = uClipX;
          }

          uHeight = pTexture->uTextureHeight;
          if ( (signed int)uY < (signed int)this->uClipY )
          {
            v19 += pTexture->uTextureWidth * (this->uClipY - uY);
            uHeight = uY - this->uClipY + pTexture->uTextureHeight;
            clipped_out_y = uClipY;
          }
          v11 = this->uClipX;
          if ( (signed int)this->uClipX < (signed int)uX )
            v11 = uX;

          if ( (signed int)(v11 + uWidth) > (signed int)this->uClipZ )
          {
            v12 = this->uClipX;
            if ( (signed int)this->uClipX < (signed int)uX )
              v12 = uX;
            uWidth = this->uClipZ - v12;
          }
          v13 = this->uClipY;
          if ( (signed int)this->uClipY < (signed int)uY )
            v13 = uY;

          if ( (signed int)(uHeight + v13) > (signed int)this->uClipW )
          {
            v15 = this->uClipY;
            if ( (signed int)this->uClipY < (signed int)uY )
              v15 = uY;
            uHeight = this->uClipW - v15;
          }
        }

        for (int y = 0; y < uHeight; ++y)
        {
          for (int x = 0; x < uWidth; ++x)
          {
            if ( *v19 )
              WritePixel16(clipped_out_x + x, clipped_out_y + y, pTexture->pPalette16[*v19]);
            ++v19;
          }
          v19 += pTexture->uTextureWidth - uWidth;
        }
      }
    }
  }
}*/


//----- (004A612A) --------------------------------------------------------
void Render::ZDrawTextureAlpha(float u, float v, Image *img, int zVal)
{
  unsigned int v6; // edx@3
  int v7; // ebx@3
  int v8; // edi@3
  int v10; // eax@5
  signed int v12; // esi@8
  signed int v14; // esi@11
  unsigned int v15; // esi@14
  unsigned int v17; // ecx@17
  int v18; // edx@23
  int v19; // [sp+Ch] [bp-Ch]@3
  int v20; // [sp+10h] [bp-8h]@3
  int uOutXa; // [sp+20h] [bp+8h]@21
  //unsigned __int8 *uOutYa; // [sp+24h] [bp+Ch]@3
  int *pZBuffer; // [sp+28h] [bp+10h]@3

  if (!this->uNumSceneBegins || !img)
      return;

  int uOutX = u * this->window->GetWidth();
  int uOutY = v * this->window->GetHeight();

      v6 = uOutY;
      v7 = img->GetHeight();
      pZBuffer = &this->pActiveZBuffer[uOutX + window->GetWidth() * uOutY];
      //uOutYa = pTexture->paletted_pixels;
      v8 = v20 = v19 = img->GetWidth();
      auto pixels = (unsigned __int32 *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);
      if ( this->bClip )
      {
        if ( uOutX < this->uClipX )
        {
          v10 = this->uClipX - uOutX;
          pixels += v10;
          v8 += uOutX - this->uClipX;
          v20 = v8;
          pZBuffer += v10;
        }
        if ( (signed int)v6 < (signed int)this->uClipY )
        {
            pixels += v19 * (this->uClipY - v6);
          v7 += v6 - this->uClipY;
          pZBuffer += window->GetWidth() * (this->uClipY - v6);
          v8 = v20;
        }
        v12 = this->uClipX;
        if ( this->uClipX < uOutX )
          v12 = uOutX;
        if ( v8 + v12 > (signed int)this->uClipZ )
        {
          v14 = this->uClipX;
          if ( this->uClipX < uOutX )
            v14 = uOutX;
          v8 = this->uClipZ - v14;
        }
        v15 = this->uClipY;
        if ( (signed int)this->uClipY < (signed int)v6 )
          v15 = v6;
        if ( (signed int)(v7 + v15) > (signed int)this->uClipW )
        {
          v17 = this->uClipY;
          if ( (signed int)this->uClipY >= (signed int)v6 )
            v6 = v17;
          v7 = this->uClipW - v6;
        }
      }



      if ( v7 > 0 )
      {
        uOutXa = v7;
        do
        {
          if ( v8 > 0 )
          {
            v18 = v8;
            do
            {
              if ( *pixels & 0xFF000000)
                *pZBuffer = zVal;
              ++pZBuffer;
              ++pixels;
              --v18;
            }
            while ( v18 );
          }
          pZBuffer += window->GetWidth() - v8;
          pixels += v19 - v8;
          --uOutXa;
        }
        while ( uOutXa );
      }

}

//----- (004A601E) --------------------------------------------------------
void Render::ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture, int a5)
{
/*  signed int v5; // edx@3
  int v6; // ebx@3
  int v7; // esi@3
  void *v8; // esi@3
  signed int v11; // edi@8
  signed int v13; // edi@11
  unsigned int v14; // edi@14
  unsigned int v16; // ecx@17
  int v17; // [sp+18h] [bp+Ch]@3
  unsigned int pTexturea; // [sp+1Ch] [bp+10h]@3

  if ( this->uNumSceneBegins && pTexture )
  {
    v5 = a3;
    v6 = pTexture->uTextureHeight;
    v7 = 5 * a3;
    v17 = pTexture->uTextureHeight;
    v8 = &this->pActiveZBuffer[a2 + (v7 << 7)];
    pTexturea = pTexture->uTextureWidth;
    if ( this->bClip )
    {
      if ( a2 < (signed int)this->uClipX )
      {
        pTexturea += a2 - this->uClipX;
        v8 = (char *)v8 + 4 * (this->uClipX - a2);
      }
      if ( v5 < (signed int)this->uClipY )
      {
        v17 += v5 - this->uClipY;
        v8 = (char *)v8 + 2560 * (this->uClipY - v5);
      }
      v11 = this->uClipX;
      if ( this->uClipX < a2 )
        v11 = a2;
      if ( (signed int)(pTexturea + v11) > (signed int)this->uClipZ )
      {
        v13 = this->uClipX;
        if ( this->uClipX < a2 )
          v13 = a2;
        pTexturea = this->uClipZ - v13;
      }
      v14 = this->uClipY;
      if ( (signed int)this->uClipY < v5 )
        v14 = v5;
      v6 = v17;
      if ( (signed int)(v17 + v14) > (signed int)this->uClipW )
      {
        v16 = this->uClipY;
        if ( (signed int)this->uClipY < v5 )
          v16 = v5;
        v6 = this->uClipW - v16;
      }
    }
    if ( v6 > 0 )
    {
      do
      {
        if ( (signed int)pTexturea > 0 )
        {
          memset32(v8, a5, pTexturea);
          v8 = (char *)v8 + 4 * pTexturea;
        }
        v8 = (char *)v8 + 4 * (window->GetWidth() - pTexturea);
        --v6;
      }
      while ( v6 );
    }
  }*/
}


/*
//----- (004A5EB2) --------------------------------------------------------
void Render::DrawTextureTransparentColorKey(signed int x, signed int y, Texture_MM7 *tex)
{
  int v5; // ebx@4
  unsigned int v8; // edx@6
  unsigned int v10; // edx@8
  unsigned int v11; // edx@9
  unsigned int v12; // esi@12
  unsigned int v13; // esi@15
  unsigned int v15; // esi@18
  int v18; // [sp+10h] [bp-10h]@4
  unsigned __int8 *v19; // [sp+18h] [bp-8h]@4
  int v20; // [sp+1Ch] [bp-4h]@4

  if ( this->uNumSceneBegins )
  {
    if ( tex )
    {
      if ( tex->pPalette16 )
      {
        v5 = tex->uTextureHeight;
        //pTarget = &this->pTargetSurface[uX + uY * this->uTargetSurfacePitch];
        v19 = tex->paletted_pixels;
        v20 = tex->uTextureWidth;
        v18 = tex->uTextureWidth;

        int clipped_out_x = x;
        int clipped_out_y = y;
        if ( this->bClip )
        {
          if ( (signed int)x < (signed int)this->uClipX )
          {
            v8 = this->uClipX - x;
            v19 += v8;
            v20 += x - this->uClipX;
            clipped_out_x = uClipX;
          }

          v5 = tex->uTextureHeight;
          if ( (signed int)y < (signed int)this->uClipY )
          {
            v10 = this->uClipY - y;
            v19 += v18 * v10;
            v5 = y - this->uClipY + tex->uTextureHeight;
            //v4 = a4;
            clipped_out_y = uClipY;
          }

          v11 = this->uClipX;
          if ( (signed int)this->uClipX < (signed int)x )
            v11 = x;

          if ( (signed int)(v11 + v20) > (signed int)this->uClipZ )
          {
            v12 = this->uClipX;
            if ( (signed int)this->uClipX < (signed int)x )
              v12 = x;
            v20 = this->uClipZ - v12;
          }

          v13 = this->uClipY;
          if ( (signed int)this->uClipY < (signed int)y )
            v13 = y;

          if ( (signed int)(v5 + v13) > (signed int)uClipW )
          {
            v15 = this->uClipY;
            if ( (signed int)this->uClipY < (signed int)y )
              v15 = y;
            v5 = uClipW - v15;
          }
        }

        for (int y = 0; y < v5; ++y)
        {
          for (int x = 0; x < v20; ++x)
          {
            if ( tex->pPalette16[*v19] != 0x7FF )// 2047
              WritePixel16(clipped_out_x + x, clipped_out_y + y, tex->pPalette16[*v19]);
            ++v19;
          }
          v19 += v18 - v20;
        }
      }
    }
  }
}*/



//----- (004524D8) --------------------------------------------------------
HWLTexture *RenderHWLContainer::LoadTexture(const char *pName, int bMipMaps)
{
  void *v13; // eax@13
  int v16; // esi@14
  int v17; // ecx@16
  int v18; // esi@16
  unsigned __int16 *v19; // eax@16
  int v20; // edx@16
  int v21; // ecx@16
  int v22; // eax@16
  int v23; // esi@16
  unsigned __int16 *v26; // [sp+24h] [bp-10h]@13
  int v27; // [sp+28h] [bp-Ch]@14
  int v28; // [sp+2Ch] [bp-8h]@13
  int pDestb; // [sp+3Ch] [bp+8h]@15

  if (!uNumItems)
    return nullptr;

  ///////////////////////////////
  //quick search(быстрый поиск)//
  ///////////////////////////////
  uint idx1 = 0,
       idx2 = uNumItems;
  while (true)
  {
    uint i = idx1 + (idx2 - idx1) / 2;

    int res = _stricmp(pName, pSpriteNames[i]);
    if (!res)
    {
      fseek(pFile, pSpriteOffsets[i], SEEK_SET);
      break;
    }
    else if (res < 0)
      idx2 = idx1 + (idx2 - idx1) / 2;
    else
      idx1 = i + 1;

    if ( idx1 >= idx2 )
      return false;
  }

  uint uCompressedSize = 0;
  fread(&uCompressedSize, 4, 1, pFile);

  HWLTexture* pTex = new HWLTexture;
  fread(&pTex->uBufferWidth, 4, 1, pFile);
  fread(&pTex->uBufferHeight, 4, 1, pFile);
  fread(&pTex->uAreaWidth, 4, 1, pFile);
  fread(&pTex->uAreaHeigth, 4, 1, pFile);
  fread(&pTex->uWidth, 4, 1, pFile);
  fread(&pTex->uHeight, 4, 1, pFile);
  fread(&pTex->uAreaX, 4, 1, pFile);
  fread(&pTex->uAreaY, 4, 1, pFile);

  pTex->pPixels = new unsigned __int16[pTex->uWidth * pTex->uHeight];
  if (uCompressedSize)
  {
    char* pCompressedData = new char[uCompressedSize];
    fread(pCompressedData, 1, uCompressedSize, pFile);
    uint uDecompressedSize = pTex->uWidth * pTex->uHeight * sizeof(short);
    zlib::MemUnzip(pTex->pPixels, &uDecompressedSize, pCompressedData, uCompressedSize);
    delete [] pCompressedData;
  }
  else
    fread(pTex->pPixels, 2, pTex->uWidth * pTex->uHeight, pFile);

  if ( scale_hwls_to_half )
  {
	__debugbreak();//Ritor1
    pTex->uHeight /= 2;
    pTex->uWidth /= 2;
    v13 = new unsigned __int16[pTex->uWidth * pTex->uHeight];
    v28 = 0;
    v26 = (unsigned __int16 *)v13;
    if ( pTex->uHeight > 0 )
    {
      v16 = pTex->uWidth;
      v27 = 1;
      do
      {
        pDestb = 0;
        if ( v16 > 0 )
        {
          do
          {
            v17 = v16 * v27;
            v18 = v28 * v16;
            v19 = pTex->pPixels;
            v20 = pDestb + 2 * v18;
            v21 = (int)&v19[2 * (pDestb + v17)];
            v22 = (int)&v19[2 * v20];
            LOWORD(v20) = *(unsigned short *)(v21 + 2);
            LOWORD(v21) = *(unsigned short *)v21;
            v23 = pDestb + v18;
			pDestb++;
            
            v26[v23] = _452442_color_cvt(*(unsigned short *)v22, *(unsigned short *)(v22 + 2), v21, v20);
            v16 = pTex->uWidth;
          }
          while (pDestb < pTex->uWidth);
        }
        ++v28;
        v27 += 2;
      }
      while ( v28 < (signed int)pTex->uHeight );
    }
    delete [] pTex->pPixels;
    pTex->pPixels = v26;
  }
  return pTex;
}
//----- (0045271F) --------------------------------------------------------
bool RenderHWLContainer::Release()
{
  __int32 v4; // eax@6
  FILE *v5; // ST24_4@6
  FILE *File; // [sp+4h] [bp-4h]@6

  if ( this->bDumpDebug)
  {
    File = fopen("logd3d.txt", "w");
    v4 = ftell(this->pFile);
    v5 = this->pFile;
    this->uDataOffset = v4;
    fwrite(&this->uNumItems, 4u, 1u, v5);
    for (uint i = 0; i < this->uNumItems; i++)
    {
      fwrite(this->pSpriteNames[i], 1u, 0x14u, this->pFile);
      fprintf(File, "D3D texture name:  %s\t\toffset: %x\n", this->pSpriteNames[i], *(unsigned int *)(&(this->pSpriteNames[i]) + 200000/sizeof(char*)));
    }
    fwrite(this->pSpriteOffsets, 4u, this->uNumItems, this->pFile);
    fseek(this->pFile, 4, 0);
    fwrite(&this->uDataOffset, 4u, 1u, this->pFile);
    fclose(this->pFile);
    fclose(File);
  }
  else
  {
    fclose(this->pFile);
    for (uint i = 0; i < this->uNumItems; i++)
    {
      delete[] this->pSpriteNames[i];
    }
  }
  return true;
}

//----- (00452347) --------------------------------------------------------
RenderHWLContainer::RenderHWLContainer():
  bDumpDebug(false)
{
  this->pFile = 0;
  uSignature = 0;
  this->uDataOffset = 0;
  memset(&this->uNumItems, 0, 0x61A84u);
  this->uNumItems = 0;
  this->scale_hwls_to_half = false;
}

//----- (0045237F) --------------------------------------------------------
bool RenderHWLContainer::Load(const wchar_t *pFilename)
{
  pFile = _wfopen(pFilename, L"rb");
  if (!pFile)
  {
    Log::Warning(L"Failed to open file: %s", pFilename);
    return false;
  }

  fread(&uSignature, 1, 4, pFile);
  if (uSignature != 'TD3D')
  {
    Log::Warning(L"Invalid format: %s", pFilename);
    return false;
  }
  
  fread(&uDataOffset, 4, 1, pFile);
  fseek(pFile, uDataOffset, SEEK_SET);
  fread(&uNumItems, 4, 1, pFile);

  memset(pSpriteNames, 0, 50000 * sizeof(char *));
  for (uint i = 0; i < uNumItems; ++i)
  {
    pSpriteNames[i] = new char[20];
    fread(pSpriteNames[i], 1, 20, pFile);
  }
  fread(pSpriteOffsets, 4, uNumItems, pFile);

  return true;
}

//----- (004A1C1E) --------------------------------------------------------
void Render::DoRenderBillboards_D3D()
{
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));

    /*if (render->uNumBillboardsToDraw)
    {
    auto p = &render->pBillboardRenderListD3D[0];
    for (int i = 0; i < p->uNumVertices; ++i)
    {
      p->pQuads[i].pos.z -= p->pQuads[i].pos.z * 0.6;
      //p->pQuads[i].rhw = + 0.8 * (1.0f - p->pQuads[i].rhw);
    }
    p->pQuads[0].pos.x = 10;
    p->pQuads[0].pos.y = 10;

    p->pQuads[1].pos.x = 10;
    p->pQuads[1].pos.y = 200;

    p->pQuads[2].pos.x = 100;
    p->pQuads[2].pos.y = 200;

    p->pQuads[3].pos.x = 100;
    p->pQuads[3].pos.y = 10;

        if (p->uOpacity != RenderBillboardD3D::NoBlend)
        SetBillboardBlendOptions(p->uOpacity);

      render->pRenderD3D->pDevice->SetTexture(0, p->pTexture);
      ErrD3D(render->pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                           D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                                           p->pQuads, p->uNumVertices,
                                                           D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS));

    }*/

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i)
    {
        if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend)
            SetBillboardBlendOptions(pBillboardRenderListD3D[i].opacity);

        pRenderD3D->pDevice->SetTexture(0, pBillboardRenderListD3D[i].pTexture);
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            pBillboardRenderListD3D[i].pQuads, pBillboardRenderListD3D[i].uNumVertices,
            D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS));
    }

    if (bFogEnabled)
    {
        bFogEnabled = false;
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0));
    }
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
}

//----- (004A1DA8) --------------------------------------------------------
void Render::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1)
{
  switch (a1)
  {
    case RenderBillboardD3D::Transparent:
    {
      if (bFogEnabled)
      {
        bFogEnabled = false;
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0));
      }

      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    }
    break;

    case RenderBillboardD3D::Opaque_1:
    case RenderBillboardD3D::Opaque_2:
    case RenderBillboardD3D::Opaque_3:
    {
      if (bUsingSpecular)
      {
        if (!bFogEnabled)
        {
          bFogEnabled = true;
          ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
        }
      }

      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
      ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
    }
    break;

    default:
      Log::Warning(L"SetBillboardBlendOptions: invalid opacity type (%u)", a1);
      assert(false);
    break;
  }
}
//----- (00424CD7) --------------------------------------------------------
int ODM_NearClip(unsigned int num_vertices)
{
  bool current_vertices_flag; // edi@1
  bool next_vertices_flag; // [sp+Ch] [bp-24h]@6
  double t; // st6@10
  bool bFound;

  bFound = false;

  if (!num_vertices)
    return 0;
  for (uint i = 0; i < num_vertices; ++i)// есть ли пограничные вершины
  {
    if ( VertexRenderList[i].vWorldViewPosition.x > 8.0 )
    {
      bFound = true;
      break;
    }
  }
  if ( !bFound )
    return 0;

  memcpy(&VertexRenderList[num_vertices], &VertexRenderList[0], sizeof(VertexRenderList[0]));
  next_vertices_flag = false;
  current_vertices_flag = false;
  if ( VertexRenderList[0].vWorldViewPosition.x <= 8.0 )
    current_vertices_flag = true;
  //check for near clip plane(проверка по ближней границе)
  //   
  // v3.__________________. v0
  //   |                  |
  //   |                  |
  //   |                  |
  //  ----------------------- 8.0(near_clip - 8.0)
  //   |                  |
  //   .__________________.
  //  v2                     v1

  int out_num_vertices = 0;
  for (uint i = 0; i < num_vertices; ++i)
  {
    next_vertices_flag = VertexRenderList[i + 1].vWorldViewPosition.x <= 8.0;//
    if ( current_vertices_flag ^ next_vertices_flag )
    {
      if ( next_vertices_flag )//следующая вершина за ближней границей
      {
        //t = near_clip - v0.x / v1.x - v0.x    (формула получения точки пересечения отрезка с плоскостью)
        t = (8.0 - VertexRenderList[i].vWorldViewPosition.x) / (VertexRenderList[i + 1].vWorldViewPosition.x - VertexRenderList[i].vWorldViewPosition.x);
        array_507D30[out_num_vertices].vWorldViewPosition.x = 8.0;
        array_507D30[out_num_vertices].vWorldViewPosition.y = VertexRenderList[i].vWorldViewPosition.y + (VertexRenderList[i + 1].vWorldViewPosition.y - VertexRenderList[i].vWorldViewPosition.y) * t;
        array_507D30[out_num_vertices].vWorldViewPosition.z = VertexRenderList[i].vWorldViewPosition.z + (VertexRenderList[i + 1].vWorldViewPosition.z - VertexRenderList[i].vWorldViewPosition.z) * t;
        array_507D30[out_num_vertices].u = VertexRenderList[i].u + (VertexRenderList[i + 1].u - VertexRenderList[i].u) * t;
        array_507D30[out_num_vertices].v = VertexRenderList[i].v + (VertexRenderList[i + 1].v - VertexRenderList[i].v) * t;
        array_507D30[out_num_vertices]._rhw = 1.0 / 8.0;
      }
      else// текущая вершина за ближней границей
      {
        t = (8.0 - VertexRenderList[i].vWorldViewPosition.x) / (VertexRenderList[i].vWorldViewPosition.x - VertexRenderList[i + 1].vWorldViewPosition.x);
        array_507D30[out_num_vertices].vWorldViewPosition.x = 8.0;
        array_507D30[out_num_vertices].vWorldViewPosition.y = VertexRenderList[i].vWorldViewPosition.y + (VertexRenderList[i].vWorldViewPosition.y - VertexRenderList[i + 1].vWorldViewPosition.y) * t;
        array_507D30[out_num_vertices].vWorldViewPosition.z = VertexRenderList[i].vWorldViewPosition.z + (VertexRenderList[i].vWorldViewPosition.z - VertexRenderList[i + 1].vWorldViewPosition.z) * t;
        array_507D30[out_num_vertices].u = VertexRenderList[i].u + (VertexRenderList[i].u - VertexRenderList[i + 1].u) * t;
        array_507D30[out_num_vertices].v = VertexRenderList[i].v + (VertexRenderList[i].v - VertexRenderList[i + 1].v) * t;
        array_507D30[out_num_vertices]._rhw = 1.0 / 8.0;
      }
      //array_507D30[out_num_vertices]._rhw = 0x3E000000u;
      ++out_num_vertices;
    }
    if ( !next_vertices_flag )
    {
      memcpy(&array_507D30[out_num_vertices], &VertexRenderList[i + 1], sizeof(VertexRenderList[i + 1]));
      out_num_vertices++;
    }
    current_vertices_flag = next_vertices_flag;
  }
  return out_num_vertices >= 3 ? out_num_vertices : 0;
}

//----- (00424EE0) --------------------------------------------------------
int ODM_FarClip(unsigned int uNumVertices)
{
  bool current_vertices_flag; // [sp+Ch] [bp-28h]@6
  bool next_vertices_flag; // edi@1
  double t; // st6@10
  signed int depth_num_vertices; // [sp+18h] [bp-1Ch]@1
  bool bFound;
  //Доп инфо "Программирование трёхмерных игр для windows" Ламот стр 910

  bFound = false;

  memcpy(&VertexRenderList[uNumVertices], &VertexRenderList[0], sizeof(VertexRenderList[uNumVertices]));
  depth_num_vertices = 0;
  current_vertices_flag = false;
  if ( VertexRenderList[0].vWorldViewPosition.x >= pODMRenderParams->shading_dist_mist )
    current_vertices_flag = true;//настоящая вершина больше границы видимости
  if ( (signed int)uNumVertices <= 0 )
    return 0;
  for (uint i = 0; i < uNumVertices; ++i)// есть ли пограничные вершины
  {
    if ( VertexRenderList[i].vWorldViewPosition.x < pODMRenderParams->shading_dist_mist )
    {
      bFound = true;
      break;
    }
  }
  if ( !bFound )
    return 0;
  //check for far clip plane(проверка по дальней границе)
  //   
  // v3.__________________. v0
  //   |                  |
  //   |                  |
  //   |                  |
  //  ----------------------- 8192.0(far_clip - 0x2000)
  //   |                  |
  //   .__________________.
  //  v2                     v1

  for ( uint i = 0; i < uNumVertices; ++i )
  {
    next_vertices_flag = VertexRenderList[i + 1].vWorldViewPosition.x >= pODMRenderParams->shading_dist_mist;
    if ( current_vertices_flag ^ next_vertices_flag )//одна из граней за границей видимости
    {
      if ( next_vertices_flag )//следующая вершина больше границы видимости(настоящая вершина меньше границы видимости) - v3
      {
        //t = far_clip - v2.x / v3.x - v2.x (формула получения точки пересечения отрезка с плоскостью)
        t = (pODMRenderParams->shading_dist_mist - VertexRenderList[i].vWorldViewPosition.x) / (VertexRenderList[i].vWorldViewPosition.x - VertexRenderList[i + 1].vWorldViewPosition.x);
        array_507D30[depth_num_vertices].vWorldViewPosition.x = pODMRenderParams->shading_dist_mist;
        //New_y = v2.y + (v3.y - v2.y)*t
        array_507D30[depth_num_vertices].vWorldViewPosition.y = VertexRenderList[i].vWorldViewPosition.y + (VertexRenderList[i].vWorldViewPosition.y - VertexRenderList[i + 1].vWorldViewPosition.y) * t;
        //New_z = v2.z + (v3.z - v2.z)*t
        array_507D30[depth_num_vertices].vWorldViewPosition.z = VertexRenderList[i].vWorldViewPosition.z + (VertexRenderList[i].vWorldViewPosition.z - VertexRenderList[i + 1].vWorldViewPosition.z) * t;
        array_507D30[depth_num_vertices].u = VertexRenderList[i].u + (VertexRenderList[i].u - VertexRenderList[i + 1].u) * t;
        array_507D30[depth_num_vertices].v = VertexRenderList[i].v + (VertexRenderList[i].v - VertexRenderList[i + 1].v) * t;
        array_507D30[depth_num_vertices]._rhw = 1.0 / pODMRenderParams->shading_dist_mist;
      }
      else//настоящая вершина больше границы видимости(следующая вершина меньше границы видимости) - v0
      {
        //t = far_clip - v1.x / v0.x - v1.x
        t = (pODMRenderParams->shading_dist_mist - VertexRenderList[i].vWorldViewPosition.x) / (VertexRenderList[i + 1].vWorldViewPosition.x - VertexRenderList[i].vWorldViewPosition.x);
        array_507D30[depth_num_vertices].vWorldViewPosition.x = pODMRenderParams->shading_dist_mist;
        //New_y = (v0.y - v1.y)*t + v1.y
        array_507D30[depth_num_vertices].vWorldViewPosition.y = VertexRenderList[i].vWorldViewPosition.y + (VertexRenderList[i + 1].vWorldViewPosition.y - VertexRenderList[i].vWorldViewPosition.y) * t;
        //New_z = (v0.z - v1.z)*t + v1.z
        array_507D30[depth_num_vertices].vWorldViewPosition.z = VertexRenderList[i].vWorldViewPosition.z + (VertexRenderList[i + 1].vWorldViewPosition.z - VertexRenderList[i].vWorldViewPosition.z) * t;
        array_507D30[depth_num_vertices].u = VertexRenderList[i].u + (VertexRenderList[i + 1].u - VertexRenderList[i].u) * t;
        array_507D30[depth_num_vertices].v = VertexRenderList[i].v + (VertexRenderList[i + 1].v - VertexRenderList[i].v) * t;
        array_507D30[depth_num_vertices]._rhw = 1.0 / pODMRenderParams->shading_dist_mist;
      }
      ++depth_num_vertices;
    }
    if ( !next_vertices_flag )//оба в границе видимости
    {
      memcpy(&array_507D30[depth_num_vertices], &VertexRenderList[i + 1], sizeof(array_507D30[depth_num_vertices]));
      depth_num_vertices++;
    }
    current_vertices_flag = next_vertices_flag;
  }
  return depth_num_vertices >= 3 ? depth_num_vertices : 0;
}

//----- (0047840D) --------------------------------------------------------
void Render::DrawBuildingsD3D()
{
  int v9; // ecx@8
  Texture_MM7 *pFaceTexture; // eax@10
  unsigned int v16; // edi@22
  int v27; // eax@57
//  int vertex_id; // eax@58
  unsigned int v34; // eax@80
  int v40; // [sp-4h] [bp-5Ch]@2
  int v49; // [sp+2Ch] [bp-2Ch]@10
  int v50; // [sp+30h] [bp-28h]@34
  int v51; // [sp+34h] [bp-24h]@35
  int v52; // [sp+38h] [bp-20h]@36
  int v53; // [sp+3Ch] [bp-1Ch]@8
  int uNumVertices; // [sp+4Ch] [bp-Ch]@34
  int unused; // [sp+50h] [bp-8h]@3

  if ( !pRenderD3D )
  {
    MessageBoxW(nullptr, L"D3D version of RenderBuildings called in software!", L"E:\\WORK\\MSDEV\\MM7\\MM7\\Code\\Odbuild.cpp:73", 0);
  }

  unused = 0;
  if ( (signed int)pOutdoor->uNumBModels > 0 )
  {
    for ( uint model_id = 0; model_id < (unsigned int)pOutdoor->uNumBModels; model_id++ )
    {
      if ( IsBModelVisible(model_id, &unused) )
      {
        pOutdoor->pBModels[model_id].field_40 |= 1;
        if ( pOutdoor->pBModels[model_id].uNumFaces > 0 )
        {
          for ( int face_id = 0; face_id < pOutdoor->pBModels[model_id].uNumFaces; face_id++ )
          {
            if (!pOutdoor->pBModels[model_id].pFaces[face_id].Invisible())
            {
            v53 = 0;
            array_77EC08[pODMRenderParams->uNumPolygons].flags = 0;
            array_77EC08[pODMRenderParams->uNumPolygons].field_32 = 0;
            v9 = pOutdoor->pBModels[model_id].pFaces[face_id].uTextureID;
            if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_TEXTURE_FRAME)
              v9 = pTextureFrameTable->GetFrameTexture(v9, pEventTimer->uTotalGameTimeElapsed);
            pFaceTexture = pBitmaps_LOD->GetTexture(v9);
            array_77EC08[pODMRenderParams->uNumPolygons].pTexture = pFaceTexture;
            if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLUID)
              array_77EC08[pODMRenderParams->uNumPolygons].flags |= 2;
            if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_INDOOR_SKY )
              HIBYTE(array_77EC08[pODMRenderParams->uNumPolygons].flags) |= 4;
            if ( pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_DIAGONAL )
              HIBYTE(array_77EC08[pODMRenderParams->uNumPolygons].flags) |= 4;
            else
            {
              if ( pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_VERTICAL )
                HIBYTE(array_77EC08[pODMRenderParams->uNumPolygons].flags) |= 8;
            }
            if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_FLOW_HORIZONTAL)
              array_77EC08[pODMRenderParams->uNumPolygons].flags |= 0x2000;
            else
            {
              if (pOutdoor->pBModels[model_id].pFaces[face_id].uAttributes & FACE_DONT_CACHE_TEXTURE)
                HIBYTE(array_77EC08[pODMRenderParams->uNumPolygons].flags) |= 0x10;
            }
            array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaU = pOutdoor->pBModels[model_id].pFaces[face_id].sTextureDeltaU;
            array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV = pOutdoor->pBModels[model_id].pFaces[face_id].sTextureDeltaV;
            v16 = GetTickCount() >> 4;
            if ( pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z && abs(pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z) >= 59082 )
            {
              if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 4 )
                array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV += v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uHeightMinus1;
              if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 8 )
                array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV -= v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uHeightMinus1;
            }
            else
            {
              if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 4 )
                array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV -= v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uHeightMinus1;
              if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 8 )
                array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV += v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uHeightMinus1;
            }
            if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 0x10 )
              array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaU -= v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uWidthMinus1;
            else
            {
              if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 0x20 )
                array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaU += v16 & array_77EC08[pODMRenderParams->uNumPolygons].pTexture->uWidthMinus1;
            }
            v50 = 0;
            v49 = 0;
            uNumVertices = pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices;
            if ( pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices > 0 )
            {
              for ( uint vertex_id = 1; vertex_id <= pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; vertex_id++ )
              {
                array_73D150[vertex_id - 1].vWorldPosition.x = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].x;
                array_73D150[vertex_id - 1].vWorldPosition.y = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].y;
                array_73D150[vertex_id - 1].vWorldPosition.z = pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[vertex_id - 1]].z;
                array_73D150[vertex_id - 1].u = (array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaU + (signed __int16)pOutdoor->pBModels[model_id].pFaces[face_id].pTextureUIDs[vertex_id - 1]) * (1.0 / (double)pFaceTexture->uTextureWidth);
                array_73D150[vertex_id - 1].v = (array_77EC08[pODMRenderParams->uNumPolygons].sTextureDeltaV + (signed __int16)pOutdoor->pBModels[model_id].pFaces[face_id].pTextureVIDs[vertex_id - 1]) * (1.0 / (double)pFaceTexture->uTextureHeight);
              }
              for ( uint i = 1; i <= pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; i++ )
              {
                if ( pOutdoor->pBModels[model_id].pVertices.pVertices[pOutdoor->pBModels[model_id].pFaces[face_id].pVertexIDs[0]].z == array_73D150[i - 1].vWorldPosition.z )
                  ++v53;
                pIndoorCameraD3D->ViewTransform(&array_73D150[i - 1], 1);
                if ( array_73D150[i - 1].vWorldViewPosition.x < 8.0 || array_73D150[i - 1].vWorldViewPosition.x > pODMRenderParams->shading_dist_mist )
                {
                  if ( array_73D150[i - 1].vWorldViewPosition.x >= 8.0 )
                    v49 = 1;
                  else
                    v50 = 1;
                }
                else
                  pIndoorCameraD3D->Project(&array_73D150[i - 1], 1, 0);
              }
            }
            if ( v53 == pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices )
              LOBYTE(array_77EC08[pODMRenderParams->uNumPolygons].field_32) |= 1;
            array_77EC08[pODMRenderParams->uNumPolygons].pODMFace = &pOutdoor->pBModels[model_id].pFaces[face_id];
            array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices = pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices;
            array_77EC08[pODMRenderParams->uNumPolygons].field_59 = 5;
            v51 = fixpoint_mul(-pOutdoor->vSunlight.x, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.x);
            v53 = fixpoint_mul(-pOutdoor->vSunlight.y, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.y);
            v52 = fixpoint_mul(-pOutdoor->vSunlight.z, pOutdoor->pBModels[model_id].pFaces[face_id].pFacePlane.vNormal.z);
            array_77EC08[pODMRenderParams->uNumPolygons].dimming_level = 20 - fixpoint_mul(20, v51 + v53 + v52);
            if ( array_77EC08[pODMRenderParams->uNumPolygons].dimming_level < 0 )
              array_77EC08[pODMRenderParams->uNumPolygons].dimming_level = 0;
            if ( array_77EC08[pODMRenderParams->uNumPolygons].dimming_level > 31 )
              array_77EC08[pODMRenderParams->uNumPolygons].dimming_level = 31;
            if ( pODMRenderParams->uNumPolygons >= 1999 + 5000)
              return;
            if ( ODMFace::IsBackfaceNotCulled(array_73D150, &array_77EC08[pODMRenderParams->uNumPolygons]) )
            {
              pOutdoor->pBModels[model_id].pFaces[face_id].bVisible = 1;
              array_77EC08[pODMRenderParams->uNumPolygons].uBModelFaceID = face_id;
              array_77EC08[pODMRenderParams->uNumPolygons].uBModelID = model_id;
              v27 = 8 * (face_id | (model_id << 6));
              LOBYTE(v27) = v27 | 6;
              array_77EC08[pODMRenderParams->uNumPolygons].field_50 = v27;
              for ( int vertex_id = 0; vertex_id < pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices; ++vertex_id)
              {
                memcpy(&VertexRenderList[vertex_id], &array_73D150[vertex_id], sizeof(VertexRenderList[vertex_id]));
                VertexRenderList[vertex_id]._rhw = 1.0 / (array_73D150[vertex_id].vWorldViewPosition.x + 0.0000001);
              }
              static stru154 static_RenderBuildingsD3D_stru_73C834;
              /*static bool __init_flag = false;
              if (!__init_flag)
              {
                __init_flag = true;
                static_RenderBuildingsD3D_byte_73C84C_init_flag |= 1u;
                stru154::stru154(&static_RenderBuildingsD3D_stru_73C834);
                atexit(loc_4789D4);
              }*/

              v40 = (int)&pOutdoor->pBModels[model_id].pFaces[face_id];
              pEngine->pLightmapBuilder->ApplyLights_OutdoorFace(&pOutdoor->pBModels[model_id].pFaces[face_id]);
              pDecalBuilder->ApplyDecals_OutdoorFace(&pOutdoor->pBModels[model_id].pFaces[face_id]);
              pEngine->pLightmapBuilder->StationaryLightsCount = 0;
              int v31 = 0;
              if ( Lights.uNumLightsApplied > 0 || pDecalBuilder->uNumDecals > 0 )
              {
                v31 = v50 ? 3 : v49 != 0 ? 5 : 0;
                static_RenderBuildingsD3D_stru_73C834.GetFacePlaneAndClassify(&pOutdoor->pBModels[model_id].pFaces[face_id], &pOutdoor->pBModels[model_id].pVertices);
                if ( pDecalBuilder->uNumDecals > 0 )
                {
                  v40 = -1;
                  pDecalBuilder->ApplyDecals(31 - array_77EC08[pODMRenderParams->uNumPolygons].dimming_level, 2, &static_RenderBuildingsD3D_stru_73C834,
                                       pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices, VertexRenderList, 0, (char)v31, -1);
                }
              }
              if ( Lights.uNumLightsApplied > 0 )
                pEngine->pLightmapBuilder->ApplyLights(&Lights, &static_RenderBuildingsD3D_stru_73C834, uNumVertices, VertexRenderList, 0, (char)v31);
              if ( v50 )
              {
                array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices = ODM_NearClip(pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices);
                uNumVertices = array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices;
                ODM_Project(array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices);
              }
              if ( v49 )
              {
                array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices = ODM_FarClip(pOutdoor->pBModels[model_id].pFaces[face_id].uNumVertices);
                uNumVertices = array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices;
                ODM_Project(array_77EC08[pODMRenderParams->uNumPolygons].uNumVertices);
              }
              if ( uNumVertices )
              {
                if ( array_77EC08[pODMRenderParams->uNumPolygons].flags & 2 )
                {
                  if ( BYTE1(array_77EC08[pODMRenderParams->uNumPolygons].flags) & 0x3C )
                    v34 = render->pHDWaterBitmapIDs[0];
                  else
                    v34 = render->pHDWaterBitmapIDs[render->hd_water_current_frame];
                  v40 = (int)pBitmaps_LOD->pHardwareTextures[v34];
                }
                else
                  v40 = (int)pBitmaps_LOD->pHardwareTextures[v9];
                render->DrawPolygon(uNumVertices, &array_77EC08[pODMRenderParams->uNumPolygons], &pOutdoor->pBModels[model_id].pFaces[face_id], (IDirect3DTexture2 *)v40);
              }
            }
          }
          }
        }
      }
    }
  }
  return;
}
//----- (00479543) --------------------------------------------------------
void Render::DrawOutdoorSkyD3D()
{
  int v9; // eax@4
  int v10; // ebx@4
  int v13; // edi@6
  int v14; // ecx@6
  int v15; // eax@8
  int v16; // eax@12
  signed __int64 v17; // qtt@13
  signed int v18; // ecx@13
  struct Polygon pSkyPolygon; // [sp+14h] [bp-150h]@1
  int v30; // [sp+134h] [bp-30h]@1
  int v32; // [sp+13Ch] [bp-28h]@6
  int v33; // [sp+140h] [bp-24h]@2
  signed __int64 v34; // [sp+144h] [bp-20h]@1
  int v35; // [sp+148h] [bp-1Ch]@4
  int v36; // [sp+14Ch] [bp-18h]@2
  int v37; // [sp+154h] [bp-10h]@8
  int v38; // [sp+158h] [bp-Ch]@1
  int v39; // [sp+15Ch] [bp-8h]@4

  v30 = (signed __int64)((double)(pODMRenderParams->int_fov_rad * pIndoorCameraD3D->vPartyPos.z)
                       / ((double)pODMRenderParams->int_fov_rad + 8192.0)
                       + (double)(pViewport->uScreenCenterY));
  v34 = cos((double)pIndoorCameraD3D->sRotationX * 0.0030664064) * 0x2000;//(double)pODMRenderParams->shading_dist_mist, 8192
  v38 = (signed __int64)((double)(pViewport->uScreenCenterY)
                       - (double)pODMRenderParams->int_fov_rad
                       / (v34 + 0.0000001)
                       * (sin((double)pIndoorCameraD3D->sRotationX * 0.0030664064)
                        * (double)-0x2000//(double)pODMRenderParams->shading_dist_mist
                        - (double)pIndoorCameraD3D->vPartyPos.z));
  pSkyPolygon.Create_48607B(&stru_8019C8);//заполняется ptr_38
  pSkyPolygon.ptr_38->_48694B_frustum_sky();

  //if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
	//pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
  //else
	pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
  pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ? (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);
  if ( pSkyPolygon.pTexture )
  {
    pSkyPolygon.dimming_level = 0;
    pSkyPolygon.uNumVertices = 4;
  //centering(центруем)-----------------------------------------------------------------
    pSkyPolygon.v_18.x = -stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX + 16);
    pSkyPolygon.v_18.y = 0;
    pSkyPolygon.v_18.z = -stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX + 16);
  
  //sky wiew position(положение неба на экране)------------------------------------------
  //                X
  // 0._____________________________.3
  //  |8,8                    468,8 |
  //  |                             |
  //  |                             |
  // Y|                             |
  //  |                             |
  //  |8,351                468,351 |
  // 1._____________________________.2
  // 
    VertexRenderList[0].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;//8
    VertexRenderList[0].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;//8

    VertexRenderList[1].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;//8
    VertexRenderList[1].vWorldViewProjY = (double)v38;//247

    VertexRenderList[2].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;//468
    VertexRenderList[2].vWorldViewProjY = (double)v38;//247

    VertexRenderList[3].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;//468
    VertexRenderList[3].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;//8

    pSkyPolygon.sTextureDeltaU = 224 * pMiscTimer->uTotalGameTimeElapsed;//7168
    pSkyPolygon.sTextureDeltaV = 224 * pMiscTimer->uTotalGameTimeElapsed;//7168

    pSkyPolygon.field_24 = 0x2000000;//maybe attributes
    v33 = 65536 / (signed int)(signed __int64)(((double)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) / 2) / tan(0.6457717418670654) + 0.5);
    for ( uint i = 0; i < pSkyPolygon.uNumVertices; ++i )
    {
      //rotate skydome(вращение купола неба)--------------------------------------
      // В игре принята своя система измерения углов. Полный угол (180). Значению угла 0 соответствует 
      // направление на север и/или юг (либо на восток и/или запад), значению 65536 еденицам(0х10000) соответствует угол 90.
      // две переменные хранят данные по углу обзора. field_14 по западу и востоку. field_20 по югу и северу
      // от -25080 до 25080
      v39 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
      v35 = v39 + pSkyPolygon.ptr_38->angle_from_north;

      v39 = fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_north_south, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.f)));
      v36 = v39 + pSkyPolygon.ptr_38->angle_from_east;

      v9 = fixpoint_mul(pSkyPolygon.v_18.z, v33 * (v30 - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
      v10 = pSkyPolygon.v_18.x + v9;
      if ( v10 > 0 )
        v10 = 0;
      v13 = v33 * (pViewport->uScreenCenterX - (signed __int64)VertexRenderList[i].vWorldViewProjX);
      v34 = -pSkyPolygon.field_24;
      v32 = (signed __int64)VertexRenderList[i].vWorldViewProjY - 1.0;
      v14 = v33 * (v30 - v32);
      while ( 1 )
      {
        if ( v10 )
        {
          v37 = abs((int)v34 >> 14);
          v15 = abs(v10);
          if ( v37 <= v15 || v32 <= (signed int)pViewport->uViewportTL_Y )
          {
            if ( v10 <= 0 )
              break;
          }
        }
        v16 = fixpoint_mul(pSkyPolygon.v_18.z, v14);
        --v32;
        v14 += v33;
        v10 = pSkyPolygon.v_18.x + v16;
      }
      LODWORD(v17) = LODWORD(v34) << 16;
      HIDWORD(v17) = v34 >> 16;
      v18 = v17 / v10;
      if ( v18 < 0 )
        v18 = pODMRenderParams->shading_dist_mist;
      v37 = v35 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_west, v13);
      v35 = 224 * pMiscTimer->uTotalGameTimeElapsed + ((signed int)fixpoint_mul(v37, v18) >> 3);
      VertexRenderList[i].u = (double)v35 / ((double)pSkyPolygon.pTexture->uTextureWidth * 65536.0);

      v36 = v36 + fixpoint_mul(pSkyPolygon.ptr_38->angle_from_south, v13);
      v35 = 224 * pMiscTimer->uTotalGameTimeElapsed + ((signed int)fixpoint_mul(v36, v18) >> 3);
      VertexRenderList[i].v = (double)v35 / ((double)pSkyPolygon.pTexture->uTextureHeight * 65536.0);

      VertexRenderList[i].vWorldViewPosition.x = (double)0x2000;//pODMRenderParams->shading_dist_mist 8192
      VertexRenderList[i]._rhw = 1.0 / (double)(v18 >> 16);
    }
    render->DrawOutdoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon, pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
    VertexRenderList[0].vWorldViewProjY = (double)v10;
    VertexRenderList[1].vWorldViewProjY = VertexRenderList[1].vWorldViewProjY + 30.0;
    VertexRenderList[2].vWorldViewProjY = VertexRenderList[2].vWorldViewProjY + 30.0;
    VertexRenderList[3].vWorldViewProjY = (double)v10;
    render->DrawOutdoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon, pBitmaps_LOD->pHardwareTextures[(signed __int16)pSkyPolygon.uTileBitmapID]);
    return;
  }
}
//----- (004226C2) --------------------------------------------------------
bool PauseGameDrawing()
{
  if ( current_screen_type != SCREEN_GAME
    && current_screen_type != SCREEN_NPC_DIALOGUE
    && current_screen_type != SCREEN_CHANGE_LOCATION )
  {
	  if (current_screen_type == SCREEN_INPUT_BLV)
		  return pMovie_Track;//pSmackerMovie != 0;
    if ( current_screen_type != SCREEN_BRANCHLESS_NPC_DIALOG )
      return true;
  }
  return false;
}


//----- (0045E03A) --------------------------------------------------------
unsigned short *Render::MakeScreenshot(signed int width, signed int height)
{
  unsigned __int16 *for_pixels; // ebx@1
  DDSURFACEDESC2 Dst; // [sp+4h] [bp-A0h]@6
  unsigned __int16 *pPixels; // [sp+80h] [bp-24h]@1
  float interval_x; // [sp+84h] [bp-20h]@1
  float interval_y; // [sp+8Ch] [bp-18h]@1

  interval_x = game_viewport_width / (double)width;
  interval_y = game_viewport_height / (double)height;

  pPixels = (unsigned __int16 *)malloc(2 * height * width);
  memset(pPixels, 0 , 2 * height * width);

  for_pixels = pPixels;

  BeginSceneD3D();

  if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
    pIndoor->Draw();
  else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    pOutdoor->Draw();
  DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
  memset(&Dst, 0, sizeof(Dst));
  Dst.dwSize = sizeof(Dst);

  if ( LockSurface_DDraw4(pBackBuffer4, &Dst, DDLOCK_WAIT) )
  {
    if (uCurrentlyLoadedLevelType == LEVEL_null)
      memset(&for_pixels, 0, sizeof(for_pixels));
	else
	{
      for (uint y = 0; y < (unsigned int)height; ++y)
      {
        for (uint x = 0; x < (unsigned int)width; ++x)
        {
          if (Dst.ddpfPixelFormat.dwRGBBitCount == 32)
          {
            unsigned __int32 *p = (unsigned __int32 *)Dst.lpSurface + (int)(x * interval_x + 8.0) + (int)(y * interval_y + 8.0) * (Dst.lPitch >> 2);
            *for_pixels = Color16((*p >> 16) & 255, (*p >> 8) & 255, *p & 255);
          }
          else if (Dst.ddpfPixelFormat.dwRGBBitCount == 16)
          {
            unsigned __int16 * p = (unsigned __int16 *)Dst.lpSurface + (int)(x * interval_x + 8.0) + y * Dst.lPitch;
            *for_pixels = *p;
          }
          else
            assert(false);
          ++for_pixels;
        }
      }
	}
    ErrD3D(pBackBuffer4->Unlock(NULL));
  }
  return pPixels;
}
//----- (0045E26C) --------------------------------------------------------
void Render::SaveScreenshot(const String &filename, unsigned int width, unsigned int height)
{
    auto pixels = MakeScreenshot(width, height);
    SavePCXImage(filename, pixels, width, height);
    free(pixels);
}

void Render::PackScreenshot(unsigned int width, unsigned int height, void *data, unsigned int data_size, unsigned int *out_screenshot_size)
{
  auto pixels = MakeScreenshot(150, 112);
  PackPCXpicture(pixels, 150, 112, data, 1000000, out_screenshot_size);
  free(pixels);
}


//----- (0046A7C8) --------------------------------------------------------
int Render::_46А6АС_GetActorsInViewport(int pDepth)
{
  unsigned int v3; // eax@2 применяется в закле Жар печи для подсчёта кол-ва монстров видимых группе и заполнения массива id видимых монстров
  unsigned int v5; // eax@2
  unsigned int v6; // eax@4
  unsigned int v12; // [sp+10h] [bp-14h]@1
  int mon_num; // [sp+1Ch] [bp-8h]@1
  unsigned int a1a; // [sp+20h] [bp-4h]@1

  mon_num = 0;
  v12 = GetBillboardDrawListSize();
  if ( (signed int)GetBillboardDrawListSize() > 0 )
  {
    for ( a1a = 0; (signed int)a1a < (signed int)v12; ++a1a )
    {
      v3 = GetParentBillboardID(a1a);
      v5 = (unsigned __int16)pBillboardRenderList[v3].object_pid;
      if ( PID_TYPE(v5) == OBJECT_Actor)
      {
        if ( pBillboardRenderList[v3].sZValue <= (unsigned int)(pDepth << 16) )
        {
          v6 = PID_ID(v5);
          if ( pActors[v6].uAIState != Dead 
		    && pActors[v6].uAIState != Dying
			&& pActors[v6].uAIState != Removed
            && pActors[v6].uAIState != Disabled
			&& pActors[v6].uAIState != Summoned )
          {
            if ( pEngine->pVisInstance->DoesRayIntersectBillboard((double)pDepth, a1a) )
            {
              if ( mon_num < 100 )
              {
                _50BF30_actors_in_viewport_ids[mon_num] = v6;
                mon_num++;
              }
            }
          }
        }
      }
    }
  }
  return mon_num;
}




void Render::BeginLightmaps()
{
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));

  if (bUsingSpecular)
    pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, pIndoorCameraD3D->LoadTextureAndGetHardwarePtr("effpar03")));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
}

void Render::EndLightmaps()
{
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));

  if (bUsingSpecular)
  {
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, uFogColor));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0));
  }
}


void Render::BeginLightmaps2()
{
  if (bUsingSpecular)
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
 
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, pIndoorCameraD3D->LoadTextureAndGetHardwarePtr("effpar03")));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
}


void Render::EndLightmaps2()
{
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));

  if (bUsingSpecular)
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
}

//----- (00437C96) --------------------------------------------------------
void Render::do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin, signed int sDiffuseBegin, const RenderVertexD3D3 *pLineEnd, signed int sDiffuseEnd, float z_stuff)
{
  RenderVertexD3D3 vertices[2]; // [sp+8h] [bp-40h]@2

  memcpy(&vertices[0], pLineBegin, sizeof(vertices[0]));
  memcpy(&vertices[1], pLineEnd, sizeof(vertices[1]));

  vertices[0].pos.z = 0.001 - z_stuff;
  vertices[1].pos.z = 0.001 - z_stuff;

  vertices[0].diffuse = sDiffuseBegin;
  vertices[1].diffuse = sDiffuseEnd;

  ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_LINELIST, 452, vertices, 2, D3DDP_DONOTLIGHT));
}


void Render::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices)
{
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_LINELIST,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              (void *)vertices,
              num_vertices,
              D3DDP_DONOTLIGHT));
}


void Render::DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices)
{
  //ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false));
  //ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
  
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                (void *)vertices,
                num_vertices,
                28));

  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  //ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE));
  //ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
}


void Render::BeginDecals()
{
  // code chunk from 0049C304
  if (bUsingSpecular)
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));

  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));

  ErrD3D(pRenderD3D->pDevice->SetTexture(0, pIndoorCameraD3D->LoadTextureAndGetHardwarePtr("hwsplat04")));
}


void Render::EndDecals()
{
  // code chunk from 0049C304
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));

  if (bUsingSpecular)
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
}



//----- (0049C095) --------------------------------------------------------
void Render::DrawDecal(Decal *pDecal, float z_bias)
{
  signed int dwFlags; // [sp+Ch] [bp-864h]@15
  RenderVertexD3D3 pVerticesD3D[64]; // [sp+20h] [bp-850h]@6

  if (pDecal->uNumVertices < 3)
  {
    Log::Warning(L"Decal has < 3 vertices");
    return;
  }

  float color_mult;
  if ( pDecal->field_C1C & 1 )
    color_mult = 1.0;
  else
    color_mult = pDecal->field_C18->_43B570_get_color_mult_by_time();

  for (uint i = 0; i < (unsigned int)pDecal->uNumVertices; ++i)
  {
    uint uTint = Render::GetActorTintColor(pDecal->pVertices[i].vWorldViewPosition.x, pDecal->field_C14, 0, 0, nullptr);

    uint uTintR = (uTint >> 16) & 0xFF,
         uTintG = (uTint >> 8) & 0xFF,
         uTintB = uTint & 0xFF;

    uint uDecalColorMultR = (pDecal->uColorMultiplier >> 16) & 0xFF,
         uDecalColorMultG = (pDecal->uColorMultiplier >> 8) & 0xFF,
         uDecalColorMultB = pDecal->uColorMultiplier & 0xFF;

    uint uFinalR = floorf(uTintR / 255.0 * color_mult * uDecalColorMultR + 0.0f),
         uFinalG = floorf(uTintG / 255.0 * color_mult * uDecalColorMultG + 0.0f),
         uFinalB = floorf(uTintB / 255.0 * color_mult * uDecalColorMultB + 0.0f);

    float v15;
    if (fabs(z_bias) < 1e-5)
      v15 = 1.0 - 1.0 / ((1.0f / pIndoorCameraD3D->GetShadingDistMist()) * pDecal->pVertices[i].vWorldViewPosition.x * 1000.0);
    else
    {
      v15 = 1.0 - 1.0 / ((1.0f / pIndoorCameraD3D->GetShadingDistMist()) * pDecal->pVertices[i].vWorldViewPosition.x * 1000.0) - z_bias;
      if (v15 < 0.000099999997)
        v15 = 0.000099999997;
    }

    pVerticesD3D[i].pos.x = pDecal->pVertices[i].vWorldViewProjX;
    pVerticesD3D[i].pos.y = pDecal->pVertices[i].vWorldViewProjY;
    pVerticesD3D[i].pos.z = v15;

    pVerticesD3D[i].rhw = 1.0 / pDecal->pVertices[i].vWorldViewPosition.x;
    pVerticesD3D[i].diffuse = (uFinalR << 16) | (uFinalG << 8) | uFinalB;
    pVerticesD3D[i].specular = 0;

    pVerticesD3D[i].texcoord.x = pDecal->pVertices[i].u;
    pVerticesD3D[i].texcoord.y = pDecal->pVertices[i].v;
  }

  if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
    dwFlags = D3DDP_DONOTLIGHT | D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS;
  else
    dwFlags = D3DDP_DONOTLIGHT;

  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            pVerticesD3D, pDecal->uNumVertices, dwFlags));
}


void Render::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, IDirect3DTexture2 *texture)
{
  ErrD3D(pRenderD3D->pDevice->SetTexture(0, texture));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS));
  ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                     D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                     (void *)vertices, 4, 28));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
  ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));
}

//----- (00452442) --------------------------------------------------------
unsigned int _452442_color_cvt(unsigned __int16 a1, unsigned __int16 a2, int a3, int a4)
{
	int v4; // ebx@0
	__int16 v5; // ST14_2@1
	__int16 v6; // dx@1
	int v7; // ecx@1
	__int16 v8; // ST10_2@1
	int v9; // edi@1
	unsigned __int16 v10; // dh@1@1
	int v11; // ebx@1
	int v12; // ebx@1
	__int16 a3a; // [sp+1Ch] [bp+8h]@1

	v5 = a2 >> 2;
	v6 = (unsigned __int16)a4 >> 2;
	v8 = a1 >> 2;
	a3a = (unsigned __int16)a3 >> 2;
	LOWORD(v7) = a3a;
	v9 = v7;
	LOWORD(v4) = ((unsigned __int16)a4 >> 2) & 0xE0;
	LOWORD(v7) = a3a & 0xE0;
	LOWORD(v9) = v9 & 0x1C00;
	v11 = v7 + v4;
	LOWORD(v7) = v5 & 0xE0;
	v12 = v7 + v11;
	LOWORD(v7) = v8 & 0xE0;
	__debugbreak(); // warning C4700: uninitialized local variable 'v10' used
	return (PID_TYPE(v8) + PID_TYPE(v5) + PID_TYPE(a3a) + PID_TYPE(v6)) | (v7 + v12) | ((v8 & 0x1C00)
		+ (v5 & 0x1C00)
		+ v9
		+ (__PAIR__(v10, (unsigned __int16)a4 >> 2) & 0x1C00));
}

//----- (0047C4FC) --------------------------------------------------------
int GetActorTintColor(int max_dimm, int min_dimm, float distance, int a4, RenderBillboard *a5)
{
	signed int v6; // edx@1
	int v8; // eax@3
	double v9; // st7@12
	int v11; // ecx@28
	double v15; // st7@44
	int v18; // ST14_4@44
	signed int v20; // [sp+10h] [bp-4h]@10
	float a3c; // [sp+1Ch] [bp+8h]@44
	int a5a; // [sp+24h] [bp+10h]@44

	//v5 = a2;
	v6 = 0;

	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
		return 8 * (31 - max_dimm) | ((8 * (31 - max_dimm) | ((31 - max_dimm) << 11)) << 8);

	if (pParty->armageddon_timer)
		return 0xFFFF0000;

	v8 = pWeather->bNight;
	if (bUnderwater)
		v8 = 0;
	if (v8)
	{
		v20 = 1;
		if (pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].Active())
			v20 = pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;
		v9 = (double)v20 * 1024.0;
		if (a4)
		{
			v6 = 216;
			goto LABEL_20;
		}
		if (distance <= v9)
		{
			if (distance > 0.0)
			{
				//a4b = distance * 216.0 / v9;
				//v10 = a4b + 6.7553994e15;
				//v6 = LODWORD(v10);
				v6 = floorf(0.5f + distance * 216.0 / v9);
				if (v6 > 216)
				{
					v6 = 216;
					goto LABEL_20;
				}
			}
		}
		else
		{
			v6 = 216;
		}
		if (distance != 0.0)
		{
		LABEL_20:
			if (a5)
				v6 = 8 * _43F55F_get_billboard_light_level(a5, v6 >> 3);
			if (v6 > 216)
				v6 = 216;
			return (255 - v6) | ((255 - v6) << 16) | ((255 - v6) << 8);
		}
		//LABEL_19:
		v6 = 216;
		goto LABEL_20;
	}



	if (fabsf(distance) < 1.0e-6f)
		return 0xFFF8F8F8;

	// dim in measured in 8-steps
	v11 = 8 * (max_dimm - min_dimm);
	//v12 = v11;
	if (v11 >= 0)
	{
		if (v11 > 216)
			v11 = 216;
	}
	else
		v11 = 0;

	float fog_density_mult = 216.0f;
	if (a4)
		fog_density_mult += distance / (double)pODMRenderParams->shading_dist_shade * 32.0;

	v6 = v11 + floorf(pOutdoor->fFogDensity * fog_density_mult + 0.5f);
	/*if ( a4 )
	{
	//a3b = pOutdoor->fFogDensity * 216.0;
	//v14 = a3b + 6.7553994e15;
	//a4a = floorf(a3b + 0.5f);//LODWORD(v14);
	}
	else
	{
	//a3a = (distance / (double)pODMRenderParams->shading_dist_shade * 32.0 + 216.0) * pOutdoor->fFogDensity;
	//v13 = a3a + 6.7553994e15;
	//a4a = floorf(a3a + 0.5f);//LODWORD(v13);
	}
	v6 = a4a + v11;*/
	if (a5)
		v6 = 8 * _43F55F_get_billboard_light_level(a5, v6 >> 3);
	if (v6 > 216)
		v6 = 216;
	if (v6 < v11)
		v6 = v11;
	if (v6 > 8 * pOutdoor->max_terrain_dimming_level)
		v6 = 8 * pOutdoor->max_terrain_dimming_level;
	if (!bUnderwater)
		return (255 - v6) | ((255 - v6) << 16) | ((255 - v6) << 8);
	else
	{
		v15 = (double)(255 - v6) * 0.0039215689;
		a3c = v15;
		//a4c = v15 * 16.0;
		//v16 = a4c + 6.7553994e15;
		a5a = floorf(v15 * 16.0 + 0.5f);//LODWORD(v16);
		//a4d = a3c * 194.0;
		//v17 = a4d + 6.7553994e15;
		v18 = floorf(a3c * 194.0 + 0.5f);//LODWORD(v17);
		//a3d = a3c * 153.0;
		//v19 = a3d + 6.7553994e15;
		return (int)floorf(a3c * 153.0 + 0.5f)/*LODWORD(v19)*/ | ((v18 | (a5a << 8)) << 8);
	}
}
// 6BE3C4: using guessed type char bUnderwater;

//----- (0043F55F) --------------------------------------------------------
int _43F55F_get_billboard_light_level(RenderBillboard *a1, int uBaseLightLevel)
{
	signed int v3; // ecx@2

	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
		v3 = pIndoor->pSectors[a1->uIndoorSectorID].uMinAmbientLightLevel;
	else
	{
		if (uBaseLightLevel == -1)
			v3 = a1->dimming_level;
		else
			v3 = uBaseLightLevel;
	}
	return _43F5C8_get_point_light_level_with_respect_to_lights(v3, a1->uIndoorSectorID, a1->world_x, a1->world_y, a1->world_z);
}

//----- (0043F5C8) --------------------------------------------------------
int _43F5C8_get_point_light_level_with_respect_to_lights(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z)
{
	signed int v6; // edi@1
	int v8; // eax@6
	int v9; // ebx@6
	unsigned int v10; // ecx@6
	unsigned int v11; // edx@9
	unsigned int v12; // edx@11
	signed int v13; // ecx@12
	BLVLightMM7 *v16; // esi@20
	int v17; // ebx@21
	signed int v24; // ecx@30
	int v26; // ebx@35
	int v37; // [sp+Ch] [bp-18h]@37
	int v39; // [sp+10h] [bp-14h]@23
	int v40; // [sp+10h] [bp-14h]@36
	int v42; // [sp+14h] [bp-10h]@22
	unsigned int v43; // [sp+18h] [bp-Ch]@12
	unsigned int v44; // [sp+18h] [bp-Ch]@30
	unsigned int v45; // [sp+18h] [bp-Ch]@44

	v6 = uBaseLightLevel;
	for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i)
	{
		MobileLight* p = &pMobileLightsStack->pLights[i];

		float distX = abs(p->vPosition.x - x);
		if (distX <= p->uRadius)
		{
			float distY = abs(p->vPosition.y - y);
			if (distY <= p->uRadius)
			{
				float distZ = abs(p->vPosition.z - z);
				if (distZ <= p->uRadius)
				{
					v8 = distX;
					v9 = distY;
					v10 = distZ;
					if (distX < distY)
					{
						v8 = distY;
						v9 = distX;
					}
					if (v8 < distZ)
					{
						v11 = v8;
						v8 = distZ;
						v10 = v11;
					}
					if (v9 < (signed int)v10)
					{
						v12 = v10;
						v10 = v9;
						v9 = v12;
					}
					v43 = ((unsigned int)(11 * v9) / 32) + (v10 / 4) + v8;
					v13 = p->uRadius;
					if ((signed int)v43 < v13)
						v6 += ((unsigned __int64)(30i64 * (signed int)(v43 << 16) / v13) >> 16) - 30;
				}
			}
		}
	}

	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
	{
		BLVSector* pSector = &pIndoor->pSectors[uSectorID];

		for (uint i = 0; i < pSector->uNumLights; ++i)
		{
			v16 = pIndoor->pLights + pSector->pLights[i];
			if (~v16->uAtributes & 8)
			{
				v17 = abs(v16->vPosition.x - x);
				if (v17 <= v16->uRadius)
				{
					v42 = abs(v16->vPosition.y - y);
					if (v42 <= v16->uRadius)
					{
						v39 = abs(v16->vPosition.z - z);
						if (v39 <= v16->uRadius)
						{
							v44 = int_get_vector_length(v17, v42, v39);
							v24 = v16->uRadius;
							if ((signed int)v44 < v24)
								v6 += ((unsigned __int64)(30i64 * (signed int)(v44 << 16) / v24) >> 16) - 30;
						}
					}
				}
			}
		}
	}

	for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i)
	{
		//StationaryLight* p = &pStationaryLightsStack->pLights[i];
		v26 = abs(pStationaryLightsStack->pLights[i].vPosition.x - x);
		if (v26 <= pStationaryLightsStack->pLights[i].uRadius)
		{
			v40 = abs(pStationaryLightsStack->pLights[i].vPosition.y - y);
			if (v40 <= pStationaryLightsStack->pLights[i].uRadius)
			{
				v37 = abs(pStationaryLightsStack->pLights[i].vPosition.z - z);
				if (v37 <= pStationaryLightsStack->pLights[i].uRadius)
				{
					v45 = int_get_vector_length(v26, v40, v37);
					//v33 = pStationaryLightsStack->pLights[i].uRadius;
					if ((signed int)v45 < pStationaryLightsStack->pLights[i].uRadius)
						v6 += ((unsigned __int64)(30i64 * (signed int)(v45 << 16) / pStationaryLightsStack->pLights[i].uRadius) >> 16) - 30;
				}
			}
		}
	}

	if (v6 <= 31)
	{
		if (v6 < 0)
			v6 = 0;
	}
	else
		v6 = 31;
	return v6;
}

//----- (0049D700) --------------------------------------------------------
unsigned int GetMaxMipLevels(unsigned int uDim)
{
	int v2; // ecx@1
	unsigned int v3; // eax@1

	v2 = 0;
	v3 = uDim - 1;
	while (v3 & 1)
	{
		v3 >>= 1;
		++v2;
	}
	return v3 == 0 ? v2 : 0;
}

//----- (0046E44E) --------------------------------------------------------
int  _46E44E_collide_against_faces_and_portals(unsigned int b1)
{
	BLVSector *pSector; // edi@1
	signed int v2; // ebx@1
	BLVFace *pFace; // esi@2
	__int16 pNextSector; // si@10
	int pArrayNum; // ecx@12
	unsigned __int8 v6; // sf@12
	unsigned __int8 v7; // of@12
	int result; // eax@14
	//int v10; // ecx@15
	int pFloor; // eax@16
	int v15; // eax@24
	int v16; // edx@25
	int v17; // eax@29
	unsigned int v18; // eax@33
	int v21; // eax@35
	int v22; // ecx@36
	int v23; // eax@40
	unsigned int v24; // eax@44
	int a3; // [sp+10h] [bp-48h]@28
	int v26; // [sp+14h] [bp-44h]@15
	int i; // [sp+18h] [bp-40h]@1
	int a10; // [sp+1Ch] [bp-3Ch]@1
	int v29; // [sp+20h] [bp-38h]@14
	int v32; // [sp+2Ch] [bp-2Ch]@15
	int pSectorsArray[10]; // [sp+30h] [bp-28h]@1

	pSector = &pIndoor->pSectors[stru_721530.uSectorID];
	i = 1;
	a10 = b1;
	pSectorsArray[0] = stru_721530.uSectorID;
	for (v2 = 0; v2 < pSector->uNumPortals; ++v2)
	{
		pFace = &pIndoor->pFaces[pSector->pPortals[v2]];
		if (stru_721530.sMaxX <= pFace->pBounding.x2 && stru_721530.sMinX >= pFace->pBounding.x1
			&& stru_721530.sMaxY <= pFace->pBounding.y2 && stru_721530.sMinY >= pFace->pBounding.y1
			&& stru_721530.sMaxZ <= pFace->pBounding.z2 && stru_721530.sMinZ >= pFace->pBounding.z1
			&& abs((pFace->pFacePlane_old.dist
			+ stru_721530.normal.x * pFace->pFacePlane_old.vNormal.x
			+ stru_721530.normal.y * pFace->pFacePlane_old.vNormal.y
			+ stru_721530.normal.z * pFace->pFacePlane_old.vNormal.z) >> 16) <= stru_721530.field_6C + 16)
		{
			pNextSector = pFace->uSectorID == stru_721530.uSectorID ? pFace->uBackSectorID : pFace->uSectorID;//FrontSectorID
			pArrayNum = i++;
			v7 = i < 10;
			v6 = i - 10 < 0;
			pSectorsArray[pArrayNum] = pNextSector;
			if (!(v6 ^ v7))
				break;
		}
	}
	result = 0;
	for (v29 = 0; v29 < i; v29++)
	{
		pSector = &pIndoor->pSectors[pSectorsArray[v29]];
		v32 = pSector->uNumFloors + pSector->uNumWalls + pSector->uNumCeilings;
		for (v26 = 0; v26 < v32; v26++)
		{
			pFloor = pSector->pFloors[v26];
			pFace = &pIndoor->pFaces[pSector->pFloors[v26]];
			if (!pFace->Portal()
				&& stru_721530.sMaxX <= pFace->pBounding.x2 && stru_721530.sMinX >= pFace->pBounding.x1
				&& stru_721530.sMaxY <= pFace->pBounding.y2 && stru_721530.sMinY >= pFace->pBounding.y1
				&& stru_721530.sMaxZ <= pFace->pBounding.z2 && stru_721530.sMinZ >= pFace->pBounding.z1
				&& pFloor != stru_721530.field_84)
			{
				v15 = (pFace->pFacePlane_old.dist + stru_721530.normal.x * pFace->pFacePlane_old.vNormal.x
					+ stru_721530.normal.y * pFace->pFacePlane_old.vNormal.y
					+ stru_721530.normal.z * pFace->pFacePlane_old.vNormal.z) >> 16;
				if (v15 > 0)
				{
					v16 = (pFace->pFacePlane_old.dist + stru_721530.normal2.x * pFace->pFacePlane_old.vNormal.x
						+ stru_721530.normal2.y * pFace->pFacePlane_old.vNormal.y
						+ stru_721530.normal2.z * pFace->pFacePlane_old.vNormal.z) >> 16;
					if (v15 <= stru_721530.prolly_normal_d || v16 <= stru_721530.prolly_normal_d)
					{
						if (v16 <= v15)
						{
							a3 = stru_721530.field_6C;
							if (sub_47531C(stru_721530.prolly_normal_d, &a3, stru_721530.normal.x, stru_721530.normal.y, stru_721530.normal.z,
								stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, pFace, a10))
							{
								v17 = a3;
							}
							else
							{
								a3 = stru_721530.field_6C + stru_721530.prolly_normal_d;
								if (!sub_475D85(&stru_721530.normal, &stru_721530.direction, &a3, pFace))
									goto LABEL_34;
								v17 = a3 - stru_721530.prolly_normal_d;
								a3 -= stru_721530.prolly_normal_d;
							}
							if (v17 < stru_721530.field_7C)
							{
								stru_721530.field_7C = v17;
								v18 = 8 * pSector->pFloors[v26];
								LOBYTE(v18) = v18 | 6;
								stru_721530.uFaceID = v18;
							}
						}
					}
				}
			LABEL_34:
				if (!(stru_721530.field_0 & 1)
					|| (v21 = (pFace->pFacePlane_old.dist + stru_721530.position.x * pFace->pFacePlane_old.vNormal.x
					+ stru_721530.position.y * pFace->pFacePlane_old.vNormal.y
					+ stru_721530.position.z * pFace->pFacePlane_old.vNormal.z) >> 16, v21 <= 0)
					|| (v22 = (pFace->pFacePlane_old.dist + stru_721530.field_4C * pFace->pFacePlane_old.vNormal.x
					+ stru_721530.field_50 * pFace->pFacePlane_old.vNormal.y
					+ stru_721530.field_54 * pFace->pFacePlane_old.vNormal.z) >> 16, v21 > stru_721530.prolly_normal_d)
					&& v22 > stru_721530.prolly_normal_d || v22 > v21)
					continue;
				a3 = stru_721530.field_6C;
				if (sub_47531C(stru_721530.field_8_radius, &a3, stru_721530.position.x, stru_721530.position.y, stru_721530.position.z,
					stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, pFace, a10))
				{
					v23 = a3;
					goto LABEL_43;
				}
				a3 = stru_721530.field_6C + stru_721530.field_8_radius;
				if (sub_475D85(&stru_721530.position, &stru_721530.direction, &a3, pFace))
				{
					v23 = a3 - stru_721530.prolly_normal_d;
					a3 -= stru_721530.prolly_normal_d;
				LABEL_43:
					if (v23 < stru_721530.field_7C)
					{
						stru_721530.field_7C = v23;
						v24 = 8 * pSector->pFloors[v26];
						LOBYTE(v24) = v24 | 6;
						stru_721530.uFaceID = v24;
					}
				}
			}
		}
		result = v29 + 1;
	}
	return result;
}
// 46E44E: using guessed type int var_28[10];

//----- (0046E889) --------------------------------------------------------
int _46E889_collide_against_bmodels(unsigned int ecx0)
{
	int result; // eax@1
	//int v3; // ebx@9
	int v8; // eax@19
	int v9; // ecx@20
	int v10; // eax@24
	unsigned int v14; // eax@28
	int v15; // eax@30
	int v16; // ecx@31
	unsigned int v17; // eax@36
	int v21; // eax@42
	unsigned int v22; // eax@43
	//int a11; // [sp+70h] [bp-18h]@1
	//int a10; // [sp+80h] [bp-8h]@1
	int a2; // [sp+84h] [bp-4h]@23

	//a11 = ecx0;

	BLVFace face; // [sp+Ch] [bp-7Ch]@1

	result = 0;
	for (uint i = 0; i < (signed int)pOutdoor->uNumBModels; ++i)
	{
		if (stru_721530.sMaxX <= pOutdoor->pBModels[i].sMaxX && stru_721530.sMinX >= pOutdoor->pBModels[i].sMinX
			&& stru_721530.sMaxY <= pOutdoor->pBModels[i].sMaxY && stru_721530.sMinY >= pOutdoor->pBModels[i].sMinY
			&& stru_721530.sMaxZ <= pOutdoor->pBModels[i].sMaxZ && stru_721530.sMinZ >= pOutdoor->pBModels[i].sMinZ)
		{
			for (uint j = 0; j < pOutdoor->pBModels[i].uNumFaces; ++j)
			{
				if (stru_721530.sMaxX <= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.x2 && stru_721530.sMinX >= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.x1
					&& stru_721530.sMaxY <= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.y2 && stru_721530.sMinY >= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.y1
					&& stru_721530.sMaxZ <= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.z2 && stru_721530.sMinZ >= pOutdoor->pBModels[i].pFaces[j].pBoundingBox.z1)
				{
					face.pFacePlane_old.vNormal.x = pOutdoor->pBModels[i].pFaces[j].pFacePlane.vNormal.x;
					face.pFacePlane_old.vNormal.y = pOutdoor->pBModels[i].pFaces[j].pFacePlane.vNormal.y;
					face.pFacePlane_old.vNormal.z = pOutdoor->pBModels[i].pFaces[j].pFacePlane.vNormal.z;

					face.pFacePlane_old.dist = pOutdoor->pBModels[i].pFaces[j].pFacePlane.dist; //incorrect

					face.uAttributes = pOutdoor->pBModels[i].pFaces[j].uAttributes;

					face.pBounding.x1 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.x1;
					face.pBounding.y1 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.y1;
					face.pBounding.z1 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.z1;

					face.pBounding.x2 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.x2;
					face.pBounding.y2 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.y2;
					face.pBounding.z2 = pOutdoor->pBModels[i].pFaces[j].pBoundingBox.z2;

					face.zCalc1 = pOutdoor->pBModels[i].pFaces[j].zCalc1;
					face.zCalc2 = pOutdoor->pBModels[i].pFaces[j].zCalc2;
					face.zCalc3 = pOutdoor->pBModels[i].pFaces[j].zCalc3;

					face.pXInterceptDisplacements = pOutdoor->pBModels[i].pFaces[j].pXInterceptDisplacements;
					face.pYInterceptDisplacements = pOutdoor->pBModels[i].pFaces[j].pYInterceptDisplacements;
					face.pZInterceptDisplacements = pOutdoor->pBModels[i].pFaces[j].pZInterceptDisplacements;

					face.uPolygonType = (PolygonType)pOutdoor->pBModels[i].pFaces[j].uPolygonType;

					face.uNumVertices = pOutdoor->pBModels[i].pFaces[j].uNumVertices;

					face.uBitmapID = pOutdoor->pBModels[i].pFaces[j].uTextureID;

					face.pVertexIDs = pOutdoor->pBModels[i].pFaces[j].pVertexIDs;

					if (!face.Ethereal() && !face.Portal())
					{
						v8 = (face.pFacePlane_old.dist + face.pFacePlane_old.vNormal.x * stru_721530.normal.x
							+ face.pFacePlane_old.vNormal.y * stru_721530.normal.y
							+ face.pFacePlane_old.vNormal.z * stru_721530.normal.z) >> 16;
						if (v8 > 0)
						{
							v9 = (face.pFacePlane_old.dist + face.pFacePlane_old.vNormal.x * stru_721530.normal2.x
								+ face.pFacePlane_old.vNormal.y * stru_721530.normal2.y
								+ face.pFacePlane_old.vNormal.z * stru_721530.normal2.z) >> 16;
							if (v8 <= stru_721530.prolly_normal_d || v9 <= stru_721530.prolly_normal_d)
							{
								if (v9 <= v8)
								{
									a2 = stru_721530.field_6C;
									if (sub_4754BF(stru_721530.prolly_normal_d, &a2, stru_721530.normal.x, stru_721530.normal.y, stru_721530.normal.z,
										stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, &face, i, ecx0))
									{
										v10 = a2;
									}
									else
									{
										a2 = stru_721530.prolly_normal_d + stru_721530.field_6C;
										if (!sub_475F30(&a2, &face, stru_721530.normal.x, stru_721530.normal.y, stru_721530.normal.z,
											stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, i))
											goto LABEL_29;
										v10 = a2 - stru_721530.prolly_normal_d;
										a2 -= stru_721530.prolly_normal_d;
									}
									if (v10 < stru_721530.field_7C)
									{
										stru_721530.field_7C = v10;
										v14 = 8 * (j | (i << 6));
										LOBYTE(v14) = v14 | 6;
										stru_721530.uFaceID = v14;
									}
								}
							}
						}
					LABEL_29:
						if (stru_721530.field_0 & 1)
						{
							v15 = (face.pFacePlane_old.dist + face.pFacePlane_old.vNormal.x * stru_721530.position.x
								+ face.pFacePlane_old.vNormal.y * stru_721530.position.y
								+ face.pFacePlane_old.vNormal.z * stru_721530.position.z) >> 16;
							if (v15 > 0)
							{
								v16 = (face.pFacePlane_old.dist + face.pFacePlane_old.vNormal.x * stru_721530.field_4C
									+ face.pFacePlane_old.vNormal.y * stru_721530.field_50
									+ face.pFacePlane_old.vNormal.z * stru_721530.field_54) >> 16;
								if (v15 <= stru_721530.prolly_normal_d || v16 <= stru_721530.prolly_normal_d)
								{
									if (v16 <= v15)
									{
										a2 = stru_721530.field_6C;
										if (sub_4754BF(stru_721530.field_8_radius, &a2, stru_721530.position.x, stru_721530.position.y, stru_721530.position.z,
											stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, &face, i, ecx0))
										{
											if (a2 < stru_721530.field_7C)
											{
												stru_721530.field_7C = a2;
												v17 = 8 * (j | (i << 6));
												LOBYTE(v17) = v17 | 6;
												stru_721530.uFaceID = v17;
											}
										}
										else
										{
											a2 = stru_721530.field_6C + stru_721530.field_8_radius;
											if (sub_475F30(&a2, &face, stru_721530.position.x, stru_721530.position.y, stru_721530.position.z,
												stru_721530.direction.x, stru_721530.direction.y, stru_721530.direction.z, i))
											{
												v21 = a2 - stru_721530.prolly_normal_d;
												a2 -= stru_721530.prolly_normal_d;
												if (a2 < stru_721530.field_7C)
												{
													stru_721530.field_7C = v21;
													v22 = 8 * (j | (i << 6));
													LOBYTE(v22) = v22 | 6;
													stru_721530.uFaceID = v22;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		result = i;
	}
	return result;
}

//----- (0046ED1B) --------------------------------------------------------
int collide_against_floor(int x, int y, int z, unsigned int *pSectorID, unsigned int *pFaceID)
{
	uint uFaceID = -1;
	int floor_level = BLV_GetFloorLevel(x, y, z, *pSectorID, &uFaceID);

	if (floor_level != -30000 && floor_level <= z + 50)
	{
		*pFaceID = uFaceID;
		return floor_level;
	}

	uint uSectorID = pIndoor->GetSector(x, y, z);
	*pSectorID = uSectorID;

	floor_level = BLV_GetFloorLevel(x, y, z, uSectorID, &uFaceID);
	if (uSectorID && floor_level != -30000)
		*pFaceID = uFaceID;
	else return -30000;
	return floor_level;
}

//----- (0046ED8A) --------------------------------------------------------
void _46ED8A_collide_against_sprite_objects(unsigned int _this)
{
	ObjectDesc *object; // edx@4
	int v10; // ecx@12
	int v11; // esi@13

	for (uint i = 0; i < uNumSpriteObjects; ++i)
	{
		if (pSpriteObjects[i].uObjectDescID)
		{
			object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
			if (!(object->uFlags & OBJECT_DESC_NO_COLLISION))
			{
				if (stru_721530.sMaxX <= pSpriteObjects[i].vPosition.x + object->uRadius && stru_721530.sMinX >= pSpriteObjects[i].vPosition.x - object->uRadius
					&& stru_721530.sMaxY <= pSpriteObjects[i].vPosition.y + object->uRadius && stru_721530.sMinY >= pSpriteObjects[i].vPosition.y - object->uRadius
					&& stru_721530.sMaxZ <= pSpriteObjects[i].vPosition.z + object->uHeight && stru_721530.sMinZ >= pSpriteObjects[i].vPosition.z)
				{
					if (abs(((pSpriteObjects[i].vPosition.x - stru_721530.normal.x) * stru_721530.direction.y
						- (pSpriteObjects[i].vPosition.y - stru_721530.normal.y) * stru_721530.direction.x) >> 16)
						<= object->uHeight + stru_721530.prolly_normal_d)
					{
						v10 = ((pSpriteObjects[i].vPosition.x - stru_721530.normal.x) * stru_721530.direction.x
							+ (pSpriteObjects[i].vPosition.y - stru_721530.normal.y) * stru_721530.direction.y) >> 16;
						if (v10 > 0)
						{
							v11 = stru_721530.normal.z + ((unsigned __int64)(stru_721530.direction.z * (signed __int64)v10) >> 16);
							if (v11 >= pSpriteObjects[i].vPosition.z - stru_721530.prolly_normal_d)
							{
								if (v11 <= object->uHeight + stru_721530.prolly_normal_d + pSpriteObjects[i].vPosition.z)
								{
									if (v10 < stru_721530.field_7C)
										sub_46DEF2(_this, i);
								}
							}
						}
					}
				}
			}
		}
	}
}

//----- (0046EF01) --------------------------------------------------------
int _46EF01_collision_chech_player(int a1)
{
	int result; // eax@1
	int v3; // ebx@7
	int v4; // esi@7
	int v5; // edi@8
	int v6; // ecx@9
	int v7; // edi@12
	int v10; // [sp+14h] [bp-8h]@7
	int v11; // [sp+18h] [bp-4h]@7

	result = pParty->vPosition.x;
	//v9 = pParty->uPartyHeight;
	if (stru_721530.sMaxX <= pParty->vPosition.x + (2 * pParty->field_14_radius) && stru_721530.sMinX >= pParty->vPosition.x - (2 * pParty->field_14_radius)
		&& stru_721530.sMaxY <= pParty->vPosition.y + (2 * pParty->field_14_radius) && stru_721530.sMinY >= pParty->vPosition.y - (2 * pParty->field_14_radius)
		&& stru_721530.sMaxZ <= pParty->vPosition.z + pParty->uPartyHeight && stru_721530.sMinZ >= pParty->vPosition.z)
	{
		v3 = stru_721530.prolly_normal_d + (2 * pParty->field_14_radius);
		v11 = pParty->vPosition.x - stru_721530.normal.x;
		v4 = ((pParty->vPosition.x - stru_721530.normal.x) * stru_721530.direction.y
			- (pParty->vPosition.y - stru_721530.normal.y) * stru_721530.direction.x) >> 16;
		v10 = pParty->vPosition.y - stru_721530.normal.y;
		result = abs(((pParty->vPosition.x - stru_721530.normal.x) * stru_721530.direction.y
			- (pParty->vPosition.y - stru_721530.normal.y) * stru_721530.direction.x) >> 16);
		if (result <= stru_721530.prolly_normal_d + (2 * pParty->field_14_radius))
		{
			result = v10 * stru_721530.direction.y;
			v5 = (v10 * stru_721530.direction.y + v11 * stru_721530.direction.x) >> 16;
			if (v5 > 0)
			{
				v6 = fixpoint_mul(stru_721530.direction.z, v5) + stru_721530.normal.z;
				result = pParty->vPosition.z;
				if (v6 >= pParty->vPosition.z)
				{
					result = pParty->uPartyHeight + pParty->vPosition.z;
					if (v6 <= (signed int)(pParty->uPartyHeight + pParty->vPosition.z) || a1)
					{
						result = integer_sqrt(v3 * v3 - v4 * v4);
						v7 = v5 - integer_sqrt(v3 * v3 - v4 * v4);
						if (v7 < 0)
							v7 = 0;
						if (v7 < stru_721530.field_7C)
						{
							stru_721530.field_7C = v7;
							stru_721530.uFaceID = 4;
						}
					}
				}
			}
		}
	}
	return result;
}

//----- (0046E0B2) --------------------------------------------------------
void  _46E0B2_collide_against_decorations()
{
	BLVSector *sector; // ebp@1
	LevelDecoration *decor; // edi@2
	DecorationDesc *decor_desc; // esi@3
	int v8; // ebx@10
	int v9; // esi@11
	int v11; // eax@12
	int v12; // esi@14
	unsigned int v13; // eax@17
	signed int i; // [sp+4h] [bp-14h]@1
	int v15; // [sp+8h] [bp-10h]@10
	int v16; // [sp+Ch] [bp-Ch]@10
	int v17; // [sp+10h] [bp-8h]@10

	sector = &pIndoor->pSectors[stru_721530.uSectorID];
	for (i = 0; i < sector->uNumDecorations; ++i)
	{
		decor = &pLevelDecorations[sector->pDecorationIDs[i]];
		if (!(decor->uFlags & LEVEL_DECORATION_INVISIBLE))
		{
			decor_desc = &pDecorationList->pDecorations[decor->uDecorationDescID];
			if (!decor_desc->CanMoveThrough())
			{
				if (stru_721530.sMaxX <= decor->vPosition.x + decor_desc->uRadius && stru_721530.sMinX >= decor->vPosition.x - decor_desc->uRadius
					&& stru_721530.sMaxY <= decor->vPosition.y + decor_desc->uRadius && stru_721530.sMinY >= decor->vPosition.y - decor_desc->uRadius
					&& stru_721530.sMaxZ <= decor->vPosition.z + decor_desc->uDecorationHeight && stru_721530.sMinZ >= decor->vPosition.z)
				{
					v16 = decor->vPosition.x - stru_721530.normal.x;
					v15 = decor->vPosition.y - stru_721530.normal.y;
					v8 = stru_721530.prolly_normal_d + decor_desc->uRadius;
					v17 = ((decor->vPosition.x - stru_721530.normal.x) * stru_721530.direction.y
						- (decor->vPosition.y - stru_721530.normal.y) * stru_721530.direction.x) >> 16;
					if (abs(v17) <= stru_721530.prolly_normal_d + decor_desc->uRadius)
					{
						v9 = (v16 * stru_721530.direction.x + v15 * stru_721530.direction.y) >> 16;
						if (v9 > 0)
						{
							v11 = stru_721530.normal.z + fixpoint_mul(stru_721530.direction.z, v9);
							if (v11 >= decor->vPosition.z)
							{
								if (v11 <= decor_desc->uDecorationHeight + decor->vPosition.z)
								{
									v12 = v9 - integer_sqrt(v8 * v8 - v17 * v17);
									if (v12 < 0)
										v12 = 0;
									if (v12 < stru_721530.field_7C)
									{
										stru_721530.field_7C = v12;
										v13 = 8 * sector->pDecorationIDs[i];
										LOBYTE(v13) = v13 | 5;
										stru_721530.uFaceID = v13;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

//----- (0046F04E) --------------------------------------------------------
int _46F04E_collide_against_portals()
{
	unsigned int v1; // eax@1
	BLVFace *face; // eax@3
	int v4; // ecx@9
	int v5; // edx@9
	signed int result; // eax@21
	unsigned int v10; // [sp+8h] [bp-Ch]@1
	int a3; // [sp+Ch] [bp-8h]@13
	int v12; // [sp+10h] [bp-4h]@15

	v1 = 0xFFFFFF;
	v10 = 0xFFFFFF;
	for (uint i = 0; i < pIndoor->pSectors[stru_721530.uSectorID].uNumPortals; ++i)
	{
		if (pIndoor->pSectors[stru_721530.uSectorID].pPortals[i] != stru_721530.field_80)
		{
			face = &pIndoor->pFaces[pIndoor->pSectors[stru_721530.uSectorID].pPortals[i]];
			if (stru_721530.sMaxX <= face->pBounding.x2 && stru_721530.sMinX >= face->pBounding.x1
				&& stru_721530.sMaxY <= face->pBounding.y2 && stru_721530.sMinY >= face->pBounding.y1
				&& stru_721530.sMaxZ <= face->pBounding.z2 && stru_721530.sMinZ >= face->pBounding.z1)
			{
				v4 = (stru_721530.normal.x * face->pFacePlane_old.vNormal.x + face->pFacePlane_old.dist
					+ stru_721530.normal.y * face->pFacePlane_old.vNormal.y
					+ stru_721530.normal.z * face->pFacePlane_old.vNormal.z) >> 16;
				v5 = (stru_721530.normal2.z * face->pFacePlane_old.vNormal.z + face->pFacePlane_old.dist
					+ stru_721530.normal2.x * face->pFacePlane_old.vNormal.x
					+ stru_721530.normal2.y * face->pFacePlane_old.vNormal.y) >> 16;
				if ((v4 < stru_721530.prolly_normal_d || v5 < stru_721530.prolly_normal_d)
					&& (v4 > -stru_721530.prolly_normal_d || v5 > -stru_721530.prolly_normal_d)
					&& (a3 = stru_721530.field_6C, sub_475D85(&stru_721530.normal, &stru_721530.direction, &a3, face))
					&& a3 < (signed int)v10)
				{
					v10 = a3;
					v12 = pIndoor->pSectors[stru_721530.uSectorID].pPortals[i];
				}
			}
		}
	}
	v1 = v10;
	if (stru_721530.field_7C >= (signed int)v1 && (signed int)v1 <= stru_721530.field_6C)
	{
		stru_721530.field_80 = v12;
		if (pIndoor->pFaces[v12].uSectorID == stru_721530.uSectorID)
			stru_721530.uSectorID = pIndoor->pFaces[v12].uBackSectorID;
		else
			stru_721530.uSectorID = pIndoor->pFaces[v12].uSectorID;
		stru_721530.field_7C = 268435455;//0xFFFFFFF
		result = 0;
	}
	else
		result = 1;
	return result;
}

//----- (0046DEF2) --------------------------------------------------------
unsigned int sub_46DEF2(signed int a2, unsigned int uLayingItemID)
{
	unsigned int result; // eax@1

	result = uLayingItemID;
	if (pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID].uFlags & 0x10)
		result = _46BFFA_update_spell_fx(uLayingItemID, a2);
	return result;
}

//----- (0047253E) --------------------------------------------------------
void UpdateObjects()
{
	ObjectDesc *object; // eax@5
	int v5; // ecx@6
	signed int v7; // eax@9
	signed int v11; // eax@17
	int v12; // edi@27
	int v18; // [sp+4h] [bp-10h]@27
	int v19; // [sp+8h] [bp-Ch]@27

	for (uint i = 0; i < uNumSpriteObjects; ++i)
	{
		if (pSpriteObjects[i].uAttributes & OBJECT_40)
			pSpriteObjects[i].uAttributes &= ~OBJECT_40;
		else
		{
			object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
			if (pSpriteObjects[i].AttachedToActor())
			{
				v5 = PID_ID(pSpriteObjects[i].spell_target_pid);
				pSpriteObjects[i].vPosition.x = pActors[v5].vPosition.x;
				pSpriteObjects[i].vPosition.y = pActors[v5].vPosition.y;
				pSpriteObjects[i].vPosition.z = pActors[v5].vPosition.z + pActors[v5].uActorHeight;
				if (!pSpriteObjects[i].uObjectDescID)
					continue;
				pSpriteObjects[i].uSpriteFrameID += pEventTimer->uTimeElapsed;
				if (!(object->uFlags & OBJECT_DESC_TEMPORARY))
					continue;
				if (pSpriteObjects[i].uSpriteFrameID >= 0)
				{
					v7 = object->uLifetime;
					if (pSpriteObjects[i].uAttributes & ITEM_BROKEN)
						v7 = pSpriteObjects[i].field_20;
					if (pSpriteObjects[i].uSpriteFrameID < v7)
						continue;
				}
				SpriteObject::OnInteraction(i);
				continue;
			}
			if (pSpriteObjects[i].uObjectDescID)
			{
				pSpriteObjects[i].uSpriteFrameID += pEventTimer->uTimeElapsed;
				if (object->uFlags & OBJECT_DESC_TEMPORARY)
				{
					if (pSpriteObjects[i].uSpriteFrameID < 0)
					{
						SpriteObject::OnInteraction(i);
						continue;
					}
					v11 = object->uLifetime;
					if (pSpriteObjects[i].uAttributes & ITEM_BROKEN)
						v11 = pSpriteObjects[i].field_20;
				}
				if (!(object->uFlags & OBJECT_DESC_TEMPORARY) || pSpriteObjects[i].uSpriteFrameID < v11)
				{
					if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
						SpriteObject::UpdateObject_fn0_BLV(i);
					else
						SpriteObject::UpdateObject_fn0_ODM(i);
					if (pParty->bTurnBasedModeOn != 1 || !(pSpriteObjects[i].uSectorID & 4))
						continue;
					v12 = abs(pParty->vPosition.x - pSpriteObjects[i].vPosition.x);
					v18 = abs(pParty->vPosition.y - pSpriteObjects[i].vPosition.y);
					v19 = abs(pParty->vPosition.z - pSpriteObjects[i].vPosition.z);
					if (int_get_vector_length(v12, v18, v19) <= 5120)
						continue;
					SpriteObject::OnInteraction(i);
					continue;
				}
				if (!(object->uFlags & OBJECT_DESC_INTERACTABLE))
				{
					SpriteObject::OnInteraction(i);
					continue;
				}
				_46BFFA_update_spell_fx(i, PID(OBJECT_Item, i));
			}
		}
	}
}

//----- (0047531C) --------------------------------------------------------
bool sub_47531C(int a1, int *a2, int pos_x, int pos_y, int pos_z, int dir_x, int dir_y, int dir_z, BLVFace *face, int a10)
{
	int v11; // ST1C_4@3
	int v12; // edi@3
	int v13; // esi@3
	int v14; // edi@4
	signed __int64 v15; // qtt@6
	//__int16 v16; // si@7
	int a7a; // [sp+30h] [bp+18h]@7
	int a9b; // [sp+38h] [bp+20h]@3
	int a9a; // [sp+38h] [bp+20h]@3
	int a10b; // [sp+3Ch] [bp+24h]@3
	signed int a10a; // [sp+3Ch] [bp+24h]@4
	int a10c; // [sp+3Ch] [bp+24h]@5

	if (a10 && face->Ethereal())
		return 0;
	v11 = fixpoint_mul(dir_x, face->pFacePlane_old.vNormal.x);
	a10b = fixpoint_mul(dir_y, face->pFacePlane_old.vNormal.y);
	a9b = fixpoint_mul(dir_z, face->pFacePlane_old.vNormal.z);
	v12 = v11 + a9b + a10b;
	a9a = v11 + a9b + a10b;
	v13 = (a1 << 16)
		- pos_x * face->pFacePlane_old.vNormal.x
		- pos_y * face->pFacePlane_old.vNormal.y
		- pos_z * face->pFacePlane_old.vNormal.z
		- face->pFacePlane_old.dist;
	if (abs((a1 << 16)
		- pos_x * face->pFacePlane_old.vNormal.x
		- pos_y * face->pFacePlane_old.vNormal.y
		- pos_z * face->pFacePlane_old.vNormal.z - face->pFacePlane_old.dist) >= a1 << 16)
	{
		a10c = abs(v13) >> 14;
		if (a10c > abs(v12))
			return 0;
		LODWORD(v15) = v13 << 16;
		HIDWORD(v15) = v13 >> 16;
		v14 = a1;
		a10a = v15 / a9a;
	}
	else
	{
		a10a = 0;
		v14 = abs(v13) >> 16;
	}
	//v16 = pos_y + ((unsigned int)fixpoint_mul(a10a, dir_y) >> 16);
	LOWORD(a7a) = (short)pos_x + ((unsigned int)fixpoint_mul(a10a, dir_x) >> 16) - fixpoint_mul(v14, face->pFacePlane_old.vNormal.x);
	HIWORD(a7a) = pos_y + ((unsigned int)fixpoint_mul(a10a, dir_y) >> 16) - fixpoint_mul(v14, face->pFacePlane_old.vNormal.y);
	if (!sub_475665(face, a7a, (short)pos_z + ((unsigned int)fixpoint_mul(a10a, dir_z) >> 16) - fixpoint_mul(v14, face->pFacePlane_old.vNormal.z)))
		return 0;
	*a2 = a10a >> 16;
	if (a10a >> 16 < 0)
		*a2 = 0;
	return 1;
}


//----- (004754BF) --------------------------------------------------------
bool sub_4754BF(int a1, int *a2, int X, int Y, int Z, int dir_x, int dir_y, int dir_z, BLVFace *face, int a10, int a11)
{
	int v12; // ST1C_4@3
	int v13; // edi@3
	int v14; // esi@3
	int v15; // edi@4
	signed __int64 v16; // qtt@6
	//__int16 v17; // si@7
	int a7a; // [sp+30h] [bp+18h]@7
	int a1b; // [sp+38h] [bp+20h]@3
	int a1a; // [sp+38h] [bp+20h]@3
	int a11b; // [sp+40h] [bp+28h]@3
	signed int a11a; // [sp+40h] [bp+28h]@4
	int a11c; // [sp+40h] [bp+28h]@5

	if (a11 && face->Ethereal())
		return false;
	v12 = fixpoint_mul(dir_x, face->pFacePlane_old.vNormal.x);
	a11b = fixpoint_mul(dir_y, face->pFacePlane_old.vNormal.y);
	a1b = fixpoint_mul(dir_z, face->pFacePlane_old.vNormal.z);
	v13 = v12 + a1b + a11b;
	a1a = v12 + a1b + a11b;
	v14 = (a1 << 16)
		- X * face->pFacePlane_old.vNormal.x
		- Y * face->pFacePlane_old.vNormal.y
		- Z * face->pFacePlane_old.vNormal.z
		- face->pFacePlane_old.dist;
	if (abs((a1 << 16)
		- X * face->pFacePlane_old.vNormal.x
		- Y * face->pFacePlane_old.vNormal.y
		- Z * face->pFacePlane_old.vNormal.z - face->pFacePlane_old.dist) >= a1 << 16)
	{
		a11c = abs(v14) >> 14;
		if (a11c > abs(v13))
			return false;
		LODWORD(v16) = v14 << 16;
		HIDWORD(v16) = v14 >> 16;
		v15 = a1;
		a11a = v16 / a1a;
	}
	else
	{
		a11a = 0;
		v15 = abs(v14) >> 16;
	}
	//v17 = Y + ((unsigned int)fixpoint_mul(a11a, dir_y) >> 16);
	LOWORD(a7a) = (short)X + ((unsigned int)fixpoint_mul(a11a, dir_x) >> 16) - fixpoint_mul(v15, face->pFacePlane_old.vNormal.x);
	HIWORD(a7a) = Y + ((unsigned int)fixpoint_mul(a11a, dir_y) >> 16) - fixpoint_mul(v15, face->pFacePlane_old.vNormal.y);
	if (!sub_4759C9(face, a10, a7a, (short)Z + ((unsigned int)fixpoint_mul(a11a, dir_z) >> 16) - fixpoint_mul(v15, face->pFacePlane_old.vNormal.z)))
		return false;
	*a2 = a11a >> 16;
	if (a11a >> 16 < 0)
		*a2 = 0;
	return true;
}

//----- (00475665) --------------------------------------------------------
int sub_475665(BLVFace *face, int a2, __int16 a3)
{
	bool v16; // edi@14
	signed int v20; // ebx@18
	int v21; // edi@20
	signed int v22; // ST14_4@22
	signed __int64 v23; // qtt@22
	signed int result; // eax@25
	int v25; // [sp+14h] [bp-10h]@14
	int v26; // [sp+1Ch] [bp-8h]@2
	signed int v27; // [sp+20h] [bp-4h]@2
	signed int v28; // [sp+30h] [bp+Ch]@2
	signed int v29; // [sp+30h] [bp+Ch]@7
	signed int v30; // [sp+30h] [bp+Ch]@11
	signed int v31; // [sp+30h] [bp+Ch]@14

	if (face->uAttributes & FACE_XY_PLANE)
	{
		v26 = (signed __int16)a2;
		v27 = SHIWORD(a2);
		if (face->uNumVertices)
		{
			for (v28 = 0; v28 < face->uNumVertices; v28++)
			{
				word_720C10_intercepts_xs[2 * v28] = face->pXInterceptDisplacements[v28] + pIndoor->pVertices[face->pVertexIDs[v28]].x;
				word_720B40_intercepts_zs[2 * v28] = face->pYInterceptDisplacements[v28] + pIndoor->pVertices[face->pVertexIDs[v28]].y;
				word_720C10_intercepts_xs[2 * v28 + 1] = face->pXInterceptDisplacements[v28 + 1] + pIndoor->pVertices[face->pVertexIDs[v28 + 1]].x;
				word_720B40_intercepts_zs[2 * v28 + 1] = face->pYInterceptDisplacements[v28 + 1] + pIndoor->pVertices[face->pVertexIDs[v28 + 1]].y;
			}
		}
	}
	else
	{
		if (face->uAttributes & FACE_XZ_PLANE)
		{
			v26 = (signed __int16)a2;
			v27 = a3;
			if (face->uNumVertices)
			{
				for (v29 = 0; v29 < face->uNumVertices; v29++)
				{
					word_720C10_intercepts_xs[2 * v29] = face->pXInterceptDisplacements[v29] + pIndoor->pVertices[face->pVertexIDs[v29]].x;
					word_720B40_intercepts_zs[2 * v29] = face->pZInterceptDisplacements[v29] + pIndoor->pVertices[face->pVertexIDs[v29]].z;
					word_720C10_intercepts_xs[2 * v29 + 1] = face->pXInterceptDisplacements[v29 + 1] + pIndoor->pVertices[face->pVertexIDs[v29 + 1]].x;
					word_720B40_intercepts_zs[2 * v29 + 1] = face->pZInterceptDisplacements[v29 + 1] + pIndoor->pVertices[face->pVertexIDs[v29 + 1]].z;
				}
			}
		}
		else
		{
			v26 = SHIWORD(a2);
			v27 = a3;
			if (face->uNumVertices)
			{
				for (v30 = 0; v30 < face->uNumVertices; v30++)
				{
					word_720C10_intercepts_xs[2 * v30] = face->pYInterceptDisplacements[v30] + pIndoor->pVertices[face->pVertexIDs[v30]].y;
					word_720B40_intercepts_zs[2 * v30] = face->pZInterceptDisplacements[v30] + pIndoor->pVertices[face->pVertexIDs[v30]].z;
					word_720C10_intercepts_xs[2 * v30 + 1] = face->pYInterceptDisplacements[v30 + 1] + pIndoor->pVertices[face->pVertexIDs[v30 + 1]].y;
					word_720B40_intercepts_zs[2 * v30 + 1] = face->pZInterceptDisplacements[v30 + 1] + pIndoor->pVertices[face->pVertexIDs[v30 + 1]].z;
				}
			}
		}
	}
	v31 = 0;
	word_720C10_intercepts_xs[2 * face->uNumVertices] = word_720C10_intercepts_xs[0];
	word_720B40_intercepts_zs[2 * face->uNumVertices] = word_720B40_intercepts_zs[0];
	v16 = word_720B40_intercepts_zs[0] >= v27;
	if (2 * face->uNumVertices <= 0)
		return 0;
	for (v25 = 0; v25 < 2 * face->uNumVertices; ++v25)
	{
		if (v31 >= 2)
			break;
		if (v16 ^ (word_720B40_intercepts_zs[v25 + 1] >= v27))
		{
			if (word_720C10_intercepts_xs[v25 + 1] >= v26)
				v20 = 0;
			else
				v20 = 2;
			v21 = v20 | (word_720C10_intercepts_xs[v25] < v26);
			if (v21 != 3)
			{
				v22 = word_720C10_intercepts_xs[v25 + 1] - word_720C10_intercepts_xs[v25];
				LODWORD(v23) = v22 << 16;
				HIDWORD(v23) = v22 >> 16;
				if (!v21
					|| (word_720C10_intercepts_xs[v25] + ((signed int)(((unsigned __int64)(v23
					/ (word_720B40_intercepts_zs[v25 + 1] - word_720B40_intercepts_zs[v25])
					* ((v27 - (signed int)word_720B40_intercepts_zs[v25]) << 16)) >> 16)
					+ 32768) >> 16) >= v26))
					++v31;
			}
		}
		v16 = word_720B40_intercepts_zs[v25 + 1] >= v27;
	}
	result = 1;
	if (v31 != 1)
		result = 0;
	return result;
}

//----- (004759C9) --------------------------------------------------------
bool sub_4759C9(BLVFace *face, int a2, int a3, __int16 a4)
{
	bool v12; // edi@14
	signed int v16; // ebx@18
	int v17; // edi@20
	signed int v18; // ST14_4@22
	signed __int64 v19; // qtt@22
	bool result; // eax@25
	int v21; // [sp+14h] [bp-10h]@14
	signed int v22; // [sp+18h] [bp-Ch]@1
	int v23; // [sp+1Ch] [bp-8h]@2
	signed int v24; // [sp+20h] [bp-4h]@2
	signed int a4d; // [sp+30h] [bp+Ch]@14

	if (face->uAttributes & FACE_XY_PLANE)
	{
		v23 = (signed __int16)a3;
		v24 = SHIWORD(a3);
		if (face->uNumVertices)
		{
			for (v22 = 0; v22 < face->uNumVertices; ++v22)
			{
				word_720A70_intercepts_xs_plus_xs[2 * v22] = face->pXInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].x);
				word_7209A0_intercepts_ys_plus_ys[2 * v22] = face->pYInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].y);
				word_720A70_intercepts_xs_plus_xs[2 * v22 + 1] = face->pXInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].x);
				word_7209A0_intercepts_ys_plus_ys[2 * v22 + 1] = face->pYInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].y);
			}
		}
	}
	else
	{
		if (face->uAttributes & FACE_XZ_PLANE)
		{
			v23 = (signed __int16)a3;
			v24 = a4;
			if (face->uNumVertices)
			{
				for (v22 = 0; v22 < face->uNumVertices; ++v22)
				{
					word_720A70_intercepts_xs_plus_xs[2 * v22] = face->pXInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].x);
					word_7209A0_intercepts_ys_plus_ys[2 * v22] = face->pZInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].z);
					word_720A70_intercepts_xs_plus_xs[2 * v22 + 1] = face->pXInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].x);
					word_7209A0_intercepts_ys_plus_ys[2 * v22 + 1] = face->pZInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].z);
				}
			}
		}
		else
		{
			v23 = SHIWORD(a3);
			v24 = a4;
			if (face->uNumVertices)
			{
				for (v22 = 0; v22 < face->uNumVertices; ++v22)
				{
					word_720A70_intercepts_xs_plus_xs[2 * v22] = face->pYInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].y);
					word_7209A0_intercepts_ys_plus_ys[2 * v22] = face->pZInterceptDisplacements[v22] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22]].z);
					word_720A70_intercepts_xs_plus_xs[2 * v22 + 1] = face->pYInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].y);
					word_7209A0_intercepts_ys_plus_ys[2 * v22 + 1] = face->pZInterceptDisplacements[v22 + 1] + LOWORD(pOutdoor->pBModels[a2].pVertices.pVertices[face->pVertexIDs[v22 + 1]].z);
				}
			}
		}
	}
	a4d = 0;
	word_720A70_intercepts_xs_plus_xs[2 * face->uNumVertices] = word_720A70_intercepts_xs_plus_xs[0];
	word_7209A0_intercepts_ys_plus_ys[2 * face->uNumVertices] = word_7209A0_intercepts_ys_plus_ys[0];
	v12 = word_7209A0_intercepts_ys_plus_ys[0] >= v24;
	if (2 * face->uNumVertices <= 0)
		return 0;
	for (v21 = 0; v21 < 2 * face->uNumVertices; ++v21)
	{
		if (a4d >= 2)
			break;
		if (v12 ^ (word_7209A0_intercepts_ys_plus_ys[v21 + 1] >= v24))
		{
			if (word_720A70_intercepts_xs_plus_xs[v21 + 1] >= v23)
				v16 = 0;
			else
				v16 = 2;
			v17 = v16 | (word_720A70_intercepts_xs_plus_xs[v21] < v23);
			if (v17 != 3)
			{
				v18 = word_720A70_intercepts_xs_plus_xs[v21 + 1] - word_720A70_intercepts_xs_plus_xs[v21];
				LODWORD(v19) = v18 << 16;
				HIDWORD(v19) = v18 >> 16;
				if (!v17
					|| (word_720A70_intercepts_xs_plus_xs[v21] + ((signed int)(((unsigned __int64)(v19
					/ (word_7209A0_intercepts_ys_plus_ys[v21 + 1] - word_7209A0_intercepts_ys_plus_ys[v21])
					* ((v24 - (signed int)word_7209A0_intercepts_ys_plus_ys[v21]) << 16)) >> 16)
					+ 0x8000) >> 16) >= v23))
					++a4d;
			}
		}
		v12 = word_7209A0_intercepts_ys_plus_ys[v21 + 1] >= v24;
	}
	result = 1;
	if (a4d != 1)
		result = 0;
	return result;
}

//----- (00475D85) --------------------------------------------------------
bool sub_475D85(Vec3_int_ *a1, Vec3_int_ *a2, int *a3, BLVFace *a4)
{
	BLVFace *v4; // ebx@1
	int v5; // ST24_4@2
	int v6; // ST28_4@2
	int v7; // edi@2
	int v8; // eax@5
	signed int v9; // esi@5
	signed __int64 v10; // qtt@10
	Vec3_int_ *v11; // esi@11
	int v12; // ST14_4@11
	Vec3_int_ *v14; // [sp+Ch] [bp-18h]@1
	Vec3_int_ *v15; // [sp+14h] [bp-10h]@1
	//  int v16; // [sp+18h] [bp-Ch]@2
	int v17; // [sp+20h] [bp-4h]@10
	int a4b; // [sp+30h] [bp+Ch]@2
	int a4c; // [sp+30h] [bp+Ch]@9
	signed int a4a; // [sp+30h] [bp+Ch]@10

	v4 = a4;
	v15 = a2;
	v14 = a1;
	v5 = fixpoint_mul(a2->x, a4->pFacePlane_old.vNormal.x);
	a4b = fixpoint_mul(a2->y, a4->pFacePlane_old.vNormal.y);
	v6 = fixpoint_mul(a2->z, v4->pFacePlane_old.vNormal.z);
	v7 = v5 + v6 + a4b;
	//(v16 = v5 + v6 + a4b) == 0;
	if (a4->uAttributes & FACE_ETHEREAL || !v7 || v7 > 0 && !v4->Portal())
		return 0;
	v8 = v4->pFacePlane_old.vNormal.z * a1->z;
	v9 = -(v4->pFacePlane_old.dist + v8 + a1->y * v4->pFacePlane_old.vNormal.y + a1->x * v4->pFacePlane_old.vNormal.x);
	if (v7 <= 0)
	{
		if (v4->pFacePlane_old.dist + v8 + a1->y * v4->pFacePlane_old.vNormal.y + a1->x * v4->pFacePlane_old.vNormal.x < 0)
			return 0;
	}
	else
	{
		if (v9 < 0)
			return 0;
	}
	a4c = abs(-(v4->pFacePlane_old.dist + v8 + a1->y * v4->pFacePlane_old.vNormal.y + a1->x * v4->pFacePlane_old.vNormal.x)) >> 14;
	v11 = v14;
	LODWORD(v10) = v9 << 16;
	HIDWORD(v10) = v9 >> 16;
	a4a = v10 / v7;
	v17 = v10 / v7;
	LOWORD(v12) = LOWORD(v14->x) + (((unsigned int)fixpoint_mul(v17, v15->x) + 0x8000) >> 16);
	HIWORD(v12) = LOWORD(v11->y) + (((unsigned int)fixpoint_mul(v17, v15->y) + 0x8000) >> 16);
	if (a4c > abs(v7) || (v17 > *a3 << 16) || !sub_475665(v4, v12, LOWORD(v11->z) + (((unsigned int)fixpoint_mul(v17, v15->z) + 0x8000) >> 16)))
		return 0;
	*a3 = a4a >> 16;
	return 1;
}

//----- (00475F30) --------------------------------------------------------
bool sub_475F30(int *a1, BLVFace *a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
	int v10; // ST20_4@2
	int v11; // ST28_4@2
	int v12; // ST24_4@2
	int v13; // zf@2
	int v14; // edi@2
	signed int v16; // esi@5
	int v17; // ST20_4@9
	signed __int64 v18; // qtt@10
	int v19; // ST14_4@11
	int v22; // [sp+1Ch] [bp-8h]@2
	int v23; // [sp+1Ch] [bp-8h]@10
	signed int v24; // [sp+20h] [bp-4h]@10

	v10 = fixpoint_mul(a6, a2->pFacePlane_old.vNormal.x);
	v11 = fixpoint_mul(a7, a2->pFacePlane_old.vNormal.y);
	v12 = fixpoint_mul(a8, a2->pFacePlane_old.vNormal.z);
	v13 = v10 + v12 + v11;
	v14 = v10 + v12 + v11;
	v22 = v10 + v12 + v11;
	if (a2->Ethereal() || !v13 || v14 > 0 && !a2->Portal())
		return 0;
	v16 = -(a2->pFacePlane_old.dist + a4 * a2->pFacePlane_old.vNormal.y + a3 * a2->pFacePlane_old.vNormal.x + a5 * a2->pFacePlane_old.vNormal.z);
	if (v14 <= 0)
	{
		if (a2->pFacePlane_old.dist + a4 * a2->pFacePlane_old.vNormal.y + a3 * a2->pFacePlane_old.vNormal.x + a5 * a2->pFacePlane_old.vNormal.z < 0)
			return 0;
	}
	else
	{
		if (v16 < 0)
			return 0;
	}
	v17 = abs(-(a2->pFacePlane_old.dist + a4 * a2->pFacePlane_old.vNormal.y + a3 * a2->pFacePlane_old.vNormal.x + a5 * a2->pFacePlane_old.vNormal.z)) >> 14;
	LODWORD(v18) = v16 << 16;
	HIDWORD(v18) = v16 >> 16;
	v24 = v18 / v22;
	v23 = v18 / v22;
	LOWORD(v19) = a3 + (((unsigned int)fixpoint_mul(v23, a6) + 0x8000) >> 16);
	HIWORD(v19) = a4 + (((unsigned int)fixpoint_mul(v23, a7) + 0x8000) >> 16);
	if (v17 > abs(v14) || v23 > *a1 << 16 || !sub_4759C9(a2, a9, v19, a5 + (((unsigned int)fixpoint_mul(v23, a8) + 0x8000) >> 16)))
		return 0;
	*a1 = v24 >> 16;
	return 1;
}

//----- (00479089) --------------------------------------------------------
bool IsBModelVisible(unsigned int uModelID, int *reachable)
{
	int v3; // edi@1
	int v4; // ebx@1
	int v9; // eax@3
	signed int v11; // esi@6
	int v12; // esi@8
	bool result; // eax@9
	int v17; // [sp+1Ch] [bp-10h]@1
	int v19; // [sp+20h] [bp-Ch]@3
	int angle; // [sp+24h] [bp-8h]@1

	angle = (signed int)(pODMRenderParams->uCameraFovInDegrees << 11) / 360 / 2;
	v3 = pOutdoor->pBModels[uModelID].vBoundingCenter.x - pIndoorCameraD3D->vPartyPos.x;
	v4 = pOutdoor->pBModels[uModelID].vBoundingCenter.y - pIndoorCameraD3D->vPartyPos.y;
	stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX);
	v17 = v3 * stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY) + v4 * stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY);
	if (pIndoorCameraD3D->sRotationX)
		v17 = fixpoint_mul(v17, stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX));
	v19 = v4 * stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY) - v3 * stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY);
	v9 = int_get_vector_length(abs(v3), abs(v4), 0);
	//v10 = v14 * 188;
	//v22 = v9;
	*reachable = false;
	if (v9 < pOutdoor->pBModels[uModelID].sBoundingRadius + 256)
		*reachable = true;
	if (v19 >= 0)
		v11 = fixpoint_mul(stru_5C6E00->Sin(angle), v17) - fixpoint_mul(stru_5C6E00->Cos(angle), v19);
	else
		v11 = fixpoint_mul(stru_5C6E00->Cos(angle), v19) + fixpoint_mul(stru_5C6E00->Sin(angle), v17);
	v12 = v11 >> 16;
	if (v9 <= pODMRenderParams->shading_dist_mist + 2048)
	{
		//if ( abs(v12) > *(int *)((char *)&pOutdoor->pBModels->sBoundingRadius + v10) + 512 )
		if (abs(v12) > pOutdoor->pBModels[uModelID].sBoundingRadius + 512)
		{
			result = v12 < 0;
			LOBYTE(result) = v12 >= 0;
			return result;
		}
		else
			return true;
	}
	return false;
}

//----- (00479295) --------------------------------------------------------
int Polygon::_479295()
{
	int v3; // ecx@4
	int v4; // eax@4
	int v5; // edx@4
	//  int v6; // ST14_4@5
	Vec3_int_ thisa; // [sp+Ch] [bp-10h]@8
	int v11; // [sp+18h] [bp-4h]@4

	if (!this->pODMFace->pFacePlane.vNormal.z)
	{
		v3 = this->pODMFace->pFacePlane.vNormal.x;
		v4 = -this->pODMFace->pFacePlane.vNormal.y;
		v5 = 0;
		v11 = 65536;
	}
	else if ((this->pODMFace->pFacePlane.vNormal.x || this->pODMFace->pFacePlane.vNormal.y)
		&& abs(this->pODMFace->pFacePlane.vNormal.z) < 59082)
	{
		thisa.x = -this->pODMFace->pFacePlane.vNormal.y;
		thisa.y = this->pODMFace->pFacePlane.vNormal.x;
		thisa.z = 0;
		thisa.Normalize_float();
		v4 = thisa.x;
		v3 = thisa.y;
		v5 = 0;
		v11 = 65536;
	}
	else
	{
		v3 = 0;
		v4 = 65536;
		v11 = 0;
		v5 = -65536;
	}
	sTextureDeltaU = this->pODMFace->sTextureDeltaU;
	sTextureDeltaV = this->pODMFace->sTextureDeltaV;
	ptr_38->_48616B_frustum_odm(v4, v3, 0, 0, v5, v11);
	return 1;
}


unsigned short *LoadTgaTexture(const wchar_t *filename, int *out_width = nullptr, int *out_height = nullptr)
{
#pragma pack(push, 1)
	struct TGAHeader
	{
		unsigned char  tgaSkip;
		unsigned char  colourmaptype;      // type of colour map 0=none, 1=has palette
		unsigned char  tgaType;            // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

		short colourmapstart;     // first colour map entry in palette
		short colourmaplength;    // number of colours in palette
		char  colourmapbits;      // number of bits per palette entry 15,16,24,32

		//unsigned char  tgaDontCare2[9];
		short xstart;             // image x origin
		short ystart;             // image y origin

		unsigned short tgaWidth;
		unsigned short tgaHeight;
		unsigned char  tgaBPP;

		char  descriptor;         // image descriptor bits:   00vhaaaa
		//      h horizontal flip
		//      v vertical flip
		//      a alpha bits
	};
#pragma pack(pop)

	if (out_width)
		*out_width = 0;
	if (out_height)
		*out_height = 0;

	DWORD w;
	void*  file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return nullptr;

	TGAHeader header;
	ReadFile(file, &header, sizeof(header), &w, nullptr);
	SetFilePointer(file, header.tgaSkip, nullptr, FILE_CURRENT);

	if (header.tgaBPP != 24 || header.tgaType != 2)
	{
		CloseHandle(file);
		return nullptr;
	}

	int imgSize = header.tgaWidth * header.tgaHeight * 3;
	unsigned char* pixels = new unsigned char[imgSize];
	ReadFile(file, pixels, imgSize, &w, nullptr);
	CloseHandle(file);

	if (w != imgSize)
	{
		delete[] pixels;
		return nullptr;
	}

	if (out_width)
		*out_width = header.tgaWidth;
	if (out_height)
		*out_height = header.tgaHeight;

	unsigned short* pixels_16bit = new unsigned short[imgSize / 3];
	for (int i = 0; i < imgSize / 3; ++i)
	{
		pixels_16bit[i] = (pixels[i * 3] / 8 & 0x1F) |
			((pixels[i * 3 + 1] / 4 & 0x3F) << 5) |
			((pixels[i * 3 + 2] / 8 & 0x1F) << 11);
	}
	delete[] pixels;
	return pixels_16bit;
}

unsigned short *skybox_xn, *skybox_xp,
*skybox_yn, *skybox_yp,
*skybox_zn, *skybox_zp;
int            skybox_width, skybox_height;

IDirect3DTexture2   *skybox_texture;
IDirectDrawSurface4 *skybox_surface;

bool Skybox_Initialize(const wchar_t *skybox_name)
{
	wchar_t xn_filename[1024], xp_filename[1024],
		yn_filename[1024], yp_filename[1024],
		zn_filename[1024], zp_filename[1024];
	swprintf(xn_filename, wcslen(L"%s_xn.tga"), L"%s_xn.tga", skybox_name); swprintf(xp_filename, wcslen(L"%s_xp.tga"), L"%s_xp.tga", skybox_name);
	swprintf(yn_filename, wcslen(L"%s_yn.tga"), L"%s_yn.tga", skybox_name); swprintf(yp_filename, wcslen(L"%s_yp.tga"), L"%s_yp.tga", skybox_name);
	swprintf(zn_filename, wcslen(L"%s_zn.tga"), L"%s_zn.tga", skybox_name); swprintf(zp_filename, wcslen(L"%s_zp.tga"), L"%s_zp.tga", skybox_name);

	int xn_width, xn_height;
	skybox_xn = LoadTgaTexture(xn_filename, &xn_width, &xn_height);
	if (!skybox_xn)
		return false;

	int xp_width, xp_height;
	skybox_xp = LoadTgaTexture(xp_filename, &xp_width, &xp_height);
	if (!skybox_xp || xp_width != xn_width || xp_height != xn_height)
	{
		delete[] skybox_xn;
		delete[] skybox_xp;
		return false;
	}

	int yn_width, yn_height;
	skybox_yn = LoadTgaTexture(yn_filename, &yn_width, &yn_height);
	if (!skybox_yn || yn_width != xn_width || yn_height != xn_height)
	{
		delete[] skybox_xn;
		delete[] skybox_xp;
		delete[] skybox_yn;
		return false;
	}

	int yp_width, yp_height;
	skybox_yp = LoadTgaTexture(yp_filename, &yp_width, &yp_height);
	if (!skybox_yp || yp_width != xn_width || yp_height != xn_height)
	{
		delete[] skybox_xn;
		delete[] skybox_xp;
		delete[] skybox_yn;
		delete[] skybox_yp;
		return false;
	}

	int zn_width, zn_height;
	skybox_zn = LoadTgaTexture(zn_filename, &zn_width, &zn_height);
	if (!skybox_zn || zn_width != xn_width || zn_height != xn_height)
	{
		delete[] skybox_xn;
		delete[] skybox_xp;
		delete[] skybox_yn;
		delete[] skybox_yp;
		delete[] skybox_zn;
		return false;
	}

	int zp_width, zp_height;
	skybox_zp = LoadTgaTexture(zp_filename, &zp_width, &zp_height);
	if (!skybox_zp || zp_width != xn_width || zp_height != xn_height)
	{
		delete[] skybox_xn;
		delete[] skybox_xp;
		delete[] skybox_yn;
		delete[] skybox_yp;
		delete[] skybox_zn;
		delete[] skybox_zp;
		return false;
	}

	skybox_width = xn_width;
	skybox_height = xn_height;

	__debugbreak();
	//if (!render->pRenderD3D->CreateTexture(skybox_width, skybox_height, &skybox_surface, &skybox_texture,
	//false, false, render->uMinDeviceTextureDim))
	return false;

	return true;
}

struct vector
{
	float x, y, z;
};

struct matrix
{
	float m[4][4];
};

void VectorNormalize(vector *v)
{
	float invmag = 1.0f / sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
	v->x *= invmag;
	v->y *= invmag;
	v->z *= invmag;
}

void MatrixRotationAxis(matrix *pout, CONST vector *pv, float angle)
{
	memset(pout, 0, sizeof(matrix));
	pout->m[3][0] = 0;
	pout->m[3][1] = 0;
	pout->m[3][2] = 0;
	pout->m[3][3] = 1;

	vector v;
	v.x = pv->x; v.y = pv->y; v.z = pv->z;
	VectorNormalize(&v);

	pout->m[0][0] = (1.0f - cos(angle)) * v.x * v.x + cos(angle);
	pout->m[1][0] = (1.0f - cos(angle)) * v.x * v.y - sin(angle) * v.z;
	pout->m[2][0] = (1.0f - cos(angle)) * v.x * v.z + sin(angle) * v.y;
	pout->m[0][1] = (1.0f - cos(angle)) * v.y * v.x + sin(angle) * v.z;
	pout->m[1][1] = (1.0f - cos(angle)) * v.y * v.y + cos(angle);
	pout->m[2][1] = (1.0f - cos(angle)) * v.y * v.z - sin(angle) * v.x;
	pout->m[0][2] = (1.0f - cos(angle)) * v.z * v.x - sin(angle) * v.y;
	pout->m[1][2] = (1.0f - cos(angle)) * v.z * v.y + sin(angle) * v.x;
	pout->m[2][2] = (1.0f - cos(angle)) * v.z * v.z + cos(angle);
}

void VectorTransform(const matrix *m, const vector *v, vector *out)
{
	out->x = m->m[0][0] * v->x + m->m[1][0] * v->y + m->m[2][0] * v->z + m->m[3][0];
	out->y = m->m[0][1] * v->x + m->m[1][1] * v->y + m->m[2][1] * v->z + m->m[3][1];
	out->z = m->m[0][2] * v->x + m->m[1][2] * v->y + m->m[2][2] * v->z + m->m[3][2];
}

bool DrawSkyD3D_Skybox()
{
	static bool initialized = false,
		initialization_failed = false;
	if (initialization_failed)
		return false;

	static int last_camera_rot_y,
		last_camera_rot_x;
	if (!initialized)
	{
		if (!Skybox_Initialize(L"data/skybox/stars"))
		{
			initialization_failed = true;
			return false;
		}
		initialized = true;

		last_camera_rot_y = pParty->sRotationY + 1; // force update for the first run 
		last_camera_rot_x = pParty->sRotationX + 1;
	}

	/*
	r(y) =
	cos y	0	sin y	0
	0	1	0	0
	-sin y	0	cos y	0
	0	0	0	1

	x cos y - z sin y
	y
	x sin y + z cos y
	1



	r(x) =     // should be r(right) actually
	1	0      	0	0
	0	cos x	-sin x	0
	0	sin x	cos x	0
	0	0	    0	1


	x
	y cos x + z sin x
	-y sin x + z cos x
	1

	*/

	if (last_camera_rot_y == pParty->sRotationY &&
		last_camera_rot_x == pParty->sRotationX)
	{
	draw:
		struct RenderVertexD3D3  v[6];

		v[0].pos.x = pViewport->uScreen_TL_X;
		v[0].pos.y = pViewport->uScreen_TL_Y;
		v[0].pos.z = 0.99989998;
		v[0].rhw = 1;
		v[0].diffuse = -1;
		v[0].specular = 0;
		v[0].texcoord.x = 0;
		v[0].texcoord.y = 0;

		v[1].pos.x = pViewport->uScreen_TL_X + pViewport->uScreenWidth;
		v[1].pos.y = pViewport->uScreen_TL_Y + pViewport->uScreenHeight;
		v[1].pos.z = 0.99989998;
		v[1].rhw = 1;
		v[1].diffuse = -1;
		v[1].specular = 0;
		v[1].texcoord.x = (float)pViewport->uScreenWidth / skybox_width;
		v[1].texcoord.y = (float)pViewport->uScreenHeight / skybox_height;

		v[2].pos.x = pViewport->uScreen_TL_X + pViewport->uScreenWidth;
		v[2].pos.y = pViewport->uScreen_TL_Y;
		v[2].pos.z = 0.99989998;
		v[2].rhw = 1;
		v[2].diffuse = -1;
		v[2].specular = 0;
		v[2].texcoord.x = (float)pViewport->uScreenWidth / skybox_width;
		v[2].texcoord.y = 0;

		memcpy(&v[3], &v[0], sizeof(*v));

		v[4].pos.x = pViewport->uScreen_TL_X;
		v[4].pos.y = pViewport->uScreen_TL_Y + pViewport->uScreenHeight;
		v[4].pos.z = 0.99989998;
		v[4].rhw = 1;
		v[4].diffuse = -1;
		v[4].specular = 0;
		v[4].texcoord.x = 0;
		v[4].texcoord.y = (float)pViewport->uScreenHeight / skybox_height;

		memcpy(&v[5], &v[1], sizeof(*v));

		__debugbreak();
		/*
		render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
		render->pRenderD3D->pDevice->SetTexture(0, skybox_texture);
		render->pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, v, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
		*/
		return true;
	}


	DDSURFACEDESC2 desc;
	desc.dwSize = sizeof(desc);
	if (!render->LockSurface_DDraw4(skybox_surface, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY))
		return false;

	last_camera_rot_y = pParty->sRotationY;
	last_camera_rot_x = pParty->sRotationX;

	float aspect = (float)pViewport->uScreenWidth / (float)pViewport->uScreenHeight;
	float fov_x = 3.141592f * (pODMRenderParams->uCameraFovInDegrees + 0) / 360.0f;
	float fov_y = fov_x / aspect;

	float ray_dx = fov_x / (float)pViewport->uScreenWidth,
		ray_dy = fov_y / (float)pViewport->uScreenHeight;
	float party_angle_x = 2 * pi_double * pParty->sRotationX / 2048.0,
		party_angle_y = 2 * pi_double * pParty->sRotationY / 2048.0;
	for (int y = 0; y < pViewport->uScreenHeight; ++y)
	for (int x = 0; x < pViewport->uScreenWidth; ++x)
	{
		float angle_x = party_angle_x - (y - pViewport->uScreenHeight / 2) * ray_dy;
		float angle_y = party_angle_y - (x - pViewport->uScreenWidth / 2) * ray_dx;

		float _dir_x_ = 1,
			_dir_y_ = 0,
			_dir_z_ = 0;

		float dir_x_ = _dir_x_ * cosf(angle_y);// - _dir_z_ * sinf(angle_y);  // rotation around y
		//float dir_y_ = _dir_y_;
		float dir_z_ = _dir_x_ * sinf(angle_y);// + _dir_z_ * cosf(angle_y);

		//float dir_x =  dir_x_;                                               // rotation around x
		//float dir_y =  /*dir_y_ * cosf(angle_x)*/ + dir_z_ * sinf(angle_x);
		//float dir_z = /*-dir_y_ * sinf(angle_x)*/ + dir_z_ * cosf(angle_x);

		vector right;                                            // rotate around right actually to avoid space distortion
		right.x = /*dir_y * 0*/ -dir_z_ * 1;
		right.y = /*dir_z_ * 0 - dir_x_ * */0;
		right.z = dir_x_ * 1/* - dir_y_ * 0*/;
		//VectorNormalize(&right);

		matrix rightMatrix;
		MatrixRotationAxis(&rightMatrix, &right, angle_x);

		vector v1, v2;
		v1.x = dir_x_; v1.y = 0; v1.z = dir_z_;
		VectorTransform(&rightMatrix, &v1, &v2);

		float dir_x = v2.x,
			dir_y = v2.y,
			dir_z = v2.z;

		float abs_dir_x = fabsf(dir_x),
			abs_dir_y = fabsf(dir_y),
			abs_dir_z = fabsf(dir_z);

		unsigned short color = (0x1F << 11) | (0x1F << 5) | (5);  //default to orange
		if (abs_dir_x >= abs_dir_y)
		{
			if (abs_dir_x >= abs_dir_z)
			{
				if (dir_x >= 0)
				{
					float instersect_y = dir_y / (2.0f * dir_x); // plane equation for this side is x + 0.5 = 0
					float instersect_z = dir_z / (2.0f * dir_x);

					float u = 1.0f - (instersect_z + 0.5f),
						v = 1.0f - (instersect_y + 0.5f);

					int tx = u * (skybox_width - 1),
						ty = v * (skybox_height - 1);

					color = skybox_xp[ty * skybox_width + tx];
					//color = ty * 0x1F / skybox_height;
				}
				else
				{
					float instersect_y = dir_y / (2.0f * dir_x);
					float instersect_z = dir_z / (2.0f * dir_x);

					float u = 1.0f - (instersect_z + 0.5f),
						v = instersect_y + 0.5f;

					int tx = u * (skybox_width - 1),
						ty = v * (skybox_height - 1);

					color = skybox_xn[ty * skybox_width + tx];
					//color = tx * 0x1F / skybox_height;
				}
			}
			else if (dir_z >= 0)
				goto DIR_ZP;
			else
				goto DIR_ZN;
		}
		else if (abs_dir_y >= abs_dir_z)
		{
			if (dir_y >= 0)
			{
				float instersect_x = dir_x / (2.0f * dir_y);
				float instersect_z = dir_z / (2.0f * dir_y);

				float u = instersect_x + 0.5f,
					v = instersect_z + 0.5f;

				int tx = u * (skybox_width - 1),
					ty = v * (skybox_height - 1);

				color = skybox_yp[ty * skybox_width + tx];
				//color = tx * 0x1F / skybox_height;
			}
			/*else   should never be seen i guess
			{
			__debugbreak();
			// -y
			//Log::Warning(L"(%03u, %03u): -y", x, y);
			}*/
		}
		else if (dir_z >= 0)
		{
		DIR_ZP:
			// +z
			float instersect_x = dir_x / (2.0f * dir_z);
			float instersect_y = dir_y / (2.0f * dir_z);
			//float intersect_z = 0.5f;

			float u = instersect_x + 0.5f,
				v = -instersect_y + 0.5f;

			int tx = u * (skybox_width - 1),
				ty = v * (skybox_height - 1);

			color = skybox_zp[ty * skybox_width + tx];
		}
		else
		{
		DIR_ZN:
			// -z
			float instersect_x = -dir_x / (2.0f * dir_z);
			float instersect_y = -dir_y / (2.0f * dir_z);
			//float intersect_z = -0.5f;

			float u = 1.0f - instersect_x - 0.5f,
				v = -instersect_y + 0.5f;

			int tx = u * (skybox_width - 1),
				ty = v * (skybox_height - 1);

			color = skybox_zn[ty * skybox_width + tx];
		}

		//render->pTargetSurface[(pViewport->uScreenY + y) * render->uTargetSurfacePitch + pViewport->uScreenX + x] = color;
		((unsigned __int16 *)((char *)desc.lpSurface + y * desc.lPitch))[x] = color;
	}

	ErrD3D((skybox_surface)->Unlock(0));
	goto draw;
}

//----- (00485F53) --------------------------------------------------------
void  sr_485F53(Vec2_int_ *v)
{
	++v->y;
	if (v->y > 1000)
		v->y = 0;
}

//----- (0048607B) --------------------------------------------------------
void Polygon::Create_48607B(stru149 *a2)
{
	this->pTexture = 0;
	this->ptr_38 = a2;
}

//----- (00486089) --------------------------------------------------------
void Polygon::_normalize_v_18()
{
	//double v2; // st7@1
	//double v3; // st6@1
	//double v5; // st5@1

	// v2 = (double)this->v_18.x;
	//v3 = (double)this->v_18.y;
	// v5 = (double)this->v_18.z;
	float len = sqrt((double)this->v_18.z * (double)this->v_18.z + (double)this->v_18.y * (double)this->v_18.y + (double)this->v_18.x * (double)this->v_18.x);
	if (fabsf(len) < 1e-6f)
	{
		v_18.x = 0;
		v_18.y = 0;
		v_18.z = 65536;
	}
	else
	{
		v_18.x = round_to_int((double)this->v_18.x / len * 65536.0);
		v_18.y = round_to_int((double)this->v_18.y / len * 65536.0);
		v_18.y = round_to_int((double)this->v_18.z / len * 65536.0);
	}
}

//----- (0048616B) --------------------------------------------------------
void stru149::_48616B_frustum_odm(int a2, int a3, int a4, int a5, int a6, int a7)
{
	int v7; // ebx@1
	int v9; // edi@1
	int v11; // edx@1
	int v17; // ST0C_4@6
	int v19; // ST0C_4@9
	int v24; // [sp+14h] [bp-14h]@1
	int v25; // [sp+18h] [bp-10h]@1
	int v27; // [sp+24h] [bp-4h]@1

	v25 = pIndoorCameraD3D->int_cosine_x;
	v7 = pIndoorCameraD3D->int_sine_y;
	v27 = pIndoorCameraD3D->int_sine_x;
	//v8 = -pIndoorCamera->pos.y;
	v9 = pIndoorCameraD3D->int_cosine_y;
	//v26 = -pIndoorCamera->pos.z;
	v11 = pIndoorCameraD3D->int_cosine_y * -pIndoorCameraD3D->vPartyPos.x + pIndoorCameraD3D->int_sine_y * -pIndoorCameraD3D->vPartyPos.y;
	v24 = pIndoorCameraD3D->int_cosine_y * -pIndoorCameraD3D->vPartyPos.y - pIndoorCameraD3D->int_sine_y * -pIndoorCameraD3D->vPartyPos.x;
	if (pIndoorCameraD3D->sRotationX)
	{
		this->field_0_party_dir_x = fixpoint_mul(v11, pIndoorCameraD3D->int_cosine_x) +
			fixpoint_mul((-pIndoorCameraD3D->vPartyPos.z) << 16, pIndoorCameraD3D->int_sine_x);
		this->field_4_party_dir_y = v24;
		this->field_8_party_dir_z = fixpoint_mul((-pIndoorCameraD3D->vPartyPos.z) << 16, v25) - fixpoint_mul(v11, v27);
	}
	else
	{
		this->field_0_party_dir_x = v11;
		this->field_4_party_dir_y = v24;
		this->field_8_party_dir_z = (-pIndoorCameraD3D->vPartyPos.z) << 16;
	}

	if (pIndoorCameraD3D->sRotationX)
	{
		v17 = fixpoint_mul(a2, v9) + fixpoint_mul(a3, v7);

		this->angle_from_north = fixpoint_mul(v17, v25) + fixpoint_mul(a4, v27);
		this->angle_from_west = fixpoint_mul(a3, v9) - fixpoint_mul(a2, v7);
		this->viewing_angle_from_west_east = fixpoint_mul(a4, v25) - fixpoint_mul(v17, v27);
	}
	else
	{
		this->angle_from_north = fixpoint_mul(a2, v9) + fixpoint_mul(a3, v7);
		this->angle_from_west = fixpoint_mul(a3, v9) - fixpoint_mul(a2, v7);
		this->viewing_angle_from_west_east = a4;
	}

	if (pIndoorCameraD3D->sRotationX)
	{
		v19 = fixpoint_mul(a5, v9) + fixpoint_mul(a6, v7);

		this->angle_from_east = fixpoint_mul(v19, v25) + fixpoint_mul(a7, v27);
		this->angle_from_south = fixpoint_mul(a6, v9) - fixpoint_mul(a5, v7);
		this->viewing_angle_from_north_south = fixpoint_mul(a7, v25) - fixpoint_mul(v19, v27);
	}
	else
	{
		this->angle_from_east = fixpoint_mul(a5, v9) + fixpoint_mul(a6, v7);
		this->angle_from_south = fixpoint_mul(a6, v9) - fixpoint_mul(a5, v7);
		this->viewing_angle_from_north_south = a7;
	}

	this->angle_from_east = -this->angle_from_east;
	this->angle_from_south = -this->angle_from_south;
	this->viewing_angle_from_north_south = -this->viewing_angle_from_north_south;

	this->field_24 = fixpoint_dot(this->angle_from_north, this->field_0_party_dir_x,
		this->angle_from_west, this->field_4_party_dir_y,
		this->viewing_angle_from_west_east, this->field_8_party_dir_z);
	this->field_28 = fixpoint_dot(this->angle_from_east, this->field_0_party_dir_x,
		this->angle_from_south, this->field_4_party_dir_y,
		this->viewing_angle_from_north_south, this->field_8_party_dir_z);
}

//----- (0048694B) --------------------------------------------------------
void stru149::_48694B_frustum_sky()
{
	this->angle_from_east = -this->angle_from_east;
	this->angle_from_south = -this->angle_from_south;
	this->viewing_angle_from_north_south = -this->viewing_angle_from_north_south;

	this->field_24 = fixpoint_dot(this->angle_from_north, this->field_0_party_dir_x,
		this->angle_from_west, this->field_4_party_dir_y,
		this->viewing_angle_from_west_east, this->field_8_party_dir_z);
	this->field_28 = fixpoint_dot(this->angle_from_east, this->field_0_party_dir_x,
		this->angle_from_south, this->field_4_party_dir_y,
		this->viewing_angle_from_north_south, this->field_8_party_dir_z);
}

