#include "LightmapBuilder.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/stru314.h"

#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ClippingFunctions.h"
#include "Engine/Graphics/Weather.h"

using EngineIoc = Engine_::IocContainer;

LightsStack_StationaryLight_ *pStationaryLightsStack = new LightsStack_StationaryLight_;
// StationaryLight pStationaryLights[400];
// int uNumStationaryLightsApplied;
LightsStack_MobileLight_ *pMobileLightsStack = new LightsStack_MobileLight_;
// MobileLight pMobileLights[400];
// int uNumMobileLightsApplied;

// TODO(pskelton): drop most of this - no longer using lightmaps

//----- (0045DF13) --------------------------------------------------------
Lightmap::Lightmap() {
    // For initialization step(I)

    NumVertices = -1;
    for (uint i = 0; i < 64; ++i) pVertices[i].flt_2C = 0.0f;
    this->field_C18 = 0;
    uColorMask = 0;
    position_z = 0;
    position_y = 0;
    position_x = 0;
    field_C0E = 0;
    fBrightness = 0;
}

//----- (0045BB06) --------------------------------------------------------
LightmapBuilder::LightmapBuilder() {
    this->log = EngineIoc::ResolveLogger();

    // For initialization step(II)

    for (int i = 0; i < 512; ++i)  // for light type 1
        this->StationaryLights[i] = Lightmap();
    this->StationaryLightsCount = 0;

    for (int i = 0; i < 768; ++i)  // for light type 2
        this->MobileLights[i] = Lightmap();
    this->MobileLightsCount = 0;

    for (int i = 0; i < 256; i++)
        this->field_3C8C34[i].flt_2C = 0.0f;

    uFlags = 0;
    tex_light_radius = 0;
    light_radius = 0;
    light_length_z2 = 0;
    light_length_z = 0;
    light_length_y2 = 0;
    light_length_y = 0;
    light_length_x2 = 0;
    light_length_x = 0;
    flt_3C8C28 = 0;
    flt_3C8C2C_lightmaps_brightness = 0;
}

// ////////////////////////OUTDOOR/LIGHT////////////////////////////////////////////
// ----- (0045CDB7) --------------------------------------------------------
bool LightmapBuilder::ApplyLights_OutdoorFace(ODMFace *pFace) {
    // For outdoor light (I)
    int v3;       // eax@1
    bool result;  // eax@9
    int pSlot;    // [sp+10h] [bp-4h]@1

    v3 = Lights.uDefaultAmbientLightLevel + pFace->uShadeType;
    pSlot = 0;
    Lights.uCurrentAmbientLightLevel = v3 /*<< 16*/;
    for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        if (pSlot >= 20) break;
        ApplyLight_ODM((StationaryLight *)&pMobileLightsStack->pLights[i], pFace,
                       (unsigned int *)&pSlot, false);
    }
    for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        if (pSlot >= 20) break;
        ApplyLight_ODM(&pStationaryLightsStack->pLights[i], pFace,
                       (unsigned int *)&pSlot, false);
    }

    result = pSlot;
    Lights.uNumLightsApplied = pSlot;
    return true;
}

//----- (0045D0D5) --------------------------------------------------------
bool LightmapBuilder::StackLight_TerrainFace(StationaryLight *pLight,
                                             Vec3f *pNormal,
                                             float *light_tile_dist,
                                             RenderVertexSoft *TerrainVertices,
                                             unsigned int uStripType, int X,
                                             unsigned int *pSlot) {
    // For outdoor terrain light (II)
    //  bool result; // eax@1
    char v57;         // dl@18
    // std::string v58;  // [sp-18h] [bp-38h]@10
    BBoxf bbox;

    //  x0,y0      x1,y1 // this is actuall ccw??
    //  .____________.
    //  |            |
    //  |            |
    //  |            |
    //  .____________.
    // x3,y3        x2,y2

    if (pLight->uRadius <= 0) return false;
    if (uStripType == 4) {
        bbox.x1 = TerrainVertices[0].vWorldPosition.x;
        bbox.x2 = TerrainVertices[3].vWorldPosition.x;

        bbox.y1 = TerrainVertices[1].vWorldPosition.y;
        bbox.y2 =  TerrainVertices[0].vWorldPosition.y;
    } else if (uStripType == 3) {
        // __debugbreak();
        if ((unsigned char)X) {
            bbox.x1 = TerrainVertices[0].vWorldPosition.x;
            bbox.x2 = TerrainVertices[2].vWorldPosition.x;

            bbox.y1 = TerrainVertices[1].vWorldPosition.y;
            bbox.y2 = TerrainVertices[2].vWorldPosition.y;
        } else {
            bbox.x1 = TerrainVertices[1].vWorldPosition.x;
            bbox.x2 = TerrainVertices[2].vWorldPosition.x;

            bbox.y1 = TerrainVertices[1].vWorldPosition.y;
            bbox.y2 = TerrainVertices[0].vWorldPosition.y;
        }
    } else {
        log->Warning("Uknown strip type detected!");
    }

    bbox.z1 = pOutdoor->GetPolygonMinZ(TerrainVertices, uStripType);
    bbox.z2 = pOutdoor->GetPolygonMaxZ(TerrainVertices, uStripType);

    //проверяем вершины
    if (!bbox.Expanded(pLight->uRadius).Contains(pLight->vPosition))
        return false;

    *light_tile_dist = -Dot(TerrainVertices->vWorldPosition, *pNormal);
    float p_dot = Dot(pLight->vPosition, *pNormal) + *light_tile_dist + 0.5f;
    if (p_dot > pLight->uRadius) return false;

    Lights._blv_lights_radii[*pSlot] = pLight->uRadius;
    Lights._blv_lights_inv_radii[*pSlot] = 65536 / pLight->uRadius;
    Lights._blv_lights_xs[*pSlot] = pLight->vPosition.x;
    Lights._blv_lights_ys[*pSlot] = pLight->vPosition.y;
    Lights._blv_lights_zs[*pSlot] = pLight->vPosition.z;

    Lights._blv_lights_rs[*pSlot] = pLight->uLightColorR / 255.0f;
    Lights._blv_lights_gs[*pSlot] = pLight->uLightColorG / 255.0f;
    Lights._blv_lights_bs[*pSlot] = pLight->uLightColorB / 255.0f;

    Lights._blv_lights_light_dot_faces[*pSlot] = abs(p_dot);
    Lights._blv_lights_types[*pSlot] = pLight->uLightType;

    v57 = Lights._blv_lights_types[*pSlot];
    if (engine->IsSpecular_FogIsOn() && Lights._blv_lights_types[*pSlot] & 4)
        v57 = _4E94D2_light_type;
    Lights._blv_lights_types[*pSlot] = v57;

    int id = 4 * *pSlot;
    if (engine->IsSpecular_FogIsOn()) {
        if (Lights._blv_lights_types[*pSlot] & 4) {
            Lights._blv_lights_rs[id] = Lights._blv_lights_rs[id] * 0.3300000131130219f;
            Lights._blv_lights_gs[id] = Lights._blv_lights_gs[id] * 0.3300000131130219f;
            Lights._blv_lights_bs[id] = Lights._blv_lights_bs[id] * 0.3300000131130219f;
        }
    }
    ++*pSlot;
    return true;
}

