#include "Engine/Objects/SpriteObject.h"

#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time.h"
#include "Engine/Events.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/stru298.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"

#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"
#include "Library/Random/Random.h"

using EngineIoc = Engine_::IocContainer;

// should be injected in SpriteObject but struct size cant be changed
static SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();
static std::shared_ptr<ParticleEngine> particle_engine = EngineIoc::ResolveParticleEngine();

std::vector<SpriteObject> pSpriteObjects;

int SpriteObject::Create(int yaw, int pitch, int speed, int which_char) {
    // check for valid sprite object
    if (!uObjectDescID) {
        return -1;
    }

    // find free sprite slot
    int sprite_slot = -1;
    for (unsigned int i = 0; i < pSpriteObjects.size(); ++i) {
        if (!pSpriteObjects[i].uObjectDescID) {
            sprite_slot = i;
            break;
        }
    }

    if (sprite_slot == -1) {
        sprite_slot = pSpriteObjects.size();
        pSpriteObjects.emplace_back();
    }

    // set initial position
    field_64.x = vPosition.x;
    field_64.y = vPosition.y;
    field_64.z = vPosition.z;

    // move sprite so it looks like it originates from char portrait
    switch (which_char) {
        case 0:
            break;  // do nothing
        case 1:
            Vec3i::Rotate((24/*<<16*/), 2048 - uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
            break;
        case 2:
            Vec3i::Rotate((8/*<<16*/), 2048 - uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
            break;
        case 3:
            Vec3i::Rotate((8/*<<16*/), 1024 - uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
            break;
        case 4:
            Vec3i::Rotate((24/*<<16*/), 1024 - uFacing, 0, vPosition, &vPosition.x, &vPosition.y, &vPosition.z);
            break;
        default:
            assert(false);
            return 0;
            break;
    }

    // set blank velocity
    vVelocity.y = 0;
    vVelocity.x = 0;
    vVelocity.z = 0;

    // calcualte angle velocity - could use rotate func here as above
    if (speed) {
        vVelocity.x = TrigLUT.Cos(yaw) * TrigLUT.Cos(pitch) * speed;
        vVelocity.y = TrigLUT.Sin(yaw) * TrigLUT.Cos(pitch) * speed;
        vVelocity.z = TrigLUT.Sin(pitch) * speed;
    }

    // copy sprite object into slot
    if (sprite_slot >= (int)pSpriteObjects.size()) {
        pSpriteObjects.resize(sprite_slot + 1);
    }
    pSpriteObjects[sprite_slot] = *this;
    return sprite_slot;
}

//----- (00471C03) --------------------------------------------------------
void SpriteObject::UpdateObject_fn0_ODM(unsigned int uLayingItemID) {
    int v6;              // eax@1
    int v7;              // ecx@1
    int v8;              // edi@1
    int v9;              // eax@4
    int v21;       // eax@41
    int i;  // edi@50
    int v26;       // edi@52
    int v27;       // eax@52
    int16_t v28;   // cx@55
    int v29;       // eax@55
    // signed int v30; // edi@59
    BSPModel *bmodel;  // ecx@61
    ODMFace *face;     // edi@61
    int v36;           // ecx@67
    int16_t v37;       // ax@67
    int v38;           // eax@72
    int v44;           // eax@77
    int v50;           // [sp+10h] [bp-98h]@52
    Vec3i v51;     // [sp+14h] [bp-94h]@11
    Particle_sw Dst;   // [sp+20h] [bp-88h]@45
    int v54;           // [sp+8Ch] [bp-1Ch]@1
    int v55;           // [sp+90h] [bp-18h]@1
    int v56;           // [sp+94h] [bp-14h]@11
    int v57;           // [sp+98h] [bp-10h]@1
    int v58;           // [sp+9Ch] [bp-Ch]@1
    int v60;           // [sp+A4h] [bp-4h]@11
    bool v49;

    v58 = 0;
    ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID];
    v57 = IsTerrainSlopeTooHigh(pSpriteObjects[uLayingItemID].vPosition.x,
                                pSpriteObjects[uLayingItemID].vPosition.y);
    v55 = 0;
    bool on_water = false;
    v6 = ODM_GetFloorLevel(pSpriteObjects[uLayingItemID].vPosition, object->uHeight, &on_water, &v55, 0);
    v7 = v6;
    v54 = v6;
    v8 = v6 + 1;
    if (pSpriteObjects[uLayingItemID].vPosition.z <= v6 + 1) {
        if (on_water) {
            v9 = v6 + 60;
            if (v55) v9 = v6 + 30;
            Create_Splash_Object(pSpriteObjects[uLayingItemID].vPosition.x,
                                     pSpriteObjects[uLayingItemID].vPosition.y,
                                     v9);
            SpriteObject::OnInteraction(uLayingItemID);
        }
    } else {
        v58 = 1;
    }
    if (!(object->uFlags & OBJECT_DESC_NO_GRAVITY)) {
        if (v58) {
            pSpriteObjects[uLayingItemID].vVelocity.z -=
                (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            goto LABEL_13;
        }
        if (v57) {
            pSpriteObjects[uLayingItemID].vPosition.z = v8;
            ODM_GetTerrainNormalAt(pSpriteObjects[uLayingItemID].vPosition.x,
                                   pSpriteObjects[uLayingItemID].vPosition.y,
                                   &v51);
            pSpriteObjects[uLayingItemID].vVelocity.z -=
                (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            v56 = abs(v51.y * pSpriteObjects[uLayingItemID].vVelocity.y +
                      v51.z * pSpriteObjects[uLayingItemID].vVelocity.z +
                      v51.x * pSpriteObjects[uLayingItemID].vVelocity.x) >>
                  16;
            // v60 = ((uint64_t)(v56 * (int64_t)v51.x) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.x +=
                fixpoint_mul(v56, v51.x);
            // v60 = ((uint64_t)(v56 * (int64_t)v51.y) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.y +=
                fixpoint_mul(v56, v51.y);
            // v60 = ((uint64_t)(v56 * (int64_t)v51.z) >> 16);
            pSpriteObjects[uLayingItemID].vVelocity.z +=
                fixpoint_mul(v56, v51.z);
            v7 = v54;
            goto LABEL_13;
        }
        if (object->uFlags & OBJECT_DESC_INTERACTABLE) {
            if (pSpriteObjects[uLayingItemID].vPosition.z < v7)
                pSpriteObjects[uLayingItemID].vPosition.z = v8;
            if (!_46BFFA_update_spell_fx(uLayingItemID, 0)) return;
        }
        pSpriteObjects[uLayingItemID].vPosition.z = v8;
        if (!(object->uFlags & OBJECT_DESC_BOUNCE) ||
            (v21 = -pSpriteObjects[uLayingItemID].vVelocity.z >> 1,
             pSpriteObjects[uLayingItemID].vVelocity.z = v21,
             (int16_t)v21 < 10))
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;

        pSpriteObjects[uLayingItemID].vVelocity.x =
            fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.x);
        pSpriteObjects[uLayingItemID].vVelocity.y =
            fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.y);
        pSpriteObjects[uLayingItemID].vVelocity.z =
            fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.z);
        if ((pSpriteObjects[uLayingItemID].vVelocity.y *
                 pSpriteObjects[uLayingItemID].vVelocity.y +
             pSpriteObjects[uLayingItemID].vVelocity.x *
                 pSpriteObjects[uLayingItemID].vVelocity.x) < 400) {
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            memset(&Dst, 0, sizeof(Particle_sw));
            Dst.x = (double)pSpriteObjects[uLayingItemID].vPosition.x;
            Dst.y = (double)pSpriteObjects[uLayingItemID].vPosition.y;
            Dst.z = (double)pSpriteObjects[uLayingItemID].vPosition.z;
            Dst.r = 0.0;
            Dst.g = 0.0;
            Dst.b = 0.0;
            if (object->uFlags & OBJECT_DESC_TRIAL_FIRE) {
                Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
                Dst.uDiffuse = colorTable.OrangeyRed.C32();
                Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                Dst.texture = spell_fx_renderer->effpar01;
                Dst.particle_size = 1.0f;
                particle_engine->AddParticle(&Dst);
            } else if (object->uFlags & OBJECT_DESC_TRIAL_LINE) {
                Dst.type = ParticleType_Line;
                Dst.uDiffuse = vrng->Random(RAND_MAX); // TODO(captainurist): TBH this makes no sense, investigate
                Dst.timeToLive = 64;
                Dst.texture = nullptr;
                Dst.particle_size = 1.0f;
                particle_engine->AddParticle(&Dst);
            } else if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE) {
                Dst.type = ParticleType_Bitmap | ParticleType_Ascending;
                Dst.uDiffuse = vrng->Random(RAND_MAX);
                Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                Dst.texture = spell_fx_renderer->effpar03;
                Dst.particle_size = 1.0f;
                particle_engine->AddParticle(&Dst);
            }
            return;
        }
    }
LABEL_13:
    if (pSpriteObjects[uLayingItemID].vPosition.x >= -0x8000 &&
            pSpriteObjects[uLayingItemID].vPosition.x <= 0x8000 &&
            pSpriteObjects[uLayingItemID].vPosition.y >= -0x8000 &&
            pSpriteObjects[uLayingItemID].vPosition.y <= 0x8000 &&
            pSpriteObjects[uLayingItemID].vPosition.z > v7 &&
            pSpriteObjects[uLayingItemID].vPosition.z <= 13000 ||
        !(object->uFlags & OBJECT_DESC_INTERACTABLE))
        goto LABEL_92;
    if (pSpriteObjects[uLayingItemID].vPosition.z < v7)
        pSpriteObjects[uLayingItemID].vPosition.z = v8;
    if (_46BFFA_update_spell_fx(uLayingItemID, 0)) {
    LABEL_92:
        collision_state.check_hi = false;
        collision_state.radius_lo = object->uRadius;
        collision_state.radius_hi = 0;
        collision_state.total_move_distance = 0;
        for (v55 = 0; v55 < 100; ++v55) {
            collision_state.position_hi.x = pSpriteObjects[uLayingItemID].vPosition.x;
            collision_state.position_lo.x = collision_state.position_hi.x;
            collision_state.uSectorID = 0;
            collision_state.position_hi.y = pSpriteObjects[uLayingItemID].vPosition.y;
            collision_state.position_lo.y = pSpriteObjects[uLayingItemID].vPosition.y;
            collision_state.position_hi.z = pSpriteObjects[uLayingItemID].vPosition.z +
                                     collision_state.radius_lo + 1;
            collision_state.position_lo.z = collision_state.position_hi.z;
            collision_state.velocity.x = pSpriteObjects[uLayingItemID].vVelocity.x;
            collision_state.velocity.y = pSpriteObjects[uLayingItemID].vVelocity.y;
            collision_state.velocity.z = pSpriteObjects[uLayingItemID].vVelocity.z;
            if (collision_state.PrepareAndCheckIfStationary(0))
                return;

            CollideOutdoorWithModels(false);
            CollideOutdoorWithDecorations(
                WorldPosToGridCellX(pSpriteObjects[uLayingItemID].vPosition.x),
                WorldPosToGridCellY(pSpriteObjects[uLayingItemID].vPosition.y));
            if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) !=
                OBJECT_Player)
                CollideWithParty(false);
            if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) ==
                OBJECT_Actor) {
                if ((PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) >=
                     0) &&
                    (PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) <
                     (signed int)(pActors.size() - 1))) {
                    for (v56 = 0; v56 < pActors.size(); ++v56) {
                        if (pActors[PID_ID(pSpriteObjects[uLayingItemID]
                                               .spell_caster_pid)]
                                .GetActorsRelation(&pActors[v56]))
                            CollideWithActor(v56, 0);
                    }
                }
            } else {
                for (i = 0; i < (signed int)pActors.size(); ++i)
                    CollideWithActor(i, 0);
            }
            v26 = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            v49 = false;
            v27 = ODM_GetFloorLevel(
                collision_state.new_position_lo.ToInt() - Vec3i(0, 0, collision_state.radius_lo + 1),
                object->uHeight, &v49, &v50, 0);
            if (on_water && v26 < v27 + 60) {
                if (v50)
                    v44 = v27 + 30;
                else
                    v44 = v54 + 60;
                Create_Splash_Object(  // splash
                    pSpriteObjects[uLayingItemID].vPosition.x,
                    pSpriteObjects[uLayingItemID].vPosition.y, v44);
                SpriteObject::OnInteraction(uLayingItemID);
                return;
            }
            if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
                pSpriteObjects[uLayingItemID].vPosition.x =
                    collision_state.new_position_lo.x;
                pSpriteObjects[uLayingItemID].vPosition.y =
                    collision_state.new_position_lo.y;
                pSpriteObjects[uLayingItemID].vPosition.z =
                    collision_state.new_position_lo.z - collision_state.radius_lo - 1;
                pSpriteObjects[uLayingItemID].uSectorID =
                    (short)collision_state.uSectorID;
                memset(&Dst, 0, sizeof(Particle_sw));
                Dst.x = (double)pSpriteObjects[uLayingItemID].vPosition.x;
                Dst.y = (double)pSpriteObjects[uLayingItemID].vPosition.y;
                Dst.z = (double)pSpriteObjects[uLayingItemID].vPosition.z;
                Dst.r = 0.0;
                Dst.g = 0.0;
                Dst.b = 0.0;
                if (object->uFlags & OBJECT_DESC_TRIAL_FIRE) {
                    Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
                    Dst.uDiffuse = colorTable.OrangeyRed.C32();
                    Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                    Dst.texture = spell_fx_renderer->effpar01;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                    return;
                } else if (object->uFlags & OBJECT_DESC_TRIAL_LINE) {
                    Dst.type = ParticleType_Line;
                    Dst.texture = nullptr;
                    Dst.uDiffuse = vrng->Random(RAND_MAX);
                    Dst.timeToLive = 64;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                    return;
                } else if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE) {
                    Dst.type = ParticleType_Bitmap | ParticleType_Ascending;
                    Dst.uDiffuse = vrng->Random(RAND_MAX);
                    Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                    Dst.texture = spell_fx_renderer->effpar03;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                }
                return;
            }
            // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.x) >> 16);
            pSpriteObjects[uLayingItemID].vPosition.x += collision_state.adjusted_move_distance * collision_state.direction.x;
            // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.y) >> 16);
            pSpriteObjects[uLayingItemID].vPosition.y += collision_state.adjusted_move_distance * collision_state.direction.y;
            // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.z) >> 16);
            v28 = (short)collision_state.uSectorID;
            pSpriteObjects[uLayingItemID].vPosition.z += collision_state.adjusted_move_distance * collision_state.direction.z;
            v29 = pSpriteObjects[uLayingItemID].vPosition.z;
            pSpriteObjects[uLayingItemID].uSectorID = v28;
            collision_state.total_move_distance += collision_state.adjusted_move_distance;
            if (object->uFlags & OBJECT_DESC_INTERACTABLE) {
                if (v29 < v54)
                    pSpriteObjects[uLayingItemID].vPosition.z = v54 + 1;
                if (!_46BFFA_update_spell_fx(uLayingItemID, collision_state.pid))
                    return;
            }
            if (PID_TYPE(collision_state.pid) == OBJECT_Decoration) break;
            if (PID_TYPE(collision_state.pid) == OBJECT_Face) {
                bmodel = &pOutdoor->pBModels[(signed int)collision_state.pid >> 9];
                face = &bmodel->pFaces[PID_ID(collision_state.pid) & 0x3F];
                if (face->uPolygonType == POLYGON_Floor) {
                    pSpriteObjects[uLayingItemID].vPosition.z =
                        bmodel->pVertices[face->pVertexIDs[0]].z + 1;
                    if (pSpriteObjects[uLayingItemID].vVelocity.x * pSpriteObjects[uLayingItemID].vVelocity.x +
                            pSpriteObjects[uLayingItemID].vVelocity.y * pSpriteObjects[uLayingItemID].vVelocity.y >= 400) {
                        if (face->uAttributes & FACE_TriggerByObject)
                            EventProcessor(face->sCogTriggeredID, 0, 1);
                    } else {
                        pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                        pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                        pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                    }
                } else {
                    v56 = abs(face->pFacePlaneOLD.vNormal.x * pSpriteObjects[uLayingItemID].vVelocity.x +
                              face->pFacePlaneOLD.vNormal.y * pSpriteObjects[uLayingItemID].vVelocity.y +
                              face->pFacePlaneOLD.vNormal.z * pSpriteObjects[uLayingItemID].vVelocity.z) >>
                          16;
                    if ((collision_state.speed / 8) > v56)
                        v56 = collision_state.speed / 8;
                    // v57 = fixpoint_mul(v56, face->pFacePlane.vNormal.x);
                    // v58 = fixpoint_mul(v56, face->pFacePlane.vNormal.y);
                    v60 = fixpoint_mul(v56, face->pFacePlaneOLD.vNormal.z);
                    pSpriteObjects[uLayingItemID].vVelocity.x += 2 * fixpoint_mul(v56, face->pFacePlaneOLD.vNormal.x);
                    pSpriteObjects[uLayingItemID].vVelocity.y += 2 * fixpoint_mul(v56, face->pFacePlaneOLD.vNormal.y);
                    if (face->pFacePlaneOLD.vNormal.z <= 32000) {
                        v37 = 2 * (short)v60;
                    } else {
                        v36 = v60;
                        pSpriteObjects[uLayingItemID].vVelocity.z += (int16_t)v60;
                        v58 = fixpoint_mul(0x7D00, v36);
                        v37 = fixpoint_mul(32000, v36);
                    }
                    pSpriteObjects[uLayingItemID].vVelocity.z += v37;
                    if (face->uAttributes & FACE_TriggerByObject)
                        EventProcessor(face->sCogTriggeredID, 0, 1);
                }
            }
        //LABEL_74:
            pSpriteObjects[uLayingItemID].vVelocity.x = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.x);
            pSpriteObjects[uLayingItemID].vVelocity.y = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.y);
            pSpriteObjects[uLayingItemID].vVelocity.z = fixpoint_mul(58500, pSpriteObjects[uLayingItemID].vVelocity.z);
        }
        v57 = integer_sqrt(pSpriteObjects[uLayingItemID].vVelocity.x * pSpriteObjects[uLayingItemID].vVelocity.x +
                           pSpriteObjects[uLayingItemID].vVelocity.y * pSpriteObjects[uLayingItemID].vVelocity.y);
        v38 = TrigLUT.Atan2(
            pSpriteObjects[uLayingItemID].vPosition.x - pLevelDecorations[PID_ID(collision_state.pid)].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y - pLevelDecorations[PID_ID(collision_state.pid)].vPosition.y);
        pSpriteObjects[uLayingItemID].vVelocity.x = TrigLUT.Cos(v38) * v57;
        pSpriteObjects[uLayingItemID].vVelocity.y = TrigLUT.Sin(v38 - TrigLUT.uIntegerHalfPi) * v57;
        //goto LABEL_74; // This goto results in an infinite loop, commented out.
    }
}

