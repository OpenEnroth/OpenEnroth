#include "Engine/Graphics/ParticleEngine.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"
#include "Engine/Time.h"

#include "Outdoor.h"
#include "Viewport.h"

#include "Sprites.h"

TrailParticleGenerator trail_particle_generator;

//----- (00440DF5) --------------------------------------------------------
void TrailParticleGenerator::AddParticle(int x, int y, int z, int bgr16) {
    particles[num_particles].x = x;
    particles[num_particles].y = y;
    particles[num_particles].z = z;
    particles[num_particles].time_to_live = rand() % 64 + 256;
    particles[num_particles].time_left = particles[num_particles].time_to_live;
    particles[num_particles].bgr16 = bgr16;

    num_particles++;
    assert(num_particles < 100);
}

//----- (00440E91) --------------------------------------------------------
void TrailParticleGenerator::GenerateTrailParticles(int x, int y, int z,
                                                    int bgr16) {
    for (int i = 0; i < 5 + rand() % 6; ++i)
        AddParticle(rand() % 33 + x - 16, rand() % 33 + y - 16, rand() % 33 + z,
                    bgr16);
}

//----- (00440F07) --------------------------------------------------------
void TrailParticleGenerator::UpdateParticles() {
    for (uint i = 0; i < 100; ++i) {
        if (particles[i].time_left > 0) {
            particles[i].x += rand() % 5 + 4;
            particles[i].y += rand() % 5 - 2;
            particles[i].z += rand() % 5 - 2;
            particles[i].time_left -= pEventTimer->uTimeElapsed;
        }
    }
}

//----- (0048AAC5) --------------------------------------------------------
ParticleEngine::ParticleEngine() {
    for (uint i = 0; i < 500; ++i)
        memset(&pParticles[i], 0, sizeof(pParticles[i]));

    ResetParticles();
}

//----- (0048AAF6) --------------------------------------------------------
void ParticleEngine::ResetParticles() {
    memset(pParticles, 0, 500 * sizeof(*pParticles));
    uStartParticle = 500;
    uEndParticle = 0;
    uTimeElapsed = 0;
}

//----- (0048AB23) --------------------------------------------------------
void ParticleEngine::AddParticle(Particle_sw *a2) {
    signed int v2;  // eax@2
    Particle *v3;   // edx@2
    Particle *v4;   // esi@10
    int v5;         // ecx@10
    // char v6; // zf@10

    if (!pMiscTimer->bPaused) {
        v2 = 0;
        v3 = this->pParticles;
        do {
            if (v3->type == ParticleType_Invalid) break;
            ++v2;
            ++v3;
        } while (v2 < 500);
        if (v2 < 500) {
            if (v2 < this->uStartParticle) this->uStartParticle = v2;
            if (v2 > this->uEndParticle) this->uEndParticle = v2;
            v4 = &this->pParticles[v2];
            v4->type = a2->type;
            v4->x = a2->x;
            v4->y = a2->y;
            v4->z = a2->z;
            v4->_x = a2->x;
            v4->_y = a2->y;
            v4->_z = a2->z;
            v4->flt_10 = a2->r;
            v4->flt_14 = a2->g;
            v4->flt_18 = a2->b;
            v5 = a2->uDiffuse;
            v4->uParticleColor = v5;
            v4->uLightColor_bgr = v5;
            // v6 = (v4->uType & 4) == 0;
            v4->timeToLive = a2->timeToLive;
            v4->texture = a2->texture;
            v4->particle_size = a2->particle_size;
            if (v4->type & ParticleType_Rotating) {
                v4->rotation_speed = (rand() % 256) - 128;
                v4->angle = rand();
            } else {
                v4->rotation_speed = 0;
                v4->angle = 0;
            }
        }
    }
}

//----- (0048ABF3) --------------------------------------------------------
void ParticleEngine::Draw() {
    uTimeElapsed += pEventTimer->uTimeElapsed;
    pLines.uNumLines = 0;

    DrawParticles_BLV();
    // if (render->pRenderD3D)
    {
        if (pLines.uNumLines) {
            render->DrawLines(pLines.pLineVertices, pLines.uNumLines);
            /*render->pRenderD3D->pDevice->SetTexture(0, 0);
            render->pRenderD3D->pDevice->DrawPrimitive(
              D3DPT_LINELIST,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              pLines.pLineVertices,
              pLines.uNumLines,
              D3DDP_DONOTLIGHT);*/
        }
    }
}

