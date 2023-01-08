#include "Render.h"

#include <d3d.h>
#include <ddraw.h>

#include <algorithm>

#include "Arcomage/Arcomage.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"

#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Direct3D/RenderD3D.h"
#include "Engine/Graphics/Direct3D/TextureD3D.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time.h"

#include "GUI/GUIWindow.h"

#include "Io/Mouse.h"

#include "Utility/Memory.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Random/Random.h"

#pragma comment(lib, "GdiPlus.lib")

using EngineIoc = Engine_::IocContainer;

struct IDirectDrawClipper *pDDrawClipper;
struct RenderVertexD3D3 pVertices[50];

DDPIXELFORMAT ddpfPrimarySuface;


RenderVertexD3D3 BatchDrawTriangles[10000] {};
struct BatchTriangles {
    RenderVertexD3D3 Verts[3] {};
    Texture* texture = nullptr;
    std::string* texname = nullptr;
    bool trans = 0;
};

BatchTriangles BatchTrianglesStore[10000] {};
BatchTriangles* StorePtr[10000] {};
int NumBatchTrianglesStore = 0;

void ErrHR(HRESULT hr, const char *pAPI, const char *pFunction,
           const char *pFile, int line) {
    if (SUCCEEDED(hr)) {
        return;
    }

    char msg[4096];
    sprintf(msg, "%s error (%08X) in\n\t%s\nin\n\t%s:%u", pAPI, hr, pFunction,
            pFile, line);

    char caption[1024];
    sprintf(caption, "%s error", pAPI);

    Error(msg);
}

Texture *Render::CreateTexture_Paletted(const std::string &name) {
    return TextureD3D::Create(new Paletted_Img_Loader(pIcons_LOD, name, 0));
}

Texture *Render::CreateTexture_ColorKey(const std::string &name, uint16_t colorkey) {
    return TextureD3D::Create(new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey));
}

Texture *Render::CreateTexture_Solid(const std::string &name) {
    return TextureD3D::Create(new Image16bit_LOD_Loader(pIcons_LOD, name));
}

Texture *Render::CreateTexture_Alpha(const std::string &name) {
    return TextureD3D::Create(new Alpha_LOD_Loader(pIcons_LOD, name));
}

Texture *Render::CreateTexture_PCXFromIconsLOD(const std::string &name) {
    return TextureD3D::Create(new PCX_LOD_Compressed_Loader(pIcons_LOD, name));
}

Texture *Render::CreateTexture_PCXFromNewLOD(const std::string &name) {
    return TextureD3D::Create(new PCX_LOD_Compressed_Loader(pNew_LOD, name));
}

Texture *Render::CreateTexture_PCXFromFile(const std::string &name) {
    return TextureD3D::Create(new PCX_File_Loader(name));
}

Texture *Render::CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) {
    return TextureD3D::Create(new PCX_LOD_Raw_Loader(pLOD, name));
}

Texture *Render::CreateTexture_Blank(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels) {

    return TextureD3D::Create(width, height, format, pixels);
}



Texture *Render::CreateTexture(const std::string &name) {
    return TextureD3D::Create(new Bitmaps_LOD_Loader(pBitmaps_LOD, name, engine->config->graphics.HWLBitmaps.Get()));
}

Texture *Render::CreateSprite(const std::string &name, unsigned int palette_id,
                              unsigned int lod_sprite_id) {
    return TextureD3D::Create(
        new Sprites_LOD_Loader(pSprites_LOD, palette_id, name, lod_sprite_id, engine->config->graphics.HWLSprites.Get()));
}

void Render::WritePixel16(int x, int y, uint16_t color) {
    // do not use this - slow
    logger->Info("Reduce use of WritePixel16");
    return;

    unsigned int b = (color & 0x1F) << 3;
    unsigned int g = ((color >> 5) & 0x3F) << 2;
    unsigned int r = ((color >> 11) & 0x1F) << 3;

    Gdiplus::Color c(r, g, b);
    p2DSurface->SetPixel(x, y, c);
}

bool Render::CheckTextureStages() {
    bool bResult = false;

    IDirectDrawSurface4 *pSurface1 = nullptr;
    IDirect3DTexture2 *pTexture1 = nullptr;
    pRenderD3D->CreateTexture(64, 64, &pSurface1, &pTexture1, true, false, 32);
    ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture1));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
                                                     D3DTADDRESS_WRAP));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 1));

    IDirectDrawSurface4 *pSurface2 = nullptr;
    IDirect3DTexture2 *pTexture2 = nullptr;
    pRenderD3D->CreateTexture(64, 64, &pSurface2, &pTexture2, true, false, 32);
    ErrD3D(pRenderD3D->pDevice->SetTexture(0, pTexture2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_ADDRESS,
                                                     D3DTADDRESS_CLAMP));
    ErrD3D(
        pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 7));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, 1));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MINFILTER, 2));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, 1));

    DWORD v4 = 0;  // num passes
    if (SUCCEEDED(pRenderD3D->pDevice->ValidateDevice(&v4)) && v4 == 1) {
        bResult = true;
    }
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1));
    pTexture1->Release();
    pTexture2->Release();
    pSurface1->Release();
    pSurface2->Release();

    return bResult;
}

bool Render::AreRenderSurfacesOk() { return pFrontBuffer4 && pBackBuffer4; }

extern unsigned int BlendColors(unsigned int a1, unsigned int a2);

void Render::DrawTerrainD3D() {  // New function
    // warning: the game uses CW culling by default, ccw is incosistent
    pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);

    static RenderVertexSoft pTerrainVertices[128 * 128] {};  // vertexCountX and vertexCountY

    //Генерация местоположения
    //вершин-------------------------------------------------------------------------
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

    // TODO(pskelton): move these calculations to map load so its not run every frame
    for (int z = 0; z < 128; ++z) {
        for (int x = 0; x < 128; ++x) {
            pTerrainVertices[z * 128 + x].vWorldPosition.x = (-64.0f + x) * blockScale;
            pTerrainVertices[z * 128 + x].vWorldPosition.y = (64.0f - z) * blockScale;
            pTerrainVertices[z * 128 + x].vWorldPosition.z = static_cast<float>(heightScale * pOutdoor->pTerrain.pHeightmap[z * 128 + x]);
        }
    }

    //-------(Отсечение невидимой части
    //карты)------------------------------------------------------------------------------------------
    float direction = (float)(pCamera3D->sRotationZ /
                              256);  // direction of the camera(напрвление
                                     // камеры) 0-East(B) 1-NorthEast(CB)
                                     // 2-North(C)
                                     // 3-WestNorth(CЗ)
                                     // 4-West(З)
                                     // 5-SouthWest(ЮЗ)
                                     // 6-South(Ю)
                                     // 7-SouthEast(ЮВ)
    unsigned int Start_X, End_X, Start_Z, End_Z;
    if (direction >= 0 && direction < 1.0) {  // East(B) - NorthEast(CB)
        Start_X = pODMRenderParams->uMapGridCellX - 2, End_X = 127;
        Start_Z = 0, End_Z = 127;
    } else if (direction >= 1.0 &&
               direction < 3.0) {  // NorthEast(CB) - WestNorth(CЗ)
        Start_X = 0, End_X = 127;
        Start_Z = 0, End_Z = pODMRenderParams->uMapGridCellY + 2;
    } else if (direction >= 3.0 &&
               direction < 5.0) {  // WestNorth(CЗ) - SouthWest(ЮЗ)
        Start_X = 0, End_X = pODMRenderParams->uMapGridCellX + 2;
        Start_Z = 0, End_Z = 127;
    } else if (direction >= 5.0 &&
               direction < 7.0) {  // SouthWest(ЮЗ) - //SouthEast(ЮВ)
        Start_X = 0, End_X = 127;
        Start_Z = pODMRenderParams->uMapGridCellY - 2, End_Z = 127;
    } else {  // SouthEast(ЮВ) - East(B)
        Start_X = pODMRenderParams->uMapGridCellX - 2, End_X = 127;
        Start_Z = 0, End_Z = 127;
    }

    for (unsigned int z = Start_Z; z < End_Z + 1; ++z) {
        for (unsigned int x = Start_X; x < End_X + 1; ++x) {
            pCamera3D->ViewTransform(&pTerrainVertices[z * 128 + x], 1);
            pCamera3D->Project(&pTerrainVertices[z * 128 + x], 1, 0);
        }
    }


    float Light_tile_dist;

    for (unsigned int z = Start_Z; z < End_Z; ++z) {
        for (unsigned int x = Start_X; x < End_X; ++x) {
            struct Polygon *pTilePolygon = &array_77EC08[pODMRenderParams->uNumPolygons];
            pTilePolygon->flags = 0;
            pTilePolygon->field_32 = 0;
            // pTilePolygon->uTileBitmapID = pOutdoor->DoGetTileTexture(x, z);
            // pTilePolygon->pTexture = (Texture_MM7
            // *)&pBitmaps_LOD->pHardwareTextures[pTilePolygon->uTileBitmapID];
            // if (pTilePolygon->uTileBitmapID == 0xFFFF)
            //    continue;
            auto tile = pOutdoor->GetTileDescByGrid(x, z);
            if (!tile) continue;

            // pTile->flags = 0x8010 |pOutdoor->GetTileAttribByPos(x, z);
            pTilePolygon->flags = pOutdoor->GetTileAttribByGrid(x, z);

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

            // verts are CW
            // memcpy(&array_73D150[0], &pTerrainVertices[z * 128 + x],
            //       sizeof(RenderVertexSoft));  // x, z
            // array_73D150[0].u = 0;
            // array_73D150[0].v = 0;
            // memcpy(&array_73D150[1], &pTerrainVertices[z * 128 + x + 1],
            //       sizeof(RenderVertexSoft));  // x + 1, z
            // array_73D150[1].u = 1;
            // array_73D150[1].v = 0;
            // memcpy(&array_73D150[2], &pTerrainVertices[(z + 1) * 128 + x + 1],
            //       sizeof(RenderVertexSoft));  // x + 1, z + 1
            // array_73D150[2].u = 1;
            // array_73D150[2].v = 1;
            // memcpy(&array_73D150[3], &pTerrainVertices[(z + 1) * 128 + x],
            //       sizeof(RenderVertexSoft));  // x, z + 1
            // array_73D150[3].u = 0;
            // array_73D150[3].v = 1;

            // verts CCW - for testing
            array_73D150[0] = pTerrainVertices[z * 128 + x];  // x, z
            array_73D150[0].u = 0;
            array_73D150[0].v = 0;
            array_73D150[3] = pTerrainVertices[z * 128 + x + 1];  // x + 1, z
            array_73D150[3].u = 1;
            array_73D150[3].v = 0;
            array_73D150[2] = pTerrainVertices[(z + 1) * 128 + x + 1];  // x + 1, z + 1
            array_73D150[2].u = 1;
            array_73D150[2].v = 1;
            array_73D150[1] = pTerrainVertices[(z + 1) * 128 + x];  // x, z + 1
            array_73D150[1].u = 0;
            array_73D150[1].v = 1;

            // better tile culling
            pTilePolygon->uNumVertices = 4;
            pCamera3D->CullFaceToCameraFrustum(array_73D150, &pTilePolygon->uNumVertices, array_73D150, 4);
            pCamera3D->CullByNearClip(&array_73D150[0], &pTilePolygon->uNumVertices);
            pCamera3D->CullByFarClip(&array_73D150[0], &pTilePolygon->uNumVertices);
            if (!pTilePolygon->uNumVertices) continue;

            // v58 = 0;
            // if (v58 == 4) // if all y == first y;  primitive in xz plane
            // pTile->field_32 |= 0x0001;
            pTilePolygon->pODMFace = nullptr;
            // pTilePolygon->uNumVertices = 4;
            pTilePolygon->field_59 = 5;


            //----------------------------------------------------------------------------

            ++pODMRenderParams->uNumPolygons;
            // ++pODMRenderParams->field_44;
            assert(pODMRenderParams->uNumPolygons < 20000);

            pTilePolygon->uBModelID = 0;
            pTilePolygon->uBModelFaceID = 0;
            pTilePolygon->pid = (8 * (0 | (0 << 6))) | 6;
            for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                VertexRenderList[k] = array_73D150[k];
                VertexRenderList[k]._rhw = 1.0f / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097f);
            }

            // shading
            // (затенение)----------------------------------------------------------------------------
            // uint norm_idx = pTerrainNormalIndices[2 * (z * 128 + x) + 1];
            uint norm_idx = pTerrainNormalIndices[(2 * x * 128) + (2 * z) + 2 /*+ 1*/ ];  // 2 is top tri // 3 is bottom
            uint bottnormidx = pTerrainNormalIndices[(2 * x * 128) + (2 * z) + 3];

            assert(norm_idx < pTerrainNormals.size());
            assert(bottnormidx < pTerrainNormals.size());

            Vec3f *norm = &pTerrainNormals[norm_idx];
            Vec3f *norm2 = &pTerrainNormals[bottnormidx];

            if (norm_idx < 0 || norm_idx >= pTerrainNormals.size())
                norm = 0;
            else
                norm = &pTerrainNormals[norm_idx];

            if (bottnormidx < 0 || bottnormidx >= pTerrainNormals.size())
                norm2 = 0;
            else
                norm2 = &pTerrainNormals[bottnormidx];


            if (norm_idx != bottnormidx) {
                // we have a split poly - need to apply lights and decals seperately to each half

                pTilePolygon->uNumVertices = 3;

                ///////////// triangle 1 - 1 2 3

                // verts CCW - for testing
                array_73D150[0] = pTerrainVertices[z * 128 + x];  // x, z
                array_73D150[0].u = 0;
                array_73D150[0].v = 0;
                array_73D150[2] = pTerrainVertices[z * 128 + x + 1];  // x + 1, z
                array_73D150[2].u = 1;
                array_73D150[2].v = 0;
                array_73D150[1] = pTerrainVertices[(z + 1) * 128 + x + 1];  // x + 1, z + 1
                array_73D150[1].u = 1;
                array_73D150[1].v = 1;
                // array_73D150[2] = pTerrainVertices[(z + 1) * 128 + x]  // x, z + 1
                // array_73D150[2].u = 0;
                // array_73D150[2].v = 1;

                for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                    VertexRenderList[k] = array_73D150[k];
                    VertexRenderList[k]._rhw = 1.0f / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097f);
                }

                float _f = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0f - floorf(20.0f * _f + 0.5f);

                lightmap_builder->StackLights_TerrainFace(norm, &Light_tile_dist, VertexRenderList, 3, 1);
                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, 3, 1);

                unsigned int a5 = 4;

                // ---------Draw distance(Дальность отрисовки)-------------------------------
                float far_clip_distance = pCamera3D->GetFarClip();
                float near_clip_distance = pCamera3D->GetNearClip();


                bool neer_clip = array_73D150[0].vWorldViewPosition.x < near_clip_distance ||
                    array_73D150[1].vWorldViewPosition.x < near_clip_distance ||
                    array_73D150[2].vWorldViewPosition.x < near_clip_distance;
                bool far_clip =
                    (float)far_clip_distance < array_73D150[0].vWorldViewPosition.x ||
                    (float)far_clip_distance < array_73D150[1].vWorldViewPosition.x ||
                    (float)far_clip_distance < array_73D150[2].vWorldViewPosition.x;

                int uClipFlag = 0;
                static stru154 static_sub_0048034E_stru_154;
                lightmap_builder->StationaryLightsCount = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                    if (neer_clip)
                        uClipFlag = 3;
                    else
                        uClipFlag = far_clip != 0 ? 5 : 0;
                    static_sub_0048034E_stru_154.ClassifyPolygon(norm, Light_tile_dist);

                    if (decal_builder->uNumSplatsThisFace > 0)
                        decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level,
                            LocationTerrain, &static_sub_0048034E_stru_154,
                            3, VertexRenderList,
                            *(float *)&uClipFlag, -1);
                    if (Lights.uNumLightsApplied > 0)
                        lightmap_builder->ApplyLights(
                            &Lights, &static_sub_0048034E_stru_154, 3,
                            VertexRenderList, 0, uClipFlag);
                }







                // pODMRenderParams->shading_dist_mist = temp;

                // check the transparency and texture (tiles) mapping (проверка
                // прозрачности и наложение текстур (тайлов))----------------------
                bool transparent = false;

                auto tile_texture = tile->GetTexture();
                if (!(pTilePolygon->flags & 1)) {
                    // не поддерживается TextureFrameTable
                    if (/*pTile->flags & 2 && */ tile->IsWaterTile()) {
                        tile_texture =
                            this->hd_water_tile_anim[this->hd_water_current_frame];
                    } else if (tile->IsWaterBorderTile()) {
                        // for all shore tiles - draw a tile water under them since
                        // they're half-empty
                        DrawBorderTiles(pTilePolygon);
                        transparent = true;
                    }
                    pTilePolygon->texture = tile_texture;

                    render->DrawTerrainPolygon(pTilePolygon, transparent, true);
                }

                ///////////// triangle 2  0 1 3
                {
                    // verts CCW - for testing
                    array_73D150[0] = pTerrainVertices[z * 128 + x];  // x, z
                    array_73D150[0].u = 0;
                    array_73D150[0].v = 0;
                    // array_73D150[2] = pTerrainVertices[z * 128 + x + 1]  // x + 1, z
                    // array_73D150[2].u = 1;
                    // array_73D150[2].v = 0;
                    array_73D150[2] = pTerrainVertices[(z + 1) * 128 + x + 1];  // x + 1, z + 1
                    array_73D150[2].u = 1;
                    array_73D150[2].v = 1;
                    array_73D150[1] = pTerrainVertices[(z + 1) * 128 + x];  // x, z + 1
                    array_73D150[1].u = 0;
                    array_73D150[1].v = 1;

                    for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                        VertexRenderList[k] = array_73D150[k];
                        VertexRenderList[k]._rhw = 1.0f / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097f);
                    }

                    float _f2 = norm2->x * pOutdoor->vSunlight.x + norm2->y * pOutdoor->vSunlight.y + norm2->z * pOutdoor->vSunlight.z;
                    pTilePolygon->dimming_level = 20.0f - floorf(20.0f * _f2 + 0.5f);


                    lightmap_builder->StackLights_TerrainFace(norm2, &Light_tile_dist, VertexRenderList, 3, 0);

                    decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm2,
                        &Light_tile_dist, VertexRenderList, 3, 0);


                    unsigned int a5_2 = 4;

                    // ---------Draw distance(Дальность отрисовки)-------------------------------
                    float far_clip_distance_2 = pCamera3D->GetFarClip();
                    float near_clip_distance_2 = pCamera3D->GetNearClip();


                    bool neer_clip_2 = array_73D150[0].vWorldViewPosition.x < near_clip_distance_2 ||
                        array_73D150[1].vWorldViewPosition.x < near_clip_distance_2 ||
                        array_73D150[2].vWorldViewPosition.x < near_clip_distance_2;
                    bool far_clip_2 =
                        (float)far_clip_distance_2 < array_73D150[0].vWorldViewPosition.x ||
                        (float)far_clip_distance_2 < array_73D150[1].vWorldViewPosition.x ||
                        (float)far_clip_distance_2 < array_73D150[2].vWorldViewPosition.x;

                    int uClipFlag_2 = 0;
                    static stru154 static_sub_0048034E_stru_154_2;
                    lightmap_builder->StationaryLightsCount = 0;
                    if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                        if (neer_clip_2)
                            uClipFlag_2 = 3;
                        else
                            uClipFlag_2 = far_clip_2 != 0 ? 5 : 0;
                        static_sub_0048034E_stru_154_2.ClassifyPolygon(norm2, Light_tile_dist);

                        if (decal_builder->uNumSplatsThisFace > 0)
                            decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level,
                                LocationTerrain, &static_sub_0048034E_stru_154_2,
                                3, VertexRenderList,
                                *(float *)&uClipFlag_2, -1);
                        if (Lights.uNumLightsApplied > 0)
                            lightmap_builder->ApplyLights(
                                &Lights, &static_sub_0048034E_stru_154_2, 3,
                                VertexRenderList, 0, uClipFlag_2);
                    }







                    // pODMRenderParams->shading_dist_mist = temp;

                    // check the transparency and texture (tiles) mapping (проверка
                    // прозрачности и наложение текстур (тайлов))----------------------
                    bool transparent = false;

                    auto tile_texture = tile->GetTexture();
                    if (!(pTilePolygon->flags & 1)) {
                        // не поддерживается TextureFrameTable
                        if (/*pTile->flags & 2 && */ tile->IsWaterTile()) {
                            tile_texture =
                                this->hd_water_tile_anim[this->hd_water_current_frame];
                        } else if (tile->IsWaterBorderTile()) {
                            // for all shore tiles - draw a tile water under them since
                            // they're half-empty
                            DrawBorderTiles(pTilePolygon);
                            transparent = true;
                        }
                        pTilePolygon->texture = tile_texture;

                        render->DrawTerrainPolygon(pTilePolygon, transparent, true);
                    }
                }  // end split trinagles
            } else {
                float _f = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0f - floorf(20.0f * _f + 0.5f);

                lightmap_builder->StackLights_TerrainFace(norm, &Light_tile_dist, VertexRenderList, pTilePolygon->uNumVertices, 1);
                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, pTilePolygon->uNumVertices, 1);

                unsigned int a5 = 4;

                // ---------Draw distance(Дальность отрисовки)-------------------------------
                float far_clip_distance = pCamera3D->GetFarClip();
                float near_clip_distance = pCamera3D->GetNearClip();

                bool neer_clip = array_73D150[0].vWorldViewPosition.x < near_clip_distance ||
                    array_73D150[1].vWorldViewPosition.x < near_clip_distance ||
                    array_73D150[2].vWorldViewPosition.x < near_clip_distance ||
                    array_73D150[3].vWorldViewPosition.x < near_clip_distance;
                bool far_clip =
                    (float)far_clip_distance < array_73D150[0].vWorldViewPosition.x ||
                    (float)far_clip_distance < array_73D150[1].vWorldViewPosition.x ||
                    (float)far_clip_distance < array_73D150[2].vWorldViewPosition.x ||
                    (float)far_clip_distance < array_73D150[3].vWorldViewPosition.x;

                int uClipFlag = 0;
                static stru154 static_sub_0048034E_stru_154;
                lightmap_builder->StationaryLightsCount = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                    if (neer_clip)
                        uClipFlag = 3;
                    else
                        uClipFlag = far_clip != 0 ? 5 : 0;
                    static_sub_0048034E_stru_154.ClassifyPolygon(norm, Light_tile_dist);

                    if (decal_builder->uNumSplatsThisFace > 0)
                        decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level,
                            LocationTerrain, &static_sub_0048034E_stru_154,
                            a5, VertexRenderList,
                            *(float *)&uClipFlag, -1);

                    if (Lights.uNumLightsApplied > 0)
                        lightmap_builder->ApplyLights(&Lights, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, uClipFlag);
                }


                // pODMRenderParams->shading_dist_mist = temp;

                // check the transparency and texture (tiles) mapping (проверка
                // прозрачности и наложение текстур (тайлов))----------------------
                bool transparent = false;

                auto tile_texture = tile->GetTexture();
                if (!(pTilePolygon->flags & 1)) {
                    // не поддерживается TextureFrameTable
                    if (/*pTile->flags & 2 && */ tile->IsWaterTile()) {
                        tile_texture =
                            this->hd_water_tile_anim[this->hd_water_current_frame];
                    } else if (tile->IsWaterBorderTile()) {
                        // for all shore tiles - draw a tile water under them since
                        // they're half-empty
                        DrawBorderTiles(pTilePolygon);
                        transparent = true;
                    }
                    pTilePolygon->texture = tile_texture;

                    render->DrawTerrainPolygon(pTilePolygon, transparent, true);
                }
            // end norm split
            }
        }
    }

    // draw any triangles that have been batched
    pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP); // terrain tiles need to be clamped
    BatchTriDraw();
}

