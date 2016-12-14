#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Engine/OurMath.h"
#include "Engine/LOD.h"

#include "ParticleEngine.h"
#include "Viewport.h"
#include "Outdoor.h"

#include "Sprites.h"

TrailParticleGenerator trail_particle_generator;


//----- (00440DF5) --------------------------------------------------------
void TrailParticleGenerator::AddParticle(int x, int y, int z, int bgr16)
{
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
void TrailParticleGenerator::GenerateTrailParticles(int x, int y, int z, int bgr16)
{
  for (int i = 0; i < 5 + rand() % 6; ++i)
    AddParticle(rand() % 33 + x - 16,
                rand() % 33 + y - 16,
                rand() % 33 + z, bgr16);
}

//----- (00440F07) --------------------------------------------------------
void TrailParticleGenerator::UpdateParticles()
{
  for (uint i = 0; i < 100; ++i)
  {
    if (particles[i].time_left > 0)
    {
      particles[i].x += rand() % 5 + 4;
      particles[i].y += rand() % 5 - 2;
      particles[i].z += rand() % 5 - 2;
      particles[i].time_left -= pEventTimer->uTimeElapsed;
    }
  }
}

//----- (0048AAC5) --------------------------------------------------------
ParticleEngine::ParticleEngine()
{
  for (uint i = 0; i < 500; ++i)
    memset(&pParticles[i], 0, sizeof(pParticles[i]));

  ResetParticles();
}

//----- (0048AAF6) --------------------------------------------------------
void ParticleEngine::ResetParticles()
{
  memset(pParticles, 0, 500 * sizeof(*pParticles));
  uStartParticle = 500;
  uEndParticle = 0;
  uTimeElapsed = 0;
}

//----- (0048AB23) --------------------------------------------------------
void ParticleEngine::AddParticle(Particle_sw *a2)
{
  signed int v2; // eax@2
  Particle *v3; // edx@2
  Particle *v4; // esi@10
  int v5; // ecx@10
  //char v6; // zf@10

  if ( !pMiscTimer->bPaused )
  {
    v2 = 0;
    v3 = (Particle *)this;
    do
    {
      if (v3->type == ParticleType_Invalid)
        break;
      ++v2;
      ++v3;
    }
    while ( v2 < 500 );
    if ( v2 < 500 )
    {
      if ( v2 < this->uStartParticle )
        this->uStartParticle = v2;
      if ( v2 > this->uEndParticle )
        this->uEndParticle = v2;
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
      //v6 = (v4->uType & 4) == 0;
      v4->timeToLive = a2->timeToLive;
      v4->uTextureID = a2->uTextureID;
      v4->flt_28 = a2->flt_28;
      if (v4->type & ParticleType_Rotating)
      {
        v4->rotation_speed = (rand() % 256) - 128;
        v4->angle = rand();
      }
      else
      {
        v4->rotation_speed = 0;
        v4->angle = 0;
      }
    }
  }
}

//----- (0048ABF3) --------------------------------------------------------
void ParticleEngine::Draw()
{
  uTimeElapsed += pEventTimer->uTimeElapsed;
  pLines.uNumLines = 0;

  if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
    DrawParticles_BLV();
  else
    DrawParticles_ODM();

  //if (pRenderer->pRenderD3D)
  {
    if (pLines.uNumLines)
    {
      pRenderer->DrawLines(pLines.pLineVertices, pLines.uNumLines);
      /*pRenderer->pRenderD3D->pDevice->SetTexture(0, 0);
      pRenderer->pRenderD3D->pDevice->DrawPrimitive(
        D3DPT_LINELIST,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
        pLines.pLineVertices,
        pLines.uNumLines,
        D3DDP_DONOTLIGHT);*/
    }
  }
}

//----- (0048AC65) --------------------------------------------------------
void ParticleEngine::UpdateParticles()
{
  unsigned int time; // edi@1
  //int v5; // eax@3
  //char v6; // sf@4
  float v7; // ST4C_4@11
  double v8; // st7@12
  //int v9; // eax@12
  //double v10; // st7@14
  signed int v19; // [sp+38h] [bp-14h]@1
  int v20; // [sp+3Ch] [bp-10h]@1
  unsigned int time_; // [sp+40h] [bp-Ch]@1
  int v22; // [sp+44h] [bp-8h]@12

  v20 = 0;
  time = pMiscTimer->bPaused == 0 ? pEventTimer->uTimeElapsed : 0;
  v19 = 500;
  time_ = pMiscTimer->bPaused == 0 ? pEventTimer->uTimeElapsed : 0;

  for (uint i = uStartParticle; i <= uEndParticle; ++i)
  {
    Particle* p = &pParticles[i];

      if (p->type == ParticleType_Invalid)
        continue;

      if (p->timeToLive <= time)
      {
        p->timeToLive = 0;
        p->type = ParticleType_Invalid;
        continue;
      }
        p->timeToLive -= time;

          if (p->type & ParticleType_Line)
          {
            p->_x = p->x;
            p->_y = p->y;
            p->_z = p->z;
          }

          if (p->type & ParticleType_1)
            p->flt_18 = p->flt_18 - (double)(signed int)time_ * 5.0;

          if (p->type & ParticleType_8)
          {
            v7 = (double)(signed int)time_;
            *(float *)&p->x += (double)(rand() % 5 - 2) * v7 / 16.0f;
            *(float *)&p->y += (double)(rand() % 5 - 2) * v7 / 16.0f;
            *(float *)&p->z += (double)(rand() % 5 + 4) * v7 / 16.0f;
          }
          v8 = (double)(signed int)time_ / 128.0f;
          //v9 = (signed int)(time * p->rotation_speed) / 16;

          p->x += v8 * p->flt_10;
          p->y += v8 * p->flt_14;
          p->z += v8 * p->flt_18;

          p->angle += time * p->rotation_speed / 16;
          v22 = 2 * p->timeToLive;
          if (v22 >= 255 )
            v22 = 255;
          //v10 = (double)v22 * 0.0039215689;
          p->uLightColor_bgr = ((uint)floorf(p->b * (v22 / 255.0f) + 0.5) << 16) |
                               ((uint)floorf(p->g * (v22 / 255.0f) + 0.5) << 8) |
                               ((uint)floorf(p->r * (v22 / 255.0f) + 0.5) << 0);
          if ( i < v19 )
            v19 = i;
          if ( i > v20 )
            v20 = i;
  }

  uEndParticle = v20;
  uStartParticle = v19;
}

//----- (0048AE74) --------------------------------------------------------
bool ParticleEngine::ViewProject_TrueIfStillVisible_BLV(unsigned int uParticleID)
{
  Particle *pParticle; // esi@1
  //double v56; // ST28_8@2
  //float v4; // eax@4
  //double v5; // ST34_8@4
  signed __int64 v6; // qtt@4
  //double v7; // st7@4
  //float v8; // ST18_4@4
//  int v9; // ecx@4
  //int v10; // eax@4
  //double v11; // ST44_8@7
  //double v12; // ST4C_8@7
//  double v13; // ST4C_8@7
//  int v14; // ecx@7
  //signed __int64 v15; // qtt@7
//  int v16; // eax@7
//  int v17; // edx@7
//  float v18; // edx@7
//  int v19; // eax@7
//  int v20; // edx@7
//  int v21; // ST50_4@8
//  int v22; // ebx@8
//  int v23; // ecx@10
//  int v24; // edi@10
  //double v25; // ST44_8@12
  //double v26; // ST4C_8@12
//  int v27; // edi@12
//  int v28; // ST40_4@12
//  int v29; // ecx@12
  //signed __int64 v30; // qtt@12
//  int v31; // eax@12
//  int v32; // edx@12
//  float v33; // edx@12
  //int v34; // eax@12
//  int v35; // ecx@12
//  int v36; // ST38_4@13
//  int v37; // ST30_4@15
//  int v38; // eax@16
  //signed __int64 v40; // qtt@18
//  int v41; // eax@18
//  int v42; // ecx@18
//  int v43; // eax@18
//  unsigned __int64 v44; // qax@18
  //double v45; // st7@18
  //int v46; // ecx@18
  //float v47; // ST18_4@18
  //unsigned __int64 v48; // qax@18
  int y_int_; // [sp+10h] [bp-40h]@2
//  int a2; // [sp+18h] [bp-38h]@10
  int x_int; // [sp+20h] [bp-30h]@2
  int z_int_; // [sp+24h] [bp-2Ch]@2
//  int z_int_4; // [sp+28h] [bp-28h]@8
  int z; // [sp+3Ch] [bp-14h]@3
//  double a5; // [sp+40h] [bp-10h]@4
//  int a6; // [sp+48h] [bp-8h]@4
  int y; // [sp+4Ch] [bp-4h]@3

  pParticle = &this->pParticles[uParticleID];
  if (pParticle->type == ParticleType_Invalid)
    return 0;
  //uParticleID = LODWORD(pParticle->x);
  //v56 = *(float *)&uParticleID + 6.7553994e15;
  x_int = floorf(pParticle->x + 0.5f);
  //uParticleID = LODWORD(pParticle->y);
  //y_int_ = *(float *)&uParticleID + 6.7553994e15;
  y_int_ = floorf(pParticle->y + 0.5f);
  //uParticleID = LODWORD(pParticle->z);
  //z_int_ = *(float *)&uParticleID + 6.7553994e15;
  z_int_ = floorf(pParticle->z + 0.5f);
  /*if ( !pRenderer->pRenderD3D )
  {
    if (pIndoorCameraD3D->sRotationX)
    {
      if (pParticle->type & ParticleType_Line)
      {
        //v11 = pParticle->_x + 6.7553994e15;
        int _uParticleID = (int)(floorf(pParticle->_x + 0.5f) - pBLVRenderParams->vPartyPos.x) << 16;
        //v12 = pParticle->_y + 6.7553994e15;
        y = (int)(floorf(pParticle->_y + 0.5f) - pBLVRenderParams->vPartyPos.y) << 16;
        z = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
        HIDWORD(a5) = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                    - z;
        a6 = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
        //v13 = pParticle->_z + 6.7553994e15;
        _uParticleID = (int)(floorf(pParticle->_z + 0.5f) - pBLVRenderParams->vPartyPos.z) << 16;
        z = ((unsigned __int64)(SHIDWORD(a5) * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
          - ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
        v14 = z;
        HIDWORD(v13) = (unsigned __int64)(SHIDWORD(a5) * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
        HIDWORD(a5) = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16;
        //LODWORD(v15) = pBLVRenderParams->field_40 << 16;
        //HIDWORD(v15) = pBLVRenderParams->field_40 >> 16;
        //v16 = v15 / z;
        v16 = fixpoint_div(pBLVRenderParams->field_40, z);
        v17 = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
        pParticle->_screenspace_scale = v16;
        _uParticleID = (unsigned __int64)(v16 * (signed __int64)(a6 + v17)) >> 16;
        LODWORD(v18) = pBLVRenderParams->uViewportCenterX
                     - ((signed int)((unsigned __int64)(v16 * (signed __int64)(a6 + v17)) >> 16) >> 16);
        v19 = pParticle->_screenspace_scale;
        pParticle->uScreenSpaceZ = v18;
        _uParticleID = (unsigned __int64)(v19 * (signed __int64)(HIDWORD(v13) + HIDWORD(a5))) >> 16;
        v20 = pBLVRenderParams->uViewportCenterY
            - ((signed int)((unsigned __int64)(v19 * (signed __int64)(HIDWORD(v13) + HIDWORD(a5))) >> 16) >> 16);
        pParticle->sZValue2 = v14;
        pParticle->uScreenSpaceW = v20;
      }
      int _uParticleID = (x_int - pBLVRenderParams->vPartyPos.x) << 16;
      y = (y_int_ - pBLVRenderParams->vPartyPos.y) << 16;
      HIDWORD(a5) = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                  - ((unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
      a6 = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
      z_int_4 = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
      _uParticleID = (z_int_ - pBLVRenderParams->vPartyPos.z) << 16;
      v21 = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
      v22 = ((unsigned __int64)(SHIDWORD(a5) * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - v21;
      z = ((unsigned __int64)(SHIDWORD(a5) * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - v21;
      if ( v22 < (signed int)0x40000u || v22 > (signed int)0x1F400000u )
        return 0;
      v23 = a6 + z_int_4;
      a2 = a6 + z_int_4;
      v24 = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
          + ((unsigned __int64)(SHIDWORD(a5) * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    }
    else
    {
      if (pParticle->type & ParticleType_Line)
      {
        //v25 = pParticle->_x + 6.7553994e15;
        int _uParticleID = ((int)floorf(pParticle->_x + 0.5f) - pBLVRenderParams->vPartyPos.x) << 16;
        //v26 = pParticle->_y + 6.7553994e15;
        y = ((int)floorf(pParticle->_y + 0.5f) - pBLVRenderParams->vPartyPos.y) << 16;
        auto _hiword_v25 = (__int64)(y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
        v27 = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - _hiword_v25;
        z = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - _hiword_v25;
        v28 = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
        //a5 = pParticle->_z + 6.7553994e15;
        v29 = ((int)floorf(pParticle->_z + 0.5f) - pBLVRenderParams->vPartyPos.z) << 16;
        //LODWORD(v30) = pBLVRenderParams->field_40 << 16;
        //HIDWORD(v30) = pBLVRenderParams->field_40 >> 16;
        //v31 = v30 / z;
        v31 = fixpoint_div(pBLVRenderParams->field_40, z);
        v32 = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
        pParticle->_screenspace_scale = v31;
        _uParticleID = (unsigned __int64)(v31 * (signed __int64)(v28 + v32)) >> 16;
        LODWORD(v33) = pBLVRenderParams->uViewportCenterX - ((signed int)((unsigned __int64)(v31 * (signed __int64)(v28 + v32)) >> 16) >> 16);
        //v34 = pParticle->_screenspace_scale;
        pParticle->uScreenSpaceZ = v33;
        v35 = pBLVRenderParams->uViewportCenterY - ((signed int)((unsigned __int64)(pParticle->_screenspace_scale * (signed __int64)v29) >> 16) >> 16);
        pParticle->sZValue2 = v27;
        pParticle->uScreenSpaceW = v35;
      }
      int _uParticleID = (x_int - pBLVRenderParams->vPartyPos.x) << 16;
      y = (y_int_ - pBLVRenderParams->vPartyPos.y) << 16;
      v36 = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
      v22 = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - v36;
      z = ((unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - v36;
      if ( v22 < 262144 || v22 > 524288000 )
        return 0;
      v37 = (unsigned __int64)((signed int)_uParticleID * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
      _uParticleID = (unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
      v23 = v37 + ((unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
      a2 = v37 + ((unsigned __int64)(y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
      v24 = (z_int_ - pBLVRenderParams->vPartyPos.z) << 16;
    }
    int _uParticleID = abs(v23);
    v38 = abs(v22);
    if ( v38 >= (signed int)_uParticleID )
    {
      //LODWORD(v40) = pBLVRenderParams->field_40 << 16;
      //HIDWORD(v40) = pBLVRenderParams->field_40 >> 16;
      v41 = fixpoint_div(pBLVRenderParams->field_40, z);
      pParticle->_screenspace_scale = v41;
      _uParticleID = (unsigned __int64)(v41 * (signed __int64)a2) >> 16;
      v42 = pBLVRenderParams->uViewportCenterX - ((signed int)((unsigned __int64)(v41 * (signed __int64)a2) >> 16) >> 16);
      v43 = pParticle->_screenspace_scale;
      pParticle->uScreenSpaceX = v42;
      v44 = v43 * (signed __int64)v24;
      //uParticleID = v44 >> 16;
      LODWORD(v44) = (signed int)(v44 >> 16) >> 16;
      pParticle->uScreenSpaceY = pBLVRenderParams->uViewportCenterY - v44;
      pParticle->_screenspace_scale = fixpoint_mul(fixpoint_from_float(pParticle->flt_28), pParticle->_screenspace_scale);
      pParticle->sZValue = z;
      return true;
    }
    return false;
  }*/

  int x;
  if ( !pIndoorCameraD3D->ApplyViewTransform_TrueIfStillVisible_BLV(
          x_int,
          y_int_,
          z_int_,
          &x,
          &y,
          &z,
          1) )
    return false;
  pIndoorCameraD3D->Project(x, y, z, &pParticle->uScreenSpaceX, &pParticle->uScreenSpaceY);
  pParticle->flt_5C = pIndoorCameraD3D->fov_x;
  //v4 = pParticle->flt_5C;
  pParticle->flt_60 = pIndoorCameraD3D->fov_y;
  //v5 = v4 + 6.7553994e15;
  LODWORD(v6) = 0;
  HIDWORD(v6) = floorf(pParticle->flt_5C + 0.5f);
  //v7 = pParticle->flt_28;
  //pParticle->_screenspace_scale = v6 / x;
  //v8 = v7;
  pParticle->_screenspace_scale = fixpoint_mul(fixpoint_from_float(pParticle->flt_28), v6 / x);
  pParticle->sZValue = x;
  return true;
}




//----- (0048B5B3) --------------------------------------------------------
bool ParticleEngine::ViewProject_TrueIfStillVisible_ODM(unsigned int uID)
{
  int v3; // ebx@1
  int v4; // edi@1
  int v5; // ecx@1
  int v11; // ST44_4@4
  signed __int64 v13; // qtt@4
  int v16; // edi@6
  int v17; // eax@6
  signed __int64 v22; // qtt@8
  int v26; // edx@9
  int v28; // ebx@12
  signed __int64 v29; // qtt@13
  int v40; // [sp+14h] [bp-3Ch]@12
  int v44; // [sp+2Ch] [bp-24h]@1
  int v45; // [sp+40h] [bp-10h]@5
  int X_4; // [sp+48h] [bp-8h]@5

  v3 = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX);
  v44 = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX);
  v4 = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY);
  v5 = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY);

  if (pParticles[uID].type == ParticleType_Invalid)
    return false;

  if ( v3 )
  {
    if (pParticles[uID].type & ParticleType_Line)
    {
      v11 = fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v4)
          + fixpoint_sub_unknown(pParticles[uID].y - pIndoorCameraD3D->vPartyPos.y, v5);
      long long _hidword_v12 = fixpoint_mul(v11, v3) + fixpoint_sub_unknown(pParticles[uID].z - pIndoorCameraD3D->vPartyPos.z, v44);
      LODWORD(v13) = 0;
      HIDWORD(v13) = SLOWORD(pODMRenderParams->int_fov_rad);
      pParticles[uID]._screenspace_scale = v13 / _hidword_v12;
      pParticles[uID].uScreenSpaceX = pViewport->uScreenCenterX
                                    - ((signed int)fixpoint_mul(pParticles[uID]._screenspace_scale, (fixpoint_sub_unknown(pParticles[uID].y
                                    - pIndoorCameraD3D->vPartyPos.y, v4)
                                    - fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v5))) >> 16);
      pParticles[uID].uScreenSpaceY = pViewport->uScreenCenterY
                                    - ((signed int)fixpoint_mul(pParticles[uID]._screenspace_scale, (fixpoint_sub_unknown(pParticles[uID].z
                                    - pIndoorCameraD3D->vPartyPos.z, v3)
                                    - fixpoint_mul(v11, v44))) >> 16);
      pParticles[uID].sZValue = _hidword_v12;
    }
    v45 = fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v4) + fixpoint_sub_unknown(pParticles[uID].y
                                                 - pIndoorCameraD3D->vPartyPos.y, v5);
    X_4 = fixpoint_sub_unknown(pParticles[uID].z - pIndoorCameraD3D->vPartyPos.z, v44) + fixpoint_mul(v45, v3);
    if ( X_4 < 0x40000 )
      return 0;
    v16 = fixpoint_sub_unknown(pParticles[uID].y - pIndoorCameraD3D->vPartyPos.y, v4)
        - fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v5);
    v17 = fixpoint_sub_unknown(pParticles[uID].z - pIndoorCameraD3D->vPartyPos.z, v3) - fixpoint_mul(v45, v44);
  }
  else
  {
    if (pParticles[uID].type & ParticleType_Line)
    {
      LODWORD(v22) = 0;
      HIDWORD(v22) = SLOWORD(pODMRenderParams->int_fov_rad);
      long long _var_123 = fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v4)
                         + fixpoint_sub_unknown(pParticles[uID].y - pIndoorCameraD3D->vPartyPos.y, v5);
      pParticles[uID]._screenspace_scale = v22 / _var_123;
      pParticles[uID].uScreenSpaceX = pViewport->uScreenCenterX
                        - ((signed int)fixpoint_mul(pParticles[uID]._screenspace_scale, (fixpoint_sub_unknown(pParticles[uID].y
                        - pIndoorCameraD3D->vPartyPos.y, v4)
                        - fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v5))) >> 16);
      pParticles[uID].uScreenSpaceY = pViewport->uScreenCenterY - (fixpoint_sub_unknown(pParticles[uID].z, pParticles[uID]._screenspace_scale) >> 16);
      pParticles[uID].sZValue = _var_123;
    }
    v26 = fixpoint_sub_unknown(pParticles[uID].y - pIndoorCameraD3D->vPartyPos.y, v5);
    X_4 = v26 + fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v4);
    if ( X_4 < 0x40000 || X_4 > (pODMRenderParams->uPickDepth - 1000) << 16 )
      return 0;
    v17 = pParticles[uID].z;
    v16 = fixpoint_sub_unknown(pParticles[uID].y - pIndoorCameraD3D->vPartyPos.y, v4)
        - fixpoint_sub_unknown(pParticles[uID].x - pIndoorCameraD3D->vPartyPos.x, v5);
  }
  v40 = v17;
  v28 = abs(v16);
  if ( abs(X_4) >= v28 )
  {
    LODWORD(v29) = 0;
    HIDWORD(v29) = SLOWORD(pODMRenderParams->int_fov_rad);
    pParticles[uID]._screenspace_scale = v29 / X_4;
    pParticles[uID].uScreenSpaceX = pViewport->uScreenCenterX - ((signed int)fixpoint_mul(pParticles[uID]._screenspace_scale, v16) >> 16);
    pParticles[uID].uScreenSpaceY = pViewport->uScreenCenterY - ((signed int)fixpoint_mul(pParticles[uID]._screenspace_scale, v40) >> 16);
    pParticles[uID]._screenspace_scale = fixpoint_mul(fixpoint_from_float(pParticles[uID].flt_28), pParticles[uID]._screenspace_scale);
    pParticles[uID].sZValue = X_4;
    if ( pParticles[uID].uScreenSpaceX >= (signed int)pViewport->uViewportTL_X
      && pParticles[uID].uScreenSpaceX < (signed int)pViewport->uViewportBR_X
      && pParticles[uID].uScreenSpaceY >= (signed int)pViewport->uViewportTL_Y
      && pParticles[uID].uScreenSpaceY < (signed int)pViewport->uViewportBR_Y )
      return true;
  }
  return false;
}