//----- (0048AC65) --------------------------------------------------------
void ParticleEngine::UpdateParticles() {
    unsigned int time;  // edi@1
    // int v5; // eax@3
    // char v6; // sf@4
    float v7;   // ST4C_4@11
    double v8;  // st7@12
    // int v9; // eax@12
    // double v10; // st7@14
    signed int v19;      // [sp+38h] [bp-14h]@1
    int v20;             // [sp+3Ch] [bp-10h]@1
    unsigned int time_;  // [sp+40h] [bp-Ch]@1
    int v22;             // [sp+44h] [bp-8h]@12

    v20 = 0;
    time = pMiscTimer->bPaused == 0 ? pEventTimer->uTimeElapsed : 0;
    v19 = 500;
    time_ = pMiscTimer->bPaused == 0 ? pEventTimer->uTimeElapsed : 0;

    for (uint i = uStartParticle; i <= uEndParticle; ++i) {
        Particle *p = &pParticles[i];

        if (p->type == ParticleType_Invalid) continue;

        if (p->timeToLive <= time) {
            p->timeToLive = 0;
            p->type = ParticleType_Invalid;
            continue;
        }
        p->timeToLive -= time;

        if (p->type & ParticleType_Line) {
            p->_x = p->x;
            p->_y = p->y;
            p->_z = p->z;
        }

        if (p->type & ParticleType_1)
            p->flt_18 = p->flt_18 - (double)(signed int)time_ * 5.0;

        if (p->type & ParticleType_8) {
            v7 = (double)(signed int)time_;
            *(float *)&p->x += (double)(rand() % 5 - 2) * v7 / 16.0f;
            *(float *)&p->y += (double)(rand() % 5 - 2) * v7 / 16.0f;
            *(float *)&p->z += (double)(rand() % 5 + 4) * v7 / 16.0f;
        }
        v8 = (double)(signed int)time_ / 128.0f;
        // v9 = (signed int)(time * p->rotation_speed) / 16;

        p->x += v8 * p->flt_10;
        p->y += v8 * p->flt_14;
        p->z += v8 * p->flt_18;

        p->angle += time * p->rotation_speed / 16;
        v22 = 2 * p->timeToLive;
        if (v22 >= 255) v22 = 255;
        // v10 = (double)v22 * 0.0039215689;
        p->uLightColor_bgr = ((uint)floorf(p->b * (v22 / 255.0f) + 0.5) << 16) |
                             ((uint)floorf(p->g * (v22 / 255.0f) + 0.5) << 8) |
                             ((uint)floorf(p->r * (v22 / 255.0f) + 0.5) << 0);
        if (i < v19) v19 = i;
        if (i > v20) v20 = i;
    }

    uEndParticle = v20;
    uStartParticle = v19;
}

//----- (0048AE74) --------------------------------------------------------
bool ParticleEngine::ViewProject_TrueIfStillVisible_BLV(
    unsigned int uParticleID) {
    Particle *pParticle;  // esi@1
    int y_int_;           // [sp+10h] [bp-40h]@2
    int x_int;            // [sp+20h] [bp-30h]@2
    int z_int_;           // [sp+24h] [bp-2Ch]@2

    pParticle = &this->pParticles[uParticleID];
    if (pParticle->type == ParticleType_Invalid) return 0;
    // uParticleID = LODWORD(pParticle->x);
    // v56 = *(float *)&uParticleID + 6.7553994e15;
    x_int = floorf(pParticle->x + 0.5f);
    // uParticleID = LODWORD(pParticle->y);
    // y_int_ = *(float *)&uParticleID + 6.7553994e15;
    y_int_ = floorf(pParticle->y + 0.5f);
    // uParticleID = LODWORD(pParticle->z);
    // z_int_ = *(float *)&uParticleID + 6.7553994e15;
    z_int_ = floorf(pParticle->z + 0.5f);

    /*fixed x, y, z;*/
    int xt, yt, zt;

    /*if (!pIndoorCameraD3D->ApplyViewTransform_TrueIfStillVisible_BLV(
            x_int, y_int_, z_int_, &x, &y, &z, 1))
        return false;*/
    if (!pIndoorCameraD3D->ViewClip(x_int, y_int_, z_int_, &xt, &yt, &zt, 0)) return false;

    /*pIndoorCameraD3D->Project(x.GetInt(), y.GetInt(), z.GetInt(),
                              &pParticle->uScreenSpaceX,
                              &pParticle->uScreenSpaceY);*/
    pIndoorCameraD3D->Project(xt, yt, zt, &pParticle->uScreenSpaceX, &pParticle->uScreenSpaceY);

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        pParticle->fov_x = pIndoorCameraD3D->fov_x;
    } else {
        pParticle->fov_x = pODMRenderParams->int_fov_rad;
    }


    // pParticle->fov_x = pIndoorCameraD3D->fov_x; //
    pParticle->fov_y = pIndoorCameraD3D->fov_y;

    /*pParticle->screenspace_scale = fixed::FromFloat(pParticle->particle_size) *
                                   fixed::FromFloat(pParticle->fov_x) / x;*/

    pParticle->screenspace_scale = /*fixed::FromFloat*/(pParticle->particle_size) *
        /*fixed::FromFloat*/(pParticle->fov_x) / /*fixed::FromInt*/(xt);

    /*pParticle->zbuffer_depth = x.GetInt();*/
    pParticle->zbuffer_depth = xt;

    return true;
}