//----- (0047136C) --------------------------------------------------------
void SpriteObject::UpdateObject_fn0_BLV(unsigned int uLayingItemID) {
    SpriteObject *pSpriteObject = &pSpriteObjects[uLayingItemID];
    ObjectDesc *pObject = &pObjectList->pObjects[pSpriteObject->uObjectDescID];

    // Break early if we're out of bounds.
    if (abs(pSpriteObject->vPosition.x) > 32767 || abs(pSpriteObject->vPosition.y) > 32767 || abs(pSpriteObject->vPosition.z) > 20000) {
        SpriteObject::OnInteraction(uLayingItemID);
        return;
    }

    unsigned int uFaceID;
    unsigned int uSectorID = pSpriteObject->uSectorID; // TODO: this should go straight into GetIndoorFloorZ as a pointer.
    int floor_lvl = GetIndoorFloorZ(pSpriteObject->vPosition, &uSectorID, &uFaceID);
    if (floor_lvl <= -30000) {
        SpriteObject::OnInteraction(uLayingItemID);
        return;
    }
    pSpriteObject->uSectorID = uSectorID;

    int v15;               // ebx@46
    int v17;                      // eax@50
    int16_t v22;                  // ax@57
    int v23;                      // edi@62
    Particle_sw Dst;              // [sp+Ch] [bp-84h]@18

    int v39;                      // [sp+80h] [bp-10h]@33
    int v40;                      // [sp+84h] [bp-Ch]@28


    if (pObject->uFlags & OBJECT_DESC_NO_GRAVITY) {
        goto LABEL_25;
    }

    // flying objects / projectiles
    if (floor_lvl <= pSpriteObject->vPosition.z - 3) {
        pSpriteObject->vVelocity.z -= (short)pEventTimer->uTimeElapsed * GetGravityStrength();
LABEL_25:
        collision_state.check_hi = false;
        collision_state.radius_lo = pObject->uRadius;
        collision_state.ignored_face_id = -1;
        collision_state.radius_hi = 0;
        collision_state.total_move_distance = 0;
        for (int loop = 0; loop < 100; loop++) {
            collision_state.position_hi.x = pSpriteObject->vPosition.x;
            collision_state.position_hi.y = pSpriteObject->vPosition.y;
            collision_state.position_hi.z = collision_state.radius_lo + pSpriteObject->vPosition.z + 1;

            collision_state.position_lo.x = collision_state.position_hi.x;
            collision_state.position_lo.y = collision_state.position_hi.y;
            collision_state.position_lo.z = collision_state.position_hi.z;

            collision_state.velocity.x = pSpriteObject->vVelocity.x;
            collision_state.velocity.y = pSpriteObject->vVelocity.y;
            collision_state.velocity.z = pSpriteObject->vVelocity.z;

            collision_state.uSectorID = pSpriteObject->uSectorID;
            if (collision_state.PrepareAndCheckIfStationary(0)) return;

            for (int loop2 = 0; loop2 < 100; ++loop2) {
                CollideIndoorWithGeometry(false);
                CollideIndoorWithDecorations();

                if (PID_TYPE(pSpriteObject->spell_caster_pid) != OBJECT_Player)
                    CollideWithParty(true);

                for (int actloop = 0; actloop < (signed int)pActors.size(); ++actloop) {
                    // dont collide against self monster type
                    if (PID_TYPE(pSpriteObject->spell_caster_pid) == OBJECT_Actor) {
                        if (pActors[PID_ID(pSpriteObject->spell_caster_pid)].pMonsterInfo.uID == pActors[actloop].pMonsterInfo.uID) continue;
                    }

                        // not sure:
                        // pMonsterList->pMonsters[v39b->word_000086_some_monster_id-1].uToHitRadius
                        int radius = 0;
                        if (pActors[actloop].word_000086_some_monster_id) {  // not always filled in from scripted monsters
                            radius = pMonsterList->pMonsters[pActors[actloop].word_000086_some_monster_id - 1].uToHitRadius;
                        }
                        CollideWithActor(actloop, radius);
                }

                if (CollideIndoorWithPortals()) break;
            }
            // end loop2

            if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
                pSpriteObject->vPosition.x = collision_state.new_position_lo.x;
                pSpriteObject->vPosition.y = collision_state.new_position_lo.y;
                pSpriteObject->vPosition.z =
                    collision_state.new_position_lo.z - collision_state.radius_lo - 1;
                pSpriteObject->uSectorID = (short)collision_state.uSectorID;
                if (!(pObject->uFlags & 0x100)) return;
                memset(&Dst, 0, sizeof(Particle_sw));
                Dst.x = (double)pSpriteObject->vPosition.x;
                Dst.y = (double)pSpriteObject->vPosition.y;
                Dst.z = (double)pSpriteObject->vPosition.z;
                Dst.r = 0.0;
                Dst.g = 0.0;
                Dst.b = 0.0;
                if (pObject->uFlags & OBJECT_DESC_TRIAL_FIRE) {
                    Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
                    Dst.uDiffuse = colorTable.OrangeyRed.C32();
                    Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                    Dst.texture = spell_fx_renderer->effpar01;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                    return;
                } else if (pObject->uFlags & OBJECT_DESC_TRIAL_LINE) {
                    Dst.type = ParticleType_Line;
                    Dst.uDiffuse = vrng->Random(RAND_MAX);
                    Dst.timeToLive = 64;
                    Dst.texture = 0;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                    return;
                } else if (pObject->uFlags & OBJECT_DESC_TRIAL_PARTICLE) {
                    Dst.type = ParticleType_Bitmap | ParticleType_Ascending;
                    Dst.uDiffuse = vrng->Random(RAND_MAX);
                    Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                    Dst.texture = spell_fx_renderer->effpar03;
                    Dst.particle_size = 1.0f;
                    particle_engine->AddParticle(&Dst);
                }
                return;
            }
            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.x) >> 16;

            pSpriteObject->vPosition.x +=
                collision_state.adjusted_move_distance * collision_state.direction.x;

            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.y) >> 16;

            pSpriteObject->vPosition.y +=
                collision_state.adjusted_move_distance * collision_state.direction.y;

            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed
            // int64_t)collision_state.direction.z) >> 16;

            pSpriteObject->vPosition.z +=
                collision_state.adjusted_move_distance * collision_state.direction.z;

            pSpriteObject->uSectorID = collision_state.uSectorID;
            collision_state.total_move_distance += collision_state.adjusted_move_distance;

            // if weve collided but dont need to react return
            if (pObject->uFlags & OBJECT_DESC_INTERACTABLE &&
                !_46BFFA_update_spell_fx(uLayingItemID, collision_state.pid))
                return;

            v15 = (signed int)collision_state.pid >> 3;
            if (PID_TYPE(collision_state.pid) == OBJECT_Decoration) {
                v40 = integer_sqrt(
                    pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x +
                    pSpriteObject->vVelocity.y * pSpriteObject->vVelocity.y);
                v23 =
                    TrigLUT.Atan2(pSpriteObject->vPosition.x -
                                           pLevelDecorations[v15].vPosition.x,
                                       pSpriteObject->vPosition.y -
                                           pLevelDecorations[v15].vPosition.y);
                pSpriteObject->vVelocity.x = TrigLUT.Cos(v23) * v40;
                pSpriteObject->vVelocity.y = TrigLUT.Sin(v23) * v40;
            }
            if (PID_TYPE(collision_state.pid) == OBJECT_Face) {
                collision_state.ignored_face_id = PID_ID(collision_state.pid);
                if (pIndoor->pFaces[v15].uPolygonType != POLYGON_Floor) {
                    floor_lvl = abs(pIndoor->pFaces[v15].pFacePlane_old.vNormal.x *
                                  pSpriteObject->vVelocity.x +
                              pIndoor->pFaces[v15].pFacePlane_old.vNormal.y *
                                  pSpriteObject->vVelocity.y +
                              pIndoor->pFaces[v15].pFacePlane_old.vNormal.z *
                                  pSpriteObject->vVelocity.z) >>
                          16;
                    if ((collision_state.speed / 8) > floor_lvl)
                        floor_lvl = collision_state.speed / 8;
                    pSpriteObject->vVelocity.x +=
                        2 *
                        fixpoint_mul(
                            floor_lvl, pIndoor->pFaces[v15].pFacePlane_old.vNormal.x);
                    pSpriteObject->vVelocity.y +=
                        2 *
                        fixpoint_mul(
                            floor_lvl, pIndoor->pFaces[v15].pFacePlane_old.vNormal.y);
                    v39 = fixpoint_mul(
                        floor_lvl, pIndoor->pFaces[v15].pFacePlane_old.vNormal.z);
                    if (pIndoor->pFaces[v15].pFacePlane_old.vNormal.z <= 32000) {
                        v22 = 2 * v39;
                    } else {
                        pSpriteObject->vVelocity.z += v39;
                        v22 = fixpoint_mul(32000, v39);
                    }
                    pSpriteObject->vVelocity.z += v22;
                    if (pIndoor->pFaces[v15].uAttributes & FACE_TriggerByObject)
                        EventProcessor(
                            pIndoor
                                ->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID]
                                .uEventID,
                            0, 1);
                    pSpriteObject->vVelocity.x =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                if (pObject->uFlags & OBJECT_DESC_BOUNCE) {
                    v17 = -pSpriteObject->vVelocity.z / 2;
                    pSpriteObject->vVelocity.z = v17;
                    if ((int16_t)v17 < 10)
                        pSpriteObject->vVelocity.z = 0;
                    if (pIndoor->pFaces[v15].uAttributes & FACE_TriggerByObject)
                        EventProcessor(
                            pIndoor
                                ->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID]
                                .uEventID,
                            0, 1);
                    pSpriteObject->vVelocity.x =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                pSpriteObject->vVelocity.z = 0;
                if (pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x +
                        pSpriteObject->vVelocity.y *
                            pSpriteObject->vVelocity.y >=
                    400) {
                    if (pIndoor->pFaces[v15].uAttributes & FACE_TriggerByObject)
                        EventProcessor(
                            pIndoor
                                ->pFaceExtras[pIndoor->pFaces[v15].uFaceExtraID]
                                .uEventID,
                            0, 1);
                    pSpriteObject->vVelocity.x =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.x);
                    pSpriteObject->vVelocity.y =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.y);
                    pSpriteObject->vVelocity.z =
                        fixpoint_mul(58500, pSpriteObject->vVelocity.z);
                    continue;
                }
                pSpriteObject->vVelocity.z = 0;
                pSpriteObject->vVelocity.y = 0;
                pSpriteObject->vVelocity.x = 0;
                pSpriteObject->vPosition.z =
                    pIndoor->pVertices[*pIndoor->pFaces[v15].pVertexIDs].z + 1;
            }
            pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
            pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
            pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
        }
        // end loop
    }

    if (!(pObject->uFlags & OBJECT_DESC_INTERACTABLE) ||
        _46BFFA_update_spell_fx(uLayingItemID, 0)) {
        pSpriteObject->vPosition.z = floor_lvl + 1;
        if (pIndoor->pFaces[uFaceID].uPolygonType == POLYGON_Floor) {
            pSpriteObject->vVelocity.z = 0;
        } else {
            if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 45000)
                pSpriteObject->vVelocity.z -=
                    (short)pEventTimer->uTimeElapsed * GetGravityStrength();
        }
        pSpriteObject->vVelocity.x = fixpoint_mul(58500, pSpriteObject->vVelocity.x);
        pSpriteObject->vVelocity.y = fixpoint_mul(58500, pSpriteObject->vVelocity.y);
        pSpriteObject->vVelocity.z = fixpoint_mul(58500, pSpriteObject->vVelocity.z);
        if (pSpriteObject->vVelocity.x * pSpriteObject->vVelocity.x +
                pSpriteObject->vVelocity.y * pSpriteObject->vVelocity.y <
            400) {
            pSpriteObject->vVelocity.x = 0;
            pSpriteObject->vVelocity.y = 0;
            pSpriteObject->vVelocity.z = 0;
            if (!(pObject->uFlags & OBJECT_DESC_NO_SPRITE)) return;
            memset(&Dst, 0, sizeof(Particle_sw));
            Dst.x = (double)pSpriteObject->vPosition.x;
            Dst.y = (double)pSpriteObject->vPosition.y;
            Dst.z = (double)pSpriteObject->vPosition.z;
            Dst.r = 0.0;
            Dst.g = 0.0;
            Dst.b = 0.0;
            if (pObject->uFlags & OBJECT_DESC_TRIAL_FIRE) {
                Dst.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
                Dst.uDiffuse = colorTable.OrangeyRed.C32();
                Dst.particle_size = 1.0f;
                Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                Dst.texture = spell_fx_renderer->effpar01;
                particle_engine->AddParticle(&Dst);
                return;
            } else if (pObject->uFlags & OBJECT_DESC_TRIAL_LINE) {
                Dst.type = ParticleType_Line;
                Dst.uDiffuse = vrng->Random(RAND_MAX);
                Dst.timeToLive = 64;
                Dst.texture = nullptr;
                Dst.particle_size = 1.0f;
                particle_engine->AddParticle(&Dst);
                return;
            } else if (pObject->uFlags & OBJECT_DESC_TRIAL_PARTICLE) {
                Dst.type = ParticleType_Bitmap | ParticleType_Ascending;
                Dst.uDiffuse = vrng->Random(RAND_MAX);
                Dst.particle_size = 1.0f;
                Dst.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
                Dst.texture = spell_fx_renderer->effpar03;
                particle_engine->AddParticle(&Dst);
            }
            return;
        }
        goto LABEL_25;
    }
}

