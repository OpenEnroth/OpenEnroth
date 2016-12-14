#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Time.h"

#include "stru6.h"

#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/LOD.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Random.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/stru160.h"
#include "Engine/OurMath.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Indoor.h"




//----- (004A7063) --------------------------------------------------------
unsigned int ModulateColor(unsigned int diffuse, float multiplier)
{
  float alpha = multiplier * ((diffuse >> 24) & 0xFF);
  int   a = (int)floorf(alpha + 0.5f);
  a = max(0, min(255, a));

  float red = multiplier * ((diffuse >> 16) & 0xFF);
  int   r = (int)floorf(red + 0.5f);
  r = max(0, min(255, r));
  
  float green = multiplier * ((diffuse >> 8) & 0xFF);
  int   g = (int)floorf(green + 0.5f);
  g = max(0, min(255, g));
  
  float blue = multiplier * ((diffuse >> 0) & 0xFF);
  int   b = (int)floorf(blue + 0.5f);
  b = max(0, min(255, b));

  return (a << 24) | (r << 16) | (g << 8) | b;
}


//----- (0042620A) --------------------------------------------------------
bool sr_42620A(RenderVertexSoft *p)
{
//  __int16 v1; // fps@1
  unsigned __int8 v2; // c0@2
  char v3; // c2@2
  unsigned __int8 v4; // c3@2
//  bool result; // eax@2
  unsigned __int8 v6; // c0@4
  char v7; // c2@4
  unsigned __int8 v8; // c3@4
  unsigned __int8 v9; // c0@6
  char v10; // c2@6
  unsigned __int8 v11; // c3@6
  double v12; // st6@7
  float v13; // ST04_4@7
  float v14; // ST00_4@7
  double v15; // st7@7
  double v16; // st6@8
  float v17; // ST04_4@8
  float v18; // ST00_4@8
  double v19; // st7@8

  //UNDEF(v1);
  if ( p->vWorldViewPosition.x < 300.0
    || (v2 = 300.0 < p[1].vWorldViewPosition.x,
        v3 = 0,
        v4 = 300.0 == p[1].vWorldViewPosition.x,
        //BYTE1(result) = HIBYTE(v1),
        !(v2 | v4)) )
  {
    if ( p->vWorldViewPosition.x < 300.0 )
    {
      v6 = 300.0 < p[1].vWorldViewPosition.x;
      v7 = 0;
      v8 = 300.0 == p[1].vWorldViewPosition.x;
      //BYTE1(result) = HIBYTE(v1);
      if ( !(v6 | v8) )
      {
        //LOBYTE(result) = 0;
        return false;
      }
    }
    v9 = 300.0 < p->vWorldViewPosition.x;
    v10 = 0;
    v11 = 300.0 == p->vWorldViewPosition.x;
    //BYTE1(result) = HIBYTE(v1);
    if ( v9 | v11 )
    {
      v16 = 1.0 / (p->vWorldViewPosition.x - p[1].vWorldViewPosition.x);
      v17 = (p->vWorldViewPosition.y - p[1].vWorldViewPosition.y) * v16;
      v18 = (p->vWorldViewPosition.z - p[1].vWorldViewPosition.z) * v16;
      v19 = 300.0 - p[1].vWorldViewPosition.x;
      p[1].vWorldViewPosition.x = v19 + p[1].vWorldViewPosition.x;
      p[1].vWorldViewPosition.y = v17 * v19 + p[1].vWorldViewPosition.y;
      p[1].vWorldViewPosition.z = v19 * v18 + p[1].vWorldViewPosition.z;
    }
    else
    {
      v12 = 1.0 / (p[1].vWorldViewPosition.x - p->vWorldViewPosition.x);
      v13 = (p[1].vWorldViewPosition.y - p->vWorldViewPosition.y) * v12;
      v14 = (p[1].vWorldViewPosition.z - p->vWorldViewPosition.z) * v12;
      v15 = 300.0 - p->vWorldViewPosition.x;
      p->vWorldViewPosition.x = v15 + p->vWorldViewPosition.x;
      p->vWorldViewPosition.y = v13 * v15 + p->vWorldViewPosition.y;
      p->vWorldViewPosition.z = v15 * v14 + p->vWorldViewPosition.z;
    }
  }
  //LOBYTE(result) = 1;
  return true;
}



//----- (004775C4) --------------------------------------------------------
stru6_stru1_indoor_sw_billboard::~stru6_stru1_indoor_sw_billboard()
{
    delete [] pArray1;
    pArray1 = nullptr;

    delete [] pArray2;
    pArray2 = nullptr;
}


//----- (00478211) --------------------------------------------------------
void stru6_stru1_indoor_sw_billboard::Initialize(int a2)
{
  uNumVec4sInArray1 = 66;
  pArray1 = new stru16x[66];

  uNumVec3sInArray2 = 128;
  pArray2 = new stru160[128];

  for (uint i = 0; i < uNumVec4sInArray1; ++i)
  {
    pArray1[i].field_0 = array_4EB8B8[i].field_0;
    pArray1[i].field_4 = array_4EB8B8[i].field_4;
    pArray1[i].field_8 = array_4EB8B8[i].field_8;
    pArray1[i].field_C = a2;
  }
  memcpy(pArray2, array_4EBBD0_x.data()/*array_4EBBD0*/, uNumVec3sInArray2 * sizeof(stru160));
}

//----- (0047829F) --------------------------------------------------------
void stru6_stru1_indoor_sw_billboard::_47829F_sphere_particle(float x_offset, float y_offset, float z_offset, float scale, int diffuse)
{
  int v7 = 0;

  for (unsigned int i = 0; i < uNumVec3sInArray2; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
    {
      field_14[j].x = x_offset + scale * *(&pArray1->field_0 + 4 * *(int *)((char *)&pArray2->field_0 + v7));
      field_14[j].y = y_offset + scale * *(&pArray1->field_4 + 4 * *(int *)((char *)&pArray2->field_0 + v7));
      field_14[j].z = z_offset + scale * *(&pArray1->field_8 + 4 * *(int *)((char *)&pArray2->field_0 + v7));
      //int v10 = *(int *)((char *)&pArray2->field_0 + v7);

      field_14[j].diffuse = *((int *)&pArray1[1].field_0 + 4 * (*(int *)((char *)&pArray2->field_0 + v7)));
      v7 += 4;
    }

    uNumVertices = 3;
    if ( sub_477C61() && sub_477F63() )
    {
      if ( sub_47802A() )
        pRenderer->_4A4CC9_AddSomeBillboard(this, diffuse);
    }
  }
}




//----- (004A71FE) --------------------------------------------------------
void stru6::DoAddProjectile(float srcX, float srcY, float srcZ, float dstX, float dstY, float dstZ, unsigned int uTextureID)
{
  //int v8; // eax@1

  //v8 = uNumProjectiles;
  if (uNumProjectiles < 32)
  {
    pProjectiles[uNumProjectiles].srcX = srcX;
    pProjectiles[uNumProjectiles].srcY = srcY;
    pProjectiles[uNumProjectiles].srcZ = srcZ;
    pProjectiles[uNumProjectiles].dstX = dstX;
    pProjectiles[uNumProjectiles].dstY = dstY;
    pProjectiles[uNumProjectiles].dstZ = dstZ;
    pProjectiles[uNumProjectiles++].uTextureID = uTextureID;
  }
}

//----- (004A7298) --------------------------------------------------------
void stru6::DrawProjectiles()
{
  float v10; // ST1C_4@8
  float v11; // ST0C_4@8
  IDirect3DTexture2 *v12; // [sp+20h] [bp-78h]@6
  RenderVertexSoft v[2]; // [sp+30h] [bp-68h]@1
  
  for (uint i = 0; i < uNumProjectiles; ++i)
  {
    ProjectileAnim* p = &pProjectiles[i];

    v[0].vWorldPosition.x = p->srcX;
    v[0].vWorldPosition.y = p->srcY;
    v[0].vWorldPosition.z = p->srcZ;
    v[1].vWorldPosition.x = p->dstX;
    v[1].vWorldPosition.y = p->dstY;
    v[1].vWorldPosition.z = p->dstZ;
    pIndoorCameraD3D->ViewTransform(v, 2);

    sr_42620A(v);

    pIndoorCameraD3D->Project(v, 2, 0);

    if (p->uTextureID != -1)
      v12 = pBitmaps_LOD->pHardwareTextures[p->uTextureID];
    else
      v12 = 0;

    v10 = pIndoorCameraD3D->fov_x / v[1].vWorldViewPosition.x * 20.0;
    v11 = pIndoorCameraD3D->fov_x / v[0].vWorldViewPosition.x * 20.0;
    pRenderer->DrawProjectile(
        v[0].vWorldViewProjX,
        v[0].vWorldViewProjY,
        v[0].vWorldViewPosition.x,
         v11,
        v[1].vWorldViewProjX,
        v[1].vWorldViewProjY,
        v[1].vWorldViewPosition.x,
         v10,
         v12);
   }
}