//----- (0045CE50) --------------------------------------------------------
bool LightmapBuilder::ApplyLight_ODM(StationaryLight *pLight, ODMFace *pFace,
                                     unsigned int *pSlot,
                                     bool bLightBackfaces) {
    // For outdoor light (IV)
    int result;  // eax@0
    float v10;  // ecx@8
    char v14;    // dl@11

    if (!pLight->uRadius) return false;

    if (pFace->pBoundingBox.Expanded(pLight->uRadius).Contains(pLight->vPosition.ToShort())) {
        v10 = pFace->pFacePlane.SignedDistanceTo(pLight->vPosition);

        if (((bLightBackfaces) || v10 >= 0.0f) && fabsf(v10) <= pLight->uRadius) {
            Lights._blv_lights_radii[*pSlot] = pLight->uRadius;
            Lights._blv_lights_inv_radii[*pSlot] = 65536 / pLight->uRadius;
            Lights._blv_lights_xs[*pSlot] = pLight->vPosition.x;
            Lights._blv_lights_ys[*pSlot] = pLight->vPosition.y;
            Lights._blv_lights_zs[*pSlot] = pLight->vPosition.z;
            Lights._blv_lights_rs[*pSlot] = pLight->uLightColorR / 255.0f;
            Lights._blv_lights_gs[*pSlot] = pLight->uLightColorG / 255.0f;
            Lights._blv_lights_bs[*pSlot] = pLight->uLightColorB / 255.0f;
            Lights._blv_lights_light_dot_faces[*pSlot] =
                abs((int)floorf(v10 + 0.5f));
            Lights._blv_lights_types[*pSlot] = pLight->uLightType;
            v14 = Lights._blv_lights_types[*pSlot];
            if (engine->IsSpecular_FogIsOn() && Lights._blv_lights_types[*pSlot] & 4)
                v14 = _4E94D2_light_type;
            Lights._blv_lights_types[*pSlot] = v14;
            result = 4 * *pSlot;
            if (engine->IsSpecular_FogIsOn()) {
                __debugbreak();
                if (Lights._blv_lights_types[*pSlot] & 4) {
                    *(float *)((char *)Lights._blv_lights_rs + result) =
                        *(float *)((char *)Lights._blv_lights_rs + result) * 0.33000001f;
                    *(float *)((char *)Lights._blv_lights_gs + result) =
                        *(float *)((char *)Lights._blv_lights_gs + result) * 0.33000001f;
                    *(float *)((char *)Lights._blv_lights_bs + result) =
                        *(float *)((char *)Lights._blv_lights_bs + result) * 0.33000001f;
                }
            }

            ++*pSlot;
            return true;
        }
    }

    return false;
}

// ////////////////////////INDOOR
// /LIGHT////////////////////////////////////////////
// ----- (0045C7F6) --------------------------------------------------------
bool LightmapBuilder::ApplyLights_IndoorFace(unsigned int uFaceID) {
    // For Indoor light (I)
    BLVFace *pFace = &pIndoor->pFaces[uFaceID];
    BLVSector *pSector = &pIndoor->pSectors[pFace->uSectorID];

    Lights.uCurrentAmbientLightLevel = (Lights.uDefaultAmbientLightLevel + (31 - pSector->uMinAmbientLightLevel))/*<< 16*/;  // 0x00180000

    uint uNumLightsApplied = 0;
    for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        if (uNumLightsApplied >= 20) break;

        ApplyLight_BLV((StationaryLight *)&pMobileLightsStack->pLights[i],
                       pFace, &uNumLightsApplied, true, 0);
    }

    for (uint i = 0; i < pSector->uNumLights; ++i) {
        if (uNumLightsApplied >= 20) break;

        BLVLightMM7 *pLight = &pIndoor->pLights[pSector->pLights[i]];
        if (~pLight->uAtributes & 0x08)
            ApplyLight_BLV((StationaryLight *)pLight, pFace, &uFaceID, false,
                           &_4E94D0_light_type);
    }

    for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        if (uNumLightsApplied >= 20) break;

        ApplyLight_BLV(&pStationaryLightsStack->pLights[i], pFace,
                       &uNumLightsApplied, false, &_4E94D0_light_type);
    }

    Lights.uNumLightsApplied = uNumLightsApplied;
    return true;
}

//----- (0045C911) --------------------------------------------------------
bool LightmapBuilder::ApplyLight_BLV(StationaryLight *pLight, BLVFace *pFace,
                                     unsigned int *pSlot, bool bLightBackfaces,
                                     char *LightType) {
    // For indoor light (II)
    double Distance;  // st7@8

    if (!pLight->uRadius)  // 800
        return false;

    if (pFace->pBounding.Expanded(pLight->uRadius).Contains(pLight->vPosition.ToShort())) {
        Distance = pFace->pFacePlane.SignedDistanceTo(pLight->vPosition);

        if ((bLightBackfaces || Distance >= 0.0f) && std::abs(Distance) <= pLight->uRadius) {
            unsigned int slot = *pSlot;

            Lights._blv_lights_radii[slot] = pLight->uRadius;
            Lights._blv_lights_inv_radii[slot] = 65536 / pLight->uRadius;
            Lights._blv_lights_xs[slot] = pLight->vPosition.x;
            Lights._blv_lights_ys[slot] = pLight->vPosition.y;
            Lights._blv_lights_zs[slot] = pLight->vPosition.z;
            Lights._blv_lights_rs[slot] = (double)pLight->uLightColorR / 255.0f;
            Lights._blv_lights_gs[slot] = (double)pLight->uLightColorG / 255.0f;
            Lights._blv_lights_bs[slot] = (double)pLight->uLightColorB / 255.0f;
            Lights._blv_lights_light_dot_faces[slot] =
                abs((int)floorf(Distance + 0.5f));
            Lights._blv_lights_types[slot] = pLight->uLightType;

            *pSlot += 1;
            return true;
        }
    }

    return false;
}

