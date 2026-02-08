#include "Engine/Objects/SpriteObject.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time/Timer.h"
#include "Engine/Evt/Processor.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/AttackList.h"
#include "Engine/MapInfo.h"

#include "Engine/Random/Random.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Objects/SpriteEnumFunctions.h"

#include "Engine/Tables/ItemTable.h"

#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"

// should be injected in SpriteObject but struct size cant be changed
static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();
static std::shared_ptr<ParticleEngine> particle_engine = EngineIocContainer::ResolveParticleEngine();

std::vector<SpriteObject> pSpriteObjects;

int SpriteObject::Create(int yaw, int pitch, int speed, int which_char) {
    // check for valid sprite object
    if (!uObjectDescID) {
        return -1;
    }

    // TODO(pskelton): refactor this so check isnt needed
    // To prevent memory corruption this function should never be called for any item in pSpriteObjects
    for (SpriteObject &ref : pSpriteObjects) {
        assert(&ref != this);
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
    initialPosition = vPosition;

    // set start timer for particle emmission
    _lastParticleTime = pMiscTimer->time();

    // move sprite so it looks like it originates from char portrait
    switch (which_char) {
        case 0:
            break;  // do nothing
        case 1:
            vPosition += Vec3f::fromPolar(24, uFacing + TrigLUT.uIntegerHalfPi, 0);
            break;
        case 2:
            vPosition += Vec3f::fromPolar(8, uFacing + TrigLUT.uIntegerHalfPi, 0);
            break;
        case 3:
            vPosition += Vec3f::fromPolar(8, uFacing - TrigLUT.uIntegerHalfPi, 0);
            break;
        case 4:
            vPosition += Vec3f::fromPolar(24, uFacing - TrigLUT.uIntegerHalfPi, 0);
            break;
        default:
            assert(false);
            return 0;
            break;
    }

    // set blank velocity
    vVelocity = Vec3f(0, 0, 0);

    // calcualte angle velocity
    if (speed) {
        vVelocity = Vec3f::fromPolar(speed, yaw, pitch);
    }

    // copy sprite object into slot
    if (sprite_slot >= (int)pSpriteObjects.size()) {
        pSpriteObjects.resize(sprite_slot + 1);
    }
    pSpriteObjects[sprite_slot] = *this;
    return sprite_slot;
}

static void createSpriteTrailParticle(Vec3f pos, ObjectDescFlags flags) {
    Particle_sw particle;
    memset(&particle, 0, sizeof(Particle_sw));
    particle.x = pos.x;
    particle.y = pos.y;
    particle.z = pos.z;
    if (flags & OBJECT_DESC_TRAIL_FIRE) {
        particle.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
        particle.uDiffuse = colorTable.OrangeyRed;
        particle.timeToLive = Duration::randomRealtimeSeconds(vrng, 1, 2); // was either 1 or 2 secs, we made it into [1, 2).
        particle.texture = spell_fx_renderer->effpar01;
        particle.particle_size = 1.0f;
        particle_engine->AddParticle(&particle);
    } else if (flags & OBJECT_DESC_TRAIL_LINE) {
        particle.type = ParticleType_Line;
        particle.uDiffuse = Color(vrng->random(0x100), vrng->random(0x100), 0, 0); // TODO(captainurist): TBH this makes no sense, investigate
        particle.timeToLive = 64_ticks;
        particle.texture = nullptr;
        particle.particle_size = 1.0f;
        particle_engine->AddParticle(&particle);
    } else if (flags & OBJECT_DESC_TRAIL_PARTICLE) {
        particle.type = ParticleType_Bitmap | ParticleType_Ascending;
        particle.uDiffuse = Color(vrng->random(0x100), vrng->random(0x100), 0, 0); // TODO(captainurist): TBH this makes no sense, investigate
        particle.timeToLive = Duration::randomRealtimeSeconds(vrng, 1, 2); // was either 1 or 2 secs, we made it into [1, 2).
        particle.texture = spell_fx_renderer->effpar03;
        particle.particle_size = 1.0f;
        particle_engine->AddParticle(&particle);
    }
}

void SpriteObject::updateObjectODM(unsigned int uLayingItemID) {
    ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID];
    bool isHighSlope = pOutdoor->pTerrain.isSlopeTooHighByPos(pSpriteObjects[uLayingItemID].vPosition);
    int bmodelPid = 0;
    bool onWater = false;
    float level = ODM_GetFloorLevel(pSpriteObjects[uLayingItemID].vPosition, &onWater, &bmodelPid);
    bool isAboveGround = pSpriteObjects[uLayingItemID].vPosition.z > level + 1;
    if (!isAboveGround && onWater) {
        int splashZ = level + 60;
        if (bmodelPid) {
            splashZ = level + 30;
        }
        createSplashObject(Vec3f(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, splashZ));
        SpriteObject::OnInteraction(uLayingItemID);
    }

    if (!(object->uFlags & OBJECT_DESC_NO_GRAVITY)) {
        if (isAboveGround) {
            pSpriteObjects[uLayingItemID].vVelocity.z -= pEventTimer->dt().ticks() * GetGravityStrength();
        } else if (isHighSlope) {
            Vec3f normf = pOutdoor->pTerrain.normalByPos(pSpriteObjects[uLayingItemID].vPosition);
            pSpriteObjects[uLayingItemID].vPosition.z = level + 1;
            pSpriteObjects[uLayingItemID].vVelocity.z -= (pEventTimer->dt().ticks() * GetGravityStrength());

            float dotp = std::abs(dot(normf, pSpriteObjects[uLayingItemID].vVelocity));
            pSpriteObjects[uLayingItemID].vVelocity += dotp * normf;
        } else {
            if (object->uFlags & OBJECT_DESC_INTERACTABLE) {
                if (pSpriteObjects[uLayingItemID].vPosition.z < level) {
                    pSpriteObjects[uLayingItemID].vPosition.z = level + 1;
                }
                if (!processSpellImpact(uLayingItemID, Pid())) {
                    return;
                }
            }
            pSpriteObjects[uLayingItemID].vPosition.z = level + 1;
            if (object->uFlags & OBJECT_DESC_BOUNCE) {
                float bounceZVel = -(pSpriteObjects[uLayingItemID].vVelocity.z / 2.0f);
                pSpriteObjects[uLayingItemID].vVelocity.z = bounceZVel;
                if (bounceZVel < 10.0f) {
                    pSpriteObjects[uLayingItemID].vVelocity.z = 0.0f;
                }
            } else {
                pSpriteObjects[uLayingItemID].vVelocity.z = 0.0f;
            }

            pSpriteObjects[uLayingItemID].vVelocity *= 0.89263916f; // was 58500 fp
            if (pSpriteObjects[uLayingItemID].vVelocity.xy().lengthSqr() < 400) {
                pSpriteObjects[uLayingItemID].vVelocity.x = 0;
                pSpriteObjects[uLayingItemID].vVelocity.y = 0;
                createSpriteTrailParticle(pSpriteObjects[uLayingItemID].vPosition, object->uFlags);
                return;
            }
        }
    }
    if (object->uFlags & OBJECT_DESC_INTERACTABLE) {
        if (std::abs(pSpriteObjects[uLayingItemID].vPosition.x) > 32768 ||
            std::abs(pSpriteObjects[uLayingItemID].vPosition.y) > 32768 ||
                pSpriteObjects[uLayingItemID].vPosition.z <= level ||
                pSpriteObjects[uLayingItemID].vPosition.z > 13000) {
            if (pSpriteObjects[uLayingItemID].vPosition.z < level) {
                pSpriteObjects[uLayingItemID].vPosition.z = level + 1;
            }
            if (!processSpellImpact(uLayingItemID, Pid())) {
                return;
            }
        }
    }

    // TODO(pskelton): move to collisions
    collision_state.check_hi = false;
    collision_state.radius_lo = object->uRadius;
    collision_state.radius_hi = 0;
    collision_state.total_move_distance = 0;
    for (int i = 0; i < 100; i++) {
        collision_state.uSectorID = 0;
        collision_state.position_lo = pSpriteObjects[uLayingItemID].vPosition + Vec3f(0, 0, collision_state.radius_lo + 1);
        collision_state.position_hi = collision_state.position_lo;
        collision_state.velocity = pSpriteObjects[uLayingItemID].vVelocity;
        if (collision_state.PrepareAndCheckIfStationary()) {
            return;
        }

        CollideOutdoorWithModels(false);
        CollideOutdoorWithDecorations(worldToGrid(pSpriteObjects[uLayingItemID].vPosition));
        ObjectType casterType = pSpriteObjects[uLayingItemID].spell_caster_pid.type();
        if (casterType != OBJECT_Character) {
            CollideWithParty(false);
        }
        if (casterType == OBJECT_Actor) {
            int actorId = pSpriteObjects[uLayingItemID].spell_caster_pid.id();
            // TODO: why pActors.size() - 1? Should just check for .size()
            if ((actorId >= 0) && (actorId < (pActors.size() - 1))) {
                for (int j = 0; j < pActors.size(); ++j) {
                    if (pActors[actorId].GetActorsRelation(&pActors[j]) != HOSTILITY_FRIENDLY) {
                        CollideWithActor(j, 0);
                    }
                }
            }
        } else {
            for (int j = 0; j < pActors.size(); ++j) {
                CollideWithActor(j, 0);
            }
        }
        int collisionZ = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
        bool collisionOnWater = false;
        int collisionBmodelPid = 0;
        Vec3f collisionPos = collision_state.new_position_lo - Vec3f(0, 0, collision_state.radius_lo + 1);
        float collisionLevel = ODM_GetFloorLevel(collisionPos, &collisionOnWater, &collisionBmodelPid);
        // TOOD(Nik-RE-dev): why initail "onWater" is used?
        if (onWater && collisionZ < (collisionLevel + 60)) {
            int splashZ = level + 60;
            if (collisionBmodelPid) {
                splashZ = collisionLevel + 30;
            }
            createSplashObject(Vec3f(pSpriteObjects[uLayingItemID].vPosition.x, pSpriteObjects[uLayingItemID].vPosition.y, splashZ));
            SpriteObject::OnInteraction(uLayingItemID);
            return;
        }
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            pSpriteObjects[uLayingItemID].vPosition = (collision_state.new_position_lo - Vec3f(0, 0, collision_state.radius_lo + 1));
            //pSpriteObjects[uLayingItemID].vPosition.x = collision_state.new_position_lo.x;
            //pSpriteObjects[uLayingItemID].vPosition.y = collision_state.new_position_lo.y;
            //pSpriteObjects[uLayingItemID].vPosition.z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            pSpriteObjects[uLayingItemID].uSectorID = collision_state.uSectorID;
            createSpriteTrailParticle(pSpriteObjects[uLayingItemID].vPosition, object->uFlags);
            return;
        }
        // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.x) >> 16);
        // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.y) >> 16);
        // v60 = ((uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.z) >> 16);
        Vec3f delta = collision_state.direction * collision_state.adjusted_move_distance;
        pSpriteObjects[uLayingItemID].vPosition += delta;
        pSpriteObjects[uLayingItemID].uSectorID = collision_state.uSectorID;
        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        if (object->uFlags & OBJECT_DESC_INTERACTABLE) {
            if (pSpriteObjects[uLayingItemID].vPosition.z < level) {
                pSpriteObjects[uLayingItemID].vPosition.z = level + 1;
            }
            if (!processSpellImpact(uLayingItemID, collision_state.pid)) {
                return;
            }
        }

        if (collision_state.pid.type() == OBJECT_Decoration) {
            Vec2f deltaXY = pSpriteObjects[uLayingItemID].vPosition.xy() - pLevelDecorations[collision_state.pid.id()].vPosition.xy();
            float velLenXY = pSpriteObjects[uLayingItemID].vVelocity.xy().length();
            float velRotXY = atan2(deltaXY.x, deltaXY.y);
            pSpriteObjects[uLayingItemID].vVelocity.x = cos(velRotXY) * velLenXY;
            pSpriteObjects[uLayingItemID].vVelocity.y = sin(velRotXY) * velLenXY;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            const BSPModel *bmodel = &pOutdoor->model(collision_state.pid);
            const ODMFace *face = &pOutdoor->face(collision_state.pid);
            if (face->polygonType == POLYGON_Floor) {
                pSpriteObjects[uLayingItemID].vPosition.z = bmodel->vertices[face->vertexIds[0]].z + 1;
                if (pSpriteObjects[uLayingItemID].vVelocity.xy().lengthSqr() >= 400) {
                    if (face->attributes & FACE_TriggerByObject) {
                        eventProcessor(face->eventId, Pid(), 1);
                    }
                } else {
                    pSpriteObjects[uLayingItemID].vVelocity = Vec3f(0, 0, 0);
                }
            } else {
                float dotFix = std::abs(dot(face->facePlane.normal, pSpriteObjects[uLayingItemID].vVelocity));
                dotFix = std::max(dotFix, collision_state.speed / 8);
                float newZVel = dotFix * face->facePlane.normal.z;
                pSpriteObjects[uLayingItemID].vVelocity.x += 2 * dotFix * face->facePlane.normal.x;
                pSpriteObjects[uLayingItemID].vVelocity.y += 2 * dotFix * face->facePlane.normal.y;
                if (face->facePlane.normal.z <= 0.48828125f) { // was 32000 fixpoint, 32000/65536=0.488...
                    newZVel = 2 * newZVel;
                } else {
                    pSpriteObjects[uLayingItemID].vVelocity.z += newZVel;
                    newZVel = 0.48828125f * newZVel;
                }
                pSpriteObjects[uLayingItemID].vVelocity.z += newZVel;
                if (face->attributes & FACE_TriggerByObject) {
                    eventProcessor(face->eventId, Pid(), 1);
                }
            }
        }
        pSpriteObjects[uLayingItemID].vVelocity *= 0.89263916f; // was 58500 fp
    }
}

