#include "Engine/Graphics/DecalBuilder.h"

#include <cstdlib>
#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/OurMath.h"
#include "Engine/Time.h"
#include "Engine/stru314.h"

#include "Outdoor.h"

#include "Engine/Graphics/stru9.h"

struct DecalBuilder *pDecalBuilder = new DecalBuilder;
struct BloodsplatContainer *pBloodsplatContainer = new BloodsplatContainer;

//----- (0043B570) --------------------------------------------------------
double DecalBuilder_stru0::_43B570_get_color_mult_by_time() {
    double result;  // st7@3

    if (field_1C_flags & 1) {
        if ((field_20_time - pEventTimer->Time() + 384) / 384.0 >= 0.0)
            result = (field_20_time - pEventTimer->Time() + 384) / 384.0;
        else
            result = 0.0;
    } else {
        result = 1.0;
    }
    return result;
}

//----- (0043B6EF) --------------------------------------------------------
void BloodsplatContainer::AddBloodsplat(float x, float y, float z, float radius,
                                        unsigned char r, unsigned char g,
                                        unsigned char b) {
    int i = this->uNumBloodsplats;
    if (this->uNumBloodsplats == 64) i = 0;
    this->std__vector_pBloodsplats[i].x = x;
    this->std__vector_pBloodsplats[i].y = y;
    this->std__vector_pBloodsplats[i].z = z;
    this->std__vector_pBloodsplats[i].radius = radius;
    this->std__vector_pBloodsplats[i].r = r;
    this->std__vector_pBloodsplats[i].g = g;
    this->std__vector_pBloodsplats[i].b = b;
    this->std__vector_pBloodsplats_size =
        min(this->std__vector_pBloodsplats_size + 1, 64);
}

//----- (0049B490) --------------------------------------------------------
void DecalBuilder::AddBloodsplat(float x, float y, float z, float r, float g,
                                 float b, float radius, int a8, int a9) {
    pBloodsplatContainer->AddBloodsplat(
        x, y, z, radius, bankersRounding(r * 255.0f),
        bankersRounding(g * 255.0f), bankersRounding(b * 255.0f));
}

//----- (0049B525) --------------------------------------------------------
void DecalBuilder::Reset(unsigned int bPreserveBloodsplats) {
    if (!bPreserveBloodsplats) {
        pBloodsplatContainer->std__vector_pBloodsplats_size = 0;
        pBloodsplatContainer->uNumBloodsplats = 0;
    }
    DecalsCount = 0;
}