//----- (004A73AA) --------------------------------------------------------
void stru6::_4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(SpriteObject *a2, unsigned int uDiffuse, unsigned int uTextureID)
{
  stru6 *v4; // edi@1
  SpriteObject *v5; // esi@1
  int v6; // eax@1
  stru6_stru2 *v7; // eax@2
  double v8; // st7@2
  double v9; // st6@2
  double v10; // st7@3
  Particle_sw local_0; // [sp+8h] [bp-68h]@1
  float x; // [sp+78h] [bp+8h]@2

  v4 = this;
  memset(&local_0, 0, 0x68u);
  v5 = a2;
  v6 = a2->field_54;
  if ( v6 )
  {
    v7 = &v4->array_4[v6 & 0x1F];
    x = ((double)a2->vPosition.x - v7->flt_0_x) * 0.5 + v7->flt_0_x;
    v8 = ((double)v5->vPosition.y - v7->flt_4_y) * 0.5 + v7->flt_4_y;
    v9 = ((double)v5->vPosition.z - v7->flt_8_z) * 0.5 + v7->flt_8_z;
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
    local_0.uDiffuse = uDiffuse;
    local_0.x = x + 4.0;
    local_0.y = v8;
    local_0.z = v9;
    local_0.r = 0.0;
    local_0.g = 0.0;
    local_0.b = 0.0;
    local_0.timeToLive = (rand() & 0x40) + 96;
    local_0.uTextureID = uTextureID;
    local_0.flt_28 = 1.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    local_0.x = x - 4.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    local_0.x = (double)v5->vPosition.x + 4.0;
    local_0.y = (double)v5->vPosition.y;
    local_0.z = (double)v5->vPosition.z;
    pEngine->pParticleEngine->AddParticle(&local_0);
    local_0.x = (double)v5->vPosition.x - 4.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    v4->array_4[v5->field_54 & 0x1F].flt_0_x = (double)v5->vPosition.x;
    v4->array_4[v5->field_54 & 0x1F].flt_4_y = (double)v5->vPosition.y;
    v4->array_4[v5->field_54 & 0x1F].flt_8_z = (double)v5->vPosition.z;
  }
  else
  {
    a2->field_54 = v4->field_0++;
    v4->array_4[a2->field_54 & 0x1F].flt_0_x = (double)a2->vPosition.x;
    v4->array_4[a2->field_54 & 0x1F].flt_4_y = (double)a2->vPosition.y;
    v4->array_4[a2->field_54 & 0x1F].flt_8_z = (double)a2->vPosition.z;
    v10 = (double)a2->vPosition.x;
    local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
    local_0.uDiffuse = uDiffuse;
    local_0.x = v10 + 4.0;
    local_0.y = (double)a2->vPosition.y;
    local_0.z = (double)a2->vPosition.z;
    local_0.r = 0.0;
    local_0.g = 0.0;
    local_0.b = 0.0;
    local_0.flt_28 = 1.0;
    local_0.timeToLive = (rand() & 0x7F) + 128;
    local_0.uTextureID = uTextureID;
    pEngine->pParticleEngine->AddParticle(&local_0);
    local_0.x = (double)a2->vPosition.x - 4.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
  }
}

//----- (004A75CC) --------------------------------------------------------
void stru6::_4A75CC_single_spell_collision_particle(SpriteObject *a1, unsigned int uDiffuse, unsigned int uTextureID)
{
  double v4; // st7@1
  signed int v5; // edi@1
  Particle_sw local_0; // [sp+8h] [bp-68h]@1

  memset(&local_0, 0, 0x68u);
  local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_1;
  local_0.x = (double)a1->vPosition.x;
  local_0.y = (double)a1->vPosition.y;
  v4 = (double)a1->vPosition.z;
  local_0.uDiffuse = uDiffuse;
  local_0.z = v4;
  v5 = 10;
  local_0.timeToLive = (rand() & 0x7F) + 128;
  local_0.uTextureID = uTextureID;
  local_0.flt_28 = 1.0;
  do
  {
    local_0.r = (double)(rand() & 0x1FF) - 255.0;
    local_0.g = (double)(rand() & 0x1FF) - 255.0;
    local_0.b = (double)(rand() & 0x1FF) - 255.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    --v5;
  }
  while ( v5 );
}

//----- (004A7688) --------------------------------------------------------
void stru6::_4A7688_fireball_collision_particle(SpriteObject *a2)
{
  double v3; // st7@1
  double v4; // st7@2
  Particle_sw local_0; // [sp+1Ch] [bp-7Ch]@1

  memset(&local_0, 0, 0x68u);

  v3 = (double)a2->uSpriteFrameID / (double)pObjectList->pObjects[a2->uObjectDescID].uLifetime;
  if ( v3 >= 0.75 )
    v4 = (1.0 - v3) * 4.0;
  else
    v4 = v3 * 1.333333333333333;

  local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_1;
  local_0.uDiffuse = 0xFF3C1E;
  local_0.x = (double)a2->vPosition.x;
  local_0.y = (double)a2->vPosition.y;
  local_0.z = (double)a2->vPosition.z;
  local_0.timeToLive = (rand() & 0x7F) + 128;
  local_0.uTextureID = pBitmaps_LOD->LoadTexture("effpar01", TEXTURE_DEFAULT);
  local_0.flt_28 = 1.0;

  // 10 fireball sparks 
  for (unsigned int i = 0; i < 10; ++i)
  {
    local_0.r = (rand() & 0x1FF) - 255;
    local_0.g = (rand() & 0x1FF) - 255;
    local_0.b = (rand() & 0x1FF) - 255;
    pEngine->pParticleEngine->AddParticle(&local_0);
  }

  pStru1->_47829F_sphere_particle((double)a2->vPosition.x, (double)a2->vPosition.y, (double)a2->vPosition.z, 
                                              (double)floorf(0.5f + (512.0 * v3)), ModulateColor(0xFF3C1E, v4));
}

//----- (004A77FD) --------------------------------------------------------
void stru6::_4A77FD_implosion_particle_d3d(SpriteObject *a1)
{
  double v4; // st7@1
  double v5; // st7@2
  int v7; // eax@4
  float v8; // ST0C_4@4
  float v9; // ST08_4@4
  float v10; // ST04_4@4
  float v11; // ST00_4@4
  float v12; // [sp+28h] [bp-4h]@1

  v4 = (double)a1->uSpriteFrameID / (double)pObjectList->pObjects[a1->uObjectDescID].uLifetime;
  v12 = 512.0 - v4 * 512.0;
  if ( v4 >= 0.75 )
    v5 = v4 * 4.0;
  else
    v5 = v4 * 1.333333333333333;

  v7 = ModulateColor(0x7E7E7E, v5);
  v8 = (double)floorf(0.5f + v12);
  v9 = (double)a1->vPosition.z;
  v10 = (double)a1->vPosition.y;
  v11 = (double)a1->vPosition.x;
  pStru1->_47829F_sphere_particle(v11, v10, v9, v8, v7);
}

//----- (004A78AE) --------------------------------------------------------
void stru6::_4A78AE_sparks_spell(SpriteObject *a1)
{
  ObjectDesc *v2; // esi@1
  unsigned int v3; // eax@1
  double v4; // st7@1
  Particle_sw local_0; // [sp+8h] [bp-68h]@1

  v2 = &pObjectList->pObjects[a1->uObjectDescID];
  memset(&local_0, 0, 0x68u);
  v3 = a1->uSpriteFrameID;
  local_0.x = (double)a1->vPosition.x;
  v4 = (double)a1->vPosition.y;
  local_0.type = ParticleType_Sprite;
  local_0.uDiffuse = 0x7F7F7F;
  local_0.timeToLive = 1;
  local_0.y = v4;
  local_0.z = (double)a1->vPosition.z;
  local_0.r = 0.0;
  local_0.g = 0.0;
  local_0.b = 0.0;
  local_0.uTextureID = pSpriteFrameTable->GetFrame(v2->uSpriteID, v3)->pHwSpriteIDs[0];
  LODWORD(local_0.flt_28) = 0x40000000u;
  pEngine->pParticleEngine->AddParticle(&local_0);
}

//----- (004A7948) --------------------------------------------------------
void stru6::_4A7948_mind_blast_after_effect(SpriteObject *a1)
{
  ObjectDesc *v2; // esi@1
  unsigned int v3; // eax@1
  double v4; // st7@1
  char v5; // al@1
  signed int v6; // edi@1
  Particle_sw Dst; // [sp+8h] [bp-68h]@1

  v2 = &pObjectList->pObjects[a1->uObjectDescID];
  memset(&Dst, 0, 0x68u);
  v3 = a1->uSpriteFrameID;
  Dst.x = (double)a1->vPosition.x;
  v4 = (double)a1->vPosition.y;
  Dst.type = ParticleType_Sprite | ParticleType_Rotating | ParticleType_1;
  Dst.uDiffuse = 0x7F7F7F;
  Dst.y = v4;
  Dst.z = (double)a1->vPosition.z;
  Dst.uTextureID = pSpriteFrameTable->GetFrame(v2->uSpriteID, v3)->pHwSpriteIDs[0];
  v5 = rand();
  v6 = 10;
  Dst.flt_28 = 1.0;
  Dst.timeToLive = (v5 & 0x7F) + 128;
  do
  {
    Dst.r = (double)(rand() & 0x1FF) - 255.0;
    Dst.g = (double)(rand() & 0x1FF) - 255.0;
    Dst.b = (double)(rand() & 0x1FF) - 255.0;
    pEngine->pParticleEngine->AddParticle(&Dst);
    --v6;
  }
  while ( v6 );
}

