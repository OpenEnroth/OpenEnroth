#ifdef _WINDOWS
    #pragma comment(lib, "opengl32.lib")
    #pragma comment(lib, "glu32.lib")

    //  on windows, this is required in gl/glu.h
    #if !defined(APIENTRY)
        #define APIENTRY __stdcall
    #endif

    #if !defined(WINGDIAPI)
        #define WINGDIAPI
    #endif

    #if !defined(CALLBACK)
        #define CALLBACK __stdcall
    #endif
#endif

#include "glad/gl.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <algorithm>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"
#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Arcomage/Arcomage.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"

#ifndef LOWORD
    #define LOWORD(l) ((unsigned short)(((std::uintptr_t)(l)) & 0xFFFF))
#endif


RenderVertexSoft VertexRenderList[50];  // array_50AC10
RenderVertexD3D3 d3d_vertex_buffer[50];

void Polygon::_normalize_v_18() {
    float len = sqrt((double)this->v_18.z * (double)this->v_18.z +
                     (double)this->v_18.y * (double)this->v_18.y +
                     (double)this->v_18.x * (double)this->v_18.x);
    if (fabsf(len) < 1e-6f) {
        v_18.x = 0;
        v_18.y = 0;
        v_18.z = 65536;
    } else {
        v_18.x = round_to_int((double)this->v_18.x / len * 65536.0);
        v_18.y = round_to_int((double)this->v_18.y / len * 65536.0);
        v_18.z = round_to_int((double)this->v_18.z / len * 65536.0);
    }
}

bool IsBModelVisible(BSPModel *model, int reachable_depth, bool *reachable) {
    // checks if model is visible in FOV cone
    float halfangle = (pCamera3D->odm_fov_rad) / 2.0;
    float rayx = model->vBoundingCenter.x - pCamera3D->vCameraPos.x;
    float rayy = model->vBoundingCenter.y - pCamera3D->vCameraPos.y;

    // approx distance
    int dist = int_get_vector_length(abs(rayx), abs(rayy), 0);
    *reachable = false;
    if (dist < model->sBoundingRadius + reachable_depth) *reachable = true;

    // dot product of camvec and ray - size in forward
    float frontvec = rayx * pCamera3D->fRotationZCosine + rayy * pCamera3D->fRotationZSine;
    if (pCamera3D->sRotationY) { frontvec *= pCamera3D->fRotationYCosine;}

    // dot product of camvec and ray - size in left
    float leftvec = rayy * pCamera3D->fRotationZCosine - rayx * pCamera3D->fRotationZSine;

    // which half fov is ray in direction of - compare slopes
    float sloperem = 0.0;
    if (leftvec >= 0) {  // acute - left
        sloperem = frontvec * sin(halfangle) - leftvec * cos(halfangle);
    } else {  // obtuse - right
        sloperem = frontvec * sin(halfangle) + leftvec * cos(halfangle);
    }

    // view range check
    if (dist <= pCamera3D->GetFarClip() + 2048) {
        // boudning point inside cone
        if (sloperem >= 0) return true;
        // bounding radius inside cone
        if (abs(sloperem) < model->sBoundingRadius + 512) return true;
    }

    // not visible
    return false;
}

int GetActorTintColor(int max_dimm, int min_dimm, float distance, int a4, RenderBillboard *a5) {
    signed int v6;   // edx@1
    int v8;          // eax@3
    double v9;       // st7@12
    int v11;         // ecx@28
    double v15;      // st7@44
    int v18;         // ST14_4@44
    signed int v20;  // [sp+10h] [bp-4h]@10
    float a3c;       // [sp+1Ch] [bp+8h]@44
    int a5a;         // [sp+24h] [bp+10h]@44

    // v5 = a2;
    v6 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        return 8 * (31 - max_dimm) | ((8 * (31 - max_dimm) | ((31 - max_dimm) << 11)) << 8);

    if (pParty->armageddon_timer) return 0xFFFF0000;

    v8 = pWeather->bNight;
    if (engine->IsUnderwater())
        v8 = 0;
    if (v8) {
        v20 = 1;
        if (pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].Active())
            v20 = pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;
        v9 = (double)v20 * 1024.0;
        if (a4) {
            v6 = 216;
            goto LABEL_20;
        }
        if (distance <= v9) {
            if (distance > 0.0) {
                // a4b = distance * 216.0 / device_caps;
                // v10 = a4b + 6.7553994e15;
                // v6 = LODWORD(v10);
                v6 = floorf(0.5f + distance * 216.0 / v9);
                if (v6 > 216) {
                    v6 = 216;
                    goto LABEL_20;
                }
            }
        } else {
            v6 = 216;
        }
        if (distance != 0.0) {
        LABEL_20:
            if (a5) v6 = 8 * _43F55F_get_billboard_light_level(a5, v6 >> 3);
            if (v6 > 216) v6 = 216;
            return (255 - v6) | ((255 - v6) << 16) | ((255 - v6) << 8);
        }
        // LABEL_19:
        v6 = 216;
        goto LABEL_20;
    }

    if (fabsf(distance) < 1.0e-6f) return 0xFFF8F8F8;

    // dim in measured in 8-steps
    v11 = 8 * (max_dimm - min_dimm);
    // v12 = v11;
    if (v11 >= 0) {
        if (v11 > 216) v11 = 216;
    } else {
        v11 = 0;
    }

    float fog_density_mult = 216.0f;
    if (a4)
        fog_density_mult +=
            distance / (double)pODMRenderParams->shading_dist_shade * 32.0;

    v6 = v11 + floorf(pOutdoor->fFogDensity * fog_density_mult + 0.5f);

    if (a5) v6 = 8 * _43F55F_get_billboard_light_level(a5, v6 >> 3);
    if (v6 > 216) v6 = 216;
    if (v6 < v11) v6 = v11;
    if (v6 > 8 * pOutdoor->max_terrain_dimming_level)
        v6 = 8 * pOutdoor->max_terrain_dimming_level;
    if (!engine->IsUnderwater()) {
        return (255 - v6) | ((255 - v6) << 16) | ((255 - v6) << 8);
    } else {
        v15 = (double)(255 - v6) * 0.0039215689;
        a3c = v15;
        // a4c = v15 * 16.0;
        // v16 = a4c + 6.7553994e15;
        a5a = floorf(v15 * 16.0 + 0.5f);  // LODWORD(v16);
                                          // a4d = a3c * 194.0;
                                          // v17 = a4d + 6.7553994e15;
        v18 = floorf(a3c * 194.0 + 0.5f);  // LODWORD(v17);
                                           // a3d = a3c * 153.0;
                                           // v19 = a3d + 6.7553994e15;
        return (int)floorf(a3c * 153.0 + 0.5f) /*LODWORD(v19)*/ |
               ((v18 | (a5a << 8)) << 8);
    }
}

std::shared_ptr<IRender> render;
int uNumDecorationsDrawnThisFrame;
RenderBillboard pBillboardRenderList[500];
unsigned int uNumBillboardsToDraw;
int uNumSpritesDrawnThisFrame;

RenderVertexSoft array_73D150[20];

void RenderOpenGL::MaskGameViewport() {
    // do not want in opengl mode
}

int _43F5C8_get_point_light_level_with_respect_to_lights(unsigned int uBaseLightLevel, int uSectorID, float x, float y, float z) {
    signed int v6;     // edi@1
    int v8;            // eax@6
    int v9;            // ebx@6
    unsigned int v10;  // ecx@6
    unsigned int v11;  // edx@9
    unsigned int v12;  // edx@11
    signed int v13;    // ecx@12
    BLVLightMM7 *v16;  // esi@20
    int v17;           // ebx@21
    signed int v24;    // ecx@30
    int v26;           // ebx@35
    int v37;           // [sp+Ch] [bp-18h]@37
    int v39;           // [sp+10h] [bp-14h]@23
    int v40;           // [sp+10h] [bp-14h]@36
    int v42;           // [sp+14h] [bp-10h]@22
    unsigned int v43;  // [sp+18h] [bp-Ch]@12
    unsigned int v44;  // [sp+18h] [bp-Ch]@30
    unsigned int v45;  // [sp+18h] [bp-Ch]@44

    v6 = uBaseLightLevel;

    for (uint i = 0; i < pMobileLightsStack->uNumLightsActive; ++i) {
        MobileLight *p = &pMobileLightsStack->pLights[i];

        float distX = abs(p->vPosition.x - x);
        if (distX <= p->uRadius) {
            float distY = abs(p->vPosition.y - y);
            if (distY <= p->uRadius) {
                float distZ = abs(p->vPosition.z - z);
                if (distZ <= p->uRadius) {
                    v8 = distX;
                    v9 = distY;
                    v10 = distZ;
                    if (distX < distY) {
                        v8 = distY;
                        v9 = distX;
                    }
                    if (v8 < distZ) {
                        v11 = v8;
                        v8 = distZ;
                        v10 = v11;
                    }
                    if (v9 < (signed int)v10) {
                        v12 = v10;
                        v10 = v9;
                        v9 = v12;
                    }
                    v43 = ((unsigned int)(11 * v9) / 32) + (v10 / 4) + v8;
                    v13 = p->uRadius;
                    if ((signed int)v43 < v13)
         //* ORIGONAL */v6 += ((unsigned __int64)(30i64 *(signed int)(v43 << 16) / v13) >> 16) - 30;
                        v6 += ((unsigned __int64)(30ll * (signed int)(v43 << 16) / v13) >> 16) - 30;
                }
            }
        }
    }

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        BLVSector *pSector = &pIndoor->pSectors[uSectorID];

        for (uint i = 0; i < pSector->uNumLights; ++i) {
            v16 = pIndoor->pLights + pSector->pLights[i];
            if (~v16->uAtributes & 8) {
                v17 = abs(v16->vPosition.x - x);
                if (v17 <= v16->uRadius) {
                    v42 = abs(v16->vPosition.y - y);
                    if (v42 <= v16->uRadius) {
                        v39 = abs(v16->vPosition.z - z);
                        if (v39 <= v16->uRadius) {
                            v44 = int_get_vector_length(v17, v42, v39);
                            v24 = v16->uRadius;
                            if ((signed int)v44 < v24)
                                v6 += ((unsigned __int64)(30ll * (signed int)(v44 << 16) / v24) >> 16) - 30;
                        }
                    }
                }
            }
        }
    }

    for (uint i = 0; i < pStationaryLightsStack->uNumLightsActive; ++i) {
        // StationaryLight* p = &pStationaryLightsStack->pLights[i];
        v26 = abs(pStationaryLightsStack->pLights[i].vPosition.x - x);
        if (v26 <= pStationaryLightsStack->pLights[i].uRadius) {
            v40 = abs(pStationaryLightsStack->pLights[i].vPosition.y - y);
            if (v40 <= pStationaryLightsStack->pLights[i].uRadius) {
                v37 = abs(pStationaryLightsStack->pLights[i].vPosition.z - z);
                if (v37 <= pStationaryLightsStack->pLights[i].uRadius) {
                    v45 = int_get_vector_length(v26, v40, v37);
                    // v33 = pStationaryLightsStack->pLights[i].uRadius;
                    if ((signed int)v45 <
                        pStationaryLightsStack->pLights[i].uRadius)
                        v6 += ((unsigned __int64)(30ll * (signed int)(v45 << 16) / pStationaryLightsStack->pLights[i].uRadius) >> 16) - 30;
                }
            }
        }
    }

    v6 = std::clamp(v6, 0, 31);
    return v6;
}


int collide_against_floor(const Vec3_int_ &pos, unsigned int *pSectorID, unsigned int *pFaceID) {
    uint uFaceID = -1;
    int floor_z = BLV_GetFloorLevel(pos, *pSectorID, &uFaceID);

    if (floor_z != -30000 && floor_z <= pos.z + 50) {
        *pFaceID = uFaceID;
        return floor_z;
    }

    uint uSectorID = pIndoor->GetSector(pos);
    *pSectorID = uSectorID;

    floor_z = BLV_GetFloorLevel(pos, uSectorID, &uFaceID);
    if (uSectorID && floor_z != -30000)
        *pFaceID = uFaceID;
    else
        return -30000;
    return floor_z;
}

void UpdateObjects() {
    int v5;   // ecx@6
    int v7;   // eax@9
    int v11;  // eax@17
    int v12;  // edi@27
    int v18;  // [sp+4h] [bp-10h]@27
    int v19;  // [sp+8h] [bp-Ch]@27

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uAttributes & OBJECT_40) {
            pSpriteObjects[i].uAttributes &= ~OBJECT_40;
        } else {
            ObjectDesc *object =
                &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
            if (pSpriteObjects[i].AttachedToActor()) {
                v5 = PID_ID(pSpriteObjects[i].spell_target_pid);
                pSpriteObjects[i].vPosition.x = pActors[v5].vPosition.x;
                pSpriteObjects[i].vPosition.y = pActors[v5].vPosition.y;
                pSpriteObjects[i].vPosition.z =
                    pActors[v5].vPosition.z + pActors[v5].uActorHeight;
                if (!pSpriteObjects[i].uObjectDescID) continue;
                pSpriteObjects[i].uSpriteFrameID += pEventTimer->uTimeElapsed;
                if (!(object->uFlags & OBJECT_DESC_TEMPORARY)) continue;
                if (pSpriteObjects[i].uSpriteFrameID >= 0) {
                    v7 = object->uLifetime;
                    if (pSpriteObjects[i].uAttributes & ITEM_BROKEN)
                        v7 = pSpriteObjects[i].field_20;
                    if (pSpriteObjects[i].uSpriteFrameID < v7) continue;
                }
                SpriteObject::OnInteraction(i);
                continue;
            }
            if (pSpriteObjects[i].uObjectDescID) {
                pSpriteObjects[i].uSpriteFrameID += pEventTimer->uTimeElapsed;
                if (object->uFlags & OBJECT_DESC_TEMPORARY) {
                    if (pSpriteObjects[i].uSpriteFrameID < 0) {
                        SpriteObject::OnInteraction(i);
                        continue;
                    }
                    v11 = object->uLifetime;
                    if (pSpriteObjects[i].uAttributes & ITEM_BROKEN)
                        v11 = pSpriteObjects[i].field_20;
                }
                if (!(object->uFlags & OBJECT_DESC_TEMPORARY) ||
                    pSpriteObjects[i].uSpriteFrameID < v11) {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                        SpriteObject::UpdateObject_fn0_BLV(i);
                    else
                        SpriteObject::UpdateObject_fn0_ODM(i);
                    if (!pParty->bTurnBasedModeOn || !(pSpriteObjects[i].uSectorID & 4)) {
                        continue;
                    }
                    v12 = abs(pParty->vPosition.x -
                              pSpriteObjects[i].vPosition.x);
                    v18 = abs(pParty->vPosition.y -
                              pSpriteObjects[i].vPosition.y);
                    v19 = abs(pParty->vPosition.z -
                              pSpriteObjects[i].vPosition.z);
                    if (int_get_vector_length(v12, v18, v19) <= 5120) continue;
                    SpriteObject::OnInteraction(i);
                    continue;
                }
                if (!(object->uFlags & OBJECT_DESC_INTERACTABLE)) {
                    SpriteObject::OnInteraction(i);
                    continue;
                }
                _46BFFA_update_spell_fx(i, PID(OBJECT_Item, i));
            }
        }
    }
}

int _43F55F_get_billboard_light_level(RenderBillboard *a1,
                                      int uBaseLightLevel) {
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

    return _43F5C8_get_point_light_level_with_respect_to_lights(
        v3, a1->uIndoorSectorID, a1->world_x, a1->world_y, a1->world_z);
}

unsigned int sub_46DEF2(signed int a2, unsigned int uLayingItemID) {
    unsigned int result = uLayingItemID;
    if (pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID].uFlags & 0x10) {
        result = _46BFFA_update_spell_fx(uLayingItemID, a2);
    }
    return result;
}

RenderVertexSoft array_507D30[50];

// sky billboard stuff

