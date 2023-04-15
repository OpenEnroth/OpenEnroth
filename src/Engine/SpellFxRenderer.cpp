#include "Engine/SpellFxRenderer.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"
#include "Engine/Time.h"
#include "Engine/stru160.h"

#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/Tables/IconFrameTable.h"

#include "Library/Random/Random.h"

//----- (004A7063) --------------------------------------------------------
unsigned int ModulateColor(unsigned int diffuse, float multiplier) {
    float alpha = multiplier * ((diffuse >> 24) & 0xFF);
    int a = (int)floorf(alpha + 0.5f);
    a = std::max(0, std::min(255, a));

    float red = multiplier * ((diffuse >> 0) & 0xFF);
    int r = (int)floorf(red + 0.5f);
    r = std::max(0, std::min(255, r));

    float green = multiplier * ((diffuse >> 8) & 0xFF);
    int g = (int)floorf(green + 0.5f);
    g = std::max(0, std::min(255, g));

    float blue = multiplier * ((diffuse >> 16) & 0xFF);
    int b = (int)floorf(blue + 0.5f);
    b = std::max(0, std::min(255, b));

    return (a << 24) | (b << 16) | (g << 8) | r;
}

//----- (0042620A) --------------------------------------------------------
bool sr_42620A(RenderVertexSoft *p) { // maybe near clipping on projectiles
    //  int16_t v1; // fps@1
    uint8_t v2;   // c0@2
    char v3;              // c2@2
    uint8_t v4;   // c3@2
                          //  bool result; // eax@2
    uint8_t v6;   // c0@4
    char v7;              // c2@4
    uint8_t v8;   // c3@4
    uint8_t v9;   // c0@6
    char v10;             // c2@6
    uint8_t v11;  // c3@6
    float v13;            // ST04_4@7
    float v14;            // ST00_4@7
    float v17;            // ST04_4@8
    float v18;            // ST00_4@8

    if (p->vWorldViewPosition.x < pCamera3D->ViewPlaneDist_X ||
        (v2 = pCamera3D->ViewPlaneDist_X < p[1].vWorldViewPosition.x, v3 = 0,
         v4 = pCamera3D->ViewPlaneDist_X == p[1].vWorldViewPosition.x,

         !(v2 | v4))) {
        if (p->vWorldViewPosition.x < pCamera3D->ViewPlaneDist_X) {
            v6 = pCamera3D->ViewPlaneDist_X < p[1].vWorldViewPosition.x;
            v7 = 0;
            v8 = pCamera3D->ViewPlaneDist_X == p[1].vWorldViewPosition.x;

            if (!(v6 | v8)) {
                //logger->Info("sr_42620A reject");
                return false;
            }
        }
        //logger->Info("sr_42620A pass");
        v9 = pCamera3D->ViewPlaneDist_X < p->vWorldViewPosition.x;
        v10 = 0;
        v11 = pCamera3D->ViewPlaneDist_X == p->vWorldViewPosition.x;

        if (v9 | v11) {
            float v16 =
                1.0f / (p->vWorldViewPosition.x - p[1].vWorldViewPosition.x);
            v17 = (p->vWorldViewPosition.y - p[1].vWorldViewPosition.y) * v16;
            v18 = (p->vWorldViewPosition.z - p[1].vWorldViewPosition.z) * v16;
            float v19 = pCamera3D->ViewPlaneDist_X - p[1].vWorldViewPosition.x;
            p[1].vWorldViewPosition.x = v19 + p[1].vWorldViewPosition.x;
            p[1].vWorldViewPosition.y = v17 * v19 + p[1].vWorldViewPosition.y;
            p[1].vWorldViewPosition.z = v19 * v18 + p[1].vWorldViewPosition.z;
        } else {
            float v12 =
                1.0f / (p[1].vWorldViewPosition.x - p->vWorldViewPosition.x);
            v13 = (p[1].vWorldViewPosition.y - p->vWorldViewPosition.y) * v12;
            v14 = (p[1].vWorldViewPosition.z - p->vWorldViewPosition.z) * v12;
            float v15 = pCamera3D->ViewPlaneDist_X - p->vWorldViewPosition.x;
            p->vWorldViewPosition.x = v15 + p->vWorldViewPosition.x;
            p->vWorldViewPosition.y = v13 * v15 + p->vWorldViewPosition.y;
            p->vWorldViewPosition.z = v15 * v14 + p->vWorldViewPosition.z;
        }
    }

    return true;
}

//----- (004775C4) --------------------------------------------------------
SpellFX_Billboard::~SpellFX_Billboard() {
    delete[] pArray1;
    pArray1 = nullptr;

    delete[] pArray2;
    pArray2 = nullptr;
}

//----- (00478211) --------------------------------------------------------
void SpellFX_Billboard::Initialize(int a2) {
    uNumVec4sInArray1 = 66;
    pArray1 = new stru16x[66];

    uNumVec3sInArray2 = 128;
    pArray2 = new stru160[128];

    for (uint i = 0; i < uNumVec4sInArray1; ++i) {
        pArray1[i].field_0 = array_4EB8B8[i].field_0;
        pArray1[i].field_4 = array_4EB8B8[i].field_4;
        pArray1[i].field_8 = array_4EB8B8[i].field_8;
        pArray1[i].field_C = a2;
    }

    for (uint i = 0; i < uNumVec3sInArray2; ++i) {
        pArray2[i].field_0 = array_4EBBD0_x[3 * i];
        pArray2[i].field_4 = array_4EBBD0_x[(3 * i)+1];
        pArray2[i].field_8 = array_4EBBD0_x[(3 * i)+2];
    }

    // doesnt copy over properly
    // memcpy(pArray2, array_4EBBD0_x.data() /*array_4EBBD0*/,
   //        uNumVec3sInArray2 * sizeof(stru160));
}

//----- (0047829F) --------------------------------------------------------
void SpellFX_Billboard::_47829F_sphere_particle(
    float x_offset, float y_offset, float z_offset, float scale, int diffuse) {
    int v7 = 0;

    // offsets are centrepoints

    // 66 total verts points - held in array1
    // 128 triangles using 66 diff verts


    for (unsigned int i = 0; i < uNumVec3sInArray2; ++i) {  // indicies for triangle in sphere
        // for (unsigned int j = 0; j < 3; ++j) {
        //    field_14[j].x =
        //        x_offset +
        //        scale * *(&pArray1->field_0 +
        //                  4 * *(int *)((char *)&pArray2->field_0 + v7));
        //    field_14[j].y =
        //        y_offset +
        //        scale * *(&pArray1->field_4 +
        //                  4 * *(int *)((char *)&pArray2->field_0 + v7));
        //    field_14[j].z =
        //        z_offset +
        //        scale * *(&pArray1->field_8 +
        //                  4 * *(int *)((char *)&pArray2->field_0 + v7));
        //    // int v10 = *(int *)((char *)&pArray2->field_0 + v7);

        //    field_14[j].diffuse =
        //        *((int *)&pArray1[1].field_0 +
        //          4 * (*(int *)((char *)&pArray2->field_0 + v7)));
        //    v7 += 4;
        //}


        field_14[0].x = x_offset + scale * pArray1[int(pArray2[i].field_0)].field_0;
        field_14[0].y = y_offset + scale * pArray1[int(pArray2[i].field_0)].field_4;
        field_14[0].z = z_offset + scale * pArray1[int(pArray2[i].field_0)].field_8;
        field_14[0].diffuse = diffuse;

        field_14[1].x = x_offset + scale * pArray1[int(pArray2[i].field_4)].field_0;
        field_14[1].y = y_offset + scale * pArray1[int(pArray2[i].field_4)].field_4;
        field_14[1].z = z_offset + scale * pArray1[int(pArray2[i].field_4)].field_8;
        field_14[1].diffuse = diffuse;

        field_14[2].x = x_offset + scale * pArray1[int(pArray2[i].field_8)].field_0;
        field_14[2].y = y_offset + scale * pArray1[int(pArray2[i].field_8)].field_4;
        field_14[2].z = z_offset + scale * pArray1[int(pArray2[i].field_8)].field_8;
        field_14[2].diffuse = diffuse;



        uNumVertices = 3;
        if (SpellFXViewTransform() && SpellFXViewClip()) {
            if (SpellFXProject()) render->BillboardSphereSpellFX(this, diffuse);
        }
    }
}