void SpriteObject::ExplosionTraps() {
    MapInfo *pMapInfo = &pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)];
    int dir_x = abs(pParty->vPosition.x - this->vPosition.x);
    int dir_y = abs(pParty->vPosition.y - this->vPosition.y);
    int dir_z = abs(pParty->vPosition.z + pParty->sEyelevel - this->vPosition.z);
    if (dir_x < dir_y) {
        std::swap(dir_x, dir_y);
    }
    if (dir_x < dir_z) {
        std::swap(dir_x, dir_z);
    }
    if (dir_y < dir_z) {
        std::swap(dir_y, dir_z);
    }
    unsigned int v10 = ((unsigned int)(11 * dir_y) >> 5) + (dir_z / 4) + dir_x;
    if (v10 <= 768) {
        int trapDamage = 5;
        if (pMapInfo->Trap_D20) {
            trapDamage += grng->RandomDice(pMapInfo->Trap_D20, 20);
        }
        DAMAGE_TYPE pDamageType;
        switch (this->uType) {
            case SPRITE_TRAP_FIRE:
                pDamageType = DMGT_FIRE;
                break;
            case SPRITE_TRAP_LIGHTNING:
                pDamageType = DMGT_ELECTR;
                break;
            case SPRITE_TRAP_COLD:
                pDamageType = DMGT_COLD;
                break;
            case SPRITE_TRAP_BODY:
                pDamageType = DMGT_BODY;
                break;
            default:
                return;
        }
        for (Player &player : pParty->pPlayers) {
            int perceptionCheckValue = player.GetPerception() + 20;
            // TODO(Nik-RE-dev): character that is asleep will not receive damage?
            if (player.CanAct() && (grng->Random(perceptionCheckValue) > 20)) {
                player.PlaySound(SPEECH_AvoidDamage, 0);
            } else {
                player.ReceiveDamage(trapDamage, pDamageType);
            }
        }
    }
}