//----- (004A7A27) --------------------------------------------------------
bool stru6::AddMobileLight(SpriteObject *a1, unsigned int uDiffuse, int uRadius)
{
  return pMobileLightsStack->AddLight(a1->vPosition.x, a1->vPosition.y, a1->vPosition.z, a1->uSectorID, uRadius,
           (uDiffuse & 0x00FF0000) >> 16,
           (uDiffuse & 0x0000FF00) >> 8,
           uDiffuse & 0x000000FF,
           _4E94D3_light_type);
}
// 4E94D3: using guessed type char _4E94D3_light_type;

//----- (004A7A66) --------------------------------------------------------
void stru6::_4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(SpriteObject *a1, unsigned int uDiffuse, unsigned int uTextureID, float a4)
{
  int v5; // eax@1
  char v6; // al@1
  double v7; // st6@1
  double v8; // st6@1
  double v9; // st7@1
  double v10; // st6@1
  Particle_sw local_0; // [sp+0h] [bp-6Ch]@1
  float v12; // [sp+68h] [bp-4h]@1
  float a1a; // [sp+74h] [bp+8h]@1
  float uDiffusea; // [sp+78h] [bp+Ch]@1
  float uTextureIDa; // [sp+7Ch] [bp+10h]@1

  memset(&local_0, 0, 0x68u);
  local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_1;
  local_0.x = (double)a1->vPosition.x;
  v5 = a1->vPosition.z;
  local_0.y = (double)a1->vPosition.y;
  local_0.uDiffuse = uDiffuse;
  local_0.z = (double)(v5 + 32);
  v6 = rand();
  local_0.flt_28 = 1.0;
  v7 = 0.0 * a4;
  local_0.timeToLive = (v6 & 0x7F) + 128;
  local_0.uTextureID = uTextureID;
  a1a = v7;
  local_0.r = v7;
  local_0.g = a4;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
  v8 = 0.70710677 * a4;
  uDiffusea = v8;
  local_0.r = v8;
  local_0.g = v8;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
  local_0.g = a1a;
  local_0.r = a4;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
  local_0.r = uDiffusea;
  local_0.b = a4;
  v9 = -uDiffusea;
  uTextureIDa = v9;
  local_0.g = v9;
  pEngine->pParticleEngine->AddParticle(&local_0);
  v10 = -1.0 * a4;
  local_0.r = a1a;
  v12 = v10;
  local_0.g = v10;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
  local_0.b = a4;
  local_0.r = uTextureIDa;
  local_0.g = uTextureIDa;
  pEngine->pParticleEngine->AddParticle(&local_0);
  local_0.r = v12;
  local_0.g = a1a;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
  local_0.r = uTextureIDa;
  local_0.g = uDiffusea;
  local_0.b = a4;
  pEngine->pParticleEngine->AddParticle(&local_0);
}

//----- (004A7C07) --------------------------------------------------------
void stru6::_4A7C07_stun_spell_fx(SpriteObject *a2)
{
  stru6 *v2; // edi@1
  SpriteObject *v3; // esi@1
  int v4; // eax@1
  ObjectDesc *v5; // ebx@1
  stru6_stru2 *v6; // eax@2
  double v7; // st6@2
  double v8; // st5@2
  double v9; // st4@2
  char v10; // al@2
  double v11; // st7@2
  double v12; // st7@3
  Particle_sw local_0; // [sp+Ch] [bp-68h]@1
  float a2a; // [sp+7Ch] [bp+8h]@2

  v2 = this;
  memset(&local_0, 0, 0x68u);
  v3 = a2;
  v4 = a2->field_54;
  v5 = &pObjectList->pObjects[a2->uObjectDescID];
  if ( v4 )
  {
    v6 = &v2->array_4[v4 & 0x1F];
    v7 = ((double)a2->vPosition.x - v6->flt_0_x) * 0.5 + v6->flt_0_x;
    v8 = ((double)a2->vPosition.y - v6->flt_4_y) * 0.5 + v6->flt_4_y;
    v9 = ((double)a2->vPosition.z - v6->flt_8_z) * 0.5 + v6->flt_8_z;
    local_0.type = ParticleType_Sprite;
    local_0.uDiffuse = 0xFFFFFF;
    a2a = v9;
    local_0.x = v7;
    local_0.z = a2a;
    local_0.y = v8;
    local_0.r = 0.0;
    local_0.g = 0.0;
    local_0.b = 0.0;
    v10 = rand();
    LODWORD(local_0.flt_28) = 0x40400000u;
    local_0.timeToLive = (v10 & 0x3F) + 64;
    local_0.uTextureID = pSpriteFrameTable->GetFrame(v5->uSpriteID, v3->uSpriteFrameID)->pHwSpriteIDs[0];
    pEngine->pParticleEngine->AddParticle(&local_0);
    v11 = (double)v3->vPosition.x;
    LODWORD(local_0.flt_28) = 0x40800000u;
    local_0.x = v11;
    local_0.y = (double)v3->vPosition.y;
    local_0.z = (double)v3->vPosition.z;
    local_0.timeToLive = (rand() & 0x3F) + 64;
    pEngine->pParticleEngine->AddParticle(&local_0);
    v2->array_4[v3->field_54 & 0x1F].flt_0_x = (double)v3->vPosition.x;
    v2->array_4[v3->field_54 & 0x1F].flt_4_y = (double)v3->vPosition.y;
    v2->array_4[v3->field_54 & 0x1F].flt_8_z = (double)v3->vPosition.z;
  }
  else
  {
    a2->field_54 = v2->field_0++;
    v2->array_4[a2->field_54 & 0x1F].flt_0_x = (double)a2->vPosition.x;
    v2->array_4[a2->field_54 & 0x1F].flt_4_y = (double)a2->vPosition.y;
    v2->array_4[a2->field_54 & 0x1F].flt_8_z = (double)a2->vPosition.z;
    v12 = (double)a2->vPosition.x;
    local_0.type = ParticleType_Sprite;
    local_0.uDiffuse = 0xFFFFFF;
    LODWORD(local_0.flt_28) = 0x40000000u;
    local_0.x = v12;
    local_0.y = (double)a2->vPosition.y;
    local_0.z = (double)a2->vPosition.z;
    local_0.r = 0.0;
    local_0.g = 0.0;
    local_0.b = 0.0;
    local_0.timeToLive = (rand() & 0x3F) + 64;
    local_0.uTextureID = pSpriteFrameTable->GetFrame(v5->uSpriteID, a2->uSpriteFrameID)->pHwSpriteIDs[0];
    pEngine->pParticleEngine->AddParticle(&local_0);
  }
}

//----- (004A7E05) --------------------------------------------------------
void stru6::AddProjectile(SpriteObject *a2, int a3, unsigned int uTextureID)
{
  if (a2->field_54)
  {
    DoAddProjectile(array_4[a2->field_54 & 0x1F].flt_0_x,
                    array_4[a2->field_54 & 0x1F].flt_4_y,
                    array_4[a2->field_54 & 0x1F].flt_8_z,
                    a2->vPosition.x,
                    a2->vPosition.y,
                    a2->vPosition.z,
                    uTextureID);
  }
  else
  {
    a2->field_54 = field_0++;
    array_4[a2->field_54 & 0x1F].flt_0_x = (double)a2->vPosition.x;
    array_4[a2->field_54 & 0x1F].flt_4_y = (double)a2->vPosition.y;
    array_4[a2->field_54 & 0x1F].flt_8_z = (double)a2->vPosition.z;
  }
}

//----- (004A7E89) --------------------------------------------------------
void stru6::_4A7E89_sparkles_on_actor_after_it_casts_buff(Actor *pActor, unsigned int uDiffuse)
{
  Actor *v3; // edi@1
  int v4; // ebx@3
  //int result; // eax@5
  Particle_sw Dst; // [sp+Ch] [bp-6Ch]@1
  int v7; // [sp+74h] [bp-4h]@2
  signed int pActora; // [sp+80h] [bp+8h]@1

  memset(&Dst, 0, 0x68u);
  Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
  Dst.timeToLive = (rand() & 0x7F) + 128;
  v3 = pActor;
  Dst.uTextureID = pBitmaps_LOD->LoadTexture("effpar02", TEXTURE_DEFAULT);
  pActora = 50;
  Dst.flt_28 = 1.0;
  do
  {
    v7 = (unsigned __int8)rand() + v3->vPosition.x - 127;
    Dst.x = (double)v7;
    v7 = (unsigned __int8)rand() + v3->vPosition.y - 127;
    Dst.y = (double)v7;
    v7 = v3->vPosition.z + (unsigned __int8)rand();
    Dst.z = (double)v7;
    if ( uDiffuse )
    {
      Dst.uDiffuse = uDiffuse;
    }
    else
    {
      v4 = rand() << 16;
      Dst.uDiffuse = rand() | v4;
    }
    pEngine->pParticleEngine->AddParticle(&Dst);
    --pActora;
  }
  while ( pActora );
}

