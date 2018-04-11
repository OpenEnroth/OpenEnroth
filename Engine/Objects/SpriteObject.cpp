#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Engine/SpellFxRenderer.h"

#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "Media/Audio/AudioPlayer.h"

#include "SpriteObject.h"
#include "../Party.h"
#include "../TurnEngine/TurnEngine.h"
#include "../OurMath.h"
#include "ObjectList.h"
#include "../LOD.h"
#include "Actor.h"
#include "../Events.h"
#include "../stru298.h"
#include "../Random.h"




size_t uNumSpriteObjects;
std::array<SpriteObject, MAX_SPRITE_OBJECTS> pSpriteObjects;

//----- (00404828) --------------------------------------------------------
SpriteObject::SpriteObject()
{
  field_22_glow_radius_multiplier = 1;
  uSoundID = 0;
  uFacing = 0;
  vVelocity.z = 0;
  vVelocity.y = 0;
  vVelocity.x = 0;
  uType = SPRITE_NULL;
  uObjectDescID = 0;
  field_61 = 0;
  field_60_distance_related_prolly_lod = 0;
  field_20 = 0;
  uSpriteFrameID = 0;
  spell_skill = 0;
  spell_level = 0;
  spell_id = 0;
  field_54 = 0;
}

int SpriteObject::Create(int yaw, int pitch, int a4, int a5) {
  int angle = yaw;
  if (!uObjectDescID) {
    return -1;
  }

  int v6 = 1000;
  for (unsigned int i = 0; i < MAX_SPRITE_OBJECTS; ++i) {
    if (!pSpriteObjects[i].uObjectDescID) {
      v6 = i;
      break;
    }
  }

  if (v6 >= 1000) {
    return -1;
  }
  field_64.x = vPosition.x;
  field_64.y = vPosition.y;
  field_64.z = vPosition.z;

  assert(sizeof(SpriteObject) == 0x70);

  switch (a5) {
    case 0:
      break;  //do nothing
    case 1:
      Vec3_int_::Rotate(24, stru_5C6E00->uIntegerHalfPi + uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
      break;
    case 2:
      Vec3_int_::Rotate(8, stru_5C6E00->uIntegerHalfPi + uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
      break;
    case 3:
      Vec3_int_::Rotate(8, uFacing - stru_5C6E00->uIntegerHalfPi, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
      break;
    case 4:
      Vec3_int_::Rotate(24, uFacing - stru_5C6E00->uIntegerHalfPi, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
      break;
    default:
      assert(false);
      return 0;
      break;
  }

  if (a4) {
    long long v13 = fixpoint_mul(stru_5C6E00->Cos(angle), stru_5C6E00->Cos(pitch));
    long long a5a = fixpoint_mul(stru_5C6E00->Sin(angle), stru_5C6E00->Cos(pitch));
    vVelocity.x = fixpoint_mul(v13, a4);
    vVelocity.y = fixpoint_mul(a5a, a4);
    vVelocity.z = fixpoint_mul(stru_5C6E00->Sin(pitch), a4);
  } else {
    vVelocity.y = 0;
    vVelocity.x = 0;
    vVelocity.z = 0;
  }

  memcpy(&pSpriteObjects[v6], this, sizeof(*this));
  if (v6 >= (int)uNumSpriteObjects) {
    uNumSpriteObjects = v6 + 1;
  }
  return v6;
}

//----- (00471C03) --------------------------------------------------------
void SpriteObject::UpdateObject_fn0_ODM(unsigned int uLayingItemID)
{
    ObjectDesc *object; // ebx@1
    int v6; // eax@1
    int v7; // ecx@1
    int v8; // edi@1
    int v9; // eax@4
  //  int v17; // ST10_4@25
    //signed int v19; // eax@28
    //Actor *v20; // edi@31
    int v21; // eax@41
  //  int v22; // ecx@43
  //  __int16 v23; // bx@45
  //  char v24; // al@46
    signed int i; // edi@50
    int v26; // edi@52
    int v27; // eax@52
    __int16 v28; // cx@55
    int v29; // eax@55
    //signed int v30; // edi@59
    BSPModel *bmodel; // ecx@61
    ODMFace *face; // edi@61
    int v36; // ecx@67
    __int16 v37; // ax@67
    int v38; // eax@72
    int v44; // eax@77
    int v49; // [sp+Ch] [bp-9Ch]@52
    int v50; // [sp+10h] [bp-98h]@52
    Vec3_int_ v51; // [sp+14h] [bp-94h]@11
    Particle_sw Dst; // [sp+20h] [bp-88h]@45
    int v54; // [sp+8Ch] [bp-1Ch]@1
    int v55; // [sp+90h] [bp-18h]@1
    int v56; // [sp+94h] [bp-14h]@11
    int v57; // [sp+98h] [bp-10h]@1
    int v58; // [sp+9Ch] [bp-Ch]@1
    int on_water; // [sp+A0h] [bp-8h]@1
    int v60; // [sp+A4h] [bp-4h]@11

    v58 = 0;
    object = &pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID];
    v57 = IsTerrainSlopeTooHigh(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y);
    v55 = 0;
    v6 = ODM_GetFloorLevel(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, object->uHeight, &on_water, &v55, 0);
    v7 = v6;
    v54 = v6;
    v8 = v6 + 1;
    if (pSpriteObjects[uLayingItemID].vPosition.z <= v6 + 1)
    {
        if (on_water)
        {
            v9 = v6 + 60;
            if (v55)
                v9 = v6 + 30;
            sub_42F960_create_object(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, v9);
            SpriteObject::OnInteraction(uLayingItemID);
        }
    }
    else
        v58 = 1;
    if (!(object->uFlags & OBJECT_DESC_NO_GRAVITY))
    {
        if (v58)
        {
            pSpriteObjects[uLayingItemID].vVelocity.z -= (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            goto LABEL_13;
        }
        if (v57)
        {
            pSpriteObjects[uLayingItemID].vPosition.z = v8;
            ODM_GetTerrainNormalAt(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, &v51);
            pSpriteObjects[uLayingItemID].vVelocity.z -= (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            v56 = abs(v51.y * pSpriteObjects[uLayingItemID].vVelocity.y + v51.z * pSpriteObjects[uLayingItemID].vVelocity.z + v51.x * pSpriteObjects[uLayingItemID].vVelocity.x) >> 16;
            //v60 = ((unsigned __int64)(v56 * (signed __int64)v51.x) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.x += fixpoint_mul(v56, v51.x);
            //v60 = ((unsigned __int64)(v56 * (signed __int64)v51.y) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.y += fixpoint_mul(v56, v51.y);
            //v60 = ((unsigned __int64)(v56 * (signed __int64)v51.z) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.z += fixpoint_mul(v56, v51.z);
            v7 = v54;
            goto LABEL_13;
        }
        if (object->uFlags & OBJECT_DESC_INTERACTABLE)
        {
            if (pSpriteObjects[uLayingItemID].vPosition.z < v7)
                pSpriteObjects[uLayingItemID].vPosition.z = v8;
            if (!_46BFFA_update_spell_fx(uLayingItemID, 0))
                return;
        }
        pSpriteObjects[uLayingItemID].vPosition.z = v8;
        if (!(object->uFlags & OBJECT_DESC_BOUNCE) || (v21 = -pSpriteObjects[uLayingItemID].vVelocity.z >> 1, pSpriteObjects[uLayingItemID].vVelocity.z = v21, (signed __int16)v21 < 10))
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;

        pSpriteObjects[uLayingItemID].vVelocity.x = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.x);
        pSpriteObjects[uLayingItemID].vVelocity.y = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.y);
        pSpriteObjects[uLayingItemID].vVelocity.z = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.z);
        if ((pSpriteObjects[uLayingItemID].vVelocity.y * pSpriteObjects[uLayingItemID].vVelocity.y
            + pSpriteObjects[uLayingItemID].vVelocity.x * pSpriteObjects[uLayingItemID].vVelocity.x) < 400)
        {
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            memset(&Dst, 0, 0x68u);
            Dst.x = (double)pSpriteObjects[uLayingItemID].vPosition.x;
            Dst.y = (double)pSpriteObjects[uLayingItemID].vPosition.y;
            Dst.z = (double)pSpriteObjects[uLayingItemID].vPosition.z;
            Dst.r = 0.0;
            Dst.g = 0.0;
            Dst.b = 0.0;
            if (object->uFlags & OBJECT_DESC_TRIAL_FIRE)
            {
                Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
                Dst.uDiffuse = 0xFF3C1E;
                Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                Dst.texture = pEngine->GetSpellFxRenderer()->effpar01;
                Dst.particle_size = 1.0f;
                pEngine->pParticleEngine->AddParticle(&Dst);
            }
            else if (object->uFlags & OBJECT_DESC_TRIAL_LINE)
            {
                Dst.type = ParticleType_Line;
                Dst.uDiffuse = rand();
                Dst.timeToLive = 64;
                Dst.texture = nullptr;
                Dst.particle_size = 1.0f;
                pEngine->pParticleEngine->AddParticle(&Dst);
            }
            else if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            {
                Dst.type = ParticleType_Bitmap | ParticleType_8;
                Dst.uDiffuse = rand();
                Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                Dst.texture = pEngine->GetSpellFxRenderer()->effpar03;
                Dst.particle_size = 1.0f;
                pEngine->pParticleEngine->AddParticle(&Dst);
            }
            return;
        }
    }
LABEL_13:
    if (pSpriteObjects[uLayingItemID].vPosition.x >= -0x8000 && pSpriteObjects[uLayingItemID].vPosition.x <= 0x8000
        && pSpriteObjects[uLayingItemID].vPosition.y >= -0x8000 && pSpriteObjects[uLayingItemID].vPosition.y <= 0x8000
        && pSpriteObjects[uLayingItemID].vPosition.z > v7 && pSpriteObjects[uLayingItemID].vPosition.z <= 13000
        || !(object->uFlags & OBJECT_DESC_INTERACTABLE))
        goto LABEL_92;
    if (pSpriteObjects[uLayingItemID].vPosition.z < v7)
        pSpriteObjects[uLayingItemID].vPosition.z = v8;
    if (_46BFFA_update_spell_fx(uLayingItemID, 0))
    {
    LABEL_92:
        stru_721530.field_0 = 0;
        stru_721530.prolly_normal_d = object->uRadius;
        stru_721530.height = object->uHeight;
        stru_721530.field_8_radius = 0;
        stru_721530.field_70 = 0;
        for (v55 = 0; v55 < 100; ++v55)
        {
            stru_721530.position.x = pSpriteObjects[uLayingItemID].vPosition.x;
            stru_721530.normal.x = stru_721530.position.x;
            stru_721530.uSectorID = 0;
            stru_721530.position.y = pSpriteObjects[uLayingItemID].vPosition.y;
            stru_721530.normal.y = pSpriteObjects[uLayingItemID].vPosition.y;
            stru_721530.position.z = pSpriteObjects[uLayingItemID].vPosition.z + stru_721530.prolly_normal_d + 1;
            stru_721530.normal.z = stru_721530.position.z;
            stru_721530.velocity.x = pSpriteObjects[uLayingItemID].vVelocity.x;
            stru_721530.velocity.y = pSpriteObjects[uLayingItemID].vVelocity.y;
            stru_721530.velocity.z = pSpriteObjects[uLayingItemID].vVelocity.z;
            if (stru_721530._47050A(0))
                return;
            _46E889_collide_against_bmodels(0);
            _46E26D_collide_against_sprites(WorldPosToGridCellX(pSpriteObjects[uLayingItemID].vPosition.x), WorldPosToGridCellZ(pSpriteObjects[uLayingItemID].vPosition.y));
            if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) != OBJECT_Player)
                _46EF01_collision_chech_player(0);
            if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) == OBJECT_Actor)
            {
                if ((PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) >= 0)
                    && (PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) < (signed int)(uNumActors - 1)))
                {
                    for (v56 = 0; v56 < uNumActors; ++v56)
                    {
                        if (pActors[PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid)].GetActorsRelation(&pActors[v56]))
                            Actor::_46DF1A_collide_against_actor(v56, 0);
                    }
                }
            }
            else
            {
                for (i = 0; i < (signed int)uNumActors; ++i)
                    Actor::_46DF1A_collide_against_actor(i, 0);
            }
            v26 = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
            v27 = ODM_GetFloorLevel(stru_721530.normal2.x, stru_721530.normal2.y, stru_721530.normal2.z - stru_721530.prolly_normal_d - 1,
                object->uHeight, &v49, &v50, 0);
            if (on_water && v26 < v27 + 60)
            {
                if (v50)
                    v44 = v27 + 30;
                else
                    v44 = v54 + 60;
                sub_42F960_create_object(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, v44);
                SpriteObject::OnInteraction(uLayingItemID);
                return;
            }
            if (stru_721530.field_7C >= stru_721530.field_6C)
            {
                pSpriteObjects[uLayingItemID].vPosition.x = stru_721530.normal2.x;
                pSpriteObjects[uLayingItemID].vPosition.y = stru_721530.normal2.y;
                pSpriteObjects[uLayingItemID].vPosition.z = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
                pSpriteObjects[uLayingItemID].uSectorID = (short)stru_721530.uSectorID;
                memset(&Dst, 0, 0x68u);
                Dst.x = (double)pSpriteObjects[uLayingItemID].vPosition.x;
                Dst.y = (double)pSpriteObjects[uLayingItemID].vPosition.y;
                Dst.z = (double)pSpriteObjects[uLayingItemID].vPosition.z;
                Dst.r = 0.0;
                Dst.g = 0.0;
                Dst.b = 0.0;
                if (object->uFlags & OBJECT_DESC_TRIAL_FIRE)
                {
                    Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
                    Dst.uDiffuse = 0xFF3C1E;
                    Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                    Dst.texture = pEngine->GetSpellFxRenderer()->effpar01;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                    return;
                }
                else if (object->uFlags & OBJECT_DESC_TRIAL_LINE)
                {
                    Dst.type = ParticleType_Line;
                    Dst.texture = nullptr;
                    Dst.uDiffuse = rand();
                    Dst.timeToLive = 64;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                    return;
                }
                else if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
                {
                    Dst.type = ParticleType_Bitmap | ParticleType_8;
                    Dst.uDiffuse = rand();
                    Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                    Dst.texture = pEngine->GetSpellFxRenderer()->effpar03;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                }
                return;
            }
            //v60 = ((unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.x) >> 16);
            pSpriteObjects[uLayingItemID].vPosition.x += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.x);
            //v60 = ((unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.y) >> 16);
            pSpriteObjects[uLayingItemID].vPosition.y += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.y);
            //v60 = ((unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.z) >> 16);
            v28 = (short)stru_721530.uSectorID;
            pSpriteObjects[uLayingItemID].vPosition.z += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
            v29 = pSpriteObjects[uLayingItemID].vPosition.z;
            pSpriteObjects[uLayingItemID].uSectorID = v28;
            stru_721530.field_70 += stru_721530.field_7C;
            if (object->uFlags & OBJECT_DESC_INTERACTABLE)
            {
                if (v29 < v54)
                    pSpriteObjects[uLayingItemID].vPosition.z = v54 + 1;
                if (!_46BFFA_update_spell_fx(uLayingItemID, stru_721530.pid))
                    return;
            }
            if (PID_TYPE(stru_721530.pid) == OBJECT_Decoration)
                break;
            if (PID_TYPE(stru_721530.pid) == OBJECT_BModel)
            {
                bmodel = &pOutdoor->pBModels[(signed int)stru_721530.pid >> 9];
                face = &bmodel->pFaces[PID_ID(stru_721530.pid) & 0x3F];
                if (face->uPolygonType == POLYGON_Floor)
                {
                    pSpriteObjects[uLayingItemID].vPosition.z = bmodel->pVertices.pVertices[face->pVertexIDs[0]].z + 1;
                    if (pSpriteObjects[uLayingItemID].vVelocity.x * pSpriteObjects[uLayingItemID].vVelocity.x
                        + pSpriteObjects[uLayingItemID].vVelocity.y * pSpriteObjects[uLayingItemID].vVelocity.y >= 400)
                    {
                        if (face->uAttributes & FACE_UNKNOW2)
                            EventProcessor(face->sCogTriggeredID, 0, 1);
                    }
                    else
                    {
                        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                    }
                }
                else
                {
                    v56 = abs(face->pFacePlane.vNormal.x * pSpriteObjects[uLayingItemID].vVelocity.x
                        + face->pFacePlane.vNormal.y * pSpriteObjects[uLayingItemID].vVelocity.y
                        + face->pFacePlane.vNormal.z * pSpriteObjects[uLayingItemID].vVelocity.z) >> 16;
                    if ((stru_721530.speed >> 3) > v56)
                        v56 = stru_721530.speed >> 3;
                    //v57 = fixpoint_mul(v56, face->pFacePlane.vNormal.x);
                    //v58 = fixpoint_mul(v56, face->pFacePlane.vNormal.y);
                    v60 = fixpoint_mul(v56, face->pFacePlane.vNormal.z);
                    pSpriteObjects[uLayingItemID].vVelocity.x += 2 * fixpoint_mul(v56, face->pFacePlane.vNormal.x);
                    pSpriteObjects[uLayingItemID].vVelocity.y += 2 * fixpoint_mul(v56, face->pFacePlane.vNormal.y);
                    if (face->pFacePlane.vNormal.z <= 32000)
                        v37 = 2 * (short)v60;
                    else
                    {
                        v36 = v60;
                        pSpriteObjects[uLayingItemID].vVelocity.z += (signed __int16)v60;
                        v58 = fixpoint_mul(0x7D00, v36);
                        v37 = fixpoint_mul(32000, v36);
                    }
                    pSpriteObjects[uLayingItemID].vVelocity.z += v37;
                    if (face->uAttributes & FACE_UNKNOW2)
                        EventProcessor(face->sCogTriggeredID, 0, 1);
                }
            }
        LABEL_74:
            pSpriteObjects[uLayingItemID].vVelocity.x = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.x);
            pSpriteObjects[uLayingItemID].vVelocity.y = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.y);
            pSpriteObjects[uLayingItemID].vVelocity.z = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.z);
        }
        v57 = integer_sqrt(pSpriteObjects[uLayingItemID].vVelocity.x * pSpriteObjects[uLayingItemID].vVelocity.x
            + pSpriteObjects[uLayingItemID].vVelocity.y * pSpriteObjects[uLayingItemID].vVelocity.y);
        v38 = stru_5C6E00->Atan2(pSpriteObjects[uLayingItemID].vPosition.x - pLevelDecorations[PID_ID(stru_721530.pid)].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y - pLevelDecorations[PID_ID(stru_721530.pid)].vPosition.y);
        pSpriteObjects[uLayingItemID].vVelocity.x = fixpoint_mul(stru_5C6E00->Cos(v38), v57);
        pSpriteObjects[uLayingItemID].vVelocity.y = fixpoint_mul(stru_5C6E00->Sin(v38 - stru_5C6E00->uIntegerHalfPi), v57);
        goto LABEL_74;
    }
}

