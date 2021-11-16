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

//----- (0043B570) --------------------------------------------------------
double Decal::Fade_by_time() {
    // splats dont fade
    if (!(decal_flags & 1)) return 1.0;

    // splats fade
    int64_t delta = fadetime - pEventTimer->Time();
    double result = (float(delta) + 3840.0) / 3840.0;
    if (result < 0) result = 0.0;
    return result;
}

//----- (0043B6EF) --------------------------------------------------------
void BloodsplatContainer::AddBloodsplat(float x, float y, float z, float radius,
    unsigned char r, unsigned char g,
    unsigned char b) {

    // this adds to store of bloodsplats to apply
    this->pBloodsplats_to_apply[this->uNumBloodsplats].x = x;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].y = y;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].z = z;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].radius = radius;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].r = r;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].g = g;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].b = b;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].dot_dist = 0;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].blood_flags = 0;
    this->pBloodsplats_to_apply[this->uNumBloodsplats].fade_timer = 0;

    this->uNumBloodsplats++;
    if (this->uNumBloodsplats == 64) this->uNumBloodsplats = 0;
}

//----- (0049B490) --------------------------------------------------------
void DecalBuilder::AddBloodsplat(float x, float y, float z, float r, float g,
    float b, float radius, int a8, int a9) {
    // a8 never used
    // a9 never used

    bloodsplat_container->AddBloodsplat(
        x, y, z, radius, bankersRounding(r * 255.0f),
        bankersRounding(g * 255.0f), bankersRounding(b * 255.0f));
}

//----- (0049B525) --------------------------------------------------------
void DecalBuilder::Reset(bool bPreserveBloodsplats) {
    if (!bPreserveBloodsplats) {
        bloodsplat_container->uNumBloodsplats = 0;
    }
    DecalsCount = 0;
}

//----- (0049B540) --------------------------------------------------------
char DecalBuilder::ApplyDecals(int light_level, char LocationFlags, stru154* a4, int a5,
    RenderVertexSoft* a6, IndoorCameraD3D_Vec4* a7,
    char ClipFlags, unsigned int uSectorID) {

    // actually build decal geom and apply it

    // LocationFlags  1 indoors 2 buildings 4 terrain
    // a4 classified polygon
    // a5 number verts
    // a6 vertices store
    // a7 camera


    stru154* v16;  // esi@12
    int BloodSplatX;       // ebx@21
    int ColourMult;       // [sp+3Ch] [bp-Ch]@21
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
            memcpy(&static_AE4F90[i], a6, sizeof(RenderVertexSoft));
            ++a6;
        }
        v16 = a4;
        if (pIndoorCameraD3D->_437376(a4, static_AE4F90, (unsigned int*)&a5) ==
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
        log->Warning("Error: Failed to get the facet orientation");

    if (this->uNumDecals > 0) {
        // a6b = this->std__vector_30B00C;
        for (int i = 0; i < this->uNumDecals; ++i) {
            // v21 = &pBloodsplatContainer->pBloodsplats_to_apply[*a6b];
            int point_light_level =
                _43F5C8_get_point_light_level_with_respect_to_lights(
                    light_level, uSectorID,
                    bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].x,
                    bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].y,
                    bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].z);

            ColourMult = bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].b |
                ((unsigned int)bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].g << 8) |
                ((unsigned int)bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].r << 16);
            BloodSplatX = (signed __int64)bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].x;

            if (!this->_49B790_build_decal_geometry(
                point_light_level, LocationFlags,
                &bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]],
                (int64_t)&BloodSplatX,
                bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].radius,
                ColourMult,
                bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].dot_dist,
                &static_AE4F60, a5, a6, ClipFlags))
                log->Warning("Error: Failed to build decal geometry");
        }
    }
    return 1;
}