//----- (0049B540) --------------------------------------------------------
char DecalBuilder::ApplyDecals(int light_level, char a3, stru154 *a4, int a5,
                               RenderVertexSoft *a6, IndoorCameraD3D_Vec4 *a7,
                               char a8, unsigned int uSectorID) {
    stru154 *v16;  // esi@12
    int v25;       // ebx@21
    int v43;       // [sp+3Ch] [bp-Ch]@21
    // DecalBuilder *thisa; // [sp+40h] [bp-8h]@1
    // RenderVertexSoft *a11; // [sp+44h] [bp-4h]@8
    //  int a6a;
    // int *a6b;

    //  __debugbreak();

    // auto a2 = light_level;
    // auto a9 = uSectorID;

    // thisa = this;
    if (!a5) return 0;

    static RenderVertexSoft static_AE4F90[64];
    static bool __init_flag1 = false;
    if (!__init_flag1) {
        __init_flag1 = true;

        for (uint i = 0; i < 64; ++i) static_AE4F90[i].flt_2C = 0.0f;
    }

    static stru314 static_AE4F60;  // idb
    /*static bool __init_flag2 = false;
    if (!__init_flag2)
    {
      __init_flag2 = true;

      stru314::stru314(&static_AE4F60);
    }*/

    // a11 = a6;
    if (a7) {
        for (int i = 0; i < a5; i++) {
            memcpy(&static_AE4F90[i], a6, 0x30u);
            ++a6;
        }
        v16 = a4;
        if (pIndoorCameraD3D->_437376(a4, static_AE4F90, (unsigned int *)&a5) ==
            1) {
            if (!a5) return 0;
            a6 = static_AE4F90;
        }
    } else {
        v16 = a4;
    }
    // v18 = v16->face_plane.vNormal.z;
    // v19 = v16->face_plane.vNormal.y;
    // v20 = v16->face_plane.vNormal.x;
    // v37 = (int)&static_AE4F60.field_1C;
    static_AE4F60.Normal.y = v16->face_plane.vNormal.y;
    static_AE4F60.Normal.x = v16->face_plane.vNormal.x;
    // LODWORD(v36) = (DWORD)&static_AE4F60.field_10;
    static_AE4F60.Normal.z = v16->face_plane.vNormal.z;
    static_AE4F60.dist = v16->face_plane.dist;
    if (!pIndoorCameraD3D->GetFacetOrientation(
            v16->polygonType, &static_AE4F60.Normal, &static_AE4F60.field_10,
            &static_AE4F60.field_1C))
        logger->Warning(L"Error: Failed to get the facet orientation");

    if (this->uNumDecals > 0) {
        // a6b = this->std__vector_30B00C;
        for (int i = 0; i < this->uNumDecals; ++i) {
            // v21 = &pBloodsplatContainer->std__vector_pBloodsplats[*a6b];
            int point_light_level =
                _43F5C8_get_point_light_level_with_respect_to_lights(
                    light_level, uSectorID,
                    pBloodsplatContainer
                        ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                        .x,
                    pBloodsplatContainer
                        ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                        .y,
                    pBloodsplatContainer
                        ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                        .z);

            v43 = pBloodsplatContainer
                      ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                      .b |
                  ((unsigned int)pBloodsplatContainer
                       ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                       .g
                   << 8) |
                  ((unsigned int)pBloodsplatContainer
                       ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                       .r
                   << 16);
            v25 = (signed __int64)pBloodsplatContainer
                      ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                      .x;

            if (!this->_49B790_build_decal_geometry(
                    point_light_level, a3,
                    &pBloodsplatContainer->std__vector_pBloodsplats
                         [this->std__vector_30B00C[i]],
                    (int)&v25,
                    pBloodsplatContainer
                        ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                        .radius,
                    v43,
                    pBloodsplatContainer
                        ->std__vector_pBloodsplats[this->std__vector_30B00C[i]]
                        .dot_dist,
                    &static_AE4F60, a5, a6, a8))
                logger->Warning(L"Error: Failed to build decal geometry");
        }
    }
    return 1;
}