//----- (0048BBA6) --------------------------------------------------------
void ParticleEngine::DrawParticles_BLV()
{
//  int v11; // eax@18
//  int v12; // ecx@20
//  int v13; // edx@20
  //Particle *v14; // eax@28
  RenderBillboardTransform_local0 v15; // [sp+Ch] [bp-58h]@1

  v15.sParentBillboardID = -1;

  for (uint i = uStartParticle; i < uEndParticle; ++i)
  {
    Particle* p = &pParticles[i];

    if (p->type == ParticleType_Invalid)
      continue;

    if (!ViewProject_TrueIfStillVisible_BLV(i))
      continue;

    if (p->uScreenSpaceX >= pBLVRenderParams->uViewportX &&
        p->uScreenSpaceX < pBLVRenderParams->uViewportZ &&
        p->uScreenSpaceY >= pBLVRenderParams->uViewportY &&
        p->uScreenSpaceY < pBLVRenderParams->uViewportW)
    {
      /*if (!pRenderer->pRenderD3D)
      {
        __debugbreak();
                    v11 = 13 * p->_screenspace_scale >> 16;
                     if ( v11 > 30 )
                       v11 = 30;
                    v12 = p->uScreenSpaceY - v11;
                    v13 = p->uScreenSpaceX - (v11 >> 1);
                     if ( v13 + v11 < (signed int)pViewport->uViewportTL_X
                       || v13 >= (signed int)pViewport->uViewportBR_X
                       || v12 + v11 < (signed int)pViewport->uViewportTL_Y
                       || v12 >= (signed int)pViewport->uViewportBR_Y )
                     {
                      ;
                     }
                     else
                     {
                       pRenderer->MakeParticleBillboardAndPush_BLV_Software(v13, v12, p->sZValue, p->uLightColor_bgr, v11);
                     }
      }
      else*/

        if (p->type & ParticleType_Diffuse)
        {
          //v14 = &pParticles[i];
          v15._screenspace_x_scaler_packedfloat = p->_screenspace_scale / 4;
          v15._screenspace_y_scaler_packedfloat = p->_screenspace_scale / 4;
          v15.uScreenSpaceX = p->uScreenSpaceX;
          v15.uScreenSpaceY = p->uScreenSpaceY;
          v15.sZValue = p->sZValue;
          pRenderer->MakeParticleBillboardAndPush_BLV(&v15, 0, p->uLightColor_bgr, p->angle);
          return;
        }
        if (p->type & ParticleType_Line)
        {
          if (pLines.uNumLines < 100)
          {
            pLines.pLineVertices[2 * pLines.uNumLines].pos.x = p->uScreenSpaceX;
            pLines.pLineVertices[2 * pLines.uNumLines].pos.y = p->uScreenSpaceY;
            pLines.pLineVertices[2 * pLines.uNumLines].pos.z = 1.0 - 1.0 / ((short)p->sZValue * 0.061758894);
            pLines.pLineVertices[2 * pLines.uNumLines].rhw = 1.0;
            pLines.pLineVertices[2 * pLines.uNumLines].diffuse = p->uLightColor_bgr;
            pLines.pLineVertices[2 * pLines.uNumLines].specular = 0;
            pLines.pLineVertices[2 * pLines.uNumLines].texcoord.x = 0.0;
            pLines.pLineVertices[2 * pLines.uNumLines].texcoord.y = 0.0;

            pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.x = p->uScreenSpaceZ;
            pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.y = p->uScreenSpaceW;
            pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.z = 1.0 - 1.0 / ((short)p->sZValue2 * 0.061758894);
            pLines.pLineVertices[2 * pLines.uNumLines + 1].rhw = 1.0;
            pLines.pLineVertices[2 * pLines.uNumLines + 1].diffuse = p->uLightColor_bgr;
            pLines.pLineVertices[2 * pLines.uNumLines + 1].specular = 0;
            pLines.pLineVertices[2 * pLines.uNumLines + 1].texcoord.x = 0.0;
            pLines.pLineVertices[2 * pLines.uNumLines++ + 1].texcoord.y = 0.0;
          }
        }
        if (p->type & ParticleType_Bitmap)
        {
          v15._screenspace_x_scaler_packedfloat = p->_screenspace_scale;
          v15._screenspace_y_scaler_packedfloat = p->_screenspace_scale;
          v15.uScreenSpaceX = p->uScreenSpaceX;
          v15.uScreenSpaceY = p->uScreenSpaceY;
          v15.sZValue = p->sZValue;
          pRenderer->MakeParticleBillboardAndPush_BLV(&v15, pBitmaps_LOD->pHardwareTextures[p->uTextureID], p->uLightColor_bgr, p->angle);
        }
        if (p->type & ParticleType_Sprite)
        {
          v15._screenspace_x_scaler_packedfloat = p->_screenspace_scale;
          v15._screenspace_y_scaler_packedfloat = p->_screenspace_scale;
          v15.uScreenSpaceX = p->uScreenSpaceX;
          v15.uScreenSpaceY = p->uScreenSpaceY;
          v15.sZValue = p->sZValue;
          pRenderer->MakeParticleBillboardAndPush_BLV(&v15, pSprites_LOD->pHardwareSprites[p->uTextureID].pTexture, p->uLightColor_bgr, p->angle);
        }
    }
  }
}