//----- (0047136C) --------------------------------------------------------
void SpriteObject::UpdateObject_fn0_BLV(unsigned int uLayingItemID)
{
    SpriteObject *pSpriteObject; // esi@1
    ObjectDesc *pObject; // edi@1
  //  int v9; // ecx@16
  //  __int16 v10; // di@18
  //  int v14; // ebx@34
    signed int v15; // ebx@46
    int v17; // eax@50
  //  int v18; // eax@52
  //  int v19; // ecx@52
  //  Vec3_short_ *v20; // ecx@53
    __int16 v22; // ax@57
    int v23; // edi@62
  //  unsigned __int8 v27; // sf@64
  //  unsigned __int8 v28; // of@64
  //  __int16 v29; // di@67
  //  char v30; // al@68
    Particle_sw Dst; // [sp+Ch] [bp-84h]@18
    unsigned int uFaceID; // [sp+7Ch] [bp-14h]@4
    int v39; // [sp+80h] [bp-10h]@33
    int v40; // [sp+84h] [bp-Ch]@28
    int v42; // [sp+8Ch] [bp-4h]@4

    pSpriteObject = &pSpriteObjects[uLayingItemID];
    pObject = &pObjectList->pObjects[pSpriteObject->uObjectDescID];
    pSpriteObject->uSectorID = pIndoor->GetSector(pSpriteObject->vPosition.x, pSpriteObject->vPosition.y, pSpriteObject->vPosition.z);
    v42 = BLV_GetFloorLevel(pSpriteObject->vPosition.x, pSpriteObject->vPosition.y, pSpriteObject->vPosition.z, pSpriteObject->uSectorID, &uFaceID);
    if (abs(pSpriteObject->vPosition.x) > 32767
        || abs(pSpriteObject->vPosition.y) > 32767
        || abs(pSpriteObject->vPosition.z) > 20000
        || v42 <= -30000
        && (pSpriteObject->uSectorID == 0))
        //     || (v42 = _46CEC3_get_floor_level(pSpriteObject->vPosition.x, pSpriteObject->vPosition.y, pSpriteObject->vPosition.z, v4, &uFaceID), v42 == -30000)) )
    {
        SpriteObject::OnInteraction(uLayingItemID);
        return;
    }
    if (pObject->uFlags & OBJECT_DESC_NO_GRAVITY)//не падающие объекты
    {
    LABEL_25:
        stru_721530.field_0 = 0;
        stru_721530.prolly_normal_d = pObject->uRadius;
        stru_721530.field_84 = -1;
        stru_721530.height = pObject->uHeight;
        stru_721530.field_8_radius = 0;
        stru_721530.field_70 = 0;
        for (uFaceID = 0; uFaceID < 100; uFaceID++)
        {
            stru_721530.position.x = pSpriteObject->vPosition.x;
            stru_721530.position.y = pSpriteObject->vPosition.y;
            stru_721530.position.z = stru_721530.prolly_normal_d + pSpriteObject->vPosition.z + 1;

            stru_721530.normal.x = stru_721530.position.x;
            stru_721530.normal.y = stru_721530.position.y;
            stru_721530.normal.z = stru_721530.position.z;

            stru_721530.velocity.x = pSpriteObject->vVelocity.x;
            stru_721530.velocity.y = pSpriteObject->vVelocity.y;
            stru_721530.velocity.z = pSpriteObject->vVelocity.z;

            stru_721530.uSectorID = pSpriteObject->uSectorID;
            if (stru_721530._47050A(0))
                return;

            for (v40 = 0; v40 < 100; ++v40)
            {
                _46E44E_collide_against_faces_and_portals(0);
                _46E0B2_collide_against_decorations();
                if (PID_TYPE(pSpriteObject->spell_caster_pid) != OBJECT_Player)
                    _46EF01_collision_chech_player(1);
                if (PID_TYPE(pSpriteObject->spell_caster_pid) == OBJECT_Actor)
                {
                    for (v42 = 0; v42 < (signed int)uNumActors; ++v42)
                    {
                        if (pActors[pSpriteObject->spell_caster_pid >> 3].pMonsterInfo.uID != pActors[v42].pMonsterInfo.uID)
                            //not sure: pMonsterList->pMonsters[v39b->word_000086_some_monster_id-1].uToHitRadius
                            Actor::_46DF1A_collide_against_actor(v42, pMonsterList->pMonsters[pActors[v42].word_000086_some_monster_id - 1].uToHitRadius);
                    }
                }
                else
                {
                    for (v42 = 0; v42 < (signed int)uNumActors; v42++)
                        Actor::_46DF1A_collide_against_actor(v42, pMonsterList->pMonsters[pActors[v42].word_000086_some_monster_id - 1].uToHitRadius);
                }
                if (_46F04E_collide_against_portals())
                    break;
            }
            if (stru_721530.field_7C >= stru_721530.field_6C)
            {
                pSpriteObject->vPosition.x = stru_721530.normal2.x;
                pSpriteObject->vPosition.y = stru_721530.normal2.y;
                pSpriteObject->vPosition.z = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
                pSpriteObject->uSectorID = (short)stru_721530.uSectorID;
                if (!(pObject->uFlags & 0x100))
                    return;
                memset(&Dst, 0, 0x68u);
                Dst.x = (double)pSpriteObject->vPosition.x;
                Dst.y = (double)pSpriteObject->vPosition.y;
                Dst.z = (double)pSpriteObject->vPosition.z;
                Dst.r = 0.0;
                Dst.g = 0.0;
                Dst.b = 0.0;
                if (pObject->uFlags & OBJECT_DESC_TRIAL_FIRE)
                {
                    Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
                    Dst.uDiffuse = 0xFF3C1E;
                    Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                    Dst.texture = pEngine->GetSpellFxRenderer()->effpar01;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                    return;
                }
                else if (pObject->uFlags & OBJECT_DESC_TRIAL_LINE)
                {
                    Dst.type = ParticleType_Line;
                    Dst.uDiffuse = rand();
                    Dst.timeToLive = 64;
                    Dst.texture = 0;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                    return;
                }
                else if (pObject->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
                {
                    Dst.type = ParticleType_Bitmap | ParticleType_8;
                    Dst.uDiffuse = rand();
                    Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                    Dst.texture = pEngine->GetSpellFxRenderer()->effpar03;
                    Dst.particle_size = 1.0f;
                    pEngine->pParticleEngine->AddParticle(&Dst);
                }
                return;
            }
            //v40 = (unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.x) >> 16;
            pSpriteObject->vPosition.x += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.x);
            //v40 = (unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.y) >> 16;
            pSpriteObject->vPosition.y += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.y);
            //v40 = (unsigned __int64)(stru_721530.field_7C * (signed __int64)stru_721530.direction.z) >> 16;
            pSpriteObject->vPosition.z += fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
            pSpriteObject->uSectorID = stru_721530.uSectorID;
            stru_721530.field_70 += stru_721530.field_7C;
            if (pObject->uFlags & OBJECT_DESC_INTERACTABLE && !_46BFFA_update_spell_fx(uLayingItemID, stru_721530.pid))
                return;
            v15 = (signed int)stru_721530.pid >> 3;
            if (PID_TYPE(stru_721530.pid) == OBJECT_Decoration)
            {
                v40 = integer_sqrt(pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x + pSpriteObject->vVelocity.y * pSpriteObject->vVelocity.y);
                v23 = stru_5C6E00->Atan2(pSpriteObject->vPosition.x - pLevelDecorations[v15].vPosition.x,
                    pSpriteObject->vPosition.y - pLevelDecorations[v15].vPosition.y);
                pSpriteObject->vVelocity.x = fixpoint_mul(stru_5C6E00->Cos(v23), v40);
                pSpriteObject->vVelocity.y = fixpoint_mul(stru_5C6E00->Sin(v23), v40);
            }
            if (PID_TYPE(stru_721530.pid) == OBJECT_BModel)
            {
                stru_721530.field_84 = (signed int)PID_ID(stru_721530.pid);
                if (pIndoor->pFaces[v15].uPolygonType != POLYGON_Floor)
                {
                    v42 = abs(pIndoor->pFaces[v15].pFacePlane_old.vNormal.x * pSpriteObject->vVelocity.x
                        + pIndoor->pFaces[v15].pFacePlane_old.vNormal.y * pSpriteObject->vVelocity.y
                        + pIndoor->pFaces[v15].pFacePlane_old.vNormal.z * pSpriteObject->vVelocity.z) >> 16;
                    if ((stru_721530.speed >> 3) > v42)
                        v42 = stru_721530.speed >> 3;
                    pSpriteObject->vVelocity.x += 2 * fixpoint_mul(v42, pIndoor->pFaces[v15].pFacePlane_old.vNormal.x);
                    pSpriteObject->vVelocity.y += 2 * fixpoint_mul(v42, pIndoor->pFaces[v15].pFacePlane_old.vNormal.y);
                    v39 = fixpoint_mul(v42, pIndoor->pFaces[v15].pFacePlane_old.vNormal.z);
                    if (pIndoor->pFaces[v15].pFacePlane_old.vNormal.z <= 32000)
                        v22 = 2 * v39;
                    else
                    {
                        pSpriteObject->vVelocity.z += v39;
                        v22 = fixpoint_mul(32000, v39);
                    }
                    pSpriteObject->vVelocity.z += v22;
                    if (pIndoor->pFaces[v15].uAttributes & FACE_UNKNOW2)
                        EventProcessor(pIndoor->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID].uEventID, 0, 1);
                    pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                if (pObject->uFlags & OBJECT_DESC_BOUNCE)
                {
                    v17 = -pSpriteObject->vVelocity.z / 2;
                    pSpriteObject->vVelocity.z = v17;
                    if ((signed __int16)v17 < 10)
                        pSpriteObject->vVelocity.z = 0;
                    if (pIndoor->pFaces[v15].uAttributes & FACE_UNKNOW2)
                        EventProcessor(pIndoor->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID].uEventID, 0, 1);
                    pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                pSpriteObject->vVelocity.z = 0;
                if (pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x + pSpriteObject->vVelocity.y * pSpriteObject->vVelocity.y >= 400)
                {
                    if (pIndoor->pFaces[v15].uAttributes & FACE_UNKNOW2)
                        EventProcessor(pIndoor->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID].uEventID, 0, 1);
                    pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                pSpriteObject->vVelocity.z = 0;
                pSpriteObject->vVelocity.y = 0;
                pSpriteObject->vVelocity.x = 0;
                pSpriteObject->vPosition.z = pIndoor->pVertices[*pIndoor->pFaces[v15].pVertexIDs].z + 1;
            }
            pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
            pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
            pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
        }
    }
    //для падающих объектов(для примера выброс вещи из инвентаря)
    if (v42 <= pSpriteObject->vPosition.z - 3)
    {
        pSpriteObject->vVelocity.z -= (short)pEventTimer->uTimeElapsed * GetGravityStrength();
        goto LABEL_25;
    }
    if (!(pObject->uFlags & OBJECT_DESC_INTERACTABLE) || _46BFFA_update_spell_fx(uLayingItemID, 0))
    {
        pSpriteObject->vPosition.z = v42 + 1;
        if (pIndoor->pFaces[uFaceID].uPolygonType == POLYGON_Floor)
            pSpriteObject->vVelocity.z = 0;
        else
        {
            if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 45000)
                pSpriteObject->vVelocity.z -= (short)pEventTimer->uTimeElapsed * GetGravityStrength();
        }
        pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
        pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
        pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
        if (pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x + pSpriteObject->vVelocity.y * pSpriteObject->vVelocity.y < 400)
        {
            pSpriteObject->vVelocity.x = 0;
            pSpriteObject->vVelocity.y = 0;
            pSpriteObject->vVelocity.z = 0;
            if (!(pObject->uFlags & OBJECT_DESC_NO_SPRITE))
                return;
            memset(&Dst, 0, 0x68u);
            Dst.x = (double)pSpriteObject->vPosition.x;
            Dst.y = (double)pSpriteObject->vPosition.y;
            Dst.z = (double)pSpriteObject->vPosition.z;
            Dst.r = 0.0;
            Dst.g = 0.0;
            Dst.b = 0.0;
            if (pObject->uFlags & OBJECT_DESC_TRIAL_FIRE)
            {
                Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
                Dst.uDiffuse = 0xFF3C1E;
                Dst.particle_size = 1.0f;
                Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                Dst.texture = pEngine->GetSpellFxRenderer()->effpar01;
                pEngine->pParticleEngine->AddParticle(&Dst);
                return;
            }
            else if (pObject->uFlags & OBJECT_DESC_TRIAL_LINE)
            {
                Dst.type = ParticleType_Line;
                Dst.uDiffuse = rand();
                Dst.timeToLive = 64;
                Dst.texture = nullptr;
                Dst.particle_size = 1.0f;
                pEngine->pParticleEngine->AddParticle(&Dst);
                return;
            }
            else if (pObject->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            {
                Dst.type = ParticleType_Bitmap | ParticleType_8;
                Dst.uDiffuse = rand();
                Dst.particle_size = 1.0f;
                Dst.timeToLive = (unsigned __int8)(rand() & 0x80) + 128;
                Dst.texture = pEngine->GetSpellFxRenderer()->effpar03;
                pEngine->pParticleEngine->AddParticle(&Dst);
            }
            return;
        }
        goto LABEL_25;
    }
}