void SkyBillboardStruct::CalcSkyFrustumVec(int x1, int y1, int z1, int x2, int y2, int z2) {
    // 6 0 0 0 6 0

    //<< 16

    // transform to odd axis??


    float cosz = pCamera3D->fRotationZCosine;  // int_cosine_Z;
    float cosx = pCamera3D->fRotationYCosine;  // int_cosine_y;
    float sinz = pCamera3D->fRotationZSine;  // int_sine_Z;
    float sinx = pCamera3D->fRotationYSine;  // int_sine_y;

    // positions all minus ?
    float v11 = cosz * -pCamera3D->vCameraPos.x + sinz * -pCamera3D->vCameraPos.y;
    float v24 = cosz * -pCamera3D->vCameraPos.y - sinz * -pCamera3D->vCameraPos.x;

    // cam position transform
    if (pCamera3D->sRotationY) {
        this->field_0_party_dir_x = (v11 * cosx) + (-pCamera3D->vCameraPos.z * sinx);
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z * cosx) /*-*/ + (v11 * sinx);
    } else {
        this->field_0_party_dir_x = v11;
        this->field_4_party_dir_y = v24;
        this->field_8_party_dir_z = (-pCamera3D->vCameraPos.z);
    }

    // set 1 position transfrom (6 0 0) looks like cam left vector
    if (pCamera3D->sRotationY) {
        float v17 = (x1 * cosz) + (y1 * sinz);

        this->CamVecLeft_Z = (v17 * cosx) + (z1 * sinx);  // dz
        this->CamVecLeft_X = (y1 * cosz) - (x1 * sinz);  // dx
        this->CamVecLeft_Y = (z1 * cosx) /*-*/ + (v17 * sinx);  // dy
    } else {
        this->CamVecLeft_Z = (x1 * cosz) + (y1 * sinz);  // dz
        this->CamVecLeft_X = (y1 * cosz) - (x1 * sinz);  // dx
        this->CamVecLeft_Y = z1;  // dy
    }

    // set 2 position transfrom (0 6 0) looks like cam front vector
    if (pCamera3D->sRotationY) {
        float v19 = (x2 * cosz) + (y2 * sinz);

        this->CamVecFront_Z = (v19 * cosx) + (z2 * sinx);  // dz
        this->CamVecFront_X = (y2 * cosz) - (x2 * sinz);  // dx
        this->CamVecFront_Y = (z2 * cosx) /*-*/ + (v19 * sinx);  // dy
    } else {
        this->CamVecFront_Z = (x2 * cosz) + (y2 * sinz);  // dz
        this->CamVecFront_X = (y2 * cosz) - (x2 * sinz);  // dx
        this->CamVecFront_Y = z2;  // dy
    }

    this->CamLeftDot =
        (this->CamVecLeft_X * this->field_0_party_dir_x) +
        (this->CamVecLeft_Z * this->field_4_party_dir_y) +
        (this->CamVecLeft_Y * this->field_8_party_dir_z);
    this->CamFrontDot =
        (this->CamVecFront_X * this->field_0_party_dir_x) +
        (this->CamVecFront_Z * this->field_4_party_dir_y) +
        (this->CamVecFront_Y * this->field_8_party_dir_z);
}

RenderOpenGL::RenderOpenGL(
    std::shared_ptr<OSWindow> window,
    DecalBuilder* decal_builder,
    LightmapBuilder* lightmap_builder,
    SpellFxRenderer* spellfx,
    std::shared_ptr<ParticleEngine> particle_engine,
    Vis* vis,
    Log* logger
) : RenderBase(window, decal_builder, lightmap_builder, spellfx, particle_engine, vis, logger) {
    clip_w = 0;
    clip_x = 0;
    clip_y = 0;
    clip_z = 0;
    render_target_rgb = nullptr;
}

RenderOpenGL::~RenderOpenGL() { /*__debugbreak();*/ }

void RenderOpenGL::Release() { __debugbreak(); }

void RenderOpenGL::SaveWinnersCertificate(const char *a1) { __debugbreak(); }

bool RenderOpenGL::InitializeFullscreen() {
    __debugbreak();
    return 0;
}

unsigned int RenderOpenGL::GetActorTintColor(int DimLevel, int tint, float WorldViewX, int a5, RenderBillboard *Billboard) {
    // GetActorTintColor(int max_dimm, int min_dimm, float distance, int a4, RenderBillboard *a5)
    return ::GetActorTintColor(DimLevel, tint, WorldViewX, a5, Billboard);
}


// when losing and regaining window focus - not required for OGL??
void RenderOpenGL::RestoreFrontBuffer() { /*__debugbreak();*/ }
void RenderOpenGL::RestoreBackBuffer() { /*__debugbreak();*/ }

void RenderOpenGL::BltBackToFontFast(int a2, int a3, Rect *a4) {
    __debugbreak();  // never called anywhere
}



unsigned int RenderOpenGL::GetRenderWidth() const { return window->GetWidth(); }
unsigned int RenderOpenGL::GetRenderHeight() const { return window->GetHeight(); }

void RenderOpenGL::ClearBlack() {  // used only at start and in game over win
    ClearZBuffer();
    ClearTarget(0);
}

void RenderOpenGL::ClearTarget(unsigned int uColor) {
    memset32(render_target_rgb, Color32(uColor), (window->GetWidth() * window->GetHeight()));
}



void RenderOpenGL::CreateZBuffer() {
    if (!pActiveZBuffer) {
        pActiveZBuffer = (int*)malloc(window->GetWidth() * window->GetHeight() * sizeof(int));
        ClearZBuffer();
    }
}

void RenderOpenGL::ClearZBuffer() {
    memset32(this->pActiveZBuffer, 0xFFFF0000, window->GetWidth() * window->GetHeight());
}

void RenderOpenGL::RasterLine2D(signed int uX, signed int uY, signed int uZ,
                                signed int uW, unsigned __int16 uColor) {
    unsigned int b = (uColor & 0x1F)*8;
    unsigned int g = ((uColor >> 5) & 0x3F)*4;
    unsigned int r = ((uColor >> 11) & 0x1F)*8;

    glDisable(GL_TEXTURE_2D);
    glLineWidth(1);
    glColor3ub(r, g, b);

    // pixel centers around 0.5 so tweak to avoid gaps and squashing
    if (uZ == uX) {
       uW += 1;
    }

    glBegin(GL_LINES);
    glVertex3f(uX, uY, 0);
    glVertex3f(uZ+.5, uW+.5, 0);
    drawcalls++;
    glEnd();
}

void RenderOpenGL::BeginSceneD3D() {
    // Setup for 3D

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 0/*0.9f, 0.5f, 0.1f, 1.0f*/);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render->uNumBillboardsToDraw = 0;  // moved from drawbillboards - cant reset this until mouse picking finished
}

extern unsigned int BlendColors(unsigned int a1, unsigned int a2);

void RenderOpenGL::DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard,
                                        RenderBillboard *billboard) {
    int v11;     // eax@9
    int v12;     // eax@9
    double v15;  // st5@12
    double v16;  // st4@12
    double v17;  // st3@12
    double v18;  // st2@12
    int v19;     // ecx@14
    double v20;  // st3@14
    int v21;     // ecx@16
    double v22;  // st3@16
    float v27;   // [sp+24h] [bp-Ch]@5
    float v29;   // [sp+2Ch] [bp-4h]@5
    float v31;   // [sp+3Ch] [bp+Ch]@5
    float a1;    // [sp+40h] [bp+10h]@5

    // if (this->uNumD3DSceneBegins == 0) {
    //    return;
    //}

    Sprite *pSprite = billboard->hwsprite;
    int dimming_level = billboard->dimming_level;

    // v4 = pSoftBillboard;
    // v5 = (double)pSoftBillboard->zbuffer_depth;
    // pSoftBillboarda = pSoftBillboard->zbuffer_depth;
    // v6 = pSoftBillboard->zbuffer_depth;
    unsigned int v7 = Billboard_ProbablyAddToListAndSortByZOrder(
        pSoftBillboard->screen_space_z);
    // v8 = dimming_level;
    // device_caps = v7;
    int v28 = dimming_level & 0xFF000000;
    if (dimming_level & 0xFF000000) {
        pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Opaque_3;
    } else {
        pBillboardRenderListD3D[v7].opacity = RenderBillboardD3D::Transparent;
    }
    // v10 = a3;
    pBillboardRenderListD3D[v7].field_90 = pSoftBillboard->field_44;
    pBillboardRenderListD3D[v7].screen_space_z = pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].object_pid = pSoftBillboard->object_pid;
    pBillboardRenderListD3D[v7].sParentBillboardID =
        pSoftBillboard->sParentBillboardID;
    // v25 = pSoftBillboard->uScreenSpaceX;
    // v24 = pSoftBillboard->uScreenSpaceY;
    a1 = pSoftBillboard->screenspace_projection_factor_x;
    v29 = pSoftBillboard->screenspace_projection_factor_y;
    v31 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v27 = (double)(pSprite->uBufferHeight - pSprite->uAreaY);
    if (pSoftBillboard->uFlags & 4) {
        v31 = v31 * -1.0;
    }
    if (config->is_tinting && pSoftBillboard->sTintColor) {
        v11 = ::GetActorTintColor(dimming_level, 0,
            pSoftBillboard->screen_space_z, 0, 0);
        v12 = BlendColors(pSoftBillboard->sTintColor, v11);
        if (v28)
            v12 =
            (uint64_t)((char *)&array_77EC08[1852].pEdgeList1[17] + 3) &
            ((unsigned int)v12 >> 1);
    } else {
        v12 = ::GetActorTintColor(dimming_level, 0,
            pSoftBillboard->screen_space_z, 0, 0);
    }
    // v13 = (double)v25;
    pBillboardRenderListD3D[v7].pQuads[0].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[0].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[0].pos.x =
        pSoftBillboard->screen_space_x - v31 * a1;
    // v14 = (double)v24;
    // v32 = v14;
    pBillboardRenderListD3D[v7].pQuads[0].pos.y =
        pSoftBillboard->screen_space_y - v27 * v29;
    v15 = 1.0 - 1.0 / (pSoftBillboard->screen_space_z * 0.061758894);
    pBillboardRenderListD3D[v7].pQuads[0].pos.z = v15;
    v16 = 1.0 / pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].pQuads[0].rhw =
        1.0 / pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[0].texcoord.y = 0.0;
    v17 = (double)((pSprite->uBufferWidth >> 1) - pSprite->uAreaX);
    v18 = (double)(pSprite->uBufferHeight - pSprite->uAreaY -
        pSprite->uAreaHeight);
    if (pSoftBillboard->uFlags & 4) {
        v17 = v17 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[1].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[1].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[1].pos.x =
        pSoftBillboard->screen_space_x - v17 * a1;
    pBillboardRenderListD3D[v7].pQuads[1].pos.y =
        pSoftBillboard->screen_space_y - v18 * v29;
    pBillboardRenderListD3D[v7].pQuads[1].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[1].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.x = 0.0;
    pBillboardRenderListD3D[v7].pQuads[1].texcoord.y = 1.0;
    v19 = pSprite->uBufferHeight - pSprite->uAreaY - pSprite->uAreaHeight;
    v20 = (double)(pSprite->uAreaX + pSprite->uAreaWidth +
        (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if (pSoftBillboard->uFlags & 4) {
        v20 = v20 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[2].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[2].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[2].pos.x =
        v20 * a1 + pSoftBillboard->screen_space_x;
    pBillboardRenderListD3D[v7].pQuads[2].pos.y =
        pSoftBillboard->screen_space_y - (double)v19 * v29;
    pBillboardRenderListD3D[v7].pQuads[2].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[2].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[2].texcoord.y = 1.0;
    v21 = pSprite->uBufferHeight - pSprite->uAreaY;
    v22 = (double)(pSprite->uAreaX + pSprite->uAreaWidth +
        (pSprite->uBufferWidth >> 1) - pSprite->uBufferWidth);
    if (pSoftBillboard->uFlags & 4) {
        v22 = v22 * -1.0;
    }
    pBillboardRenderListD3D[v7].pQuads[3].specular = 0;
    pBillboardRenderListD3D[v7].pQuads[3].diffuse = v12;
    pBillboardRenderListD3D[v7].pQuads[3].pos.x =
        v22 * a1 + pSoftBillboard->screen_space_x;
    pBillboardRenderListD3D[v7].pQuads[3].pos.y =
        pSoftBillboard->screen_space_y - (double)v21 * v29;
    pBillboardRenderListD3D[v7].pQuads[3].pos.z = v15;
    pBillboardRenderListD3D[v7].pQuads[3].rhw = v16;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.x = 1.0;
    pBillboardRenderListD3D[v7].pQuads[3].texcoord.y = 0.0;
    // v23 = pSprite->pTexture;
    pBillboardRenderListD3D[v7].uNumVertices = 4;
    pBillboardRenderListD3D[v7].z_order = pSoftBillboard->screen_space_z;
    pBillboardRenderListD3D[v7].texture = pSprite->texture;
}

void RenderOpenGL::_4A4CC9_AddSomeBillboard(
    struct SpellFX_Billboard *a1, int diffuse) {
    __debugbreak();
}

void RenderOpenGL::DrawBillboardList_BLV() {
    SoftwareBillboard soft_billboard = { 0 };
    soft_billboard.sParentBillboardID = -1;
    //  soft_billboard.pTarget = pBLVRenderParams->pRenderTarget;
    soft_billboard.pTargetZ = pBLVRenderParams->pTargetZBuffer;
    //  soft_billboard.uTargetPitch = uTargetSurfacePitch;
    soft_billboard.uViewportX = pBLVRenderParams->uViewportX;
    soft_billboard.uViewportY = pBLVRenderParams->uViewportY;
    soft_billboard.uViewportZ = pBLVRenderParams->uViewportZ - 1;
    soft_billboard.uViewportW = pBLVRenderParams->uViewportW;

    pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;
    for (uint i = 0; i < ::uNumBillboardsToDraw; ++i) {
        RenderBillboard *p = &pBillboardRenderList[i];
        if (p->hwsprite) {
            soft_billboard.screen_space_x = p->screen_space_x;
            soft_billboard.screen_space_y = p->screen_space_y;
            soft_billboard.screen_space_z = p->screen_space_z;
            soft_billboard.sParentBillboardID = i;
            soft_billboard.screenspace_projection_factor_x =
                p->screenspace_projection_factor_x;
            soft_billboard.screenspace_projection_factor_y =
                p->screenspace_projection_factor_y;
            soft_billboard.object_pid = p->object_pid;
            soft_billboard.uFlags = p->field_1E;
            soft_billboard.sTintColor = p->sTintColor;

            DrawBillboard_Indoor(&soft_billboard, p);
        }
    }
}


void RenderOpenGL::DrawProjectile(float srcX, float srcY, float a3, float a4,
                                  float dstX, float dstY, float a7, float a8,
                                  Texture *texture) {
    __debugbreak();
}
void RenderOpenGL::ScreenFade(unsigned int color, float t) { __debugbreak(); }


void RenderOpenGL::DrawTextureOffset(int pX, int pY, int move_X, int move_Y,
                                     Image *pTexture) {
    DrawTextureNew((float)(pX - move_X)/window->GetWidth(), (float)(pY - move_Y)/window->GetHeight(), pTexture);
}


void RenderOpenGL::DrawImage(Image *img, const Rect &rect) {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    bool blendFlag = 1;

    // check if loaded
    auto texture = (TextureOpenGL *)img;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    if (blendFlag) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float depth = 0;

    GLfloat Vertices[] = { (float)rect.x, (float)rect.y, depth,
        (float)rect.z, (float)rect.y, depth,
        (float)rect.z, (float)rect.w, depth,
        (float)rect.x, (float)rect.w, depth };

    GLfloat TexCoord[] = { 0, 0,
        1, 0,
        1, 1,
        0, 1 };

    GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    drawcalls++;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning("OpenGL: draw image error: (%u)", err);
    }
}


void RenderOpenGL::ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture,
                                  int a5) {
    // __debugbreak();
    // blank in d3d
}


void RenderOpenGL::ZDrawTextureAlpha(float u, float v, Image *img, int zVal) {
    if (!img) return;

    int uOutX = u * this->window->GetWidth();
    int uOutY = v * this->window->GetHeight();
    unsigned int imgheight = img->GetHeight();
    unsigned int imgwidth = img->GetWidth();
    auto pixels = (uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (uOutX < 0)
        uOutX = 0;
    if (uOutY < 0)
        uOutY = 0;

    for (int xs = 0; xs < imgwidth; xs++) {
        for (int ys = 0; ys < imgheight; ys++) {
            if (pixels[xs + imgwidth * ys] & 0xFF000000) {
                this->pActiveZBuffer[uOutX + xs + window->GetWidth() * (uOutY + ys)] = zVal;
            }
        }
    }
}




void RenderOpenGL::BlendTextures(int x, int y, Image* imgin, Image* imgblend, int time, int start_opacity,
    int end_opacity) {
    // thrown together as a crude estimate of the enchaintg
                          // effects

      // leaves gap where it shouldnt on dark pixels currently
      // doesnt use opacity params

    const uint32_t* pixelpoint;
    const uint32_t* pixelpointblend;

    if (imgin && imgblend) {  // 2 images to blend
        pixelpoint = (const uint32_t*)imgin->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        pixelpointblend =
            (const uint32_t*)imgblend->GetPixels(IMAGE_FORMAT_A8R8G8B8);

        int Width = imgin->GetWidth();
        int Height = imgin->GetHeight();
        // Image* temp = Image::Create(Width, Height, IMAGE_FORMAT_A8R8G8B8);
        // uint32_t* temppix = (uint32_t*)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

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

                    // temppix[xdraw + ydraw * Width] = Color32(rcur, gcur, bcur);
                    render_target_rgb[x + xdraw + (render->GetRenderWidth() * (y + ydraw))] = Color32(rcur, gcur, bcur);
                }

                pixelpoint++;
            }

            pixelpoint += imgin->GetWidth() - Width;
        }
        // draw image
        // render->DrawTextureAlphaNew(x / float(window->GetWidth()), y / float(window->GetHeight()), temp);
        // temp->Release();
    }
}