//----- (0047136C) --------------------------------------------------------
void SpriteObject::updateObjectBLV(unsigned int uLayingItemID) {
    SpriteObject *pSpriteObject = &pSpriteObjects[uLayingItemID];
    ObjectDesc *pObject = &pObjectList->pObjects[pSpriteObject->uObjectDescID];

    // Break early if we're out of bounds.
    if (std::abs(pSpriteObject->vPosition.x) > 32767 ||
        std::abs(pSpriteObject->vPosition.y) > 32767 ||
        std::abs(pSpriteObject->vPosition.z) > 20000) {
        SpriteObject::OnInteraction(uLayingItemID);
        return;
    }

    int uFaceID;
    float floor_lvl = GetIndoorFloorZ(pSpriteObject->vPosition, &pSpriteObject->uSectorID, &uFaceID);
    if (floor_lvl <= -30000) {
        SpriteObject::OnInteraction(uLayingItemID);
        return;
    }

    if (pObject->uFlags & OBJECT_DESC_NO_GRAVITY) {
        goto LABEL_25;
    }

    // flying objects / projectiles
    if (floor_lvl <= pSpriteObject->vPosition.z - 3) {
        pSpriteObject->vVelocity.z -= pEventTimer->dt().ticks() * GetGravityStrength();
        // TODO(Nik-RE-dev): get rid of goto here
        // TODO(pskelton): move to Collisions
LABEL_25:
        collision_state.check_hi = false;
        collision_state.radius_lo = pObject->uRadius;
        collision_state.radius_hi = 0;
        collision_state.total_move_distance = 0;
        for (int loop = 0; loop < 100; loop++) {
            collision_state.position_hi = pSpriteObject->vPosition + Vec3f(0, 0, collision_state.radius_lo + 1);
            collision_state.position_lo = collision_state.position_hi;
            collision_state.velocity = pSpriteObject->vVelocity;
            collision_state.uSectorID = pSpriteObject->uSectorID;
            if (collision_state.PrepareAndCheckIfStationary()) {
                return;
            }

            // TODO(Nik-RE-dev): check purpose of inner loop
            for (int loop2 = 0; loop2 < 100; ++loop2) {
                CollideIndoorWithGeometry(false);
                CollideIndoorWithDecorations();

                if (pSpriteObject->spell_caster_pid.type() != OBJECT_Character) {
                    CollideWithParty(true);
                }

                for (int actloop = 0; actloop < (signed int)pActors.size(); ++actloop) {
                    // dont collide against self monster type
                    if (pSpriteObject->spell_caster_pid.type() == OBJECT_Actor) {
                        if (pActors[pSpriteObject->spell_caster_pid.id()].monsterInfo.id == pActors[actloop].monsterInfo.id) {
                            continue;
                        }
                    }

                    // not sure:
                    // pMonsterList->pMonsters[v39b->monsterId-1].uToHitRadius
                    int radius = 0;
                    if (pActors[actloop].monsterId != MONSTER_INVALID) {  // not always filled in from scripted monsters
                        radius = pMonsterList->monsters[pActors[actloop].monsterId].toHitRadius;
                    }
                    CollideWithActor(actloop, radius);
                }

                if (CollideIndoorWithPortals()) {
                    break;
                }
            }
            // end loop2

            if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
                pSpriteObject->vPosition = (collision_state.new_position_lo - Vec3f(0, 0, collision_state.radius_lo + 1));
                pSpriteObject->uSectorID = collision_state.uSectorID;
                if (!(pObject->uFlags & OBJECT_DESC_TRAIL_PARTICLE)) {
                    return;
                }
                createSpriteTrailParticle(pSpriteObject->vPosition, pObject->uFlags);
                return;
            }
            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.x) >> 16;
            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.y) >> 16;
            // v40 = (uint64_t)(collision_state.adjusted_move_distance * (signed int64_t)collision_state.direction.z) >> 16;

            Vec3f delta = collision_state.direction * collision_state.adjusted_move_distance;
            pSpriteObject->vPosition += delta;
            pSpriteObject->uSectorID = collision_state.uSectorID;
            collision_state.total_move_distance += collision_state.adjusted_move_distance;

            // if weve collided but dont need to react return
            if ((pObject->uFlags & OBJECT_DESC_INTERACTABLE) &&
                !processSpellImpact(uLayingItemID, collision_state.pid)) {
                return;
            }

            int pidId = collision_state.pid.id();
            if (collision_state.pid.type() == OBJECT_Decoration) {
                Vec2f deltaXY = pSpriteObject->vPosition.xy() - pLevelDecorations[pidId].vPosition.xy();
                float velXYLen = pSpriteObject->vVelocity.xy().length();
                float velXYRot = atan2(deltaXY.x, deltaXY.y);
                pSpriteObject->vVelocity.x = cos(velXYRot) * velXYLen;
                pSpriteObject->vVelocity.y = sin(velXYRot) * velXYLen;
            }

            if (collision_state.pid.type() == OBJECT_Face) {
                if (pIndoor->faces[pidId].polygonType != POLYGON_Floor) {
                    // Before this variable changed floor_lvl variable which is obviously invalid.
                    float dotFix = std::abs(dot(pIndoor->faces[pidId].facePlane.normal, pSpriteObject->vVelocity));
                    dotFix = std::max(dotFix, collision_state.speed / 8);
                    pSpriteObject->vVelocity.x += 2 * dotFix * pIndoor->faces[pidId].facePlane.normal.x;
                    pSpriteObject->vVelocity.y += 2 * dotFix * pIndoor->faces[pidId].facePlane.normal.y;
                    float newZVel = dotFix * pIndoor->faces[pidId].facePlane.normal.z;
                    if (pIndoor->faces[pidId].facePlane.normal.z <= 0.48828125f) { // was 32000 fixpoint
                        newZVel = 2 * newZVel;
                    } else {
                        pSpriteObject->vVelocity.z += newZVel;
                        newZVel = 0.48828125f * newZVel;
                    }
                    pSpriteObject->vVelocity.z += newZVel;
                    if (pIndoor->faces[pidId].attributes & FACE_TriggerByObject) {
                        eventProcessor(pIndoor->faceExtras[pIndoor->faces[pidId].faceExtraId].uEventID, Pid(), 1);
                    }
                    pSpriteObject->vVelocity *= 0.89263916f; // was 58500 fp
                    continue;
                }
                if (pObject->uFlags & OBJECT_DESC_BOUNCE) {
                    pSpriteObject->vVelocity.z = -pSpriteObject->vVelocity.z / 2;
                    if (pSpriteObject->vVelocity.z < 10) {
                        pSpriteObject->vVelocity.z = 0;
                    }
                    if (pIndoor->faces[pidId].attributes & FACE_TriggerByObject) {
                        eventProcessor(pIndoor->faceExtras[pIndoor->faces[pidId].faceExtraId].uEventID, Pid(), 1);
                    }
                    pSpriteObject->vVelocity *= 0.89263916f; // was 58500 fp
                    continue;
                }
                pSpriteObject->vVelocity.z = 0;
                if (pSpriteObject->vVelocity.xy().lengthSqr() >= 400) {
                    if (pIndoor->faces[pidId].attributes & FACE_TriggerByObject) {
                        eventProcessor(pIndoor->faceExtras[pIndoor->faces[pidId].faceExtraId].uEventID, Pid(), 1);
                    }
                    pSpriteObject->vVelocity *= 0.89263916f; // was 58500 fp
                    continue;
                }
                pSpriteObject->vVelocity = Vec3f(0, 0, 0);
                pSpriteObject->vPosition.z = pIndoor->vertices[*pIndoor->faces[pidId].vertexIds].z + 1;
            }
            pSpriteObject->vVelocity *= 0.89263916f; // was 58500 fp
        }
        // end loop
    }

    if (!(pObject->uFlags & OBJECT_DESC_INTERACTABLE) || processSpellImpact(uLayingItemID, Pid())) {
        pSpriteObject->vPosition.z = floor_lvl + 1;
        if (pIndoor->faces[uFaceID].polygonType == POLYGON_Floor) {
            pSpriteObject->vVelocity.z = 0;
        } else {
            if (pIndoor->faces[uFaceID].facePlane.normal.z < 0.68664550781f) { // was 45000 fixpoint
                pSpriteObject->vVelocity.z -= pEventTimer->dt().ticks() * GetGravityStrength();
            }
        }
        pSpriteObject->vVelocity *= 0.89263916f; // was 58500 fp
        if (pSpriteObject->vVelocity.xy().lengthSqr() < 400) {
            pSpriteObject->vVelocity = Vec3f(0, 0, 0);
            if (!(pObject->uFlags & OBJECT_DESC_NO_SPRITE)) {
                return;
            }
            createSpriteTrailParticle(pSpriteObject->vPosition, pObject->uFlags);
            return;
        }
        // TODO(Nik-RE-dev): is this correct?
        goto LABEL_25;
    }
}