//----- (00438E35) --------------------------------------------------------
void SpriteObject::ExplosionTraps()
{
    MapInfo *pMapInfo; // esi@1
    int dir_x; // ebx@1
    int v7; // edx@2
    unsigned int v10; // eax@7
    signed int v11; // ebx@8
    signed int v13; // edi@20
    int dir_y; // [sp+Ch] [bp-Ch]@1
    int dir_z; // [sp+10h] [bp-8h]@1
    DAMAGE_TYPE pDamageType; // [sp+14h] [bp-4h]@14

    pMapInfo = &pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)];
    dir_x = abs(pParty->vPosition.x - this->vPosition.x);
    dir_y = abs(pParty->vPosition.y - this->vPosition.y);
    dir_z = abs(pParty->vPosition.z + pParty->sEyelevel - this->vPosition.z);
    if (dir_x < dir_y)
    {
        v7 = dir_x;
        dir_x = dir_y;
        dir_y = v7;
    }
    if (dir_x < dir_z)
    {
        v7 = dir_x;
        dir_x = dir_z;
        dir_z = v7;
    }
    if (dir_y < dir_z)
    {
        v7 = dir_z;
        dir_z = dir_y;
        dir_y = v7;
    }
    v10 = ((unsigned int)(11 * dir_y) >> 5) + (dir_z / 4) + dir_x;
    if ((signed int)v10 <= 768)
    {
        v11 = 5;
        if (pMapInfo->Trap_D20)
        {
            for (uint i = 0; i < pMapInfo->Trap_D20; ++i)
                v11 += rand() % 20 + 1;
        }
        switch (this->uType)
        {
        case 811:
            pDamageType = DMGT_FIRE;
            break;
        case 812:
            pDamageType = DMGT_ELECTR;
            break;
        case 813:
            pDamageType = DMGT_COLD;
            break;
        case 814:
            pDamageType = DMGT_BODY;
            break;
        default:
            return;
        }
        for (uint i = 1; i <= 4; ++i)
        {
            if (pPlayers[i]->CanAct() && (v13 = pPlayers[i]->GetPerception() + 20, rand() % v13 > 20))
                pPlayers[i]->PlaySound(SPEECH_6, 0);
            else
                pPlayers[i]->ReceiveDamage(v11, pDamageType);
        }
    }
}

