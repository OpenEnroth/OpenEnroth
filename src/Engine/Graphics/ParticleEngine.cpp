#include "Engine/Graphics/ParticleEngine.h"

#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Random/Random.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"
#include "Engine/Time.h"

#include "Utility/Math/TrigLut.h"

#include "Outdoor.h"
#include "Sprites.h"

TrailParticleGenerator trail_particle_generator;

//----- (00440DF5) --------------------------------------------------------
void TrailParticleGenerator::AddParticle(int x, int y, int z, Color color) {
    particles[num_particles].x = x;
    particles[num_particles].y = y;
    particles[num_particles].z = z;
    particles[num_particles].time_to_live = vrng->random(64) + 256;
    particles[num_particles].time_left = particles[num_particles].time_to_live;
    particles[num_particles].color = color;

    num_particles++;
    assert(num_particles < 100);
}

//----- (00440E91) --------------------------------------------------------
void TrailParticleGenerator::GenerateTrailParticles(int x, int y, int z,
                                                    Color color) {
    for (int i = 0, count = 5 + vrng->random(6); i < count; ++i)
        AddParticle(vrng->random(33) + x - 16, vrng->random(33) + y - 16, vrng->random(33) + z, color);
}

//----- (00440F07) --------------------------------------------------------
void TrailParticleGenerator::UpdateParticles() {
    for (unsigned int i = 0; i < 100; ++i) {
        if (particles[i].time_left > 0) {
            particles[i].x += vrng->random(5) + 4;
            particles[i].y += vrng->random(5) - 2;
            particles[i].z += vrng->random(5) - 2;
            particles[i].time_left -= pEventTimer->uTimeElapsed;
        }
    }
}

ParticleEngine::ParticleEngine() {
    ResetParticles();
}

void ParticleEngine::ResetParticles() {
    pParticles.fill({});
    uStartParticle = PARTICLES_ARRAY_SIZE;
    uEndParticle = 0;
    uTimeElapsed = 0;
}

void ParticleEngine::AddParticle(Particle_sw *particle) {
    if (!pMiscTimer->bPaused) {
        Particle *freeParticle = nullptr;

        for (int i = 0; i < pParticles.size(); i++) {
            if (pParticles[i].type == ParticleType_Invalid) {
                if (i < this->uStartParticle) {
                    this->uStartParticle = i;
                }
                if (i > this->uEndParticle) {
                    this->uEndParticle = i;
                }
                freeParticle = &pParticles[i];
                break;
            }
        }

        if (freeParticle) {
            freeParticle->type = particle->type;
            freeParticle->x = particle->x;
            freeParticle->y = particle->y;
            freeParticle->z = particle->z;
            freeParticle->_x = particle->x;
            freeParticle->_y = particle->y;
            freeParticle->_z = particle->z;
            freeParticle->shift_x = particle->r; // TODO: seems Particle_sw struct fields are mixed up here
            freeParticle->shift_y = particle->g;
            freeParticle->shift_z = particle->b;
            freeParticle->uParticleColor = particle->uDiffuse;
            freeParticle->uLightColor_bgr = particle->uDiffuse;
            // v6 = (v4->uType & 4) == 0;
            freeParticle->timeToLive = particle->timeToLive;
            freeParticle->texture = particle->texture;
            freeParticle->paletteID = particle->paletteID;
            freeParticle->particle_size = particle->particle_size;
            if (freeParticle->type & ParticleType_Rotating) {
                freeParticle->rotation_speed = vrng->random(256) - 128;
                freeParticle->angle = vrng->random(TrigLUT.uIntegerDoublePi);
            } else {
                freeParticle->rotation_speed = 0;
                freeParticle->angle = 0;
            }
        }
    }
}

void ParticleEngine::Draw() {
    uTimeElapsed += pEventTimer->uTimeElapsed;
    pLines.uNumLines = 0;

    DrawParticles_BLV();
    if (pLines.uNumLines) {
        render->DrawLines(pLines.pLineVertices, pLines.uNumLines);
    }
}