//----- (0045DA56) --------------------------------------------------------
bool LightmapBuilder::DoDraw_183808_Lightmaps(float z_bias) {
    // For indoor light (X)
    Vec3f v;  // [sp+Ch] [bp-1Ch]@2
    v.z = 1.0;
    v.y = 1.0;
    v.x = 1.0;

    for (uint i = 0; i < MobileLightsCount; ++i)
        if (!render->DrawLightmap(&MobileLights[i], &v, z_bias))
            Error("Invalid lightmap detected! (%u)", i);

    return true;
}

//----- (0045C4B9) --------------------------------------------------------
int LightmapBuilder::_45C4B9(int a2, RenderVertexSoft *a3,
                             Lightmap *pLightmap) {
    // For indoor light (?)
    RenderVertexSoft *v9;   // esi@3
    RenderVertexSoft *v12;  // ecx@8
    char v13;               // bl@17
    signed int v14;         // edx@17
    double v15;             // st6@18
    double v16;             // st6@21
    double v17;             // st6@24
    signed int v18;         // edx@33
    float v22;              // [sp+Ch] [bp-14h]@23
    float v23;              // [sp+10h] [bp-10h]@20
    int v24;                // [sp+14h] [bp-Ch]@1
    char v26;               // [sp+1Eh] [bp-2h]@17
    char v27;               // [sp+1Fh] [bp-1h]@17

    v24 = 0;
    for (uint i = 0; i < pLightmap->NumVertices; i++) {
        v9 = &pLightmap->pVertices[(i + 1) % pLightmap->NumVertices];
        if (pLightmap->pVertices[i].vWorldPosition.x != v9->vWorldPosition.x ||
            pLightmap->pVertices[i].vWorldPosition.y != v9->vWorldPosition.y ||
            pLightmap->pVertices[i].vWorldPosition.z != v9->vWorldPosition.z) {
            for (uint j = 0; j < a2; ++j) {
                v12 = &a3[(j + 1) % a2];
                if ((a3[j].vWorldPosition.x != v12->vWorldPosition.x ||
                     a3[j].vWorldPosition.y != v12->vWorldPosition.y ||
                     a3[j].vWorldPosition.z != v12->vWorldPosition.z) &&
                    pLightmap->pVertices[i].vWorldPosition.x ==
                        a3[j].vWorldPosition.x &&
                    pLightmap->pVertices[i].vWorldPosition.y ==
                        a3[j].vWorldPosition.y &&
                    pLightmap->pVertices[i].vWorldPosition.z ==
                        a3[j].vWorldPosition.z &&
                    (v9->vWorldPosition.x != v12->vWorldPosition.x ||
                     v9->vWorldPosition.y != v12->vWorldPosition.y ||
                     v9->vWorldPosition.z != v12->vWorldPosition.z)) {
                    v13 = 0;
                    v14 = 0;
                    v27 = 0;
                    v26 = 0;

                    if (v9->vWorldPosition.x <= (double)v12->vWorldPosition.x)
                        v15 = v12->vWorldPosition.x - v9->vWorldPosition.x;
                    else
                        v15 = v9->vWorldPosition.x - v12->vWorldPosition.x;
                    v23 = v15;

                    if (v9->vWorldPosition.y <= (double)v12->vWorldPosition.y)
                        v16 = v12->vWorldPosition.y - v9->vWorldPosition.y;
                    else
                        v16 = v9->vWorldPosition.y - v12->vWorldPosition.y;
                    v22 = v16;

                    if (v9->vWorldPosition.z <= (double)v12->vWorldPosition.z)
                        v17 = v12->vWorldPosition.z - v9->vWorldPosition.z;
                    else
                        v17 = v9->vWorldPosition.z - v12->vWorldPosition.z;

                    if (v23 < 1.0) {
                        v13 = 1;
                        v14 = 1;
                    }

                    if (v22 < 1.0) {
                        v27 = 1;
                        ++v14;
                    }

                    if (v17 < 1.0) {
                        v26 = 1;
                        ++v14;
                    }

                    if (v14 > 1) {
                        v18 = 0;
                        if (v13 &&
                            v9->vWorldPosition.x != v12->vWorldPosition.x) {
                            v18 = 1;
                            v9->vWorldPosition.x = v12->vWorldPosition.x;
                        }

                        if (v27 &&
                            v9->vWorldPosition.y != v12->vWorldPosition.y) {
                            ++v18;
                            v9->vWorldPosition.y = v12->vWorldPosition.y;
                        }

                        if (v26 &&
                            v9->vWorldPosition.z != v12->vWorldPosition.z) {
                            ++v18;
                            v9->vWorldPosition.z = v12->vWorldPosition.z;
                        }

                        if (v18 > 0) ++v24;
                    }
                }
            }
        }
    }
    return v24;
}

// //////////////////////FOR OUTDOOR
// /TERRAIN//////////////////////////////////////////
// ----- (0045D036) --------------------------------------------------------
bool LightmapBuilder::StackLights_TerrainFace(Vec3f *pNormal,
                                              float *Light_tile_dist,
                                              RenderVertexSoft *VertList,
                                              unsigned int uStripType,
                                              bool bLightBackfaces) {
    // For outdoor terrain light(I)
    unsigned int num_lights = 0;  // [sp+Ch] [bp-4h]@1
    Lights.uCurrentAmbientLightLevel = pOutdoor->max_terrain_dimming_level;

    for (int i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        if (num_lights >= 20) break;
        StackLight_TerrainFace(
            (StationaryLight *)&pMobileLightsStack->pLights[i], pNormal,
            Light_tile_dist, VertList, uStripType, bLightBackfaces, &num_lights);
    }

    for (int i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        if (num_lights >= 20) break;
        StackLight_TerrainFace(&pStationaryLightsStack->pLights[i], pNormal,
                               Light_tile_dist, VertList, uStripType, bLightBackfaces,
                               &num_lights);
    }

    Lights.uNumLightsApplied = num_lights;
    return true;
}