void SpriteObject::OnInteraction(unsigned int uLayingItemID) {
  pSpriteObjects[uLayingItemID].uObjectDescID = 0;
  if (pParty->bTurnBasedModeOn == 1) {
    if (pSpriteObjects[uLayingItemID].uAttributes & 4) {
      pSpriteObjects[uLayingItemID].uAttributes &= 0xFFFB;  // ~0x00000004
      --pTurnEngine->pending_actions;
    }
  }
}

void CompactLayingItemsList() {
  int new_obj_pos = 0;

  for (int i = 0; i < MAX_SPRITE_OBJECTS; ++i) {
    if (pSpriteObjects[i].uObjectDescID) {
      if (i != new_obj_pos) {
        memcpy(&pSpriteObjects[new_obj_pos], &pSpriteObjects[i], sizeof(SpriteObject));
        pSpriteObjects[i].uObjectDescID = 0;
      }
      new_obj_pos++;
    }
  }
  uNumSpriteObjects = new_obj_pos;
}

void SpriteObject::InitializeSpriteObjects() {
  for (size_t i = 0; i < uNumSpriteObjects; ++i) {
    SpriteObject* item = &pSpriteObjects[i];
    if (item->uType && (item->uSoundID & 8 || pObjectList->pObjects[item->uType].uFlags & OBJECT_DESC_UNPICKABLE)) {
      SpriteObject::OnInteraction(i);
    }
  }
}