//----- (0049B790) --------------------------------------------------------
char DecalBuilder::_49B790_build_decal_geometry(
    int a2, char a3, Bloodsplat *blood, int a5, float a6,
    unsigned int uColorMultiplier, float a8, stru314 *a9, signed int a10,
    RenderVertexSoft *a11, char uClipFlags) {
    Decal *decal;     // edi@2
    double v28;       // st7@5
    char result;      // al@6
    int v34;          // eax@19
    std::string v37;  // [sp-4h] [bp-24h]@15

    unsigned int a8b = 0;

    if (a6 == 0.0f) return 1;
    decal = &this->Decals[this->curent_decal_id];
    this->Decals[this->curent_decal_id].field_C18 = (DecalBuilder_stru0 *)blood;
    this->Decals[this->curent_decal_id].field_C1C = 0;
    if (a3 & 2) this->Decals[this->curent_decal_id].field_C1C = 1;
    this->field_30C028 = a6 - a8;
    this->field_30C02C =
        sqrt((a6 + a6 - this->field_30C028) * this->field_30C028);

    this->flt_30C030 = 1.0 - (a6 - this->field_30C02C) / a6;
    decal->field_C08 = (signed __int64)(blood->x - a8 * a9->Normal.x);
    decal->field_C0A = (signed __int64)(blood->y - a8 * a9->Normal.y);
    decal->field_C0C = (signed __int64)(blood->z - a8 * a9->Normal.z);

    this->field_30C034 = a6 * this->flt_30C030;
    this->field_30C010 = this->field_30C034 * a9->field_10.x;
    this->field_30C014 = this->field_30C034 * a9->field_10.y;
    this->field_30C018 = this->field_30C034 * a9->field_10.z;

    this->field_30C01C = this->field_30C034 * a9->field_1C.x;
    this->field_30C020 = this->field_30C034 * a9->field_1C.y;
    this->field_30C024 = this->field_30C034 * a9->field_1C.z;

    decal->pVertices[0].vWorldPosition.x =
        (double)decal->field_C08 - this->field_30C01C + this->field_30C010;
    decal->pVertices[0].vWorldPosition.y =
        (double)decal->field_C0A - this->field_30C020 + this->field_30C014;
    decal->pVertices[0].vWorldPosition.z =
        (double)decal->field_C0A - this->field_30C024 + this->field_30C018;
    decal->pVertices[0].u = 0.0;
    decal->pVertices[0].v = 0.0;

    decal->pVertices[1].vWorldPosition.x =
        (double)decal->field_C08 - this->field_30C01C - this->field_30C010;
    decal->pVertices[1].vWorldPosition.y =
        (double)decal->field_C0A - this->field_30C020 - this->field_30C014;
    decal->pVertices[1].vWorldPosition.z =
        (double)decal->field_C0A - this->field_30C024 - this->field_30C018;
    decal->pVertices[1].u = 0.0;
    decal->pVertices[1].v = 1.0;

    decal->pVertices[2].vWorldPosition.x =
        (double)decal->field_C08 + this->field_30C01C - this->field_30C010;
    decal->pVertices[2].vWorldPosition.y =
        (double)decal->field_C0A + this->field_30C020 - this->field_30C014;
    decal->pVertices[2].vWorldPosition.z =
        (double)decal->field_C0A + this->field_30C024 - this->field_30C018;
    decal->pVertices[2].u = 1.0;
    decal->pVertices[2].v = 1.0;

    decal->pVertices[3].vWorldPosition.x =
        (double)decal->field_C08 + this->field_30C01C + this->field_30C010;
    decal->pVertices[3].vWorldPosition.y =
        (double)decal->field_C0A + this->field_30C020 + this->field_30C014;
    decal->pVertices[3].vWorldPosition.z =
        (double)decal->field_C0A + this->field_30C024 + this->field_30C018;
    decal->pVertices[3].u = 1.0;
    decal->pVertices[3].v = 0.0;

    for (uint i = 0; i < 4; ++i) {
        v28 = a9->Normal.x * decal->pVertices[i].vWorldPosition.x +
              a9->Normal.y * decal->pVertices[i].vWorldPosition.y +
              a9->Normal.z * decal->pVertices[i].vWorldPosition.z + a9->dist;
        decal->pVertices[i].vWorldPosition.x =
            decal->pVertices[i].vWorldPosition.x - v28 * a9->Normal.x;
        decal->pVertices[i].vWorldPosition.y =
            decal->pVertices[i].vWorldPosition.y - v28 * a9->Normal.y;
        decal->pVertices[i].vWorldPosition.z =
            decal->pVertices[i].vWorldPosition.z - v28 * a9->Normal.z;
    }
    decal->uColorMultiplier = uColorMultiplier;
    decal->uNumVertices = 4;
    decal->field_C14 = a2;
    result = pEngine->pStru9Instance->_4980B9(
        a11, a10, a9->Normal.x, a9->Normal.y, a9->Normal.z, decal->pVertices,
        (signed int *)&decal->uNumVertices);
    if (result) {
        if (!decal->uNumVertices) return 1;

        pIndoorCameraD3D->ViewTransform(decal->pVertices,
                                        (unsigned int)decal->uNumVertices);
        pIndoorCameraD3D->Project(decal->pVertices, decal->uNumVertices, 0);
        if (!(uClipFlags & 1)) {
            ++this->curent_decal_id;
            v34 = 1024;
            if (this->curent_decal_id == 1024) this->curent_decal_id = 0;
            if ((signed int)(this->DecalsCount + 1) <= 1024)
                v34 = this->DecalsCount + 1;
            this->DecalsCount = v34;
            return 1;
        }
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
            if (uClipFlags & 2) {
                pIndoorCameraD3D->LightmapNeerClip(decal->pVertices,
                                                   decal->uNumVertices,
                                                   this->pVertices, &a8b);
                pIndoorCameraD3D->_437143(a8b, decal->pVertices,
                                          this->pVertices,
                                          (signed int *)&decal->uNumVertices);
            } else if (uClipFlags & 4) {
                pIndoorCameraD3D->LightmapFarClip(decal->pVertices,
                                                  decal->uNumVertices,
                                                  this->pVertices, &a8b);
                pIndoorCameraD3D->_437143(a8b, decal->pVertices,
                                          this->pVertices,
                                          (signed int *)&decal->uNumVertices);
            } else {
                logger->Warning(L"Undefined clip flag specified");
            }
        } else {
            logger->Warning(
                L"Lightpoly builder native indoor clipping not implemented");
        }
        if (a8b != 0) {
            ++this->curent_decal_id;
            v34 = 1024;
            if (this->curent_decal_id == 1024) this->curent_decal_id = 0;
            if ((signed int)(this->DecalsCount + 1) <= 1024)
                v34 = this->DecalsCount + 1;
            this->DecalsCount = v34;
            return 1;
        }
        result = 1;
    }
    return result;
}