//----- (0049B790) --------------------------------------------------------
char DecalBuilder::_49B790_build_decal_geometry(
    int LightLevel, char LocationFlags, Bloodsplat* blood, int64_t DecalXPos, float DecalRadius,
    unsigned int uColorMultiplier, float DecalDotDist, stru314* FacetNormals, signed int numfaceverts,
    RenderVertexSoft* faceverts, char uClipFlags) {

    // LocationFlags  1 indoors 2 buildings 4 terrain

    /*this->_49B790_build_decal_geometry(
                point_light_level, LocationFlags,
                &bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]],
                (int)&BloodSplatX,
                bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].radius,
                ColourMult,
                bloodsplat_container->pBloodsplats_to_apply[this->std__vector_30B00C[i]].dot_dist,
                &static_AE4F60, a5, a6, a8))*/

    Decal* decal;     // edi@2
    double v28;       // st7@5
    char result;      // al@6
    int v34;          // eax@19
    // std::string v37;  // [sp-4h] [bp-24h]@15

    unsigned int a8b = 0;

    if (DecalRadius == 0.0f) return 1;
    decal = &this->Decals[this->DecalsCount];
    this->Decals[this->DecalsCount].fadetime = blood->fade_timer;
    this->Decals[this->DecalsCount].decal_flags = blood->blood_flags;

    // buildings decals dont fade ??
    if (LocationFlags & 2) this->Decals[this->DecalsCount].decal_flags = 0;

    this->field_30C028 = DecalRadius - DecalDotDist;
    this->field_30C02C = sqrt((DecalRadius + DecalRadius - this->field_30C028) * this->field_30C028);

    this->flt_30C030 = 1.0 - (DecalRadius - this->field_30C02C) / DecalRadius;
    decal->DecalXPos = (signed __int64)(blood->x - DecalDotDist * FacetNormals->Normal.x);
    decal->DecalYPos = (signed __int64)(blood->y - DecalDotDist * FacetNormals->Normal.y);
    decal->DecalZPos = (signed __int64)(blood->z - DecalDotDist * FacetNormals->Normal.z);

    // for decal size
    this->field_30C034 = DecalRadius * this->flt_30C030;
    this->field_30C010 = this->field_30C034 * FacetNormals->field_10.x;
    this->field_30C014 = this->field_30C034 * FacetNormals->field_10.y;
    this->field_30C018 = this->field_30C034 * FacetNormals->field_10.z;

    this->field_30C01C = this->field_30C034 * FacetNormals->field_1C.x;
    this->field_30C020 = this->field_30C034 * FacetNormals->field_1C.y;
    this->field_30C024 = this->field_30C034 * FacetNormals->field_1C.z;

    // vertex position sizing
    decal->pVertices[0].vWorldPosition.x = (double)decal->DecalXPos - this->field_30C01C + this->field_30C010;
    decal->pVertices[0].vWorldPosition.y = (double)decal->DecalYPos - this->field_30C020 + this->field_30C014;
    decal->pVertices[0].vWorldPosition.z = (double)decal->DecalZPos - this->field_30C024 + this->field_30C018;
    decal->pVertices[0].u = 0.0;
    decal->pVertices[0].v = 0.0;

    decal->pVertices[1].vWorldPosition.x = (double)decal->DecalXPos - this->field_30C01C - this->field_30C010;
    decal->pVertices[1].vWorldPosition.y = (double)decal->DecalYPos - this->field_30C020 - this->field_30C014;
    decal->pVertices[1].vWorldPosition.z = (double)decal->DecalZPos - this->field_30C024 - this->field_30C018;
    decal->pVertices[1].u = 0.0;
    decal->pVertices[1].v = 1.0;

    decal->pVertices[2].vWorldPosition.x = (double)decal->DecalXPos + this->field_30C01C - this->field_30C010;
    decal->pVertices[2].vWorldPosition.y = (double)decal->DecalYPos + this->field_30C020 - this->field_30C014;
    decal->pVertices[2].vWorldPosition.z = (double)decal->DecalZPos + this->field_30C024 - this->field_30C018;
    decal->pVertices[2].u = 1.0;
    decal->pVertices[2].v = 1.0;

    decal->pVertices[3].vWorldPosition.x = (double)decal->DecalXPos + this->field_30C01C + this->field_30C010;
    decal->pVertices[3].vWorldPosition.y = (double)decal->DecalYPos + this->field_30C020 + this->field_30C014;
    decal->pVertices[3].vWorldPosition.z = (double)decal->DecalZPos + this->field_30C024 + this->field_30C018;
    decal->pVertices[3].u = 1.0;
    decal->pVertices[3].v = 0.0;


    //
    for (uint i = 0; i < 4; ++i) {
        v28 = FacetNormals->Normal.x * decal->pVertices[i].vWorldPosition.x +
            FacetNormals->Normal.y * decal->pVertices[i].vWorldPosition.y +
            FacetNormals->Normal.z * decal->pVertices[i].vWorldPosition.z + FacetNormals->dist;
        decal->pVertices[i].vWorldPosition.x =
            decal->pVertices[i].vWorldPosition.x - v28 * FacetNormals->Normal.x;
        decal->pVertices[i].vWorldPosition.y =
            decal->pVertices[i].vWorldPosition.y - v28 * FacetNormals->Normal.y;
        decal->pVertices[i].vWorldPosition.z =
            decal->pVertices[i].vWorldPosition.z - v28 * FacetNormals->Normal.z;
    }

    decal->uColorMultiplier = uColorMultiplier;
    decal->uNumVertices = 4;
    decal->DimmingLevel = LightLevel;

    // clip decals to face
    result = engine->pStru9Instance->ClipVertsToFace(
        faceverts, numfaceverts, FacetNormals->Normal.x, FacetNormals->Normal.y, FacetNormals->Normal.z, decal->pVertices,
        (signed int*)&decal->uNumVertices);

    if (result) {
        if (!decal->uNumVertices) return 1;

        // pIndoorCameraD3D->ViewTransform(decal->pVertices, (unsigned int)decal->uNumVertices);
        // pIndoorCameraD3D->Project(decal->pVertices, decal->uNumVertices, 0);

        // if (!(uClipFlags & 1)) {
            this->DecalsCount++;
            if (this->DecalsCount == 1024) this->DecalsCount = 0;
            return 1;
        // }

        // if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        //    if (uClipFlags & 2) {
        //        pIndoorCameraD3D->LightmapNeerClip(decal->pVertices, decal->uNumVertices, this->pVertices, &a8b);
        //        pIndoorCameraD3D->LightmapProject(a8b, decal->pVertices, this->pVertices, (signed int*)&decal->uNumVertices);
        //    }
        //    else if (uClipFlags & 4) {
        //        pIndoorCameraD3D->LightmapFarClip(decal->pVertices, decal->uNumVertices, this->pVertices, &a8b);
        //        pIndoorCameraD3D->LightmapProject(a8b, decal->pVertices, this->pVertices, (signed int*)&decal->uNumVertices);
        //    }
        //    else {
        //        log->Warning("Undefined clip flag specified");
        //    }
        // } else {
        //    log->Warning("Lightpoly builder native indoor clipping not implemented");
        // }

        // if (a8b != 0) {
        //    ++this->curent_decal_id;
        //    v34 = 1024;
        //    if (this->curent_decal_id == 1024) this->curent_decal_id = 0;
        //    if ((signed int)(this->DecalsCount + 1) <= 1024)
        //        v34 = this->DecalsCount + 1;
        //    this->DecalsCount = v34;
        //    return 1;
        // }

        // result = 1;
    }

    return result;
}