void RenderOpenGL::TexturePixelRotateDraw(float u, float v, Image *img, int time) {
    if (img) {
        auto pixelpoint = (const uint32_t *)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        int width = img->GetWidth();
        int height = img->GetHeight();

        int brightloc = -1;
        int brightval = 0;
        int darkloc = -1;
        int darkval = 765;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int nudge = x + y * width;
                // Test the brightness against the threshold
                int bright = (*(pixelpoint + nudge) & 0xFF) + ((*(pixelpoint + nudge) >> 8) & 0xFF) + ((*(pixelpoint + nudge) >> 16) & 0xFF);
                if (bright == 0) continue;

                if (bright > brightval) {
                    brightval = bright;
                    brightloc = nudge;
                }
                if (bright < darkval) {
                    darkval = bright;
                    darkloc = nudge;
                }
            }
        }

        // find brightest
        unsigned int bmax = (*(pixelpoint + brightloc) & 0xFF);
        unsigned int gmax = ((*(pixelpoint + brightloc) >> 8) & 0xFF);
        unsigned int rmax = ((*(pixelpoint + brightloc) >> 16) & 0xFF);

        // find darkest not black
        unsigned int bmin = (*(pixelpoint + darkloc) & 0xFF);
        unsigned int gmin = ((*(pixelpoint + darkloc) >> 8) & 0xFF);
        unsigned int rmin = ((*(pixelpoint + darkloc) >> 16) & 0xFF);

        // steps pixels
        float bstep = (bmax - bmin) / 128.;
        float gstep = (gmax - gmin) / 128.;
        float rstep = (rmax - rmin) / 128.;

        int timestep = time % 256;

        // loop through
        for (int ydraw = 0; ydraw < height; ++ydraw) {
            for (int xdraw = 0; xdraw < width; ++xdraw) {
                if (*pixelpoint) {  // check orig item not got blakc pixel
                    unsigned int bcur = (*(pixelpoint) & 0xFF);
                    unsigned int gcur = ((*(pixelpoint) >> 8) & 0xFF);
                    unsigned int rcur = ((*(pixelpoint) >> 16) & 0xFF);
                    int pixstepb = (bcur - bmin) / bstep + timestep;
                    if (pixstepb > 255) pixstepb = pixstepb - 256;
                    if (pixstepb >= 0 && pixstepb < 128)  // 0-127
                        bcur = bmin + pixstepb * bstep;
                    if (pixstepb >= 128 && pixstepb < 256) {  // 128-255
                        pixstepb = pixstepb - 128;
                        bcur = bmax - pixstepb * bstep;
                    }
                    int pixstepr = (rcur - rmin) / rstep + timestep;
                    if (pixstepr > 255) pixstepr = pixstepr - 256;
                    if (pixstepr >= 0 && pixstepr < 128)  // 0-127
                        rcur = rmin + pixstepr * rstep;
                    if (pixstepr >= 128 && pixstepr < 256) {  // 128-255
                        pixstepr = pixstepr - 128;
                        rcur = rmax - pixstepr * rstep;
                    }
                    int pixstepg = (gcur - gmin) / gstep + timestep;
                    if (pixstepg > 255) pixstepg = pixstepg - 256;
                    if (pixstepg >= 0 && pixstepg < 128)  // 0-127
                        gcur = gmin + pixstepg * gstep;
                    if (pixstepg >= 128 && pixstepg < 256) {  // 128-255
                        pixstepg = pixstepg - 128;
                        gcur = gmax - pixstepg * gstep;
                    }
                    // out pixel
                    // temppix[xdraw + ydraw * width] = (rcur << 24) | (gcur << 16) | (bcur << 8) | 0xFF;//Color32(rcur, gcur, bcur);
                    render_target_rgb[int((u*window->GetWidth())+xdraw + window->GetWidth() *(v*window->GetHeight()+ydraw))] = Color32(rcur, gcur, bcur);
                }
                pixelpoint++;
            }
        }
        // draw image
        // render->Update_Texture(img);
        // render->DrawTextureAlphaNew(u, v, img);
        // temp->Release();
    }
}



void RenderOpenGL::DrawMonsterPortrait(Rect rc, SpriteFrame *Portrait, int Y_Offset) {
    Rect rct;
    rct.x = rc.x + 64 + Portrait->hw_sprites[0]->uAreaX - Portrait->hw_sprites[0]->uBufferWidth / 2;
    rct.y = rc.y + Y_Offset + Portrait->hw_sprites[0]->uAreaY;
    rct.z = rct.x + Portrait->hw_sprites[0]->uAreaWidth;
    rct.w = rct.y + Portrait->hw_sprites[0]->uAreaHeight;

    render->SetUIClipRect(rc.x, rc.y, rc.z, rc.w);
    render->DrawImage(Portrait->hw_sprites[0]->texture, rct);
    render->ResetUIClipRect();
}

void RenderOpenGL::DrawTransparentRedShade(float u, float v, Image *a4) {
    DrawMasked(u, v, a4, 0, 0xF800);
}

void RenderOpenGL::DrawTransparentGreenShade(float u, float v, Image *pTexture) {
    DrawMasked(u, v, pTexture, 0, 0x07E0);
}

//void RenderOpenGL::DrawFansTransparent(const RenderVertexD3D3 *vertices,
//                                       unsigned int num_vertices) {
//    __debugbreak();
//}

inline uint32_t PixelDim(uint32_t pix, int dimming) {
    return Color32((((pix >> 16) & 0xFF) >> dimming),
        (((pix >> 8) & 0xFF) >> dimming),
        ((pix & 0xFF) >> dimming));
}

void RenderOpenGL::DrawMasked(float u, float v, Image *pTexture, unsigned int color_dimming_level,
                              unsigned __int16 mask) {
    if (!pTexture) {
        return;
    }
    uint32_t width = pTexture->GetWidth();
    uint32_t *pixels = (uint32_t *)pTexture->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    // Image *temp = Image::Create(width, pTexture->GetHeight(), IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *temppix = (uint32_t *)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    int x = window->GetWidth() * u;
    int y = window->GetHeight() * v;

    for (unsigned int dy = 0; dy < pTexture->GetHeight(); ++dy) {
        for (unsigned int dx = 0; dx < width; ++dx) {
            if (*pixels & 0xFF000000) {
                if ((x + dx) < window->GetWidth() && (y + dy) < window->GetHeight()) {
                    render_target_rgb[x + dx + window->GetWidth() * (y + dy)] = PixelDim(*pixels, color_dimming_level) & Color32(mask);
                }
            }
            ++pixels;
        }
    }
    // render->DrawTextureAlphaNew(u, v, temp);
    // temp->Release();;
}



void RenderOpenGL::DrawTextureGrayShade(float a2, float a3, Image *a4) {
    DrawMasked(a2, a3, a4, 1, 0x7BEF);
}

void RenderOpenGL::DrawIndoorSky(unsigned int uNumVertices,
                                 unsigned int uFaceID) {
    __debugbreak();
}

void RenderOpenGL::DrawIndoorSkyPolygon(signed int uNumVertices,
                                        struct Polygon *pSkyPolygon) {
    __debugbreak();
}

bool RenderOpenGL::AreRenderSurfacesOk() { return true; }

unsigned short *RenderOpenGL::MakeScreenshot(int width, int height) {
    GLubyte* sPixels = new GLubyte[3 * window->GetWidth() * window->GetHeight()];

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        pOutdoor->Draw();
    }

    if (uCurrentlyLoadedLevelType != LEVEL_null)
        DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();

    glReadPixels(0, 0, window->GetWidth(), window->GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, sPixels);

    uint16_t *for_pixels;  // ebx@1

    float interval_x = game_viewport_width / (double)width;
    float interval_y = game_viewport_height / (double)height;

    uint16_t *pPixels = (uint16_t *)malloc(sizeof(uint16_t) * height * width);
    memset(pPixels, 0, sizeof(uint16_t) * height * width);

    for_pixels = pPixels;

    if (uCurrentlyLoadedLevelType == LEVEL_null) {
        memset(&for_pixels, 0, sizeof(for_pixels));
    } else {
        for (uint y = 0; y < (unsigned int)height; ++y) {
            for (uint x = 0; x < (unsigned int)width; ++x) {
                unsigned __int8 *p;

                p = sPixels + 3 * (int)(x * interval_x + 8.0) + 3 * (int)(window->GetHeight() - (y * interval_y) - 8.0) * window->GetWidth();

                *for_pixels = Color16(*p & 255, *(p + 1) & 255, *(p + 2) & 255);
                ++for_pixels;
            }
        }
    }

    return pPixels;
}

Image *RenderOpenGL::TakeScreenshot(unsigned int width, unsigned int height) {
    auto pixels = MakeScreenshot(width, height);
    Image *image = Image::Create(width, height, IMAGE_FORMAT_R5G6B5, pixels);
    free(pixels);
    return image;
}

void RenderOpenGL::SaveScreenshot(const std::string &filename, unsigned int width, unsigned int height) {
    auto pixels = MakeScreenshot(width, height);

    FILE *result = fopen(filename.c_str(), "wb");
    if (result == nullptr) {
        return;
    }

    unsigned int pcx_data_size = width * height * 5;
    uint8_t *pcx_data = new uint8_t[pcx_data_size];
    unsigned int pcx_data_real_size = 0;
    PCX::Encode16(pixels, width, height, pcx_data, pcx_data_size, &pcx_data_real_size);
    fwrite(pcx_data, pcx_data_real_size, 1, result);
    delete[] pcx_data;
    fclose(result);
}

void RenderOpenGL::PackScreenshot(unsigned int width, unsigned int height,
                                  void *out_data, unsigned int data_size,
                                  unsigned int *screenshot_size) {
    auto pixels = MakeScreenshot(width, height);
    SaveScreenshot("save.pcx", width, height);
    PCX::Encode16(pixels, 150, 112, out_data, 1000000, screenshot_size);
    free(pixels);
}

void RenderOpenGL::SavePCXScreenshot() { __debugbreak(); }
int RenderOpenGL::GetActorsInViewport(int pDepth) {
    __debugbreak();
    return 0;
}

void RenderOpenGL::BeginLightmaps() {
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    auto effpar03 = assets->GetBitmap("effpar03");
    auto texture = (TextureOpenGL*)effpar03;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
}

void RenderOpenGL::EndLightmaps() {
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, GL_lastboundtex);
}


void RenderOpenGL::BeginLightmaps2() {
    glDisable(GL_CULL_FACE);
    glDepthMask(false);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    static Texture* effpar03 = assets->GetBitmap("effpar03");
    auto texture = (TextureOpenGL*)effpar03;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
}


void RenderOpenGL::EndLightmaps2() {
    glDisable(GL_BLEND);
    glDepthMask(true);
    glEnable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D, GL_lastboundtex);
}

bool RenderOpenGL::DrawLightmap(struct Lightmap *pLightmap, struct Vec3_float_ *pColorMult, float z_bias) {
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

    glBegin(GL_TRIANGLE_FAN);

    for (uint i = 0; i < pLightmap->NumVertices; ++i) {
        glColor4f((uLightmapColorR) / 255.0f, (uLightmapColorG) / 255.0f, (uLightmapColorB) / 255.0f, 1.0f);
        glTexCoord2f(pLightmap->pVertices[i].u, pLightmap->pVertices[i].v);
        glVertex3f(pLightmap->pVertices[i].vWorldPosition.x, pLightmap->pVertices[i].vWorldPosition.y, pLightmap->pVertices[i].vWorldPosition.z);
    }

    glEnd();

    drawcalls++;
    return true;
}

void RenderOpenGL::BeginDecals() {
    auto texture = (TextureOpenGL*)assets->GetBitmap("hwsplat04");
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void RenderOpenGL::EndDecals() {
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void RenderOpenGL::DrawDecal(struct Decal *pDecal, float z_bias) {
    // need to add z biasing
    RenderVertexD3D3 pVerticesD3D[64];

    if (pDecal->uNumVertices < 3) {
        log->Warning("Decal has < 3 vertices");
        return;
    }

    float color_mult = pDecal->Fade_by_time();
    if (color_mult == 0.0) return;

    // temp - bloodsplat persistance
    // color_mult = 1;

    glBegin(GL_TRIANGLE_FAN);

    for (uint i = 0; i < (unsigned int)pDecal->uNumVertices; ++i) {
        uint uTint =
            GetActorTintColor(pDecal->DimmingLevel, 0, pDecal->pVertices[i].vWorldViewPosition.x, 0, nullptr);

        uint uTintR = (uTint >> 16) & 0xFF, uTintG = (uTint >> 8) & 0xFF,
            uTintB = uTint & 0xFF;

        uint uDecalColorMultR = (pDecal->uColorMultiplier >> 16) & 0xFF,
            uDecalColorMultG = (pDecal->uColorMultiplier >> 8) & 0xFF,
            uDecalColorMultB = pDecal->uColorMultiplier & 0xFF;

        uint uFinalR =
            floorf(uTintR / 255.0 * color_mult * uDecalColorMultR + 0.0f),
            uFinalG =
            floorf(uTintG / 255.0 * color_mult * uDecalColorMultG + 0.0f),
            uFinalB =
            floorf(uTintB / 255.0 * color_mult * uDecalColorMultB + 0.0f);

        glColor4f((uFinalR) / 255.0f, (uFinalG) / 255.0f, (uFinalB) / 255.0f, 1.0f);
        glTexCoord2f(pDecal->pVertices[i].u, pDecal->pVertices[i].v);
        glVertex3f(pDecal->pVertices[i].vWorldPosition.x, pDecal->pVertices[i].vWorldPosition.y, pDecal->pVertices[i].vWorldPosition.z);

        drawcalls++;
    }
    glEnd();
}

void RenderOpenGL::do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin,
                                          signed int sDiffuseBegin,
                                          const RenderVertexD3D3 *pLineEnd,
                                          signed int sDiffuseEnd,
                                          float z_stuff) {
    __debugbreak();
}
void RenderOpenGL::DrawLines(const RenderVertexD3D3 *vertices,
                             unsigned int num_vertices) {
    __debugbreak();
}
void RenderOpenGL::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices,
                                          Texture *texture) {
    __debugbreak();
}