//----- (004A71FE) --------------------------------------------------------
void SpellFxRenderer::DoAddProjectile(float srcX, float srcY, float srcZ,
                                      float dstX, float dstY, float dstZ,
                                      Texture *texture) {
    // int v8; // eax@1

    // v8 = uNumProjectiles;
    if (uNumProjectiles < 32) {
        pProjectiles[uNumProjectiles].srcX = srcX;
        pProjectiles[uNumProjectiles].srcY = srcY;
        pProjectiles[uNumProjectiles].srcZ = srcZ;
        pProjectiles[uNumProjectiles].dstX = dstX;
        pProjectiles[uNumProjectiles].dstY = dstY;
        pProjectiles[uNumProjectiles].dstZ = dstZ;
        pProjectiles[uNumProjectiles].texture = texture;
        uNumProjectiles++;
    }
}

//----- (004A7298) --------------------------------------------------------
void SpellFxRenderer::DrawProjectiles() {
    float v10;              // ST1C_4@8
    float v11;              // ST0C_4@8
    RenderVertexSoft v[2];  // [sp+30h] [bp-68h]@1

    for (int i = 0; i < uNumProjectiles; ++i) {
        ProjectileAnim *p = &pProjectiles[i];

        v[0].vWorldPosition.x = p->srcX;
        v[0].vWorldPosition.y = p->srcY;
        v[0].vWorldPosition.z = p->srcZ;
        v[1].vWorldPosition.x = p->dstX;
        v[1].vWorldPosition.y = p->dstY;
        v[1].vWorldPosition.z = p->dstZ;
        pCamera3D->ViewTransform(v, 2);

        sr_42620A(v);

        pCamera3D->Project(v, 2, 0);

        // 20.0f is width scaling factor
        v10 = pCamera3D->ViewPlaneDist_X / v[1].vWorldViewPosition.x * 20.0f;
        v11 = pCamera3D->ViewPlaneDist_X / v[0].vWorldViewPosition.x * 20.0f;
        render->DrawProjectile(v[0].vWorldViewProjX, v[0].vWorldViewProjY,
                               v[0].vWorldViewPosition.x, v11,
                               v[1].vWorldViewProjX, v[1].vWorldViewProjY,
                               v[1].vWorldViewPosition.x, v10, p->texture);
    }
}

//----- (004A73AA) --------------------------------------------------------
void SpellFxRenderer::_4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
        SpriteObject *a2, unsigned int uDiffuse, Texture *texture) {
    // check if enough time has passed to add particle into the trail
    if (a2->_lastParticleTime + a2->_ticksPerParticle < pEventTimer->uTotalGameTimeElapsed) {
        a2->_lastParticleTime += a2->_ticksPerParticle;
    } else {
        return;
    }

    SpellFxRenderer *thisspellfxrend;  // edi@1
    SpriteObject *v5;     // esi@1
    int v6;               // eax@1
    stru6_stru2 *v7;      // eax@2
    double y;            // st7@2
    double z;            // st6@2
    double v10;           // st7@3
    Particle_sw local_0;  // [sp+8h] [bp-68h]@1
    float x;              // [sp+78h] [bp+8h]@2

    thisspellfxrend = this;
    memset(&local_0, 0, sizeof(Particle_sw));
    v5 = a2;
    v6 = a2->field_54;
    if (v6) {
        v7 = &thisspellfxrend->array_4[v6 & 0x1F];  // v7 particle origin?
        x = ((float)a2->vPosition.x - v7->flt_0_x) * 0.5f + v7->flt_0_x;
        y = ((float)v5->vPosition.y - v7->flt_4_y) * 0.5f + v7->flt_4_y;
        z = ((float)v5->vPosition.z - v7->flt_8_z) * 0.5f + v7->flt_8_z;
        local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
        local_0.uDiffuse = uDiffuse;
        local_0.x = x + 4.0;
        local_0.y = y;
        local_0.z = z;
        local_0.r = 0.0;
        local_0.g = 0.0;
        local_0.b = 0.0;
        local_0.timeToLive = vrng->random(0x40) + 96; // was rand() & 0x40
        local_0.texture = texture;
        local_0.particle_size = 1.0f;
        particle_engine->AddParticle(&local_0);
        local_0.x = x - 4.0f;
        particle_engine->AddParticle(&local_0);
        local_0.x = (float)v5->vPosition.x + 4.0f;
        local_0.y = (float)v5->vPosition.y;
        local_0.z = (float)v5->vPosition.z;
        particle_engine->AddParticle(&local_0);
        local_0.x = (float)v5->vPosition.x - 4.0f;
        particle_engine->AddParticle(&local_0);
        thisspellfxrend->array_4[v5->field_54 & 0x1F].flt_0_x = (float)v5->vPosition.x;
        thisspellfxrend->array_4[v5->field_54 & 0x1F].flt_4_y = (float)v5->vPosition.y;
        thisspellfxrend->array_4[v5->field_54 & 0x1F].flt_8_z = (float)v5->vPosition.z;
    } else {
        a2->field_54 = thisspellfxrend->field_0++;
        thisspellfxrend->array_4[a2->field_54 & 0x1F].flt_0_x = (float)a2->vPosition.x;
        thisspellfxrend->array_4[a2->field_54 & 0x1F].flt_4_y = (float)a2->vPosition.y;
        thisspellfxrend->array_4[a2->field_54 & 0x1F].flt_8_z = (float)a2->vPosition.z;
        v10 = (float)a2->vPosition.x;
        local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
        local_0.uDiffuse = uDiffuse;
        local_0.x = v10 + 4.0f;
        local_0.y = (float)a2->vPosition.y;
        local_0.z = (float)a2->vPosition.z;
        local_0.r = 0.0f;
        local_0.g = 0.0f;
        local_0.b = 0.0f;
        local_0.particle_size = 1.0f;
        local_0.timeToLive = vrng->random(0x80) + 128;
        local_0.texture = texture;
        particle_engine->AddParticle(&local_0);
        local_0.x = (float)a2->vPosition.x - 4.0f;
        particle_engine->AddParticle(&local_0);
    }
}