void BatchTriSort() {
    // set pointers
    for (int i = 0; i < NumBatchTrianglesStore; ++i) {
        StorePtr[i] = &BatchTrianglesStore[i];
    }
    // sort tris
    std::sort(StorePtr, StorePtr + NumBatchTrianglesStore, SortByTransThenTex);
}

void Render::BatchTriDraw() {
    if (!NumBatchTrianglesStore) return;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL));

    // sort batched traingles
    BatchTriSort();

    // reset params
    std::string *currtex = nullptr;
    bool currtrans = 0;
    int trisinbatch = 0;
    int batches = 0;

    // loop through store
    for (int z = 0; z < NumBatchTrianglesStore; z++) {
        BatchTriangles* thistri = StorePtr[z];
        // add tringles that match current settings to render batch
        if ((thistri->texname == currtex) && (thistri->trans == currtrans)) {
            // add triangles to render list
            for (int t = 0; t < 3; t++) {
                BatchDrawTriangles[(trisinbatch * 3 + t)] = thistri->Verts[t];
            }
            trisinbatch++;
        } else {
            // draw current batch
            if (trisinbatch) {
                this->pRenderD3D->pDevice->DrawPrimitive(
                    D3DPT_TRIANGLELIST,
                    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                    BatchDrawTriangles, (trisinbatch*3), D3DDP_DONOTLIGHT);

                drawcalls++;
            }

            // start new batch with altered settings
            batches++;
            trisinbatch = 0;

            // set new params and load
            currtex = thistri->texname;
            currtrans = thistri->trans;

            if (currtrans) {
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
            } else {
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
                this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
            }

            auto texture = (TextureD3D*)thistri->texture;
            this->pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture());

            for (int t = 0; t < 3; t++) {
                BatchDrawTriangles[(trisinbatch * 3 + t)] = thistri->Verts[t];
            }
            trisinbatch++;
        }
    }

    // draw any left in pipeline
    if (trisinbatch) {
        this->pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLELIST,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            BatchDrawTriangles, (trisinbatch*3), D3DDP_DONOTLIGHT);

        drawcalls++;
    }

    // reset
    batches++;
    trisinbatch = 0;
    //logger->Info("%u Triangles sorted to %u batches", NumBatchTrianglesStore, batches);
    NumBatchTrianglesStore = 0;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));
}

bool SortByTransThenTex(const BatchTriangles* lhs, const BatchTriangles* rhs) {
    if (lhs->trans != rhs->trans) {
        return lhs->trans < rhs->trans;
    } else {
        return lhs->texname < rhs->texname;
    }
}

void Render::DrawBorderTiles(struct Polygon *poly) {
    struct Polygon poly_clone = *poly;
    poly_clone.texture = this->hd_water_tile_anim[this->hd_water_current_frame];

    //pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    DrawTerrainPolygon(&poly_clone, false, true); // t t

    //pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, true);
}


void Render::PrepareDecorationsRenderList_ODM() {
    unsigned int v6;        // edi@9
    int v7;                 // eax@9
    SpriteFrame *frame;     // eax@9
    uint16_t *v10;  // eax@9
    int v13;                // ecx@9
    int r;                 // ecx@20
    int g;                 // dl@20
    int b_;                // eax@20
    Particle_sw local_0;    // [sp+Ch] [bp-98h]@7
    uint16_t *v37;  // [sp+84h] [bp-20h]@9
    int v38;                // [sp+88h] [bp-1Ch]@9

    for (unsigned int i = 0; i < pLevelDecorations.size(); ++i) {
        // LevelDecoration* decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST) ||
             pLevelDecorations[i].IsObeliskChestActive()) &&
            !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            const DecorationDesc *decor_desc = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);
            if (!(decor_desc->uFlags & DECORATION_DESC_EMITS_FIRE)) {
                if (!(decor_desc->uFlags & (DECORATION_DESC_MARKER | DECORATION_DESC_DONT_DRAW))) {
                    v6 = pMiscTimer->uTotalGameTimeElapsed;
                    v7 = abs(pLevelDecorations[i].vPosition.x +
                             pLevelDecorations[i].vPosition.y);

                    frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                                                        v6 + v7);

                    if (engine->config->graphics.SeasonsChange.Get()) {
                        frame = LevelDecorationChangeSeason(decor_desc, v6 + v7, pParty->uCurrentMonth);
                    }

                    if (!frame || frame->texture_name == "null" || frame->hw_sprites[0] == NULL) {
                        continue;
                    }

                    // v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                    // v6 + v7);

                    v10 = (uint16_t *)TrigLUT.Atan2(
                        pLevelDecorations[i].vPosition.x -
                            pCamera3D->vCameraPos.x,
                        pLevelDecorations[i].vPosition.y -
                            pCamera3D->vCameraPos.y);
                    v38 = 0;
                    v13 = ((signed int)(TrigLUT.uIntegerPi +
                                        ((signed int)TrigLUT.uIntegerPi >>
                                         3) +
                                        pLevelDecorations[i].field_10_y_rot -
                                        (signed int)v10) >>
                           8) &
                          7;
                    v37 = (uint16_t *)v13;
                    if (frame->uFlags & 2) v38 = 2;
                    if ((256 << v13) & frame->uFlags) v38 |= 4;
                    if (frame->uFlags & 0x40000) v38 |= 0x40;
                    if (frame->uFlags & 0x20000) v38 |= 0x80;

                    // for light
                    if (frame->uGlowRadius) {
                        r = 255;
                        g = 255;
                        b_ = 255;
                        if (render->config->graphics.ColoredLights.Get()) {
                            r = decor_desc->uColoredLightRed;
                            g = decor_desc->uColoredLightGreen;
                            b_ = decor_desc->uColoredLightBlue;
                        }
                        pStationaryLightsStack->AddLight(
                            pLevelDecorations[i].vPosition.ToFloat() +
                                Vec3f(0, 0, decor_desc->uDecorationHeight / 2),
                            frame->uGlowRadius, r, g, b_, _4E94D0_light_type);
                    }  // for light

                    // v17 = (pLevelDecorations[i].vPosition.x -
                    // pCamera3D->vCameraPos.x) << 16; v40 =
                    // (pLevelDecorations[i].vPosition.y -
                    // pCamera3D->vCameraPos.y) << 16;
                    int party_to_decor_x = pLevelDecorations[i].vPosition.x -
                                           pCamera3D->vCameraPos.x;
                    int party_to_decor_y = pLevelDecorations[i].vPosition.y -
                                           pCamera3D->vCameraPos.y;
                    int party_to_decor_z = pLevelDecorations[i].vPosition.z -
                                           pCamera3D->vCameraPos.z;

                    int view_x = 0;
                    int view_y = 0;
                    int view_z = 0;
                    bool visible = pCamera3D->ViewClip(
                        pLevelDecorations[i].vPosition.x,
                        pLevelDecorations[i].vPosition.y,
                        pLevelDecorations[i].vPosition.z, &view_x, &view_y,
                        &view_z);

                    if (visible) {
                        // if (2 * abs(view_x) >= abs(view_y)) {
                            int projected_x = 0;
                            int projected_y = 0;
                            pCamera3D->Project(view_x, view_y, view_z,
                                                      &projected_x,
                                                      &projected_y);

                            float _v41 = frame->scale *
                                pCamera3D->ViewPlaneDist_X /
                                view_x;

                            int screen_space_half_width = 0;
                            screen_space_half_width =
                                _v41 *
                                frame->hw_sprites[(int)v37]->uBufferWidth / 2;


                            if (projected_x + screen_space_half_width >=
                                    (signed int)pViewport->uViewportTL_X &&
                                projected_x - screen_space_half_width <=
                                    (signed int)pViewport->uViewportBR_X) {
                                if (::uNumBillboardsToDraw >= 500) return;
                                ::uNumBillboardsToDraw++;
                                ++uNumDecorationsDrawnThisFrame;

                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .hwsprite = frame->hw_sprites[(int)v37];

                                // error catching
                                if (frame->hw_sprites[(int)v37]->texture->GetHeight() == 0 || frame->hw_sprites[(int)v37]->texture->GetWidth() == 0)
                                    __debugbreak();

                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_x = pLevelDecorations[i].vPosition.x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_y = pLevelDecorations[i].vPosition.y;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .world_z = pLevelDecorations[i].vPosition.z;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_x = projected_x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_y = projected_y;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screen_space_z = view_x;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screenspace_projection_factor_x = _v41;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .screenspace_projection_factor_y = _v41;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .uPaletteIndex = frame->GetPaletteIndex();
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .field_1E = v38 | 0x200;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .uIndoorSectorID = 0;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .object_pid = PID(OBJECT_Decoration, i);
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .dimming_level = 0;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .pSpriteFrame = frame;
                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .sTintColor = 0;
                            } else {
                                // temp
                            }
                       // }
                    }
                }
            } else {
                memset(&local_0, 0, sizeof(Particle_sw));
                local_0.type = ParticleType_Bitmap | ParticleType_Rotating |
                               ParticleType_8;
                local_0.uDiffuse = 0xFF3C1E;
                local_0.x = (double)pLevelDecorations[i].vPosition.x;
                local_0.y = (double)pLevelDecorations[i].vPosition.y;
                local_0.z = (double)pLevelDecorations[i].vPosition.z;
                local_0.r = 0.0;
                local_0.g = 0.0;
                local_0.b = 0.0;
                local_0.particle_size = 1.0;
                local_0.timeToLive = Random(0x80) + 128; // was rand() & 0x80
                local_0.texture = spell_fx_renderer->effpar01;
                particle_engine->AddParticle(&local_0);
            }
        }
    }
}

void Render::DrawPolygon(struct Polygon *pPolygon) {
    if ((uNumD3DSceneBegins == 0) || (pPolygon->uNumVertices < 3)) {
        return;
    }

    unsigned int v41;     // eax@29
    unsigned int sCorrectedColor;  // [sp+64h] [bp-4h]@4

    auto texture = (TextureD3D *)pPolygon->texture;
    ODMFace *pFace = pPolygon->pODMFace;
    auto uNumVertices = pPolygon->uNumVertices;

    if (lightmap_builder->StationaryLightsCount) {
        sCorrectedColor = -1;
    }
    engine->AlterGamma_ODM(pFace, &sCorrectedColor);
    if (false) {
        int v8 = ::GetActorTintColor(
            pPolygon->dimming_level, 0,
            VertexRenderList[0].vWorldViewPosition.x, 0, 0);
        lightmap_builder->DrawLightmaps(v8 /*, 0*/);
    } else {
        if (!lightmap_builder->StationaryLightsCount) {
            // send for batching

            for (int z = 0; z < (uNumVertices - 2); z++) {
                // 123, 134, 145, 156..
                BatchTriangles* thistri = &BatchTrianglesStore[NumBatchTrianglesStore];

                thistri->texture = pPolygon->texture;
                thistri->texname = pPolygon->texture->GetName();
                thistri->trans = 0;

                // copy first
                //for (uint i = 0; i < uNumVertices; ++i) {
                thistri->Verts[0].pos.x = VertexRenderList[0].vWorldViewProjX;
                thistri->Verts[0].pos.y = VertexRenderList[0].vWorldViewProjY;
                thistri->Verts[0].pos.z =
                    1.0 - 1.0 / ((VertexRenderList[0].vWorldViewPosition.x * 1000) /
                        pCamera3D->GetFarClip());
                thistri->Verts[0].rhw =
                    1.0 / (VertexRenderList[0].vWorldViewPosition.x + 0.0000001);
                thistri->Verts[0].diffuse = ::GetActorTintColor(
                    pPolygon->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x,
                    0, 0);

                engine->AlterGamma_ODM(pFace, &thistri->Verts[0].diffuse);

                thistri->Verts[0].specular = 0;
                if (engine->IsSpecular_FogIsOn())
                    thistri->Verts[0].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[0].vWorldViewPosition.x);

                thistri->Verts[0].texcoord.x = VertexRenderList[0].u;
                thistri->Verts[0].texcoord.y = VertexRenderList[0].v;
                //}




                // copy other two (z+1)(z+2)
                for (uint i = 1; i < 3; ++i) {
                    thistri->Verts[i].pos.x = VertexRenderList[z + i].vWorldViewProjX;
                    thistri->Verts[i].pos.y = VertexRenderList[z + i].vWorldViewProjY;
                    thistri->Verts[i].pos.z =
                        1.0 - 1.0 / ((VertexRenderList[z + i].vWorldViewPosition.x * 1000) /
                            pCamera3D->GetFarClip());
                    thistri->Verts[i].rhw =
                        1.0 / (VertexRenderList[z + i].vWorldViewPosition.x + 0.0000001);
                    thistri->Verts[i].diffuse = ::GetActorTintColor(
                        pPolygon->dimming_level, 0, VertexRenderList[z + i].vWorldViewPosition.x,
                        0, 0);

                    engine->AlterGamma_ODM(pFace, &thistri->Verts[i].diffuse);

                    thistri->Verts[i].specular = 0;
                    if (engine->IsSpecular_FogIsOn())
                        thistri->Verts[i].specular = sub_47C3D7_get_fog_specular(
                            0, 0, VertexRenderList[z + i].vWorldViewPosition.x);

                    thistri->Verts[i].texcoord.x = VertexRenderList[z + i].u;
                    thistri->Verts[i].texcoord.y = VertexRenderList[z + i].v;
                }

                if (pFace->uAttributes & FACE_OUTLINED) {
                    int color;
                    if (platform->TickCount() % 300 >= 150)
                        color = 0xFFFF2020;
                    else
                        color = 0xFF901010;

                    for (uint i = 0; i < 3; ++i)
                        thistri->Verts[i].diffuse = color;
                }

                ++NumBatchTrianglesStore;
                if (NumBatchTrianglesStore > 9500) BatchTriDraw();
            }



        } else {
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].pos.x =
                    VertexRenderList[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y =
                    VertexRenderList[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z =
                    1.0 -
                    1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) /
                           pCamera3D->GetFarClip());
                d3d_vertex_buffer[i].rhw =
                    1.0 /
                    (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
                d3d_vertex_buffer[i].diffuse = GetActorTintColor(
                    pPolygon->dimming_level, 0,
                    VertexRenderList[i].vWorldViewPosition.x, 0, 0);
                if (engine->IsSpecular_FogIsOn())
                    d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[i].vWorldViewPosition.x);
                else
                    d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
                d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
                                                             D3DTADDRESS_WRAP));
            if (engine->IsSpecular_FogIsOn())
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

            ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
            drawcalls++;
            // v50 = (const char *)v5->pRenderD3D->pDevice;
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
                                                       D3DCULL_NONE));
            // (*(void (**)(void))(*(int *)v50 + 88))();
            lightmap_builder->DrawLightmaps(-1);
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
            }
            ErrD3D(pRenderD3D->pDevice->SetTexture(
                0, texture->GetDirect3DTexture()));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
                                                             D3DTADDRESS_WRAP));
            if (!engine->IsSpecular_FogIsOn())
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

            ErrD3D(pRenderD3D->pDevice->SetRenderState(
                D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                                       D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                                       D3DBLEND_SRCCOLOR));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
            drawcalls++;

            if (engine->IsSpecular_FogIsOn()) {
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));

                for (uint i = 0; i < uNumVertices; ++i) {
                    d3d_vertex_buffer[i].diffuse =
                        render->uFogColor |
                        d3d_vertex_buffer[i].specular & 0xFF000000;
                    d3d_vertex_buffer[i].specular = 0;
                }

                ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(
                    D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(
                    D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA));
                ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                    D3DPT_TRIANGLEFAN,
                    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR |
                        D3DFVF_TEX1,
                    d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
                drawcalls++;
                ErrD3D(pRenderD3D->pDevice->SetRenderState(
                    D3DRENDERSTATE_FOGENABLE, TRUE));
                // v40 = render->pRenderD3D->pDevice->lpVtbl;
                v41 = GetLevelFogColor();
                pRenderD3D->pDevice->SetRenderState(
                    D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF);
                pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE,
                                                    0);
            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                                       D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                                       D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(
                D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
        }
    }

    if (engine->config->debug.Terrain.Get()) {
        pCamera3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x0000FFFF, 0.0);
    }
    if (engine->config->debug.ShowPickedFace.Get()) {
        if (pFace->uAttributes & FACE_OUTLINED) {
            RenderVertexSoft cam;
            cam.vWorldPosition.x = pCamera3D->vCameraPos.x;
            cam.vWorldPosition.y = pCamera3D->vCameraPos.y;
            cam.vWorldPosition.z = pCamera3D->vCameraPos.z;
            pCamera3D->do_draw_debug_line_sw(&cam, -1, &vis->debugpick, 0xFFFF00u, 0, 0);
        }
    }
}