// ///////////////////////TOGETHER///////////////////////////////////////////////////////
// ----- (0045BC07) --------------------------------------------------------
bool LightmapBuilder::ApplyLights(LightsData *pLights, stru154 *FacePlane, unsigned int uNumVertices,
                                  RenderVertexSoft *FaceVertexList, IndoorCameraD3D_Vec4 *a6, char uClipFlag) {
    // a6 - portal frustum ??

    // lightmap_builder->ApplyLights(&Lights, &faceplane, uNumVerticesa, VertsTransformed, pVertices, 0);

    // For outdoor terrain and indoor light (III)(III)
    Vec3i pos;         // [sp+2Ch] [bp-40h]@21
    RenderVertexSoft *a9;  // [sp+68h] [bp-4h]@8

    if (!uNumVertices) return false;

    static RenderVertexSoft static_69B140[64];

    a9 = FaceVertexList;
    if (a6) {
        for (uint i = 0; i < uNumVertices; ++i)
            static_69B140[i] = FaceVertexList[i];

        if (pCamera3D->CullVertsToPlane(FacePlane, static_69B140, &uNumVertices) == 1) {
            if (!uNumVertices) return false;
            a9 = static_69B140;
        }
    }

    static stru314 static_69B110;
    static_69B110.Normal.x = FacePlane->face_plane.vNormal.x;
    static_69B110.Normal.y = FacePlane->face_plane.vNormal.y;
    static_69B110.Normal.z = FacePlane->face_plane.vNormal.z;
    static_69B110.dist = FacePlane->face_plane.dist;
    if (!pCamera3D->GetFacetOrientation(
        FacePlane->polygonType, &static_69B110.Normal, &static_69B110.field_10,
            &static_69B110.field_1C)) {
        log->Warning("Error: Failed to get the facet orientation");
        Engine_DeinitializeAndTerminate(0);
    }

    for (uint i = 0; i < pLights->uNumLightsApplied; ++i) {
        pos.x = pLights->_blv_lights_xs[i];
        pos.y = pLights->_blv_lights_ys[i];
        pos.z = pLights->_blv_lights_zs[i];

        uint uColorR =
                 (uint)floorf(pLights->_blv_lights_rs[i] * 255.0 + 0.5f) & 0xFF,
             uColorG =
                 (uint)floorf(pLights->_blv_lights_gs[i] * 255.0 + 0.5f) & 0xFF,
             uColorB =
                 (uint)floorf(pLights->_blv_lights_bs[i] * 255.0 + 0.5f) & 0xFF;
        uint uColor = (uColorB << 16) | (uColorG << 8) | uColorR;

        if (!uColor) {
            uColor = 0x00FFFFFF;  // 0x00FFFFF;
        }

        if (!_45BE86_build_light_polygon(
                &pos, pLights->_blv_lights_radii[i], uColor,
                pLights->_blv_lights_light_dot_faces[i],
                pLights->_blv_lights_types[i], &static_69B110, uNumVertices, a9,
                uClipFlag)) {
            log->Warning("Error: Failed to build light polygon");
        }
    }
    return true;
}