void RenderOpenGL::am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3, int blend_mode) {
    // want to draw psrcrect section @ point
    // __debugbreak();


    // need to add blue masking


    glEnable(GL_TEXTURE_2D);
    float col = (blend_mode == 2) ? 1 : 0.5;

    glColor3f(col, col, col);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto texture = (TextureOpenGL*)pArcomageGame->pSprites;
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int texwidth = texture->GetWidth();
    int texheight = texture->GetHeight();

    int width = pSrcRect->z - pSrcRect->x;
    int height = pSrcRect->w - pSrcRect->y;

    int x = pTargetPoint->x;  // u* window->GetWidth();
    int y = pTargetPoint->y;  // v* window->GetHeight();
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= (int)window->GetWidth() || x >= clipz || y >= (int)window->GetHeight() || y >= clipw) return;
    // check for overlap
    if (!(clipx < z && clipz > x && clipy < w && clipw > y)) return;

    int drawx = x;  // std::max(x, clipx);
    int drawy = y;  // std::max(y, clipy);
    int draww = w;  // std::min(w, clipw);
    int drawz = z;  // std::min(z, clipz);

    float depth = 0;

    GLfloat Vertices[] = { (float)drawx, (float)drawy, depth,
        (float)drawz, (float)drawy, depth,
        (float)drawz, (float)draww, depth,
        (float)drawx, (float)draww, depth };

    float texx = pSrcRect->x / float(texwidth);  // (drawx - x) / float(width);
    float texy = pSrcRect->y / float(texheight);  //  (drawy - y) / float(height);
    float texz = pSrcRect->z / float(texwidth);  //  (width - (z - drawz)) / float(width);
    float texw = pSrcRect->w / float(texheight);  // (height - (w - draww)) / float(height);

    GLfloat TexCoord[] = { texx, texy,
        texz, texy,
        texz, texw,
        texx, texw,
    };

    GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    drawcalls++;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);


    // blank over same bit of this render_target_rgb to stop text overlaps
    for (int ys = drawy; ys < draww; ys++) {
        memset(this->render_target_rgb + (ys * window->GetWidth() + drawx), 0x00000000, (drawz - drawx) * 4);
    }

    return;
}



void RenderOpenGL::PrepareDecorationsRenderList_ODM() {
    unsigned int v6;        // edi@9
    int v7;                 // eax@9
    SpriteFrame *frame;     // eax@9
    unsigned __int16 *v10;  // eax@9
    int v13;                // ecx@9
    char r;                 // ecx@20
    char g;                 // dl@20
    char b_;                // eax@20
    Particle_sw local_0;    // [sp+Ch] [bp-98h]@7
    unsigned __int16 *v37;  // [sp+84h] [bp-20h]@9
    int v38;                // [sp+88h] [bp-1Ch]@9

    for (unsigned int i = 0; i < uNumLevelDecorations; ++i) {
        // LevelDecoration* decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST) ||
            pLevelDecorations[i].IsObeliskChestActive()) &&
            !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            DecorationDesc *decor_desc = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);
            if (!(decor_desc->uFlags & 0x80)) {
                if (!(decor_desc->uFlags & 0x22)) {
                    v6 = pMiscTimer->uTotalGameTimeElapsed;
                    v7 = abs(pLevelDecorations[i].vPosition.x +
                        pLevelDecorations[i].vPosition.y);

                    frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                        v6 + v7);

                    if (engine->config->seasons_change) {
                        frame = LevelDecorationChangeSeason(decor_desc, v6 + v7, pParty->uCurrentMonth);
                    }

                    if (!frame || frame->texture_name == "null" || frame->hw_sprites[0] == NULL) {
                        continue;
                    }

                    // v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                    // v6 + v7);

                    v10 = (unsigned __int16 *)TrigLUT->Atan2(
                        pLevelDecorations[i].vPosition.x -
                        pCamera3D->vCameraPos.x,
                        pLevelDecorations[i].vPosition.y -
                        pCamera3D->vCameraPos.y);
                    v38 = 0;
                    v13 = ((signed int)(TrigLUT->uIntegerPi +
                        ((signed int)TrigLUT->uIntegerPi >>
                            3) +
                        pLevelDecorations[i].field_10_y_rot -
                        (int64_t)v10) >>
                        8) &
                        7;
                    v37 = (unsigned __int16 *)v13;
                    if (frame->uFlags & 2) v38 = 2;
                    if ((256 << v13) & frame->uFlags) v38 |= 4;
                    if (frame->uFlags & 0x40000) v38 |= 0x40;
                    if (frame->uFlags & 0x20000) v38 |= 0x80;

                    // for light
                    if (frame->uGlowRadius) {
                        r = 255;
                        g = 255;
                        b_ = 255;
                        if (render->config->is_using_colored_lights) {
                            r = decor_desc->uColoredLightRed;
                            g = decor_desc->uColoredLightGreen;
                            b_ = decor_desc->uColoredLightBlue;
                        }
                        pStationaryLightsStack->AddLight(
                            pLevelDecorations[i].vPosition.x,
                            pLevelDecorations[i].vPosition.y,
                            pLevelDecorations[i].vPosition.z +
                            decor_desc->uDecorationHeight / 2,
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
                        if (2 * abs(view_x) >= abs(view_y)) {
                            int projected_x = 0;
                            int projected_y = 0;
                            pCamera3D->Project(view_x, view_y, view_z,
                                &projected_x,
                                &projected_y);

                            float _v41 = frame->scale * (pCamera3D->ViewPlaneDist_X) / (view_x);

                            int screen_space_half_width = _v41 * frame->hw_sprites[(int64_t)v37]->uBufferWidth / 2;

                            if (projected_x + screen_space_half_width >=
                                (signed int)pViewport->uViewportTL_X &&
                                projected_x - screen_space_half_width <=
                                (signed int)pViewport->uViewportBR_X) {
                                if (::uNumBillboardsToDraw >= 500) return;
                                ::uNumBillboardsToDraw++;
                                ++uNumDecorationsDrawnThisFrame;

                                pBillboardRenderList[::uNumBillboardsToDraw - 1]
                                    .hwsprite = frame->hw_sprites[(int64_t)v37];
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
                                    .uPalette = frame->uPaletteIndex;
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
                            }
                        }
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
                local_0.timeToLive = (rand() & 0x80) + 128;
                local_0.texture = spell_fx_renderer->effpar01;
                particle_engine->AddParticle(&local_0);
            }
        }
    }
}

/*#pragma pack(push, 1)
typedef struct {
        char  idlength;
        char  colourmaptype;
        char  datatypecode;
        short int colourmaporigin;
        short int colourmaplength;
        char  colourmapdepth;
        short int x_origin;
        short int y_origin;
        short width;
        short height;
        char  bitsperpixel;
        char  imagedescriptor;
} tga;
#pragma pack(pop)

FILE *CreateTga(const char *filename, int image_width, int image_height)
{
        auto f = fopen(filename, "w+b");

        tga tga_header;
        memset(&tga_header, 0, sizeof(tga_header));

        tga_header.colourmaptype = 0;
        tga_header.datatypecode = 2;
        //tga_header.colourmaporigin = 0;
        //tga_header.colourmaplength = image_width * image_height;
        //tga_header.colourmapdepth = 32;
        tga_header.x_origin = 0;
        tga_header.y_origin = 0;
        tga_header.width = image_width;
        tga_header.height = image_height;
        tga_header.bitsperpixel = 32;
        tga_header.imagedescriptor = 32; // top-down
        fwrite(&tga_header, 1, sizeof(tga_header), f);

        return f;
}*/

Texture *RenderOpenGL::CreateTexture_ColorKey(const std::string &name, uint16_t colorkey) {
    return TextureOpenGL::Create(new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey));
}

Texture *RenderOpenGL::CreateTexture_Solid(const std::string &name) {
    return TextureOpenGL::Create(new Image16bit_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_Alpha(const std::string &name) {
    return TextureOpenGL::Create(new Alpha_LOD_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromIconsLOD(const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Compressed_Loader(pIcons_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromNewLOD(const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Compressed_Loader(pNew_LOD, name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromFile(const std::string &name) {
    return TextureOpenGL::Create(new PCX_File_Loader(name));
}

Texture *RenderOpenGL::CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) {
    return TextureOpenGL::Create(new PCX_LOD_Raw_Loader(pLOD, name));
}

Texture *RenderOpenGL::CreateTexture_Blank(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels) {

    return TextureOpenGL::Create(width, height, format, pixels);
}


Texture *RenderOpenGL::CreateTexture(const std::string &name) {
    return TextureOpenGL::Create(new Bitmaps_LOD_Loader(pBitmaps_LOD, name, engine->config->use_hwl_bitmaps));
}

Texture *RenderOpenGL::CreateSprite(const std::string &name, unsigned int palette_id,
                                    /*refactor*/ unsigned int lod_sprite_id) {
    return TextureOpenGL::Create(
        new Sprites_LOD_Loader(pSprites_LOD, palette_id, name, lod_sprite_id));
}

void RenderOpenGL::Update_Texture(Texture *texture) {
    // takes care of endian flip from literals here - hence BGRA

    auto t = (TextureOpenGL *)texture;
    glBindTexture(GL_TEXTURE_2D, t->GetOpenGlTexture());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->GetWidth(), t->GetHeight(), GL_BGRA, GL_UNSIGNED_BYTE, t->GetPixels(IMAGE_FORMAT_A8R8G8B8));
    glBindTexture(GL_TEXTURE_2D, NULL);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning("OpenGL: update texture error: (%u)", err);
    }
}

void RenderOpenGL::DeleteTexture(Texture *texture) {
    // crash here when assets not loaded as texture

    auto t = (TextureOpenGL *)texture;
    GLuint texid = t->GetOpenGlTexture();
    if (texid != -1) {
        glDeleteTextures(1, &texid);
    }
}

void RenderOpenGL::RemoveTextureFromDevice(Texture* texture) { __debugbreak(); }

bool RenderOpenGL::MoveTextureToDevice(Texture *texture) {
    auto t = (TextureOpenGL *)texture;
    auto native_format = t->GetFormat();
    int gl_format = GL_RGB;
        // native_format == IMAGE_FORMAT_A1R5G5B5 ? GL_RGBA : GL_RGB;

    unsigned __int8 *pixels = nullptr;
    if (native_format == IMAGE_FORMAT_R5G6B5 || native_format == IMAGE_FORMAT_A1R5G5B5 || native_format == IMAGE_FORMAT_A8R8G8B8 || native_format == IMAGE_FORMAT_R8G8B8A8) {
        pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8A8);  // rgba
        gl_format = GL_RGBA;
    } else {
        log->Warning("Image not loaded!");
    }

    if (pixels) {
        GLuint texid;
        glGenTextures(1, &texid);
        t->SetOpenGlTexture(texid);

        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, gl_format, t->GetWidth(), t->GetHeight(),
                     0, gl_format, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    return false;
}

void _set_3d_projection_matrix() {
    float near_clip = pCamera3D->GetNearClip();
    float far_clip = pCamera3D->GetFarClip();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // ogl uses fov in Y
    gluPerspective(pCamera3D->fov_y_deg, double(game_viewport_width/double(game_viewport_height)), near_clip, far_clip);
}

void _set_3d_modelview_matrix() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(-1.0f, 1.0f, -1.0f);

    int camera_x = pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0);
    int camera_y = pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0);
    int camera_z = pParty->vPosition.z + pParty->sEyelevel;

    gluLookAt(camera_x, camera_y, camera_z,

              camera_x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0f),
              camera_y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0f),
              camera_z - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationY / 2048.0f),
              0, 0, 1);
}

void _set_ortho_projection(bool gameviewport = false) {
    if (!gameviewport) {  // project over entire window
        glViewport(0, 0, window->GetWidth(), window->GetHeight());

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window->GetWidth(), window->GetHeight(), 0, -1, 1);
    } else {  // project to game viewport
        glViewport(game_viewport_x, window->GetHeight()-game_viewport_w-1, game_viewport_width, game_viewport_height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(game_viewport_x, game_viewport_z, game_viewport_w, game_viewport_y, 1, -1);  // far = 1 but ogl looks down -z
    }
}