//----- (0048BEEF) --------------------------------------------------------
void ParticleEngine::DrawParticles_ODM()
{
  ParticleEngine *pParticleEngine; // esi@1
  //int pParticleNum; // eax@1
//  unsigned __int8 v3; // zf@1
//  char v4; // sf@1
//  unsigned __int8 v5; // of@1
  //char *v7; // edi@2
  //int v8; // eax@6
  //signed int pNumLines; // eax@8
//  int v10; // eax@14
//  int v11; // ecx@16
//  int v12; // edx@16
  //Particle *pParticle; // eax@24
  RenderBillboardTransform_local0 pBillboard; // [sp+Ch] [bp-58h]@1
  //int v15; // [sp+5Ch] [bp-8h]@9
//  int v16; // [sp+60h] [bp-4h]@1

  pBillboard.sParentBillboardID = -1;
  pParticleEngine = this;
  //v2 = this->uStartParticle;
  //v5 = v2 > this->uEndParticle;//  v5 = __OFSUB__(v2, this->uEndParticle);
  //v3 = v2 == this->uEndParticle;
  //v4 = v2 - this->uEndParticle < 0;
  //v16 = this->uStartParticle;
  for (uint i = uStartParticle; i <= uEndParticle; ++i)
  {
    Particle* particle = &pParticles[i];
    if (particle->type == ParticleType_Invalid || !ViewProject_TrueIfStillVisible_ODM(i))
      continue;

        /*if ( !pRenderer->pRenderD3D )
        {
          __debugbreak();
          v10 = 13 * particle->_screenspace_scale >> 16;
          if ( v10 > 30 )
            v10 = 30;
          v11 = particle->uScreenSpaceX - (v10 >> 1);
          v12 = particle->uScreenSpaceY - v10;
          if ( v11 + v10 < pViewport->uViewportTL_X
            || v11 >= pViewport->uViewportBR_X
            || particle->uScreenSpaceY < pViewport->uViewportTL_Y
            || v12 >= (signed int)pViewport->uViewportBR_Y )
          {
            ;
          }
          else
          {
            pRenderer->MakeParticleBillboardAndPush_BLV_Software(v11, v12, particle->sZValue, particle->uLightColor_bgr, v10);
          }
        }
        else*/

          //v8 = *(_DWORD *)(v7 - 82);
          if (particle->type & ParticleType_Diffuse)
          {
            pBillboard._screenspace_x_scaler_packedfloat = particle->_screenspace_scale / 4;
            pBillboard._screenspace_y_scaler_packedfloat = particle->_screenspace_scale / 4;
            pBillboard.uScreenSpaceX = particle->uScreenSpaceX;
            pBillboard.uScreenSpaceY = particle->uScreenSpaceY;
            pBillboard.sZValue = particle->sZValue;
            pRenderer->MakeParticleBillboardAndPush_ODM(&pBillboard, 0, particle->uLightColor_bgr, particle->angle);
            return;
          }
          if (particle->type & ParticleType_Line)
          {
            if (pLines.uNumLines < 100)
            {
              pLines.pLineVertices[2 * pLines.uNumLines].pos.x = particle->uScreenSpaceX;
              pLines.pLineVertices[2 * pLines.uNumLines].pos.y = particle->uScreenSpaceY;
              pLines.pLineVertices[2 * pLines.uNumLines].pos.z = 1.0 - 1.0 / ((double)particle->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
              pLines.pLineVertices[2 * pLines.uNumLines].rhw = 1.0;
              pLines.pLineVertices[2 * pLines.uNumLines].diffuse = particle->uLightColor_bgr;
              pLines.pLineVertices[2 * pLines.uNumLines].specular = 0;
              pLines.pLineVertices[2 * pLines.uNumLines].texcoord.x = 0.0;
              pLines.pLineVertices[2 * pLines.uNumLines].texcoord.y = 0.0;

              pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.x = particle->uScreenSpaceZ;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.y = particle->uScreenSpaceW;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].pos.z = 1.0 - 1.0 / ((double)particle->zbuffer_depth * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
              pLines.pLineVertices[2 * pLines.uNumLines + 1].rhw = 1.0;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].diffuse = particle->uLightColor_bgr;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].specular = 0;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].texcoord.x = 0.0;
              pLines.pLineVertices[2 * pLines.uNumLines + 1].texcoord.y = 0.0;
              pLines.uNumLines++;
            }
          }
          if (particle->type & ParticleType_Bitmap)
          {
            pBillboard._screenspace_x_scaler_packedfloat = particle->_screenspace_scale;
            pBillboard._screenspace_y_scaler_packedfloat = particle->_screenspace_scale;
            pBillboard.uScreenSpaceX = particle->uScreenSpaceX;
            pBillboard.uScreenSpaceY = particle->uScreenSpaceY;
            pBillboard.sZValue = particle->sZValue;
            pRenderer->MakeParticleBillboardAndPush_ODM(&pBillboard, pBitmaps_LOD->pHardwareTextures[particle->uTextureID], particle->uLightColor_bgr, particle->angle);
          }
          if (particle->type & ParticleType_Sprite)
          {
            pBillboard._screenspace_x_scaler_packedfloat = particle->_screenspace_scale;
            pBillboard._screenspace_y_scaler_packedfloat = particle->_screenspace_scale;
            pBillboard.uScreenSpaceX = particle->uScreenSpaceX;
            pBillboard.uScreenSpaceY = particle->uScreenSpaceY;
            pBillboard.sZValue = particle->sZValue;
            pRenderer->MakeParticleBillboardAndPush_ODM(&pBillboard, pSprites_LOD->pHardwareSprites[particle->uTextureID].pTexture, particle->uLightColor_bgr, particle->angle);
          }   
  }
}