Render::Render(
    std::shared_ptr<Application::GameConfig> config,
    DecalBuilder* decal_builder,
    LightmapBuilder* lightmap_builder,
    SpellFxRenderer* spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis* vis,
    Logger* logger
) : RenderBase(config, decal_builder, lightmap_builder, spellfx, particle_engine, vis, logger) {
    this->pDirectDraw4 = nullptr;
    this->pFrontBuffer4 = nullptr;
    this->pBackBuffer4 = nullptr;
    this->bWindowMode = 1;
    this->pActiveZBuffer = nullptr;
    this->pDefaultZBuffer = nullptr;
    this->pRenderD3D = 0;
    this->uNumD3DSceneBegins = 0;
    this->bRequiredTextureStagesAvailable = 0;

    this->uDesiredDirect3DDevice = 0;
    this->uMaxDeviceTextureDim = 0;
    this->uMinDeviceTextureDim = 0;

    this->uTargetBBits = 0;
    this->uTargetGBits = 0;
    this->uTargetRBits = 0;

    uNumBillboardsToDraw = 0;

    hd_water_current_frame = 0;

    this->p2DGraphics = nullptr;
    this->p2DSurface = nullptr;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

Render::~Render() {
    free(pDefaultZBuffer);
    Release();
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

bool Render::Initialize() {
    if (!RenderBase::Initialize()) {
        return false;
    }

    uDesiredDirect3DDevice = render->config->graphics.D3DDevice.Get();

    PostInitialization();

    return true;
}

void Render::ClearBlack() { pRenderD3D->ClearTarget(true, 0, false, 0.0); }

void Render::PresentBlackScreen() {
    RECT dest_rect = {0};
    GetWindowRect((HWND)window->SystemHandle(), &dest_rect);

    DDBLTFX lpDDBltFx = {0};
    lpDDBltFx.dwSize = sizeof(DDBLTFX);
    lpDDBltFx.dwFillColor = 0;
    pBackBuffer4->Blt(&dest_rect, NULL, NULL, DDBLT_COLORFILL, &lpDDBltFx);
    render->Present();
}

void Render::SavePCXScreenshot() {
    size_t zeros_number = 5;
    std::string screenshot_number = std::to_string(engine->config->settings.ScreenshotNumber.Increment());
    std::string file_name = "screenshot_" + std::string(zeros_number - std::min(zeros_number, screenshot_number.length()), '0') + screenshot_number + ".pcx";

    SaveWinnersCertificate(file_name.c_str());
}

uint8_t *ReadScreenPixels() {}

void Render::SaveWinnersCertificate(const char *file_name) {
    BeginScene();

    // grab and copy back buffer
    DDSURFACEDESC2 Dst;
    memset(&Dst, 0, sizeof(Dst));
    Dst.dwSize = sizeof(Dst);

    if (LockSurface_DDraw4(pRenderD3D->pBackBuffer, &Dst, DDLOCK_WAIT)) {
        SavePCXImage32(file_name, (uint16_t *)Dst.lpSurface, render->GetRenderDimensions().w, render->GetRenderDimensions().h);
        ErrD3D(pRenderD3D->pBackBuffer->Unlock(NULL));
    }

    EndScene();
}

void Render::SavePCXImage32(const std::string &filename, uint16_t *picture_data,
                            int width, int height) {
    // TODO(pskelton): add "Screenshots" folder?
    std::string thispath = MakeDataPath(filename);
    FILE *result = fopen(thispath.c_str(), "wb");
    if (result == nullptr) {
        return;
    }

    unsigned int pcx_data_size = width * height * 5;
    uint8_t *pcx_data = new uint8_t[pcx_data_size];
    unsigned int pcx_data_real_size = 0;
    PCX::Encode32(picture_data, width, height, pcx_data, pcx_data_size,
                  &pcx_data_real_size);
    fwrite(pcx_data, pcx_data_real_size, 1, result);
    delete[] pcx_data;
    fclose(result);
}

void Render::SavePCXImage16(const std::string &filename, uint16_t *picture_data,
                            int width, int height) {
    std::string thispath = MakeDataPath(filename);
    FILE *result = fopen(thispath.c_str(), "wb");
    if (result == nullptr) {
        return;
    }

    unsigned int pcx_data_size = width * height * 5;
    uint8_t *pcx_data = new uint8_t[pcx_data_size];
    unsigned int pcx_data_real_size = 0;
    PCX::Encode16(picture_data, width, height, pcx_data, pcx_data_size,
                  &pcx_data_real_size);
    fwrite(pcx_data, pcx_data_real_size, 1, result);
    delete[] pcx_data;
    fclose(result);
}

void Render::ClearTarget(unsigned int uColor) {
    pRenderD3D->ClearTarget(true, uColor, false, 0.0);
}

void Render::Present() {
    pBeforePresentFunction();
    if (pRenderD3D) {
        pRenderD3D->Present(false);
    } else {
        assert(false);
    }
}

void Render::CreateZBuffer() {
    if (!pDefaultZBuffer) {
        pDefaultZBuffer = pActiveZBuffer = (int *)malloc(window->GetWidth() * window->GetHeight() * sizeof(int));
        ClearZBuffer();
    }
}

void Render::ClearZBuffer() {
    memset32(this->pActiveZBuffer, 0xFFFF0000, window->GetWidth() * window->GetHeight());
}

void Render::Release() {
    if (pRenderD3D) {
        pRenderD3D->ClearTarget(true, 0, false, 1.0);
        pRenderD3D->Present(0);
        pRenderD3D->ClearTarget(true, 0, false, 1.0);
        this->pBackBuffer4 = nullptr;
        this->pFrontBuffer4 = nullptr;
        this->pDirectDraw4 = nullptr;
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
    }
}

// copies in 2d elements - dst is 3d level
void Present32(uint32_t* src, unsigned int src_pitch, uint32_t* dst, unsigned int dst_pitch) {
    // y 0 - viewport start
    for (uint y = 0; y < pViewport->uViewportTL_Y; ++y) {
        memcpy(dst + y * dst_pitch, src + y * src_pitch, src_pitch * sizeof(uint32_t));
    }

    // y viewport start - vewport end
    for (uint y = pViewport->uViewportTL_Y; y < pViewport->uViewportBR_Y; ++y) {
        // x 0 - 8
        memcpy(dst + y * dst_pitch, src + y * src_pitch, 8 * sizeof(uint32_t));
        // x edge viewport - screen wodth
        memcpy(dst + pViewport->uViewportBR_X + y * dst_pitch,
            src + pViewport->uViewportBR_X + y * src_pitch,
            (window->GetWidth() - pViewport->uViewportBR_X) * sizeof(uint32_t));
    }

    // y viewport end to screen height
    for (uint y = pViewport->uViewportBR_Y; y < window->GetHeight(); ++y) {
        memcpy(dst + y * dst_pitch, src + y * src_pitch, src_pitch * sizeof(uint32_t));
    }

    // mask for 2d elements over viewport
    for (uint y = pViewport->uViewportTL_Y; y < pViewport->uViewportBR_Y; ++y) {
        for (uint x = pViewport->uViewportTL_X; x < pViewport->uViewportBR_X; ++x) {
            if (src[x + y * src_pitch] != render->teal_mask_32) {
                dst[x + y * dst_pitch] = src[x + y * src_pitch];
            }
        }
    }
}

void Present_NoColorKey() {
    Render *r = (Render *)render.get();

    DDSURFACEDESC2 Dst = {0};
    Dst.dwSize = sizeof(Dst);
    if (r->LockSurface_DDraw4(r->pBackBuffer4, &Dst, DDLOCK_WAIT)) {
        Gdiplus::Rect rect(0, 0, Dst.dwWidth, Dst.dwHeight);
        Gdiplus::BitmapData bitmapData;
        r->p2DSurface->LockBits(&rect, Gdiplus::ImageLockModeRead,
            PixelFormat32bppARGB, &bitmapData);
        Present32((uint32_t *)bitmapData.Scan0, bitmapData.Width,
            (uint32_t *)Dst.lpSurface, Dst.lPitch / 4);
        r->p2DSurface->UnlockBits(&bitmapData);
        ErrD3D(r->pBackBuffer4->Unlock(NULL));
    }
}

bool Render::InitializeFullscreen() {
    this->pBackBuffer4 = nullptr;
    this->pFrontBuffer4 = nullptr;
    this->pDirectDraw4 = nullptr;
    Release();
    CreateZBuffer();

    pRenderD3D = new RenderD3D;

    int v29 = -1;
    RenderD3D__DevInfo *v7 = pRenderD3D->pAvailableDevices;
    bool v8 = false;
    if (pRenderD3D->pAvailableDevices[uDesiredDirect3DDevice]
            .bIsDeviceCompatible) {
        v8 = pRenderD3D->CreateDevice(uDesiredDirect3DDevice, true);
    } else {
        if (v7[1].bIsDeviceCompatible) {
            v8 = pRenderD3D->CreateDevice(1, true);
        } else {
            if (!v7->bIsDeviceCompatible)
                Error("There aren't any D3D devices to create.");

            v8 = pRenderD3D->CreateDevice(0, true);
        }
    }
    if (!v8) {
        Error("D3Drend->Init failed.");
    }

    pBackBuffer4 = pRenderD3D->pBackBuffer;
    pFrontBuffer4 = pRenderD3D->pFrontBuffer;
    pDirectDraw4 = pRenderD3D->pHost;

    unsigned int v10 = pRenderD3D->GetDeviceCaps();
    if (v10 & 1) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error("Direct3D renderer:  The device failed to return capabilities.");
    }
    if (v10 & 0x3E) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error(
            "Direct3D renderer:  The device doesn't support the necessary "
            "alpha blending modes.");
    }
    if ((v10 & 0x80) != 0) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error(
            "Direct3D renderer:  The device doesn't support non-square "
            "textures.");
    }

    bRequiredTextureStagesAvailable = CheckTextureStages();

    D3DDEVICEDESC halCaps = {0};
    halCaps.dwSize = sizeof(halCaps);

    D3DDEVICEDESC refCaps = {0};
    refCaps.dwSize = sizeof(refCaps);

    ErrD3D(pRenderD3D->pDevice->GetCaps(&halCaps, &refCaps));

    uMinDeviceTextureDim = halCaps.dwMinTextureWidth;
    if ((unsigned int)halCaps.dwMinTextureWidth >= halCaps.dwMinTextureHeight)
        uMinDeviceTextureDim = halCaps.dwMinTextureHeight;
    uMinDeviceTextureDim = halCaps.dwMaxTextureWidth;
    if ((unsigned int)halCaps.dwMaxTextureWidth < halCaps.dwMaxTextureHeight)
        uMinDeviceTextureDim = halCaps.dwMaxTextureHeight;
    if ((unsigned int)uMinDeviceTextureDim < 4) uMinDeviceTextureDim = 4;

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, true));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, true));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));  // D3DCMP_LESS
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE,
                                               false));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE,
                                               false));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,
                                               false));
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

    ddpfPrimarySuface.dwSize = sizeof(DDPIXELFORMAT);
    GetTargetPixelFormat(&ddpfPrimarySuface);
    ParseTargetPixelFormat();

    if (!pRenderD3D) {
        __debugbreak();
        pBeforePresentFunction = 0;
    }

    p2DSurface = new Gdiplus::Bitmap(window->GetWidth(), window->GetHeight(),
                                     PixelFormat32bppRGB);
    p2DGraphics = new Gdiplus::Graphics(p2DSurface);
    pBeforePresentFunction = Present_NoColorKey;

    bWindowMode = 0;
    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    pViewport->ResetScreen();

    return true;
}

bool Render::DrawLightmap(Lightmap *pLightmap, Vec3f *pColorMult,
                          float z_bias) {
    // For outdoor terrain and indoor light (VII)(VII)
    if (pLightmap->NumVertices < 3) {
        log->Warning("Lightmap uNumVertices < 3");
        return false;
    }

    unsigned int uLightmapColorMaskR = (pLightmap->uColorMask >> 16) & 0xFF;
    unsigned int uLightmapColorMaskG = (pLightmap->uColorMask >> 8) & 0xFF;
    unsigned int uLightmapColorMaskB = pLightmap->uColorMask & 0xFF;

    unsigned int uLightmapColorR = floorf(
        uLightmapColorMaskR * pLightmap->fBrightness * pColorMult->x + 0.5f);
    unsigned int uLightmapColorG = floorf(
        uLightmapColorMaskG * pLightmap->fBrightness * pColorMult->y + 0.5f);
    unsigned int uLightmapColorB = floorf(
        uLightmapColorMaskB * pLightmap->fBrightness * pColorMult->z + 0.5f);

    RenderVertexD3D3 pVerticesD3D[64];
    for (uint i = 0; i < pLightmap->NumVertices; ++i) {
        float v18;
        if (fabs(z_bias) < 1e-5) {
            v18 = 1.0 - 1.0 / ((1.0f / 16192.0) * pLightmap->pVertices[i].vWorldViewPosition.x * 1000.0);
        } else {
            v18 = 1.0 - 1.0 / ((1.0f / 16192.0) * pLightmap->pVertices[i].vWorldViewPosition.x * 1000.0) - z_bias;

            if (v18 < 0.000099999997f) {
                v18 = 0.000099999997f;
            }
        }

        pVerticesD3D[i].pos.x = pLightmap->pVertices[i].vWorldViewProjX;
        pVerticesD3D[i].pos.y = pLightmap->pVertices[i].vWorldViewProjY;
        pVerticesD3D[i].pos.z = v18;

        pVerticesD3D[i].rhw = 1.0 / pLightmap->pVertices[i].vWorldViewPosition.x;
        pVerticesD3D[i].diffuse = (uLightmapColorR << 16) | (uLightmapColorG << 8) | uLightmapColorB;
        pVerticesD3D[i].specular = 0;

        pVerticesD3D[i].texcoord.x = pLightmap->pVertices[i].u;
        pVerticesD3D[i].texcoord.y = pLightmap->pVertices[i].v;
    }

    int dwFlags = D3DDP_DONOTLIGHT;

    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        pVerticesD3D, pLightmap->NumVertices, dwFlags));
    drawcalls++;

    return true;
}

// blue mask
void Render::DrawFromSpriteSheet(Recti *pSrcRect, Pointi *pTargetPoint, int a3, int blend_mode) {
    uint16_t *pSrc;          // eax@2
    int uSrcTotalWidth = 0;      // ecx@4
    unsigned int v10;        // esi@9
    int v21;                 // [sp+Ch] [bp-18h]@8
    uint16_t *src_surf_pos;  // [sp+10h] [bp-14h]@9
    int32_t src_width;       // [sp+14h] [bp-10h]@3
    int32_t src_height;      // [sp+18h] [bp-Ch]@3
    int uSrcPitch;           // [sp+1Ch] [bp-8h]@5

    if (!pArcomageGame->pBlit_Copy_pixels) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Warning("Missing Arcomage Sprite Sheet");
        return;
    }

    src_width = pSrcRect->w;
    src_height = pSrcRect->h;

    /*if (pArcomageGame->pBlit_Copy_pixels == pArcomageGame->pBackgroundPixels)
        uSrcTotalWidth = pArcomageGame->pGameBackground->GetWidth();
    else*/ if (pArcomageGame->pBlit_Copy_pixels == pArcomageGame->pSpritesPixels)
        uSrcTotalWidth = pArcomageGame->pSprites->GetWidth();

    pSrc = pArcomageGame->pBlit_Copy_pixels;
    uSrcPitch = uSrcTotalWidth;
    src_surf_pos = &pSrc[pSrcRect->x + uSrcPitch * pSrcRect->y];
    v10 = 0x1F;
    v21 = (uTargetGBits != 6 ? 0x31EF : 0x7BEF);

    Image *temp = Image::Create(src_width, src_height, IMAGE_FORMAT_A8R8G8B8);
    uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (blend_mode == 2) {
        uSrcPitch = (uSrcPitch - src_width);
        for (int i = 0; i < src_height; ++i) {
            for (int j = 0; j < src_width; ++j) {
                if (*src_surf_pos != v10) {
                    if (pTargetPoint->x + j >= 0 &&
                        pTargetPoint->x + j <= window->GetWidth() - 1 &&
                        pTargetPoint->y + i >= 0 &&
                        pTargetPoint->y + i <= window->GetHeight() - 1)
                        temppix[j + i * src_width] = Color32(*src_surf_pos);
                }
                ++src_surf_pos;
            }
            src_surf_pos += uSrcPitch;
        }
    } else {
        uSrcPitch = (uSrcPitch - src_width);
        for (int i = 0; i < src_height; ++i) {
            for (int j = 0; j < src_width; ++j) {
                if (*src_surf_pos != v10) {
                    if (pTargetPoint->x + j >= 0 &&//
                        pTargetPoint->x + j <= window->GetWidth() - 1 &&//
                        pTargetPoint->y + i >= 0 &&//
                        pTargetPoint->y + i <= window->GetHeight() - 1)//
                        temppix[j + i * src_width] = Color32((0x7BEF & (*src_surf_pos / 2)));//
                }//
                ++src_surf_pos;//
            }//
            src_surf_pos += uSrcPitch;//
        }//
    }//
    render->DrawTextureAlphaNew(pTargetPoint->x / float(window->GetWidth()), pTargetPoint->y / float(window->GetHeight()), temp);//
    temp->Release();//
}

bool Render::SwitchToWindow() {
    // pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    pViewport->ResetScreen();
    Release();

    pBackBuffer4 = nullptr;
    pFrontBuffer4 = nullptr;
    pDirectDraw4 = nullptr;
    CreateZBuffer();
    pRenderD3D = new RenderD3D;

    bool v7 = false;
    if (pRenderD3D->pAvailableDevices[uDesiredDirect3DDevice]
            .bIsDeviceCompatible &&
        uDesiredDirect3DDevice != 1) {
        v7 = pRenderD3D->CreateDevice(uDesiredDirect3DDevice, true);
    } else {
        if (!pRenderD3D->pAvailableDevices[0].bIsDeviceCompatible) {
            Error("There aren't any D3D devices to init.");
        }
        v7 = pRenderD3D->CreateDevice(0, true);
    }
    if (!v7) Error("D3Drend->Init failed.");

    pBackBuffer4 = pRenderD3D->pBackBuffer;
    pFrontBuffer4 = pRenderD3D->pFrontBuffer;
    pDirectDraw4 = pRenderD3D->pHost;

    unsigned int device_caps = pRenderD3D->GetDeviceCaps();
    if (device_caps & 1) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error("Direct3D renderer:  The device failed to return capabilities.");
    }
    if (device_caps & 0x3E) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error(
            "Direct3D renderer:  The device doesn't support the necessary "
            "alpha blending modes.");
    }
    if (device_caps & 0x80) {
        if (pRenderD3D) {
            pRenderD3D->Release();
            delete pRenderD3D;
        }
        pRenderD3D = nullptr;
        pBackBuffer4 = nullptr;
        pFrontBuffer4 = nullptr;
        pDirectDraw4 = nullptr;
        Error(
            "Direct3D renderer:  The device doesn't support non-square "
            "textures.");
    }

    bRequiredTextureStagesAvailable = CheckTextureStages();

    D3DDEVICEDESC halCaps = {0};
    halCaps.dwSize = sizeof(halCaps);

    D3DDEVICEDESC refCaps = {0};
    refCaps.dwSize = sizeof(refCaps);

    ErrD3D(pRenderD3D->pDevice->GetCaps(&halCaps, &refCaps));
    int v12 = halCaps.dwMinTextureWidth;
    if ((unsigned int)halCaps.dwMinTextureWidth > halCaps.dwMinTextureHeight)
        v12 = halCaps.dwMinTextureHeight;
    uMinDeviceTextureDim = v12;
    int v13 = halCaps.dwMaxTextureWidth;
    if ((unsigned int)halCaps.dwMaxTextureWidth < halCaps.dwMaxTextureHeight)
        v13 = halCaps.dwMaxTextureHeight;
    uMaxDeviceTextureDim = v13;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 1));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 1));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, 2));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, 0));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 0));
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

    ddpfPrimarySuface.dwSize = sizeof(DDPIXELFORMAT);
    GetTargetPixelFormat(&ddpfPrimarySuface);
    ParseTargetPixelFormat();

    if (!pRenderD3D) {
        __debugbreak();
    }

    p2DSurface = new Gdiplus::Bitmap(window->GetWidth(), window->GetHeight(),
                                     PixelFormat32bppRGB);
    p2DGraphics = new Gdiplus::Graphics(p2DSurface);
    pBeforePresentFunction = Present_NoColorKey;

    return true;
}