void _set_ortho_modelview() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const int terrain_block_scale = 512;
const int terrain_height_scale = 32;
void RenderOpenGL::RenderTerrainD3D() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    static RenderVertexSoft pTerrainVertices[128 * 128];

    int blockScale = 512;
    int heightScale = 32;

    // TODO(pskelton): move these calculations to map load so its not run every frame
    for (unsigned int z = 0; z < 128; ++z) {
        for (unsigned int x = 0; x < 128; ++x) {
            pTerrainVertices[z * 128 + x].vWorldPosition.x = (-64 + (signed)x) * blockScale;
            pTerrainVertices[z * 128 + x].vWorldPosition.y = (64 - (signed)z) * blockScale;
            pTerrainVertices[z * 128 + x].vWorldPosition.z = heightScale * pOutdoor->pTerrain.pHeightmap[z * 128 + x];
        }
    }


    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    TextureOpenGL* texture = nullptr;
    Texture* tile_texture = nullptr;
    GL_lastboundtex = -1;  // keeps track of which tile texture is loaded
    GLint thistex = -1;


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
    } else if (direction >= 1.0 && direction < 3.0) {  // NorthEast(CB) - WestNorth(CЗ)
        Start_X = 0, End_X = 127;
        Start_Z = 0, End_Z = pODMRenderParams->uMapGridCellY + 2;
    } else if (direction >= 3.0 && direction < 5.0) {  // WestNorth(CЗ) - SouthWest(ЮЗ)
        Start_X = 0, End_X = pODMRenderParams->uMapGridCellX + 2;
        Start_Z = 0, End_Z = 127;
    } else if (direction >= 5.0 && direction < 7.0) {  // SouthWest(ЮЗ) - //SouthEast(ЮВ)
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



    // tile culling maths
    int camx = WorldPosToGridCellX(pCamera3D->vCameraPos.x);
    int camy = WorldPosToGridCellY(pCamera3D->vCameraPos.y);
    int tilerange = (pCamera3D->GetFarClip() / terrain_block_scale) + 3;

    float Light_tile_dist;


    for (int z = Start_Z; z < End_Z; ++z) {
        for (int x = Start_X; x < End_X; ++x) {
            // tile culling
            int xdist = camx - x;
            int zdist = camy - z;

            if (xdist > tilerange || zdist > tilerange) continue;

            int dist = sqrt((xdist) * (xdist) + (zdist) * (zdist));
            if (dist > tilerange) continue;  // crude distance culling

            struct Polygon* pTilePolygon = &array_77EC08[pODMRenderParams->uNumPolygons];
            pTilePolygon->flags = 0;
            pTilePolygon->field_32 = 0;

            // draw tile
            auto tile = pOutdoor->DoGetTile(x, z);
            if (!tile) continue;

            // struct Polygon p;
            // auto *poly = &p;

            pTilePolygon->flags = pOutdoor->GetSomeOtherTileInfo(x, z);
            pTilePolygon->field_32 = 0;
            pTilePolygon->field_59 = 1;
            pTilePolygon->sTextureDeltaU = 0;
            pTilePolygon->sTextureDeltaV = 0;

            memcpy(&array_73D150[0], &pTerrainVertices[z * 128 + x],
                sizeof(RenderVertexSoft));  // x, z
            array_73D150[0].u = 0;
            array_73D150[0].v = 0;
            memcpy(&array_73D150[3], &pTerrainVertices[z * 128 + x + 1],
                sizeof(RenderVertexSoft));  // x + 1, z
            array_73D150[3].u = 1;
            array_73D150[3].v = 0;
            memcpy(&array_73D150[2], &pTerrainVertices[(z + 1) * 128 + x + 1],
                sizeof(RenderVertexSoft));  // x + 1, z + 1
            array_73D150[2].u = 1;
            array_73D150[2].v = 1;
            memcpy(&array_73D150[1], &pTerrainVertices[(z + 1) * 128 + x],
                sizeof(RenderVertexSoft));  // x, z + 1
            array_73D150[1].u = 0;
            array_73D150[1].v = 1;

            pTilePolygon->uNumVertices = 4;
            // better tile culling
            pCamera3D->CullFaceToCameraFrustum(array_73D150, &pTilePolygon->uNumVertices, array_73D150, 4);
            if (!pTilePolygon->uNumVertices) continue;

            pTilePolygon->pODMFace = nullptr;
            // pTilePolygon->uNumVertices = 4;
            pTilePolygon->field_59 = 5;

            ++pODMRenderParams->uNumPolygons;
            // ++pODMRenderParams->field_44;
            assert(pODMRenderParams->uNumPolygons < 20000);

            pTilePolygon->uBModelID = 0;
            pTilePolygon->uBModelFaceID = 0;
            pTilePolygon->pid = (8 * (0 | (0 << 6))) | 6;
            for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                memcpy(&VertexRenderList[k], &array_73D150[k], sizeof(struct RenderVertexSoft));
                VertexRenderList[k]._rhw = 1.0 / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097);
            }

            uint norm_idx = pTerrainNormalIndices[(2 * x * 128) + (2 * z) + 2 /*+ 1*/];  // 2 is top tri // 3 is bottom
            uint bottnormidx = pTerrainNormalIndices[(2 * x * 128) + (2 * z) + 3];

            assert(norm_idx < uNumTerrainNormals);
            assert(bottnormidx < uNumTerrainNormals);

            Vec3_float_* norm = &pTerrainNormals[norm_idx];
            Vec3_float_* norm2 = &pTerrainNormals[bottnormidx];

            if (norm_idx < 0 || norm_idx > uNumTerrainNormals - 1)
                norm = 0;
            else
                norm = &pTerrainNormals[norm_idx];

            if (bottnormidx < 0 || bottnormidx > uNumTerrainNormals - 1)
                norm2 = 0;
            else
                norm2 = &pTerrainNormals[bottnormidx];


            if (norm_idx != bottnormidx) {
                // we have a split poly - need to apply lights and decals seperately to each half

                pTilePolygon->uNumVertices = 3;

                ///////////// triangle 1 - 1 2 3

                // verts CCW - for testing
                memcpy(&array_73D150[0], &pTerrainVertices[z * 128 + x],
                    sizeof(RenderVertexSoft));  // x, z
                array_73D150[0].u = 0;
                array_73D150[0].v = 0;
                memcpy(&array_73D150[2], &pTerrainVertices[z * 128 + x + 1],
                    sizeof(RenderVertexSoft));  // x + 1, z
                array_73D150[2].u = 1;
                array_73D150[2].v = 0;
                memcpy(&array_73D150[1], &pTerrainVertices[(z + 1) * 128 + x + 1],
                    sizeof(RenderVertexSoft));  // x + 1, z + 1
                array_73D150[1].u = 1;
                array_73D150[1].v = 1;
                // memcpy(&array_73D150[2], &pTerrainVertices[(z + 1) * 128 + x],
                //    sizeof(RenderVertexSoft));  // x, z + 1
                // array_73D150[2].u = 0;
                // array_73D150[2].v = 1;

                for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                    memcpy(&VertexRenderList[k], &array_73D150[k], sizeof(struct RenderVertexSoft));
                    VertexRenderList[k]._rhw = 1.0 / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097);
                }

                float _f = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);

                lightmap_builder->StackLights_TerrainFace(norm, &Light_tile_dist, VertexRenderList, 3, 1);
                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, 3, 1);

                unsigned int a5 = 4;

                // ---------Draw distance(Дальность отрисовки)-------------------------------
                int far_clip_distance = pCamera3D->GetFarClip();
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
                            4, &static_sub_0048034E_stru_154,
                            3, VertexRenderList,
                            *(float*)&uClipFlag, -1);
                    if (Lights.uNumLightsApplied > 0)
                        lightmap_builder->ApplyLights(
                            &Lights, &static_sub_0048034E_stru_154, 3,
                            VertexRenderList, 0, uClipFlag);
                }







                // pODMRenderParams->shading_dist_mist = temp;

                // check the transparency and texture (tiles) mapping (проверка
                // прозрачности и наложение текстур (тайлов))----------------------

                if (tile->IsWaterTile()) {
                    tile_texture = this->hd_water_tile_anim[this->hd_water_current_frame];
                } else {
                    tile_texture = tile->GetTexture();
                }
                // draw animated water under shore
                bool water_border_tile = false;
                if (tile->IsWaterBorderTile()) {
                    glDepthMask(GL_FALSE);
                    {
                        pTilePolygon->texture =
                            this->hd_water_tile_anim[this->hd_water_current_frame];

                        auto texturew = (TextureOpenGL*)pTilePolygon->texture;
                        GLint thistexw = texturew->GetOpenGlTexture();

                        // avoid rebinding same tex if we can
                        if (GL_lastboundtex != thistexw) {
                            glBindTexture(GL_TEXTURE_2D, thistexw);
                            GL_lastboundtex = thistexw;
                        }

                        this->DrawTerrainPolygon(pTilePolygon, false, true);
                        pTilePolygon->texture = tile->GetTexture();
                    }
                    glDepthMask(GL_TRUE);

                    water_border_tile = true;
                }

                pTilePolygon->texture = tile_texture;
                auto texture = (TextureOpenGL*)pTilePolygon->texture;
                GLint thistex = texture->GetOpenGlTexture();

                // avoid rebinding same tex if we can
                if (GL_lastboundtex != thistex) {
                    glBindTexture(GL_TEXTURE_2D, thistex);
                    GL_lastboundtex = thistex;
                }

                this->DrawTerrainPolygon(pTilePolygon, water_border_tile, true);

                ///////////// triangle 2  0 1 3

                    // verts CCW - for testing
                memcpy(&array_73D150[0], &pTerrainVertices[z * 128 + x],
                    sizeof(RenderVertexSoft));  // x, z
                array_73D150[0].u = 0;
                array_73D150[0].v = 0;
                // memcpy(&array_73D150[2], &pTerrainVertices[z * 128 + x + 1],
                //    sizeof(RenderVertexSoft));  // x + 1, z
                // array_73D150[2].u = 1;
                // array_73D150[2].v = 0;
                memcpy(&array_73D150[2], &pTerrainVertices[(z + 1) * 128 + x + 1],
                    sizeof(RenderVertexSoft));  // x + 1, z + 1
                array_73D150[2].u = 1;
                array_73D150[2].v = 1;
                memcpy(&array_73D150[1], &pTerrainVertices[(z + 1) * 128 + x],
                    sizeof(RenderVertexSoft));  // x, z + 1
                array_73D150[1].u = 0;
                array_73D150[1].v = 1;

                for (unsigned int k = 0; k < pTilePolygon->uNumVertices; ++k) {
                    memcpy(&VertexRenderList[k], &array_73D150[k], sizeof(struct RenderVertexSoft));
                    VertexRenderList[k]._rhw = 1.0 / (array_73D150[k].vWorldViewPosition.x + 0.0000001000000011686097);
                }

                float _f2 = norm2->x * pOutdoor->vSunlight.x + norm2->y * pOutdoor->vSunlight.y + norm2->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f2 + 0.5f);


                lightmap_builder->StackLights_TerrainFace(norm2, &Light_tile_dist, VertexRenderList, 3, 0);

                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm2,
                    &Light_tile_dist, VertexRenderList, 3, 0);


                unsigned int a5_2 = 4;

                // ---------Draw distance(Дальность отрисовки)-------------------------------
                int far_clip_distance_2 = pCamera3D->GetFarClip();
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
                            4, &static_sub_0048034E_stru_154_2,
                            3, VertexRenderList,
                            *(float*)&uClipFlag_2, -1);
                    if (Lights.uNumLightsApplied > 0)
                        lightmap_builder->ApplyLights(
                            &Lights, &static_sub_0048034E_stru_154_2, 3,
                            VertexRenderList, 0, uClipFlag_2);
                }







                // pODMRenderParams->shading_dist_mist = temp;

                // check the transparency and texture (tiles) mapping (проверка
                // прозрачности и наложение текстур (тайлов))----------------------

                if (tile->IsWaterTile()) {
                    tile_texture = this->hd_water_tile_anim[this->hd_water_current_frame];
                } else {
                    tile_texture = tile->GetTexture();
                }

                // draw animated water under shore
                water_border_tile = false;
                if (tile->IsWaterBorderTile()) {
                    glDepthMask(GL_FALSE);
                    {
                        pTilePolygon->texture =
                            this->hd_water_tile_anim[this->hd_water_current_frame];

                        auto texturew = (TextureOpenGL*)pTilePolygon->texture;
                        GLint thistexw = texturew->GetOpenGlTexture();

                        // avoid rebinding same tex if we can
                        if (GL_lastboundtex != thistexw) {
                            glBindTexture(GL_TEXTURE_2D, thistexw);
                            GL_lastboundtex = thistexw;
                        }

                        this->DrawTerrainPolygon(pTilePolygon, false, true);
                        pTilePolygon->texture = tile->GetTexture();
                    }
                    glDepthMask(GL_TRUE);

                    water_border_tile = true;
                }

                pTilePolygon->texture = tile_texture;
                texture = (TextureOpenGL*)pTilePolygon->texture;
                thistex = texture->GetOpenGlTexture();

                // avoid rebinding same tex if we can
                if (GL_lastboundtex != thistex) {
                    glBindTexture(GL_TEXTURE_2D, thistex);
                    GL_lastboundtex = thistex;
                }

                this->DrawTerrainPolygon(pTilePolygon, water_border_tile, true);

                // end split trinagles
            } else {
                float _f = norm->x * pOutdoor->vSunlight.x + norm->y * pOutdoor->vSunlight.y + norm->z * pOutdoor->vSunlight.z;
                pTilePolygon->dimming_level = 20.0 - floorf(20.0 * _f + 0.5f);

                lightmap_builder->StackLights_TerrainFace(norm, &Light_tile_dist, VertexRenderList, pTilePolygon->uNumVertices, 1);
                decal_builder->ApplyBloodSplatToTerrain(pTilePolygon, norm, &Light_tile_dist, VertexRenderList, pTilePolygon->uNumVertices, 1);

                unsigned int a5 = 4;

                // ---------Draw distance(Дальность отрисовки)-------------------------------
               // int far_clip_distance = pCamera3D->GetFarClip();
                // float near_clip_distance = pCamera3D->GetNearClip();

               // if (engine->config->extended_draw_distance)
               //     far_clip_distance = 0x5000;
               // bool neer_clip = array_73D150[0].vWorldViewPosition.x < near_clip_distance ||
               //     array_73D150[1].vWorldViewPosition.x < near_clip_distance ||
               //     array_73D150[2].vWorldViewPosition.x < near_clip_distance ||
               //     array_73D150[3].vWorldViewPosition.x < near_clip_distance;
               // bool far_clip =
               //     (float)far_clip_distance < array_73D150[0].vWorldViewPosition.x ||
                //    (float)far_clip_distance < array_73D150[1].vWorldViewPosition.x ||
                //    (float)far_clip_distance < array_73D150[2].vWorldViewPosition.x ||
                //    (float)far_clip_distance < array_73D150[3].vWorldViewPosition.x;

                int uClipFlag = 0;
                static stru154 static_sub_0048034E_stru_154;
                lightmap_builder->StationaryLightsCount = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumSplatsThisFace > 0) {
                    // if (neer_clip)
                     //   uClipFlag = 3;
                    // else
                        // uClipFlag = far_clip != 0 ? 5 : 0;
                    static_sub_0048034E_stru_154.ClassifyPolygon(norm, Light_tile_dist);

                    if (decal_builder->uNumSplatsThisFace > 0)
                        decal_builder->BuildAndApplyDecals(31 - pTilePolygon->dimming_level,
                            4, &static_sub_0048034E_stru_154,
                            a5, VertexRenderList,
                            *(float*)&uClipFlag, -1);

                    if (Lights.uNumLightsApplied > 0)
                        lightmap_builder->ApplyLights(&Lights, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, uClipFlag);
                }


                // pODMRenderParams->shading_dist_mist = temp;

                // check the transparency and texture (tiles) mapping (проверка
                // прозрачности и наложение текстур (тайлов))----------------------


                if (tile->IsWaterTile()) {
                    tile_texture = this->hd_water_tile_anim[this->hd_water_current_frame];
                } else {
                    tile_texture = tile->GetTexture();
                }
                // draw animated water under shore
                bool water_border_tile = false;
                if (tile->IsWaterBorderTile()) {
                    glDepthMask(GL_FALSE);
                    {
                        pTilePolygon->texture =
                            this->hd_water_tile_anim[this->hd_water_current_frame];

                        auto texturew = (TextureOpenGL*)pTilePolygon->texture;
                        GLint thistexw = texturew->GetOpenGlTexture();

                        // avoid rebinding same tex if we can
                        if (GL_lastboundtex != thistexw) {
                            glBindTexture(GL_TEXTURE_2D, thistexw);
                            GL_lastboundtex = thistexw;
                        }

                        this->DrawTerrainPolygon(pTilePolygon, false, true);
                        pTilePolygon->texture = tile->GetTexture();
                    }
                    glDepthMask(GL_TRUE);

                    water_border_tile = true;
                }

                pTilePolygon->texture = tile_texture;
                auto texture = (TextureOpenGL*)pTilePolygon->texture;
                GLint thistex = texture->GetOpenGlTexture();

                // avoid rebinding same tex if we can
                if (GL_lastboundtex != thistex) {
                    glBindTexture(GL_TEXTURE_2D, thistex);
                    GL_lastboundtex = thistex;
                }

                this->DrawTerrainPolygon(pTilePolygon, water_border_tile, true);
            }
            // end norm split
        }
    }
}