unsigned int SpriteObject::GetLifetime() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pObjectDesc->uLifetime;
}

SpriteFrame *SpriteObject::GetSpriteFrame() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pSpriteFrameTable->GetFrame(pObjectDesc->uSpriteID, uSpriteFrameID);
}

bool SpriteObject::IsUnpickable() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return ((pObjectDesc->uFlags & OBJECT_DESC_UNPICKABLE) == OBJECT_DESC_UNPICKABLE);
}

bool SpriteObject::HasSprite() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return !pObjectDesc->NoSprite();
}

uint8_t SpriteObject::GetParticleTrailColorR() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pObjectDesc->uParticleTrailColorR;
}

uint8_t SpriteObject::GetParticleTrailColorG() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pObjectDesc->uParticleTrailColorG;
}

uint8_t SpriteObject::GetParticleTrailColorB() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pObjectDesc->uParticleTrailColorB;
}

void SpriteObject::OnInteraction(unsigned int uLayingItemID) {
    pSpriteObjects[uLayingItemID].uObjectDescID = 0;
    if (pParty->bTurnBasedModeOn) {
        if (pSpriteObjects[uLayingItemID].uAttributes & SPRITE_HALT_TURN_BASED) {
            pSpriteObjects[uLayingItemID].uAttributes &= ~SPRITE_HALT_TURN_BASED;
            --pTurnEngine->pending_actions;
        }
    }
}