//----- (004A7F74) --------------------------------------------------------
void stru6::_4A7F74(int x, int y, int z)
{
  stru6 *v4; // esi@1
  char v5; // al@1
  signed int v6; // edi@1
  unsigned int v7; // eax@1
  double v8; // st7@2
  double v9; // st7@2
  Particle_sw local_0; // [sp+8h] [bp-78h]@1
  double v11; // [sp+70h] [bp-10h]@1
  double v12; // [sp+78h] [bp-8h]@1
  float z1; // [sp+88h] [bp+8h]@2

  v4 = this;
  memset(&local_0, 0, 0x68u);
  local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_1;
  local_0.uDiffuse = 0x7E7E7E;
  v5 = rand();
  local_0.flt_28 = 1.0;
  v6 = 8;
  local_0.timeToLive = (v5 & 0x7F) + 128;
  v7 = v4->uTextureID_effpar1;
  v12 = (double)x;
  local_0.uTextureID = v7;
  v11 = (double)y;
  do
  {
    v8 = pRnd->GetRandom();
    local_0.x = v8 * 40.0 - 20.0 + v12;
    v9 = pRnd->GetRandom();
    z1 = (double)z;
    local_0.z = z1;
    local_0.y = v9 * 40.0 - 20.0 + v11;
    local_0.r = pRnd->GetRandom() * 400.0 - 200.0;
    local_0.g = pRnd->GetRandom() * 400.0 - 200.0;
    local_0.b = pRnd->GetRandom() * 150.0 + 50.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    --v6;
  }
  while ( v6 );
}

//----- (004A806F) --------------------------------------------------------
int stru6::_4A806F_get_mass_distortion_value(Actor *pActor)
{
    int v2; // ecx@1
    int v3; // eax@1
    double v4; // st7@2

    v3 = *(int *)&pActor->pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].expire_time.value - pMiscTimer->uTotalGameTimeElapsed;
    if (v3 > 64)
    {
        v2 = (v3 - 64) * (v3 - 64);
        v4 = (double)v2 / 5120.0 + 0.2;
    }
    else if (v3 > 0)
    {
        v2 = v3 * v3;
        v4 = 1.0 - (double)(signed int)(v3 * v3) / 5120.0;
    }
    else
    {
        pActor->pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Reset();
        v4 = 1.0;
    }

    return fixpoint_from_float(v4);
}

//----- (004A81CA) --------------------------------------------------------
bool stru6::RenderAsSprite(SpriteObject *a2)
{
  //stru6 *v2; // ebx@1
  int result; // eax@1
//  int v4; // eax@27
  //unsigned int diffuse; // esi@41
  //int v6; // ecx@49
  int v7; // eax@54
  int v8; // eax@55
  char v9; // zf@56
  int v10; // eax@59
  int v11; // eax@61
  int v12; // eax@85
  int v13; // eax@86
  int v14; // eax@96
  int v15; // eax@111
  int v16; // eax@118
  int v17; // eax@139
  int v18; // eax@140
  int v19; // eax@141
  int v20; // eax@151
  char pContainer[7]; // [sp+10h] [bp-8h]@81

  //__debugbreak(); // need to refactor carefully & collect data
  //v2 = this;
  result = a2->uType;

    switch (a2->uType)
    {

        case SPRITE_PROJECTILE_500:
        case SPRITE_PROJECTILE_530:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x00AAAFF, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_500_IMPACT:
        case SPRITE_PROJECTILE_530_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xAAAFF, uTextureID_effpar1);
            return true;

        case SPRITE_PROJECTILE_505:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x5C310E, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_505_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x5C310E, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_510:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xFF3C1E, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_510_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_515:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x0062D0, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_515_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x0062D0, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_520:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x0AB450, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_520_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x0AB450, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_525:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xC8C805, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_525_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xC8C805, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_535:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xFFFFFF, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_535_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFFFFFF, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_540:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x7E7E7E, uTextureID_effpar1);
            return false;
        case SPRITE_PROJECTILE_540_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x7E7E7E, uTextureID_effpar1);
            return false;

        case SPRITE_PROJECTILE_545:
        case SPRITE_PROJECTILE_550:
        case SPRITE_PROJECTILE_555:
            return true;


        case SPRITE_556: __debugbreak();//what kind of effect is this?
            AddMobileLight(a2, 0xFF0000, 256);
            return false;

        case SPRITE_600: __debugbreak();//what kind of effect is this?
            AddMobileLight(a2, 0xFF3C1E, 256);
            return true;

        case 546: case 547: case 548: case 549:
        case 551: case 552: case 553: case 554: case 557: case 558: case 559:
        case 560: case 561: case 562: case 563: case 564: case 565: case 566: case 567: case 568: case 569:
        case 570: case 571: case 572: case 573: case 574: case 575: case 576: case 577: case 578: case 579:
        case 580: case 581: case 582: case 583: case 584: case 585: case 586: case 587: case 588: case 589:
        case 590: case 591: case 592: case 593: case 594: case 595: case 596: case 597: case 598: case 599:
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            return false;

        case SPRITE_811:
        case SPRITE_812:
        case SPRITE_813:
        case SPRITE_814:
            return true;


        case SPRITE_SPELL_FIRE_FIRE_BOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xFF3C1E, uTextureID_effpar1);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;

        case SPRITE_SPELL_FIRE_FIRE_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;



        case SPRITE_SPELL_FIRE_FIREBALL:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xFF3C1E, uTextureID_effpar1);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;

        case SPRITE_SPELL_FIRE_FIREBALL_IMPACT:
            AddMobileLight(a2, 0xFF3C1E, 256);
            //if (pRenderer->pRenderD3D)
            {
                if (PID_TYPE(a2->spell_caster_pid) != OBJECT_Actor &&
                    PID_TYPE(a2->spell_caster_pid) != OBJECT_Item)
                {
                    if (field_204 != 4)
                    {
                        field_204++;
                        _4A7688_fireball_collision_particle(a2);
                    }
                    return false;
                }
            }
            return true;


        case SPRITE_SPELL_FIRE_FIRE_SPIKE:
            return true;
        case SPRITE_SPELL_FIRE_FIRE_SPIKE_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xFF3C1E, uTextureID_effpar1, 250.0);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;

        case SPRITE_SPELL_FIRE_IMMOLATION:
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            return false;

        case SPRITE_SPELL_FIRE_METEOR_SHOWER:
            return true;
        case SPRITE_SPELL_FIRE_METEOR_SHOWER_1:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xFF3C1E, uTextureID_effpar1, 300.0);
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xFF3C1E, uTextureID_effpar1, 250.0);
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xFF3C1E, uTextureID_effpar1, 200.0);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;

        case SPRITE_SPELL_FIRE_INFERNO:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xFF3C1Eu, uTextureID_effpar1, 250.0);
            return false;

        case SPRITE_SPELL_FIRE_INCINERATE:
            return true;
        case SPRITE_SPELL_FIRE_INCINERATE_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            _4A75CC_single_spell_collision_particle(a2, 0xFF3C1E, uTextureID_effpar1);
            AddMobileLight(a2, 0xFF3C1E, 256);
            return false;



        case SPRITE_SPELL_AIR_SPARKS:
            //if ( !pRenderer->pRenderD3D )
            //  return true;
            _4A78AE_sparks_spell(a2);
            AddMobileLight(a2, 0x64640F, 128);
            return false;

        case SPRITE_SPELL_AIR_LIGHNING_BOLT:
            //if ( !pRenderer->pRenderD3D )
            //  return true;
            memcpy(pContainer, "sp18h1", 7);
            pRnd->SetRange(1, 6);
            pContainer[5] = pRnd->GetInRange() + '0';
            AddProjectile(a2, 100, pBitmaps_LOD->LoadTexture(pContainer));
            return false;
        case SPRITE_SPELL_AIR_LIGHNING_BOLT_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xC8C814, uTextureID_effpar2, 200.0);
            AddMobileLight(a2, 0xC8C814, 256);
            return false;

        case SPRITE_SPELL_AIR_IMPLOSION:
        case SPRITE_SPELL_AIR_IMPLOSION_IMPACT:
            //if ( pRenderer->pRenderD3D )
            _4A77FD_implosion_particle_d3d(a2);
            /*else
            _4A80DC_implosion_particle_sw(a2);*/
            return false;

        case SPRITE_SPELL_AIR_STARBURST:
            return true;
        case SPRITE_SPELL_AIR_STARBURST_1:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0xC8C814, uTextureID_effpar1, 200.0);
            AddMobileLight(a2, 0xC8C814, 256);
            return false;



        case SPRITE_SPELL_WATER_POISON_SPRAY:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xAB450, uTextureID_effpar1);
            AddMobileLight(a2, 0xAB450, 256);
            return false;
        case SPRITE_SPELL_WATER_POISON_SPRAY_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xAB450, uTextureID_effpar1);
            AddMobileLight(a2, 0xAB450, 256);
            return false;

        case SPRITE_SPELL_WATER_ICE_BOLT:
            return true;
        case SPRITE_SPELL_WATER_ICE_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x9EB9F1, uTextureID_effpar1);
            AddMobileLight(a2, 0x9EB9F1, 256);
            return false;

        case SPRITE_SPELL_WATER_ACID_BURST:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x0AB450, uTextureID_effpar1);
            AddMobileLight(a2, 0x0AB450, 256);
            return false;
        case SPRITE_SPELL_WATER_ACID_BURST_IMPACT:
            return true;

        case SPRITE_SPELL_WATER_ICE_BLAST:
            return true;
        case SPRITE_SPELL_WATER_ICE_BLAST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x9EB9F1, uTextureID_effpar1);
            AddMobileLight(a2, 0x9EB9F1, 256);
            return false;
        case SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x9EB9F1, uTextureID_effpar1);
            return false;


        case SPRITE_SPELL_EARTH_STUN:
            //if ( !pRenderer->pRenderD3D )
            //  return true;
            _4A7C07_stun_spell_fx(a2);
            return false;

        case SPRITE_SPELL_EARTH_DEADLY_SWARM:
        case SPRITE_SPELL_EARTH_DEADLY_SWARM_IMPACT:
            return true;

        case SPRITE_SPELL_EARTH_ROCK_BLAST:
            return true;
        case SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0x5C310E, uTextureID_effpar1);
            return false;

        case SPRITE_SPELL_EARTH_TELEKINESIS:
            return true;

        case SPRITE_SPELL_EARTH_BLADES:
            return true;
        case SPRITE_SPELL_EARTH_BLADES_IMPACT:
            _4A7948_mind_blast_after_effect(a2);
            return false;

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x7E7E7E, uTextureID_effpar1);
            return true;
        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT:
            _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(a2, 0x7E7E7E, uTextureID_effpar1, 200.0);
            return false;
        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0x7E7E7E, uTextureID_effpar1);
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
            //if ( !pRenderer->pRenderD3D )
            //  return true;
            _4A78AE_sparks_spell(a2);
            AddMobileLight(a2, 0x64640F, 128);
            return false;
        case SPRITE_SPELL_BODY_HARM_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xF00000, uTextureID_effpar1);
            return false;

        case SPRITE_SPELL_BODY_FLYING_FIST:
            return true;
        case SPRITE_SPELL_BODY_FLYING_FIST_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xF00000, uTextureID_effpar1);
            AddMobileLight(a2, 0xF00000, 256);
            return false;



        case SPRITE_SPELL_LIGHT_LIGHT_BOLT:
            _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(a2, 0xFFFFFF, uTextureID_effpar3);
            AddMobileLight(a2, 0xFFFFFF, 128);
            return false;
        case SPRITE_SPELL_LIGHT_LIGHT_BOLT_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFFFFFF, uTextureID_effpar2);
            AddMobileLight(a2, 0xFFFFFF, 256);
            return false;

        case SPRITE_SPELL_LIGHT_DESTROY_UNDEAD:
            AddMobileLight(a2, 0xFFFFFF, 64);
            return false;

        case SPRITE_SPELL_LIGHT_PARALYZE:
            return true;

        case SPRITE_SPELL_LIGHT_SUMMON_ELEMENTAL:
        case SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT:
            return true;

        case SPRITE_SPELL_LIGHT_SUNRAY:
            AddMobileLight(a2, 0xFFFFFFu, 128);
            //if ( !pRenderer->pRenderD3D )
            //  return true;
            AddProjectile(a2, 100, -1);
            return false;
        case SPRITE_SPELL_LIGHT_SUNRAY_IMPACT:
            _4A75CC_single_spell_collision_particle(a2, 0xFFFFFF, uTextureID_effpar3);
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
            _4A75CC_single_spell_collision_particle(a2, 0x7E7E7E, uTextureID_effpar1);
            return false;

        case SPRITE_SPELL_DARK_SACRIFICE:
        case SPRITE_SPELL_DARK_DRAGON_BREATH:
        case SPRITE_SPELL_DARK_DRAGON_BREATH_1:
            return true;
  }

  return false;
}