void ParticleEngine::UpdateParticles() {
    unsigned uCurrentEnd = 0;
    unsigned uCurrentBegin = PARTICLES_ARRAY_SIZE;
    int time = pMiscTimer->bPaused == 0 ? pEventTimer->uTimeElapsed : 0;

    if (time == 0) {
        return;
    }

    for (unsigned int i = uStartParticle; i <= uEndParticle; ++i) {
        Particle *p = &pParticles[i];

        if (p->type == ParticleType_Invalid) {
            continue;
        }

        if (p->timeToLive <= time) {
            p->timeToLive = 0;
            p->type = ParticleType_Invalid;
            continue;
        }

        p->timeToLive -= time;

        // Line particle type appear unused
        if (p->type & ParticleType_Line) {
            p->_x = p->x;
            p->_y = p->y;
            p->_z = p->z;
        }

        // Dropping particles drop downward with acceleration
        if (p->type & ParticleType_Dropping) {
            p->shift_z -= time * 5.0;
        }

        // Ascending particles slowly float upward
        if (p->type & ParticleType_Ascending) {
            p->x += (vrng->random(5) - 2) * time / 16.0;
            p->y += (vrng->random(5) - 2) * time / 16.0;
            p->z += (vrng->random(5) + 4) * time / 16.0;
        }

        // v9 = (signed int)(time * p->rotation_speed) / 16;

        // Particle shift with time
        double shift = time / 128.0f;
        p->x += shift * p->shift_x;
        p->y += shift * p->shift_y;
        p->z += shift * p->shift_z;

        p->angle += time * p->rotation_speed / 16;

        // With time particles become more transparent
        int dissipate_value = 2 * p->timeToLive;
        if (dissipate_value >= 255) {
            dissipate_value = 255;
        }
        float dissipate_factor = dissipate_value / 255.0f;
        // v10 = (double)v22 * 0.0039215689;
        // TODO(Nik-RE-dev): check colour format use in particles
        p->uLightColor_bgr = Color(floorf(p->uParticleColor.r * dissipate_factor + 0.5),
                                   floorf(p->uParticleColor.g * dissipate_factor + 0.5),
                                   floorf(p->uParticleColor.b * dissipate_factor + 0.5));

        if (i < uCurrentBegin) {
           uCurrentBegin = i;
        }
        if (i > uCurrentEnd) {
           uCurrentEnd = i;
        }
    }

    uEndParticle = uCurrentEnd;
    uStartParticle = uCurrentBegin;
}

bool ParticleEngine::ViewProject_TrueIfStillVisible_BLV(unsigned int uParticleID) {
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

    if (!pCamera3D->ViewClip(x_int, y_int_, z_int_, &xt, &yt, &zt, 0)) return false;
    pCamera3D->Project(xt, yt, zt, &pParticle->uScreenSpaceX, &pParticle->uScreenSpaceY);

    pParticle->fov_x = pCamera3D->ViewPlaneDistPixels;

    /*pParticle->screenspace_scale = fixed::FromFloat(pParticle->particle_size) *
                                   fixed::FromFloat(pParticle->fov_x) / x;*/

    pParticle->screenspace_scale = /*fixed::FromFloat*/(pParticle->particle_size) *
        /*fixed::FromFloat*/(pParticle->fov_x) / /*fixed::FromInt*/(xt);

    /*pParticle->zbuffer_depth = x.GetInt();*/
    pParticle->zbuffer_depth = xt;

    return true;
}

void ParticleEngine::DrawParticles_BLV() {
    SoftwareBillboard v15 {};  // [sp+Ch] [bp-58h]@1

    v15.sParentBillboardID = -1;

    for (unsigned int i = uStartParticle; i <= uEndParticle; ++i) {
        Particle *p = &pParticles[i];

        if (p->type == ParticleType_Invalid) continue;

        if (!ViewProject_TrueIfStillVisible_BLV(i)) continue;

        // TODO(pskelton): reinstate this guard check
        // TODO(Nik-RE-dev): all types except for Line appear to behave identically
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
                v15.screen_space_z = p->zbuffer_depth;
                v15.paletteID = p->paletteID;
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
                    pLines.pLineVertices[2 * pLines.uNumLines].specular = Color();
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
                    pLines.pLineVertices[2 * pLines.uNumLines + 1].specular = Color();
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
                v15.screen_space_z = p->zbuffer_depth;
                v15.paletteID = p->paletteID;
                render->MakeParticleBillboardAndPush(
                    &v15, p->texture, p->uLightColor_bgr, p->angle);
            } else if (p->type & ParticleType_Sprite) {
                v15.screenspace_projection_factor_x = p->screenspace_scale;
                v15.screenspace_projection_factor_y = p->screenspace_scale;
                v15.screen_space_x = p->uScreenSpaceX;
                v15.screen_space_y = p->uScreenSpaceY;
                v15.screen_space_z = p->zbuffer_depth;
                v15.paletteID = p->paletteID;
                render->MakeParticleBillboardAndPush(
                    &v15, p->texture, p->uLightColor_bgr, p->angle);
            }
        }
    }
}