void Render::BeginLines2D() {
    return;
}

void Render::EndLines2D() {
    return;
}

void Render::RasterLine2D(int uX, int uY, int uZ, int uW, uint16_t color) {
    // change to 32bit clor input??

    unsigned int b = (color & 0x1F) << 3;
    unsigned int g = ((color >> 5) & 0x3F) << 2;
    unsigned int r = ((color >> 11) & 0x1F) << 3;
    Gdiplus::Pen pen(Gdiplus::Color(r, g, b));
    p2DGraphics->DrawLine(&pen, uX, uY, uZ, uW);
}



void Render::ParseTargetPixelFormat() {
    int v2 = 0;
    unsigned int uRedMask = ddpfPrimarySuface.dwRBitMask;
    this->uTargetBBits = 0;
    do {
        if ((1 << v2) & uRedMask) {
            ++this->uTargetRBits;
        }
        ++v2;
    } while (v2 < 32);

    unsigned int uGreenMask = ddpfPrimarySuface.dwGBitMask;
    int v5 = 0;
    do {
        if ((1 << v5) & uGreenMask) ++this->uTargetGBits;
        ++v5;
    } while (v5 < 32);

    unsigned int uBlueMask = ddpfPrimarySuface.dwBBitMask;
    int v7 = 0;
    do {
        if ((1 << v7) & uBlueMask) ++this->uTargetBBits;
        ++v7;
    } while (v7 < 32);
}

bool Render::LockSurface_DDraw4(IDirectDrawSurface4 *pSurface,
                                DDSURFACEDESC2 *pDesc,
                                unsigned int uLockFlags) {
    HRESULT result = pSurface->Lock(NULL, pDesc, uLockFlags, NULL);
    /*
    Когда объект DirectDrawSurface теряет поверхностную память, методы возвратят
    DDERR_SURFACELOST и не выполнят никакую другую функцию. Метод
    IDirectDrawSurface::Restore перераспределит поверхностную память и повторно
    присоединит ее к объекту DirectDrawSurface.
    */
    if (result == DDERR_SURFACELOST) {
        HRESULT v6 =
            pSurface->Restore();  //Восстанавливает потерянную поверхность. Это
                                  //происходит, когда поверхностная память,
                                  //связанная с объектом DirectDrawSurface была
                                  //освобождена.
        if (v6) {
            if (v6 !=
                DDERR_IMPLICITLYCREATED) {  // DDERR_IMPLICITLYCREATED -
                                            // Поверхность не может быть
                                            // восстановлена, потому что она -
                                            // неявно созданная поверхность.
                result = (bool)memset(pDesc, 0, 4);
                return 0;
            }
            this->pFrontBuffer4->Restore();
            pSurface->Restore();
        }
        result = pSurface->Lock(NULL, pDesc, DDLOCK_WAIT, NULL);
        if (result == DDERR_INVALIDRECT ||
            result ==
                DDERR_SURFACEBUSY) {  // DDERR_SURFACEBUSY - Доступ к этой
                                      // поверхности отказан, потому что
                                      // поверхность блокирована другой нитью.
                                      // DDERR_INVALIDRECT - Обеспечиваемый
                                      // прямоугольник недопустим.
            result = (bool)memset(pDesc, 0, 4);
            return 0;
        }
        ErrD3D(result);
        if (result) {
            result = (bool)memset(pDesc, 0, 4);
            return result;
        }
        if (pRenderD3D) {
            pRenderD3D->HandleLostResources();
        }
        result = this->pDirectDraw4->RestoreAllSurfaces();
    } else {
        if (result) {
            if (result == DDERR_INVALIDRECT || result == DDERR_SURFACEBUSY) {
                result = (bool)memset(pDesc, 0, 4);
                return result;
            }
            ErrD3D(result);
        }
    }
    return true;
}

void Render::CreateClipper() {
    ErrD3D(pDirectDraw4->CreateClipper(0, &pDDrawClipper, NULL));
    ErrD3D(pDDrawClipper->SetHWnd(0, (HWND)window->SystemHandle()));
    ErrD3D(pFrontBuffer4->SetClipper(pDDrawClipper));
}

void Render::GetTargetPixelFormat(DDPIXELFORMAT *pOut) {
    pFrontBuffer4->GetPixelFormat(pOut);
}

void Render::RestoreFrontBuffer() {
    if (pFrontBuffer4->IsLost() == DDERR_SURFACELOST) {
        pFrontBuffer4->Restore();
    }
}

void Render::RestoreBackBuffer() {
    if (pBackBuffer4->IsLost() == DDERR_SURFACELOST) {
        pBackBuffer4->Restore();
    }
}

void Render::BltBackToFontFast(int a2, int a3, Recti *pSrcRect) {
    IDirectDrawSurface *pFront;
    IDirectDrawSurface *pBack;
    pFront = (IDirectDrawSurface *)this->pFrontBuffer4;
    pBack = (IDirectDrawSurface *)this->pBackBuffer4;

    RECT rect;
    rect.left = pSrcRect->x;
    rect.top = pSrcRect->y;
    rect.right = pSrcRect->x + pSrcRect->w;
    rect.bottom = pSrcRect->y + pSrcRect->h;
    pFront->BltFast(NULL, NULL, pBack, &rect, DDBLTFAST_WAIT);
}

unsigned int Render::GetBillboardDrawListSize() {
    return render->uNumBillboardsToDraw;
}

unsigned int Render::GetParentBillboardID(unsigned int uBillboardID) {
    return render->pBillboardRenderListD3D[uBillboardID].sParentBillboardID;
}

void Render::BeginSceneD3D() {
    if (!uNumD3DSceneBegins++) {
        pRenderD3D->ClearTarget(true, /*0x00F08020*/0x00000000, true, 1.0);
        render->uNumBillboardsToDraw = 0;
        pRenderD3D->pDevice->BeginScene();

        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
            uFogColor = GetLevelFogColor();
        else
            uFogColor = 0;

        if (uFogColor & 0xFF000000) {
            pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, 1);
            pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
                                                uFogColor & 0xFFFFFF);
            pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0);
            engine->SetSpecular_FogIsOn(true);
        } else {
            pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, 0);
            engine->SetSpecular_FogIsOn(false);
        }
    }
}

void Render::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {
    --uNumD3DSceneBegins;
    if (uNumD3DSceneBegins == 0) {
        engine->draw_debug_outlines();
        DoRenderBillboards_D3D();
        spell_fx_renderer->RenderSpecialEffects();
        pRenderD3D->pDevice->EndScene();
    }
}

unsigned int Render::GetActorTintColor(int DimLevel, int tint, float WorldViewX, int a5, RenderBillboard *Billboard) {
    // GetActorTintColor(int max_dimm, int min_dimm, float distance, int a4, RenderBillboard *a5)
    return ::GetActorTintColor(DimLevel, tint, WorldViewX, a5, Billboard);
}

void Render::DrawTerrainPolygon(struct Polygon *a4, bool transparent,
                                bool clampAtTextureBorders) {
    int v11;           // eax@5
    unsigned int v45;  // eax@28

    unsigned int uNumVertices = a4->uNumVertices;

    auto texture = (TextureD3D *)a4->texture;

    if (!this->uNumD3DSceneBegins) return;
    if (uNumVertices < 3) return;

    if (false) {
        __debugbreak();
        v11 =
            ::GetActorTintColor(a4->dimming_level, 0,
                                VertexRenderList[0].vWorldViewPosition.x, 0, 0);
        lightmap_builder->DrawLightmaps(v11 /*, 0*/);
    } else if (transparent || !lightmap_builder->StationaryLightsCount) {
        // send for batching

        for (int z = 0; z < (uNumVertices - 2); z++) {
            // 123, 134, 145, 156..
            BatchTriangles* thistri = &BatchTrianglesStore[NumBatchTrianglesStore];

            thistri->texture = a4->texture;
            thistri->texname = a4->texture->GetName();
            thistri->trans = transparent;

            // copy first
            //for (uint i = 0; i < uNumVertices; ++i) {
            thistri->Verts[0].pos.x = VertexRenderList[0].vWorldViewProjX;
            thistri->Verts[0].pos.y = VertexRenderList[0].vWorldViewProjY;
            thistri->Verts[0].pos.z =
                    1.0 - 1.0 / ((VertexRenderList[0].vWorldViewPosition.x * 1000) /
                        pCamera3D->GetFarClip());
            thistri->Verts[0].rhw =
                    1.0 / (VertexRenderList[0].vWorldViewPosition.x + 0.0000001);
            thistri->Verts[0].diffuse = ::GetActorTintColor(
                    a4->dimming_level, 0, VertexRenderList[0].vWorldViewPosition.x,
                    0, 0);
            thistri->Verts[0].specular = 0;
                if (engine->IsSpecular_FogIsOn())
                    thistri->Verts[0].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[0].vWorldViewPosition.x);

                thistri->Verts[0].texcoord.x = VertexRenderList[0].u;
                thistri->Verts[0].texcoord.y = VertexRenderList[0].v;
            //}

            // copy other two (z+1)(z+2)
            for (uint i = 1; i < 3; ++i) {
                thistri->Verts[i].pos.x = VertexRenderList[z+i].vWorldViewProjX;
                thistri->Verts[i].pos.y = VertexRenderList[z+ i].vWorldViewProjY;
                thistri->Verts[i].pos.z =
                    1.0 - 1.0 / ((VertexRenderList[z+ i].vWorldViewPosition.x * 1000) /
                        pCamera3D->GetFarClip());
                thistri->Verts[i].rhw =
                    1.0 / (VertexRenderList[z+ i].vWorldViewPosition.x + 0.0000001);
                thistri->Verts[i].diffuse = ::GetActorTintColor(
                    a4->dimming_level, 0, VertexRenderList[z+ i].vWorldViewPosition.x,
                    0, 0);
                thistri->Verts[i].specular = 0;
                if (engine->IsSpecular_FogIsOn())
                    thistri->Verts[i].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[z+ i].vWorldViewPosition.x);

                thistri->Verts[i].texcoord.x = VertexRenderList[z+i].u;
                thistri->Verts[i].texcoord.y = VertexRenderList[z+ i].v;
            }



            ++NumBatchTrianglesStore;
            if (NumBatchTrianglesStore > 9500) BatchTriDraw();
        }


    } else if (lightmap_builder->StationaryLightsCount) {
        // return;
        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldViewProjX;
            d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldViewProjY;
            d3d_vertex_buffer[i].pos.z =
                1.0 - 1.0 / ((VertexRenderList[i].vWorldViewPosition.x * 1000) /
                             pCamera3D->GetFarClip());
            d3d_vertex_buffer[i].rhw =
                1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
            d3d_vertex_buffer[i].diffuse = GetActorTintColor(a4->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
            d3d_vertex_buffer[i].specular = 0;
            if (engine->IsSpecular_FogIsOn())
                d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                    0, 0, VertexRenderList[i].vWorldViewPosition.x);
            d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
            d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                                                   FALSE));
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
                                                         D3DTADDRESS_WRAP));
        if (engine->IsSpecular_FogIsOn())
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

        ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLEFAN,  //рисуется текстурка с светом
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
        drawcalls++;

        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));

        lightmap_builder->DrawLightmaps(-1 /*, 0*/);

        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].diffuse = 0xFFFFFFFF; /*-1;*/
        }
        ErrD3D(pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture()));  //текстурка
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
        if (!engine->IsSpecular_FogIsOn()) {
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                                   D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                                   D3DBLEND_SRCCOLOR));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
        drawcalls++;

        if (engine->IsSpecular_FogIsOn()) {
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
            ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].diffuse =
                    render->uFogColor |
                    d3d_vertex_buffer[i].specular & 0xFF000000;
                d3d_vertex_buffer[i].specular = 0;
            }

            ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));  // problem
           ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                                       D3DBLEND_INVSRCALPHA));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                                       D3DBLEND_SRCALPHA));
            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
            drawcalls++;
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,
                                                       TRUE));
            v45 = GetLevelFogColor();
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
                                                       v45 & 0xFFFFFF));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, FALSE));
        }
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
        //}
    }

    if (engine->config->debug.Terrain.Get())
        pCamera3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x00FFFFFF, 0.0);
}



void Render::DrawIndoorBatched() {
    pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);  // buildings
    BatchTriDraw();
    // NumBatchTrianglesStore = 0;
}

void Render::DrawIndoorFaces() {
    for (uint i = 0; i < pBspRenderer->num_faces; ++i) {
        // viewed through portal
        /*IndoorLocation::ExecDraw_d3d(pBspRenderer->faces[i].uFaceID,
            pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].ViewportNodeFrustum,
            4, pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].pPortalBounding);*/
        unsigned int uFaceID = pBspRenderer->faces[i].uFaceID;
        IndoorCameraD3D_Vec4 *portalfrustumnorm = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].ViewportNodeFrustum;
        unsigned int uNumFrustums = 4;
        RenderVertexSoft* pPortalBounding = pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].pPortalBounding;

                 uint ColourMask;  // ebx@25
            // IDirect3DTexture2 *v27; // eax@42
            unsigned int uNumVerticesa;  // [sp+24h] [bp-4h]@17
            int LightLevel;                     // [sp+34h] [bp+Ch]@25

            if (uFaceID >= pIndoor->pFaces.size())
                continue;

            static RenderVertexSoft static_vertices_buff_in[64];  // buff in
            static RenderVertexSoft static_vertices_calc_out[64];  // buff out - calc portal shape

            static stru154 FacePlaneHolder;  // idb


            BLVFace* pFace = &pIndoor->pFaces[uFaceID];

            if (pFace->Portal()) {
                // pCamera3D->DebugDrawPortal(pFace);
                continue;
            }

            if (pFace->uNumVertices < 3) continue;

            if (pFace->Invisible()) {
                continue;
            }


            // stack decals outside of clipping now

            if (decal_builder->bloodsplat_container->uNumBloodsplats) {
                decal_builder->ApplyBloodsplatDecals_IndoorFace(uFaceID);
                if (decal_builder->uNumSplatsThisFace) {
                    FacePlaneHolder.face_plane.vNormal.x = pFace->pFacePlane.vNormal.x;
                    FacePlaneHolder.polygonType = pFace->uPolygonType;
                    FacePlaneHolder.face_plane.vNormal.y = pFace->pFacePlane.vNormal.y;
                    FacePlaneHolder.face_plane.vNormal.z = pFace->pFacePlane.vNormal.z;
                    FacePlaneHolder.face_plane.dist = pFace->pFacePlane.dist;

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

                    // blood draw
                    decal_builder->BuildAndApplyDecals(Lights.uCurrentAmbientLightLevel, LocationIndoors, &FacePlaneHolder,
                        pFace->uNumVertices, static_vertices_buff_in,
                        0, pFace->uSectorID);
                }
            }

            if (!pFace->GetTexture()) {
                continue;
            }


            if (pCamera3D->is_face_faced_to_cameraBLV(pFace)) {
                uNumVerticesa = pFace->uNumVertices;

                // copy to buff in
                for (uint i = 0; i < pFace->uNumVertices; ++i) {
                    static_vertices_buff_in[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
                    static_vertices_buff_in[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
                    static_vertices_buff_in[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
                    static_vertices_buff_in[i].u = (signed short)pFace->pVertexUIDs[i];
                    static_vertices_buff_in[i].v = (signed short)pFace->pVertexVIDs[i];
                }

                // check if this face is visible through current portal node
                if (pCamera3D->CullFaceToFrustum(static_vertices_buff_in, &uNumVerticesa, static_vertices_calc_out, portalfrustumnorm, 4)/* ||  true*/
                    // pCamera3D->ClipFaceToFrustum(static_vertices_buff_in, &uNumVerticesa, static_vertices_calc_out, portalfrustumnorm, 4, 0, 0) || true
                    ) {
                    ++pBLVRenderParams->uNumFacesRenderedThisFrame;

                    /*for (uint i = 0; i < pFace->uNumVertices; ++i) {
                        static_vertices_calc_out[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
                        static_vertices_calc_out[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
                        static_vertices_calc_out[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
                        static_vertices_calc_out[i].u = (signed short)pFace->pVertexUIDs[i];
                        static_vertices_calc_out[i].v = (signed short)pFace->pVertexVIDs[i];
                    }*/

                    /*int xd = pParty->vPosition.x - pIndoor->pVertices[pFace->pVertexIDs[0]].x;
                    int yd = pParty->vPosition.y - pIndoor->pVertices[pFace->pVertexIDs[0]].y;
                    int zd = pParty->vPosition.z - pIndoor->pVertices[pFace->pVertexIDs[0]].z;
                    int dist = sqrt(xd * xd + yd * yd + zd * zd);*/

                    // if (dist < 2000) {
                    pFace->uAttributes |= FACE_SeenByParty;
                    //}

                    FaceFlowTextureOffset(uFaceID);

                    lightmap_builder->ApplyLights_IndoorFace(uFaceID);

                    LightLevel = Lights.uCurrentAmbientLightLevel & 31;
                    // lightlevel is 0 to 31
                    //if (LightLevel < 5) LightLevel = 5;

                    ColourMask = ((LightLevel << 3)) | ((LightLevel << 3)) << 8 | ((LightLevel << 3)) << 16;

                    pCamera3D->ViewTransfrom_OffsetUV(static_vertices_calc_out, uNumVerticesa, array_507D30, &Lights);
                    pCamera3D->Project(array_507D30, uNumVerticesa, 0);

                    lightmap_builder->StationaryLightsCount = 0;
                    if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                        FacePlaneHolder.face_plane.vNormal.x = pFace->pFacePlane.vNormal.x;
                        FacePlaneHolder.polygonType = pFace->uPolygonType;
                        FacePlaneHolder.face_plane.vNormal.y = pFace->pFacePlane.vNormal.y;
                        FacePlaneHolder.face_plane.vNormal.z = pFace->pFacePlane.vNormal.z;
                        FacePlaneHolder.face_plane.dist = pFace->pFacePlane.dist;
                    }

                    if (Lights.uNumLightsApplied > 0 && !pFace->Indoor_sky())  // for torchlight(для света факелов)
                        lightmap_builder->ApplyLights(&Lights, &FacePlaneHolder, uNumVerticesa, array_507D30, /*pVertices*/0, 0);

                    Texture* face_texture = pFace->GetTexture();
                    if (pFace->Fluid()) {
                        face_texture = (Texture*)pFace->resource;
                        uint eightSeconds = platform->TickCount() % 8000;
                        float angle = (eightSeconds / 8000.0f) * 2 * 3.1415f;

                        // animte lava back and forth
                        for (uint i = 0; i < uNumVerticesa; ++i)
                            array_507D30[i].v += (face_texture->GetHeight() - 1) * cosf(angle);
                    } else if (pFace->IsTextureFrameTable()) {
                        face_texture = pTextureFrameTable->GetFrameTexture((int64_t)pFace->resource, pBLVRenderParams->field_0_timer_);
                    }

                    if (pFace->Indoor_sky()) {
                        render->DrawIndoorSky(uNumVerticesa, uFaceID);
                    } else {
                        render->DrawIndoorPolygon(uNumVerticesa, pFace, PID(OBJECT_Face, uFaceID), ColourMask, 0);
                    }
                }
            }
    }
    DrawIndoorBatched();
}

Sizei Render::GetRenderDimensions() {
    return window->Size();
}
Sizei Render::GetPresentDimensions() {
    return window->Size();
}

bool Render::Reinitialize(bool firstInit) {
    // blank here
    return true;
}

void Render::ReloadShaders() {
    // blank here
}