void RenderOpenGL::DrawTerrainPolygon(struct Polygon *poly, bool transparent, bool clampAtTextureBorders) {
    int v11;           // eax@5

    unsigned int uNumVertices = poly->uNumVertices;

    auto texture = (TextureOpenGL*)poly->texture;

    // if (!this->uNumD3DSceneBegins) return;
    if (uNumVertices < 3) return;

    if (_4D864C_force_sw_render_rules && engine->config->Flag1_1()) {
        v11 =
            ::GetActorTintColor(poly->dimming_level, 0,
                VertexRenderList[0].vWorldViewPosition.x, 0, 0);
        lightmap_builder->DrawLightmaps(v11 /*, 0*/);
    } else if (transparent || !lightmap_builder->StationaryLightsCount ||
        _4D864C_force_sw_render_rules && engine->config->Flag1_2()) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldPosition.x;
            d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldPosition.y;
            d3d_vertex_buffer[i].pos.z = VertexRenderList[i].vWorldPosition.z;
            d3d_vertex_buffer[i].rhw =
                1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
            d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(
                poly->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x,
                0, 0);
            d3d_vertex_buffer[i].specular = 0;
            if (config->is_using_specular)
                d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                    0, 0, VertexRenderList[i].vWorldViewPosition.x);

            d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
            d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
        }

        glBegin(GL_TRIANGLE_FAN);  // GL_TRIANGLE_FAN

        for (uint i = 0; i < uNumVertices; ++i) {
            glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);

            glColor4f(
                ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                config->is_using_specular
                ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                : 1.0f);

            glVertex3f(d3d_vertex_buffer[i].pos.x,
                d3d_vertex_buffer[i].pos.y,
                d3d_vertex_buffer[i].pos.z);
        }

        glEnd();

        drawcalls++;
    } else if (lightmap_builder->StationaryLightsCount) {
        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].pos.x = VertexRenderList[i].vWorldPosition.x;
            d3d_vertex_buffer[i].pos.y = VertexRenderList[i].vWorldPosition.y;
            d3d_vertex_buffer[i].pos.z = VertexRenderList[i].vWorldPosition.z;
            d3d_vertex_buffer[i].rhw =
                1.0 / (VertexRenderList[i].vWorldViewPosition.x + 0.0000001);
            d3d_vertex_buffer[i].diffuse = GetActorTintColor(poly->dimming_level, 0, VertexRenderList[i].vWorldViewPosition.x, 0, 0);
            d3d_vertex_buffer[i].specular = 0;
            if (config->is_using_specular)
                d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                    0, 0, VertexRenderList[i].vWorldViewPosition.x);
            d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
            d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
        }

        glDepthMask(false);
        int savetex = GL_lastboundtex;
        glBindTexture(GL_TEXTURE_2D, 0);

        glBegin(GL_TRIANGLE_FAN);  // GL_TRIANGLE_FAN

        for (uint i = 0; i < uNumVertices; ++i) {
            glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);

            glColor4f(
                ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                config->is_using_specular
                ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                : 1.0f);

            glVertex3f(d3d_vertex_buffer[i].pos.x,
                d3d_vertex_buffer[i].pos.y,
                d3d_vertex_buffer[i].pos.z);
        }

        glEnd();

        drawcalls++;

        glDisable(GL_CULL_FACE);

        lightmap_builder->DrawLightmaps(-1 /*, 0*/);

        for (uint i = 0; i < uNumVertices; ++i) {
            d3d_vertex_buffer[i].diffuse = 0xFFFFFFFF; /*-1;*/
        }

        glBindTexture(GL_TEXTURE_2D, GL_lastboundtex);

        glDepthMask(true);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);

            glBegin(GL_TRIANGLE_FAN);  // GL_TRIANGLE_FAN

            for (uint i = 0; i < uNumVertices; ++i) {
                glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);

                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    config->is_using_specular
                    ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                    : 1.0f);

                glVertex3f(d3d_vertex_buffer[i].pos.x,
                    d3d_vertex_buffer[i].pos.y,
                    d3d_vertex_buffer[i].pos.z);
            }

            glEnd();


        drawcalls++;

        if (!transparent) glDisable(GL_BLEND);
    }

    // if (pIndoorCamera->flags & INDOOR_CAMERA_DRAW_TERRAIN_OUTLINES ||
    // pBLVRenderParams->uFlags & INDOOR_CAMERA_DRAW_TERRAIN_OUTLINES) if
    // (pCamera3D->debug_flags & ODM_RENDER_DRAW_TERRAIN_OUTLINES)
    if (engine->config->debug_terrain)
        pCamera3D->debug_outline_d3d(d3d_vertex_buffer, uNumVertices, 0x00FFFFFF, 0.0);
}

void RenderOpenGL::DrawOutdoorSkyD3D() {
    double rot_to_rads = ((2 * pi_double) / 2048);

    // lowers clouds as party goes up
    float  horizon_height_offset = ((double)(pCamera3D->ViewPlaneDist_X * pCamera3D->vCameraPos.z)
        / ((double)pCamera3D->ViewPlaneDist_X + 8192.0)
        + (double)(pViewport->uScreenCenterY));

    // magnitude in up direction
    float cam_vec_up = cos((double)pCamera3D->sRotationY * rot_to_rads) *
    pCamera3D->GetFarClip();

    float bot_y_proj = ((double)(pViewport->uScreenCenterY) -
        (double)pCamera3D->ViewPlaneDist_X /
        (cam_vec_up + 0.0000001) *
        (sin((double)pCamera3D->sRotationY * rot_to_rads)
            *
            pCamera3D->GetFarClip() -
            (double)pCamera3D->vCameraPos.z));

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
        // plane of sky polygon rotation vector
        float v18x, v18y, v18z;
        /*pSkyPolygon.v_18.x*/ v18x = -TrigLUT->Sin(-pCamera3D->sRotationY + 16) / 65536.0;
        /*pSkyPolygon.v_18.y*/ v18y = 0;
        /*pSkyPolygon.v_18.z*/ v18z = -TrigLUT->Cos(pCamera3D->sRotationY + 16) / 65536.0;

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

        double half_fov_angle_rads = ((pCamera3D->odm_fov_deg) * pi_double) / 360;

        // far width per pixel??
        float widthperpixel = 1 /
            (((double)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) / 2)
                / tan(half_fov_angle_rads) +
                0.5);

        for (uint i = 0; i < pSkyPolygon.uNumVertices; ++i) {
            // rotate skydome(вращение купола
            // неба)--------------------------------------
            // В игре принята своя система измерения углов. Полный угол (180).
            // Значению угла 0 соответствует направление на север и/или юг (либо
            // на восток и/или запад), значению 65536 еденицам(0х10000)
            // соответствует угол 90. две переменные хранят данные по углу
            // обзора. field_14 по западу и востоку. field_20 по югу и северу от
            // -25080 до 25080

            float v13 = widthperpixel * (pViewport->uScreenCenterX - VertexRenderList[i].vWorldViewProjX);


            float v39 = (pSkyPolygon.ptr_38->CamVecLeft_Y * widthperpixel * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));
            float v35 = v39 + pSkyPolygon.ptr_38->CamVecLeft_Z;

            float skyfinalleft = v35 + (pSkyPolygon.ptr_38->CamVecLeft_X * v13);

            v39 = (pSkyPolygon.ptr_38->CamVecFront_Y * widthperpixel * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.f)));
            float v36 = v39 + pSkyPolygon.ptr_38->CamVecFront_Z;

            float finalskyfront = v36 + (pSkyPolygon.ptr_38->CamVecFront_X * v13);


            float v9 = (/*pSkyPolygon.v_18.z*/v18z * widthperpixel * (horizon_height_offset - floor(VertexRenderList[i].vWorldViewProjY + 0.5)));




            float top_y_proj = /*pSkyPolygon.v_18.x*/v18x + v9;
            if (top_y_proj > 0) top_y_proj = 0;

            /* v32 = (signed __int64)VertexRenderList[i].vWorldViewProjY - 1.0; */
            // v14 = widthperpixel * (horizon_height_offset - v32);
            // while (1) {
            //    if (top_y_proj) {
            //        v37 = 0.03125;  // abs((int)cam_vec_up >> 14);
            //        v15 = abs(top_y_proj);
            //        if (v37 <= v15 ||
            //            v32 <= (signed int)pViewport->uViewportTL_Y) {
            //            if (top_y_proj <= 0) break;
            //        }
            //    }
            //    v16 = (/*pSkyPolygon.v_18.z*/v18z * v14);  // does this bit ever get called?
            //    --v32;
            //    v14 += widthperpixel;
            //    top_y_proj = /*pSkyPolygon.v_18.x*/v18x + v16;
            // }

            float worldviewdepth = -64.0 / top_y_proj;
            if (worldviewdepth < 0) worldviewdepth = pCamera3D->GetFarClip();

            float texoffset_U = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((skyfinalleft * worldviewdepth));
            VertexRenderList[i].u = texoffset_U / ((float)pSkyPolygon.texture->GetWidth());

            float texoffset_V = (float(pMiscTimer->uTotalGameTimeElapsed) / 128.0) + ((finalskyfront * worldviewdepth));
            VertexRenderList[i].v = texoffset_V / ((float)pSkyPolygon.texture->GetHeight());

            VertexRenderList[i].vWorldViewPosition.x = pCamera3D->GetFarClip();
            VertexRenderList[i]._rhw = 1.0 / (double)(worldviewdepth);
        }

        _set_ortho_projection(1);
        _set_ortho_modelview();

        DrawOutdoorSkyPolygon(&pSkyPolygon);

        // adjust and draw again to fill gap below horizon
        // could mirror over??

        // VertexRenderList[0].vWorldViewProjY += 60;
        // VertexRenderList[1].vWorldViewProjY += 60;
        // VertexRenderList[2].vWorldViewProjY += 60;
        // VertexRenderList[3].vWorldViewProjY += 60;

        // DrawOutdoorSkyPolygon(&pSkyPolygon);
    }
}

//----- (004A2DA3) --------------------------------------------------------
void RenderOpenGL::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) {
    auto texture = (TextureOpenGL *)pSkyPolygon->texture;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    VertexRenderList[0].u = 0 - (float)pParty->sRotationZ / 512;
    VertexRenderList[1].u = 0 - (float)pParty->sRotationZ / 512;
    VertexRenderList[2].u = 1 - (float)pParty->sRotationZ / 512;
    VertexRenderList[3].u = 1 - (float)pParty->sRotationZ / 512;

    if (pParty->sRotationY > 0) {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationY / 1024;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationY / 1024;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationY / 1024;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationY / 1024;
    } else {
        VertexRenderList[0].v = 0 - (float)pParty->sRotationY / 256;
        VertexRenderList[1].v = 1 - (float)pParty->sRotationY / 256;
        VertexRenderList[2].v = 1 - (float)pParty->sRotationY / 256;
        VertexRenderList[3].v = 0 - (float)pParty->sRotationY / 256;
    }

    glBegin(GL_TRIANGLE_FAN);
    {
        for (int i = 0; i < pSkyPolygon->uNumVertices; ++i) {
            unsigned int diffuse = ::GetActorTintColor(
                31, 0, VertexRenderList[i].vWorldViewPosition.x, 1, 0);

            glColor4f(((diffuse >> 16) & 0xFF) / 255.0f,
                      ((diffuse >> 8) & 0xFF) / 255.0f,
                      (diffuse & 0xFF) / 255.0f, 1.0f);

            glTexCoord2f(VertexRenderList[i].u,
                         /*max_v*/ /*-*/VertexRenderList[i].v);

            glVertex3f(VertexRenderList[i].vWorldViewProjX,
                VertexRenderList[i].vWorldViewProjY,
                0.99989998);  // z is negative in OpenGL
        }
    }
    drawcalls++;
    glEnd();
}

void RenderOpenGL::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {
    engine->draw_debug_outlines();
    this->DoRenderBillboards_D3D();
    spell_fx_renderer->RenderSpecialEffects();
}

//----- (004A1C1E) --------------------------------------------------------
void RenderOpenGL::DoRenderBillboards_D3D() {
    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);  // some quads are reversed to reuse sprites opposite hand
    glEnable(GL_TEXTURE_2D);

    _set_ortho_projection(1);
    _set_ortho_modelview();

    for (int i = uNumBillboardsToDraw - 1; i >= 0; --i) {
        if (pBillboardRenderListD3D[i].opacity != RenderBillboardD3D::NoBlend) {
            SetBillboardBlendOptions(pBillboardRenderListD3D[i].opacity);
        }

        auto texture = (TextureOpenGL *)pBillboardRenderListD3D[i].texture;
        glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

        glBegin(GL_TRIANGLE_FAN);
        {
            auto billboard = &pBillboardRenderListD3D[i];
            auto b = &pBillboardRenderList[i];

            // since OpenGL 1.0 can't mirror texture borders, we should offset
            // UV to avoid black edges
            billboard->pQuads[0].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[0].texcoord.y += 0.5f / texture->GetHeight();
            billboard->pQuads[1].texcoord.x += 0.5f / texture->GetWidth();
            billboard->pQuads[1].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[2].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[2].texcoord.y -= 0.5f / texture->GetHeight();
            billboard->pQuads[3].texcoord.x -= 0.5f / texture->GetWidth();
            billboard->pQuads[3].texcoord.y += 0.5f / texture->GetHeight();

            for (unsigned int j = 0; j < billboard->uNumVertices; ++j) {
                glColor4f(
                    ((billboard->pQuads[j].diffuse >> 16) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 8) & 0xFF) / 255.0f,
                    ((billboard->pQuads[j].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f);

                glTexCoord2f(billboard->pQuads[j].texcoord.x,
                             billboard->pQuads[j].texcoord.y);

                float oneoz = 1. / (billboard->screen_space_z);
                float oneon = 1. / (pCamera3D->GetNearClip()+4);
                float oneof = 1. / pCamera3D->GetFarClip();

                glVertex3f(
                    billboard->pQuads[j].pos.x,
                    billboard->pQuads[j].pos.y,
                    (oneoz - oneon)/(oneof - oneon) );  // depth is  non linear  proportional to reciprocal of distance
            }
        }
        drawcalls++;
        glEnd();
    }

    // uNumBillboardsToDraw = 0;


    if (config->is_using_fog) {
        SetUsingFog(false);
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP);

        GLfloat fog_color[] = {((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
                               ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
                               ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f,
                               1.0f};
        glFogfv(GL_FOG_COLOR, fog_color);
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

//----- (004A1DA8) --------------------------------------------------------
void RenderOpenGL::SetBillboardBlendOptions(RenderBillboardD3D::OpacityType a1) {
    switch (a1) {
        case RenderBillboardD3D::Transparent: {
            if (config->is_using_fog) {
                SetUsingFog(false);
                glEnable(GL_FOG);
                glFogi(GL_FOG_MODE, GL_EXP);

                GLfloat fog_color[] = {
                    ((GetLevelFogColor() >> 16) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 8) & 0xFF) / 255.0f,
                    ((GetLevelFogColor() >> 0) & 0xFF) / 255.0f, 1.0f};
                glFogfv(GL_FOG_COLOR, fog_color);
            }

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } break;

        case RenderBillboardD3D::Opaque_1:
        case RenderBillboardD3D::Opaque_2:
        case RenderBillboardD3D::Opaque_3: {
            if (config->is_using_specular) {
                if (!config->is_using_fog) {
                    SetUsingFog(true);
                    glDisable(GL_FOG);
                }
            }

            glBlendFunc(GL_ONE, GL_ONE);  // zero
        } break;

        default:
            log->Warning(
                "SetBillboardBlendOptions: invalid opacity type (%u)", a1);
            assert(false);
            break;
    }
}

void RenderOpenGL::SetUIClipRect(unsigned int x, unsigned int y, unsigned int z,
                                 unsigned int w) {
    this->clip_x = x;
    this->clip_y = y;
    this->clip_z = z;
    this->clip_w = w;
    glScissor(x, this->window->GetHeight() -w, z-x, w-y);  // invert glscissor co-ords 0,0 is BL
}

void RenderOpenGL::ResetUIClipRect() {
    this->SetUIClipRect(0, 0, this->window->GetWidth(), this->window->GetHeight());
}

void RenderOpenGL::PresentBlackScreen() {
    BeginScene();
    ClearBlack();
    EndScene();
    Present();
}

void RenderOpenGL::BeginScene() {
    // Setup for 2D

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    _set_ortho_projection();
    _set_ortho_modelview();
}

void RenderOpenGL::EndScene() {
    // blank in d3d
}



void RenderOpenGL::DrawTextureAlphaNew(float u, float v, Image *img) {
    DrawTextureNew(u, v, img);
    return;
}

void RenderOpenGL::DrawTextureNew(float u, float v, Image *tex) {
    if (!tex) __debugbreak();

    TextureOpenGL* texture = dynamic_cast<TextureOpenGL*>(tex);
    if (!texture) {
        __debugbreak();
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

    int clipx = this->clip_x;
    int clipy = this->clip_y;
    int clipw = this->clip_w;
    int clipz = this->clip_z;

    int width = tex->GetWidth();
    int height = tex->GetHeight();

    int x = u * window->GetWidth();
    int y = v * window->GetHeight();
    int z = x + width;
    int w = y + height;

    // check bounds
    if (x >= (int)window->GetWidth() || x >= clipz || y >= (int)window->GetHeight() || y >= clipw) return;
    // check for overlap
    if ((clipx < z && clipz > x && clipy > w && clipw < y)) return;

    int drawx = std::max(x, clipx);
    int drawy = std::max(y, clipy);
    int draww = std::min(w, clipw);
    int drawz = std::min(z, clipz);
    if (drawz <= drawx || draww <= drawy) return;

    float depth = 0;

    GLfloat Vertices[] = { (float)drawx, (float)drawy, depth,
        (float)drawz, (float)drawy, depth,
        (float)drawz, (float)draww, depth,
        (float)drawx, (float)draww, depth };

    float texx = (drawx - x) / float(width);
    float texy = (drawy - y) / float(height);
    float texz = (width - (z - drawz)) / float(width);
    float texw = (height - (w - draww)) / float(height);

    GLfloat TexCoord[] = { texx, texy,
        texz, texy,
        texz, texw,
        texx, texw,
    };

     GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    drawcalls++;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning("OpenGL: draw texture error: (%u)", err);
    }

    // blank over same bit of this render_target_rgb to stop text overlaps
    for (int ys = drawy; ys < draww; ys++) {
        memset(this->render_target_rgb +(ys * window->GetWidth() + drawx), 0x00000000, (drawz - drawx) * 4);
    }
}

void RenderOpenGL::DrawTextureCustomHeight(float u, float v, class Image *img,
                                           int custom_height) {
    unsigned __int16 *v6;  // esi@3
    unsigned int v8;       // eax@5
    unsigned int v11;      // eax@7
    unsigned int v12;      // ebx@8
    unsigned int v15;      // eax@14
    int v19;               // [sp+10h] [bp-8h]@3

    if (!img) return;

    unsigned int uOutX = window->GetWidth() * u;
    unsigned int uOutY = window->GetHeight() * v;

    int width = img->GetWidth();
    int height = std::min((int)img->GetHeight(), custom_height);
    v6 = (unsigned __int16 *)img->GetPixels(IMAGE_FORMAT_R5G6B5);

    // v5 = &this->pTargetSurface[uOutX + uOutY * this->uTargetSurfacePitch];
    v19 = width;
    // if (this->bClip)
    {
        if ((signed int)uOutX < (signed int)this->clip_x) {
            v8 = this->clip_x - uOutX;
            unsigned int v9 = uOutX - this->clip_x;
            v8 *= 2;
            width += v9;
            v6 = (unsigned __int16 *)((char *)v6 + v8);
            // v5 = (unsigned __int16 *)((char *)v5 + v8);
        }
        if ((signed int)uOutY < (signed int)this->clip_y) {
            v11 = this->clip_y - uOutY;
            v6 += v19 * v11;
            height += uOutY - this->clip_y;
            // v5 += this->uTargetSurfacePitch * v11;
        }
        v12 = std::max((unsigned int)this->clip_x, uOutX);
        if ((signed int)(width + v12) > (signed int)this->clip_z) {
            width = this->clip_z - std::max(this->clip_x, (int)uOutX);
        }
        v15 = std::max((unsigned int)this->clip_y, uOutY);
        if ((signed int)(v15 + height) > (signed int)this->clip_w) {
            height = this->clip_w - std::max(this->clip_y, (int)uOutY);
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            WritePixel16(uOutX + x, uOutY + y, *v6);
            // *v5 = *v6;
            // ++v5;
            ++v6;
        }
        v6 += v19 - width;
        // v5 += this->uTargetSurfacePitch - v4;
    }
}

void RenderOpenGL::DrawText(int uOutX, int uOutY, uint8_t* pFontPixels,
                            unsigned int uCharWidth, unsigned int uCharHeight,
                            uint8_t* pFontPalette, uint16_t uFaceColor,
                            uint16_t uShadowColor) {
    // needs limits checks adding

    // Image *fonttemp = Image::Create(uCharWidth, uCharHeight, IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *fontpix = (uint32_t*)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    for (uint y = 0; y < uCharHeight; ++y) {
        for (uint x = 0; x < uCharWidth; ++x) {
            if (*pFontPixels) {
                uint16_t color = uShadowColor;
                if (*pFontPixels != 1) {
                    color = uFaceColor;
                }
                // fontpix[x + y * uCharWidth] = Color32(color);
                this->render_target_rgb[(uOutX+x)+(uOutY+y)*window->GetWidth()] = Color32(color);
            }
            ++pFontPixels;
        }
    }
    // render->DrawTextureAlphaNew(uOutX / 640., uOutY / 480., fonttemp);
    // fonttemp->Release();
}

void RenderOpenGL::DrawTextAlpha(int x, int y, unsigned char *font_pixels,
                                 int uCharWidth, unsigned int uFontHeight,
                                 uint8_t *pPalette,
                                 bool present_time_transparency) {
    // needs limits checks adding

    // Image *fonttemp = Image::Create(uCharWidth, uFontHeight, IMAGE_FORMAT_A8R8G8B8);
    // uint32_t *fontpix = (uint32_t *)fonttemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

    if (present_time_transparency) {
        for (unsigned int dy = 0; dy < uFontHeight; ++dy) {
            for (unsigned int dx = 0; dx < uCharWidth; ++dx) {
                uint16_t color = (*font_pixels)
                    ? pPalette[*font_pixels]
                    : teal_mask_16;  // transparent color 16bit
                              // render->uTargetGMask |
                              // render->uTargetBMask;
                this->render_target_rgb[(x + dx) + (y + dy) * window->GetWidth()] = Color32(color);
                // fontpix[dx + dy * uCharWidth] = Color32(color);
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
                    this->render_target_rgb[(x + dx) + (y + dy) * window->GetWidth()] = Color32(r, g, b);
                    // fontpix[dx + dy * uCharWidth] = Color32(r, g, b);
                }
                ++font_pixels;
            }
        }
    }
    // render->DrawTextureAlphaNew(x / 640., y / 480., fonttemp);
    // fonttemp->Release();
}