void SpriteObject::explosionTraps() {
    MapInfo *pMapInfo = &pMapStats->pInfos[engine->_currentLoadedMapId];
    int dir_x = std::abs(pParty->pos.x - this->vPosition.x);
    int dir_y = std::abs(pParty->pos.y - this->vPosition.y);
    int dir_z = std::abs(pParty->pos.z + pParty->eyeLevel - this->vPosition.z);
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
        if (pMapInfo->trapDamageD20DiceCount) {
            trapDamage += grng->randomDice(pMapInfo->trapDamageD20DiceCount, 20);
        }
        DamageType pDamageType;
        switch (this->uType) {
            case SPRITE_TRAP_FIRE:
                pDamageType = DAMAGE_FIRE;
                break;
            case SPRITE_TRAP_LIGHTNING:
                pDamageType = DAMAGE_AIR;
                break;
            case SPRITE_TRAP_COLD:
                pDamageType = DAMAGE_WATER;
                break;
            case SPRITE_TRAP_BODY:
                pDamageType = DAMAGE_BODY;
                break;
            default:
                return;
        }
        for (Character &player : pParty->pCharacters) {
            int perceptionCheckValue = player.GetPerception() + 20;
            if (player.CanAct() && (grng->random(perceptionCheckValue) > 20)) {
                player.playReaction(SPEECH_AVOID_DAMAGE);
            } else {
                player.receiveDamage(trapDamage, pDamageType);
            }
        }
    }
}