void CompactLayingItemsList() {
    int new_obj_pos = 0;

    for (int i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (i != new_obj_pos) {
                pSpriteObjects[new_obj_pos] = pSpriteObjects[i];
                pSpriteObjects[i].uObjectDescID = 0;
            }
            new_obj_pos++;
        }
    }

    pSpriteObjects.resize(new_obj_pos);
}

void SpriteObject::InitializeSpriteObjects() {
    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        SpriteObject *item = &pSpriteObjects[i];
        if (item->uObjectDescID &&
            (item->uSoundID & 8 || pObjectList->pObjects[item->uObjectDescID].uFlags &
                                       OBJECT_DESC_UNPICKABLE)) {
            SpriteObject::OnInteraction(i);
        }
    }
}

void SpriteObject::_46BEF1_apply_spells_aoe() {
    __debugbreak();  // Ritor1

    for (size_t i = 0; i < pActors.size(); ++i) {
        if (pActors[i].CanAct()) {
            int v7 = pActors[i].vPosition.x - this->vPosition.x;
            int v9 = pActors[i].vPosition.y - this->vPosition.y;
            int v10 = pActors[i].uActorHeight / 2 + pActors[i].vPosition.z -
                      this->vVelocity.y;

            int v11 = this->vVelocity.x * this->vVelocity.x;

            if (v11 >= v7 * v7 + v9 * v9 + v10 * v10) {
                if (pActors[i].DoesDmgTypeDoDamage(DMGT_DARK)) {
                    pActors[i].pActorBuffs[ACTOR_BUFF_INDEX(this->uSpellID)].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(this->spell_level)),
                        this->spell_skill, 4, 0, 0);
                    pActors[i].uAttributes |= ACTOR_AGGRESSOR;
                }
            }
        }
    }
}

//----- (0042F7EB) --------------------------------------------------------
bool SpriteObject::Drop_Item_At(SPRITE_OBJECT_TYPE sprite, int x,
                                         int y, int z, int a4, int count,
                                         int a7, SPRITE_ATTRIBUTES attributes,
                                         ItemGen *a9) {
    SpriteObject pSpellObject;       // [sp+Ch] [bp-78h]@1

    pSpellObject.containing_item.Reset();
    if (a9)
        memcpy(&pSpellObject.containing_item, a9,
               sizeof(pSpellObject.containing_item));
    pSpellObject.spell_skill = PLAYER_SKILL_MASTERY_NONE;
    pSpellObject.spell_level = 0;
    pSpellObject.uSpellID = SPELL_NONE;
    pSpellObject.field_54 = 0;
    pSpellObject.uType = sprite;
    pSpellObject.uObjectDescID = pObjectList->ObjectIDByItemID(sprite);
    pSpellObject.vPosition.x = x;
    pSpellObject.vPosition.y = y;
    pSpellObject.vPosition.z = z;
    pSpellObject.uSoundID = 0;
    pSpellObject.uAttributes = attributes;
    pSpellObject.uSectorID = pIndoor->GetSector(x, y, z);
    pSpellObject.uSpriteFrameID = 0;
    pSpellObject.spell_caster_pid = 0;
    pSpellObject.spell_target_pid = 0;
    if (!(pSpellObject.uAttributes & SPRITE_IGNORE_RANGE)) {
        for (ITEM_TYPE i : pItemTable->pItems.indices()) {
            if (pItemTable->pItems[i].uSpriteID == sprite)
                pSpellObject.containing_item.uItemID = i;
        }
    }
    if (a7) {
        if (count > 0) {
            for (uint i = count; i; --i) {
                // Not sure if using grng is right here, but would rather err on the side of safety.
                pSpellObject.uFacing = grng->Random(TrigLUT.uIntegerDoublePi);
                pSpellObject.Create(
                    (int16_t)pSpellObject.uFacing,
                    ((int)TrigLUT.uIntegerHalfPi / 2) +
                        (grng->Random((signed int)TrigLUT.uIntegerHalfPi / 2)),
                    a4, 0);
            }
        }
    } else {
        pSpellObject.uFacing = 0;
        if (count > 0) {
            for (uint i = count; i; --i) {
                pSpellObject.Create((int16_t)pSpellObject.uFacing,
                                    TrigLUT.uIntegerHalfPi, a4, 0);
            }
        }
    }
    return true;
}