//----- (0049BBBD) --------------------------------------------------------
bool DecalBuilder::ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID) {
    double v7;  // st7@12

    uNumDecals = 0;
    if (!bloodsplat_container->uNumBloodsplats) return true;

    BLVFace* pFace = &pIndoor->pFaces[uFaceID];

    if (pFace->Indoor_sky() || pFace->Fluid()) return true;
    for (uint i = 0; i < bloodsplat_container->uNumBloodsplats; ++i) {
        Bloodsplat* pBloodsplat = &bloodsplat_container->pBloodsplats_to_apply[i];
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
char DecalBuilder::ApplyDecals_OutdoorFace(ODMFace* pFace) {
    double v8;  // st7@12
    // unsigned int v10; // [sp+20h] [bp-1Ch]@1

    Bloodsplat* pBloodsplat;

    this->uNumDecals = 0;
    // v10 = pBloodsplatContainer->std__vector_pBloodsplats_size;
    if (!pFace->Indoor_sky() && !pFace->Fluid()) {
        for (int i = 0; i < bloodsplat_container->uNumBloodsplats; i++) {
            pBloodsplat = &bloodsplat_container->pBloodsplats_to_apply[i];
            if ((double)pFace->pBoundingBox.x1 - pBloodsplat->radius < pBloodsplat->x &&
                (double)pFace->pBoundingBox.x2 + pBloodsplat->radius > pBloodsplat->x &&
                (double)pFace->pBoundingBox.y1 - pBloodsplat->radius < pBloodsplat->y &&
                (double)pFace->pBoundingBox.y2 + pBloodsplat->radius > pBloodsplat->y &&
                (double)pFace->pBoundingBox.z1 - pBloodsplat->radius < pBloodsplat->z &&
                (double)pFace->pBoundingBox.z2 + pBloodsplat->radius > pBloodsplat->z) {
                v8 = (double)((pFace->pFacePlane.dist +
                    round_to_int(pBloodsplat->x) * pFace->pFacePlane.vNormal.x +
                    round_to_int(pBloodsplat->y) * pFace->pFacePlane.vNormal.y +
                    round_to_int(pBloodsplat->z) * pFace->pFacePlane.vNormal.z) >> 16);
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
// apply outdoor blodsplats - check to see if bloodsplat hits terrain triangle
bool DecalBuilder::ApplyBloodSplatToTerrain(struct Polygon* terrpoly, Vec3_float_* terrnorm, float* tridotdist,
    RenderVertexSoft* triverts, unsigned int uStripType, char tri_orient) {
    float WorldYPosU = 0;
    float WorldYPosD = 0;
    float WorldXPosL = 0;
    float WorldXPosR = 0;
    float WorldMaxZ = 0;
    float WorldMinZ = 0;
    float planedist = 0;

    // tracks how many decals are applied to this tri
    this->uNumDecals = 0;

    if (!bloodsplat_container->uNumBloodsplats) return false;
    unsigned int NumBloodsplats = bloodsplat_container->uNumBloodsplats;

    if (NumBloodsplats > 0) {
        // loop over blood to lay
        for (uint i = 0; i < NumBloodsplats; ++i) {
            // get triangle geometery
            if (uStripType == 4) {
                WorldXPosR = triverts[0].vWorldPosition.x;  // left
                WorldXPosL = triverts[3].vWorldPosition.x;  // right
                WorldYPosU = triverts[1].vWorldPosition.y;  // bott
                WorldYPosD = triverts[0].vWorldPosition.y;  // top
            } else if (uStripType == 3) {
                if (tri_orient) {
                    WorldXPosR = triverts->vWorldPosition.x;
                    WorldXPosL = triverts[2].vWorldPosition.x;
                    WorldYPosU = triverts[1].vWorldPosition.y;
                    WorldYPosD = triverts[2].vWorldPosition.y;
                } else {
                    WorldXPosR = triverts[1].vWorldPosition.x;
                    WorldXPosL = triverts[2].vWorldPosition.x;
                    WorldYPosU = triverts[1].vWorldPosition.y;
                    WorldYPosD = triverts->vWorldPosition.y;
                }
            } else {
                log->Warning("Uknown strip type detected!");
            }

            WorldMinZ = pIndoorCameraD3D->GetPolygonMinZ(triverts, uStripType);
            WorldMaxZ = pIndoorCameraD3D->GetPolygonMaxZ(triverts, uStripType);

            // check xy bounds
            if (WorldXPosR - bloodsplat_container->pBloodsplats_to_apply[i].radius <
                bloodsplat_container->pBloodsplats_to_apply[i].x &&
                WorldXPosL + bloodsplat_container->pBloodsplats_to_apply[i].radius >
                bloodsplat_container->pBloodsplats_to_apply[i].x &&
                WorldYPosU - bloodsplat_container->pBloodsplats_to_apply[i].radius <
                bloodsplat_container->pBloodsplats_to_apply[i].y &&
                WorldYPosD + bloodsplat_container->pBloodsplats_to_apply[i].radius >
                bloodsplat_container->pBloodsplats_to_apply[i].y) {
                // check z bounds
                if (WorldMinZ - bloodsplat_container->pBloodsplats_to_apply[i].radius <
                    bloodsplat_container->pBloodsplats_to_apply[i].z &&
                    WorldMaxZ + bloodsplat_container->pBloodsplats_to_apply[i].radius >
                    bloodsplat_container->pBloodsplats_to_apply[i].z) {
                    // check plane distance
                    Vec3_float_::NegDot(&triverts->vWorldPosition, terrnorm, tridotdist);

                    planedist = terrnorm->y * bloodsplat_container->pBloodsplats_to_apply[i].y +
                        terrnorm->z * bloodsplat_container->pBloodsplats_to_apply[i].z +
                        terrnorm->x * bloodsplat_container->pBloodsplats_to_apply[i].x + *tridotdist;
                    planedist += 0.5f;

                    if (planedist <= bloodsplat_container->pBloodsplats_to_apply[i].radius) {
                        // blood splat hits this terrain tri

                        // check if water or something else (maybe should be border tile or swampy>)
                        if (terrpoly->flags & 2 || terrpoly->flags & 0x100) {
                            // apply fade flags
                            if (!(bloodsplat_container->pBloodsplats_to_apply[i].blood_flags & 1)) {
                                bloodsplat_container->pBloodsplats_to_apply[i].blood_flags |= 1;
                                bloodsplat_container->pBloodsplats_to_apply[i].fade_timer = pEventTimer->Time();
                            }
                        }

                        bloodsplat_container->pBloodsplats_to_apply[i].fade_timer = pEventTimer->Time();
                        bloodsplat_container->pBloodsplats_to_apply[i].dot_dist = planedist;

                        // store this decal to apply
                        this->std__vector_30B00C[this->uNumDecals] = i;
                        ++this->uNumDecals;
                    }
                }
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

    DrawDecals(0.00039999999);  // 0.00039999999

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