//----- (004A89BD) --------------------------------------------------------
void stru6::SetPlayerBuffAnim(unsigned __int16 uSpellID, unsigned __int16 uPlayerID)
{
  //stru6 *v3; // edi@1
  PlayerBuffAnim *v4; // esi@1
  const char *v6; // [sp-4h] [bp-10h]@2

  v4 = &pPlayerBuffs[uPlayerID];
  v4->uSpellAnimTimeElapsed = 0;
  v4->bRender = uSpellID != 0;

  switch (uSpellID)
  {
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
      __debugbreak(); // spell id == 152 wtf
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

    case SPELL_FIRE_HASTE:                  v6 = "spell05"; break;
    case SPELL_AIR_PROTECTION_FROM_AIR:     v6 = "spell14"; break;
    case SPELL_AIR_SHIELD:                  v6 = "spell17"; break;
    case SPELL_WATER_PROTECTION_FROM_WATER: v6 = "spell25"; break;
    case SPELL_EARTH_PROTECTION_FROM_EARTH: v6 = "spell36"; break;
    case SPELL_EARTH_STONESKIN:             v6 = "spell38"; break;
    case SPELL_SPIRIT_BLESS:                v6 = "spell46"; break;
    case SPELL_SPIRIT_HEROISM:              v6 = "spell51"; break;
    case SPELL_SPIRIT_RESSURECTION:         v6 = "spell55"; break;
    case SPELL_MIND_PROTECTION_FROM_MIND:   v6 = "spell58"; break;
    case SPELL_BODY_PROTECTION_FROM_BODY:   v6 = "spell69"; break;
    case SPELL_BODY_REGENERATION:           v6 = "spell71"; break;
    case SPELL_BODY_HAMMERHANDS:            v6 = "spell73"; break;
    case SPELL_BODY_PROTECTION_FROM_MAGIC:  v6 = "spell75"; break;

    default:
      v4->bRender = false;
      return;
  }
  
  v4->uSpellIconID = pIconsFrameTable->FindIcon(v6);
  if (v4->bRender)
    v4->uSpellAnimTime = pIconsFrameTable->GetIcon(v4->uSpellIconID)->GetAnimLength();
}

//----- (004A8BDF) --------------------------------------------------------
void stru6::FadeScreen__like_Turn_Undead_and_mb_Armageddon(unsigned int uDiffuseColor, unsigned int uFadeTime)
{
  this->uFadeTime = uFadeTime;
  this->uFadeLength = uFadeTime;
  this->uFadeColor = uDiffuseColor;
}

//----- (004A8BFC) --------------------------------------------------------
int stru6::_4A8BFC() //for SPELL_LIGHT_PRISMATIC_LIGHT
{
  uAnimLength = 8 * pSpriteFrameTable->pSpriteSFrames[pSpriteFrameTable->FastFindSprite("spell84")].uAnimLength;
  return uAnimLength;
}

//----- (004A8C27) --------------------------------------------------------
void stru6::RenderSpecialEffects()
{
    double v4; // st7@4
    double v5; // st6@4
    float v7; // ST14_4@6
    unsigned int v8; // ST14_4@8
    SpriteFrame *v10; // eax@8
    int v11; // edi@8
    RenderVertexD3D3 vd3d[4]; // [sp+60h] [bp-8Ch]@9

    if (uNumProjectiles)
    {
        DrawProjectiles();
        uNumProjectiles = 0;
    }

    field_204 = 0;
    if (uFadeTime > 0)
    {
        v4 = (double)uFadeTime / (double)uFadeLength;
        v5 = 1.0 - v4 * v4;
        //v6 = v5;
        if (v5 > 0.9)
            v5 = 1.0 - (v5 - 0.9) * 10.0;
        v7 = v5;
        pRenderer->ScreenFade(uFadeColor, v7);
        uFadeTime -= pEventTimer->uTimeElapsed;
    }

    if (uAnimLength > 0)
    {
        v8 = 8 * pSpriteFrameTable->pSpriteSFrames[pSpriteFrameTable->FastFindSprite("spell84")].uAnimLength - uAnimLength;
        v10 = pSpriteFrameTable->GetFrame(pSpriteFrameTable->FastFindSprite("spell84"), v8);
        v11 = v10->pHwSpriteIDs[0];
        uAnimLength -= pEventTimer->uTimeElapsed;
        //if ( pRenderer->pRenderD3D )
        //{
        vd3d[0].pos.x = (double)(signed int)pViewport->uViewportTL_X;
        vd3d[0].pos.y = (double)(signed int)pViewport->uViewportTL_Y;
        vd3d[0].pos.z = 0.0;
        vd3d[0].diffuse = 0x7F7F7Fu;
        vd3d[0].specular = 0;
        vd3d[0].rhw = 1.0;
        vd3d[0].texcoord.x = 0.0;
        vd3d[0].texcoord.y = 0.0;

        vd3d[1].pos.x = (double)(signed int)pViewport->uViewportTL_X;
        vd3d[1].pos.y = (double)(pViewport->uViewportBR_Y + 1);
        vd3d[1].pos.z = 0.0;
        vd3d[1].diffuse = 0x7F7F7Fu;
        vd3d[1].specular = 0;
        vd3d[1].rhw = 1.0;
        vd3d[1].texcoord.x = 0.0;
        vd3d[1].texcoord.y = 1.0;

        vd3d[2].pos.x = (double)(signed int)pViewport->uViewportBR_X;
        vd3d[2].pos.y = (double)(pViewport->uViewportBR_Y + 1);
        vd3d[2].pos.z = 0.0;
        vd3d[2].diffuse = 0x7F7F7Fu;
        vd3d[2].specular = 0;
        vd3d[2].rhw = 1.0;
        vd3d[2].texcoord.x = 1.0;
        vd3d[2].texcoord.y = 1.0;

        vd3d[3].pos.x = (double)(signed int)pViewport->uViewportBR_X;
        vd3d[3].pos.y = (double)(signed int)pViewport->uViewportTL_Y;
        vd3d[3].pos.z = 0.0;
        vd3d[3].diffuse = 0x7F7F7Fu;
        vd3d[3].specular = 0;
        vd3d[3].rhw = 1.0;
        vd3d[3].texcoord.x = 1.0;
        vd3d[3].texcoord.y = 0.0;

        pRenderer->DrawSpecialEffectsQuad(vd3d, pSprites_LOD->pHardwareSprites[v11].pTexture);
    }
}