Duration SpriteObject::GetLifetime() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pObjectDesc->uLifetime;
}

SpriteFrame *SpriteObject::getSpriteFrame() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return pSpriteFrameTable->GetFrame(pObjectDesc->uSpriteID, timeSinceCreated);
}

bool SpriteObject::IsUnpickable() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return ((pObjectDesc->uFlags & OBJECT_DESC_UNPICKABLE) == OBJECT_DESC_UNPICKABLE);
}

bool SpriteObject::HasSprite() {
    ObjectDesc *pObjectDesc = &pObjectList->pObjects[uObjectDescID];
    return !pObjectDesc->NoSprite();
}

Color SpriteObject::GetParticleTrailColor() {
    return pObjectList->pObjects[uObjectDescID].uParticleTrailColor;
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
        // TODO(captainurist): item->uSoundID & 8 checks for laser projectiles, wtf...
        if (item->uObjectDescID && (item->uSoundID & 8 || pObjectList->pObjects[item->uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE)) {
            SpriteObject::OnInteraction(i);
        }
    }
}

// Process GM Shrink Ray spell that affect group of actors
bool SpriteObject::applyShrinkRayAoe() {
    bool isApplied = false;
    // Calculation was moved from initial sprite creation processing
    Duration duration = Duration::fromMinutes(this->spell_level * 5);
    static const int shrinkPower = 4;
    int effectDistance = engine->config->gameplay.ShrinkRayAoeDistance.value();

    for (Actor &actor : pActors) {
        // TODO(Nik-RE-dev): paralyzed actor will not be affected?
        if (actor.CanAct()) {
            float distanceSq = (actor.pos - this->vPosition + Vec3f(0, 0, actor.height / 2)).lengthSqr();
            float checkDistanceSq = (effectDistance + actor.radius) * (effectDistance + actor.radius);

            if (distanceSq <= checkDistanceSq) {
                if (actor.DoesDmgTypeDoDamage(DAMAGE_DARK)) {
                    actor.buffs[ACTOR_BUFF_SHRINK].Apply(pParty->GetPlayingTime() + duration, this->spell_skill, shrinkPower, 0, 0);
                    actor.attributes |= ACTOR_AGGRESSOR;
                    isApplied = true;
                }
            }
        }
    }
    return isApplied;
}