void SpriteObject::_46BEF1_apply_spells_aoe() {
  __debugbreak();//Ritor1

  if (uNumActors > 0) {
    for (size_t i = 0; i < uNumActors; ++i) {
      if (pActors[i].CanAct()) {
        int v7 = pActors[i].vPosition.x - this->vPosition.x;
        int v9 = pActors[i].vPosition.y - this->vPosition.y;
        int v10 = pActors[i].uActorHeight / 2 + pActors[i].vPosition.z - this->vVelocity.y;

        int v11 = this->vVelocity.x * this->vVelocity.x;

        if (v11 >= v7 * v7 + v9 * v9 + v10 * v10) {
          if (pActors[i].DoesDmgTypeDoDamage(DMGT_DARK)) {
            pActors[i].pActorBuffs[this->spell_id].Apply(
              pParty->GetPlayingTime() + GameTime::FromSeconds(this->spell_level),
              this->spell_skill, 4, 0, 0);
            pActors[i].uAttributes |= 0x80000;
          }
        }
      }
    }
  }
}


//----- (0042F7EB) --------------------------------------------------------
bool SpriteObject::sub_42F7EB_DropItemAt(SPRITE_OBJECT_TYPE sprite, int x, int y, int z, int a4, int count, int a7, unsigned __int16 attributes, ItemGen *a9)
{
    unsigned __int16 pObjectDescID; // ax@7
    SpriteObject pSpellObject; // [sp+Ch] [bp-78h]@1

    pSpellObject.containing_item.Reset();
    if (a9)
        memcpy(&pSpellObject.containing_item, a9, sizeof(pSpellObject.containing_item));
    pSpellObject.spell_skill = 0;
    pSpellObject.spell_level = 0;
    pSpellObject.spell_id = 0;
    pSpellObject.field_54 = 0;
    pSpellObject.uType = sprite;
    pObjectDescID = 0;
    for (uint i = 0; i < (signed int)pObjectList->uNumObjects; ++i)
    {
        if (sprite == pObjectList->pObjects[i].uObjectID)
            pObjectDescID = i;
    }
    pSpellObject.uObjectDescID = pObjectDescID;
    pSpellObject.vPosition.x = x;
    pSpellObject.vPosition.y = y;
    pSpellObject.vPosition.z = z;
    pSpellObject.uSoundID = 0;
    pSpellObject.uAttributes = attributes;
    pSpellObject.uSectorID = pIndoor->GetSector(x, y, z);
    pSpellObject.uSpriteFrameID = 0;
    pSpellObject.spell_caster_pid = 0;
    pSpellObject.spell_target_pid = 0;
    if (!(pSpellObject.uAttributes & 0x10))
    {
        if (pItemsTable->uAllItemsCount)
        {
            for (uint i = 1; i < pItemsTable->uAllItemsCount; ++i)
            {
                if (pItemsTable->pItems[i].uSpriteID == sprite)
                    pSpellObject.containing_item.uItemID = i;
            }
        }
    }
    if (a7)
    {
        if (count > 0)
        {
            for (uint i = count; i; --i)
            {
                pSpellObject.uFacing = rand() % (signed int)stru_5C6E00->uIntegerDoublePi;
                pSpellObject.Create((signed __int16)pSpellObject.uFacing,
                    ((signed int)stru_5C6E00->uIntegerHalfPi / 2) + (rand() % ((signed int)stru_5C6E00->uIntegerHalfPi / 2)), a4, 0);

            }
        }
    }
    else
    {
        pSpellObject.uFacing = 0;
        if (count > 0)
        {
            for (uint i = count; i; --i)
            {
                pSpellObject.Create((signed __int16)pSpellObject.uFacing, stru_5C6E00->uIntegerHalfPi, a4, 0);
            }
        }
    }
    return true;
}

//----- (0042F960) --------------------------------------------------------
void SpriteObject::sub_42F960_create_object(int x, int y, int z)
{
  unsigned __int16 v7; // ax@5
  signed int v8; // eax@6
  signed int v9; // eax@7

  SpriteObject a1; // [sp+Ch] [bp-70h]@1
  //SpriteObject::SpriteObject(&a1);
  a1.containing_item.Reset();

  a1.spell_skill = 0;
  a1.spell_level = 0;
  a1.spell_id = 0;
  a1.field_54 = 0;
  a1.uType = SPRITE_800;
  v7 = 0;
  for ( uint i = 0; i < (signed int)pObjectList->uNumObjects; ++i )
  {
    if ( a1.uType == pObjectList->pObjects[i].uObjectID  )
      v7 = i;
  }
  a1.uObjectDescID = v7;
  a1.vPosition.x = x;
  a1.vPosition.y = y;
  a1.vPosition.z = z;
  a1.uSoundID = 0;
  a1.uAttributes = 0;
  a1.uSectorID = pIndoor->GetSector(x, y, z);
  a1.uSpriteFrameID = 0;
  a1.spell_caster_pid = 0;
  a1.spell_target_pid = 0;
  v8 = a1.Create(0, 0, 0, 0);
  if ( v8 != -1 )
  {
    v9 = 8 * v8;
    v9 |= 2;
    pAudioPlayer->PlaySound((SoundID)SOUND_splash, v9, 0, -1, 0, 0, 0, 0);
  }
}