void Render::DrawIndoorPolygon(unsigned int uNumVertices, BLVFace *pFace,
                               int uPackedID, unsigned int uColor, int a8) {
    if (!uNumD3DSceneBegins || uNumVertices < 3) {
        return;
    }

    unsigned int sCorrectedColor = uColor;

    TextureD3D *texture = (TextureD3D *)pFace->GetTexture();

    if (lightmap_builder->StationaryLightsCount) {
        sCorrectedColor =  0xFFFFFFFF/*-1*/;
    }


    // perception
    // engine->AlterGamma_BLV(pFace, &sCorrectedColor);

    if (engine->IsSaturateFaces() && (pFace->uAttributes & FACE_IsSecret)) {
        uint eightSeconds = platform->TickCount() % 3000;
        float angle = (eightSeconds / 3000.0f) * 2 * 3.1415f;

        int redstart = (sCorrectedColor & 0x00FF0000) >> 16;

        int col = redstart * abs(cosf(angle));
        // (a << 24) | (r << 16) | (g << 8) | b;
        sCorrectedColor = (0xFF << 24) | (redstart << 16) | (col << 8) | col;
    }

    if (pFace->uAttributes & FACE_OUTLINED) {
        if (platform->TickCount() % 300 >= 150)
            uColor = sCorrectedColor = 0xFF20FF20;
        else
            uColor = sCorrectedColor = 0xFF109010;

        /*for (int out = 0; out < uNumVertices; out++) {
            log->Info(L"X: %.6f, Y: %.6f, Z: %.6f", array_507D30[out].vWorldViewProjX, array_507D30[out].vWorldViewProjY, array_507D30[out].vWorldViewPosition.x);
        }
        log->Info(L"FAce done");*/
        RenderVertexSoft cam;
        cam.vWorldPosition.x = pCamera3D->vCameraPos.x;
        cam.vWorldPosition.y = pCamera3D->vCameraPos.y;
        cam.vWorldPosition.z = pCamera3D->vCameraPos.z;
        pCamera3D->do_draw_debug_line_sw(&cam, -1, &vis->debugpick, 0xFFFF00u, 0, 0);
    }

    if (false) {
        // __debugbreak();
    } else {
        if (!lightmap_builder->StationaryLightsCount) {
           // return;

            // send for batching

            for (int z = 0; z < (uNumVertices - 2); z++) {
                // 123, 134, 145, 156..
                BatchTriangles* thistri = &BatchTrianglesStore[NumBatchTrianglesStore];

                thistri->texture = pFace->GetTexture();
                thistri->texname = pFace->GetTexture()->GetName();
                thistri->trans = false;

                // copy first
                //for (uint i = 0; i < uNumVertices; ++i) {
                thistri->Verts[0].pos.x = array_507D30[0].vWorldViewProjX;
                thistri->Verts[0].pos.y = array_507D30[0].vWorldViewProjY;
                thistri->Verts[0].pos.z =
                    1.0 -
                    1.0 / (array_507D30[0].vWorldViewPosition.x * 0.061758894);
                thistri->Verts[0].rhw =
                    1.0 / array_507D30[0].vWorldViewPosition.x;
                thistri->Verts[0].diffuse = sCorrectedColor;
                thistri->Verts[0].specular = 0;
                if (engine->IsSpecular_FogIsOn())
                    thistri->Verts[0].specular = 0;

                thistri->Verts[0].texcoord.x = array_507D30[0].u / (double)pFace->GetTexture()->GetWidth();
                thistri->Verts[0].texcoord.y = array_507D30[0].v / (double)pFace->GetTexture()->GetHeight();
                //}

                // copy other two (z+1)(z+2)
                for (uint i = 1; i < 3; ++i) {
                    thistri->Verts[i].pos.x = array_507D30[z + i].vWorldViewProjX;
                    thistri->Verts[i].pos.y = array_507D30[z + i].vWorldViewProjY;
                    thistri->Verts[i].pos.z =
                        1.0 -
                        1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
                    thistri->Verts[i].rhw =
                        1.0 / array_507D30[i].vWorldViewPosition.x;
                    thistri->Verts[i].diffuse = sCorrectedColor;
                    thistri->Verts[i].specular = 0;
                    if (engine->IsSpecular_FogIsOn())
                        thistri->Verts[i].specular = 0;

                    thistri->Verts[i].texcoord.x = array_507D30[z + i].u / (double)pFace->GetTexture()->GetWidth();
                    thistri->Verts[i].texcoord.y = array_507D30[z + i].v / (double)pFace->GetTexture()->GetHeight();
                }



                ++NumBatchTrianglesStore;
                if (NumBatchTrianglesStore > 9500) BatchTriDraw();
            }


        } else {
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z =
                    1.0 -
                    1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
                d3d_vertex_buffer[i].rhw = 1.0 / array_507D30[i].vWorldViewPosition.x;
                d3d_vertex_buffer[i].diffuse = uColor;
                d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x =
                    array_507D30[i].u / (double)pFace->GetTexture()->GetWidth();
                d3d_vertex_buffer[i].texcoord.y =
                    array_507D30[i].v /
                    (double)pFace->GetTexture()->GetHeight();
            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
            ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));

            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
            drawcalls++;

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));

            lightmap_builder->DrawLightmaps(-1 /*, 0*/);

            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
            }

            ErrD3D(pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture()));
            ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR));

            ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
                D3DPT_TRIANGLEFAN,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
            drawcalls++;

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
        }
    }

    if (engine->config->debug.Terrain.Get())
        pCamera3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x00FF0000, 0.0);
}

void Render::DrawProjectile(float srcX, float srcY, float a3, float a4,
                            float dstX, float dstY, float a7, float a8,
                            Texture *texture) {
    double v20;  // st4@8
    double v21;  // st4@10
    double v22;  // st4@10
    double v23;  // st4@10
    double v25;  // st4@11
    double v26;  // st4@13
    double v28;  // st4@13

    TextureD3D *textured3d = (TextureD3D *)texture;

    int xDifference = bankersRounding(dstX - srcX);
    int yDifference = bankersRounding(dstY - srcY);
    int absYDifference = abs(yDifference);
    int absXDifference = abs(xDifference);
    unsigned int smallerabsdiff = std::min(absXDifference, absYDifference);
    unsigned int largerabsdiff = std::max(absXDifference, absYDifference);
    int v32 = (11 * smallerabsdiff >> 5) + largerabsdiff;
    double v16 = 1.0 / (double)v32;
    double v17 = (double)yDifference * v16 * a4;
    double v18 = (double)xDifference * v16 * a4;
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        v20 = a3 * 1000.0 / pCamera3D->GetFarClip();
        v25 = a7 * 1000.0 / pCamera3D->GetFarClip();
    } else {
        v20 = a3 * 0.061758894;
        v25 = a7 * 0.061758894;
    }
    v21 = 1.0 / a3;
    v22 = (double)yDifference * v16 * a8;
    v23 = (double)xDifference * v16 * a8;
    v26 = 1.0 - 1.0 / v25;
    v28 = 1.0 / a7;

    RenderVertexD3D3 v29[4];
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

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
                                               D3DCULL_NONE));
    ErrD3D(
        pRenderD3D->pDevice->SetTexture(0, textured3d->GetDirect3DTexture()));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, v29, 4,
        24));
    drawcalls++;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ZERO));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
                                               D3DCULL_CW));
}

void Render::BillboardSphereSpellFX(SpellFX_Billboard *a1,
                                      int diffuse) {
    if (a1->uNumVertices < 3) {
        return;
    }

    float depth = 1000000.0;
    for (uint i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        if (a1->field_104[i].z < depth) {
            depth = a1->field_104[i].z;
        }
    }

    unsigned int v5 = Billboard_ProbablyAddToListAndSortByZOrder(depth);
    pBillboardRenderListD3D[v5].field_90 = 0;
    pBillboardRenderListD3D[v5].sParentBillboardID = -1;
    pBillboardRenderListD3D[v5].opacity = RenderBillboardD3D::Opaque_2;
    pBillboardRenderListD3D[v5].texture = 0;
    pBillboardRenderListD3D[v5].uNumVertices = a1->uNumVertices;
    pBillboardRenderListD3D[v5].z_order = depth;

    for (unsigned int i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        pBillboardRenderListD3D[v5].pQuads[i].pos.x = a1->field_104[i].x;
        pBillboardRenderListD3D[v5].pQuads[i].pos.y = a1->field_104[i].y;



        // if (a1->field_104[i].z < 17) a1->field_104[i].z = 17;
        float rhw = 1.f / a1->field_104[i].z;
        float z = 1.f - 1.f / (a1->field_104[i].z * 1000.f / pCamera3D->GetFarClip());



        double v10 = a1->field_104[i].z;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            v10 *= 1000.f / 16192.f;
        } else {
            v10 *= 1000.f / pCamera3D->GetFarClip();
        }


        pBillboardRenderListD3D[v5].pQuads[i].pos.z = z;  // 1.0 - 1.0 / v10;
        pBillboardRenderListD3D[v5].pQuads[i].rhw = rhw;  // 1.0 / a1->field_104[i].z;

        int v12;
        if (diffuse & 0xFF000000) {
            v12 = a1->field_104[i].diffuse;
        } else {
            v12 = diffuse;
        }
        pBillboardRenderListD3D[v5].pQuads[i].diffuse = v12;
        pBillboardRenderListD3D[v5].pQuads[i].specular = 0;

        pBillboardRenderListD3D[v5].pQuads[i].texcoord.x = 0.0;
        pBillboardRenderListD3D[v5].pQuads[i].texcoord.y = 0.0;
    }
}

void Render::Update_Texture(Texture *texture) {
    // nothing
}
void Render::DeleteTexture(Texture *texture) {
    // nothing
}

void Render::RemoveTextureFromDevice(Texture* texture) {
    auto t = (TextureD3D*)texture;
    if (t->initialized) {
        IDirectDrawSurface* dds_get = t->GetDirectDrawSurface();
        IDirect3DTexture2* d3dt_get = t->GetDirect3DTexture();
        if (dds_get) dds_get->Release();
        if (d3dt_get) d3dt_get->Release();
    }
}

bool Render::MoveTextureToDevice(Texture *texture) {
    auto t = (TextureD3D *)texture;
    if (t) {
        auto pixels = (uint16_t *)t->GetPixels(IMAGE_FORMAT_A1R5G5B5);

        IDirectDrawSurface4 *dds;
        IDirect3DTexture2 *d3dt;

        if (!pRenderD3D->CreateTexture(t->GetWidth(), t->GetHeight(), &dds,
                                       &d3dt, true, false,
                                       uMinDeviceTextureDim))
            Error(
                "HiScreen16::LoadTexture - D3Drend->CreateTexture() failed: %x",
                0);

        DDSCAPS2 v19;
        memset(&v19, 0, sizeof(DDSCAPS2));
        v19.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

        DDSURFACEDESC2 desc;
        memset(&desc, 0, sizeof(DDSURFACEDESC2));
        desc.dwSize = sizeof(DDSURFACEDESC2);

        if (LockSurface_DDraw4(dds, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY)) {
            auto v13 = pixels;
            auto v14 = (uint16_t *)desc.lpSurface;
            for (uint bMipMaps = 0; bMipMaps < desc.dwHeight; bMipMaps++) {
                for (auto v15 = 0; v15 < desc.dwWidth; v15++) {
                    *v14 = *v13;
                    ++v14;
                    ++v13;
                }
                v14 += (desc.lPitch >> 1) - desc.dwWidth;
            }
            ErrD3D(dds->Unlock(NULL));
        }

        t->SetDirect3DTexture2(d3dt);
        t->SetDirectDrawSurface((IDirectDrawSurface *)dds);

        return true;
    }
    return false;
}

void Render::BeginScene() {}

void Render::EndScene() {}