//----- (004A902A) --------------------------------------------------------
void stru6::DrawPlayerBuffAnims()
{
  for (uint i = 0; i < 4; ++i)
  {
    PlayerBuffAnim* buff = &pPlayerBuffs[i];
    if (!buff->bRender)
      continue;

    buff->uSpellAnimTimeElapsed += pEventTimer->uTimeElapsed;
    if (buff->uSpellAnimTimeElapsed >= buff->uSpellAnimTime)
    {
      buff->bRender = false;
      continue;
    }

    Icon* icon = pIconsFrameTable->GetFrame(buff->uSpellIconID, buff->uSpellAnimTimeElapsed);
    pRenderer->DrawTextureAlphaNew(pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i]/640.0f, 385/480.0f, icon->texture);

    pOtherOverlayList->bRedraw = true;
  }
}

//----- (004A90A0) --------------------------------------------------------
void stru6::LoadAnimations()
{
  uTextureID_effpar1 = pBitmaps_LOD->LoadTexture("effpar01", TEXTURE_DEFAULT);
  uTextureID_effpar2 = pBitmaps_LOD->LoadTexture("effpar02", TEXTURE_DEFAULT);
  uTextureID_effpar3 = pBitmaps_LOD->LoadTexture("effpar03", TEXTURE_DEFAULT);
  uSpriteID_sp57c = pSprites_LOD->LoadSprite("sp57c", 6);

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
int stru6_stru1_indoor_sw_billboard::_4775ED(float a2)
{
	char *v2; // edi@1
	//int v3; // eax@1
	char *v4; // edx@2
	char *v5; // esi@3
	double v6; // st7@6
	signed __int64 v7; // ST84_8@6
	double v8; // ST0C_8@6
	int v9; // esi@6
	double v10; // ST44_8@6
	int v11; // ecx@6
	double v12; // ST34_8@6
	int v13; // ecx@6
	double v14; // ST14_8@6
	double v15; // st7@8
	unsigned int v16; // ecx@8
	signed __int64 v17; // ST64_8@8
	double v18; // ST24_8@8
	int v19; // edi@8
	double v20; // ST3C_8@8
	int v21; // ecx@8
	double v22; // ST2C_8@8
	int v23; // ST9C_4@8
	double v24; // ST1C_8@8
	int *v25; // edi@8
	int v26; // esi@8
	int *v27; // edi@10
	int v28; // esi@10
	//  int result; // eax@12
	__int64 v30; // [sp+A8h] [bp-30h]@8
	float v31; // [sp+B0h] [bp-28h]@6
	float v32; // [sp+B4h] [bp-24h]@6
	int v33; // [sp+B8h] [bp-20h]@6
	int v34; // [sp+BCh] [bp-1Ch]@2
	stru6_stru1_indoor_sw_billboard *v35; // [sp+C0h] [bp-18h]@1
	float v36; // [sp+C4h] [bp-14h]@6
	int v37; // [sp+C8h] [bp-10h]@6
	int v38; // [sp+CCh] [bp-Ch]@1
	float v39; // [sp+D0h] [bp-8h]@6
	int *v40; // [sp+D4h] [bp-4h]@2

	//  __debugbreak();//нужно почистить, срабатывает при применении закла Точечный взрыв и при стрельбе из жезла
	v2 = (char *)&this->field_64[4 * this->uNumVertices];
	v38 = 0;
	*(int *)v2 = this->field_64[0];
	v2 += 4;
	*(int *)v2 = this->field_64[1];
	v2 += 4;
	*(int *)v2 = this->field_64[2];
	*((int *)v2 + 1) = this->field_64[3];
	//v3 = this->uNumVertices;
	v35 = this;
	if (this->uNumVertices > 0)
	{
		v40 = &this->field_64[20];
		v4 = (char *)&this->field_64[3] + 3;

		//while ( 1 )
		for (v34 = this->uNumVertices; v34; --v34)
		{
			v5 = v4 - 15;
			if (*(float *)(v4 - 15) <= (double)a2 && *(float *)(v4 + 1) <= (double)a2)
			{
				v4 += 16;
				//--v34;
				//if ( !v34 )
				//return this->uNumVertices = v38;
				continue;
			}
			if (*(float *)v5 <= (double)a2)
			{
				v6 = (a2 - *(float *)v5) / (*(float *)(v4 + 1) - *(float *)v5);
				v7 = (unsigned __int8)v4[16] - (unsigned int)(unsigned __int8)*v4;
				v36 = v6;
				v31 = (*(float *)(v4 + 5) - *(float *)(v4 - 11)) * v6 + *(float *)(v4 - 11);
				v32 = (*(float *)(v4 + 9) - *(float *)(v4 - 7)) * v6 + *(float *)(v4 - 7);
				*(float *)&v37 = (double)v7 * v6;
				v8 = *(float *)&v37 + 6.7553994e15;
				v9 = (unsigned __int8)*v4;
				*(float *)&v37 = (double)((unsigned __int8)v4[15] - (unsigned int)(unsigned __int8)*(v4 - 1)) * v36;
				v10 = *(float *)&v37 + 6.7553994e15;
				v11 = (unsigned __int8)*(v4 - 2);
				v37 = LODWORD(v10) + (unsigned __int8)*(v4 - 1);
				v39 = (double)((unsigned int)(unsigned __int8)v4[14] - v11) * v36;
				v12 = v39 + 6.7553994e15;
				v13 = LODWORD(v12) + (unsigned __int8)*(v4 - 2);
				v39 = (double)((*(int *)(v4 + 13) & 0xFF) - (*(int *)(v4 - 3) & 0xFFu)) * v36;
				v14 = v39 + 6.7553994e15;
				v33 = (LODWORD(v14) + (*(int *)(v4 - 3) & 0xFF)) | ((v13 | ((v37 | ((LODWORD(v8) + v9) << 8)) << 8)) << 8);
				//this = v35;
				v5 = (char *)&v30 + 4;
			}
			else if (*(float *)(v4 + 1) <= (double)a2)
			{
				v15 = (a2 - *(float *)v5) / (*(float *)(v4 + 1) - *(float *)v5);
				v16 = (unsigned __int8)*v4;
				HIDWORD(v30) = LODWORD(a2);
				v17 = (unsigned __int8)v4[16] - v16;
				v36 = v15;
				v31 = (*(float *)(v4 + 5) - *(float *)(v4 - 11)) * v15 + *(float *)(v4 - 11);
				v32 = (*(float *)(v4 + 9) - *(float *)(v4 - 7)) * v15 + *(float *)(v4 - 7);
				v39 = (double)v17 * v15;
				v18 = v39 + 6.7553994e15;
				v19 = (unsigned __int8)*v4;
				v39 = (double)((unsigned __int8)v4[15] - (unsigned int)(unsigned __int8)*(v4 - 1)) * v36;
				v20 = v39 + 6.7553994e15;
				v21 = (unsigned __int8)*(v4 - 2);
				v37 = LODWORD(v20) + (unsigned __int8)*(v4 - 1);
				v39 = (double)((unsigned int)(unsigned __int8)v4[14] - v21) * v36;
				v22 = v39 + 6.7553994e15;
				v23 = LODWORD(v22) + (unsigned __int8)*(v4 - 2);
				v39 = (double)((*(int *)(v4 + 13) & 0xFF) - (*(int *)(v4 - 3) & 0xFFu)) * v36;
				v24 = v39 + 6.7553994e15;
				v33 = (LODWORD(v24) + (*(int *)(v4 - 3) & 0xFF)) | ((v23 | ((v37 | ((LODWORD(v18) + v19) << 8)) << 8)) << 8);
				v25 = v40;
				*v40 = *(int *)v5;
				v26 = (int)(v5 + 4);
				++v25;
				*v25 = *(int *)v26;
				v26 += 4;
				++v25;
				++v38;
				v40 += 4;
				*v25 = *(int *)v26;
				v25[1] = *(int *)(v26 + 4);
				v5 = (char *)&v30 + 4;
			}
			v27 = v40;
			++v38;
			*v40 = *(int *)v5;
			v28 = (int)(v5 + 4);
			++v27;
			*v27 = *(int *)v28;
			v28 += 4;
			++v27;
			v40 += 4;
			*v27 = *(int *)v28;
			v27[1] = *(int *)(v28 + 4);
			v4 += 16;
			//--v34;
			//if ( !v34 )
			//return this->uNumVertices = v38;
		}
	}
	return this->uNumVertices = v38;
}

//----- (00477927) --------------------------------------------------------
int stru6_stru1_indoor_sw_billboard::_477927(float a2)
{
	char *v2; // edi@1
	int v3; // eax@1
	char *v4; // edx@2
	char *v5; // esi@3
	double v6; // st7@6
	signed __int64 v7; // ST84_8@6
	double v8; // ST0C_8@6
	int v9; // esi@6
	double v10; // ST44_8@6
	int v11; // ecx@6
	double v12; // ST34_8@6
	int v13; // ecx@6
	double v14; // ST14_8@6
	double v15; // st7@8
	unsigned int v16; // ecx@8
	signed __int64 v17; // ST64_8@8
	double v18; // ST24_8@8
	int v19; // edi@8
	double v20; // ST3C_8@8
	int v21; // ecx@8
	double v22; // ST2C_8@8
	int v23; // ST9C_4@8
	double v24; // ST1C_8@8
	int *v25; // edi@8
	int v26; // esi@8
	int *v27; // edi@10
	int v28; // esi@10
	//  int result; // eax@12
	__int64 v30; // [sp+A8h] [bp-30h]@8
	float v31; // [sp+B0h] [bp-28h]@6
	float v32; // [sp+B4h] [bp-24h]@6
	int v33; // [sp+B8h] [bp-20h]@6
	int v34; // [sp+BCh] [bp-1Ch]@2
	stru6_stru1_indoor_sw_billboard *v35; // [sp+C0h] [bp-18h]@1
	float v36; // [sp+C4h] [bp-14h]@6
	int v37; // [sp+C8h] [bp-10h]@6
	int v38; // [sp+CCh] [bp-Ch]@1
	float v39; // [sp+D0h] [bp-8h]@6
	int *v40; // [sp+D4h] [bp-4h]@2

	__debugbreak();//нужно почистить
	v2 = (char *)&this->field_64[4 * this->uNumVertices];
	v38 = 0;
	*(int *)v2 = this->field_64[0];
	v2 += 4;
	*(int *)v2 = this->field_64[1];
	v2 += 4;
	*(int *)v2 = this->field_64[2];
	*((int *)v2 + 1) = this->field_64[3];
	v3 = this->uNumVertices;
	v35 = this;
	if (v3 > 0)
	{
		v40 = &this->field_64[20];
		v4 = (char *)&this->field_64[3] + 3;
		v34 = v3;
		while (1)
		{
			v5 = v4 - 15;
			if (*(float *)(v4 - 15) >= (double)a2 && *(float *)(v4 + 1) >= (double)a2)
			{
				v4 += 16;
				--v34;
				if (!v34)
					return this->uNumVertices = v38;
				continue;
			}
			if (*(float *)v5 >= (double)a2)
			{
				v6 = (a2 - *(float *)v5) / (*(float *)(v4 + 1) - *(float *)v5);
				v7 = (unsigned __int8)v4[16] - (unsigned int)(unsigned __int8)*v4;
				v36 = v6;
				v31 = (*(float *)(v4 + 5) - *(float *)(v4 - 11)) * v6 + *(float *)(v4 - 11);
				v32 = (*(float *)(v4 + 9) - *(float *)(v4 - 7)) * v6 + *(float *)(v4 - 7);
				*(float *)&v37 = (double)v7 * v6;
				v8 = *(float *)&v37 + 6.7553994e15;
				v9 = (unsigned __int8)*v4;
				*(float *)&v37 = (double)((unsigned __int8)v4[15] - (unsigned int)(unsigned __int8)*(v4 - 1)) * v36;
				v10 = *(float *)&v37 + 6.7553994e15;
				v11 = (unsigned __int8)*(v4 - 2);
				v37 = LODWORD(v10) + (unsigned __int8)*(v4 - 1);
				v39 = (double)((unsigned int)(unsigned __int8)v4[14] - v11) * v36;
				v12 = v39 + 6.7553994e15;
				v13 = LODWORD(v12) + (unsigned __int8)*(v4 - 2);
				v39 = (double)((*(int *)(v4 + 13) & 0xFF) - (*(int *)(v4 - 3) & 0xFFu)) * v36;
				v14 = v39 + 6.7553994e15;
				v33 = (LODWORD(v14) + (*(int *)(v4 - 3) & 0xFF)) | ((v13 | ((v37 | ((LODWORD(v8) + v9) << 8)) << 8)) << 8);
				//this = v35;
				v5 = (char *)&v30 + 4;
			}
			else if (*(float *)(v4 + 1) >= (double)a2)
			{
				v15 = (a2 - *(float *)v5) / (*(float *)(v4 + 1) - *(float *)v5);
				v16 = (unsigned __int8)*v4;
				HIDWORD(v30) = LODWORD(a2);
				v17 = (unsigned __int8)v4[16] - v16;
				v36 = v15;
				v31 = (*(float *)(v4 + 5) - *(float *)(v4 - 11)) * v15 + *(float *)(v4 - 11);
				v32 = (*(float *)(v4 + 9) - *(float *)(v4 - 7)) * v15 + *(float *)(v4 - 7);
				v39 = (double)v17 * v15;
				v18 = v39 + 6.7553994e15;
				v19 = (unsigned __int8)*v4;
				v39 = (double)((unsigned __int8)v4[15] - (unsigned int)(unsigned __int8)*(v4 - 1)) * v36;
				v20 = v39 + 6.7553994e15;
				v21 = (unsigned __int8)*(v4 - 2);
				v37 = LODWORD(v20) + (unsigned __int8)*(v4 - 1);
				v39 = (double)((unsigned int)(unsigned __int8)v4[14] - v21) * v36;
				v22 = v39 + 6.7553994e15;
				v23 = LODWORD(v22) + (unsigned __int8)*(v4 - 2);
				v39 = (double)((*(int *)(v4 + 13) & 0xFF) - (*(int *)(v4 - 3) & 0xFFu)) * v36;
				v24 = v39 + 6.7553994e15;
				v33 = (LODWORD(v24) + (*(int *)(v4 - 3) & 0xFF)) | ((v23 | ((v37 | ((LODWORD(v18) + v19) << 8)) << 8)) << 8);
				v25 = v40;
				*v40 = *(int *)v5;
				v26 = (int)(v5 + 4);
				++v25;
				*v25 = *(int *)v26;
				v26 += 4;
				++v25;
				++v38;
				v40 += 4;
				*v25 = *(int *)v26;
				v25[1] = *(int *)(v26 + 4);
				v5 = (char *)&v30 + 4;
			}
			v27 = v40;
			++v38;
			*v40 = *(int *)v5;
			v28 = (int)(v5 + 4);
			++v27;
			*v27 = *(int *)v28;
			v28 += 4;
			++v27;
			v40 += 4;
			*v27 = *(int *)v28;
			v27[1] = *(int *)(v28 + 4);
			v4 += 16;
			--v34;
			if (!v34)
				return this->uNumVertices = v38;
		}
	}
	return this->uNumVertices = v38;
}

//----- (00477C61) --------------------------------------------------------
int stru6_stru1_indoor_sw_billboard::sub_477C61()
{
	//stru6_stru1_indoor_sw_billboard *v1; // ebx@1
	int v2; // ecx@2
	int v3; // eax@3
	double v4; // st7@4
	double v5; // st7@5
	double v6; // st6@5
	double v7; // st5@6
	float v8; // ST30_4@8
	float v9; // ST24_4@8
	double v10; // st7@8
	double v11; // st6@8
	double v12; // st5@8
	float v13; // ST24_4@13
	int v14; // esi@13
	char *v15; // esi@15
	//signed int v16; // eax@16
	//  __int16 v17; // fps@16
	//  unsigned __int8 v18; // c2@16
	//  unsigned __int8 v19; // c3@16
	double v20; // st6@16
	float v21; // ST18_4@17
	float v22; // ST2C_4@17
	float v23; // ST34_4@17
	float v24; // ST24_4@17
	double v25; // st7@17
	double v26; // st6@17
	float v27; // ST34_4@18
	float v28; // ST30_4@18
	int v29; // eax@19
	signed int v31; // [sp+8h] [bp-28h]@15
	float v32; // [sp+Ch] [bp-24h]@16
	float v33; // [sp+14h] [bp-1Ch]@16
	float v34; // [sp+18h] [bp-18h]@16
	float v35; // [sp+1Ch] [bp-14h]@17
	float v36; // [sp+20h] [bp-10h]@4
	float v37; // [sp+24h] [bp-Ch]@4
	float v38; // [sp+24h] [bp-Ch]@16
	float v39; // [sp+28h] [bp-8h]@9
	float v40; // [sp+28h] [bp-8h]@16
	float v41; // [sp+2Ch] [bp-4h]@6
	float v42; // [sp+2Ch] [bp-4h]@9

	//__debugbreak();//нужно почистить, срабатывает при применении  закла Точечный взрыв
	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
	{

		if (this->uNumVertices > 0)
		{
			v3 = (int)&this->field_14[1];
			//do
			for (v2 = 0; v2 < this->uNumVertices; ++v2)
			{
				v4 = *(float *)(v3 - 4);
				LODWORD(v37) = *(int *)v3;
				LODWORD(v36) = *(int *)(v3 + 4);
				if (pIndoorCameraD3D->sRotationX)
				{
					v5 = v4 - (double)pIndoorCameraD3D->vPartyPos.x;
					v6 = v37 - (double)pIndoorCameraD3D->vPartyPos.y;
					//if ( pRenderer->pRenderD3D )
					//{
					v41 = pIndoorCameraD3D->fRotationYSine * v6 + pIndoorCameraD3D->fRotationYCosine * v5;
					v7 = pIndoorCameraD3D->fRotationYSine * v5 - pIndoorCameraD3D->fRotationYCosine * v6;
					/*}
					else
					{
					v41 = pBLVRenderParams->fCosineY * v5 - pBLVRenderParams->fSineY * v6;
					v7 = pBLVRenderParams->fSineY * v5 + pBLVRenderParams->fCosineY * v6;
					}*/
					v8 = v7;
					v9 = v36 - (double)pIndoorCameraD3D->vPartyPos.z;
					v10 = pIndoorCameraD3D->fRotationXCosine * v41 - pIndoorCameraD3D->fRotationXSine * v9;
					v11 = v8;
					v12 = pIndoorCameraD3D->fRotationXCosine * v9 + pIndoorCameraD3D->fRotationXSine * v41;
				}
				else
				{
					v42 = v4 - (double)pIndoorCameraD3D->vPartyPos.x;
					v39 = v37 - (double)pIndoorCameraD3D->vPartyPos.y;
					//if ( pRenderer->pRenderD3D )
					//{
					v10 = pIndoorCameraD3D->fRotationYSine * v39 + pIndoorCameraD3D->fRotationYCosine * v42;
					v11 = pIndoorCameraD3D->fRotationYSine * v42 - pIndoorCameraD3D->fRotationYCosine * v39;
					/*}
					else
					{
					v10 = pBLVRenderParams->fCosineY * v42 - pBLVRenderParams->fSineY * v39;
					v11 = pBLVRenderParams->fSineY * v42 + pBLVRenderParams->fCosineY * v39;
					}*/
					v12 = v36 - (double)pIndoorCameraD3D->vPartyPos.z;
				}
				v13 = v12;
				//++v2;
				*(int *)(v3 + 84) = LODWORD(v13);
				v14 = *(int *)(v3 + 8);
				*(float *)(v3 + 76) = v10;
				*(int *)(v3 + 88) = v14;
				*(float *)(v3 + 80) = v11;
				v3 += 16;
			}
			//while ( v2 < this->uNumVertices );
		}
	}
	else
	{
		v15 = (char *)&this->field_14[1];
		//do
		for (v31 = 3; v31; --v31)
		{
			v40 = (double)stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX) * 0.0000152587890625;
			v32 = (double)stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX) * 0.0000152587890625;
			v34 = (double)stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY) * 0.0000152587890625;
			v33 = (double)stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY) * 0.0000152587890625;
			//v16 = stru_5C6E00->Sin(pODMRenderParams->rotation_y);
			LODWORD(v38) = *(int *)v15;
			//UNDEF(v17);
			v20 = *((float *)v15 - 1) - (double)pIndoorCameraD3D->vPartyPos.x;
			//if ( v19 | v18 )
			if (pIndoorCameraD3D->vPartyPos.x == 0)
			{
				v27 = v20;
				LODWORD(v35) = *((int *)v15 + 1);
				v28 = v38 - (double)pIndoorCameraD3D->vPartyPos.y;
				v25 = v33 * v28 + v34 * v27;
				v26 = v34 * v28 - v33 * v27;
			}
			else
			{
				v21 = v20;
				v22 = v38 - (double)pIndoorCameraD3D->vPartyPos.y;
				v23 = v33 * v22 + v34 * v21;
				v24 = *((float *)v15 + 1) - (double)pIndoorCameraD3D->vPartyPos.z;
				v25 = v32 * v24 + v40 * v23;
				v26 = v34 * v22 - v33 * v21;
				v35 = v40 * v24 - v32 * v23;
			}
			*((int *)v15 + 21) = LODWORD(v35);
			v29 = *((int *)v15 + 2);
			*((float *)v15 + 19) = v25;
			*((int *)v15 + 22) = v29;
			*((float *)v15 + 20) = v26;
			v15 += 16;
			//--v31;
		}
		//while ( v31 );
	}
	this->uNumVertices = 3;
	return 1;
}