bool SpriteObject::dropItemAt(SpriteId sprite, Vec3f pos, int speed, int count,
                              bool randomRotate, SpriteAttributes attributes, Item *item) {
    SpriteObject pSpellObject;

    pSpellObject.uType = sprite;
    pSpellObject.uObjectDescID = pObjectList->ObjectIDByItemID(sprite);
    pSpellObject.vPosition = pos;
    pSpellObject.uAttributes = attributes;
    pSpellObject.uSectorID = pIndoor->GetSector(pos);
    pSpellObject.containing_item.Reset();
    if (item) {
        pSpellObject.containing_item = *item;
    }

    if (!(pSpellObject.uAttributes & SPRITE_IGNORE_RANGE)) {
        for (ItemId i : pItemTable->items.indices()) {
            if (pItemTable->items[i].spriteId == sprite) {
                pSpellObject.containing_item.itemId = i;
            }
        }
    }

    if (randomRotate) {
        for (int i = 0; i < count; i++) {
            // Not sure if using grng is right here, but would rather err on the side of safety.
            pSpellObject.uFacing = grng->random(TrigLUT.uIntegerDoublePi);
            int pitch = TrigLUT.uIntegerQuarterPi + grng->random(TrigLUT.uIntegerQuarterPi);
            pSpellObject.Create(pSpellObject.uFacing, pitch, speed, 0);
        }
    } else {
        pSpellObject.uFacing = 0;
        for (int i = 0; i < count; i++) {
            pSpellObject.Create(pSpellObject.uFacing, TrigLUT.uIntegerHalfPi, speed, 0);
        }
    }
    return true;
}

// splash on water
void SpriteObject::createSplashObject(Vec3f pos) {
    SpriteObject sprite;
    sprite.containing_item.Reset();
    sprite.uType = SPRITE_WATER_SPLASH;
    sprite.uObjectDescID = pObjectList->ObjectIDByItemID(sprite.uType);
    sprite.vPosition = pos;
    sprite.uSectorID = pIndoor->GetSector(pos);
    int objID = sprite.Create(0, 0, 0, 0);
    if (objID != -1) {
        pAudioPlayer->playSound(SOUND_splash, SOUND_MODE_PID, Pid(OBJECT_Sprite, objID));
    }
}

static void updateSpriteOnImpact(SpriteObject *object) {
    object->uType = impactSprite(object->uType);
    object->uObjectDescID = pObjectList->ObjectIDByItemID(object->uType);
}