//----- (004A75CC) --------------------------------------------------------
void SpellFxRenderer::_4A75CC_single_spell_collision_particle(
    SpriteObject *a1, unsigned int uDiffuse, Texture *texture) {
    double v4;            // st7@1
    signed int v5;        // edi@1
    Particle_sw local_0;  // [sp+8h] [bp-68h]@1

    memset(&local_0, 0, sizeof(Particle_sw));
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Dropping;
    local_0.x = (float)a1->vPosition.x;
    local_0.y = (float)a1->vPosition.y;
    v4 = (float)a1->vPosition.z;
    local_0.uDiffuse = uDiffuse;
    local_0.z = v4;
    v5 = 10;
    local_0.timeToLive = vrng->random(0x80) + 128;
    local_0.texture = texture;
    local_0.particle_size = 1.0f;
    do {
        local_0.r = (float) vrng->random(0x200) - 255.0f;
        local_0.g = (float) vrng->random(0x200) - 255.0f;
        local_0.b = (float) vrng->random(0x200) - 255.0f;
        particle_engine->AddParticle(&local_0);
        --v5;
    } while (v5);
}

void SpellFxRenderer::_4A7688_fireball_collision_particle(SpriteObject *a2) {
    double v3 = (double)a2->uSpriteFrameID / (double)a2->GetLifetime();
    double v4;
    if (v3 >= 0.75)
        v4 = (1.0 - v3) * 4.0;
    else
        v4 = v3 * 1.333333333333333;

    Particle_sw local_0 = { 0 };
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Dropping;
    local_0.uDiffuse = colorTable.OrangeyRed.c32();
    local_0.x = (float)a2->vPosition.x;
    local_0.y = (float)a2->vPosition.y;
    local_0.z = (float)a2->vPosition.z;
    local_0.timeToLive = vrng->random(0x80) + 128;
    local_0.texture = this->effpar01;
    local_0.particle_size = 1.0;

    // 10 fireball sparks
    for (unsigned int i = 0; i < 10; ++i) {
        local_0.r = vrng->random(0x200) - 255;
        local_0.g = vrng->random(0x200) - 255;
        local_0.b = vrng->random(0x200) - 255;
        particle_engine->AddParticle(&local_0);
    }

    pStru1->_47829F_sphere_particle((float)a2->vPosition.x,
                                    (float)a2->vPosition.y,
                                    (float)a2->vPosition.z,
                                    floorf(0.5f + (512.0 * v3)),
                                    ModulateColor(colorTable.OrangeyRed.c32(), v4));
}

void SpellFxRenderer::_4A77FD_implosion_particle_d3d(SpriteObject *a1) {
    double v4 = (double)a1->uSpriteFrameID / (double)a1->GetLifetime();
    double v5;
    if (v4 >= 0.75) {
        v5 = v4 * 4.0;
    } else {
        v5 = v4 * 1.333333333333333;
    }

    pStru1->_47829F_sphere_particle(a1->vPosition.x,
                                    a1->vPosition.y,
                                    a1->vPosition.z,
                                    floorf(0.5f + (512.f - v4 * 512.f)),
                                    ModulateColor(colorTable.MediumGrey.c32(), v5));
}

//----- (004A7948) --------------------------------------------------------
void SpellFxRenderer::_4A7948_mind_blast_after_effect(SpriteObject *a1) {
    Particle_sw Dst = { 0 };
    Dst.type = ParticleType_Sprite | ParticleType_Rotating | ParticleType_Dropping;
    Dst.uDiffuse = colorTable.MediumGrey.c32();
    Dst.x = (float)a1->vPosition.x;
    Dst.y = (float)a1->vPosition.y;
    Dst.z = (float)a1->vPosition.z;
    Dst.texture = a1->GetSpriteFrame()->hw_sprites[0]->texture;
    Dst.particle_size = 1.0;
    Dst.timeToLive = vrng->random(0x80) + 128;
    for (int i = 0; i < 10; i++) {
        Dst.r = (float) vrng->random(0x200) - 255.0f;
        Dst.g = (float) vrng->random(0x200) - 255.0f;
        Dst.b = (float) vrng->random(0x200) - 255.0f;
        particle_engine->AddParticle(&Dst);
    }
}

//----- (004A7A27) --------------------------------------------------------
bool SpellFxRenderer::AddMobileLight(SpriteObject *a1, unsigned int uDiffuse,
                                     int uRadius) {
    return pMobileLightsStack->AddLight(a1->vPosition.toFloat(), a1->uSectorID,
        uRadius, uDiffuse & 0x000000FF, (uDiffuse & 0x0000FF00) >> 8,
        (uDiffuse & 0x00FF0000) >> 16, _4E94D3_light_type);
}

//----- (004A7A66) --------------------------------------------------------
void SpellFxRenderer::
    _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
        SpriteObject *a1, unsigned int uDiffuse, Texture *texture, float a4) {
    int v5;               // eax@1
    double v7;            // st6@1
    double v8;            // st6@1
    double v9;            // st7@1
    double v10;           // st6@1
    Particle_sw local_0;  // [sp+0h] [bp-6Ch]@1
    float v12;            // [sp+68h] [bp-4h]@1
    float a1a;            // [sp+74h] [bp+8h]@1
    float uDiffusea;      // [sp+78h] [bp+Ch]@1
    float uTextureIDa;    // [sp+7Ch] [bp+10h]@1

    memset(&local_0, 0, sizeof(Particle_sw));
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Dropping;
    local_0.x = (float)a1->vPosition.x;
    v5 = a1->vPosition.z;
    local_0.y = (float)a1->vPosition.y;
    local_0.uDiffuse = uDiffuse;
    local_0.z = (float)(v5 + 32);
    local_0.particle_size = 1.0;
    v7 = 0.0 * a4;
    local_0.timeToLive = vrng->random(0x80) + 128;
    local_0.texture = texture;
    a1a = v7;
    local_0.r = v7;
    local_0.g = a4;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
    v8 = 0.70710677 * a4;
    uDiffusea = v8;
    local_0.r = v8;
    local_0.g = v8;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
    local_0.g = a1a;
    local_0.r = a4;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
    local_0.r = uDiffusea;
    local_0.b = a4;
    v9 = -uDiffusea;
    uTextureIDa = v9;
    local_0.g = v9;
    particle_engine->AddParticle(&local_0);
    v10 = -1.0 * a4;
    local_0.r = a1a;
    v12 = v10;
    local_0.g = v10;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
    local_0.b = a4;
    local_0.r = uTextureIDa;
    local_0.g = uTextureIDa;
    particle_engine->AddParticle(&local_0);
    local_0.r = v12;
    local_0.g = a1a;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
    local_0.r = uTextureIDa;
    local_0.g = uDiffusea;
    local_0.b = a4;
    particle_engine->AddParticle(&local_0);
}