void Render::ScreenFade(unsigned int color, float t) {
    unsigned int v3 = 0;

    //{
    if (t > 1.0f)
        t = 1.0f;
    else if (t < 0.0f)
        t = 0.0f;

    int v40 = (char)floorf(t * 255.0f + 0.5f);

    unsigned int v7 = color | (v40 << 24);

    RenderVertexD3D3 v36[4];
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
    v36[1].pos.y = (double)(pViewport->uViewportBR_Y);
    v36[1].pos.z = 0.0;
    v36[1].diffuse = v7;
    v36[1].rhw = 1.0;
    v36[1].texcoord.x = 0.0;
    v36[1].texcoord.y = 0.0;

    v36[2].specular = 0;
    v36[2].pos.x = (double)pViewport->uViewportBR_X;
    v36[2].pos.y = (double)(pViewport->uViewportBR_Y);
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
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_SRCALPHA));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_INVSRCALPHA));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,
                                               D3DCMP_ALWAYS));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, v36, 4,
        28));
    drawcalls++;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               FALSE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));
    /*}
    else
    {
    v40 = (1.0 - a3) * 65536.0;
    v39 = v40 + 6.7553994e15;
    LODWORD(a3) = LODWORD(v39);
    v38 = (signed int)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) >>
    1; HIDWORD(v39) = pViewport->uViewportBR_Y - pViewport->uViewportTL_Y + 1;
    v13 = pViewport->uViewportTL_X + ecx0->uTargetSurfacePitch -
    pViewport->uViewportBR_X; v14 =
    &ecx0->pTargetSurface[pViewport->uViewportTL_X + pViewport->uViewportTL_Y *
    ecx0->uTargetSurfacePitch]; v37 = 2 * v13; LODWORD(v40) = (int)v14;

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
    this_ = (((65536 - LODWORD(a3)) * (uint16_t)(v17 & 0xF800) &
    0xF800FFFF | v18 & 0x1F0000 | (65536 - LODWORD(a3)) * (v17 & 0x7E0) &
    0x7E00000u) >> 16 << 16) | (((65536 - LODWORD(a3)) * (uint16_t)(v17
    & 0xF800) & 0xF800FFFF | v18 & 0x1F0000 | (65536 - LODWORD(a3)) * (v17 &
    0x7E0) & 0x7E00000u) >> 16); v19 = v40; v20 = off_4EFDB0; v21 =
    HIDWORD(v39); do
    {
    v22 = v38;
    v31 = v21;
    do
    {
    v23 = (*(int *)((char *)v20
    + ((((uint16_t)(*(short *)((char *)v20 + ((*(unsigned int
    *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | *(unsigned int *)LODWORD(v19) &
    0x7FF) & 0x7C0u) >> 4)) << 6) | (*(int *)((char *)v20 + ((((*(int *)((char
    *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int
    *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) <<
    27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0x7C00000u) >> 20)) <<
    22) | ((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >>
    9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) &
    0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19) & 0x7FF07FF) &
    0xF81FF81F; result = this_
    + (*((int *)v20
    + (((uint8_t)(*((char *)v20
    + ((((uint16_t)(*(short *)((char *)v20
    + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | *(unsigned int
    *)LODWORD(v19) & 0x7FF) & 0x7C0u) >> 4)) << 6) | *(unsigned int
    *)LODWORD(v19) & 0x1F) & 0x1F)) | (*(int *)((char *)v20 + ((v23 & 0x1F0000u)
    >> 14)) << 16) | ((*(int *)((char *)v20 + ((((uint16_t)(*(short
    *)((char *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) |
    *(unsigned int *)LODWORD(v19) & 0x7FF) & 0x7C0u) >> 4)) << 6) | (*(int
    *)((char *)v20 + ((((*(int *)((char *)v20 + ((*(unsigned int *)LODWORD(v19)
    & 0xF800u) >> 9)) << 11) | (*(int *)((char *)v20 + ((*(unsigned int
    *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) | *(unsigned int *)LODWORD(v19)
    & 0x7FF07FF) & 0x7C00000u) >> 20)) << 22) | ((*(int *)((char *)v20 +
    ((*(unsigned int *)LODWORD(v19) & 0xF800u) >> 9)) << 11) | (*(int *)((char
    *)v20 + ((*(unsigned int *)LODWORD(v19) & 0xF8000000u) >> 25)) << 27) |
    *(unsigned int *)LODWORD(v19) & 0x7FF07FF) & 0xF81FF81F) & 0xFFE0FFE0);
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
    this_ = (((65536 - LODWORD(a3)) * (v24 & 0x7C00) & 0x7C000000 | v25 &
    0x1F0000 | (65536 - LODWORD(a3))
    * (v24 & 0x3E0) & 0x3E00000u) >> 16 << 16) | (((65536 - LODWORD(a3)) * (v24
    & 0x7C00) & 0x7C000000 | v25 & 0x1F0000 | (65536 - LODWORD(a3)) * (v24 &
    0x3E0) & 0x3E00000u) >> 16); v26 = v40; v27 = (char *)off_4EFDB0; v28 =
    HIDWORD(v39); do
    {
    v29 = v38;
    v32 = v28;
    do
    {
    v30 = 32
    * *(int *)&v27[(((uint16_t)(*(short *)&v27[(*(unsigned int
    *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | *(unsigned int *)LODWORD(v26) &
    0x3FF) & 0x3E0u) >> 3] | (*(int *)&v27[(((*(int *)&v27[(*(unsigned int
    *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int
    *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26)
    & 0x3FF03FF) & 0x3E00000u) >> 19] << 21) | ((*(int *)&v27[(*(unsigned int
    *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int *)&v27[(*(unsigned int
    *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) | *(unsigned int *)LODWORD(v26)
    & 0x3FF03FF) & 0x7C1F7C1F; result = this_
    + (*(int *)&v27[4
    * (((uint8_t)(32
    * v27[(((uint16_t)(*(short *)&v27[(*(unsigned int *)LODWORD(v26) &
    0x7C00u) >> 8] << 10) | *(unsigned int *)LODWORD(v26) & 0x3FF) & 0x3E0u) >>
    3]) | *(unsigned int *)LODWORD(v26) & 0x1F) & 0x1F)] | (*(int *)&v27[(v30 &
    0x1F0000u) >> 14] << 16) | (32 * *(int *)&v27[(((uint16_t)(*(short
    *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | *(unsigned
    int *)LODWORD(v26) & 0x3FF) & 0x3E0u) >> 3] | (*(int *)&v27[(((*(int
    *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) | (*(int
    *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) |
    *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x3E00000u) >> 19] << 21) |
    ((*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C00u) >> 8] << 10) |
    (*(int *)&v27[(*(unsigned int *)LODWORD(v26) & 0x7C000000u) >> 24] << 26) |
    *(unsigned int *)LODWORD(v26) & 0x3FF03FF) & 0x7C1F7C1F) & 0xFFE0FFE0);
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

void Render::SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ,
                           unsigned int uW) {
    p2DGraphics->SetClip(Gdiplus::Rect(uX, uY, uZ - uX, uW - uY));
}

void Render::ResetUIClipRect() {
    p2DGraphics->SetClip(
        Gdiplus::Rect(0, 0, window->GetWidth(), window->GetHeight()));
}

Gdiplus::Bitmap *Render::BitmapWithImage(Image *image) {
    if (image == nullptr) {
        return nullptr;
    }

    Gdiplus::PixelFormat format = 0;
    uint8_t *data = nullptr;
    int stride = image->GetWidth();

    switch (image->GetFormat()) {
        case IMAGE_FORMAT_R5G6B5: {
            format = PixelFormat16bppRGB565;
            data = (uint8_t *)image->GetPixels(IMAGE_FORMAT_R5G6B5);
            stride *= 2;
            break;
        }
        case IMAGE_FORMAT_A1R5G5B5: {
            format = PixelFormat16bppARGB1555;
            data = (uint8_t *)image->GetPixels(IMAGE_FORMAT_A1R5G5B5);
            stride *= 2;
            break;
        }
        case IMAGE_FORMAT_A8R8G8B8: {
            format = PixelFormat32bppARGB;
            data = (uint8_t *)image->GetPixels(IMAGE_FORMAT_A8R8G8B8);
            stride *= 4;
            break;
        }
        case IMAGE_FORMAT_R8G8B8: {
            format = PixelFormat32bppRGB;
            data = (uint8_t *)image->GetPixels(IMAGE_FORMAT_R8G8B8);
            stride *= 4;
            break;
        }
        case IMAGE_FORMAT_R8G8B8A8:
        default:
            return nullptr;
    }

    if (data == nullptr) {
        return nullptr;
    }

    Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(
        image->GetWidth(), image->GetHeight(), stride, format, data);
    if (bitmap->GetLastStatus() != Gdiplus::Ok) {
        delete bitmap;
        bitmap = nullptr;
    }

    return bitmap;
}

void Render::DrawTextureCustomHeight(float u, float v, class Image *image,
                                     int custom_height) {
    Gdiplus::Bitmap *bitmap = BitmapWithImage(image);
    if (bitmap == nullptr) {
        return;
    }

    int x = window->GetWidth() * u;
    int y = window->GetHeight() * v;

    p2DGraphics->DrawImage(bitmap, x, y, 0, 0, image->GetWidth(), custom_height,
        Gdiplus::UnitPixel);

    delete bitmap;
}

void Render::DrawTextureNew(float u, float v, Image *bmp, uint32_t colourmask) {
    DrawTextureCustomHeight(u, v, bmp, bmp->GetHeight());
}

void Render::DrawTextureOffset(int x, int y, int offset_x, int offset_y,
                               Image *image) {
    Gdiplus::Bitmap *bitmap = BitmapWithImage(image);
    if (bitmap == nullptr) {
        return;
    }

    p2DGraphics->DrawImage(bitmap, x, y, offset_x, offset_y,
                           image->GetWidth() - offset_x,
                           image->GetHeight() - offset_y, Gdiplus::UnitPixel);

    delete bitmap;
}

void Render::DrawImage(Image *image, const Recti &rect, const uint paletteid) {
    Gdiplus::Bitmap *bitmap = BitmapWithImage(image);
    if (bitmap == nullptr) {
        return;
    }

    Gdiplus::Rect r(rect.x, rect.y, rect.w, rect.h);
    p2DGraphics->DrawImage(bitmap, r);

    delete bitmap;
}

void Render::DrawTextureGrayShade(float u, float v, Image *img) {
    DrawMasked(u, v, img, 1, 0x7BEF);
}

void Render::FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth,
                          unsigned int uHeight, unsigned int color) {
    unsigned int b = (color & 0x1F) << 3;
    unsigned int g = ((color >> 5) & 0x3F) << 2;
    unsigned int r = ((color >> 11) & 0x1F) << 3;

    Gdiplus::Color c(r, g, b);
    Gdiplus::SolidBrush brush(c);
    p2DGraphics->FillRectangle(&brush, (INT)uX, (INT)uY, (INT)uWidth,
        (INT)uHeight);
}

void Render::BeginTextNew(Texture *main, Texture *shadow) {
    return;
}

void Render::EndTextNew() {
    return;
}

void Render::DrawTextNew(int x, int y, int width, int h, float u1, float v1, float u2, float v2, int isshadow, uint16_t colour) {
    return;
}

void Render::DrawText(int uOutX, int uOutY, uint8_t *pFontPixels,
                      unsigned int uCharWidth, unsigned int uCharHeight,
                      uint8_t *pFontPalette, uint16_t uFaceColor,
                      uint16_t uShadowColor) {
    Image *fonttemp = Image::Create(uCharWidth, uCharHeight, IMAGE_FORMAT_A8R8G8B8);
    uint32_t *fontpix = (uint32_t*)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (uint y = 0; y < uCharHeight; ++y) {
        for (uint x = 0; x < uCharWidth; ++x) {
            if (*pFontPixels) {
                uint16_t color = uShadowColor;
                if (*pFontPixels != 1) {
                    color = uFaceColor;
                }
                fontpix[x + y * uCharWidth] = Color32(color);
            }
            ++pFontPixels;
        }
    }
    render->DrawTextureAlphaNew(uOutX / float(window->GetWidth()), uOutY / float(window->GetHeight()), fonttemp);
    fonttemp->Release();
}

void Render::DrawTextAlpha(int x, int y, uint8_t *font_pixels, int uCharWidth,
                           unsigned int uFontHeight, uint8_t *pPalette,
                           bool present_time_transparency) {
    Image *fonttemp = Image::Create(uCharWidth, uFontHeight, IMAGE_FORMAT_A8R8G8B8);
    uint32_t *fontpix = (uint32_t *)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (present_time_transparency) {
        for (unsigned int dy = 0; dy < uFontHeight; ++dy) {
            for (unsigned int dx = 0; dx < uCharWidth; ++dx) {
                uint16_t color = (*font_pixels)
                                     ? pPalette[*font_pixels]
                                     : teal_mask_16;
                fontpix[dx + dy * uCharWidth] = Color32(color);
                ++font_pixels;
            }
        }
    } else {
        for (unsigned int dy = 0; dy < uFontHeight; ++dy) {
            for (unsigned int dx = 0; dx < uCharWidth; ++dx) {
                if (*font_pixels) {
                    uint8_t index = *font_pixels;
                    uint8_t r = pPalette[index * 3 + 0];
                    uint8_t g = pPalette[index * 3 + 1];
                    uint8_t b = pPalette[index * 3 + 2];
                    fontpix[dx + dy * uCharWidth] = Color32(r, g, b);
                }
                ++font_pixels;
            }
        }
    }
    render->DrawTextureAlphaNew(x / float(window->GetWidth()), y / float(window->GetHeight()), fonttemp);
    fonttemp->Release();
}

void Render::DrawTransparentGreenShade(float u, float v, Image *pTexture) {
    DrawMasked(u, v, pTexture, 0, 0x07E0);
}

void Render::DrawTransparentRedShade(float u, float v, Image *a4) {
    DrawMasked(u, v, a4, 0, 0xF800);
}

inline uint32_t PixelDim(uint32_t pix, int dimming) {
    return Color32((((pix >> 16) & 0xFF) >> dimming),
        (((pix >> 8) & 0xFF) >> dimming),
        ((pix & 0xFF) >> dimming));
}

void Render::DrawMasked(float u, float v, Image *pTexture,
                        unsigned int color_dimming_level, uint16_t mask) {
    if (!pTexture) {
        return;
    }
    uint32_t width = pTexture->GetWidth();
    uint32_t *pixels = (uint32_t *)pTexture->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    Image *temp = Image::Create(width, pTexture->GetHeight(), IMAGE_FORMAT_A8R8G8B8);
    uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (unsigned int dy = 0; dy < pTexture->GetHeight(); ++dy) {
        for (unsigned int dx = 0; dx < width; ++dx) {
            if (*pixels & 0xFF000000)
                temppix[dx + dy * width] = PixelDim(*pixels, color_dimming_level) & Color32(mask);
            ++pixels;
        }
    }
    render->DrawTextureAlphaNew(u, v, temp);
    temp->Release();
}

void Render::TexturePixelRotateDraw(float u, float v, Image *img, int time) {
    if (img) {
        uint8_t *palpoint24 = (uint8_t *)img->GetPalette();
        int width = img->GetWidth();
        int height = img->GetHeight();
        Texture *temp = CreateTexture_Blank(width, height, IMAGE_FORMAT_A8R8G8B8);
        uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        uint8_t *texpix24 = (uint8_t *)img->GetPixels(IMAGE_FORMAT_R8G8B8);
        uint8_t thispix;
        int palindex;

        for (uint dy = 0; dy < height; ++dy) {
            for (int dx = 0; dx < width; ++dx) {
                thispix = *texpix24;
                if (thispix >= 0 && thispix <= 63) {
                    palindex = (time + thispix) % (2 * 63);
                    if (palindex >= 63)
                        palindex = (2 * 63) - palindex;
                    temppix[dx + dy * width] = Color32(palpoint24[palindex * 3], palpoint24[palindex * 3 + 1], palpoint24[palindex * 3 + 2]);
                }
                ++texpix24;
            }
        }

        // draw image
        render->DrawTextureAlphaNew(u, v, temp);
        temp->Release();
    }
}

void Render::BlendTextures(
    int x, int y, Image *imgin, Image *imgblend, int time, int start_opacity,
    int end_opacity) {  // thrown together as a crude estimate of the enchaintg
                        // effects

    // leaves gap where it shouldnt on dark pixels currently
    // doesnt use opacity params

    const uint32_t *pixelpoint;
    const uint32_t *pixelpointblend;

    if (imgin && imgblend) {  // 2 images to blend
        pixelpoint = (const uint32_t *)imgin->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        pixelpointblend =
            (const uint32_t *)imgblend->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        int Width = imgin->GetWidth();
        int Height = imgin->GetHeight();
        Image *temp = Image::Create(Width, Height, IMAGE_FORMAT_A8R8G8B8);
        uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        uint32_t c = *(pixelpointblend + 2700);  // guess at brightest pixel
        unsigned int bmax = (c & 0xFF);
        unsigned int gmax = ((c >> 8) & 0xFF);
        unsigned int rmax = ((c >> 16) & 0xFF);

        unsigned int bmin = bmax / 10;
        unsigned int gmin = gmax / 10;
        unsigned int rmin = rmax / 10;

        unsigned int bstep = (bmax - bmin) / 128;
        unsigned int gstep = (gmax - gmin) / 128;
        unsigned int rstep = (rmax - rmin) / 128;

        for (int ydraw = 0; ydraw < Height; ++ydraw) {
            for (int xdraw = 0; xdraw < Width; ++xdraw) {
                // should go blue -> black -> blue reverse
                // patchy -> solid -> patchy

                if (*pixelpoint) {  // check orig item not got blakc pixel
                    uint32_t nudge =
                        (xdraw % imgblend->GetWidth()) +
                        (ydraw % imgblend->GetHeight()) * imgblend->GetWidth();
                    uint32_t pixcol = *(pixelpointblend + nudge);

                    unsigned int bcur = (pixcol & 0xFF);
                    unsigned int gcur = ((pixcol >> 8) & 0xFF);
                    unsigned int rcur = ((pixcol >> 16) & 0xFF);

                    int steps = (time) % 128;

                    if ((time) % 256 >= 128) {  // step down
                        bcur += bstep * (128 - steps);
                        gcur += gstep * (128 - steps);
                        rcur += rstep * (128 - steps);
                    } else {  // step up
                        bcur += bstep * steps;
                        gcur += gstep * steps;
                        rcur += rstep * steps;
                    }

                    if (bcur > bmax) bcur = bmax;  // limit check
                    if (gcur > gmax) gcur = gmax;
                    if (rcur > rmax) rcur = rmax;
                    if (bcur < bmin) bcur = bmin;
                    if (gcur < gmin) gcur = gmin;
                    if (rcur < rmin) rcur = rmin;

                    temppix[xdraw + ydraw * Width] = Color32(rcur, gcur, bcur);
                }

                pixelpoint++;
            }

            pixelpoint += imgin->GetWidth() - Width;
        }
        // draw image
        render->DrawTextureAlphaNew(x / float(window->GetWidth()), y / float(window->GetHeight()) , temp);
        temp->Release();
    }
}

void Render::DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait, int Y_Offset) {
    int dst_x = rc.x + 64 + Portrait->hw_sprites[0]->uAreaX - Portrait->hw_sprites[0]->uBufferWidth / 2;
    int dst_y = rc.y + Y_Offset + Portrait->hw_sprites[0]->uAreaY;
    uint dst_z = dst_x + Portrait->hw_sprites[0]->uAreaWidth;
    uint dst_w = dst_y + Portrait->hw_sprites[0]->uAreaHeight;

    uint Clipped_X = 0;
    uint Clipped_Y = 0;

    if (dst_x < rc.x) {
        Clipped_X = rc.x - dst_x;
        dst_x = rc.x;
    }

    if (dst_y < rc.y) {
        Clipped_Y = rc.y - dst_y;
        dst_y = rc.y;
    }

    if (dst_z > rc.x + rc.w)
        dst_z = rc.x + rc.w;
    if (dst_w > rc.y + rc.h)
        dst_w = rc.y + rc.h;

    Image *temp = Image::Create(128, 128, IMAGE_FORMAT_R5G6B5);
    uint16_t *temppix = (uint16_t *)temp->GetPixels(IMAGE_FORMAT_R5G6B5);

    int width = Portrait->hw_sprites[0]->texture->GetWidth();
    int height = Portrait->hw_sprites[0]->texture->GetHeight();

    ushort* src = (uint16_t *)Portrait->hw_sprites[0]->texture->GetPixels(IMAGE_FORMAT_A1R5G5B5);
    int num_top_scanlines_above_frame_y = Clipped_Y - dst_y;

    for (uint y = dst_y; y < dst_w; ++y) {
        uint src_y = num_top_scanlines_above_frame_y + y;

        for (uint x = dst_x; x < dst_z; ++x) {
            uint src_x = Clipped_X - dst_x + x;  // num scanlines left to frame_x  + current x

            uint idx =
                height * src_y / Portrait->hw_sprites[0]->uAreaHeight * width +
                width * src_x / Portrait->hw_sprites[0]->uAreaWidth;

            uint a = 2 * (src[idx] & 0xFFE0);
            uint b = src[idx] & 0x1F;

            temppix[(x - dst_x) + 128 * (y - dst_y)] = (b | a);
        }
    }

    render->SetUIClipRect(rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
    render->DrawTextureAlphaNew(dst_x / float(window->GetWidth()), dst_y / float(window->GetHeight()), temp);
    temp->Release();
    render->ResetUIClipRect();
}

void Render::DrawTextureAlphaNew(float u, float v, Image *image) {
    Gdiplus::Bitmap *bitmap = BitmapWithImage(image);
    if (!bitmap) {
        return;
    }

    int uX = u * window->GetWidth();
    int uY = v * window->GetHeight();
    p2DGraphics->DrawImage(bitmap, uX, uY);

    delete bitmap;
}

void Render::ZDrawTextureAlpha(float u, float v, Image *img, int zVal) {
    if (!img) return;

    int winwidth = window->GetWidth();
    int uOutX = u * winwidth;
    int uOutY = v * window->GetHeight();
    unsigned int imgheight = img->GetHeight();
    unsigned int imgwidth = img->GetWidth();
    auto pixels = (uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (int xs = 0; xs < imgwidth; xs++) {
        for (int ys = 0; ys < imgheight; ys++) {
            if (pixels[xs + imgwidth * ys] & 0xFF000000) {
                this->pActiveZBuffer[uOutX + xs + winwidth * (uOutY + ys)] = zVal;
            }
        }
    }
}


void Render::DoRenderBillboards_D3D() {
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i) {
        if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend) {
            SetBillboardBlendOptions(pBillboardRenderListD3D[i].opacity);
        }

        if (pBillboardRenderListD3D[i].texture) {
            pRenderD3D->pDevice->SetTexture(
                0, ((TextureD3D *)pBillboardRenderListD3D[i].texture)
                ->GetDirect3DTexture());
        } else {
            // auto hwsplat04 = assets->GetBitmap("hwsplat04");
            // ErrD3D(pRenderD3D->pDevice->SetTexture(0, ((TextureD3D *)hwsplat04)->GetDirect3DTexture()));
            // testing
            ErrD3D(pRenderD3D->pDevice->SetTexture(0, 0));
        }

        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            pBillboardRenderListD3D[i].pQuads,
            pBillboardRenderListD3D[i].uNumVertices,
            D3DDP_DONOTLIGHT /*| D3DDP_DONOTUPDATEEXTENTS*/));
        drawcalls++;
    }

    if (engine->IsFog()) {
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

void Render::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1) {
    switch (a1) {
        case RenderBillboardD3D::Transparent: {
            if (engine->IsFog()) {
                engine->SetFog(false);
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, GetLevelFogColor() & 0xFFFFFF));
                ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0));
            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));

            break;
        }

        case RenderBillboardD3D::Opaque_1:
        case RenderBillboardD3D::Opaque_2:
        case RenderBillboardD3D::Opaque_3: {
            if (engine->IsSpecular_FogIsOn()) {
                if (!engine->IsFog()) {
                    engine->SetFog(true);
                    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
                }
            }

            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
            ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));

            break;
        }

        default: {
            log->Warning(
                "SetBillboardBlendOptions: invalid opacity type (%u)", a1);
            assert(false);  // fireball and impolision can crash here - maybe clipping related
            break;
        }
    }
}

void Render::MaskGameViewport() {
    FillRectFast(
        pViewport->uViewportTL_X, pViewport->uViewportTL_Y,
        pViewport->uViewportBR_X - pViewport->uViewportTL_X,
        pViewport->uViewportBR_Y - pViewport->uViewportTL_Y,
        teal_mask_16
    );
}

void Render::DrawBuildingsD3D() {
    int farclip;  // [sp+2Ch] [bp-2Ch]@10
    int nearclip;  // [sp+30h] [bp-28h]@34
    // int v51;  // [sp+34h] [bp-24h]@35
    // int v52;  // [sp+38h] [bp-20h]@36
    int v53;  // [sp+3Ch] [bp-1Ch]@8

    for (BSPModel &model : pOutdoor->pBModels) {
        bool reachable_unused;
        if (!IsBModelVisible(&model, 256, &reachable_unused)) {
            continue;
        }
        model.field_40 |= 1;
        if (model.pFaces.empty()) {
            continue;
        }

        for (ODMFace &face : model.pFaces) {
            if (face.Invisible()) {
                continue;
            }

            v53 = 0;
            auto poly = &array_77EC08[pODMRenderParams->uNumPolygons];

            poly->flags = 0;
            poly->field_32 = 0;
            poly->texture = face.GetTexture();

            if (face.uAttributes & FACE_IsFluid) poly->flags |= 2;
            if (face.uAttributes & FACE_INDOOR_SKY) poly->flags |= 0x400;

            if (face.uAttributes & FACE_FlowDown)
                poly->flags |= 0x400;
            else if (face.uAttributes & FACE_FlowUp)
                poly->flags |= 0x800;

            if (face.uAttributes & FACE_FlowRight)
                poly->flags |= 0x2000;
            else if (face.uAttributes & FACE_FlowLeft)
                poly->flags |= 0x1000;

            poly->sTextureDeltaU = face.sTextureDeltaU;
            poly->sTextureDeltaV = face.sTextureDeltaV;

            unsigned int flow_anim_timer = platform->TickCount() >> 4;
            unsigned int flow_u_mod = poly->texture->GetWidth() - 1;
            unsigned int flow_v_mod = poly->texture->GetHeight() - 1;

            if (face.pFacePlaneOLD.vNormal.z &&
                abs(face.pFacePlaneOLD.vNormal.z) >= 59082) {
                if (poly->flags & 0x400)
                    poly->sTextureDeltaV += flow_anim_timer & flow_v_mod;
                if (poly->flags & 0x800)
                    poly->sTextureDeltaV -= flow_anim_timer & flow_v_mod;
            } else {
                if (poly->flags & 0x400)
                    poly->sTextureDeltaV -= flow_anim_timer & flow_v_mod;
                if (poly->flags & 0x800)
                    poly->sTextureDeltaV += flow_anim_timer & flow_v_mod;
            }

            if (poly->flags & 0x1000)
                poly->sTextureDeltaU -= flow_anim_timer & flow_u_mod;
            else if (poly->flags & 0x2000)
                poly->sTextureDeltaU += flow_anim_timer & flow_u_mod;

            nearclip = 0;
            farclip = 0;

            for (uint vertex_id = 1; vertex_id <= face.uNumVertices;
                 vertex_id++) {
                array_73D150[vertex_id - 1].vWorldPosition.x =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].x;
                array_73D150[vertex_id - 1].vWorldPosition.y =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].y;
                array_73D150[vertex_id - 1].vWorldPosition.z =
                    model.pVertices[face.pVertexIDs[vertex_id - 1]].z;
                array_73D150[vertex_id - 1].u =
                    (poly->sTextureDeltaU +
                     (int16_t)face.pTextureUIDs[vertex_id - 1]) *
                    (1.0 / (double)poly->texture->GetWidth());
                array_73D150[vertex_id - 1].v =
                    (poly->sTextureDeltaV +
                     (int16_t)face.pTextureVIDs[vertex_id - 1]) *
                    (1.0 / (double)poly->texture->GetHeight());
            }
            for (uint i = 1; i <= face.uNumVertices; i++) {
                if (model.pVertices[face.pVertexIDs[0]].z ==
                    array_73D150[i - 1].vWorldPosition.z)
                    ++v53;
                pCamera3D->ViewTransform(&array_73D150[i - 1], 1);
               // if (array_73D150[i - 1].vWorldViewPosition.x <
               //         pCamera3D->GetNearClip() ||
                //    array_73D150[i - 1].vWorldViewPosition.x >
                //        pCamera3D->GetFarClip()) {
                //    if (array_73D150[i - 1].vWorldViewPosition.x >=
                 //       pCamera3D->GetNearClip())
                //        farclip = 1;
                //    else
                //        nearclip = 1;
                //} else {
                    pCamera3D->Project(&array_73D150[i - 1], 1, 0);
                //}
            }

            if (v53 == face.uNumVertices) poly->field_32 |= 1;
            poly->pODMFace = &face;
            poly->uNumVertices = face.uNumVertices;
            poly->field_59 = 5;

            float f = face.pFacePlane.vNormal.x * pOutdoor->vSunlight.x + face.pFacePlane.vNormal.y * pOutdoor->vSunlight.y + face.pFacePlane.vNormal.z * pOutdoor->vSunlight.z;
            poly->dimming_level = 20 - std::round(20 * f);

            if (poly->dimming_level < 0) poly->dimming_level = 0;
            if (poly->dimming_level > 31) poly->dimming_level = 31;
            if (pODMRenderParams->uNumPolygons >= 1999 + 5000) return;

            if (pCamera3D->is_face_faced_to_cameraODM(&face, &array_73D150[0])) {
                face.bVisible = 1;
                poly->uBModelFaceID = face.index;
                poly->uBModelID = model.index;
                poly->pid = PID(OBJECT_Face, (face.index | (model.index << 6)));
                for (int vertex_id = 0; vertex_id < face.uNumVertices;
                     ++vertex_id) {
                    VertexRenderList[vertex_id] = array_73D150[vertex_id];
                    VertexRenderList[vertex_id]._rhw =
                        1.0 / (array_73D150[vertex_id].vWorldViewPosition.x +
                               0.0000001);
                }
                static stru154 static_RenderBuildingsD3D_stru_73C834;

                lightmap_builder->ApplyLights_OutdoorFace(&face);
                decal_builder->ApplyBloodSplat_OutdoorFace(&face);
                lightmap_builder->StationaryLightsCount = 0;
                int v31 = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                    v31 = nearclip ? 3 : farclip != 0 ? 5 : 0;

                   static_RenderBuildingsD3D_stru_73C834.GetFacePlaneAndClassify(&face, model.pVertices);
                    if (decal_builder->uNumSplatsThisFace > 0) {
                        decal_builder->BuildAndApplyDecals(
                            31 - poly->dimming_level, LocationBuildings,
                            &static_RenderBuildingsD3D_stru_73C834,
                            face.uNumVertices, VertexRenderList, (char)v31,
                            -1);
                    }
                }
                if (Lights.uNumLightsApplied > 0)
                    // if (face.uAttributes & FACE_OUTLINED)
                    lightmap_builder->ApplyLights(
                        &Lights, &static_RenderBuildingsD3D_stru_73C834,
                        poly->uNumVertices, VertexRenderList, 0, (char)v31);

                //if (nearclip) {
                //    pCamera3D->CullByNearClip(&VertexRenderList[0], &poly->uNumVertices);
                //    pCamera3D->Project(&VertexRenderList[0], poly->uNumVertices);
                //}

                //if (farclip) {
                //    pCamera3D->CullByFarClip(&VertexRenderList[0], &poly->uNumVertices);
                //    pCamera3D->Project(&VertexRenderList[0], poly->uNumVertices);
                //}

                if (poly->uNumVertices) {
                    if (poly->IsWater()) {
                        if (poly->IsWaterAnimDisabled())
                            poly->texture = render->hd_water_tile_anim[0];
                        else
                            poly->texture =
                                render->hd_water_tile_anim
                                    [render->hd_water_current_frame];
                    }

                    render->DrawPolygon(poly);
                }
            }
        }
    }

    pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);  // buildings
    BatchTriDraw();
}