void SpriteObject::Create_Splash_Object(int x, int y, int z) {  // splash on water
    SpriteObject a1;
    a1.containing_item.Reset();
    a1.spell_skill = PLAYER_SKILL_MASTERY_NONE;
    a1.spell_level = 0;
    a1.uSpellID = SPELL_NONE;
    a1.field_54 = 0;
    a1.uType = SPRITE_WATER_SPLASH;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.vPosition.x = x;
    a1.vPosition.y = y;
    a1.vPosition.z = z;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(x, y, z);
    a1.uSpriteFrameID = 0;
    a1.spell_caster_pid = 0;
    a1.spell_target_pid = 0;
    int objID = a1.Create(0, 0, 0, 0);
    if (objID != -1) {
        pAudioPlayer->PlaySound((SoundID)SOUND_splash, PID(OBJECT_Item, objID), 0, 0, 0, 0);
    }
}

bool _46BFFA_update_spell_fx(unsigned int uLayingItemID, int pid) {
    int v90;                // ST34_4@159
    int v91;                // eax@159
    unsigned int v107 = 0;      // edx@220
    int v108;        // ebx@225
    // int v110;        // ebx@234
    int v114;               // eax@242
    int v135 = 0;        // [sp-4h] [bp-30h]@217
    int v136;               // [sp+Ch] [bp-20h]@208
    int v137;               // [sp+10h] [bp-1Ch]@208
    int v138;        // [sp+14h] [bp-18h]@207
    // int v141;        // [sp+1Ch] [bp-10h]@117
    uint16_t v150;  // [sp+20h] [bp-Ch]@208
    PLAYER_SKILL_MASTERY skillMastery;        // [sp+24h] [bp-8h]@208

    ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID];
    if (PID_TYPE(pid) == OBJECT_Actor) {
        if (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) == OBJECT_Actor &&
            !pActors[PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid)].GetActorsRelation(&pActors[PID_ID(pid)]))
            return 1;
    } else {
        if (PID_TYPE(pid) == OBJECT_Player &&
            PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) == OBJECT_Player)
            return 1;
    }
    if (pParty->bTurnBasedModeOn) {
        if (pSpriteObjects[uLayingItemID].uAttributes & SPRITE_HALT_TURN_BASED) {
            --pTurnEngine->pending_actions;
            pSpriteObjects[uLayingItemID].uAttributes &= ~SPRITE_HALT_TURN_BASED;
        }
    }
    if (PID_TYPE(pid) == OBJECT_Face &&
        PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid) != OBJECT_Player) {
        if (PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid) < 500)  // bugfix  PID_ID(v2->spell_caster_pid)==1000
            pActors[PID_ID(pSpriteObjects[uLayingItemID].spell_caster_pid)].uAttributes |= ACTOR_UNKNOW5;
    }

    switch (pSpriteObjects[uLayingItemID].uType) {
        case SPRITE_SPELL_FIRE_FIRE_SPIKE:
        case SPRITE_SPELL_AIR_SPARKS:
        case SPRITE_SPELL_DARK_TOXIC_CLOUD: {
            if (PID_TYPE(pid) == OBJECT_Face
                || PID_TYPE(pid) == OBJECT_Decoration
                || PID_TYPE(pid) == OBJECT_None) {
                return 1;
            }
            if (PID_TYPE(pid) != OBJECT_Item) {
                Apply_Spell_Sprite_Damage(uLayingItemID, pid);
                pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
                pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
                if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
                int v97 = 0;
                if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                    v97 = (int16_t)pSpriteObjects[uLayingItemID].uSoundID + 4;
                }
                //            v125 =
                //            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
                //            - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125,
                //            v124, 0, -1, 0, v97, 0, 0);
                // Originally used "spell_id - 1" but currently it will result in wrong sound on spell impact.
                pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                             PID(OBJECT_Item, uLayingItemID), true);
                return 0;
            }
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            pAudioPlayer->PlaySound(SOUND_fireBall, PID(OBJECT_Item, uLayingItemID), 0, -1, 0, 0);
            return 0;
        }

        case SPRITE_PROJECTILE_AIRBOLT:
        case SPRITE_PROJECTILE_EARTHBOLT:
        case SPRITE_PROJECTILE_FIREBOLT:
        case SPRITE_PROJECTILE_WATERBOLT:
        case SPRITE_PROJECTILE_520:
        case SPRITE_PROJECTILE_525:
        case SPRITE_PROJECTILE_530:
        case SPRITE_PROJECTILE_LIGHTBOLT:
        case SPRITE_PROJECTILE_DARKBOLT: {
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            if (pSpriteObjects[uLayingItemID].uType == SPRITE_BLASTER_PROJECTILE) {
                pAudioPlayer->PlaySound(SOUND_fireBall, PID(OBJECT_Item, uLayingItemID), 0, -1, 0, 0);
            }
            return 0;
        }

        case SPRITE_ARROW_PROJECTILE:
        case SPRITE_PROJECTILE_EXPLOSIVE: {
            // Note that ITEM_SPELLBOOK_FIREBALL is an MM6 remnant here,
            // in MM6 it was Percival artifact (id 405) which has swiftness and carnage enchantments
            if (pSpriteObjects[uLayingItemID].containing_item.uItemID != ITEM_SPELLBOOK_FIREBALL &&
                pSpriteObjects[uLayingItemID].containing_item.special_enchantment != ITEM_ENCHANTMENT_OF_CARNAGE) {
                pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
                Apply_Spell_Sprite_Damage(uLayingItemID, pid);
                SpriteObject::OnInteraction(uLayingItemID);
                int v16 = 0;
                if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                    v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                }
                //            v124 = 8 * uLayingItemID;
                //            v124 |= 2;
                //            v125 =
                //            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id]
                //            + 1; pAudioPlayer->PlaySound((SoundID)v125, v124,
                //            0, -1, 0, v16, 0, 0);
                pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                             PID(OBJECT_Item, uLayingItemID), true);
                return 0;
            }
            pSpriteObjects[uLayingItemID].uType = SPRITE_OBJECT_EXPLODE;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_OBJECT_EXPLODE);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            if (pSpriteObjects[uLayingItemID].uType != 555) {
                pAudioPlayer->PlaySound(SOUND_fireBall, PID(OBJECT_Item, uLayingItemID), 0, -1, 0, 0);
                return 0;
            }
            return 0;
        }

        case SPRITE_OBJECT_EXPLODE: {  // actor death explode
            pSpriteObjects[uLayingItemID].uType = SPRITE_OBJECT_EXPLODE_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_OBJECT_EXPLODE_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            AttackerInfo.Add(PID(OBJECT_Item, uLayingItemID), 512,
                             pSpriteObjects[uLayingItemID].vPosition.x,
                             pSpriteObjects[uLayingItemID].vPosition.y,
                             pSpriteObjects[uLayingItemID].vPosition.z,
                             ABILITY_ATTACK1, 0);
            if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE) {
                trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x,
                                                                pSpriteObjects[uLayingItemID].vPosition.y,
                                                                pSpriteObjects[uLayingItemID].vPosition.z,
                                                                object->uParticleTrailColor);
            }
            pAudioPlayer->PlaySound(SOUND_fireBall, PID(OBJECT_Item, uLayingItemID), 0, -1, 0, 0);
            return 0;
        }

        case SPRITE_SPELL_FIRE_FIRE_BOLT:
        case SPRITE_SPELL_FIRE_INCINERATE:
        case SPRITE_SPELL_AIR_LIGHTNING_BOLT:
        case SPRITE_SPELL_WATER_POISON_SPRAY:
        case SPRITE_SPELL_WATER_ICE_BOLT:
        case SPRITE_SPELL_WATER_ACID_BURST:
        case SPRITE_SPELL_EARTH_STUN:
        case SPRITE_SPELL_EARTH_DEADLY_SWARM:
        case SPRITE_SPELL_EARTH_BLADES:
        case SPRITE_SPELL_EARTH_MASS_DISTORTION:
        case SPRITE_SPELL_MIND_MIND_BLAST:
        case SPRITE_SPELL_MIND_PSYCHIC_SHOCK: {
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            int v97 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_BLASTER_PROJECTILE: {
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            pSpriteObjects[uLayingItemID].uType = SPRITE_BLASTER_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_BLASTER_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            return 0;
        }

        case SPRITE_SPELL_WATER_ICE_BLAST: {
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            int v65 = 7;
            if (pSpriteObjects[uLayingItemID].spell_skill == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                v65 = 9;
            }
            int v64 = pSpriteObjects[uLayingItemID].uFacing - TrigLUT.uIntegerDoublePi;
            for (int i = 0; i < v65; i++) {
                v64 += (int)TrigLUT.uIntegerHalfPi / 2;
                pSpriteObjects[uLayingItemID].Create(v64, 0, 1000, 0);
            }
            SpriteObject::OnInteraction(uLayingItemID);
            int v16 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT: {
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_WATER_ICE_BLAST_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BLAST_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            int v16 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_EARTH_ROCK_BLAST: {
            if (PID_TYPE(pid) == OBJECT_Face || PID_TYPE(pid) == OBJECT_Decoration || PID_TYPE(pid) == OBJECT_None) {
                return 1;
            }
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            AttackerInfo.Add(PID(OBJECT_Item, uLayingItemID), 512,
                             pSpriteObjects[uLayingItemID].vPosition.x,
                             pSpriteObjects[uLayingItemID].vPosition.y,
                             pSpriteObjects[uLayingItemID].vPosition.z,
                             ABILITY_ATTACK1, 0);
            int v78 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v78 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125,
            //        pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78,
            //        0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM: {
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            int v89 = pSpriteObjects[uLayingItemID].uFacing - TrigLUT.uIntegerDoublePi;
            for (int i = 0; i < 8; i++) {
                v90 = grng->RandomInSegment(-128, 128);
                v91 = grng->RandomInSegment(5, 500);
                v89 += TrigLUT.uIntegerHalfPi / 2;
                pSpriteObjects[uLayingItemID].Create(v90 + v89, 0, v91, 0);
            }
            SpriteObject::OnInteraction(uLayingItemID);
            int v16 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT: {
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            AttackerInfo.Add(PID(OBJECT_Item, uLayingItemID), 512,
                             pSpriteObjects[uLayingItemID].vPosition.x,
                             pSpriteObjects[uLayingItemID].vPosition.y,
                             pSpriteObjects[uLayingItemID].vPosition.z,
                             pSpriteObjects[uLayingItemID].field_61,
                             0);
            int v78 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v78 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125,
            //        pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78,
            //        0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_LIGHT_DESTROY_UNDEAD: {
            if (PID_TYPE(pid) == OBJECT_Actor &&
                MonsterStats::BelongsToSupertype(pActors[PID_ID(pid)].pMonsterInfo.uID,
                                                 MONSTER_SUPERTYPE_UNDEAD)) {
                                                 Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            }
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_LIGHT_DESTROY_UNDEAD_1;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_LIGHT_DESTROY_UNDEAD_1);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            int v97 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v92 = uLayingItemID;
            //        v124 = 8 * v92;
            //        v124 |= 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_BODY_HARM:
        case SPRITE_SPELL_BODY_FLYING_FIST:
        case SPRITE_SPELL_LIGHT_LIGHT_BOLT:
        case SPRITE_SPELL_LIGHT_SUNRAY: {
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            int v97 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        case SPRITE_SPELL_MIND_CHARM:
        case SPRITE_SPELL_LIGHT_PARALYZE:
        case SPRITE_SPELL_DARK_SHRINKING_RAY:  {
            // int v143 = 17030;
            // switch (pSpriteObjects[uLayingItemID].uType) {
            //     case SPRITE_SPELL_MIND_CHARM:
            //         v143 = 15040;
            //         break;
            //     case SPRITE_SPELL_EARTH_SLOW:
            //         v143 = 13010;
            //         break;
            //     case SPRITE_SPELL_DARK_SHRINKING_RAY:
            //         v143 = 18030;
            //         break;
            //     default:
            //         break;
            // }
            v138 = 1;
            if (PID_TYPE(pid) != OBJECT_Actor) {
                if (pSpriteObjects[uLayingItemID].uType != SPRITE_SPELL_DARK_SHRINKING_RAY ||
                    pSpriteObjects[uLayingItemID].spell_skill != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                    SpriteObject::OnInteraction(uLayingItemID);
                    return 0;
                }
                pSpriteObjects[uLayingItemID]._46BEF1_apply_spells_aoe();
                if (!v138) {
                    pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(
                        pSpriteObjects[uLayingItemID].uType + 1);
                    pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
                    if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                        SpriteObject::OnInteraction(uLayingItemID);
                    }
                    pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                    pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                    pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                    pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
                    int v114 = 0;
                    if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                        v114 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                    }
                    //                v115 = 8 * uLayingItemID;
                    //                v115 |= PID(OBJECT_Item, uLayingItemID);
                    //                v125 = v143 + 1;
                    //                pAudioPlayer->PlaySound((SoundID)v125,
                    //                v115, 0, -1, 0, v114, 0, 0);
                    pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                                 PID(OBJECT_Item, uLayingItemID), true);
                } else {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                return 0;
            }
            v150 = 0;
            v137 = pSpriteObjects[uLayingItemID].spell_level;
            skillMastery = pSpriteObjects[uLayingItemID].spell_skill;
            v136 = pSpriteObjects[uLayingItemID].uSpellID;
            if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_DARK_SHRINKING_RAY) {
                v150 = 2;
                if (skillMastery == PLAYER_SKILL_MASTERY_EXPERT) {
                    v150 = 3;
                } else {
                    if (skillMastery >= PLAYER_SKILL_MASTERY_MASTER) v150 = 4;
                }
                pActors[PID_ID(pid)].uAttributes |= ACTOR_AGGRESSOR;
                v107 = v135;
            }
            if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_MIND_CHARM) {
                v135 = 7;
                v107 = v135;
            } else {
                if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_LIGHT_PARALYZE) {
                    v135 = 9;
                    v107 = v135;
                } else {
                    if (pSpriteObjects[uLayingItemID].uType != SPRITE_SPELL_DARK_SHRINKING_RAY) {
                        v107 = v136;
                    }
                    if (pSpriteObjects[uLayingItemID].uType == SPRITE_SPELL_DARK_SHRINKING_RAY) {
                        v135 = 10;
                        v107 = v135;
                    }
                }
            }
            if (pSpriteObjects[uLayingItemID].uType != SPRITE_SPELL_DARK_SHRINKING_RAY || skillMastery != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                v108 = PID_ID(pid);
                if (pActors[PID_ID(pid)].DoesDmgTypeDoDamage((DAMAGE_TYPE)v107)) {
                    v138 = 0;
                    if (pSpriteObjects[uLayingItemID].uType ==
                        SPRITE_SPELL_LIGHT_PARALYZE) {
                        pActors[v108].uAIState = Standing;
                        pActors[v108].UpdateAnimation();
                    }
                    pActors[v108].pActorBuffs[ACTOR_BUFF_INDEX(v136)].Apply(
                        GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(v137)),
                        skillMastery, v150, 0, 0);
                }
            } else {
                pSpriteObjects[uLayingItemID]._46BEF1_apply_spells_aoe();
            }
            pSpriteObjects[uLayingItemID].spell_level = 0;
            pSpriteObjects[uLayingItemID].spell_skill = PLAYER_SKILL_MASTERY_NONE;
            pSpriteObjects[uLayingItemID].uSpellID = SPELL_NONE;
            if (!v138) {
                pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
                pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
                if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                pSpriteObjects[uLayingItemID].vVelocity.z = 0;
                pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
                v114 = 0;
                if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                    v114 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                }
                //            v115 = 8 * uLayingItemID;
                //            v115 |= PID(OBJECT_Item, uLayingItemID);
                //            v125 = v143 + 1;
                //            pAudioPlayer->PlaySound((SoundID)v125, v115, 0,
                //            -1, 0, v114, 0, 0);
                pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                             PID(OBJECT_Item, uLayingItemID), true);
            } else {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            return 0;
        }

        case SPRITE_SPELL_DARK_SHARPMETAL: {
            Apply_Spell_Sprite_Damage(uLayingItemID, pid);
            pSpriteObjects[uLayingItemID].uType = SPRITE_SPELL_DARK_SHARPMETAL_IMPACT;
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_DARK_SHARPMETAL_IMPACT);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            int v97 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

            /*
            case 1080:
            case 2100:
            {
            if (PID_TYPE(a2) != 3)
            {
            //v32 = 0;
            pSpriteObjects[uLayingItemID].uType =
            pSpriteObjects[uLayingItemID].uType + 1; v46 = 0; for (v146 = 0;
            v146 < (signed int)pObjectList->uNumObjects; ++v146)
            {
            if (pSpriteObjects[uLayingItemID].uType ==
            pObjectList->pObjects[v146].uObjectID) v46 = v146;
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
            AttackerInfo.Add(v102, 512,
            pSpriteObjects[uLayingItemID].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y,
            pSpriteObjects[uLayingItemID].vPosition.z, v100, 0); if
            (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y,
            pSpriteObjects[uLayingItemID].vPosition.z,
            object->uParticleTrailColor); if
            (!pSpriteObjects[uLayingItemID].uSoundID) v47 = 0; else v47 =
            (int16_t)pSpriteObjects[uLayingItemID].uSoundID + 4; v125 =
            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] +
            1; pAudioPlayer->PlaySound((SoundID)v125, v102, 0, -1, 0, v47, 0,
            0); return 0;
            }
            return 1;
            }*/

        case SPRITE_SPELL_FIRE_METEOR_SHOWER:
        case SPRITE_SPELL_AIR_STARBURST: {
            if (PID_TYPE(pid) == OBJECT_Actor) return 1;
            // else go to next case
        }

        case SPRITE_SPELL_FIRE_FIREBALL:
        case SPRITE_SPELL_DARK_DRAGON_BREATH: {
            pSpriteObjects[uLayingItemID].uType = (SPRITE_OBJECT_TYPE)(pSpriteObjects[uLayingItemID].uType + 1);
            pSpriteObjects[uLayingItemID].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[uLayingItemID].uType);
            if (pSpriteObjects[uLayingItemID].uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            pSpriteObjects[uLayingItemID].uSpriteFrameID = 0;
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            AttackerInfo.Add(PID(OBJECT_Item, uLayingItemID), 512,
                             pSpriteObjects[uLayingItemID].vPosition.x,
                             pSpriteObjects[uLayingItemID].vPosition.y,
                             pSpriteObjects[uLayingItemID].vPosition.z,
                             pSpriteObjects[uLayingItemID].field_61, 0);
            if (object->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
                trail_particle_generator.GenerateTrailParticles(
                    pSpriteObjects[uLayingItemID].vPosition.x,
                    pSpriteObjects[uLayingItemID].vPosition.y,
                    pSpriteObjects[uLayingItemID].vPosition.z,
                    object->uParticleTrailColor);
            int v47 = 0;
            if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                v47 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->PlaySound((SoundID)v125, v102, 0,
            //        -1, 0, v47, 0, 0);
            pAudioPlayer->PlaySpellSound(pSpriteObjects[uLayingItemID].uSpellID,
                                         PID(OBJECT_Item, uLayingItemID), true);
            return 0;
        }

        default:
            return 0;
    }
}