void SpellFxRenderer::_4A7C07_stun_spell_fx(SpriteObject *a2) {
    Particle_sw local_0 = { 0 };

    if (a2->field_54 != 0) {
        stru6_stru2 *v6 = &array_4[a2->field_54 & 0x1F];
        local_0.type = ParticleType_Sprite;
        local_0.uDiffuse = colorTable.White.c32();
        local_0.x = ((float)a2->vPosition.x - v6->flt_0_x) * 0.5f + v6->flt_0_x;
        local_0.y = ((float)a2->vPosition.y - v6->flt_4_y) * 0.5f + v6->flt_4_y;
        local_0.z = ((float)a2->vPosition.z - v6->flt_8_z) * 0.5f + v6->flt_8_z;
        local_0.r = 0.0f;
        local_0.g = 0.0f;
        local_0.b = 0.0f;

        local_0.particle_size = 3.0;
        local_0.timeToLive = vrng->random(0x40) + 64;
        local_0.texture = a2->GetSpriteFrame()->hw_sprites[0]->texture;
        local_0.paletteID = a2->GetSpriteFrame()->uPaletteID;
        particle_engine->AddParticle(&local_0);
        local_0.particle_size = 2.0;  // was 4.0 - reduce size of stun ring;
        local_0.x = (float)a2->vPosition.x;
        local_0.y = (float)a2->vPosition.y;
        local_0.z = (float)a2->vPosition.z;
        local_0.timeToLive = vrng->random(0x40) + 64;
        particle_engine->AddParticle(&local_0);
        v6->flt_0_x = (float)a2->vPosition.x;
        v6->flt_4_y = (float)a2->vPosition.y;
        v6->flt_8_z = (float)a2->vPosition.z;
    } else {
        a2->field_54 = field_0++;
        array_4[a2->field_54 & 0x1F].flt_0_x = (float)a2->vPosition.x;
        array_4[a2->field_54 & 0x1F].flt_4_y = (float)a2->vPosition.y;
        array_4[a2->field_54 & 0x1F].flt_8_z = (float)a2->vPosition.z;
        local_0.type = ParticleType_Sprite;
        local_0.uDiffuse = colorTable.White.c32();
        local_0.particle_size = 1.0;  // was 2.0 - reduce size of stun ring;
        local_0.x = (float)a2->vPosition.x;
        local_0.y = (float)a2->vPosition.y;
        local_0.z = (float)a2->vPosition.z;
        local_0.r = 0.0f;
        local_0.g = 0.0f;
        local_0.b = 0.0f;
        local_0.timeToLive = vrng->random(0x40) + 64;
        local_0.texture = a2->GetSpriteFrame()->hw_sprites[0]->texture;
        local_0.paletteID = a2->GetSpriteFrame()->uPaletteID;
        particle_engine->AddParticle(&local_0);
    }
}

//----- (004A7E05) --------------------------------------------------------
void SpellFxRenderer::AddProjectile(SpriteObject *a2, int a3,
                                    Texture *texture) {
    if (a2->field_54) {
        DoAddProjectile(array_4[a2->field_54 & 0x1F].flt_0_x,
                        array_4[a2->field_54 & 0x1F].flt_4_y,
                        array_4[a2->field_54 & 0x1F].flt_8_z, a2->vPosition.x,
                        a2->vPosition.y, a2->vPosition.z, texture);
    } else {
        a2->field_54 = field_0++;
        array_4[a2->field_54 & 0x1F].flt_0_x = (float)a2->vPosition.x;
        array_4[a2->field_54 & 0x1F].flt_4_y = (float)a2->vPosition.y;
        array_4[a2->field_54 & 0x1F].flt_8_z = (float)a2->vPosition.z;
    }
}

void SpellFxRenderer::sparklesOnActorAfterItCastsBuff(Actor *pActor, uint32_t uDiffuse) {
    Particle_sw particle;

    memset(&particle, 0, sizeof(Particle_sw));
    particle.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
    particle.timeToLive = vrng->random(128) + 128;
    particle.texture = this->effpar02;
    particle.particle_size = 1.0;

    for (int i = 0; i < 50; i++) {
        particle.x = vrng->random(256) + pActor->vPosition.x - 127;
        particle.y = vrng->random(256) + pActor->vPosition.y - 127;
        particle.z = vrng->random(256) + pActor->vPosition.z;
        if (uDiffuse) {
            particle.uDiffuse = uDiffuse;
        } else {
            // TODO(Nik-RE-dev): check colour format
            particle.uDiffuse = vrng->random(0x10000) | (vrng->random(0x10000) << 16);
        }
        particle_engine->AddParticle(&particle);
    }
}

//----- (004A7F74) --------------------------------------------------------
void SpellFxRenderer::_4A7F74(int x, int y, int z) {
    signed int v6;        // edi@1
    Particle_sw local_0;  // [sp+8h] [bp-78h]@1
    double v11;           // [sp+70h] [bp-10h]@1
    double v12;           // [sp+78h] [bp-8h]@1
    float z1;             // [sp+88h] [bp+8h]@2

    memset(&local_0, 0, sizeof(local_0));
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Dropping;
    local_0.uDiffuse = colorTable.MediumGrey.c32();
    local_0.particle_size = 1.0;
    v6 = 8;
    local_0.timeToLive = vrng->random(0x80) + 128;

    v12 = (float)x;
    local_0.texture = this->effpar01;
    v11 = (float)y;
    do {
        local_0.x = vrng->randomFloat() * 40.0f - 20.0f + v12;
        z1 = (float)z;
        local_0.z = z1;
        local_0.y = vrng->randomFloat() * 40.0f - 20.0f + v11;
        local_0.r = vrng->randomFloat() * 400.0f - 200.0f;
        local_0.g = vrng->randomFloat() * 400.0f - 200.0f;
        local_0.b = vrng->randomFloat() * 150.0f + 50.0f;
        particle_engine->AddParticle(&local_0);
        --v6;
    } while (v6);
}

//----- (004A806F) --------------------------------------------------------
float SpellFxRenderer::_4A806F_get_mass_distortion_value(Actor *pActor) {
    int v2;     // ecx@1
    int v3;     // eax@1
    double v4;  // st7@2

    v3 = *(int *)&pActor->pActorBuffs[ACTOR_BUFF_MASS_DISTORTION]
              .expire_time.value -
         pMiscTimer->uTotalGameTimeElapsed;
    if (v3 > 64) {
        v2 = (v3 - 64) * (v3 - 64);
        v4 = (double)v2 / 5120.0 + 0.2;
    } else if (v3 > 0) {
        v2 = v3 * v3;
        v4 = 1.0 - (double)(signed int)(v3 * v3) / 5120.0;
    } else {
        pActor->pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Reset();
        v4 = 1.0;
    }

    return v4;
}