//----- (0045BE86) --------------------------------------------------------
bool LightmapBuilder::_45BE86_build_light_polygon(Vec3i *pos, float radius, unsigned int uColorMask, float dot_dist,
    int uLightType, stru314 *FacePlaneNormals, unsigned int uNumVertices, RenderVertexSoft *a9, char uClipFlag) {

    // For outdoor terrain and indoor light (IV)(IV)
    Lightmap *lightmap;  // edi@3
                         //  double v17; // st7@5
    double v24;          // st7@6
    double v38;          // st7@14
    double v39;          // st7@16
    double v40;          // st7@16
    int v45;             // eax@24

    if (radius < 0.0f) return true;

    lightmap = uLightType & 1 ? &StationaryLights[StationaryLightsCount]
                              :                       // stationary
                   &MobileLights[MobileLightsCount];  // mobile

    tex_light_radius = radius - dot_dist;
    // flt_3C8C28 = sqrt((radius + radius - tex_light_radius) * tex_light_radius);
    flt_3C8C28 = sqrt(radius * radius - dot_dist * dot_dist);
    flt_3C8C2C_lightmaps_brightness = 1.0 - (radius - flt_3C8C28) / radius;


    lightmap->position_x = (double)pos->x - dot_dist * FacePlaneNormals->Normal.x;
    lightmap->position_y = (double)pos->y - dot_dist * FacePlaneNormals->Normal.y;
    lightmap->position_z = (double)pos->z - dot_dist * FacePlaneNormals->Normal.z;

    light_radius = radius * flt_3C8C2C_lightmaps_brightness;
    light_length_x = light_radius * FacePlaneNormals->field_10.x;
    light_length_y = light_radius * FacePlaneNormals->field_10.y;
    light_length_z = light_radius * FacePlaneNormals->field_10.z;

    light_length_x2 = light_radius * FacePlaneNormals->field_1C.x;
    light_length_y2 = light_radius * FacePlaneNormals->field_1C.y;
    light_length_z2 = light_radius * FacePlaneNormals->field_1C.z;

    lightmap->pVertices[0].vWorldPosition.x =
        lightmap->position_x - light_length_x2 + light_length_x;
    lightmap->pVertices[0].vWorldPosition.y =
        lightmap->position_y - light_length_y2 + light_length_y;
    lightmap->pVertices[0].vWorldPosition.z =
        lightmap->position_z - light_length_z2 + light_length_z;
    lightmap->pVertices[0].u = 0.0;
    lightmap->pVertices[0].v = 0.0;

    lightmap->pVertices[1].vWorldPosition.x =
        lightmap->position_x - light_length_x2 - light_length_x;
    lightmap->pVertices[1].vWorldPosition.y =
        lightmap->position_y - light_length_y2 - light_length_y;
    lightmap->pVertices[1].vWorldPosition.z =
        lightmap->position_z - light_length_z2 - light_length_z;
    lightmap->pVertices[1].u = 0.0;
    lightmap->pVertices[1].v = 1.0;

    lightmap->pVertices[2].vWorldPosition.x =
        lightmap->position_x + light_length_x2 - light_length_x;
    lightmap->pVertices[2].vWorldPosition.y =
        lightmap->position_y + light_length_y2 - light_length_y;
    lightmap->pVertices[2].vWorldPosition.z =
        lightmap->position_z + light_length_z2 - light_length_z;
    lightmap->pVertices[2].u = 1.0;
    lightmap->pVertices[2].v = 1.0;

    lightmap->pVertices[3].vWorldPosition.x =
        lightmap->position_x + light_length_x2 + light_length_x;
    lightmap->pVertices[3].vWorldPosition.y =
        lightmap->position_y + light_length_y2 + light_length_y;
    lightmap->pVertices[3].vWorldPosition.z =
        lightmap->position_z + light_length_z2 + light_length_z;
    lightmap->pVertices[3].u = 1.0;
    lightmap->pVertices[3].v = 0.0;

    for (uint i = 0; i < 4; ++i) {
        v24 = FacePlaneNormals->Normal.y * lightmap->pVertices[i].vWorldPosition.y +
              FacePlaneNormals->Normal.z * lightmap->pVertices[i].vWorldPosition.z +
              FacePlaneNormals->Normal.x * lightmap->pVertices[i].vWorldPosition.x + FacePlaneNormals->dist;

        lightmap->pVertices[i].vWorldPosition.x -= v24 * FacePlaneNormals->Normal.x;
        lightmap->pVertices[i].vWorldPosition.y -= v24 * FacePlaneNormals->Normal.y;
        lightmap->pVertices[i].vWorldPosition.z -= v24 * FacePlaneNormals->Normal.z;
    }

    lightmap->uColorMask = uColorMask;
    lightmap->NumVertices = 4;

    // Brightness(яркость)/////////////////////////////////
    if (!engine->config->graphics.DynamicBrightness.Get()) {
        lightmap->fBrightness = flt_3C8C2C_lightmaps_brightness;
    } else {
        Vec3f a1;  // [sp+2Ch] [bp-20h]@8
        a1.x = (double)pos->x - lightmap->position_x;
        a1.y = (double)pos->y - lightmap->position_y;
        a1.z = (double)pos->z - lightmap->position_z;
        a1.Normalize();

        auto temp1 = lightmap->position_x + 0.5f,
             temp2 = lightmap->position_y + 0.5f,
             temp3 = lightmap->position_z + 0.5f;

        auto dist_x = abs(pos->x - temp1),  // v31
            dist_y = abs(pos->y - temp2),   // v32  arg0a
            dist_z = abs(pos->z - temp3);   // v33  _v64
        v38 = int_get_vector_length(dist_x, dist_y, dist_z);
        if (v38 > radius) return true;
        // radius = (1 / radius) * v38;
        if (uLightType & 4) {  // LIGHT_ATTR_POINT
            v39 = fabs(a1.x * FacePlaneNormals->Normal.x + a1.z * FacePlaneNormals->Normal.z +
                       a1.y * FacePlaneNormals->Normal.y);
            v40 = v39 * 1.0 * flt_4D86CC;

            lightmap->fBrightness = v40 - ((1 / radius) * v38) * v40;
        } else if (uLightType & 8) {  // LIGHT_ATTR_SPOTLIGHT
            v40 = 1.0 * 1.0;
            lightmap->fBrightness = v40 - ((1 / radius) * v38);
        } else {
            log->Warning("Invalid light type!");
        }
    }
    // Brightness(яркость)/////////////////////////////////////////////////////

    // trim to surface
    if (!engine->pStru9Instance->ClipVertsToFace(
            a9, uNumVertices, FacePlaneNormals->Normal.x, FacePlaneNormals->Normal.y, FacePlaneNormals->Normal.z,
            lightmap->pVertices, &lightmap->NumVertices))
        return false;

    if (!lightmap->NumVertices)
        return true;

    v45 = _45C6D6(uNumVertices, a9, lightmap);
    if (v45 != uNumVertices && v45 > 0) _45C4B9(uNumVertices, a9, lightmap);

    pCamera3D->ViewTransform(lightmap->pVertices, lightmap->NumVertices);
    pCamera3D->Project(lightmap->pVertices, lightmap->NumVertices, 0);

    unsigned int _a4 = 1;  //  0;
    if (_a4) {
        if (uLightType & 1) {
            if (StationaryLightsCount < 512 - 1) ++StationaryLightsCount;
        } else {
            if (MobileLightsCount < 768 - 1) ++MobileLightsCount;
        }
        // if ( v50 ^ v51 )
        //  *(unsigned int *)v48 = v49 + 1;
    }
    return true;
}

//----- (0045C6D6) --------------------------------------------------------
int LightmapBuilder::_45C6D6(int uNumVertices, RenderVertexSoft *a3,
                             Lightmap *pLightmap) {
    // For outdoor terrain and indoor light (V)(V)
    signed int v6;  // esi@1
    float temp_x;   // st7@6
    float temp_y;   // st6@10
    float temp_z;   // st5@14
    float v12;      // st7@17
    int v15;        // [sp+Ch] [bp-8h]@1

    v6 = -1;
    v15 = 0;
    if ((signed int)pLightmap->NumVertices > 0) {
        for (uint i = 0; i < (signed int)pLightmap->NumVertices; ++i) {
            if (uNumVertices > 0) {
                for (uint j = 0; j < uNumVertices; ++j) {
                    if (pLightmap->pVertices[i].vWorldPosition.x <=
                        (double)a3[j].vWorldPosition.x)
                        temp_x = a3[j].vWorldPosition.x -
                                 pLightmap->pVertices[i].vWorldPosition.x;
                    else
                        temp_x = pLightmap->pVertices[i].vWorldPosition.x -
                                 a3[j].vWorldPosition.x;
                    if (temp_x < 2.0) {
                        if (pLightmap->pVertices[i].vWorldPosition.y <=
                            (double)a3[j].vWorldPosition.y)
                            temp_y = a3[j].vWorldPosition.y -
                                     pLightmap->pVertices[i].vWorldPosition.y;
                        else
                            temp_y = pLightmap->pVertices[i].vWorldPosition.y -
                                     a3[j].vWorldPosition.y;
                        if (temp_y < 2.0) {
                            if (pLightmap->pVertices[i].vWorldPosition.z <=
                                (double)a3[j].vWorldPosition.z)
                                temp_z =
                                    a3[j].vWorldPosition.z -
                                    pLightmap->pVertices[i].vWorldPosition.z;
                            else
                                temp_z =
                                    pLightmap->pVertices[i].vWorldPosition.z -
                                    a3[j].vWorldPosition.z;
                            if (temp_z < 2.0) {
                                v12 = temp_x + temp_y + temp_z;
                                if (v12 < FLT_MAX) v6 = j;
                            }
                        }
                    }
                }  // For
                if (v6 != -1) {
                    ++v15;
                    pLightmap->pVertices[i].vWorldPosition.x =
                        a3[v6].vWorldPosition.x;
                    pLightmap->pVertices[i].vWorldPosition.y =
                        a3[v6].vWorldPosition.y;
                    pLightmap->pVertices[i].vWorldPosition.z =
                        a3[v6].vWorldPosition.z;
                }
            }
            v6 = -1;
        }  // For
    }
    return v15;
}