//----- (0049BBBD) --------------------------------------------------------
bool DecalBuilder::ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID) {
    double v7;  // st7@12

    uNumDecals = 0;
    if (!pBloodsplatContainer->std__vector_pBloodsplats_size) return true;

    BLVFace *pFace = &pIndoor->pFaces[uFaceID];

    if (pFace->Indoor_sky() || pFace->Fluid()) return true;
    for (uint i = 0; i < pBloodsplatContainer->std__vector_pBloodsplats_size;
         ++i) {
        Bloodsplat *pBloodsplat =
            &pBloodsplatContainer->std__vector_pBloodsplats[i];
        if (pFace->pBounding.x1 - pBloodsplat->radius < pBloodsplat->x &&
            pFace->pBounding.x2 + pBloodsplat->radius > pBloodsplat->x &&
            pFace->pBounding.y1 - pBloodsplat->radius < pBloodsplat->y &&
            pFace->pBounding.y2 + pBloodsplat->radius > pBloodsplat->y &&
            pFace->pBounding.z1 - pBloodsplat->radius < pBloodsplat->z &&
            pFace->pBounding.z2 + pBloodsplat->radius > pBloodsplat->z) {
            v7 = pFace->pFacePlane.vNormal.z * pBloodsplat->z +
                 pFace->pFacePlane.vNormal.y * pBloodsplat->y +
                 pFace->pFacePlane.vNormal.x * pBloodsplat->x +
                 pFace->pFacePlane.dist;
            if (v7 <= pBloodsplat->radius) {
                pBloodsplat->dot_dist = v7;
                std__vector_30B00C[uNumDecals++] = i;
            }
        }
    }

    return true;
}

//----- (0049BCEB) --------------------------------------------------------
char DecalBuilder::ApplyDecals_OutdoorFace(ODMFace *pFace) {
    double v8;  // st7@12
    // unsigned int v10; // [sp+20h] [bp-1Ch]@1

    Bloodsplat *pBloodsplat;

    this->uNumDecals = 0;
    // v10 = pBloodsplatContainer->std__vector_pBloodsplats_size;
    if (!pFace->Indoor_sky() && !pFace->Fluid()) {
        for (int i = 0; i < pBloodsplatContainer->std__vector_pBloodsplats_size;
             i++) {
            pBloodsplat = &pBloodsplatContainer->std__vector_pBloodsplats[i];
            if ((double)pFace->pBoundingBox.x1 - pBloodsplat->radius <
                    pBloodsplat->x &&
                (double)pFace->pBoundingBox.x2 + pBloodsplat->radius >
                    pBloodsplat->x &&
                (double)pFace->pBoundingBox.y1 - pBloodsplat->radius <
                    pBloodsplat->y &&
                (double)pFace->pBoundingBox.y2 + pBloodsplat->radius >
                    pBloodsplat->y &&
                (double)pFace->pBoundingBox.z1 - pBloodsplat->radius <
                    pBloodsplat->z &&
                (double)pFace->pBoundingBox.z2 + pBloodsplat->radius >
                    pBloodsplat->z) {
                v8 = (double)((pFace->pFacePlane.dist +
                               round_to_int(pBloodsplat->x) *
                                   pFace->pFacePlane.vNormal.x +
                               round_to_int(pBloodsplat->y) *
                                   pFace->pFacePlane.vNormal.y +
                               round_to_int(pBloodsplat->z) *
                                   pFace->pFacePlane.vNormal.z) >>
                              16);
                if (v8 <= pBloodsplat->radius) {
                    pBloodsplat->dot_dist = v8;
                    this->std__vector_30B00C[this->uNumDecals++] = i;
                }
            }
        }
    }
    return 1;
}