unsigned short *Render::MakeScreenshot16(int width, int height) {
    uint16_t *for_pixels;  // ebx@1
    DDSURFACEDESC2 Dst;    // [sp+4h] [bp-A0h]@6

    int interval_x = game_viewport_width / (double)width;
    int interval_y = game_viewport_height / (double)height;

    uint16_t *pPixels = (uint16_t *)malloc(sizeof(uint16_t) * height * width);
    memset(pPixels, 0, sizeof(uint16_t) * height * width);

    for_pixels = pPixels;

    BeginSceneD3D();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        pOutdoor->Draw();
    }
    DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    memset(&Dst, 0, sizeof(Dst));
    Dst.dwSize = sizeof(Dst);

    if (LockSurface_DDraw4(pBackBuffer4, &Dst, DDLOCK_WAIT)) {
        if (uCurrentlyLoadedLevelType == LEVEL_null) {
            memset(&for_pixels, 0, sizeof(for_pixels));
        } else {
            for (uint y = 0; y < (unsigned int)height; ++y) {
                for (uint x = 0; x < (unsigned int)width; ++x) {
                    if (Dst.ddpfPixelFormat.dwRGBBitCount == 32) {
                        uint32_t *p =
                            (uint32_t *)Dst.lpSurface +
                            (int)(x * interval_x + 8.0) +
                            (int)(y * interval_y + 8.0) * (Dst.lPitch >> 2);
                        *for_pixels = Color16((*p >> 16) & 255, (*p >> 8) & 255,
                                              *p & 255);
                    } else if (Dst.ddpfPixelFormat.dwRGBBitCount == 16) {
                        uint16_t *p =
                            (uint16_t*)Dst.lpSurface +
                            (int)(x * interval_x + 8.0) + y * Dst.lPitch;
                        *for_pixels = *p;
                    } else {
                        assert(false);
                    }
                    ++for_pixels;
                }
            }
        }
        ErrD3D(pBackBuffer4->Unlock(NULL));
    }
    return pPixels;
}

Image *Render::TakeScreenshot(unsigned int width, unsigned int height) {
    auto pixels = MakeScreenshot16(width, height);
    Image *image = Image::Create(width, height, IMAGE_FORMAT_R5G6B5, pixels);
    free(pixels);
    return image;
}

void Render::SaveScreenshot(const std::string &filename, unsigned int width,
                            unsigned int height) {
    auto pixels = MakeScreenshot16(width, height);
    SavePCXImage16(filename, pixels, width, height);
    free(pixels);
}

void Render::PackScreenshot(unsigned int width, unsigned int height, void *data,
                            unsigned int data_size,
                            unsigned int *out_screenshot_size) {
    auto pixels = MakeScreenshot16(150, 112);
    PCX::Encode16(pixels, width, height, data, data_size, out_screenshot_size);
    free(pixels);
}

int Render::GetActorsInViewport(int pDepth) {
    // TODO: merge this function with RenderOpenGL::GetActorsInViewport

    int
        v3;  // eax@2 применяется в закле Жар печи для подсчёта кол-ва монстров
             // видимых группе и заполнения массива id видимых монстров
    unsigned int v5;   // eax@2
    unsigned int v6;   // eax@4
    unsigned int v12;  // [sp+10h] [bp-14h]@1
    int mon_num;       // [sp+1Ch] [bp-8h]@1
    unsigned int a1a;  // [sp+20h] [bp-4h]@1

    mon_num = 0;
    v12 = GetBillboardDrawListSize();
    if ((signed int)GetBillboardDrawListSize() > 0) {
        for (a1a = 0; (signed int)a1a < (signed int)v12; ++a1a) {
            v3 = GetParentBillboardID(a1a);
            if(v3 == -1)
                continue;

            v5 = (uint16_t)pBillboardRenderList[v3].object_pid;
            if (PID_TYPE(v5) == OBJECT_Actor) {
                if (pBillboardRenderList[v3].screen_space_z <= pDepth) {
                    v6 = PID_ID(v5);
                    if (pActors[v6].uAIState != Dead &&
                        pActors[v6].uAIState != Dying &&
                        pActors[v6].uAIState != Removed &&
                        pActors[v6].uAIState != Disabled &&
                        pActors[v6].uAIState != Summoned) {
                        if (vis->DoesRayIntersectBillboard(pDepth, a1a)) {
                            if (mon_num < 100) {
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

void Render::BeginLightmaps() {
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));

    if (engine->IsSpecular_FogIsOn())
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));

    // ErrD3D(pRenderD3D->pDevice->SetTexture(0,
    // pCamera3D->LoadTextureAndGetHardwarePtr("effpar03")));
    static Texture* effpar03 = assets->GetBitmap("effpar03");

    ErrD3D(pRenderD3D->pDevice->SetTexture(
        0, ((TextureD3D *)effpar03)->GetDirect3DTexture()));

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ONE));
}

void Render::EndLightmaps() {
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));

    if (engine->IsSpecular_FogIsOn()) {
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, uFogColor));
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 0));
    }
}

void Render::BeginLightmaps2() {
    if (engine->IsSpecular_FogIsOn())
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));

    // ErrD3D(pRenderD3D->pDevice->SetTexture(0,
    // pCamera3D->LoadTextureAndGetHardwarePtr("effpar03")));
    auto effpar03 = assets->GetBitmap("effpar03");
    ErrD3D(pRenderD3D->pDevice->SetTexture(
        0, ((TextureD3D *)effpar03)->GetDirect3DTexture()));

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
}

void Render::EndLightmaps2() {
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));

    if (engine->IsSpecular_FogIsOn())
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
}

void Render::Do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin,
                                    signed int sDiffuseBegin,
                                    const RenderVertexD3D3 *pLineEnd,
                                    signed int sDiffuseEnd, float z_stuff) {
    RenderVertexD3D3 vertices[2];  // [sp+8h] [bp-40h]@2
    vertices[0] = *pLineBegin;
    vertices[1] = *pLineEnd;

    vertices[0].pos.z = 0.001 - z_stuff;
    vertices[1].pos.z = 0.001 - z_stuff;

    vertices[0].diffuse = sDiffuseBegin;
    vertices[1].diffuse = sDiffuseEnd;

    ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_LINELIST, 452, vertices, 2,
                                              D3DDP_DONOTLIGHT));
    drawcalls++;
}

void Render::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {
    ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_LINELIST,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        (void *)vertices, num_vertices, D3DDP_DONOTLIGHT));
    drawcalls++;
}

//void Render::DrawFansTransparent(const RenderVertexD3D3 *vertices,
//                                 unsigned int num_vertices) {
//    // ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
//    // false));
//    // ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,
//    // false));
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
//                                               TRUE));
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
//                                               D3DBLEND_SRCALPHA));
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
//                                               D3DBLEND_INVSRCALPHA));
//
//    ErrD3D(pRenderD3D->pDevice->SetTexture(0, nullptr));
//    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
//        D3DPT_TRIANGLEFAN,
//        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
//        (void *)vertices, num_vertices, 28));
//    drawcalls++;
//
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
//                                               D3DBLEND_ONE));
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
//                                               D3DBLEND_ZERO));
//    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
//                                               FALSE));
//    // ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,
//    // TRUE));
//    // ErrD3D(render->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
//    // TRUE));
//}

void Render::BeginDecals() {
    // code chunk from 0049C304
    if (engine->IsSpecular_FogIsOn())
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP));

    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE));

    // ErrD3D(pRenderD3D->pDevice->SetTexture(0,
    // pCamera3D->LoadTextureAndGetHardwarePtr("hwsplat04")));
    auto hwsplat04 = assets->GetBitmap("hwsplat04");
    ErrD3D(pRenderD3D->pDevice->SetTexture(0, ((TextureD3D *)hwsplat04)->GetDirect3DTexture()));
}

void Render::EndDecals() {
    // code chunk from 0049C304
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO));

    if (engine->IsSpecular_FogIsOn())
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE));
}

void Render::DrawDecal(Decal *pDecal, float z_bias) {
    // decals need recalculating here now
    pCamera3D->ViewTransform(pDecal->pVertices, (unsigned int)pDecal->uNumVertices);
    pCamera3D->Project(pDecal->pVertices, pDecal->uNumVertices, 0);

    int dwFlags;                        // [sp+Ch] [bp-864h]@15
    RenderVertexD3D3 pVerticesD3D[64];  // [sp+20h] [bp-850h]@6

    if (pDecal->uNumVertices < 3) {
        log->Warning("Decal has < 3 vertices");
        return;
    }

    float color_mult = pDecal->Fade_by_time();
    if (color_mult == 0.0f) return;

    for (uint i = 0; i < (unsigned int)pDecal->uNumVertices; ++i) {
        uint uTint =
            Render::GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[i].vWorldViewPosition.x, 0, nullptr);

        uint uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF,
             uTintB = uTint & 0xFF;

        uint uDecalColorMultR = (pDecal->uColorMultiplier >> 16) & 0xFF,
             uDecalColorMultG = (pDecal->uColorMultiplier >> 8) & 0xFF,
             uDecalColorMultB = pDecal->uColorMultiplier & 0xFF;

        uint uFinalR =
                 floorf(uTintR / 255.0f * color_mult * uDecalColorMultR + 0.0f),
             uFinalG =
                 floorf(uTintG / 255.0f * color_mult * uDecalColorMultG + 0.0f),
             uFinalB =
                 floorf(uTintB / 255.0f * color_mult * uDecalColorMultB + 0.0f);

        // temp - make yellow for easier spotting
        // uFinalR = 255;
        // uFinalG = 255;

        float v15;
        if (fabs(z_bias) < 1e-5) {
            v15 = 1.0 -
                1.0 / ((1.0f / pCamera3D->GetFarClip()) *
                    pDecal->pVertices[i].vWorldViewPosition.x * 1000.0);
        } else {
            v15 = 1.0 -
                  1.0 / ((1.0f / pCamera3D->GetFarClip()) *
                         pDecal->pVertices[i].vWorldViewPosition.x * 1000.0) -
                  z_bias;
            if (v15 < 0.000099999997f) v15 = 0.000099999997f;
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

    dwFlags = D3DDP_DONOTLIGHT;

    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        pVerticesD3D, pDecal->uNumVertices, dwFlags));
    drawcalls++;
}

void Render::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices,
                                    Texture *texture) {
    auto gapi_texture = ((TextureD3D *)texture)->GetDirect3DTexture();
    ErrD3D(
        pRenderD3D->pDevice->SetTexture(0, (IDirect3DTexture2 *)gapi_texture));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               TRUE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE,
                                               FALSE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,
                                               D3DCMP_ALWAYS));
    ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        (void *)vertices, 4, 28));
    drawcalls++;
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                               D3DBLEND_ONE));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                               D3DBLEND_ZERO));
    ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                                               FALSE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE));
    ErrD3D(
        pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS));
}

unsigned int _452442_color_cvt(uint16_t a1, uint16_t a2, int a3,
                               int a4) {
    int v4 {};                // ebx@0
    int16_t v5;            // ST14_2@1
    int16_t v6;            // dx@1
    int v7 {};                // ecx@1
    int16_t v8;            // ST10_2@1
    int v9;                // edi@1
    uint16_t v10 = 0;  // dh@1@1
    int v11;               // ebx@1
    int v12;               // ebx@1
    int16_t a3a;           // [sp+1Ch] [bp+8h]@1

    v5 = a2 >> 2;
    v6 = (uint16_t)a4 >> 2;
    v8 = a1 >> 2;
    a3a = (uint16_t)a3 >> 2;
    HEXRAYS_LOWORD(v7) = a3a;
    v9 = v7;
    HEXRAYS_LOWORD(v4) = ((uint16_t)a4 >> 2) & 0xE0;
    HEXRAYS_LOWORD(v7) = a3a & 0xE0;
    HEXRAYS_LOWORD(v9) = v9 & 0x1C00;
    v11 = v7 + v4;
    HEXRAYS_LOWORD(v7) = v5 & 0xE0;
    v12 = v7 + v11;
    HEXRAYS_LOWORD(v7) = v8 & 0xE0;
    __debugbreak();  // warning C4700: uninitialized local variable 'v10' used
    return ((int)PID_TYPE(v8) + (int)PID_TYPE(v5) + (int)PID_TYPE(a3a) + (int)PID_TYPE(v6)) |
           (v7 + v12) |
           ((v8 & 0x1C00) + (v5 & 0x1C00) + v9 +
            (((int32_t)v10 << 16) | (((uint16_t)a4 >> 2) & 0x1C00)));
}

// int Polygon::_479295() {
//    int v3;           // ecx@4
//    int v4;           // eax@4
//    int v5;           // edx@4
//    Vec3i thisa;  // [sp+Ch] [bp-10h]@8
//    int v11;          // [sp+18h] [bp-4h]@4
//
//    if (!this->pODMFace->pFacePlane.vNormal.z) {
//        v3 = this->pODMFace->pFacePlane.vNormal.x;
//        v4 = -this->pODMFace->pFacePlane.vNormal.y;
//        v5 = 0;
//        v11 = 65536;
//    } else if ((this->pODMFace->pFacePlane.vNormal.x ||
//                this->pODMFace->pFacePlane.vNormal.y) &&
//               abs(this->pODMFace->pFacePlane.vNormal.z) < 59082) {
//        thisa.x = -this->pODMFace->pFacePlane.vNormal.y;
//        thisa.y = this->pODMFace->pFacePlane.vNormal.x;
//        thisa.z = 0;
//        thisa.Normalize_float();
//        v4 = thisa.x;
//        v3 = thisa.y;
//        v5 = 0;
//        v11 = 65536;
//    } else {
//        v3 = 0;
//        v4 = 65536;
//        v11 = 0;
//        v5 = -65536;
//    }
//    sTextureDeltaU = this->pODMFace->sTextureDeltaU;
//    sTextureDeltaV = this->pODMFace->sTextureDeltaV;
//    ptr_38->CalcSkyFrustumVec(v4, v3, 0, 0, v5, v11);
//    return 1;
//}