//----- (0046BFFA) --------------------------------------------------------
bool _46BFFA_update_spell_fx(unsigned int uLayingItemID, signed int a2)
{
    ObjectDesc *object; // ebx@1
    unsigned int v8; // eax@19
    signed int v10; // ebx@19
    char *v11; // edx@20
    unsigned __int16 v12; // ax@23
    int v13; // eax@27
    int v16; // eax@36
    __int16 v18; // di@37
    signed int v19; // edx@37
    unsigned __int16 v22; // ax@41
    signed int v24; // ebx@46
    char *v25; // edx@47
    signed int v34; // edx@65
    unsigned __int16 v36; // ax@69
    int v37; // ST14_4@72
    int v38; // eax@72
    int v39; // ST10_4@72
    int v40; // ST0C_4@72
    unsigned __int8 v44; // zf@79
    int v47; // eax@81
    signed int v52; // ebx@93
    signed int v56; // ebx@98
    unsigned __int16 v58; // ax@102
    unsigned __int16 v59; // ax@107
    unsigned __int16 v63; // ax@111
    int v64; // ebx@114
    signed int v65; // eax@114
    signed int v69; // ebx@124
    unsigned __int16 v71; // ax@128
    unsigned int v72; // ebx@131
    int v78; // eax@133
    signed int v81; // edx@140
    unsigned __int16 v83; // ax@144
    signed int v86; // ebx@151
    unsigned __int16 v88; // ax@155
    unsigned int v89; // eax@158
    int v90; // ST34_4@159
    int v91; // eax@159
    unsigned int v92; // eax@163
    unsigned __int16 v95; // ax@181
    unsigned __int16 v96; // ax@184
    int v97; // eax@185
    char v100; // ST18_1@198
    int v102; // eax@198
    signed int v106; // eax@208
    unsigned int v107; // edx@220
    signed int v108; // ebx@225
    signed int v110; // ebx@234
    unsigned __int16 v112; // ax@238
    unsigned __int16 v113; // si@241
    int v114; // eax@242
    int v115; // eax@245
    signed int v119; // ebx@251
    unsigned __int16 v121; // ax@255
    int v124; // eax@267
    char v132; // [sp-8h] [bp-34h]@131
    char v134; // [sp-4h] [bp-30h]@131
    signed int v135; // [sp-4h] [bp-30h]@217
    int v136; // [sp+Ch] [bp-20h]@208
    int v137; // [sp+10h] [bp-1Ch]@208
    signed int v138; // [sp+14h] [bp-18h]@207
    signed int v139; // [sp+18h] [bp-14h]@208
    signed int v141; // [sp+1Ch] [bp-10h]@117
    unsigned int v142; // [sp+1Ch] [bp-10h]@158
    signed int v143; // [sp+1Ch] [bp-10h]@172
    signed int v146; // [sp+20h] [bp-Ch]@60
    int v147; // [sp+20h] [bp-Ch]@72
    signed int v148; // [sp+20h] [bp-Ch]@158
    unsigned __int16 v150; // [sp+20h] [bp-Ch]@208
    signed int v152; // [sp+24h] [bp-8h]@208

    object = &pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID];
    //v151 = PID_TYPE(a2);
    if (PID_TYPE(a2) == OBJECT_Actor)
    {
        if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) == OBJECT_Actor
            && !pActors[PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid)].GetActorsRelation(&pActors[PID_ID(a2)]))
            return 1;
    }
    else
    {
        if (PID_TYPE(a2) == OBJECT_Player && PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) == OBJECT_Player)
            return 1;
    }
    if (pParty->bTurnBasedModeOn == 1)
    {
        if (pSpriteObjects[uLayingItemID].uAttributes & 4)
        {
            --pTurnEngine->pending_actions;
            pSpriteObjects[uLayingItemID].uAttributes &= 0xFFFB;//~0x00000004
        }
    }
    if (PID_TYPE(a2) == OBJECT_BModel && PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) != OBJECT_Player)
    {
        if (PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) < 500)  //bugfix  PID_ID(v2->spell_caster_pid)==1000
            pActors[PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid)].uAttributes |= ACTOR_UNKNOW5;
    }

    //v6 = v2->uType;
    //v7 = v2->uType;

    switch (pSpriteObjects[uLayingItemID].uType)
    {

    case SPRITE_SPELL_FIRE_FIRE_SPIKE:
    case SPRITE_SPELL_AIR_SPARKS:
    case SPRITE_SPELL_DARK_TOXIC_CLOUD:
    {
        //v9 = 0;
        if (PID_TYPE(a2) == 6 || PID_TYPE(a2) == 5 || !PID_TYPE(a2))
            return 1;
        if (PID_TYPE(a2) != 2)
        {
            sub_43A97E(uLayingItemID, a2);
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            v95 = 0;
            for (v52 = 0; v52 < (signed int)pObjectList->uNumObjects; ++v52)
            {
                if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v52].uObjectID)
                    v95 = v52;
            }
            pSpriteObjects[uLayingItemID].uObjectDescID = v95;
            if (!v95)
                SpriteObject::OnInteraction(uLayingItemID);
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            if (!pSpriteObjects[uLayingItemID].uSoundID) {
              v97 = 0;
            } else {
              v97 = (signed __int16)pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
//            v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//            pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v97, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id - 1, PID(OBJECT_Item, uLayingItemID));
            return 0;
        }
        pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
        v121 = 0;
        for (v119 = 0; v119 < (signed int)pObjectList->uNumObjects; ++v119)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v119].uObjectID)
                v121 = v119;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v121;
        if (!v121)
            SpriteObject::OnInteraction(uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        v13 = 8 * uLayingItemID;
        v13 |= PID(OBJECT_Item, uLayingItemID);
        pAudioPlayer->PlaySound(SOUND_fireBall, v13, 0, -1, 0, 0, 0, 0);
        return 0;
    }


    case SPRITE_PROJECTILE_500:
    case SPRITE_PROJECTILE_505:
    case SPRITE_PROJECTILE_510:
    case SPRITE_PROJECTILE_515:
    case SPRITE_PROJECTILE_520:
    case SPRITE_PROJECTILE_525:
    case SPRITE_PROJECTILE_530:
    case SPRITE_PROJECTILE_535:
    case SPRITE_PROJECTILE_540:
    {
        sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
        v12 = 0;
        for (v10 = 0; v10 < (signed int)pObjectList->uNumObjects; ++v10)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v10].uObjectID)
                v12 = v10;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v12;
        if (!v12)
            SpriteObject::OnInteraction(uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (pSpriteObjects[uLayingItemID].uType == 555)
        {
            v13 = 8 * uLayingItemID;
            v13 |= PID(OBJECT_Item, uLayingItemID);
            pAudioPlayer->PlaySound(SOUND_fireBall, v13, 0, -1, 0, 0, 0, 0);
        }
        return 0;
    }

    case SPRITE_PROJECTILE_545:
    case SPRITE_PROJECTILE_550:
    {
        if (pSpriteObjects[uLayingItemID].containing_item.uItemID != 405 && pSpriteObjects[uLayingItemID].containing_item.special_enchantment != 3)
        {
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            sub_43A97E(uLayingItemID, a2);
            SpriteObject::OnInteraction(uLayingItemID);
            if (pSpriteObjects[uLayingItemID].uSoundID == 0) {
              v16 = 0;
            } else {
              v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
//            v124 = 8 * uLayingItemID;
//            v124 |= 2;
//            v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id] + 1;
//            pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v16, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
            return 0;
        }
        v18 = 0;
        pSpriteObjects[uLayingItemID].uType = SPRITE_600;
        v22 = 0;
        for (v19 = 0; v19 < (signed int)pObjectList->uNumObjects; ++v19)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v19].uObjectID)
                v22 = v19;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v22;
        if (!v22)
            SpriteObject::OnInteraction(uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.z = v18;
        pSpriteObjects[uLayingItemID].vVelocity.y = v18;
        pSpriteObjects[uLayingItemID].vVelocity.x = v18;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = v18;
        v12 = 0;
        for (v10; v10 < (signed int)v8; ++v10)
        {
            v11 += 56;
            if (pSpriteObjects[uLayingItemID].uType != *(short *)v11)
                v12 = v10;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v12;
        if (!v12)
            SpriteObject::OnInteraction(uLayingItemID);
        v44 = pSpriteObjects[uLayingItemID].uType == 555;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (!v44)
        {
            v13 = 8 * uLayingItemID;
            v13 |= PID(OBJECT_Item, uLayingItemID);
            pAudioPlayer->PlaySound(SOUND_fireBall, v13, 0, -1, 0, 0, 0, 0);
            return 0;
        }
        return 0;
    }

    case SPRITE_600:
    {
        pSpriteObjects[uLayingItemID].uType = SPRITE_601;
        v36 = 0;
        for (v34 = 0; v34 < (signed int)pObjectList->uNumObjects; ++v34)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v34].uObjectID)
                v36 = v34;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v36;
        if (!v36)
            SpriteObject::OnInteraction(uLayingItemID);
        v37 = pSpriteObjects[uLayingItemID].vPosition.z;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        v38 = 8 * uLayingItemID;
        v39 = pSpriteObjects[uLayingItemID].vPosition.y;
        v38 |= PID(OBJECT_Item, uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        v40 = pSpriteObjects[uLayingItemID].vPosition.x;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        v147 = v38;
        AttackerInfo.Add(v38, 512, v40, v39, v37, 0, 0);
        if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, object->uParticleTrailColor);
        pAudioPlayer->PlaySound(SOUND_fireBall, v147, 0, -1, 0, 0, 0, 0);
        return 0;
    }

    case SPRITE_SPELL_FIRE_FIRE_BOLT:
    case SPRITE_SPELL_FIRE_INCINERATE:
    case SPRITE_SPELL_AIR_LIGHNING_BOLT:
    case SPRITE_SPELL_WATER_POISON_SPRAY:
    case SPRITE_SPELL_WATER_ICE_BOLT:
    case SPRITE_SPELL_WATER_ACID_BURST:
    case SPRITE_SPELL_EARTH_STUN:
    case SPRITE_SPELL_EARTH_DEADLY_SWARM:
    case SPRITE_SPELL_EARTH_BLADES:
    case SPRITE_SPELL_EARTH_MASS_DISTORTION:
    case SPRITE_SPELL_MIND_MIND_BLAST:
    case SPRITE_SPELL_MIND_PSYCHIC_SHOCK:
    {
        sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
        v95 = 0;
        for (v52 = 0; v52 < (signed int)pObjectList->uNumObjects; ++v52)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v52].uObjectID)
                v95 = v52;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v95;
        if (!v95)
            SpriteObject::OnInteraction(uLayingItemID);
        v96 = pSpriteObjects[uLayingItemID].uSoundID;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (!v96) {
          v97 = 0;
        } else {
          v97 = v96 + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= v124 | 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v97, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }


    case SPRITE_PROJECTILE_555:
    {
        sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = SPRITE_556;
        v18 = 0;
        v22 = 0;
        v25 = (char *)&pObjectList->pObjects->uObjectID;
        for (v24 = 0; v24 < (signed int)pObjectList->uNumObjects; ++v24)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v24].uObjectID)
                v22 = v24;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v22;
        if (v22 == v18)
            SpriteObject::OnInteraction(uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.z = v18;
        pSpriteObjects[uLayingItemID].vVelocity.y = v18;
        pSpriteObjects[uLayingItemID].vVelocity.x = v18;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = v18;
        return 0;
    }

    case SPRITE_SPELL_WATER_ICE_BLAST: {
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT;
        v63 = 0;
        for (unsigned int i = 0; i < pObjectList->uNumObjects; ++i) {
          if (v59 == pObjectList->pObjects[i].uObjectID) {
            v63 = i;
          }
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v63;
        if (!v63) {
          SpriteObject::OnInteraction(uLayingItemID);
        }
        v64 = pSpriteObjects[uLayingItemID].uFacing - stru_5C6E00->uIntegerDoublePi;
        v44 = pSpriteObjects[uLayingItemID].spell_skill == 4;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        v65 = 7;
        if (v44) {
          v65 = 9;
        }
        if (v65 > 0) {
          v141 = v65;
          do {
            v64 += (signed int)stru_5C6E00->uIntegerHalfPi / 2;
            pSpriteObjects[uLayingItemID].Create(v64, 0, 1000, 0);
            --v141;
          } while (v141);
        }
        SpriteObject::OnInteraction(uLayingItemID);
        if (!pSpriteObjects[uLayingItemID].uSoundID) {
          v16 = 0;
        } else {
          v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= v124 | 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v16, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    case SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT:
    {
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_WATER_ICE_BLAST_IMPACT;
        v58 = 0;
        for (v56 = 0; v56 < (signed int)pObjectList->uNumObjects; ++v56)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v56].uObjectID)
                v58 = v56;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v58;
        if (!v58)
            SpriteObject::OnInteraction(uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        sub_43A97E(uLayingItemID, a2);
        if (!pSpriteObjects[uLayingItemID].uSoundID) {
          v16 = 0;
        } else {
          v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= v124 | 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v16, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    case SPRITE_SPELL_EARTH_ROCK_BLAST:
    {
        if (PID_TYPE(a2) == 6 || PID_TYPE(a2) == 5 || !PID_TYPE(a2))
            return 1;
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT;
        v71 = 0;
        for (v69 = 0; v69 < (signed int)pObjectList->uNumObjects; ++v69)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v69].uObjectID)
                v71 = v69;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v71;
        if (!v71)
            SpriteObject::OnInteraction(uLayingItemID);
        v134 = 0;
        v72 = uLayingItemID;
        v132 = 0;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        AttackerInfo.Add(PID(OBJECT_Item, v72), 512, pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, v132, v134);
        if (!pSpriteObjects[uLayingItemID].uSoundID) {
          v78 = 0;
        } else {
          v78 = pSpriteObjects[uLayingItemID].uSoundID + 4;
        }
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, -1);  // Wrong PID for this spell!
        return 0;
    }

    case SPRITE_SPELL_EARTH_DEATH_BLOSSOM:
    {
        //v9 = 0;
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT;
        v88 = 0;
        for (v86 = 0; v86 < (signed int)pObjectList->uNumObjects; ++v86)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v86].uObjectID)
                v88 = v86;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v88;
        if (!v88)
            SpriteObject::OnInteraction(uLayingItemID);
        v89 = pSpriteObjects[uLayingItemID].uFacing - stru_5C6E00->uIntegerDoublePi;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        v142 = v89;
        v148 = 7;
        do
        {
            pRnd->SetRange(-128, 128);
            v90 = pRnd->GetInRange();
            pRnd->SetRange(5, 500);
            v91 = pRnd->GetInRange();
            v142 += (signed int)stru_5C6E00->uIntegerHalfPi >> 1;
            pSpriteObjects[uLayingItemID].Create(v90 + v142, 0, v91, 0);
            --v148;
        } while (v148);
        SpriteObject::OnInteraction(uLayingItemID);
        if (!pSpriteObjects[uLayingItemID].uSoundID) {
          v16 = 0;
        } else {
          v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v16, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT:
    {
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT;
        v83 = 0;
        for (v81 = 0; v81 < (signed int)pObjectList->uNumObjects; ++v81)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v81].uObjectID)
                v83 = v81;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v83;
        if (!v83)
            SpriteObject::OnInteraction(uLayingItemID);
        v134 = 0;
        //v72 = uLayingItemID;
        v132 = pSpriteObjects[uLayingItemID].field_61;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        AttackerInfo.Add(PID(OBJECT_Item, uLayingItemID), 512, pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, v132, v134);
        if (!pSpriteObjects[uLayingItemID].uSoundID)
            v78 = 0;
        else
            v78 = (signed __int16)pSpriteObjects[uLayingItemID].uSoundID + 4;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, pSpriteObjects[uLayingItemID].vPosition.x);  // Wrong pid???
        return 0;
    }

    case SPRITE_SPELL_LIGHT_DESTROY_UNDEAD:
    {
        if (PID_TYPE(a2) == 3
            && MonsterStats::BelongsToSupertype(pActors[PID_ID(a2)].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD))
            sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_LIGHT_DESTROY_UNDEAD_1;
        //v9 = 0;
        v95 = 0;
        for (v52 = 0; v52 < (signed int)pObjectList->uNumObjects; ++v52)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v52].uObjectID)
                v95 = v52;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v95;
        if (!v95)
            SpriteObject::OnInteraction(uLayingItemID);
        v96 = pSpriteObjects[uLayingItemID].uSoundID;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (!v96)
            v97 = 0;
        else
            v97 = (signed __int16)v96 + 4;