//----- (0043A97E) --------------------------------------------------------
void Apply_Spell_Sprite_Damage(unsigned int uLayingItemID, int pid) {
    if (PID_TYPE(pid) == OBJECT_Player) {
        layingitem_vel_50FDFC.x = pSpriteObjects[uLayingItemID].vVelocity.x;
        layingitem_vel_50FDFC.y = pSpriteObjects[uLayingItemID].vVelocity.y;
        layingitem_vel_50FDFC.z = pSpriteObjects[uLayingItemID].vVelocity.z;

        normalize_to_fixpoint(&layingitem_vel_50FDFC.x, &layingitem_vel_50FDFC.y,
                             &layingitem_vel_50FDFC.z);
        DamagePlayerFromMonster(PID(OBJECT_Item, uLayingItemID),
                                pSpriteObjects[uLayingItemID].field_61,
                                &layingitem_vel_50FDFC, -1);
    } else if (PID_TYPE(pid) == OBJECT_Actor) {
        layingitem_vel_50FDFC.x = pSpriteObjects[uLayingItemID].vVelocity.x;
        layingitem_vel_50FDFC.y = pSpriteObjects[uLayingItemID].vVelocity.y;
        layingitem_vel_50FDFC.z = pSpriteObjects[uLayingItemID].vVelocity.z;

        normalize_to_fixpoint(&layingitem_vel_50FDFC.x, &layingitem_vel_50FDFC.y,
                             &layingitem_vel_50FDFC.z);
        switch (PID_TYPE(pSpriteObjects[uLayingItemID].spell_caster_pid)) {
            case OBJECT_Actor:
                Actor::ActorDamageFromMonster(
                    PID(OBJECT_Item, uLayingItemID), PID_ID(pid),
                    &layingitem_vel_50FDFC,
                    pSpriteObjects[uLayingItemID].field_61);
                break;
            case OBJECT_Player:
                Actor::DamageMonsterFromParty(PID(OBJECT_Item, uLayingItemID),
                                              PID_ID(pid),
                                              &layingitem_vel_50FDFC);
                break;
            case OBJECT_Item:
                ItemDamageFromActor(PID(OBJECT_Item, uLayingItemID), PID_ID(pid),
                                    &layingitem_vel_50FDFC);
                break;
            default:
                break;
        }
    }
}