//----- (004A81CA) --------------------------------------------------------
bool SpellFxRenderer::RenderAsSprite(SpriteObject *a2) {
    // SpellFxRenderer *v2; // ebx@1
    int result;  // eax@1
                 //  int v4; // eax@27
    // unsigned int diffuse; // esi@41
    // int v6; // ecx@49
    //  int v7; // eax@54
    //  int v8; // eax@55
    //  char v9; // zf@56
    //  int v10; // eax@59
    //  int v11; // eax@61
    //  int v12; // eax@85
    //  int v13; // eax@86
    //  int v14; // eax@96
    //  int v15; // eax@111
    //  int v16; // eax@118
    //  int v17; // eax@139
    //  int v18; // eax@140
    //  int v19; // eax@141
    //  int v20; // eax@151

    // __debugbreak(); // need to refactor carefully & collect data
    // v2 = this;
    result = a2->uType;

    switch (a2->uType) {
        case SPRITE_PROJECTILE_AIRBOLT:
        case SPRITE_PROJECTILE_530:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.Azure.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_AIRBOLT_IMPACT:
        case SPRITE_PROJECTILE_530_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.Azure.c32(), effpar01);
            return true;

        case SPRITE_PROJECTILE_EARTHBOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.CarnabyTan.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_EARTHBOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.CarnabyTan.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_FIREBOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.OrangeyRed.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_FIREBOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_WATERBOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.ScienceBlue.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_WATERBOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.ScienceBlue.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_520:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.GreenTeal.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_520_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.GreenTeal.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_525:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.DirtyYellow.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_525_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.DirtyYellow.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_LIGHTBOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.White.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_LIGHTBOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.White.c32(), effpar01);
            return false;

        case SPRITE_PROJECTILE_DARKBOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.MediumGrey.c32(), effpar01);
            return false;
        case SPRITE_PROJECTILE_DARKBOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.MediumGrey.c32(), effpar01);
            return false;

        case SPRITE_ARROW_PROJECTILE:
        case SPRITE_PROJECTILE_EXPLOSIVE:
        case SPRITE_BLASTER_PROJECTILE:
            return true;

        case SPRITE_BLASTER_IMPACT:
            AddMobileLight(a2, colorTable.Red.c32(), 256);
            return false;

        case SPRITE_OBJECT_EXPLODE_IMPACT:
            // __debugbreak();  // what kind of effect is this?
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return true;

        case SPRITE_546:
        case SPRITE_547:
        case SPRITE_548:
        case SPRITE_549:
        case SPRITE_551:
        case SPRITE_552:
        case SPRITE_553:
        case SPRITE_554:
        case SPRITE_557:
        case SPRITE_558:
        case SPRITE_559:
        case SPRITE_560:
        case SPRITE_561:
        case SPRITE_562:
        case SPRITE_563:
        case SPRITE_564:
        case SPRITE_565:
        case SPRITE_566:
        case SPRITE_567:
        case SPRITE_568:
        case SPRITE_569:
        case SPRITE_570:
        case SPRITE_571:
        case SPRITE_572:
        case SPRITE_573:
        case SPRITE_574:
        case SPRITE_575:
        case SPRITE_576:
        case SPRITE_577:
        case SPRITE_578:
        case SPRITE_579:
        case SPRITE_580:
        case SPRITE_581:
        case SPRITE_582:
        case SPRITE_583:
        case SPRITE_584:
        case SPRITE_585:
        case SPRITE_586:
        case SPRITE_587:
        case SPRITE_588:
        case SPRITE_589:
        case SPRITE_590:
        case SPRITE_591:
        case SPRITE_592:
        case SPRITE_593:
        case SPRITE_594:
        case SPRITE_595:
        case SPRITE_596:
        case SPRITE_597:
        case SPRITE_598:
        case SPRITE_599:
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            return false;

        case SPRITE_TRAP_FIRE:
        case SPRITE_TRAP_LIGHTNING:
        case SPRITE_TRAP_COLD:
        case SPRITE_TRAP_BODY:
            return true;

        case SPRITE_SPELL_FIRE_FIRE_BOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.OrangeyRed.c32(), effpar01);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_FIRE_FIRE_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_FIRE_FIREBALL:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
                a2, colorTable.OrangeyRed.c32(), effpar01);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_FIRE_FIREBALL_IMPACT:
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            // if (render->pRenderD3D)
            {
                if (/*PID_TYPE(a2->spell_caster_pid) != OBJECT_Actor &&*/
                    PID_TYPE(a2->spell_caster_pid) != OBJECT_Item) {
                    if (field_204 != 4) {
                        field_204++;
                        _4A7688_fireball_collision_particle(a2);
                    }
                    return true;  // sphere and sprite
                }
            }
            return true;

        case SPRITE_SPELL_FIRE_FIRE_SPIKE:
            return true;
        case SPRITE_SPELL_FIRE_FIRE_SPIKE_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
                a2, colorTable.OrangeyRed.c32(), effpar01, 250.0);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_FIRE_IMMOLATION:
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            return false;

        case SPRITE_SPELL_FIRE_METEOR_SHOWER:
            return true;
        case SPRITE_SPELL_FIRE_METEOR_SHOWER_1:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
                a2, colorTable.OrangeyRed.c32(), effpar01, 300.0);
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
                a2, colorTable.OrangeyRed.c32(), effpar01, 250.0);
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
                a2, colorTable.OrangeyRed.c32(), effpar01, 200.0);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_FIRE_INFERNO:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
                a2, colorTable.OrangeyRed.c32(), effpar01, 250.0);
            return false;

        case SPRITE_SPELL_FIRE_INCINERATE:
            return true;
        case SPRITE_SPELL_FIRE_INCINERATE_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            _4A75CC_single_spell_collision_particle(a2, colorTable.OrangeyRed.c32(), effpar01);
            AddMobileLight(a2, colorTable.OrangeyRed.c32(), 256);
            return false;

        case SPRITE_SPELL_AIR_SPARKS:
            //_4A78AE_sparks_spell(a2);
            //AddMobileLight(a2, 0x2F3351, 128);  // 0x64640F
            return true;
            //return false;

        case SPRITE_SPELL_AIR_SPARKS_POP:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, colorTable.MustardYellow.c32(), effpar02, 200.0);
            return false;

        case SPRITE_SPELL_AIR_LIGHTNING_BOLT:
            // if ( !render->pRenderD3D )
            //  return true;
            AddProjectile(a2, 100, assets->GetBitmap(fmt::format("sp18h{}", vrng->randomInSegment(1, 6))));
            return false;
        case SPRITE_SPELL_AIR_LIGHTNING_BOLT_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, colorTable.MustardYellow.c32(), effpar02, 200.0);
            AddMobileLight(a2, colorTable.MustardYellow.c32(), 256);
            return false;

        case SPRITE_SPELL_AIR_IMPLOSION:
        case SPRITE_SPELL_AIR_IMPLOSION_IMPACT:
            _4A77FD_implosion_particle_d3d(a2);
            return false;

        case SPRITE_SPELL_AIR_STARBURST:
            return true;
        case SPRITE_SPELL_AIR_STARBURST_1:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, colorTable.MustardYellow.c32(), effpar01, 200.0);
            AddMobileLight(a2, colorTable.MustardYellow.c32(), 256);
            return false;

        case SPRITE_SPELL_WATER_POISON_SPRAY:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.GreenTeal.c32(), effpar01);
            AddMobileLight(a2, colorTable.GreenTeal.c32(), 256);
            return false;
        case SPRITE_SPELL_WATER_POISON_SPRAY_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.GreenTeal.c32(), effpar01);
            AddMobileLight(a2, colorTable.GreenTeal.c32(), 256);
            return false;

        case SPRITE_SPELL_WATER_ICE_BOLT:
            return true;
        case SPRITE_SPELL_WATER_ICE_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.CarolinaBlue.c32(), effpar01);
            AddMobileLight(a2, colorTable.CarolinaBlue.c32(), 256);
            return false;

        case SPRITE_SPELL_WATER_ACID_BURST:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.GreenTeal.c32(), effpar01);
            AddMobileLight(a2, colorTable.GreenTeal.c32(), 256);
            return false;
        case SPRITE_SPELL_WATER_ACID_BURST_IMPACT:
            return true;

        case SPRITE_SPELL_WATER_ICE_BLAST:
            return true;
        case SPRITE_SPELL_WATER_ICE_BLAST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.CarolinaBlue.c32(), effpar01);
            AddMobileLight(a2, colorTable.CarolinaBlue.c32(), 256);
            return false;
        case SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.CarolinaBlue.c32(), effpar01);
            return false;

        case SPRITE_SPELL_EARTH_STUN:
            // if ( !render->pRenderD3D )
            //  return true;
            _4A7C07_stun_spell_fx(a2);
            return false;

        case SPRITE_SPELL_EARTH_DEADLY_SWARM:
        case SPRITE_SPELL_EARTH_DEADLY_SWARM_IMPACT:
            return true;

        case SPRITE_SPELL_EARTH_ROCK_BLAST:
            return true;
        case SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.CarnabyTan.c32(), effpar01);
            return false;

        case SPRITE_SPELL_EARTH_TELEKINESIS:
            return true;

        case SPRITE_SPELL_EARTH_BLADES:
            return true;
        case SPRITE_SPELL_EARTH_BLADES_IMPACT:
            _4A7948_mind_blast_after_effect(a2);
            return false;

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.MediumGrey.c32(), effpar01);
            return true;
        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, colorTable.MediumGrey.c32(), effpar01, 200.0);
            return false;
        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.MediumGrey.c32(), effpar01);
            return false;

        case SPRITE_SPELL_EARTH_MASS_DISTORTION:
            return false;

        case SPRITE_SPELL_MIND_MIND_BLAST:
        case SPRITE_SPELL_MIND_TELEPATHY:
        case SPRITE_SPELL_MIND_BERSERK:
        case SPRITE_SPELL_MIND_CHARM:
        case SPRITE_SPELL_MIND_MASS_FEAR:
        case SPRITE_SPELL_MIND_ENSLAVE:
        case SPRITE_SPELL_MIND_PSYCHIC_SHOCK:
            return true;

        case SPRITE_SPELL_MIND_MIND_BLAST_IMPACT:
            _4A7948_mind_blast_after_effect(a2);
            return false;

        case SPRITE_SPELL_BODY_HARM:
            // if ( !render->pRenderD3D )
            //  return true;
            //_4A78AE_sparks_spell(a2);
            AddMobileLight(a2, colorTable.GreenishBrown.c32(), 128);
            return true;
            //return false;
        case SPRITE_SPELL_BODY_HARM_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.BloodRed.c32(), effpar01);
            return false;

        case SPRITE_SPELL_BODY_FLYING_FIST:
            return true;
        case SPRITE_SPELL_BODY_FLYING_FIST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.BloodRed.c32(), effpar01);
            AddMobileLight(a2, colorTable.BloodRed.c32(), 256);
            return false;

        case SPRITE_SPELL_LIGHT_LIGHT_BOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, colorTable.White.c32(), effpar03);
            AddMobileLight(a2, colorTable.White.c32(), 128);
            return false;
        case SPRITE_SPELL_LIGHT_LIGHT_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.White.c32(), effpar02);
            AddMobileLight(a2, colorTable.White.c32(), 256);
            return false;

        case SPRITE_SPELL_LIGHT_DESTROY_UNDEAD:
            AddMobileLight(a2, colorTable.White.c32(), 64);
            return false;

        case SPRITE_SPELL_LIGHT_PARALYZE:
            return true;

        case SPRITE_SPELL_LIGHT_SUMMON_ELEMENTAL:
        case SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT:
            return true;

        case SPRITE_SPELL_LIGHT_SUNRAY:
            AddMobileLight(a2, colorTable.White.c32(), 128);
            // if ( !render->pRenderD3D )
            //  return true;
            AddProjectile(a2, 100, nullptr);
            return false;
        case SPRITE_SPELL_LIGHT_SUNRAY_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.White.c32(), effpar03);
            return false;

        case SPRITE_SPELL_DARK_REANIMATE:
            return true;

        case SPRITE_SPELL_DARK_TOXIC_CLOUD:
        case SPRITE_SPELL_DARK_SHRINKING_RAY:
        case SPRITE_SPELL_DARK_CONTROL_UNDEAD:
            return true;

        case SPRITE_SPELL_DARK_SHARPMETAL:
            return true;
        case SPRITE_SPELL_DARK_SHARPMETAL_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, colorTable.MediumGrey.c32(), effpar01);
            return false;

        case SPRITE_SPELL_DARK_SACRIFICE:
        case SPRITE_SPELL_DARK_DRAGON_BREATH:
        case SPRITE_SPELL_DARK_DRAGON_BREATH_1:
            return true;

        default:
            //logger->Info("sprite spell {}", a2->uType);
            return false;
    }
}