void RenderOpenGL::Present() {
    // screen overlay holds all text and changing images at the moment

    static Texture *screen_text_overlay = 0;
    if (!screen_text_overlay) {
        screen_text_overlay = render->CreateTexture_Blank(window->GetWidth(), window->GetHeight(), IMAGE_FORMAT_A8R8G8B8);
    }

    uint32_t *pix = (uint32_t*)screen_text_overlay->GetPixels(IMAGE_FORMAT_A8R8G8B8);
    unsigned int num_pixels = screen_text_overlay->GetWidth() * screen_text_overlay->GetHeight();
    unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A8R8G8B8);

    // update pixels
    memcpy(pix, this->render_target_rgb, num_pixels_bytes);
    // update texture
    render->Update_Texture(screen_text_overlay);
    // draw
    render->DrawTextureAlphaNew(0, 0, screen_text_overlay);

    window->OpenGlSwapBuffers();
}

// RenderVertexSoft ogl_draw_buildings_vertices[20];


void RenderOpenGL::DrawBuildingsD3D() {
    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // int v27;  // eax@57
    int farclip;  // [sp+2Ch] [bp-2Ch]@10
    int nearclip;  // [sp+30h] [bp-28h]@34
    int v51;  // [sp+34h] [bp-24h]@35
    int v52;  // [sp+38h] [bp-20h]@36
    int v53;  // [sp+3Ch] [bp-1Ch]@8

    for (BSPModel& model : pOutdoor->pBModels) {
        bool reachable_unused;
        if (!IsBModelVisible(&model, 256, &reachable_unused)) {
            continue;
        }
        model.field_40 |= 1;
        if (model.pFaces.empty()) {
            continue;
        }

        for (ODMFace& face : model.pFaces) {
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

            unsigned int flow_anim_timer = OS_GetTime() >> 4;
            unsigned int flow_u_mod = poly->texture->GetWidth() - 1;
            unsigned int flow_v_mod = poly->texture->GetHeight() - 1;

            if (face.pFacePlane.vNormal.z && abs(face.pFacePlane.vNormal.z) >= 0.9) {
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

            for (uint vertex_id = 1; vertex_id <= face.uNumVertices; vertex_id++) {
                array_73D150[vertex_id - 1].vWorldPosition.x = model.pVertices.pVertices[face.pVertexIDs[vertex_id - 1]].x;
                array_73D150[vertex_id - 1].vWorldPosition.y = model.pVertices.pVertices[face.pVertexIDs[vertex_id - 1]].y;
                array_73D150[vertex_id - 1].vWorldPosition.z = model.pVertices.pVertices[face.pVertexIDs[vertex_id - 1]].z;
                array_73D150[vertex_id - 1].u = (poly->sTextureDeltaU +
                        (__int16)face.pTextureUIDs[vertex_id - 1]) *
                    (1.0 / (double)poly->texture->GetWidth());
                array_73D150[vertex_id - 1].v = (poly->sTextureDeltaV +
                        (__int16)face.pTextureVIDs[vertex_id - 1]) *
                    (1.0 / (double)poly->texture->GetHeight());
            }

            for (uint i = 1; i <= face.uNumVertices; i++) {
                if (model.pVertices.pVertices[face.pVertexIDs[0]].z ==
                    array_73D150[i - 1].vWorldPosition.z)
                    ++v53;
                pCamera3D->ViewTransform(&array_73D150[i - 1], 1);
                //if (array_73D150[i - 1].vWorldViewPosition.x <
                //    pCamera3D->GetNearClip() ||
                //    array_73D150[i - 1].vWorldViewPosition.x >
                //    pCamera3D->GetFarClip()) {
                //    if (array_73D150[i - 1].vWorldViewPosition.x >=
                //        pCamera3D->GetNearClip())
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
                poly->pid =
                    PID(OBJECT_BModel, (face.index | (model.index << 6)));
                for (int vertex_id = 0; vertex_id < face.uNumVertices;
                    ++vertex_id) {
                    memcpy(&VertexRenderList[vertex_id],
                        &array_73D150[vertex_id],
                        sizeof(VertexRenderList[vertex_id]));
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

                    // if (face.uAttributes & FACE_OUTLINED) __debugbreak();

                    static_RenderBuildingsD3D_stru_73C834.GetFacePlaneAndClassify(&face, &model.pVertices);
                    if (decal_builder->uNumSplatsThisFace > 0) {
                        decal_builder->BuildAndApplyDecals(
                            31 - poly->dimming_level, 2,
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

                // if (nearclip) {
                //    poly->uNumVertices = ODM_NearClip(face.uNumVertices);
                //    ODM_Project(poly->uNumVertices);
                // }
                // if (farclip) {
                //    poly->uNumVertices = ODM_FarClip(face.uNumVertices);
                //    ODM_Project(poly->uNumVertices);
                // }

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
}

void RenderOpenGL::DrawPolygon(struct Polygon *pPolygon) {
    if (pPolygon->uNumVertices < 3) {
        return;
    }

    unsigned int sCorrectedColor;  // [sp+64h] [bp-4h]@4

    auto texture = (TextureOpenGL*)pPolygon->texture;
    ODMFace* pFace = pPolygon->pODMFace;
    auto uNumVertices = pPolygon->uNumVertices;

    if (lightmap_builder->StationaryLightsCount) {
        sCorrectedColor = -1;
    }
    engine->AlterGamma_ODM(pFace, &sCorrectedColor);
    if (_4D864C_force_sw_render_rules && engine->config->Flag1_1()) {
        int v8 = ::GetActorTintColor(
            pPolygon->dimming_level, 0,
            VertexRenderList[0].vWorldViewPosition.x, 0, 0);
        lightmap_builder->DrawLightmaps(v8 /*, 0*/);
    } else {
        if (!lightmap_builder->StationaryLightsCount ||
            _4D864C_force_sw_render_rules && engine->config->Flag1_2()) {
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBegin(GL_TRIANGLE_FAN);

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
                d3d_vertex_buffer[i].diffuse = ::GetActorTintColor(
                    pPolygon->dimming_level, 0,
                    VertexRenderList[i].vWorldViewPosition.x, 0, 0);
                engine->AlterGamma_ODM(pFace, &d3d_vertex_buffer[i].diffuse);

                if (config->is_using_specular)
                    d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[i].vWorldViewPosition.x);
                else
                    d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
                d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
                //}

                if (pFace->uAttributes & FACE_OUTLINED) {
                    int color;
                    if (OS_GetTime() % 300 >= 150)
                        color = 0xFFFF2020;
                    else
                        color = 0xFF901010;

                    // for (uint i = 0; i < uNumVertices; ++i)
                    d3d_vertex_buffer[i].diffuse = color;
                }



                glTexCoord2f(VertexRenderList[i].u,
                    VertexRenderList[i].v);

                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    config->is_using_specular
                    ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                    : 1.0f);

                glVertex3f(VertexRenderList[i].vWorldPosition.x,
                    VertexRenderList[i].vWorldPosition.y,
                    VertexRenderList[i].vWorldPosition.z);
            }

            drawcalls++;
            glEnd();

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
                if (config->is_using_specular)
                    d3d_vertex_buffer[i].specular = sub_47C3D7_get_fog_specular(
                        0, 0, VertexRenderList[i].vWorldViewPosition.x);
                else
                    d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x = VertexRenderList[i].u;
                d3d_vertex_buffer[i].texcoord.y = VertexRenderList[i].v;
            }

            glDepthMask(false);
            glBindTexture(GL_TEXTURE_2D, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBegin(GL_TRIANGLE_FAN);  // GL_TRIANGLE_FAN

            for (uint i = 0; i < uNumVertices; ++i) {
                glTexCoord2f(VertexRenderList[i].u, VertexRenderList[i].v);

                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    config->is_using_specular
                    ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                    : 1.0f);

                glVertex3f(VertexRenderList[i].vWorldPosition.x,
                    VertexRenderList[i].vWorldPosition.y,
                    VertexRenderList[i].vWorldPosition.z);
            }

            glEnd();


            drawcalls++;

            glDisable(GL_CULL_FACE);

            // (*(void (**)(void))(*(int *)v50 + 88))();
            lightmap_builder->DrawLightmaps(-1);
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
            }

            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

            glDepthMask(true);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ZERO, GL_SRC_COLOR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glBegin(GL_TRIANGLE_FAN);

                for (uint i = 0; i < uNumVertices; ++i) {
                    glTexCoord2f(VertexRenderList[i].u, VertexRenderList[i].v);

                    glColor4f(
                        ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                        ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                        ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                        config->is_using_specular
                        ? ((d3d_vertex_buffer[i].diffuse >> 24) & 0xFF) / 255.0f
                        : 1.0f);

                    glVertex3f(VertexRenderList[i].vWorldPosition.x,
                        VertexRenderList[i].vWorldPosition.y,
                        VertexRenderList[i].vWorldPosition.z);
                }

                glEnd();

            drawcalls++;

            glBlendFunc(GL_ONE, GL_ZERO);
            glDisable(GL_BLEND);
        }
    }
}

void RenderOpenGL::DrawIndoorPolygon(unsigned int uNumVertices, BLVFace *pFace,
    int uPackedID, unsigned int uColor,
    int a8) {


    if (uNumVertices < 3) {
        return;
    }

    _set_ortho_projection(1);
    _set_ortho_modelview();

    _set_3d_projection_matrix();
    _set_3d_modelview_matrix();

    unsigned int sCorrectedColor = uColor;

    TextureOpenGL *texture = (TextureOpenGL *)pFace->GetTexture();

    if (lightmap_builder->StationaryLightsCount) {
        sCorrectedColor = 0xFFFFFFFF/*-1*/;
    }

    // perception
    // engine->AlterGamma_BLV(pFace, &sCorrectedColor);

    if (engine->CanSaturateFaces() && (pFace->uAttributes & FACE_IsSecret)) {
        uint eightSeconds = OS_GetTime() % 3000;
        float angle = (eightSeconds / 3000.0f) * 2 * 3.1415f;

        int redstart = (sCorrectedColor & 0x00FF0000) >> 16;

        int col = redstart * abs(cosf(angle));
        // (a << 24) | (r << 16) | (g << 8) | b;
        sCorrectedColor = (0xFF << 24) | (redstart << 16) | (col << 8) | col;
    }

    if (pFace->uAttributes & FACE_OUTLINED) {
        if (OS_GetTime() % 300 >= 150)
            uColor = sCorrectedColor = 0xFF20FF20;
        else
            uColor = sCorrectedColor = 0xFF109010;
        // TODO(pskelton): add debug pick lines in
    }


    if (_4D864C_force_sw_render_rules && engine->config->Flag1_1()) {
        /*
        __debugbreak();
        ErrD3D(pRenderD3D->pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
        false)); ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0,
        D3DTSS_ADDRESS, D3DTADDRESS_WRAP)); for (uint i = 0; i <
        uNumVertices; ++i)
        {
        d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
        d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
        d3d_vertex_buffer[i].pos.z = 1.0 - 1.0 /
        (array_507D30[i].vWorldViewPosition.x * 0.061758894);
        d3d_vertex_buffer[i].rhw = 1.0 /
        array_507D30[i].vWorldViewPosition.x; d3d_vertex_buffer[i].diffuse =
        sCorrectedColor; d3d_vertex_buffer[i].specular = 0;
        d3d_vertex_buffer[i].texcoord.x = array_507D30[i].u /
        (double)pFace->GetTexture()->GetWidth();
        d3d_vertex_buffer[i].texcoord.y = array_507D30[i].v /
        (double)pFace->GetTexture()->GetHeight();
        }

        ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS,
        D3DTADDRESS_WRAP)); ErrD3D(pRenderD3D->pDevice->SetTexture(0,
        nullptr));
        ErrD3D(pRenderD3D->pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        d3d_vertex_buffer, uNumVertices, 28));
        lightmap_builder->DrawLightmaps(-1);
        */
    } else {
        if (!lightmap_builder->StationaryLightsCount || _4D864C_force_sw_render_rules && engine->config->Flag1_2()) {
            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].pos.x = array_507D30[i].vWorldViewProjX;
                d3d_vertex_buffer[i].pos.y = array_507D30[i].vWorldViewProjY;
                d3d_vertex_buffer[i].pos.z =
                    1.0 -
                    1.0 / (array_507D30[i].vWorldViewPosition.x * 0.061758894);
                d3d_vertex_buffer[i].rhw =
                    1.0 / array_507D30[i].vWorldViewPosition.x;
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
                d3d_vertex_buffer[i].specular = 0;
                d3d_vertex_buffer[i].texcoord.x =
                    array_507D30[i].u / (double)pFace->GetTexture()->GetWidth();
                d3d_vertex_buffer[i].texcoord.y =
                    array_507D30[i].v /
                    (double)pFace->GetTexture()->GetHeight();
            }

            // glEnable(GL_TEXTURE_2D);
            // glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());

            // glDisable(GL_CULL_FACE);  // testing
            // glDisable(GL_DEPTH_TEST);

            // if (uNumVertices != 3 ) return; //3 ,4, 5 ,6

            glBegin(GL_TRIANGLE_FAN);

            for (uint i = 0; i < pFace->uNumVertices; ++i) {
                // glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);
                glTexCoord2f(((pFace->pVertexUIDs[i] + Lights.pDeltaUV[0]) / (double)pFace->GetTexture()->GetWidth()), ((pFace->pVertexVIDs[i] + Lights.pDeltaUV[1]) / (double)pFace->GetTexture()->GetHeight()));

                glColor4f(
                     (float)((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                     (float)((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                     (float)((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f);

                glVertex3f(pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].y,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].z);
            }
            drawcalls++;

            glEnd();
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

            glDepthMask(false);

            // ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));
            glBindTexture(GL_TEXTURE_2D, 0);

            glBegin(GL_TRIANGLE_FAN);

            for (uint i = 0; i < pFace->uNumVertices; ++i) {
                // glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);
                glTexCoord2f(((pFace->pVertexUIDs[i] + Lights.pDeltaUV[0]) / (double)pFace->GetTexture()->GetWidth()), ((pFace->pVertexVIDs[i] + Lights.pDeltaUV[1]) / (double)pFace->GetTexture()->GetHeight()));


                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f);

                glVertex3f(pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].y,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].z);
            }
            drawcalls++;

            glEnd();
            glDisable(GL_CULL_FACE);

            lightmap_builder->DrawLightmaps(-1 /*, 0*/);

            for (uint i = 0; i < uNumVertices; ++i) {
                d3d_vertex_buffer[i].diffuse = sCorrectedColor;
            }

            glBindTexture(GL_TEXTURE_2D, texture->GetOpenGlTexture());
            // ErrD3D(pRenderD3D->pDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP));

            glDepthMask(true);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ZERO, GL_SRC_COLOR);

            glBegin(GL_TRIANGLE_FAN);

            for (uint i = 0; i < pFace->uNumVertices; ++i) {
                // glTexCoord2f(d3d_vertex_buffer[i].texcoord.x, d3d_vertex_buffer[i].texcoord.y);
                glTexCoord2f(((pFace->pVertexUIDs[i] + Lights.pDeltaUV[0]) / (double)pFace->GetTexture()->GetWidth()), ((pFace->pVertexVIDs[i] + Lights.pDeltaUV[1]) / (double)pFace->GetTexture()->GetHeight()));


                glColor4f(
                    ((d3d_vertex_buffer[i].diffuse >> 16) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 8) & 0xFF) / 255.0f,
                    ((d3d_vertex_buffer[i].diffuse >> 0) & 0xFF) / 255.0f,
                    1.0f);

                glVertex3f(pIndoor->pVertices[pFace->pVertexIDs[i]].x,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].y,
                    pIndoor->pVertices[pFace->pVertexIDs[i]].z);
            }
            drawcalls++;

            glEnd();

            glDisable(GL_BLEND);
        }
    }
}