//----- (00477F63) --------------------------------------------------------
bool stru6_stru1_indoor_sw_billboard::sub_477F63()
{
	signed int v1; // ebx@1
	double v3; // st7@2
	//int v4; // edx@4
	char *v5; // ecx@5
	int v6; // edi@5
	float v7; // ST08_4@13
	signed int v9; // [sp+Ch] [bp-8h]@1
	float v10; // [sp+10h] [bp-4h]@2

	//__debugbreak();// почистить
	v1 = 0;
	v9 = 0;
	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
	{
		v10 = 16192.0;
		v3 = (double)pBLVRenderParams->fov_rad_fixpoint * 0.000015258789;
	}
	else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
	{
		v10 = (double)pODMRenderParams->shading_dist_mist;
		v3 = 8.0;
	}
	else
		__debugbreak();//Error
	if (this->uNumVertices <= 0)
	{
		memcpy(&this->field_14[40], &this->field_14[20], 16 * this->uNumVertices);
		return this->uNumVertices != 0;
	}
	v5 = (char *)&this->field_14[20];
	for (v6 = 0; v6 < this->uNumVertices; v6++)
	{
		if (v3 >= *(float *)v5 || *(float *)v5 >= (double)v10)
		{
			if (v3 < *(float *)v5)
				v9 = 1;
			else
				v1 = 1;
		}
		v5 += 16;
	}
	if (!v1)
	{
		if (v9)
		{
			this->_477927(v10);
			return this->uNumVertices != 0;
		}
		memcpy(&this->field_14[40], &this->field_14[20], 16 * this->uNumVertices);
		return this->uNumVertices != 0;
	}
	v7 = v3;
	_4775ED(v7);
	return this->uNumVertices != 0;
}