//        v92 = uLayingItemID;
//        v124 = 8 * v92;
//        v124 |= 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v97, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    case SPRITE_SPELL_BODY_HARM:
    case SPRITE_SPELL_BODY_FLYING_FIST:
    case SPRITE_SPELL_LIGHT_LIGHT_BOLT:
    case SPRITE_SPELL_LIGHT_SUNRAY:
    {
        sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
        v95 = 0;
        for (v52 = 0; v52 < (signed int)pObjectList->uNumObjects; ++v52)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v52].uObjectID)
                v95 = v52;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v95;
        if (!v95)
            SpriteObject::OnInteraction(uLayingItemID);
        v96 = pSpriteObjects[uLayingItemID].uSoundID;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (!v96) {
          v97 = 0;
        } else {
          v97 = v96 + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v97, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    case SPRITE_SPELL_MIND_CHARM:
    case SPRITE_SPELL_LIGHT_PARALYZE:
    case SPRITE_SPELL_DARK_SHRINKING_RAY:
    {
        v143 = 17030;
        switch (pSpriteObjects[uLayingItemID].uType) {
          case 0x1798u:
              v143 = 15040;
              break;
          case 0xFAAu:
              v143 = 13010;
              break;
          case 0x2346u:
              v143 = 18030;
              break;
        }
        v138 = 1;
        if (PID_TYPE(a2) != OBJECT_Actor)
        {
            if (pSpriteObjects[uLayingItemID].uType != 9030 || pSpriteObjects[uLayingItemID].spell_skill != 4)
            {
                SpriteObject::OnInteraction(uLayingItemID);
                return 0;
            }
            pSpriteObjects[uLayingItemID]._46BEF1_apply_spells_aoe();
            if (!v138)
            {
                pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
                v112 = 0;
                for (v110 = 0; v110 < (signed int)pObjectList->uNumObjects; ++v110)
                {
                    if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v110].uObjectID)
                        v112 = v110;
                }
                pSpriteObjects[uLayingItemID].uObjectDescID = v112;
                if (!v112)
                    SpriteObject::OnInteraction(uLayingItemID);
                pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
                v113 = pSpriteObjects[uLayingItemID].uSoundID;
                if (v113) {
                  v114 = v113 + 4;
                } else {
                  v114 = 0;
                }
//                v115 = 8 * uLayingItemID;
//                v115 |= PID(OBJECT_Item, uLayingItemID);
//                v125 = v143 + 1;
//                pAudioPlayer->PlaySound((SoundID)v125, v115, 0, -1, 0, v114, 0, 0);
                pAudioPlayer->PlaySpellSound(v143 + 1, PID(OBJECT_Item, uLayingItemID));
            }
            else
                SpriteObject::OnInteraction(uLayingItemID);
            return 0;
        }
        v106 = a2;
        v150 = 0;
        v139 = PID_ID(v106);
        v137 = pSpriteObjects[uLayingItemID].spell_level;
        v152 = pSpriteObjects[uLayingItemID].spell_skill;
        v136 = pSpriteObjects[uLayingItemID].spell_id;
        if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_DARK_SHRINKING_RAY)
        {
            v150 = 2;
            if (v152 == 2)
                v150 = 3;
            else
            {
                if (v152 >= 3)
                    v150 = 4;
            }
            pActors[v139].uAttributes |= ACTOR_AGGRESSOR;
            v107 = v135;
        }
        if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_MIND_CHARM)
        {
            v135 = 7;
            v107 = v135;
        }
        else
        {
            if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_LIGHT_PARALYZE)
            {
                v135 = 9;
                v107 = v135;
            }
            else
            {
                if (pSpriteObjects[uLayingItemID].uType != SPRITE_SPELL_DARK_SHRINKING_RAY)
                {
                    v107 = v136;
                }
                if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_DARK_SHRINKING_RAY)
                {
                    v135 = 10;
                    v107 = v135;
                }
            }
        }
        if (pSpriteObjects[uLayingItemID].uType != SPRITE_SPELL_DARK_SHRINKING_RAY || v152 != 4)
        {
            v108 = v139;
            if (pActors[v139].DoesDmgTypeDoDamage((DAMAGE_TYPE)v107))
            {
                v138 = 0;
                if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_LIGHT_PARALYZE)
                {
                    pActors[v108].uAIState = Standing;
                    pActors[v108].UpdateAnimation();
                }
                pActors[v108].pActorBuffs[v136].Apply(
                    pParty->GetPlayingTime() + GameTime::FromSeconds(v137),
                    v152, v150, 0, 0
                );
            }
        }
        else
        {
            pSpriteObjects[uLayingItemID]._46BEF1_apply_spells_aoe();
        }
        pSpriteObjects[uLayingItemID].spell_level = 0;
        pSpriteObjects[uLayingItemID].spell_skill = 0;
        pSpriteObjects[uLayingItemID].spell_id = 0;
        if (!v138)
        {
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            v112 = 0;
            for (v110 = 0; v110 < (signed int)pObjectList->uNumObjects; ++v110)
            {
                if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v110].uObjectID)
                    v112 = v110;
            }
            pSpriteObjects[uLayingItemID].uObjectDescID = v112;
            if (!v112)
                SpriteObject::OnInteraction(uLayingItemID);
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            v113 = pSpriteObjects[uLayingItemID].uSoundID;
            if (v113) {
              v114 = v113 + 4;
            } else {
              v114 = 0;
            }