//----- (004A89BD) --------------------------------------------------------
void SpellFxRenderer::SetPlayerBuffAnim(SPELL_TYPE uSpellID,
                                        uint16_t uPlayerID) {
    // SpellFxRenderer *v3; // edi@1
    PlayerBuffAnim *v4;  // esi@1
    const char *v6;      // [sp-4h] [bp-10h]@2

    v4 = &pPlayerBuffs[uPlayerID];
    v4->uSpellAnimTimeElapsed = 0;
    v4->bRender = uSpellID != SPELL_NONE;

    switch (uSpellID) {
        case SPELL_DISEASE:
            v6 = "zapp";
            break;

        case BECOME_MAGIC_GUILD_MEMBER:
        case SPELL_AIR_FEATHER_FALL:
        case SPELL_SPIRIT_DETECT_LIFE:
        case SPELL_SPIRIT_FATE:
            v6 = "spboost1";
            break;

        case SPELL_QUEST_COMPLETED:
        case SPELL_AIR_INVISIBILITY:
        case SPELL_WATER_WATER_WALK:
        case SPELL_SPIRIT_PRESERVATION:
            v6 = "spboost2";
            break;

        case SPELL_152:
            __debugbreak();  // spell id == 152 wtf
        case SPELL_LIGHT_HOUR_OF_POWER:
        case SPELL_LIGHT_DAY_OF_THE_GODS:
        case SPELL_LIGHT_DAY_OF_PROTECTION:
        case SPELL_LIGHT_DIVINE_INTERVENTION:
            v6 = "spboost3";
            break;

        case SPELL_SPIRIT_REMOVE_CURSE:
        case SPELL_MIND_REMOVE_FEAR:
        case SPELL_BODY_CURE_WEAKNESS:
            v6 = "spheal1";
            break;

        case SPELL_SPIRIT_SHARED_LIFE:
        case SPELL_MIND_CURE_PARALYSIS:
        case SPELL_MIND_CURE_INSANITY:
        case SPELL_BODY_FIRST_AID:
        case SPELL_BODY_CURE_POISON:
        case SPELL_BODY_CURE_DISEASE:
        case SPELL_DARK_SACRIFICE:
            v6 = "spheal2";
            break;

        case SPELL_BODY_POWER_CURE:
        case SPELL_DARK_SOULDRINKER:
            v6 = "spheal3";
            break;

        case SPELL_FIRE_PROTECTION_FROM_FIRE:
        case SPELL_FIRE_IMMOLATION:
            v6 = "spell03";
            break;

        case SPELL_FIRE_HASTE:
            v6 = "spell05";
            break;
        case SPELL_AIR_PROTECTION_FROM_AIR:
            v6 = "spell14";
            break;
        case SPELL_AIR_SHIELD:
            v6 = "spell17";
            break;
        case SPELL_WATER_PROTECTION_FROM_WATER:
            v6 = "spell25";
            break;
        case SPELL_EARTH_PROTECTION_FROM_EARTH:
            v6 = "spell36";
            break;
        case SPELL_EARTH_STONESKIN:
            v6 = "spell38";
            break;
        case SPELL_SPIRIT_BLESS:
            v6 = "spell46";
            break;
        case SPELL_SPIRIT_HEROISM:
            v6 = "spell51";
            break;
        case SPELL_SPIRIT_RESSURECTION:
            v6 = "spell55";
            break;
        case SPELL_MIND_PROTECTION_FROM_MIND:
            v6 = "spell58";
            break;
        case SPELL_BODY_PROTECTION_FROM_BODY:
            v6 = "spell69";
            break;
        case SPELL_BODY_REGENERATION:
            v6 = "spell71";
            break;
        case SPELL_BODY_HAMMERHANDS:
            v6 = "spell73";
            break;
        case SPELL_BODY_PROTECTION_FROM_MAGIC:
            v6 = "spell75";
            break;

        default:
            v4->bRender = false;
            return;
    }

    v4->uSpellIconID = pIconsFrameTable->FindIcon(v6);
    if (v4->bRender)
        v4->uSpellAnimTime =
            pIconsFrameTable->GetIcon(v4->uSpellIconID)->GetAnimLength();
}