//----- (0045D73F) --------------------------------------------------------
void LightmapBuilder::DrawLightmapsType(int type) {
    // For outdoor and indoor light (II)(VIII)
    if (type == 2) Draw_183808_Lightmaps();
}

//----- (0045D74F) --------------------------------------------------------
void LightmapBuilder::DrawLightmaps(int indices) {
    // For outdoor terrain and indoor light (VI)(VI)

    if (StationaryLightsCount > 0) {
        if (false) {
            __debugbreak();
            return;
        }


        render->BeginLightmaps();

        Vec3f arg4;
        arg4.x = 1.0f;
        arg4.y = 1.0f;
        arg4.z = 1.0f;

        if (indices != -1) {
            for (unsigned int i = 0; i < MobileLightsCount; ++i) {
                if (!render->DrawLightmap(&MobileLights[i], &arg4, 0.0))
                    Error("Invalid lightmap detected! (%u)", i);
            }
        } else {
            for (unsigned int i = 0; i < StationaryLightsCount; ++i)
                if (!render->DrawLightmap(&StationaryLights[i], &arg4, 0.0))
                    Error("Invalid lightmap detected! (%u)", i);
        }

        render->EndLightmaps();
    }
}

//----- (0045DCA9) --------------------------------------------------------
void LightmapBuilder::Draw_183808_Lightmaps() {
    // For outdoor and indoor light (III)(IX)
    if (!MobileLightsCount) return;

    render->BeginLightmaps2();

    DoDraw_183808_Lightmaps(0.00050000002);

    render->EndLightmaps2();
}

// //////////////////////OTHER////////////////////////////////////////////////////////
// ----- (0045CA88) --------------------------------------------------------
int *LightmapBuilder::_45CA88(LightsData *a2, RenderVertexSoft *a3, int a4,
                              Vec3f *pNormal) {
    int *result;            // eax@1
    LightsData *v6;        // ecx@2
    RenderVertexSoft *v7;  // ebx@2
    double v8;             // st7@2
    double v9;             // st6@2
    char *v10;             // eax@3
    double v11;            // st7@5
    int64_t v12 {};           // ST2C_8@5
    float v13 {};             // edx@5
    int v14;               // eax@5
    float v15;             // ST10_4@5
    Vec3f v16;       // ST00_12@5
    double v17;            // st7@5
    int a5 {};                // [sp+2Ch] [bp-1Ch]@1
    float v19;             // [sp+30h] [bp-18h]@1
    float v20;             // [sp+34h] [bp-14h]@1
    // LightmapBuilder *thisa; // [sp+38h] [bp-10h]@1
    int v22;  // [sp+3Ch] [bp-Ch]@1
    int *j;   // [sp+40h] [bp-8h]@3
    int i;    // [sp+44h] [bp-4h]@1
    int a3a;  // [sp+58h] [bp+10h]@2

    __debugbreak();  // Not used?

    *(float *)&a5 = 0.0;
    v19 = 0.0;
    // thisa = this;
    v20 = 0.0;
    result = _45CBD4(a3, a4, dword_69B010.data(), &v22);
    for (i = 0; i < v22; result = (int *)i) {
        v6 = a2;
        a3a = 0;
        v7 = &a3[dword_69B010[i]];
        v8 = v7->vWorldPosition.z;
        v9 = v7->vWorldPosition.y;
        *(float *)&a5 = v7->vWorldPosition.x;
        v19 = v9;
        v20 = v8;
        v7->flt_2C = 0.0;
        if (a2->uNumLightsApplied > 0) {
            v10 = (char *)a2->_blv_lights_ys;
            for (j = a2->_blv_lights_ys;; v10 = (char *)j) {
                v11 = (double)*((signed int *)v10 - 60);
                HEXRAYS_LODWORD(v12) = *((unsigned int *)v10 - 20);
                HEXRAYS_HIDWORD(v12) = *(unsigned int *)v10;
                HEXRAYS_LODWORD(v13) = *((unsigned int *)v10 + 60);
                v14 = a3a;
                HEXRAYS_LOBYTE(v14) = v6->_blv_lights_types[a3a];
                v15 = v11;
                *(_QWORD *)&v16.x = v12;
                v16.z = v13;
                v17 = _45CC0C_light(v16 /*COERCE_VEC3_FLOAT_(v16.x)*/, 1.0, v15,
                                    pNormal,
                                    *(float *)&a5 /*COERCE_FLOAT(&a5)*/, v14) +
                      v7->flt_2C;
                ++a3a;
                ++j;
                v7->flt_2C = v17;
                if (a3a >= a2->uNumLightsApplied) break;
                v6 = a2;
            }
        }
        ++i;
    }
    return result;
}