//----- (0047802A) --------------------------------------------------------
int stru6_stru1_indoor_sw_billboard::sub_47802A()
{
	double v6; // st7@4
	signed int v16; // [sp+38h] [bp-Ch]@1
	int a6; // [sp+3Ch] [bp-8h]@5
	int a5; // [sp+40h] [bp-4h]@5

	//  __debugbreak(); //необходимо проверить this->field_B4[i*4+16]
	v16 = 0;
	if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
	{
		for (int i = 0; i < this->uNumVertices; i++)
		{
			v6 = (double)pBLVRenderParams->fov_rad_fixpoint * 0.000015258789 / this->field_B4[i * 4];
			//if ( pRenderer->pRenderD3D )
			{
				pIndoorCameraD3D->Project(round_to_int(this->field_B4[i * 4]), round_to_int(this->field_B4[i * 4 + 1]), round_to_int(this->field_B4[i * 4 + 2]),
					&a5, &a6);
				this->field_B4[i * 4 + 16] = (double)a5;
				this->field_B4[i * 4 + 17] = (double)a6;
				this->field_B4[i * 4 + 18] = round_to_int(this->field_B4[i * 4]);
			}
			/*else
			{
			this->field_B4[i*4+16] = (double)pBLVRenderParams->uViewportCenterX - v6 * this->field_B4[i*4+1];
			this->field_B4[i*4+17] = (double)pBLVRenderParams->uViewportCenterY - v6 * this->field_B4[i*4+2];
			this->field_B4[i*4+18] = this->field_B4[i*4];
			}*/
			this->field_B4[i * 4 + 19] = this->field_B4[i * 4 + 3];
			if ((double)(signed int)pViewport->uViewportTL_X <= this->field_B4[i * 4 + 16] && (double)(signed int)pViewport->uViewportBR_X > this->field_B4[i * 4 + 16]
				&& (double)(signed int)pViewport->uViewportTL_Y <= this->field_B4[i * 4 + 17] && (double)(signed int)pViewport->uViewportBR_Y > this->field_B4[i * 4 + 17])
				v16 = 1;
		}
	}
	else
	{
		for (int i = 0; i < this->uNumVertices; i++)
		{
			this->field_B4[i * 4 + 20] = (double)pViewport->uScreenCenterX - (double)pODMRenderParams->int_fov_rad / this->field_B4[i * 4] * this->field_B4[i * 4 + 1];
			this->field_B4[i * 4 + 21] = (double)pViewport->uScreenCenterY - (double)pODMRenderParams->int_fov_rad / this->field_B4[i * 4] * this->field_B4[i * 4 + 2];
			*((int *)&this->field_B4[i * 4 + 22]) = (int)this->field_B4[i * 4];
			*((int *)&this->field_B4[i * 4 + 23]) = this->field_B4[i * 4 + 3];
			if ((double)(signed int)pViewport->uViewportTL_X <= this->field_B4[i * 4 + 20] && (double)(signed int)pViewport->uViewportBR_X > this->field_B4[i * 4 + 20]
				&& (double)(signed int)pViewport->uViewportTL_Y <= this->field_B4[i * 4 + 21] && (double)(signed int)pViewport->uViewportBR_Y > this->field_B4[i * 4 + 21])
				v16 = 1;
		}
	}
	return v16;
}