void UpdateObjects() {
    int v5;   // ecx@6
    int v7;   // eax@9
    int v11;  // eax@17
    int v12;  // edi@27
    int v18;  // [sp+4h] [bp-10h]@27
    int v19;  // [sp+8h] [bp-Ch]@27

    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uAttributes & SPRITE_SKIP_A_FRAME) {
            pSpriteObjects[i].uAttributes &= ~SPRITE_SKIP_A_FRAME;
        } else {
            ObjectDesc *object =
                &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
            if (pSpriteObjects[i].AttachedToActor()) {
                v5 = PID_ID(pSpriteObjects[i].spell_target_pid);
                if (v5 > pActors.size()) continue;
                pSpriteObjects[i].vPosition.x = pActors[v5].vPosition.x;
                pSpriteObjects[i].vPosition.y = pActors[v5].vPosition.y;
                pSpriteObjects[i].vPosition.z =
                    pActors[v5].vPosition.z + pActors[v5].uActorHeight;
                if (!pSpriteObjects[i].uObjectDescID) continue;
                pSpriteObjects[i].uSpriteFrameID += pEventTimer->uTimeElapsed;
                if (!(object->uFlags & OBJECT_DESC_TEMPORARY)) continue;
                if (pSpriteObjects[i].uSpriteFrameID >= 0) {
                    v7 = object->uLifetime;
                    if (pSpriteObjects[i].uAttributes & SPRITE_TEMPORARY)
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
                    if (pSpriteObjects[i].uAttributes & SPRITE_TEMPORARY)
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

unsigned int sub_46DEF2(signed int pid, unsigned int uLayingItemID) {
    unsigned int result = uLayingItemID;
    if (pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID].uFlags & 0x10) {
        result = _46BFFA_update_spell_fx(uLayingItemID, pid);
    }
    return result;
}