bool RenderOpenGL::SwitchToWindow() {
    // pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    pViewport->ResetScreen();
    CreateZBuffer();

    return true;
}


bool RenderOpenGL::Initialize() {
    if (!RenderBase::Initialize()) {
        return false;
    }

    if (window != nullptr) {
        window->OpenGlCreate();

        glShadeModel(GL_SMOOTH);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // Black Background
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, window->GetWidth(), window->GetHeight());
        glScissor(0, 0, window->GetWidth(), window->GetHeight());

        glEnable(GL_SCISSOR_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Calculate The Aspect Ratio Of The Window
        gluPerspective(45.0f,
            (GLfloat)window->GetWidth() / (GLfloat)window->GetHeight(),
            0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Swap Buffers (Double Buffering)
        window->OpenGlSwapBuffers();

        this->clip_x = this->clip_y = 0;
        this->clip_z = window->GetWidth();
        this->clip_w = window->GetHeight();
        this->render_target_rgb =
            new uint32_t[window->GetWidth() *
            window->GetHeight()];

        PostInitialization();

        return true;
    }

    return false;
}

void RenderOpenGL::WritePixel16(int x, int y, uint16_t color) {
    // render target now 32 bit - format A8R8G8B8
    render_target_rgb[x + window->GetWidth() * y] = Color32(color);
}

void RenderOpenGL::FillRectFast(unsigned int uX, unsigned int uY,
                                unsigned int uWidth, unsigned int uHeight,
                                unsigned int uColor16) {
    // uint32_t col = Color32(uColor16);
    // for (unsigned int dy = 0; dy < uHeight; ++dy) {
    //    memset32(this->render_target_rgb + ((uY+dy) * window->GetWidth() + uX), col, uWidth);
    // }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    unsigned int b = (uColor16 & 0x1F) * 8;
    unsigned int g = ((uColor16 >> 5) & 0x3F) * 4;
    unsigned int r = ((uColor16 >> 11) & 0x1F) * 8;
    glColor3ub(r, g, b);

    float depth = 0;

    GLfloat Vertices[] = { (float)uX, (float)uY, depth,
        (float)(uX+uWidth), (float)uY, depth,
        (float)(uX + uWidth), (float)(uY+uHeight), depth,
        (float)uX, (float)(uY + uHeight), depth };

    GLubyte indices[] = { 0, 1, 2,  // first triangle (bottom left - top left - top right)
        0, 2, 3 };  // second triangle (bottom left - top right - bottom right)

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    drawcalls++;
    glDisableClientState(GL_VERTEX_ARRAY);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        log->Warning("OpenGL: fill rectangle error: (%u)", err);
    }
}

bool RenderOpenGL::NuklearInitialize(struct nk_tex_font *tfont) {
    struct nk_context* nk_ctx = nuklear->ctx;
    if (!nk_ctx) {
        log->Warning("Nuklear context is not available");
        return false;
    }

    if (!NuklearCreateDevice()) {
        log->Warning("Nuklear device creation failed");
        NuklearRelease();
        return false;
    }

    nk_font_atlas_init_default(&nk_dev.atlas);
    struct nk_tex_font *font = NuklearFontLoad(NULL, 13);
    nk_dev.atlas.default_font = font->font;
    if (!nk_dev.atlas.default_font) {
        log->Warning("Nuklear default font loading failed");
        NuklearRelease();
        return false;
    }

    memcpy(tfont, font, sizeof(struct nk_tex_font));

    if (!nk_init_default(nk_ctx, &nk_dev.atlas.default_font->handle)) {
        log->Warning("Nuklear initialization failed");
        NuklearRelease();
        return false;
    }

    nk_buffer_init_default(&nk_dev.cmds);

    return true;
}

bool RenderOpenGL::NuklearCreateDevice() {
    GLint status;
    static const GLchar* vertex_shader =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static const GLchar* fragment_shader =
        NK_SHADER_VERSION
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    nk_buffer_init_default(&nk_dev.cmds);
    nk_dev.prog = glCreateProgram();
    nk_dev.vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    nk_dev.frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(nk_dev.vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(nk_dev.frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(nk_dev.vert_shdr);
    glCompileShader(nk_dev.frag_shdr);
    glGetShaderiv(nk_dev.vert_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
        return false;
    glGetShaderiv(nk_dev.frag_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
        return false;
    glAttachShader(nk_dev.prog, nk_dev.vert_shdr);
    glAttachShader(nk_dev.prog, nk_dev.frag_shdr);
    glLinkProgram(nk_dev.prog);
    glGetProgramiv(nk_dev.prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
        return false;

    nk_dev.uniform_tex = glGetUniformLocation(nk_dev.prog, "Texture");
    nk_dev.uniform_proj = glGetUniformLocation(nk_dev.prog, "ProjMtx");
    nk_dev.attrib_pos = glGetAttribLocation(nk_dev.prog, "Position");
    nk_dev.attrib_uv = glGetAttribLocation(nk_dev.prog, "TexCoord");
    nk_dev.attrib_col = glGetAttribLocation(nk_dev.prog, "Color");

    {
        GLsizei vs = sizeof(struct nk_vertex);
        size_t vp = offsetof(struct nk_vertex, position);
        size_t vt = offsetof(struct nk_vertex, uv);
        size_t vc = offsetof(struct nk_vertex, col);

        glGenBuffers(1, &nk_dev.vbo);
        glGenBuffers(1, &nk_dev.ebo);
        glGenVertexArrays(1, &nk_dev.vao);

        glBindVertexArray(nk_dev.vao);
        glBindBuffer(GL_ARRAY_BUFFER, nk_dev.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nk_dev.ebo);

        glEnableVertexAttribArray((GLuint)nk_dev.attrib_pos);
        glEnableVertexAttribArray((GLuint)nk_dev.attrib_uv);
        glEnableVertexAttribArray((GLuint)nk_dev.attrib_col);

        glVertexAttribPointer((GLuint)nk_dev.attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)nk_dev.attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)nk_dev.attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool RenderOpenGL::NuklearRender(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer) {
    struct nk_context *nk_ctx = nuklear->ctx;
    if (!nk_ctx)
        return false;

    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale;
    GLfloat ortho[4][4] = {
        { 2.0f,  0.0f,  0.0f,  0.0f },
        { 0.0f, -2.0f,  0.0f,  0.0f },
        { 0.0f,  0.0f, -1.0f,  0.0f },
        { -1.0f, 1.0f,  0.0f,  1.0f },
    };

    height = window->GetHeight();
    width = window->GetWidth();
    display_height = render->GetRenderHeight();
    display_width = render->GetRenderWidth();

    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    scale.x = (float)display_width / (float)width;
    scale.y = (float)display_height / (float)height;

    /* setup global state */
    glViewport(0, 0, display_width, display_height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(nk_dev.prog);
    glUniform1i(nk_dev.uniform_tex, 0);
    glUniformMatrix4fv(nk_dev.uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* allocate vertex and element buffer */
        glBindVertexArray(nk_dev.vao);
        glBindBuffer(GL_ARRAY_BUFFER, nk_dev.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nk_dev.ebo);

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load vertices/elements directly into vertex/element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_vertex);
            config.null = nk_dev.null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (nk_size)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (nk_size)max_element_buffer);
            nk_convert(nk_ctx, &nk_dev.cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, nk_ctx, &nk_dev.cmds) {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor((GLint)(cmd->clip_rect.x * scale.x),
                (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                (GLint)(cmd->clip_rect.w * scale.x),
                (GLint)(cmd->clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(nk_ctx);
        nk_buffer_clear(&nk_dev.cmds);
    }

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    // glDisable(GL_SCISSOR_TEST);

    return true;
}

void RenderOpenGL::NuklearRelease() {
    nk_font_atlas_clear(&nk_dev.atlas);

    glDetachShader(nk_dev.prog, nk_dev.vert_shdr);
    glDetachShader(nk_dev.prog, nk_dev.frag_shdr);
    glDeleteShader(nk_dev.vert_shdr);
    glDeleteShader(nk_dev.frag_shdr);
    glDeleteProgram(nk_dev.prog);
    glDeleteBuffers(1, &nk_dev.vbo);
    glDeleteBuffers(1, &nk_dev.ebo);
    glDeleteVertexArrays(1, &nk_dev.vao);

    nk_buffer_free(&nk_dev.cmds);

    memset(&nk_dev, 0, sizeof(nk_dev));
}

struct nk_tex_font *RenderOpenGL::NuklearFontLoad(const char* font_path, size_t font_size) {
    const void *image;
    int w, h;
    GLuint texid;

    struct nk_tex_font *tfont = new (struct nk_tex_font);
    if (!tfont)
        return NULL;

    struct nk_font_config cfg = nk_font_config(font_size);
    cfg.merge_mode = nk_false;
    cfg.coord_type = NK_COORD_UV;
    cfg.spacing = nk_vec2(0, 0);
    cfg.oversample_h = 3;
    cfg.oversample_v = 1;
    cfg.range = nk_font_cyrillic_glyph_ranges();
    cfg.size = font_size;
    cfg.pixel_snap = 0;
    cfg.fallback_glyph = '?';

    nk_font_atlas_begin(&nk_dev.atlas);

    if (!font_path)
        tfont->font = nk_font_atlas_add_default(&nk_dev.atlas, font_size, 0);
    else
        tfont->font = nk_font_atlas_add_from_file(&nk_dev.atlas, font_path, font_size, &cfg);

    image = nk_font_atlas_bake(&nk_dev.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    tfont->texid = texid;
    nk_font_atlas_end(&nk_dev.atlas, nk_handle_id(texid), &nk_dev.null);

    return tfont;
}

void RenderOpenGL::NuklearFontFree(struct nk_tex_font *tfont) {
    if (tfont)
        glDeleteTextures(1, &tfont->texid);
}

struct nk_image RenderOpenGL::NuklearImageLoad(Image *img) {
    GLuint texid;
    auto t = (TextureOpenGL *)img;
    unsigned __int8 *pixels = (unsigned __int8 *)t->GetPixels(IMAGE_FORMAT_R8G8B8A8);

    glGenTextures(1, &texid);
    t->SetOpenGlTexture(texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->GetWidth(), t->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return nk_image_id(texid);
}

void RenderOpenGL::NuklearImageFree(Image *img) {
    auto t = (TextureOpenGL *)img;
    GLuint texid = t->GetOpenGlTexture();
    if (texid != -1) {
        glDeleteTextures(1, &texid);
    }
}