void SpellFxRenderer::SetPartyBuffAnim(SPELL_TYPE uSpellID) {
    for (int i = 0; i < pParty->pPlayers.size(); i++) {
        SetPlayerBuffAnim(uSpellID, i);
    }
}

//----- (004A8BDF) --------------------------------------------------------
void SpellFxRenderer::FadeScreen__like_Turn_Undead_and_mb_Armageddon(
    unsigned int uDiffuseColor, unsigned int uFadeTime) {
    this->uFadeTime = uFadeTime;
    this->uFadeLength = uFadeTime;
    this->uFadeColor = uDiffuseColor;
}

//----- (004A8BFC) --------------------------------------------------------
int SpellFxRenderer::_4A8BFC_prismatic_light() {  // for SPELL_LIGHT_PRISMATIC_LIGHT
    uAnimLength =
        8 * pSpriteFrameTable
                ->pSpriteSFrames[pSpriteFrameTable->FastFindSprite("spell84")]
                .uAnimLength;
    return uAnimLength;
}

//----- (004A8C27) --------------------------------------------------------
void SpellFxRenderer::RenderSpecialEffects() {
    double v4;         // st7@4
    double v5;         // st6@4
    float v7;          // ST14_4@6
    unsigned int v8;   // ST14_4@8
    SpriteFrame *v10;  // eax@8
    // int v11; // edi@8
    RenderVertexD3D3 vd3d[4];  // [sp+60h] [bp-8Ch]@9

    if (uNumProjectiles) {
        DrawProjectiles();
        uNumProjectiles = 0;
    }

    field_204 = 0;
    if (uFadeTime > 0) {
        v4 = (double)uFadeTime / (double)uFadeLength;
        v5 = 1.0 - v4 * v4;
        // v6 = v5;
        if (v5 > 0.9) v5 = 1.0 - (v5 - 0.9) * 10.0;
        v7 = v5;
        render->ScreenFade(uFadeColor, v7);
        uFadeTime -= pEventTimer->uTimeElapsed;
    }

    if (uAnimLength > 0) {
        // prismatic light
        v8 = 8 * pSpriteFrameTable->pSpriteSFrames[pSpriteFrameTable->FastFindSprite("spell84")].uAnimLength - uAnimLength;
        v10 = pSpriteFrameTable->GetFrame(pSpriteFrameTable->FastFindSprite("spell84"), v8);
        int pal = v10->GetPaletteIndex();
        uAnimLength -= pEventTimer->uTimeElapsed;

        render->DrawSpecialEffectsQuad(v10->hw_sprites[0]->texture, pal);
    }
}

//----- (004A902A) --------------------------------------------------------
void SpellFxRenderer::DrawPlayerBuffAnims() {
    for (uint i = 0; i < 4; ++i) {
        PlayerBuffAnim *buff = &pPlayerBuffs[i];
        if (!buff->bRender) continue;

        buff->uSpellAnimTimeElapsed += pEventTimer->uTimeElapsed;
        if (buff->uSpellAnimTimeElapsed >= buff->uSpellAnimTime) {
            buff->bRender = false;
            continue;
        }

        Icon *icon = pIconsFrameTable->GetFrame(buff->uSpellIconID,
                                                buff->uSpellAnimTimeElapsed);
        render->DrawTextureNew(
            pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f,
            385 / 480.0f, icon->GetTexture());
    }
}

//----- (004A90A0) --------------------------------------------------------
void SpellFxRenderer::LoadAnimations() {
    effpar01 = assets->GetBitmap("effpar01");  // pBitmaps_LOD->LoadTexture("effpar01");
    effpar02 = assets->GetBitmap("effpar02");  // pBitmaps_LOD->LoadTexture("effpar02");
    effpar03 = assets->GetBitmap("effpar03");  // pBitmaps_LOD->LoadTexture("effpar03");

    // spell not implemented in the game
    // uSpriteID_sp57c = pSprites_LOD->LoadSprite("sp57c", 6);

    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("zapp"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spheal1"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spheal2"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spheal3"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spboost1"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spboost2"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spboost3"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell03"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell05"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell14"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell17"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell21"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell25"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell27"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell36"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell38"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell46"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell51"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell55"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell58"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell69"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell71"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell73"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell75"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("spell96"));

    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell01"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell02"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell03"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell09"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell11"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell18"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell22"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell26"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell29"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell39"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell39c"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell41"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell57c"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell62"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell65"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell66"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell70"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell76"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell84"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell90"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell92"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell93"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell97"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell97c"));
    pSpriteFrameTable->InitializeSprite(pSpriteFrameTable->FastFindSprite("spell97c"));
}

//----- (004775ED) --------------------------------------------------------
int SpellFX_Billboard::SpellFXNearClipAdjust(float NearClip) {  // near clip adjust - needs diffuse sorting properly??
    if (!uNumVertices) return 0;

    // copies first vert to position 4
    field_64[uNumVertices].x = field_64[0].x;
    field_64[uNumVertices].y = field_64[0].y;
    field_64[uNumVertices].z = field_64[0].z;
    field_64[uNumVertices].diffuse = field_64[0].diffuse;

    bool currvert = (field_64[0].x <= NearClip);
    bool nextvert = false;
    int ProducedVerts = 0;
    double Tmult;

    for (int i = 0; i  < uNumVertices; ++i) {  // cycle through
       nextvert = (field_64[i + 1].x <= NearClip);

       if (currvert ^ nextvert) {  // XOR
           if (nextvert) {  // adjust verts and copy out
               Tmult = (NearClip - field_64[i].x) / (field_64[i + 1].x - field_64[i].x);

               // x view = nearclip
               field_B4[ProducedVerts].x = NearClip;
               field_B4[ProducedVerts].y = (field_64[i + 1].y - field_64[i].y) * Tmult + field_64[i].y;
               field_B4[ProducedVerts].z = (field_64[i + 1].z - field_64[i].z) * Tmult + field_64[i].z;
               field_B4[ProducedVerts].diffuse = field_64[i].diffuse;  // (field_64[(i+1) * 4 + 3] - field_64[i * 4 + 3]) * Tmult + field_64[i * 4 + 3];

           } else {  // currvert
               Tmult = (NearClip - field_64[i].x) / (field_64[i].x - field_64[i + 1].x);

               // x view = nearclip
               field_B4[ProducedVerts].x = NearClip;
               field_B4[ProducedVerts].y = (field_64[i].y - field_64[i + 1].y) * Tmult + field_64[i].y;
               field_B4[ProducedVerts].z = (field_64[i].z - field_64[i + 1].z) * Tmult + field_64[i].z;
               field_B4[ProducedVerts].diffuse = field_64[i].diffuse;  // (field_64[i * 4 + 3] - field_64[(i+1) * 4 + 3]) * Tmult + field_64[i * 4 + 3];
           }
           ++ProducedVerts;
       }

       if (!nextvert) {
           // copy out - vert doesnt need adjusting

           field_B4[ProducedVerts].x = field_64[i + 1].x;
           field_B4[ProducedVerts].y = field_64[i + 1].y;
           field_B4[ProducedVerts].z = field_64[i + 1].z;
           field_B4[ProducedVerts].diffuse = field_64[i + 1].diffuse;

           ++ProducedVerts;
       }
       currvert = nextvert;
    }

    return this->uNumVertices = ProducedVerts;
}