bool processSpellImpact(unsigned int uLayingItemID, Pid pid) {
    SpriteObject *object = &pSpriteObjects[uLayingItemID];
    ObjectDesc *objectDesc = &pObjectList->pObjects[object->uObjectDescID];

    if (pid.type() == OBJECT_Actor) {
        if (object->spell_caster_pid.type() == OBJECT_Actor && pActors[object->spell_caster_pid.id()].GetActorsRelation(&pActors[pid.id()]) == HOSTILITY_FRIENDLY) {
            return 1;
        }
    } else {
        if (pid.type() == OBJECT_Character && object->spell_caster_pid.type() == OBJECT_Character) {
            return 1;
        }
    }

    if (pParty->bTurnBasedModeOn) {
        if (object->uAttributes & SPRITE_HALT_TURN_BASED) {
            --pTurnEngine->pending_actions;
            object->uAttributes &= ~SPRITE_HALT_TURN_BASED;
        }
    }
    if (pid.type() == OBJECT_Face && object->spell_caster_pid.type() != OBJECT_Character) {
        if (object->spell_caster_pid.id() < pActors.size()) {  // bugfix  v2->spell_caster_pid.id()==1000
            pActors[object->spell_caster_pid.id()].attributes |= ACTOR_LAST_SPELL_MISSED;
        }
    }

    switch (object->uType) {
        case SPRITE_SPELL_FIRE_FIRE_SPIKE:
        case SPRITE_SPELL_AIR_SPARKS:
        case SPRITE_SPELL_DARK_TOXIC_CLOUD: {
            if (pid.type() == OBJECT_Face
                || pid.type() == OBJECT_Decoration
                || pid.type() == OBJECT_None) {
                return 1;
            }
            if (pid.type() != OBJECT_Sprite) {
                applySpellSpriteDamage(uLayingItemID, pid);
                updateSpriteOnImpact(object);
                if (object->uObjectDescID == 0) {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                object->spellSpriteStop();
                // v97 = 0;
                // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                //     v97 = (int16_t)pSpriteObjects[uLayingItemID].uSoundID + 4;
                // }
                //            v125 =
                //            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
                //            - 1] + 1; pAudioPlayer->playSound(v125,
                //            v124, 0, -1, 0, v97, 0, 0);
                pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
                return 0;
            }
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            pAudioPlayer->playSound(SOUND_fireBall, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_PROJECTILE_AIR_BOLT:
        case SPRITE_PROJECTILE_EARTH_BOLT:
        case SPRITE_PROJECTILE_FIRE_BOLT:
        case SPRITE_PROJECTILE_WATER_BOLT:
        case SPRITE_PROJECTILE_BODY_BOLT:
        case SPRITE_PROJECTILE_MIND_BOLT:
        case SPRITE_PROJECTILE_SPIRIT_BOLT:
        case SPRITE_PROJECTILE_LIGHT_BOLT:
        case SPRITE_PROJECTILE_DARK_BOLT: {
            applySpellSpriteDamage(uLayingItemID, pid);
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            if (object->uType == SPRITE_PROJECTILE_BLASTER) {
                // TODO(Nik-RE-dev): unreachable, these cases does not process this sprite type
                pAudioPlayer->playSound(SOUND_fireBall, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            }
            return 0;
        }

        case SPRITE_PROJECTILE_ARROW:
        case SPRITE_PROJECTILE_FLAMING_ARROW: {
            // Note that ITEM_SPELLBOOK_FIREBALL is an MM6 remnant here,
            // in MM6 it was Percival artifact (id 405) which has swiftness and carnage enchantments
            if (object->containing_item.itemId != ITEM_SPELLBOOK_FIREBALL &&
                object->containing_item.specialEnchantment != ITEM_ENCHANTMENT_OF_CARNAGE) {
                object->spellSpriteStop();
                applySpellSpriteDamage(uLayingItemID, pid);
                SpriteObject::OnInteraction(uLayingItemID);
                // v16 = 0;
                // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                //     v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                // }
                //            v124 = 8 * uLayingItemID;
                //            v124 |= 2;
                //            v125 =
                //            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id]
                //            + 1; pAudioPlayer->playSound(v125, v124,
                //            0, -1, 0, v16, 0, 0);
                pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
                return 0;
            }
            object->uType = SPRITE_OBJECT_EXPLODE;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_OBJECT_EXPLODE);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            object->uObjectDescID = pObjectList->ObjectIDByItemID(object->uType);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            if (object->uType != SPRITE_PROJECTILE_BLASTER) {
                pAudioPlayer->playSound(SOUND_fireBall, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
                return 0;
            }
            return 0;
        }

        case SPRITE_OBJECT_EXPLODE: {  // actor death explode
            object->uType = SPRITE_OBJECT_EXPLODE_IMPACT;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_OBJECT_EXPLODE_IMPACT);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            pushAoeAttack(Pid(OBJECT_Sprite, uLayingItemID), engine->config->gameplay.AoeDamageDistance.value(),
                    pSpriteObjects[uLayingItemID].vPosition, ABILITY_ATTACK1);
            if (objectDesc->uFlags & OBJECT_DESC_TRAIL_PARTICLE) {
                trail_particle_generator.GenerateTrailParticles(object->vPosition.x, object->vPosition.y, object->vPosition.z,
                                                                objectDesc->uParticleTrailColor);
            }
            pAudioPlayer->playSound(SOUND_fireBall, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
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
        case SPRITE_SPELL_MIND_PSYCHIC_SHOCK:
        case SPRITE_SPELL_BODY_HARM:
        case SPRITE_SPELL_BODY_FLYING_FIST:
        case SPRITE_SPELL_LIGHT_LIGHT_BOLT:
        case SPRITE_SPELL_LIGHT_SUNRAY:
        case SPRITE_SPELL_DARK_SHARPMETAL: {
            applySpellSpriteDamage(uLayingItemID, pid);
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            // int v97 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_PROJECTILE_BLASTER: {
            applySpellSpriteDamage(uLayingItemID, pid);
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            return 0;
        }

        case SPRITE_SPELL_WATER_ICE_BLAST: {
            object->uType = SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->vVelocity = Vec3f(0, 0, 0);
            int iceParticles = (object->spell_skill == MASTERY_GRANDMASTER) ? 9 : 7;
            int yaw = object->uFacing - TrigLUT.uIntegerDoublePi;
            SpriteObject temp = *object;
            for (int i = 0; i < iceParticles; i++) {
                yaw += TrigLUT.uIntegerQuarterPi;
                temp.Create(yaw, 0, 1000, 0);
            }
            SpriteObject::OnInteraction(uLayingItemID);
            // int v16 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->playSpellSound(temp.uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_SPELL_WATER_ICE_BLAST_FALLOUT: {
            object->uType = SPRITE_SPELL_WATER_ICE_BLAST_IMPACT;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BLAST_IMPACT);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            applySpellSpriteDamage(uLayingItemID, pid);
            // int v16 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= v124 | 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_SPELL_EARTH_ROCK_BLAST: {
            if (pid.type() == OBJECT_Face || pid.type() == OBJECT_Decoration || pid.type() == OBJECT_None) {
                return 1;
            }
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            pushAoeAttack(Pid(OBJECT_Sprite, uLayingItemID), engine->config->gameplay.AoeDamageDistance.value(),
                    pSpriteObjects[uLayingItemID].vPosition, ABILITY_ATTACK1);
            // int v78 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v78 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125,
            //        pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78,
            //        0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM: {
            object->uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->vVelocity = Vec3f(0, 0, 0);
            int yaw = object->uFacing - TrigLUT.uIntegerDoublePi;
            SpriteObject temp = *object;
            for (int i = 0; i < 8; i++) {
                int yawRandomDelta = grng->randomInSegment(-128, 128);
                int randomSpeed = grng->randomInSegment(5, 500);
                yaw += TrigLUT.uIntegerQuarterPi;
                temp.Create(yaw + yawRandomDelta, 0, randomSpeed, 0);
            }
            SpriteObject::OnInteraction(uLayingItemID);
            // int v16 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v16 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v124 = 8 * uLayingItemID;
            //        v124 |= 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v124, 0,
            //        -1, 0, v16, 0, 0);
            pAudioPlayer->playSpellSound(temp.uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT: {
            object->uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT;
            object->uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_EARTH_DEATH_BLOSSOM_IMPACT);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            pushAoeAttack(Pid(OBJECT_Sprite, uLayingItemID), engine->config->gameplay.AoeDamageDistance.value(),
                    pSpriteObjects[uLayingItemID].vPosition, object->spellCasterAbility);
            // int v78 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v78 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125,
            //        pSpriteObjects[uLayingItemID].vPosition.x, 0, -1, 0, v78,
            //        0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        case SPRITE_SPELL_LIGHT_DESTROY_UNDEAD: {
            if (pid.type() == OBJECT_Actor &&
                supertypeForMonsterId(pActors[pid.id()].monsterInfo.id) == MONSTER_SUPERTYPE_UNDEAD) {
                applySpellSpriteDamage(uLayingItemID, pid);
            }
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            // int v97 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v97 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v92 = uLayingItemID;
            //        v124 = 8 * v92;
            //        v124 |= 2;
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v124, 0,
            //        -1, 0, v97, 0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
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
            bool isDamaged = false;
            bool isShrinkingRayAoe = (object->uType == SPRITE_SPELL_DARK_SHRINKING_RAY) && (object->spell_skill == MASTERY_GRANDMASTER);
            if (pid.type() != OBJECT_Actor) {
                if (!isShrinkingRayAoe) {
                    SpriteObject::OnInteraction(uLayingItemID);
                    return 0;
                }
                isDamaged = object->applyShrinkRayAoe();
                if (isDamaged) {
                    updateSpriteOnImpact(object);
                    if (object->uObjectDescID == 0) {
                        SpriteObject::OnInteraction(uLayingItemID);
                    }
                    object->spellSpriteStop();
                    // int v114 = 0;
                    // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                    //     v114 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                    // }
                    //                v115 = 8 * uLayingItemID;
                    //                v115 |= Pid(OBJECT_Sprite, uLayingItemID);
                    //                v125 = v143 + 1;
                    //                pAudioPlayer->playSound(v125,
                    //                v115, 0, -1, 0, v114, 0, 0);
                    pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
                } else {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                return 0;
            }
            int shrinkPower = 0;
            // Calculation was moved from initial sprite creation processing
            Duration duration = Duration::fromMinutes(object->spell_level * 5);
            Mastery skillMastery = object->spell_skill;
            DamageType dmgType;
            ActorBuff buffIdx;
            switch (object->uType) {
                case SPRITE_SPELL_MIND_CHARM:
                    dmgType = DAMAGE_MIND;
                    buffIdx = ACTOR_BUFF_CHARM;
                    break;
                case SPRITE_SPELL_LIGHT_PARALYZE:
                    dmgType = DAMAGE_LIGHT;
                    buffIdx = ACTOR_BUFF_PARALYZED;
                    break;
                case SPRITE_SPELL_DARK_SHRINKING_RAY:
                    dmgType = DAMAGE_DARK;
                    buffIdx = ACTOR_BUFF_SHRINK;
                    break;
                default:
                    assert(false);
                    break;
            }
            if (object->uType == SPRITE_SPELL_DARK_SHRINKING_RAY) {
                switch (skillMastery) {
                    case MASTERY_NOVICE:
                        shrinkPower = 2;
                        break;
                    case MASTERY_EXPERT:
                        shrinkPower = 3;
                        break;
                    case MASTERY_MASTER:
                    case MASTERY_GRANDMASTER:
                        shrinkPower = 4;
                        break;
                    default:
                        assert(false);
                        break;
                }
                pActors[pid.id()].attributes |= ACTOR_AGGRESSOR;
            }

            if (!isShrinkingRayAoe) {
                int actorId = pid.id();
                if (pActors[pid.id()].DoesDmgTypeDoDamage(dmgType)) {
                    isDamaged = true;
                    if (object->uType == SPRITE_SPELL_LIGHT_PARALYZE) {
                        pActors[actorId].aiState = Standing;
                        pActors[actorId].UpdateAnimation();
                    }
                    pActors[actorId].buffs[buffIdx].Apply(pParty->GetPlayingTime() + duration, skillMastery, shrinkPower, 0, 0);
                }
            } else {
                isDamaged = object->applyShrinkRayAoe();
            }
            object->spell_level = 0;
            object->spell_skill = MASTERY_NONE;
            object->uSpellID = SPELL_NONE;
            if (isDamaged) {
                updateSpriteOnImpact(object);
                if (object->uObjectDescID == 0) {
                    SpriteObject::OnInteraction(uLayingItemID);
                }
                object->spellSpriteStop();
                // int v114 = 0;
                // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
                //     v114 = pSpriteObjects[uLayingItemID].uSoundID + 4;
                // }
                //            v115 = 8 * uLayingItemID;
                //            v115 |= Pid(OBJECT_Sprite, uLayingItemID);
                //            v125 = v143 + 1;
                //            pAudioPlayer->playSound(v125, v115, 0,
                //            -1, 0, v114, 0, 0);
                pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            } else {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            return 0;
        }

            /*
            case 1080:
            case 2100:
            {
            if (a2.type() != 3)
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
            LOBYTE(v102) = Pid(OBJECT_Sprite, uLayingItemID);
            pSpriteObjects[uLayingItemID].vVelocity.x = 0;
            pSpriteObjects[uLayingItemID].vVelocity.y = 0;
            pSpriteObjects[uLayingItemID].vVelocity.z = 0;
            AttackerInfo.Add(v102, 512,
            pSpriteObjects[uLayingItemID].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y,
            pSpriteObjects[uLayingItemID].vPosition.z, v100, 0); if
            (objectDesc->uFlags & OBJECT_DESC_TRIAL_PARTICLE)
            trail_particle_generator.GenerateTrailParticles(pSpriteObjects[uLayingItemID].vPosition.x,
            pSpriteObjects[uLayingItemID].vPosition.y,
            pSpriteObjects[uLayingItemID].vPosition.z,
            objectDesc->uParticleTrailColor); if
            (!pSpriteObjects[uLayingItemID].uSoundID) v47 = 0; else v47 =
            (int16_t)pSpriteObjects[uLayingItemID].uSoundID + 4; v125 =
            word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id - 1] +
            1; pAudioPlayer->playSound(v125, v102, 0, -1, 0, v47, 0,
            0); return 0;
            }
            return 1;
            }*/

        case SPRITE_SPELL_FIRE_METEOR_SHOWER:
        case SPRITE_SPELL_AIR_STARBURST: {
            if (pid.type() == OBJECT_Actor) {
                return 1;
            }
            // else go to next case
        }

        case SPRITE_SPELL_FIRE_FIREBALL:
        case SPRITE_SPELL_DARK_DRAGON_BREATH: {
            updateSpriteOnImpact(object);
            if (object->uObjectDescID == 0) {
                SpriteObject::OnInteraction(uLayingItemID);
            }
            object->spellSpriteStop();
            pushAoeAttack(Pid(OBJECT_Sprite, uLayingItemID), engine->config->gameplay.AoeDamageDistance.value(),
                    pSpriteObjects[uLayingItemID].vPosition, object->spellCasterAbility);
            if (objectDesc->uFlags & OBJECT_DESC_TRAIL_PARTICLE) {
                trail_particle_generator.GenerateTrailParticles(
                    object->vPosition.x, object->vPosition.y, object->vPosition.z,
                    objectDesc->uParticleTrailColor);
            }
            // int v47 = 0;
            // if (pSpriteObjects[uLayingItemID].uSoundID != 0) {
            //     v47 = pSpriteObjects[uLayingItemID].uSoundID + 4;
            // }
            //        v125 =
            //        word_4EE088_sound_ids[pSpriteObjects[uLayingItemID].spell_id
            //        - 1] + 1; pAudioPlayer->playSound(v125, v102, 0,
            //        -1, 0, v47, 0, 0);
            pAudioPlayer->playSpellSound(object->uSpellID, true, SOUND_MODE_PID, Pid(OBJECT_Sprite, uLayingItemID));
            return 0;
        }

        default:
            return 0;
    }
}

void applySpellSpriteDamage(unsigned int uLayingItemID, Pid pid) {
    if (pid.type() == OBJECT_Character) {
        DamageCharacterFromMonster(Pid(OBJECT_Sprite, uLayingItemID), pSpriteObjects[uLayingItemID].spellCasterAbility, -1);
    } else if (pid.type() == OBJECT_Actor) {
        Vec3f velF = pSpriteObjects[uLayingItemID].vVelocity;
        velF.normalize();
        switch (pSpriteObjects[uLayingItemID].spell_caster_pid.type()) {
            case OBJECT_Actor:
                Actor::ActorDamageFromMonster(Pid(OBJECT_Sprite, uLayingItemID), pid.id(), velF, pSpriteObjects[uLayingItemID].spellCasterAbility);
                break;
            case OBJECT_Character:
                Actor::DamageMonsterFromParty(Pid(OBJECT_Sprite, uLayingItemID), pid.id(), velF);
                break;
            case OBJECT_Sprite:
                ItemDamageFromActor(Pid(OBJECT_Sprite, uLayingItemID), pid.id(), velF);
                break;
            default:
                break;
        }
    }
}

void UpdateObjects() {
    for (unsigned i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uAttributes & SPRITE_SKIP_A_FRAME) {
            pSpriteObjects[i].uAttributes &= ~SPRITE_SKIP_A_FRAME;
        } else {
            ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
            if (pSpriteObjects[i].attachedToActor()) {
                int actorId = pSpriteObjects[i].spell_target_pid.id();
                if (actorId > pActors.size()) {
                    continue;
                }
                pSpriteObjects[i].vPosition = pActors[actorId].pos + Vec3f(0, 0, pActors[actorId].height);
                if (!pSpriteObjects[i].uObjectDescID) {
                    continue;
                }
                pSpriteObjects[i].timeSinceCreated += pEventTimer->dt();
                if (!(object->uFlags & OBJECT_DESC_TEMPORARY)) {
                    continue;
                }
                if (pSpriteObjects[i].timeSinceCreated >= 0_ticks) {
                    Duration lifetime = object->uLifetime;
                    if (pSpriteObjects[i].uAttributes & SPRITE_TEMPORARY) {
                        lifetime = pSpriteObjects[i].tempLifetime;
                    }
                    if (pSpriteObjects[i].timeSinceCreated < lifetime) {
                        continue;
                    }
                }
                SpriteObject::OnInteraction(i);
                continue;
            }
            if (pSpriteObjects[i].uObjectDescID) {
                Duration lifetime;
                pSpriteObjects[i].timeSinceCreated += pEventTimer->dt();
                if (object->uFlags & OBJECT_DESC_TEMPORARY) {
                    if (pSpriteObjects[i].timeSinceCreated < 0_ticks) {
                        SpriteObject::OnInteraction(i);
                        continue;
                    }
                    lifetime = object->uLifetime;
                    if (pSpriteObjects[i].uAttributes & SPRITE_TEMPORARY) {
                        lifetime = pSpriteObjects[i].tempLifetime;
                    }
                }
                if (!(object->uFlags & OBJECT_DESC_TEMPORARY) ||
                    pSpriteObjects[i].timeSinceCreated < lifetime) {
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        SpriteObject::updateObjectBLV(i);
                    } else {
                        SpriteObject::updateObjectODM(i);
                    }
                    if (!pParty->bTurnBasedModeOn || !(object->uFlags & OBJECT_DESC_TEMPORARY)) {
                        continue;
                    }
                    if ((pParty->pos - pSpriteObjects[i].vPosition).length() <= 5120) {
                        continue;
                    }
                    // Temporary object in turn based mode that gets too far from party
                    SpriteObject::OnInteraction(i);
                    continue;
                }
                // Lifetime expired
                if (!(object->uFlags & OBJECT_DESC_INTERACTABLE)) {
                    SpriteObject::OnInteraction(i);
                    continue;
                }
                processSpellImpact(i, Pid(OBJECT_Sprite, i));
            }
        }
    }
}

unsigned int collideWithActor(unsigned int uLayingItemID, Pid pid) {
    unsigned int result = uLayingItemID;
    if (pObjectList->pObjects[pSpriteObjects[uLayingItemID].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE) {
        result = processSpellImpact(uLayingItemID, pid);
    }
    return result;
}