//----- (0049BE8A) --------------------------------------------------------
bool DecalBuilder::_49BE8A(struct Polygon *a2, Vec3_float_ *_a3, float *a4,
                           RenderVertexSoft *a5, unsigned int uStripType,
                           char a7) {
    bool result;  // eax@1
    // RenderVertexSoft *v8; // edi@3
    // Vec3_float_ *v9; // ebx@3
    // Bloodsplat *v10; // esi@3
    // float v11; // eax@5
    float v12;  // eax@6
    // double v13; // st7@13
    double v14;  // st7@19
    // short v15; // eax@20
    int v16;  // eax@22
    // int v17; // edx@24
    // DecalBuilder *v18; // eax@24
    std::string v19;  // [sp-18h] [bp-54h]@12
                      //  const char *v20; // [sp-8h] [bp-44h]@12
    // int v21; // [sp-4h] [bp-40h]@12
    double v22;        // [sp+Ch] [bp-30h]@19
    unsigned int v23;  // [sp+14h] [bp-28h]@1
    // DecalBuilder *v24; // [sp+18h] [bp-24h]@1
    // int v25; // [sp+1Ch] [bp-20h]@19
    float v26;  // [sp+20h] [bp-1Ch]@12
                //  int v27; // [sp+24h] [bp-18h]@12
    float v28;  // [sp+28h] [bp-14h]@13
    // float v29; // [sp+2Ch] [bp-10h]@7
    float v30;  // [sp+30h] [bp-Ch]@6
    float v31;  // [sp+34h] [bp-8h]@6
    // bool v32; // [sp+38h] [bp-4h]@2
    float a3;

    this->uNumDecals = 0;
    if (!pBloodsplatContainer->std__vector_pBloodsplats_size) return false;
    // v24 = this;
    v23 = pBloodsplatContainer->std__vector_pBloodsplats_size;
    if (pBloodsplatContainer->std__vector_pBloodsplats_size) {
        if ((signed int)pBloodsplatContainer->std__vector_pBloodsplats_size >
            0) {
            // v8 = a5;
            // v9 = _a3;
            for (uint i = 0; i < (signed int)v23; ++i) {
                if (uStripType == 4) {
                    a3 = a5->vWorldPosition.x;
                    // v11 = v8[3].vWorldPosition.x;
                    v31 = a5[3].vWorldPosition.x;
                    v30 = a5[1].vWorldPosition.y;
                    v12 = a5->vWorldPosition.y;
                    // v29 = v12;
                } else if (uStripType == 3) {
                    if (a7) {
                        a3 = a5->vWorldPosition.x;
                        v31 = a5[2].vWorldPosition.x;
                        v30 = a5[1].vWorldPosition.y;
                        v12 = a5[2].vWorldPosition.y;
                        // v29 = v12;
                    } else {
                        a3 = a5[1].vWorldPosition.x;
                        // v11 = v8[2].vWorldPosition.x;
                        v31 = a5[2].vWorldPosition.x;
                        v30 = a5[1].vWorldPosition.y;
                        v12 = a5->vWorldPosition.y;
                        // v29 = v12;
                    }
                } else {
                    logger->Warning(L"Uknown strip type detected!");
                }
                // v21 = uStripType;
                // v13 = pIndoorCameraD3D->GetPolygonMinZ(v8, uStripType);
                // v21 = uStripType;
                v28 = pIndoorCameraD3D->GetPolygonMinZ(a5, uStripType);
                v26 = pIndoorCameraD3D->GetPolygonMaxZ(a5, uStripType);
                if (a3 - pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius <
                        pBloodsplatContainer->std__vector_pBloodsplats[i].x &&
                    v31 + pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius >
                        pBloodsplatContainer->std__vector_pBloodsplats[i].x &&
                    v30 - pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius <
                        pBloodsplatContainer->std__vector_pBloodsplats[i].y &&
                    v12 + pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius >
                        pBloodsplatContainer->std__vector_pBloodsplats[i].y &&
                    v28 - pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius <
                        pBloodsplatContainer->std__vector_pBloodsplats[i].z &&
                    v26 + pBloodsplatContainer->std__vector_pBloodsplats[i]
                                .radius >
                        pBloodsplatContainer->std__vector_pBloodsplats[i].z) {
                    Vec3_float_::NegDot(&a5->vWorldPosition, _a3, a4);
                    v26 = _a3->y *
                              pBloodsplatContainer->std__vector_pBloodsplats[i]
                                  .y +
                          _a3->z *
                              pBloodsplatContainer->std__vector_pBloodsplats[i]
                                  .z +
                          _a3->x *
                              pBloodsplatContainer->std__vector_pBloodsplats[i]
                                  .x +
                          *a4;
                    v22 = v26 + 0.5f;
                    // v25 = LODWORD(v22);
                    v14 = (double)HEXRAYS_SLODWORD(v22);
                    v28 = v14;
                    if (v14 <= pBloodsplatContainer->std__vector_pBloodsplats[i]
                                   .radius) {
                        // v15 = a2->flags;
                        if (a2->flags & 2 || a2->flags & 0x100) {
                            v16 = pBloodsplatContainer
                                      ->std__vector_pBloodsplats[i]
                                      .field_1C;
                            if (!(pBloodsplatContainer
                                      ->std__vector_pBloodsplats[i]
                                      .field_1C &
                                  1)) {
                                v16 |= 1;
                                pBloodsplatContainer
                                    ->std__vector_pBloodsplats[i]
                                    .field_1C = v16;
                                pBloodsplatContainer
                                    ->std__vector_pBloodsplats[i]
                                    .field_20 = pEventTimer->Time();
                            }
                        }
                        // v17 = v32;
                        pBloodsplatContainer->std__vector_pBloodsplats[i]
                            .dot_dist = HEXRAYS_LODWORD(v28);
                        // v18 = this;
                        this->std__vector_30B00C[this->uNumDecals] = i;
                        ++this->uNumDecals;
                    }
                }
                // ++v32;
                // ++v10;
                result = i;
            }
        }
    }

    return true;
}

//----- (0049C2CD) --------------------------------------------------------
void DecalBuilder::DrawDecals(float z_bias) {
    for (uint i = 0; i < DecalsCount; ++i)
        render->DrawDecal(&Decals[i], z_bias);
}

//----- (0049C304) --------------------------------------------------------
void DecalBuilder::DrawBloodsplats() {
    if (!DecalsCount) return;

    render->BeginDecals();

    DrawDecals(0.00039999999);

    render->EndDecals();
}

//----- (0049C550) --------------------------------------------------------
void DecalBuilder::DrawDecalDebugOutlines() {
    for (int i = 0; i < DecalsCount; i++)
        pIndoorCameraD3D->debug_outline_sw(
            Decals[i].pVertices, Decals[i].uNumVertices, 0xC86400u, 0.0f);
}

//----- (0040E4C2) --------------------------------------------------------
void Decal::Decal_base_ctor() {
    uNumVertices = -1;
    for (uint i = 0; i < 64; ++i) pVertices[i].flt_2C = 0.0f;
}