//----- (00477927) --------------------------------------------------------
int SpellFX_Billboard::SpellFXFarClipAdjust(float farclip) {  // far clip adjust - needs diffuse sorting properly??
    // refactored but not tested
    // __debugbreak();

    if (!uNumVertices) return 0;

    // copies first vert to position 4
    field_64[uNumVertices].x = field_64[0].x;
    field_64[uNumVertices].y = field_64[0].y;
    field_64[uNumVertices].z = field_64[0].z;
    field_64[uNumVertices].diffuse = field_64[0].diffuse;

    bool currvert = (field_64[0].x >= farclip);
    bool nextvert = false;
    int ProducedVerts = 0;
    double Tmult;

    for (int i = 0; i < uNumVertices; ++i) {  // cycle through
        nextvert = field_64[i + 1].x >= farclip;

        if (currvert ^ nextvert) {  // XOR
            if (nextvert) {  // adjust verts and copy out
                Tmult = (farclip - field_64[i].x) / (field_64[i + 1].x - field_64[i].x);

                // x view = nearclip
                field_B4[ProducedVerts].x = farclip;
                field_B4[ProducedVerts].y = (field_64[i + 1].y - field_64[i].y) * Tmult + field_64[i].y;
                field_B4[ProducedVerts].z = (field_64[i + 1].z - field_64[i].z) * Tmult + field_64[i].z;
                field_B4[ProducedVerts].diffuse = field_64[i].diffuse;  // (field_64[(i+1) * 4 + 3] - field_64[i * 4 + 3]) * Tmult + field_64[i * 4 + 3];
            } else {  // currvert
                Tmult = (farclip - field_64[i].x) / (field_64[i].x - field_64[i + 1].x);

                // x view = nearclip
                field_B4[ProducedVerts].x = farclip;
                field_B4[ProducedVerts].y = (field_64[i].y - field_64[i + 1].y) * Tmult + field_64[i].y;
                field_B4[ProducedVerts].z = (field_64[i].z - field_64[i + 1].z) * Tmult + field_64[i].z;
                field_B4[ProducedVerts].diffuse = field_64[i * 4 + 3].diffuse;  // (field_64[i * 4 + 3] - field_64[(i+1) * 4 + 3]) * Tmult + field_64[i * 4 + 3];
            }
            ++ProducedVerts;
        }

        if (!nextvert) {
            // copy out - vert doesnt need adjusting

            field_B4[ProducedVerts].x = field_64[i + 1].x;
            field_B4[ProducedVerts].y = field_64[i + 1].y;
            field_B4[ProducedVerts].z = field_64[i + 1].z;
            field_B4[ProducedVerts].diffuse = field_64[i + 1].diffuse;

            ++ProducedVerts;
        }
        currvert = nextvert;
    }

    return this->uNumVertices = ProducedVerts;
}

//----- (00477C61) --------------------------------------------------------
int SpellFX_Billboard::SpellFXViewTransform() {  // view transform
    int ViewPosX;
    int ViewPosY;
    int ViewPosZ;

    if (this->uNumVertices > 0) {
        for (int v2 = 0; v2 < this->uNumVertices; ++v2) {
            // view tranfrom
            pCamera3D->ViewTransform(field_14[v2].x, field_14[v2].y, field_14[v2].z, &ViewPosX, &ViewPosY, &ViewPosZ);

            // load into field 64
            field_64[v2].x = ViewPosX;
            field_64[v2].y = ViewPosY;
            field_64[v2].z = ViewPosZ;
            field_64[v2].diffuse = field_14[v2].diffuse;
        }
    }

    this->uNumVertices = 3;
    return 1;
}

//----- (00477F63) --------------------------------------------------------
bool SpellFX_Billboard::SpellFXViewClip() {
    bool NeedNearClip = 0;
    bool NeedFarClip = 0;
    double NearClip = pCamera3D->GetNearClip();
    double FarClip = pCamera3D->GetFarClip();

    if (this->uNumVertices <= 0) {  //  what?? behaviour needs investigating
        memcpy(field_B4, field_64, uNumVertices * sizeof(local_01));
        return this->uNumVertices != 0;
    }

    for (int v6 = 0; v6 < this->uNumVertices; v6++) {
        if (NearClip >= field_64[v6].x || field_64[v6].x >= FarClip) {
            if (NearClip < field_64[v6].x)
                NeedFarClip = 1;
            else
                NeedNearClip = 1;
        }
    }

    if (!NeedNearClip) {
        if (NeedFarClip) {
            // far clip
            this->SpellFXFarClipAdjust(FarClip);
            return this->uNumVertices != 0;
        }

        // no clipping required- copy out
        for (int i = 0; i < uNumVertices; ++i) {
            field_B4[i].x = field_64[i].x;
            field_B4[i].y = field_64[i].y;
            field_B4[i].z = field_64[i].z;
            field_B4[i].diffuse = field_64[i].diffuse;
        }
        return this->uNumVertices != 0;
    }

    // near clip
    SpellFXNearClipAdjust(NearClip);
    return this->uNumVertices != 0;
}

//----- (0047802A) --------------------------------------------------------
int SpellFX_Billboard::SpellFXProject() {  // project to billboard coords
    int Result = 0;
    int Xproj, Yproj;

    for (int i = 0; i < this->uNumVertices; i++) {
        pCamera3D->Project(
            round_to_int(this->field_B4[i].x),
            round_to_int(this->field_B4[i].y),
            round_to_int(this->field_B4[i].z), &Yproj, &Xproj);

        field_104[i].x = (float)Yproj;
        field_104[i].y = (float)Xproj;
        field_104[i].z = field_B4[i].x;
        field_104[i].diffuse = field_B4[i].diffuse;

        if (true)
        /*(double)(signed int)pViewport->uViewportTL_X <=
                field_104[i].x &&
                (double)(signed int)pViewport->uViewportBR_X >
                field_104[i].x &&
                (double)(signed int)pViewport->uViewportTL_Y <=
                field_104[i].y &&
                (double)(signed int)pViewport->uViewportBR_Y >
                field_104[i].y*/
                Result = 1;
        }

    return Result;
}