//----- (0048BBA6) --------------------------------------------------------
void ParticleEngine::DrawParticles_BLV() {
    SoftwareBillboard v15;  // [sp+Ch] [bp-58h]@1

    v15.sParentBillboardID = -1;

    for (uint i = uStartParticle; i <= uEndParticle; ++i) {
        Particle *p = &pParticles[i];

        if (p->type == ParticleType_Invalid) continue;

        if (!ViewProject_TrueIfStillVisible_BLV(i)) continue;

        if (true) {
            /*p->uScreenSpaceX >= pBLVRenderParams->uViewportX &&
            p->uScreenSpaceX < pBLVRenderParams->uViewportZ &&
            p->uScreenSpaceY >= pBLVRenderParams->uViewportY &&
            p->uScreenSpaceY < pBLVRenderParams->uViewportW) { */
            if (p->type & ParticleType_Diffuse) {
                // v14 = &pParticles[i];
                v15.screenspace_projection_factor_x = p->screenspace_scale;
                v15.screenspace_projection_factor_y = p->screenspace_scale;
                v15.screen_space_x = p->uScreenSpaceX;
                v15.screen_space_y = p->uScreenSpaceY;
                v15.object_pid = p->object_pid;
                v15.screen_space_z = p->zbuffer_depth;
                render->MakeParticleBillboardAndPush(
                    &v15, 0, p->uLightColor_bgr, p->angle);
            } else if (p->type & ParticleType_Line) {  // type doesnt appear to be used
                if (pLines.uNumLines < 100) {
                    pLines.pLineVertices[2 * pLines.uNumLines].pos.x =
                        p->uScreenSpaceX;
                    pLines.pLineVertices[2 * pLines.uNumLines].pos.y =
                        p->uScreenSpaceY;
                    pLines.pLineVertices[2 * pLines.uNumLines].pos.z =
                        1.0 - 1.0 / (p->zbuffer_depth * 0.061758894);
                    pLines.pLineVertices[2 * pLines.uNumLines].rhw = 1.0;
                    pLines.pLineVertices[2 * pLines.uNumLines].diffuse =
                        p->uLightColor_bgr;
                    pLines.pLineVertices[2 * pLines.uNumLines].specular = 0;
                    pLines.pLineVertices[2 * pLines.uNumLines].texcoord.x = 0.0;
                    pLines.pLineVertices[2 * pLines.uNumLines].texcoord.y = 0.0;

                    pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.x =
                        p->uScreenSpaceZ;  // where is this set?
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.y =
                        p->uScreenSpaceW;
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.z =
                        1.0 - 1.0 / ((short)p->sZValue2 * 0.061758894);
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].rhw = 1.0;
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].diffuse =
                        p->uLightColor_bgr;
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].specular = 0;
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].texcoord.x =
                        0.0;
                    pLines.pLineVertices[2 * pLines.uNumLines++ + 1]
                        .texcoord.y = 0.0;
                }
            } else if (p->type & ParticleType_Bitmap) {
                v15.screenspace_projection_factor_x = p->screenspace_scale;
                v15.screenspace_projection_factor_y = p->screenspace_scale;
                v15.screen_space_x = p->uScreenSpaceX;
                v15.screen_space_y = p->uScreenSpaceY;
                v15.object_pid = p->object_pid;
                v15.screen_space_z = p->zbuffer_depth;
                render->MakeParticleBillboardAndPush(
                    &v15, p->texture, p->uLightColor_bgr, p->angle);
            } else if (p->type & ParticleType_Sprite) {
                v15.screenspace_projection_factor_x = p->screenspace_scale;
                v15.screenspace_projection_factor_y = p->screenspace_scale;
                v15.screen_space_x = p->uScreenSpaceX;
                v15.screen_space_y = p->uScreenSpaceY;
                v15.object_pid = p->object_pid;
                v15.screen_space_z = p->zbuffer_depth;
                render->MakeParticleBillboardAndPush(
                    &v15, p->texture, p->uLightColor_bgr, p->angle);
            }
        }
    }
}