//----- (0045CB89) --------------------------------------------------------
int LightmapBuilder::_45CB89(RenderVertexSoft *a1, int a2) {
    int v3;       // edx@1
    int result {};   // eax@2
    char *v5;     // ecx@2
    double v6;    // st7@4
    int16_t v7 = 0;   // fps@4
    char v8;      // c0@4
    char v9;      // c2@4
    char v10;     // c3@4
    double v11;   // st7@5
    double v12;   // st7@6
    int16_t v13 = 0;  // fps@6
    char v14;     // c0@6
    char v15;     // c2@6
    char v16;     // c3@6

    __debugbreak();  // Not used?
    v3 = a2;
    if (a2 > 0) {
        HEXRAYS_HIWORD(result) = HEXRAYS_HIWORD(a1);
        v5 = (char *)&a1->flt_2C;
        do {
            __debugbreak();  // warning C4700: uninitialized local variable 'v7'
                             // used
            if (*(float *)v5 < 0.0 ||
                (v6 = *(float *)v5, /*UNDEF(v7),*/ v8 = 1.0 < v6, v9 = 0,
                 v10 = 1.0 == v6, HEXRAYS_LOWORD(result) = v7, v6 <= 1.0)) {
                v12 = *(float *)v5;
                // UNDEF(v13);
                v14 = 0.0 < v12;
                v15 = 0;
                v16 = 0.0 == v12;
                __debugbreak();  // warning C4700: uninitialized local variable
                                 // 'v13' used
                HEXRAYS_LOWORD(result) = v13;
                if (v12 >= 0.0)
                    v11 = *(float *)v5;
                else
                    v11 = 0.0;
            } else {
                v11 = 1.0;
            }
            *(float *)v5 = v11;
            v5 += 48;
            --v3;
        } while (v3);
    }
    return result;
}

//----- (0045CBD4) --------------------------------------------------------
int *LightmapBuilder::_45CBD4(RenderVertexSoft *a2, int a3, int *a4, int *a5) {
    int *result;  // eax@1
    int v6;      // edx@1
    int v7;      // ecx@2
    int v8;      // esi@2

    __debugbreak();  // Not used?
    result = a5;
    v6 = 0;
    for (*a5 = 0; v6 < a3; ++v6) {
        v7 = *a5;
        v8 = 0;
        if (*a5 <= 0) {
            // LABEL_5:
            a4[v7] = v6;
            ++*a5;
        } else {
            while (a4[v8] != v6) {
                ++v8;
                if (v8 >= v7) {
                    a4[v7] = v6;
                    ++*a5;
                    break;
                    // goto LABEL_5;
                }
            }
        }
    }
    return result;
}

//----- (0045CC0C) --------------------------------------------------------
double LightmapBuilder::_45CC0C_light(Vec3f a1, float a2, float a3,
                                      Vec3f *pNormal, float a5,
                                      int uLightType) {
    float v7;            // esi@1
    int v8;              // eax@1
    double v14;          // st7@7
    double result;       // st7@8
    double v16;          // st7@9
    int v17;             // esi@9
    const char *v18 = nullptr;     // ecx@9
    double v19;          // st7@10
    double v20;          // st7@10
    std::string v21;     // [sp-10h] [bp-40h]@13
    std::array<const char *, 6> v22 {};  // [sp+0h] [bp-30h]@10
    double v23 {};          // [sp+18h] [bp-18h]@1
                         //  double v24; // [sp+20h] [bp-10h]@1
    int v25;             // [sp+28h] [bp-8h]@1
    int v26;             // [sp+2Ch] [bp-4h]@1

    __debugbreak();  // Not used?
    v7 = a5;
    HEXRAYS_LODWORD(a5) = *(unsigned int *)(HEXRAYS_LODWORD(a5) + 8);
    // v24 = a5 + 6.7553994e15;
    v26 = floorf(a5 + 0.5f);  // LODWORD(v24);
    HEXRAYS_LODWORD(a5) = *(unsigned int *)(HEXRAYS_LODWORD(v7) + 4);
    // v24 = a5 + 6.7553994e15;
    auto _v24 = floorf(a5 + 0.5f);
    HEXRAYS_LODWORD(a5) = *(unsigned int *)HEXRAYS_LODWORD(v7);
    // v23 = a5 + 6.7553994e15;
    auto _v23 = floorf(a5 + 0.5f);
    // *(_QWORD *)((char *)&v24 + 4) = __PAIR__(LODWORD(v24), LODWORD(v23));
    v26 = abs((signed)HEXRAYS_LODWORD(a1.z) - v26);
    // v25 = abs((signed)LODWORD(a1.y) - (signed)LODWORD(v24));
    // v8 = abs((int)a1.x - (signed)LODWORD(v23));
    v25 = abs((signed)HEXRAYS_LODWORD(a1.y) - (signed)_v24);
    v8 = abs((int)a1.x - (signed)_v23);
    v14 = int_get_vector_length(v26, v25, v8);
    if (v14 <= a3) {
        a5 = v14 / a3;
        v16 = (double)(signed int)a1.x;
        *(float *)&v23 = (double)HEXRAYS_SLODWORD(a1.y);
        HEXRAYS_LODWORD(a1.x) = *(unsigned int *)HEXRAYS_LODWORD(v7);
        v17 = HEXRAYS_LODWORD(v7) + 4;
        *((float *)&v23 + 1) = (double)HEXRAYS_SLODWORD(a1.z);
        HEXRAYS_LODWORD(a1.y) = *(unsigned int *)v17;
        HEXRAYS_LODWORD(a1.z) = *(unsigned int *)(v17 + 4);
        a3 = *((float *)&v23 + 1) - a1.z;
        a1.z = a3;
        a1.x = v16 - a1.x;
        a1.y = (float)v23 - a1.y;
        a1.Normalize();
        if (uLightType & 4) {
            __debugbreak();  // warning C4700: uninitialized local variable
                             // 'v18' used
            v22[1] = v18;
            uLightType = dword_4D86D8;
            v22[0] = v18;
            v19 =
                fabs(a1.z * pNormal->z + a1.y * pNormal->y + a1.x * pNormal->x);
            v20 = v19 * *(float *)&uLightType * a2;
        } else {
            if (uLightType & 8) {
                v20 = 1.3 * a2;
            } else {
                log->Warning("Invalid light type detected!");
                v20 = *(float *)&uLightType;
            }
        }
        result = v20 - a5 * v20;
    } else {
        result = 0.0;
    }
    return result;
}

//----- (0045D698) --------------------------------------------------------
void LightmapBuilder::DrawDebugOutlines(
    char bit_one_for_list1__bit_two_for_list2) {
    if (bit_one_for_list1__bit_two_for_list2 & 1) {
        for (int i = 0; i < this->StationaryLightsCount; ++i)
            pCamera3D->debug_outline_sw(
                this->StationaryLights[i].pVertices,
                this->StationaryLights[i].NumVertices, colorTable.Red.C32(), 0.0f);
    }
    if (bit_one_for_list1__bit_two_for_list2 & 2) {
        for (uint i = 0; i < this->MobileLightsCount; ++i)
            pCamera3D->debug_outline_sw(
                this->MobileLights[i].pVertices,
                this->MobileLights[i].NumVertices, colorTable.Mahogany.C32(), 0.00019999999f);
    }
}