void Render::DrawOutdoorSkyD3D() {
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  horizon_height_offset = ((double)(pCamera3D->ViewPlaneDist_X * pCamera3D->vCameraPos.z)
                                    / ((double)pCamera3D->ViewPlaneDist_X + pCamera3D->GetFarClip())
                                    + (double)(pViewport->uScreenCenterY));

    float depth_to_far_clip = cos((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = sin((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();

    float bot_y_proj = ((double)(pViewport->uScreenCenterY) -
        (double)pCamera3D->ViewPlaneDist_X /
        (depth_to_far_clip + 0.0000001) *
        (height_to_far_clip - (double)pCamera3D->vCameraPos.z));

    struct Polygon pSkyPolygon;
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.ptr_38 = &SkyBillboard;


    // if ( pParty->uCurrentHour > 20 || pParty->uCurrentHour < 5 )
    // pSkyPolygon.uTileBitmapID = pOutdoor->New_SKY_NIGHT_ID;
    // else
    // pSkyPolygon.uTileBitmapID = pOutdoor->sSky_TextureID;//179(original 166)
    // pSkyPolygon.pTexture = (Texture_MM7 *)(pSkyPolygon.uTileBitmapID != -1 ?
    // (int)&pBitmaps_LOD->pTextures[pSkyPolygon.uTileBitmapID] : 0);

    pSkyPolygon.texture = pOutdoor->sky_texture;

    if (pSkyPolygon.texture) {
        pSkyPolygon.dimming_level = 0;
        pSkyPolygon.uNumVertices = 4;

        // centering(центруем)-----------------------------------------------------------------
        // plane of sky polygon rotation vector - pitch rotation around y
        float v18x = -sin((-pCamera3D->sRotationY + 16) * rot_to_rads);
        float v18y = 0;
        float v18z = -cos((pCamera3D->sRotationY + 16) * rot_to_rads);

        // sky wiew position(положение неба на
        // экране)------------------------------------------
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
        VertexRenderList[0].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;  // 8
        VertexRenderList[0].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;  // 8

        VertexRenderList[1].vWorldViewProjX = (double)(signed int)pViewport->uViewportTL_X;   // 8
        VertexRenderList[1].vWorldViewProjY = (double)bot_y_proj + 1;  // 247

        VertexRenderList[2].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;   // 468
        VertexRenderList[2].vWorldViewProjY = (double)bot_y_proj + 1;  // 247

        VertexRenderList[3].vWorldViewProjX = (double)(signed int)pViewport->uViewportBR_X;  // 468
        VertexRenderList[3].vWorldViewProjY = (double)(signed int)pViewport->uViewportTL_Y;  // 8

        float widthperpixel = 1 / pCamera3D->ViewPlaneDist_X;

        for (uint i = 0; i < pSkyPolygon.uNumVertices; ++i) {
            // outbound screen X dist
            float x_dist = widthperpixel * (pViewport->uScreenCenterX - VertexRenderList[i].vWorldViewProjX);
            // outbound screen y dist
            float y_dist = widthperpixel * (horizon_height_offset - VertexRenderList[i].vWorldViewProjY);

            // rotate vectors to cam facing
            float skyfinalleft = (pSkyPolygon.ptr_38->CamVecLeft_X * x_dist) + (pSkyPolygon.ptr_38->CamVecLeft_Z * y_dist) + pSkyPolygon.ptr_38->CamVecLeft_Y;
            float skyfinalfront = (pSkyPolygon.ptr_38->CamVecFront_X * x_dist) + (pSkyPolygon.ptr_38->CamVecFront_Z * y_dist) + pSkyPolygon.ptr_38->CamVecFront_Y;

            // pitch rotate sky to get top
            float top_y_proj = v18x + v18y + v18z * y_dist;
            if (top_y_proj > 0.0f) top_y_proj = -0.0000001f;

            float worldviewdepth = -64.0 / top_y_proj;
            if (worldviewdepth < 0) worldviewdepth = pCamera3D->GetFarClip();

            // offset tex coords
            float texoffset_U = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalleft * worldviewdepth));
            VertexRenderList[i].u = texoffset_U / ((float)pSkyPolygon.texture->GetWidth());
            float texoffset_V = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0)  + ((skyfinalfront * worldviewdepth));
            VertexRenderList[i].v = texoffset_V / ((float)pSkyPolygon.texture->GetHeight());

            VertexRenderList[i].vWorldViewPosition.x = pCamera3D->GetFarClip();

            // this basically acts as texture perspective correction
            VertexRenderList[i]._rhw = 1.0 / (double)(worldviewdepth);
        }

        DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}

void Render::DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID) {
    // for floor and wall(for example Celeste)-------------------
    BLVFace *pFace = &pIndoor->pFaces[uFaceID];
    if (pFace->uPolygonType == POLYGON_InBetweenFloorAndWall || pFace->uPolygonType == POLYGON_Floor) {
        int v69 = (platform->TickCount() / 32.0f) - pCamera3D->vCameraPos.x;
        int v55 = (platform->TickCount() / 32.0f) + pCamera3D->vCameraPos.y;
        for (uint i = 0; i < uNumVertices; ++i) {
            array_507D30[i].u = (v69 + array_507D30[i].u) * 0.25f;
            array_507D30[i].v = (v55 + array_507D30[i].v) * 0.25f;
        }
        render->DrawIndoorPolygon(uNumVertices, pFace, PID(OBJECT_Face, uFaceID), -1, 0);
        return;
    }
    //---------------------------------------

    if ((signed int)uNumVertices <= 0) return;

    struct Polygon pSkyPolygon;
    pSkyPolygon.texture = nullptr;
    pSkyPolygon.texture = pFace->GetTexture();
    if (!pSkyPolygon.texture) return;

    pSkyPolygon.ptr_38 = &SkyBillboard;
    pSkyPolygon.dimming_level = 0;
    pSkyPolygon.uNumVertices = uNumVertices;

    SkyBillboard.CalcSkyFrustumVec(1, 0, 0, 0, 1, 0);

    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  blv_horizon_height_offset = ((double)(pCamera3D->ViewPlaneDist_X * pCamera3D->vCameraPos.z)
        / ((double)pCamera3D->ViewPlaneDist_X + pCamera3D->GetFarClip())
        + (double)(pBLVRenderParams->uViewportCenterY));

    double cam_y_rot_rad = (double)pCamera3D->sRotationY * rot_to_rads;

    float depth_to_far_clip = cos((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();
    float height_to_far_clip = sin((double)pCamera3D->sRotationY * rot_to_rads) * pCamera3D->GetFarClip();

    float blv_bottom_y_proj = ((double)(pBLVRenderParams->uViewportCenterY) -
        (double)pCamera3D->ViewPlaneDist_X /
        (depth_to_far_clip + 0.0000001) *
        (height_to_far_clip - (double)pCamera3D->vCameraPos.z));

    // rotation vec for sky plane - pitch
    float v_18x = -sin((-pCamera3D->sRotationY + 16) * rot_to_rads);
    float v_18y = 0.0f;
    float v_18z = -cos((pCamera3D->sRotationY + 16) * rot_to_rads);

    float inv_viewplanedist = 1.0f / pCamera3D->ViewPlaneDist_X;

    int _507D30_idx = 0;
    for (_507D30_idx; _507D30_idx < pSkyPolygon.uNumVertices; _507D30_idx++) {
        // outbound screen x dist
        float x_dist = inv_viewplanedist * (pBLVRenderParams->uViewportCenterX - array_507D30[_507D30_idx].vWorldViewProjX);
        // outbound screen y dist
        float y_dist = inv_viewplanedist * (blv_horizon_height_offset - array_507D30[_507D30_idx].vWorldViewProjY);

        // rotate vectors to cam facing
        float skyfinalleft = (pSkyPolygon.ptr_38->CamVecLeft_X * x_dist) + (pSkyPolygon.ptr_38->CamVecLeft_Z * y_dist) + pSkyPolygon.ptr_38->CamVecLeft_Y;
        float skyfinalfront = (pSkyPolygon.ptr_38->CamVecFront_X * x_dist) + (pSkyPolygon.ptr_38->CamVecFront_Z * y_dist) + pSkyPolygon.ptr_38->CamVecFront_Y;

        // pitch rotate sky to get top projection
        float newX = v_18x + v_18y + (v_18z * y_dist);
        float worldviewdepth = -512.0f / newX;

        // offset tex coords
        float texoffset_U = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalleft * worldviewdepth) / 16.0f);
        array_507D30[_507D30_idx].u = texoffset_U / ((float)pSkyPolygon.texture->GetWidth());
        float texoffset_V = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalfront * worldviewdepth) / 16.0f);
        array_507D30[_507D30_idx].v = texoffset_V / ((float)pSkyPolygon.texture->GetHeight());

        // this basically acts as texture perspective correction
        array_507D30[_507D30_idx]._rhw = 1.0f / (double)worldviewdepth;
    }

    // no clipped polygon so draw and return??
    if (_507D30_idx >= pSkyPolygon.uNumVertices) {
        DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon);
        return;
    }






    logger->Info("past normal section");
    __debugbreak();
    // please provide save game / details if you get here
    // TODO(pskelton): below looks like some vert clipping but dont think its ever gets here now - delete below after testing

        // copy last to first
    memcpy(&array_507D30[uNumVertices], array_507D30, sizeof(array_507D30[uNumVertices]));

    int texture_height = pSkyPolygon.texture->GetHeight() - 1;
    int texture_width = pSkyPolygon.texture->GetWidth() - 1;
    float tex_u_offset = 224 * pMiscTimer->uTotalGameTimeElapsed & texture_height;
    float tex_v_offset = 224 * pMiscTimer->uTotalGameTimeElapsed & texture_width;
    float one_over_texheight = 1.0 / (double)pSkyPolygon.texture->GetHeight();
    float one_over_texwidth = 1.0 / (double)pSkyPolygon.texture->GetWidth();
    double y_proj;
    double v28;
    double v33;                  // st6@23
    const void *v35;             // ecx@31
    int v36;                     // eax@31
    const void *v37;             // edi@31
    int64_t v38;          // qax@31
    int v39;                     // ecx@31
    int v40;                     // ebx@33
    int v41;                     // eax@36
    int64_t v42 {};          // qtt@39
    int v43;                     // eax@39
    double v48;                  // st7@41
    double v51;                  // st7@46
    unsigned int v65;            // [sp+128h] [bp-4Ch]@1
    int64_t v69 {};                 // [sp+13Ch] [bp-38h]@3
    int X {};                       // [sp+148h] [bp-2Ch]@9
    float v73 {};                   // [sp+150h] [bp-24h]@16
    signed int inter_left;            // [sp+154h] [bp-20h]@3
    RenderVertexSoft *v75;       // [sp+158h] [bp-1Ch]@3
    float v76 {};                   // [sp+15Ch] [bp-18h]@9
    int v77 {};                     // [sp+160h] [bp-14h]@9
    int toggle_flag;                     // [sp+164h] [bp-10h]@7
    void *fp_worldviewdepth;                   // [sp+168h] [bp-Ch]@9
    float v80;                   // [sp+16Ch] [bp-8h]@3
    const void *v81;             // [sp+170h] [bp-4h]@7
    int fp_over_viewplanedist {};

    HEXRAYS_LODWORD(v73) = 0;
    v80 = v76;

    if ((signed int)pSkyPolygon.uNumVertices > 0) {
        v28 = (double)HEXRAYS_SLODWORD(v76);
        y_proj = (int)(char *)VertexRenderList + 28;
        uint i = 0;
        for (toggle_flag = pSkyPolygon.uNumVertices; toggle_flag; --toggle_flag) {
            ++HEXRAYS_LODWORD(v73);
            VertexRenderList[i] = array_507D30[i];
            y_proj += 48;
            if (v28 <= array_507D30[i].vWorldViewProjY ||
                v28 >= array_507D30[i + 1].vWorldViewProjY) {
                if (v28 >= array_507D30[i].vWorldViewProjY ||
                    v28 <= array_507D30[i + 1].vWorldViewProjY) {
                    i++;
                    continue;
                }
                v33 = (array_507D30[i + 1].vWorldViewProjX - array_507D30[i].vWorldViewProjX) *
                    v28 /
                    (array_507D30[i + 1].vWorldViewProjY - array_507D30[i].vWorldViewProjY) +
                    array_507D30[i + 1].vWorldViewProjX;
            } else {
                v33 = (array_507D30[i].vWorldViewProjX - array_507D30[i + 1].vWorldViewProjX) *
                    v28 /
                    (array_507D30[i].vWorldViewProjY - array_507D30[i + 1].vWorldViewProjY) +
                    array_507D30[i].vWorldViewProjX;
            }
            VertexRenderList[i + 1].vWorldViewProjX = v33;
            ++HEXRAYS_LODWORD(v73);
            *(unsigned int *)(int64_t)y_proj = v28;
            y_proj += 48;
            i++;
        }
    }
    uint j = 0;
    if (HEXRAYS_SLODWORD(v73) <= 0) goto LABEL_40;
    // v34 = (char *)&VertexRenderList[0].vWorldViewProjY;
    v65 = v77 >> 14;
    // HIDWORD(v69) = LODWORD(v73);
    for (int t = (int)HEXRAYS_LODWORD(v73); t > 1; t--) {
        v35 = (const void *)(fp_over_viewplanedist * ((int)blv_horizon_height_offset - (uint64_t)(int64_t)
            VertexRenderList[j]
            .vWorldViewProjY));

        // toggle_flag = pSkyPolygon.ptr_38->viewing_angle_from_west_east;
        // v81 = (const void
        // *)fixpoint_mul(pSkyPolygon.ptr_38->viewing_angle_from_west_east, v35);
        v36 =
            fixpoint_mul(static_cast<int>(pSkyPolygon.ptr_38->CamVecLeft_Z * 65536.0), (int)v35) +
            pSkyPolygon.ptr_38->CamVecLeft_Y * 65536.0;

        v81 = v35;
        inter_left = v36;
        // toggle_flag = pSkyPolygon.ptr_38->viewing_angle_from_north_south;
        v81 = (const void *)fixpoint_mul(
            static_cast<int>(pSkyPolygon.ptr_38->CamVecFront_Z * 65536.0), (int)v35);
        toggle_flag = (int)v35;
        v75 = (RenderVertexSoft *)((char *)v81 +
            int(pSkyPolygon.ptr_38->CamVecFront_Y * 65536.0));
        // v81 = (const void *)pSkyPolygon.v_18.z;
        toggle_flag = fixpoint_mul(pSkyPolygon.v_18.z, (int)v35);
        v37 = (const void *)(fp_over_viewplanedist * (pBLVRenderParams->uViewportCenterX -
            (uint64_t)(int64_t)
            VertexRenderList[j]
            .vWorldViewProjX));
        v38 = (int64_t)(VertexRenderList[j].vWorldViewProjY - 1.0);
        v81 = 0;
        y_proj = v38;
        v39 = fp_over_viewplanedist * (blv_horizon_height_offset - v38);
        while (1) {
            toggle_flag = v39;
            if (!X) goto LABEL_36;
            v40 = abs(X);
            if (abs((int64_t)v65) <= v40) break;
            if (HEXRAYS_SLODWORD(v76) <= (signed int)pViewport->uViewportTL_Y)
                break;
            v39 = toggle_flag;
        LABEL_36:
            toggle_flag = pSkyPolygon.v_18.z;
            v41 = fixpoint_mul(pSkyPolygon.v_18.z, v39);
            --y_proj;
            v39 += fp_over_viewplanedist;
            X = v41 + pSkyPolygon.v_18.x;
            v81 = (const void *)1;
        }
        if (v81) {
            fp_worldviewdepth = (void *)pSkyPolygon.v_18.z;
            toggle_flag = 2 * (int64_t)y_proj;
            v81 = (const void *)fixpoint_mul(
                pSkyPolygon.v_18.z,
                static_cast<int>(
                (((double)blv_horizon_height_offset - ((double)(2 * (int64_t)y_proj) -
                    VertexRenderList[j].vWorldViewProjY)) *
                    (double)fp_over_viewplanedist)));
            X = (int)((char *)v81 + pSkyPolygon.v_18.x);
        }
        HEXRAYS_LODWORD(v42) = v77 << 16;
        HEXRAYS_HIDWORD(v42) = v77 >> 16;
        fp_worldviewdepth = (void *)(v42 / X);
        v81 = v37;

        // toggle_flag = pSkyPolygon.ptr_38->angle_from_west;
        v81 = (const void *)fixpoint_mul(static_cast<int>(pSkyPolygon.ptr_38->CamVecLeft_X * 65536.0f),
            (int)v37);
        v43 = inter_left + fixpoint_mul(static_cast<int>(pSkyPolygon.ptr_38->CamVecLeft_X * 65536.0f), (int)v37);
        inter_left = (unsigned int)v37;
        y_proj = v43;

        // toggle_flag = pSkyPolygon.ptr_38->angle_from_south;
        v75 = (RenderVertexSoft *)((char *)v75 +
            fixpoint_mul(
                static_cast<int>(pSkyPolygon.ptr_38->CamVecFront_X * 65536.0f),
                (int)v37));
        // inter_left = fixpoint_mul(v43, v42 / X);
        v81 = (const void *)fixpoint_mul((int)v75, v42 / X);

        // v34 += 48;
        // toggle_flag = tex_v_offset + ((signed int)fixpoint_mul(v43, v42 / X) >> 4);
        // v44 = HIDWORD(v69)-- == 1;
        // v45 = (double)(tex_v_offset + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) *
        // 0.000015259022; toggle_flag = tex_u_offset + ((signed int)fixpoint_mul((int)v75, v42 /
        // X) >> 4);
        VertexRenderList[j].u =
            ((double)(tex_v_offset + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) *
                0.000015259022) *
                (1.0 / (double)pSkyPolygon.texture->GetWidth());
        VertexRenderList[j].v =
            ((double)(tex_v_offset + ((signed int)fixpoint_mul(v43, v42 / X) >> 4)) *
                0.000015259022) *
            one_over_texheight;
        // v46 = (double)(signed int)fp_worldviewdepth;
        VertexRenderList[j].vWorldViewPosition.x =
            0.000015258789 * (double)(signed int)fp_worldviewdepth;
        VertexRenderList[j]._rhw = 65536.0 / (double)(signed int)fp_worldviewdepth;
        ++j;
    }
    // while ( !v44 );
LABEL_40:
    uint i = 0;
    if (HEXRAYS_SLODWORD(v73) > 0) {
        v48 = (double)HEXRAYS_SLODWORD(v80);
        for (HEXRAYS_HIDWORD(v69) = HEXRAYS_LODWORD(v73); HEXRAYS_HIDWORD(v69);
            --HEXRAYS_HIDWORD(v69)) {
            if (v48 >= VertexRenderList[i].vWorldViewProjY) {
                ++i;
                array_507D30[i] = VertexRenderList[i];
            }
        }
    }
    pSkyPolygon.uNumVertices = i;
    DrawIndoorSkyPolygon(pSkyPolygon.uNumVertices, &pSkyPolygon);
    int pNumVertices = 0;
    if (HEXRAYS_SLODWORD(v73) > 0) {
        v51 = (double)HEXRAYS_SLODWORD(v80);
        for (v80 = v73; v80 != 0.0; --HEXRAYS_LODWORD(v80)) {
            if (v51 <= VertexRenderList[pNumVertices].vWorldViewProjY) {
                ++pNumVertices;
                array_507D30[pNumVertices] = VertexRenderList[pNumVertices];
            }
        }
    }
    DrawIndoorSkyPolygon(pNumVertices, &pSkyPolygon);
}

void Render::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) {
    if (uNumD3DSceneBegins == 0) {
        return;
    }

    unsigned int uNumVertices = pSkyPolygon->uNumVertices;
    TextureD3D *texture = (TextureD3D *)pSkyPolygon->texture;

    if (uNumVertices >= 3) {
        this->pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
            D3DTADDRESS_WRAP);
        if (engine->IsSpecular_FogIsOn()) {
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
            this->pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
        }
        for (unsigned int i = 0; i < uNumVertices; ++i) {
            pVertices[i].pos.x = VertexRenderList[i].vWorldViewProjX;
            pVertices[i].pos.y = VertexRenderList[i].vWorldViewProjY;
            pVertices[i].pos.z = 0.99989998f;
            pVertices[i].rhw = VertexRenderList[i]._rhw;

            pVertices[i].diffuse = ::GetActorTintColor(31, 0, VertexRenderList[i].vWorldViewPosition.x, 1, 0);
            int v7 = 0;
            if (engine->IsSpecular_FogIsOn())
                v7 = sub_47C3D7_get_fog_specular(0, 1, VertexRenderList[i].vWorldViewPosition.x);
            pVertices[i].specular = v7;
            pVertices[i].texcoord.x = VertexRenderList[i].u;
            pVertices[i].texcoord.y = VertexRenderList[i].v;
        }
        pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture());
        pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            pVertices, uNumVertices,
            /*D3DDP_DONOTUPDATEEXTENTS |*/ D3DDP_DONOTLIGHT);
        drawcalls++;
    }
}

void Render::DrawIndoorSkyPolygon(int uNumVertices, struct Polygon *pSkyPolygon) {
    TextureD3D *texture = (TextureD3D *)pSkyPolygon->texture;

    if (uNumD3DSceneBegins == 0) {
        return;
    }

    if (uNumVertices >= 3) {
        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
            D3DTADDRESS_WRAP));
        int v5 = 31 - (pSkyPolygon->dimming_level & 0x1F);
        if (v5 < pOutdoor->max_terrain_dimming_level) {
            v5 = pOutdoor->max_terrain_dimming_level;
        }
        for (uint i = 0; i < (unsigned int)uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
            d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
            d3d_vertex_buffer[i].pos.z =
                1.0 -
                1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
            d3d_vertex_buffer[i].rhw = array_507D30[i]._rhw;
            d3d_vertex_buffer[i].diffuse =
                8 * v5 | ((8 * v5 | (v5 << 11)) << 8);
            d3d_vertex_buffer[i].specular = 0;
            d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u;
            d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v;
        }
        ErrD3D(
            pRenderD3D->pDevice->SetTexture(0, texture->GetDirect3DTexture()));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(
            D3DPT_TRIANGLEFAN,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
            d3d_vertex_buffer, uNumVertices, D3DDP_DONOTLIGHT));
        drawcalls++;

        if (engine->config->debug.Terrain.Get())
            pCamera3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x00FF0000, 0.0);
    }
}

bool Render::NuklearCreateDevice() { return false; }
bool Render::NuklearInitialize(struct nk_tex_font *tfont) { return false; }
bool Render::NuklearRender(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer) { return false; }
void Render::NuklearRelease() {}
struct nk_tex_font *Render::NuklearFontLoad(const char *font_path, size_t font_size) { return NULL; }
void Render::NuklearFontFree(struct nk_tex_font *tfont) {}
struct nk_image Render::NuklearImageLoad(Image* img) { return nk_image_id(0);  }
void Render::NuklearImageFree(Image *img) {}

void Render::ReleaseTerrain() { return; }
void Render::ReleaseBSP() { return; }
void Render::DrawTwodVerts() { return; }