//            v115 = 8 * uLayingItemID;
//            v115 |= PID(OBJECT_Item, uLayingItemID);
//            v125 = v143 + 1;
//            pAudioPlayer->PlaySound((SoundID)v125, v115, 0, -1, 0, v114, 0, 0);
            pAudioPlayer->PlaySpellSound(v143 + 1, PID(OBJECT_Item, uLayingItemID));
        }
        else
            SpriteObject::OnInteraction(uLayingItemID);
        return 0;
    }

    case SPRITE_SPELL_DARK_SHARPMETAL:
    {
        sub_43A97E(uLayingItemID, a2);
        pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_DARK_SHARPMETAL_IMPACT;
        v95 = 0;
        for (v52 = 0; v52 < (signed int)pObjectList->uNumObjects; ++v52)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v52].uObjectID)
                v95 = v52;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v95;
        if (!v95)
            SpriteObject::OnInteraction(uLayingItemID);
        v96 = pSpriteObjects[uLayingItemID].uSoundID;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        if (!v96) {
          v97 = 0;
        } else {
          v97 = v96 + 4;
        }
//        v124 = 8 * uLayingItemID;
//        v124 |= v124 | 2;
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v124, 0, -1, 0, v97, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, PID(OBJECT_Item, uLayingItemID));
        return 0;
    }

    /*
    case 1080:
    case 2100:
    {
    if (PID_TYPE(a2) != 3)
    {
    //v32 = 0;
    pSpriteObjects[uLayingItemID].uType = pSpriteObjects[uLayingItemID].uType + 1;
    v46 = 0;
    for (v146 = 0; v146 < (signed int)pObjectList->uNumObjects; ++v146)
    {
    if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v146].uObjectID)
    v46 = v146;
    }
    pSpriteObjects[uLayingItemID].uObjectDescID = v46;
    if (!v46)
    SpriteObject::OnInteraction(uLayingItemID);
    v100 = pSpriteObjects[uLayingItemID].field_61;
    pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
    v102 = 8 * uLayingItemID;
    LOBYTE(v102) = PID(OBJECT_Item, uLayingItemID);
    pSpriteObjects[uLayingItemID].vVelocity.x = 0;
    pSpriteObjects[uLayingItemID].vVelocity.y = 0;
    pSpriteObjects[uLayingItemID].vVelocity.z = 0;
    AttackerInfo.Add(v102, 512, pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, v100, 0);
    if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
    trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, object->uParticleTrailColor);
    if (!pSpriteObjects[uLayingItemID].uSoundID)
    v47 = 0;
    else
    v47 = (signed __int16)pSpriteObjects[uLayingItemID].uSoundID + 4;
    v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
    pAudioPlayer->PlaySound((SoundID)v125, v102, 0, -1, 0, v47, 0, 0);
    return 0;
    }
    return 1;
    }*/

    case SPRITE_SPELL_FIRE_METEOR_SHOWER:
    case SPRITE_SPELL_AIR_STARBURST:
    {
        if (PID_TYPE(a2) == 3)
            return 1;
        //else go to next case
    }

    case SPRITE_SPELL_FIRE_FIREBALL:
    case SPRITE_SPELL_DARK_DRAGON_BREATH:
    {
        v95 = 0;
        pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
        for (v146 = 0; v146 < (signed int)pObjectList->uNumObjects; ++v146)
        {
            if (pSpriteObjects[uLayingItemID].uType == pObjectList->pObjects[v146].uObjectID)
                v95 = v146;
        }
        pSpriteObjects[uLayingItemID].uObjectDescID = v95;
        if (!v95)
            SpriteObject::OnInteraction(uLayingItemID);
        v100 = pSpriteObjects[uLayingItemID].field_61;
        pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
        v102 = PID(OBJECT_Item, uLayingItemID);
        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
        AttackerInfo.Add(v102, 512, pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, v100, 0);
        if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, pSpriteObjects[uLayingItemID].vPosition.z, object->uParticleTrailColor);
        if (!pSpriteObjects[uLayingItemID].uSoundID) {
          v47 = 0;
        } else {
          v47 = pSpriteObjects[uLayingItemID].uSoundID + 4;
        }
//        v125 = word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] + 1;
//        pAudioPlayer->PlaySound((SoundID)v125, v102, 0, -1, 0, v47, 0, 0);
        pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].spell_id, v102);
        return 0;
    }

    default:
        return 0;
    }
}

//----- (0043A97E) --------------------------------------------------------
void sub_43A97E(unsigned int uLayingItemID, signed int a2)
{
    if (PID_TYPE(a2) == OBJECT_Player)
    {
        layingitem_vel_50FDFC.x = pSpriteObjects[uLayingItemID].vVelocity.x;
        layingitem_vel_50FDFC.y = pSpriteObjects[uLayingItemID].vVelocity.y;
        layingitem_vel_50FDFC.z = pSpriteObjects[uLayingItemID].vVelocity.z;

        Vec3_int_::Normalize(&layingitem_vel_50FDFC.x, &layingitem_vel_50FDFC.y, &layingitem_vel_50FDFC.z);
        DamagePlayerFromMonster(PID(OBJECT_Item, uLayingItemID), pSpriteObjects[uLayingItemID].field_61, &layingitem_vel_50FDFC, -1);
    }
    else if (PID_TYPE(a2) == OBJECT_Actor)
    {
        layingitem_vel_50FDFC.x = pSpriteObjects[uLayingItemID].vVelocity.x;
        layingitem_vel_50FDFC.y = pSpriteObjects[uLayingItemID].vVelocity.y;
        layingitem_vel_50FDFC.z = pSpriteObjects[uLayingItemID].vVelocity.z;

        Vec3_int_::Normalize(&layingitem_vel_50FDFC.x, &layingitem_vel_50FDFC.y, &layingitem_vel_50FDFC.z);
        switch (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid))
        {
        case OBJECT_Actor:
            Actor::ActorDamageFromMonster(PID(OBJECT_Item, uLayingItemID), PID_ID(a2), &layingitem_vel_50FDFC, pSpriteObjects[uLayingItemID].field_61);
            break;
        case OBJECT_Player:
            Actor::DamageMonsterFromParty(PID(OBJECT_Item, uLayingItemID), PID_ID(a2), &layingitem_vel_50FDFC);
            break;
        case OBJECT_Item:
            ItemDamageFromActor(PID(OBJECT_Item, uLayingItemID), PID_ID(a2), &layingitem_vel_50FDFC);
            break;
        }
    }
}