/**
 * @todo(pskelton) Remove this from render classes
 *
 * @param max_dimm                      Maximum dimming level allowed (0-31). 31 * 8 ~ 255.
 * @param min_dimm                      Minimum dimming level allowed (0-31).
 * @param distance                      Depth distance to billboard.
 * @param bNoLight                      No light dimming.
 * @param pBillboard                    Pointer to the billboard to test against or Nullptr.
 *
 * @return                              Return 32bit colour ARGB.
 */
int GetActorTintColor(int max_dimm, int min_dimm, float distance, int bNoLight, RenderBillboard *pBillboard) {
    int dimminglevel = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        return 8 * (31 - max_dimm) | ((8 * (31 - max_dimm) | ((31 - max_dimm) << 11)) << 8);

    if (pParty->armageddon_timer) return 0xFFFF0000;

    bool isNight = pWeather->bNight;
    if (engine->IsUnderwater())
        isNight = false;

    if (isNight) {
        dimminglevel = 216;
        if (pBillboard) dimminglevel = 8 * _43F55F_get_billboard_light_level(pBillboard, dimminglevel >> 3);
        dimminglevel = std::clamp(dimminglevel, 0, 216);
        return (255 - dimminglevel) | ((255 - dimminglevel) << 16) | ((255 - dimminglevel) << 8);
    }

    // daytime
    if (fabsf(distance) < 1.0e-6f) return 0xFFF8F8F8;

    // dim in measured in 8-steps
    int rangewidth = 8 * (max_dimm - min_dimm);
    rangewidth = std::clamp(rangewidth, 0, 216);

    float fog_density_mult = 216.0f;
    if (bNoLight)
        fog_density_mult += distance / pODMRenderParams->shading_dist_shade * 32.0f;

    dimminglevel = static_cast<int>(rangewidth + floorf(pOutdoor->fFogDensity * fog_density_mult + 0.5f));

    if (pBillboard) dimminglevel = 8 * _43F55F_get_billboard_light_level(pBillboard, dimminglevel >> 3);
    dimminglevel = std::clamp(dimminglevel, rangewidth, 216);
    if (dimminglevel > 8 * pOutdoor->max_terrain_dimming_level)
        dimminglevel = 8 * pOutdoor->max_terrain_dimming_level;

    if (!engine->IsUnderwater()) {
        return (255 - dimminglevel) | ((255 - dimminglevel) << 16) | ((255 - dimminglevel) << 8);
    } else {
        // underwater
        float col = (255 - dimminglevel) * 0.0039215689f;
        int red = static_cast<int>(floorf(col * 16.0f + 0.5f));
        int grn = static_cast<int>(floorf(col * 194.0f + 0.5f));
        int blue = static_cast<int>(floorf(col * 153.0f + 0.5f));
        return red | (grn << 8) | (blue << 16);
    }
}

/**
 * @offset 0x0043F5C8.
 *
 * @param uBaseLightLevel               Starting dimming level at point (0-31).
 * @param uSectorID                     Sector ID if indoors or 0.
 * @param x, y, z                       Co-ords of point.
 *
 * @return                              Dimming level (0-31) with lights effect added.
 */
int GetLightLevelAtPoint(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z) {
    int lightlevel = uBaseLightLevel;
    float light_radius{};
    float distX{};
    float distY{};
    float distZ{};
    unsigned int approx_distance;

    // mobile lights
    for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        MobileLight *p = &pMobileLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = abs(p->vPosition.z - z);
                if (distZ <= light_radius) {
                    approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                    if (approx_distance < light_radius)
                        //* ORIGONAL */lightlevel += ((uint64_t)(30i64 *(signed int)(approx_distance << 16) / light_radius) >> 16) - 30;
                        lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                }
            }
        }
    }

    // sector lights
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        BLVSector *pSector = &pIndoor->pSectors[uSectorID];

        for (uint i = 0; i < pSector->uNumLights; ++i) {
            BLVLightMM7 *this_light = &pIndoor->pLights[pSector->pLights[i]];
            light_radius = this_light->uRadius;

            if (~this_light->uAtributes & 8) {
                distX = abs(this_light->vPosition.x - x);
                if (distX <= light_radius) {
                    distY = abs(this_light->vPosition.y - y);
                    if (distY <= light_radius) {
                        distZ = abs(this_light->vPosition.z - z);
                        if (distZ <= light_radius) {
                            approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                            if (approx_distance < light_radius)
                                lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                        }
                    }
                }
            }
        }
    }

    // stationary lights
    for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        StationaryLight *p = &pStationaryLightsStack->pLights[i];
        light_radius = p->uRadius;

        distX = abs(p->vPosition.x - x);
        if (distX <= light_radius) {
            distY = abs(p->vPosition.y - y);
            if (distY <= light_radius) {
                distZ = abs(p->vPosition.z - z);
                if (distZ <= light_radius) {
                    approx_distance = int_get_vector_length(static_cast<int>(distX), static_cast<int>(distY), static_cast<int>(distZ));
                    if (approx_distance < light_radius)
                        lightlevel += static_cast<int> (30 * approx_distance / light_radius) - 30;
                }
            }
        }
    }

    lightlevel = std::clamp(lightlevel, 0, 31);
    return lightlevel;
}

/**
 * @offset 0x0043F55F.
 *
 * @param *a1                           Pointer to billboard to check.
 * @param uBaseLightLevel               Base light dimming value (0-31).
 *
 * @return                              Billboard dimming value (0-31) with lights applied.
 */
int _43F55F_get_billboard_light_level(RenderBillboard *a1, int uBaseLightLevel) {
    int v3 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        v3 = pIndoor->pSectors[a1->uIndoorSectorID].uMinAmbientLightLevel;
    } else {
        if (uBaseLightLevel == -1) {
            v3 = a1->dimming_level;
        } else {
            v3 = uBaseLightLevel;
        }
    }

    return GetLightLevelAtPoint(v3, a1->uIndoorSectorID, a1->world_x, a1->world_y, a1->world_z);
}
