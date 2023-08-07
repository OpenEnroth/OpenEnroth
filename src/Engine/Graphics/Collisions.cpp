#include "Collisions.h"

#include <algorithm>
#include <limits>

#include "Engine/Events/Processor.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/MM7.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"

#include "Utility/Math/Float.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Math/FixPoint.h"

CollisionState collision_state;

constexpr float COLLISIONS_EPS = 0.01f;
constexpr float COLLISIONS_MIN_MOVE_DISTANCE = 0.5f; // Minimal movement distance, anything below this value gets rounded down to zero.

//
// Helper functions.
//

/**
 * @offset 0x0047531C, 0x004754BF.
 *
 * @param face                          Polygon to check collision against.
 * @param pos                           Actor position to check.
 * @param radius                        Actor radius.
 * @param dir                           Movement direction as a unit vector in fixpoint format.
 * @param[out] out_move_distance        Move distance along the `dir` axis required to touch the provided polygon.
 *                                      Always non-negative. This parameter is not set if the function returns false.
 *                                      Note that "touching" in this context means that the distance from the actor's
 *                                      center to the polygon equals actor's radius.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
 * @return                              Whether the actor, basically modeled as a sphere, can actually collide with the
 *                                      polygon if moving along the `dir` axis.
 */
static bool CollideSphereWithFace(BLVFace *face, const Vec3f &pos, float radius, const Vec3f &dir,
                                  float *out_move_distance, bool ignore_ethereal, int model_idx) {
    if (ignore_ethereal && face->Ethereal())
        return false;

    float dir_normal_projection = dot(dir, face->facePlane.normal);

    // This is checked by the caller, we should be moving into the face or sideways, so projection of dir onto the
    // face normal should either be negative or close to zero.
    assert(dir_normal_projection < COLLISIONS_EPS);

    float center_face_distance = face->facePlane.signedDistanceTo(pos);
    assert(center_face_distance > 0); // Checked by the caller, we should be in front of the face, not behind it.

    float move_distance;
    Vec3f projected_pos = pos;

    if (center_face_distance < radius) {
        // Already colliding.
        move_distance = 0;
        projected_pos += center_face_distance * -face->facePlane.normal;
    } else {
        // Moving sideways & not already colliding? No collision.
        if (fuzzyIsNull(dir_normal_projection, COLLISIONS_EPS))
            return false;

        // Otherwise can move along the dir vector until the sphere touches the face.
        move_distance = (center_face_distance - radius) / -dir_normal_projection;
        assert(move_distance >= 0);

        projected_pos += move_distance * dir - radius * face->facePlane.normal;
    }

    assert(fuzzyIsNull(face->facePlane.signedDistanceTo(projected_pos), COLLISIONS_EPS)); // TODO(captainurist): move into face->Contains.

    if (!face->Contains(projected_pos.toInt(), model_idx))
        return false; // We've just managed to slide past the face, no collision happened.

    *out_move_distance = move_distance;
    return true;
}

/**
 * @offset 0x00475D85, 0x00475F30.
 *
 * @param face                          Polygon to check collision against.
 * @param pos                           Actor position to check.
 * @param dir                           Movement direction as a unit vector in fixpoint format.
 * @param[in,out] out_move_distance     Current movement distance along the `dir` axis. This parameter is not touched
 *                                      when the function returns false. If the function returns true, then the
 *                                      distance required to hit the polygon is stored here. Note that this effectively
 *                                      means that this function can only decrease `move_distance`, but never increase
 *                                      it.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
 * @return                              Whether the actor, modeled as a point, hits the provided polygon if moving from
 *                                      `pos` along the `dir` axis by at most `move_distance`.
 */
static bool CollidePointWithFace(BLVFace *face, const Vec3f &pos, const Vec3f &dir, float *out_move_distance,
                                 int model_idx) {
    // _fp suffix => that's a fixpoint number

    // dot_product(dir, normal) is a cosine of an angle between them.
    float cos_dir_normal = dot(dir, face->facePlane.normal);

    if (fuzzyIsNull(cos_dir_normal, COLLISIONS_EPS))
        return false; // dir is perpendicular to face normal.

    if (face->uAttributes & FACE_ETHEREAL)
        return false;

    if (cos_dir_normal > 0 && !face->isPortal())
        return false; // We're facing away && face is not a portal.

    float pos_face_distance = face->facePlane.signedDistanceTo(pos);

    if (cos_dir_normal < 0 && pos_face_distance < 0)
        return false; // Facing towards the face but already inside the model.

    if (cos_dir_normal > 0 && pos_face_distance > 0)
        return false; // Facing away from the face and outside the model.

    // How far we need to move along the `dir` axis to hit face.
    float move_distance = -pos_face_distance / cos_dir_normal;

    Vec3f new_pos = pos + move_distance * dir;

    if (move_distance > *out_move_distance)
        return false; // No correction needed.

    if (!face->Contains(new_pos.toInt(), model_idx))
        return false;

    *out_move_distance = move_distance;
    return true;
}

/**
 * Helper function that performs several collision checks between both the "feet" and the "head" spheres of the
 * collision state, and the provided face.
 *
 * @param face                          Face to check.
 * @param face_pid                      Pid of the provided face.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
*/
static void CollideBodyWithFace(BLVFace *face, Pid face_pid, bool ignore_ethereal, int model_idx) {
    auto collide_once = [&](const Vec3f &old_pos, const Vec3f &new_pos, const Vec3f &dir, int radius) {
        float distance_old = face->facePlane.signedDistanceTo(old_pos);
        float distance_new = face->facePlane.signedDistanceTo(new_pos);
        if (distance_old > 0 && (distance_old <= radius || distance_new <= radius) && distance_new <= distance_old) {
            bool have_collision = false;
            float move_distance = collision_state.move_distance;
            if (CollideSphereWithFace(face, old_pos, radius, dir, &move_distance, ignore_ethereal, model_idx)) {
                have_collision = true;
            } else {
                move_distance = collision_state.move_distance + radius;
                if (CollidePointWithFace(face, old_pos, dir, &move_distance, model_idx)) {
                    have_collision = true;
                    move_distance -= radius;
                }
            }

            if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                collision_state.adjusted_move_distance = move_distance;
                collision_state.pid = face_pid;
            }
        }
    };

    collide_once(collision_state.position_lo, collision_state.new_position_lo, collision_state.direction, collision_state.radius_lo);

    if (!collision_state.check_hi)
        return;

    collide_once(collision_state.position_hi, collision_state.new_position_hi, collision_state.direction, collision_state.radius_hi);
}

/**
 * Performs a collision check with a cylinder.
 *
 * @param center_lo                     Center of the cylinder's base.
 * @param radius                        Cylinder radius.
 * @param height                        Cylinder height.
 * @param pid                           Pid of the object represented by the cylinder.
 * @param jagged_top                    See `CollideWithParty`.
 * @return                              Whether there is a collision.
 */
static bool CollideWithCylinder(const Vec3f &center_lo, float radius, float height, Pid pid, bool jagged_top) {
    BBoxf bbox = BBoxf::forCylinder(center_lo, radius, height);
    if (!collision_state.bbox.intersects(bbox))
        return false;

    // dist vector points from position center into cylinder center.
    float dist_x = center_lo.x - collision_state.position_lo.x;
    float dist_y = center_lo.y - collision_state.position_lo.y;
    float sum_radius = collision_state.radius_lo + radius;

    // Area of the parallelogram formed by dist and collision_state.direction. Direction is a unit vector,
    // thus this actually is length(dist) * sin(dist, collision_state.direction).
    // This in turn is the distance from cylinder center to the line of actor's movement.
    Vec3f dir = collision_state.direction;
    float closest_dist = dist_x * dir.y - dist_y * dir.x;
    if (std::abs(closest_dist) > sum_radius)
        return false; // No chance to collide.

    // Length of dist vector projected onto collision_state.direction.
    float dist_dot_dir = dist_x * dir.x + dist_y * dir.y;
    if (dist_dot_dir <= 0)
        return false; // We're moving away from the cylinder.

    // Z-coordinate of the actor at the point closest to the cylinder in XY plane.
    float closest_z = collision_state.position_lo.z + dir.z * dist_dot_dir;
    if (closest_z < bbox.z1 || (closest_z > bbox.z2 && !jagged_top))
        return false;

    // That's how far can we go along the collision_state.direction axis until the actor touches the cylinder,
    // i.e. distance between them goes below sum_radius.
    float move_distance = dist_dot_dir - std::sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
    if (move_distance < 0)
        move_distance = 0;

    if (move_distance < collision_state.adjusted_move_distance) {
        collision_state.adjusted_move_distance = move_distance;
        collision_state.pid = pid;
    }
    return true;
}

static void CollideWithDecoration(int id) {
    LevelDecoration *decor = &pLevelDecorations[id];
    if (decor->uFlags & LEVEL_DECORATION_INVISIBLE)
        return;

    const DecorationDesc *desc = pDecorationList->GetDecoration(decor->uDecorationDescID);
    if (desc->CanMoveThrough())
        return;

    CollideWithCylinder(decor->vPosition.toFloat(), desc->uRadius, desc->uDecorationHeight, Pid(OBJECT_Decoration, id), false);
}


//
// Public API.
//

bool CollisionState::PrepareAndCheckIfStationary(int dt_fp) {
    if (!dt_fp)
        dt_fp = pEventTimer->dt_fixpoint;
    float dt = fixpoint_to_float(dt_fp);

    this->speed = this->velocity.length();
    if (fuzzyIsNull(this->speed, COLLISIONS_EPS))
        return true;

    this->direction = this->velocity / this->speed;

    this->move_distance = dt * this->speed - this->total_move_distance;
    if (this->move_distance <= COLLISIONS_MIN_MOVE_DISTANCE)
        return true;

    this->new_position_hi = this->position_hi + this->move_distance * this->direction;
    this->new_position_lo = this->position_lo + this->move_distance * this->direction;

    this->bbox =
        BBoxf::cubic(this->position_lo, this->radius_lo) |
        BBoxf::cubic(this->new_position_lo, this->radius_lo) |
        BBoxf::cubic(this->position_hi, this->radius_hi) |
        BBoxf::cubic(this->new_position_hi, this->radius_hi);

    this->pid = Pid();
    this->adjusted_move_distance = this->move_distance;

    return false;
}

void CollideIndoorWithGeometry(bool ignore_ethereal) {
    std::array<int, 10> pSectorsArray;
    pSectorsArray[0] = collision_state.uSectorID;
    int totalSectors = 1;

    // See if we're touching portals. If we do, we need to add corresponding sectors to the sectors array.
    BLVSector *pSector = &pIndoor->pSectors[collision_state.uSectorID];
    for (int j = 0; j < pSector->uNumPortals; ++j) {
        BLVFace *pFace = &pIndoor->pFaces[pSector->pPortals[j]];
        if (!collision_state.bbox.intersects(pFace->pBounding))
            continue;

        float distance = std::abs(pFace->facePlane.signedDistanceTo(collision_state.position_lo));
        if(distance > collision_state.move_distance + 16)
            continue;

        pSectorsArray[totalSectors++] =
            pFace->uSectorID == collision_state.uSectorID ? pFace->uBackSectorID : pFace->uSectorID;
        break;
    }

    for (int i = 0; i < totalSectors; i++) {
        pSector = &pIndoor->pSectors[pSectorsArray[i]];

        int totalFaces = pSector->uNumFloors + pSector->uNumWalls + pSector->uNumCeilings;
        for (int j = 0; j < totalFaces; j++) {
            BLVFace *face = &pIndoor->pFaces[pSector->pFloors[j]];
            if (face->isPortal() || !collision_state.bbox.intersects(face->pBounding))
                continue;

            int face_id = pSector->pFloors[j];
            if (face_id == collision_state.ignored_face_id)
                continue;

            CollideBodyWithFace(face, Pid(OBJECT_Face, face_id), ignore_ethereal, MODEL_INDOOR);
        }
    }
}

void CollideOutdoorWithModels(bool ignore_ethereal) {
    for (BSPModel &model : pOutdoor->pBModels) {
        if (!collision_state.bbox.intersects(model.pBoundingBox))
            continue;

        for (ODMFace &mface : model.pFaces) {
            if (!collision_state.bbox.intersects(mface.pBoundingBox))
                continue;

            // TODO: we should really either merge two face classes, or template the functions down the chain call here.
            BLVFace face;
            face.facePlane = mface.facePlane;
            face.uAttributes = mface.uAttributes;
            face.pBounding = mface.pBoundingBox;
            face.zCalc = mface.zCalc;
            face.uPolygonType = mface.uPolygonType;
            face.uNumVertices = mface.uNumVertices;
            face.resource = mface.resource;
            face.pVertexIDs = mface.pVertexIDs.data();

            if (face.Ethereal() || face.isPortal()) // TODO: this doesn't respect ignore_ethereal parameter
                continue;

            Pid pid = Pid::odmFace(model.index, mface.index);
            CollideBodyWithFace(&face, pid, ignore_ethereal, model.index);
        }
    }
}

void CollideIndoorWithDecorations() {
    BLVSector *sector = &pIndoor->pSectors[collision_state.uSectorID];
    for (unsigned int i = 0; i < sector->uNumDecorations; ++i)
        CollideWithDecoration(sector->pDecorationIDs[i]);
}

void CollideOutdoorWithDecorations(int grid_x, int grid_y) {
    if (grid_x < 0 || grid_x > 127 || grid_y < 0 || grid_y > 127)
        return;

    int grid_index = grid_x + (grid_y << 7);
    int list_index = pOutdoor->pOMAP[grid_index];

    for(int i = list_index; i < pOutdoor->pFaceIDLIST.size(); i++) {
        Pid pid = pOutdoor->pFaceIDLIST[i];
        if (!pid)
            break;

        if (pid.type() != OBJECT_Decoration)
            continue;

        CollideWithDecoration(pid.id());
    }
}

bool CollideIndoorWithPortals() {
    int portal_id = 0;            // [sp+10h] [bp-4h]@15
    float min_move_distance = std::numeric_limits<float>::max();
    for (unsigned int i = 0; i < pIndoor->pSectors[collision_state.uSectorID].uNumPortals; ++i) {
        BLVFace *face = &pIndoor->pFaces[pIndoor->pSectors[collision_state.uSectorID].pPortals[i]];
        if (!collision_state.bbox.intersects(face->pBounding))
            continue;

        float distance_lo_old = face->facePlane.signedDistanceTo(collision_state.position_lo);
        float distance_lo_new = face->facePlane.signedDistanceTo(collision_state.new_position_lo);
        float move_distance = collision_state.move_distance;
        if ((distance_lo_old < collision_state.radius_lo || distance_lo_new < collision_state.radius_lo) &&
            (distance_lo_old > -collision_state.radius_lo || distance_lo_new > -collision_state.radius_lo) &&
            CollidePointWithFace(face, collision_state.position_lo, collision_state.direction, &move_distance, MODEL_INDOOR) &&
            move_distance < min_move_distance) {
            min_move_distance = move_distance;
            portal_id = pIndoor->pSectors[collision_state.uSectorID].pPortals[i];
        }
    }

    if (collision_state.adjusted_move_distance >= min_move_distance && min_move_distance <= collision_state.move_distance) {
        if (pIndoor->pFaces[portal_id].uSectorID == collision_state.uSectorID) {
            collision_state.uSectorID = pIndoor->pFaces[portal_id].uBackSectorID;
        } else {
            collision_state.uSectorID = pIndoor->pFaces[portal_id].uSectorID;
        }
        collision_state.adjusted_move_distance = collision_state.move_distance;
        return false;
    }

    return true;
}

bool CollideWithActor(int actor_idx, int override_radius) {
    Actor *actor = &pActors[actor_idx];
    if (actor->aiState == Removed || actor->aiState == Dying || actor->aiState == Disabled ||
        actor->aiState == Dead || actor->aiState == Summoned)
        return false;

    float radius = actor->radius;
    if (override_radius != 0)
        radius = override_radius;

    return CollideWithCylinder(actor->pos.toFloat(), radius, actor->height, Pid(OBJECT_Actor, actor_idx), true);
}

void _46ED8A_collide_against_sprite_objects(Pid pid) {
    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID == 0)
            continue;

        ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
        if (object->uFlags & OBJECT_DESC_NO_COLLISION)
            continue;

        // This code is very close to what we have in CollideWithCylinder, but factoring out common parts just
        // seemed not worth it.

        BBoxf bbox = BBoxf::forCylinder(pSpriteObjects[i].vPosition.toFloat(), object->uRadius, object->uHeight);
        if (!collision_state.bbox.intersects(bbox))
            continue;

        float dist_x = pSpriteObjects[i].vPosition.x - collision_state.position_lo.x;
        float dist_y = pSpriteObjects[i].vPosition.y - collision_state.position_lo.y;
        float sum_radius = object->uHeight + collision_state.radius_lo;

        Vec3f dir = collision_state.direction;
        float closest_dist = dist_x * dir.y - dist_y * dir.x;
        if (std::abs(closest_dist) > sum_radius)
            continue;

        float dist_dot_dir = dist_x * dir.x + dist_y * dir.y;
        if (dist_dot_dir <= 0)
            continue;

        float closest_z = collision_state.position_lo.z + dir.z * dist_dot_dir;
        if (closest_z < bbox.z1 - collision_state.radius_lo || closest_z > bbox.z2 + collision_state.radius_lo)
            continue;

        if (dist_dot_dir < collision_state.adjusted_move_distance)
            collideWithActor(i, pid);
    }
}

void CollideWithParty(bool jagged_top) {
    CollideWithCylinder(pParty->pos.toFloat(), 2 * pParty->radius, pParty->height, Pid::character(0), jagged_top);
}

void ProcessActorCollisionsBLV(Actor &actor, bool isAboveGround, bool isFlying) {
    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.check_hi = true;
    collision_state.radius_hi = actor.radius;
    collision_state.radius_lo = actor.radius;

    for (int attempt = 0; attempt < 100; attempt++) {
        collision_state.position_lo = actor.pos.toFloat() + Vec3f(0, 0, actor.radius + 1);
        collision_state.position_hi = actor.pos.toFloat() + Vec3f(0, 0, actor.height - actor.radius - 1);
        collision_state.position_hi.z = std::max(collision_state.position_hi.z, collision_state.position_lo.z);
        collision_state.velocity = actor.speed.toFloat();
        collision_state.uSectorID = actor.sectorId;
        if (collision_state.PrepareAndCheckIfStationary(0))
            break;

        int actorCollisions = 0;
        for (int i = 0; i < 100; ++i) {
            CollideIndoorWithGeometry(true);
            CollideIndoorWithDecorations();
            CollideWithParty(false);
            _46ED8A_collide_against_sprite_objects(Pid(OBJECT_Actor, actor.id));
            for (int j = 0; j < ai_arrays_size; j++)
                if (ai_near_actors_ids[j] != actor.id && CollideWithActor(ai_near_actors_ids[j], 40))
                    actorCollisions++;
            if (CollideIndoorWithPortals())
                break;
        }
        bool isInCrowd = actorCollisions > 1;

        Vec3f newPos = actor.pos.toFloat() + collision_state.adjusted_move_distance * collision_state.direction;
        int newFaceID = -1;
        int newFloorZ = GetIndoorFloorZ(newPos.toInt(), &collision_state.uSectorID, &newFaceID);
        if (newFloorZ == -30000)
            break; // New pos is out of bounds, running more iterations won't help.

        if (pIndoor->pFaces[newFaceID].uAttributes & FACE_INDOOR_SKY) {
            if (actor.aiState == Dead) {
                actor.aiState = Removed;
                break; // Actor removed, no point in running more iterations.
            }

            if (!isAboveGround && !isFlying) {
                if (actor.monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly || isInCrowd)
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0, nullptr);

                break; // Trying to walk into indoor sky, bad idea!
            }
        }

        // Prevent actors from falling off ledges.
        if (actor.currentActionAnimation == ANIM_Walking && newFloorZ < actor.pos.z - 100 && !isAboveGround && !isFlying) {
            if (actor.pos.x & 1) { // TODO(captainurist): replace with Random?
                actor.yawAngle += 100;
            } else {
                actor.yawAngle -= 100;
            }
            break; // We'll try again in the next frame.
        }

        actor.pos = newPos.toShort();
        actor.sectorId = collision_state.uSectorID;
        if (fuzzyEquals(collision_state.adjusted_move_distance, collision_state.move_distance))
            break; // No collisions happened.

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly;
                bool otherFriendly = pActors[id].monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly;
                if (isInCrowd) {
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0, nullptr);
                } else if (isFriendly && otherFriendly) {
                    Actor::AI_FaceObject(actor.id, collision_state.pid, 0, nullptr);
                } else {
                    Actor::AI_Flee(actor.id, collision_state.pid, 0, nullptr);
                }
            }
        }

        if (type == OBJECT_Character) {
            if (actor.GetActorsRelation(0)) {
                actor.speed.y = 0;
                actor.speed.x = 0;

                if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                }
            } else {
                Actor::AI_FaceObject(actor.id, collision_state.pid, 0, nullptr);
            }
        }

        if (type == OBJECT_Decoration) {
            int speed = integer_sqrt(actor.speed.x * actor.speed.x + actor.speed.y * actor.speed.y);
            int angle = TrigLUT.atan2(actor.pos.x - pLevelDecorations[id].vPosition.x, actor.pos.y - pLevelDecorations[id].vPosition.y); // Face away from the decoration.
            actor.speed.x = TrigLUT.cos(angle) * speed;
            actor.speed.y = TrigLUT.sin(angle) * speed;
        }

        if (type == OBJECT_Face) {
            BLVFace *face = &pIndoor->pFaces[id];

            collision_state.ignored_face_id = collision_state.pid.id();
            if (pIndoor->pFaces[id].uPolygonType == POLYGON_Floor) {
                actor.speed.z = 0;
                actor.pos.z = pIndoor->pVertices[face->pVertexIDs[0]].z + 1;
                if (actor.speed.lengthSqr() < 400) {
                    actor.speed.x = 0;
                    actor.speed.y = 0;
                    continue; // TODO(captainurist): drop this continue
                }
            } else {
                float velocityDotNormal = dot(face->facePlane.normal, actor.speed.toFloat());
                velocityDotNormal = std::max(std::abs(velocityDotNormal), collision_state.speed / 8);
                actor.speed += (velocityDotNormal * face->facePlane.normal).toShort();
                if (face->uPolygonType != POLYGON_InBetweenFloorAndWall && face->uPolygonType != POLYGON_Floor) {
                    float overshoot = collision_state.radius_lo - face->facePlane.signedDistanceTo(actor.pos.toFloat());
                    if (overshoot > 0)
                        actor.pos += (overshoot * pIndoor->pFaces[id].facePlane.normal).toShort();
                    actor.yawAngle = TrigLUT.atan2(actor.speed.x, actor.speed.y);
                }
            }
            if (pIndoor->pFaces[id].uAttributes & FACE_TriggerByMonster)
                eventProcessor(pIndoor->pFaceExtras[pIndoor->pFaces[id].uFaceExtraID].uEventID, Pid(), 1);
        }

        actor.speed.x = fixpoint_mul(58500, actor.speed.x);
        actor.speed.y = fixpoint_mul(58500, actor.speed.y);
        actor.speed.z = fixpoint_mul(58500, actor.speed.z);
    }
}

void ProcessActorCollisionsODM(Actor &actor, bool isFlying) {
    int actorRadius = !isFlying ? 40 : actor.radius;

    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.check_hi = true;
    collision_state.radius_hi = actorRadius;
    collision_state.radius_lo = actorRadius;

    for (int attempt = 0; attempt < 100; ++attempt) {
        collision_state.position_lo = actor.pos.toFloat() + Vec3f(0, 0, actorRadius + 1);
        collision_state.position_hi = actor.pos.toFloat() + Vec3f(0, 0, actor.height - actorRadius - 1);
        collision_state.position_hi.z = std::max(collision_state.position_hi.z, collision_state.position_lo.z);
        collision_state.velocity = actor.speed.toFloat();
        collision_state.uSectorID = 0;
        if (collision_state.PrepareAndCheckIfStationary(0))
            break;

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(WorldPosToGridCellX(actor.pos.x), WorldPosToGridCellY(actor.pos.y));
        CollideWithParty(false);
        _46ED8A_collide_against_sprite_objects(Pid(OBJECT_Actor, actor.id));

        int actorCollisions = 0;
        for (int i = 0; i < ai_arrays_size; i++)
            if (ai_near_actors_ids[i] != actor.id && CollideWithActor(ai_near_actors_ids[i], 40))
                actorCollisions++;
        int isInCrowd = actorCollisions > 1;

        //if (collision_state.adjusted_move_distance < collision_state.move_distance)
        //    Slope_High = collision_state.adjusted_move_distance * collision_state.direction.z;

        Vec3f newPos = actor.pos.toFloat() + collision_state.adjusted_move_distance * collision_state.direction;
        bool isOnWater = false;
        int modelPid = 0;
        int newFloorZ = ODM_GetFloorLevel(newPos.toInt(), actor.height, &isOnWater, &modelPid, 0);
        if (isOnWater) {
            if (actor.pos.z < newFloorZ + 60) {
                if (actor.aiState == Dead || actor.aiState == Dying ||
                    actor.aiState == Removed || actor.aiState == Disabled) {
                    SpriteObject::createSplashObject(Vec3i(actor.pos.x, actor.pos.y, modelPid ? newFloorZ + 30 : newFloorZ + 60));
                    actor.aiState = Removed;
                    break;
                }
            }
        }

        actor.pos = newPos.toShort();
        if (fuzzyEquals(collision_state.adjusted_move_distance, collision_state.move_distance))
            break; // No collision happened.

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly;
                bool otherFriendly = pActors[id].monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly;
                if (isInCrowd) {
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0, nullptr);
                } else if (isFriendly && otherFriendly) {
                    Actor::AI_FaceObject(actor.id, collision_state.pid, 0, nullptr);
                } else {
                    Actor::AI_Flee(actor.id, collision_state.pid, 0, nullptr);
                }
            }
        }

        if (type == OBJECT_Character) {
            if (actor.GetActorsRelation(0)) {
                actor.speed.y = 0;
                actor.speed.x = 0;

                if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                }
            } else {
                Actor::AI_FaceObject(actor.id, collision_state.pid, 0, nullptr);
            }
        }

        if (type == OBJECT_Decoration) {
            int speed = integer_sqrt(actor.speed.x * actor.speed.x + actor.speed.y * actor.speed.y);
            int angle = TrigLUT.atan2(actor.pos.x - pLevelDecorations[id].vPosition.x, actor.pos.y - pLevelDecorations[id].vPosition.y);
            actor.speed.x = TrigLUT.cos(angle) * speed;
            actor.speed.y = TrigLUT.sin(angle) * speed;
        }

        if (type == OBJECT_Face) {
            const ODMFace *face = &pOutdoor->face(collision_state.pid);

            if (!face->Ethereal()) {
                if (face->uPolygonType == POLYGON_Floor) {
                    actor.speed.z = 0;
                    actor.pos.z = pOutdoor->model(collision_state.pid).pVertices[face->pVertexIDs[0]].z + 1;
                    if (actor.speed.lengthSqr() < 400) {
                        actor.speed.y = 0;
                        actor.speed.x = 0;
                    }
                } else {
                    float velocityDotNormal = dot(face->facePlane.normal, actor.speed.toFloat());
                    // TODO(captainurist): in BLV code we have std::abs(velocityDotNormal) here, and adding std::abs affects traces.
                    // Note that not all copies of this code have std::abs. Why?
                    velocityDotNormal = std::max(velocityDotNormal, collision_state.speed / 8);

                    actor.speed += (velocityDotNormal * face->facePlane.normal).toShort();
                    if (face->uPolygonType != POLYGON_InBetweenFloorAndWall) {
                        float overshoot = collision_state.radius_lo - face->facePlane.signedDistanceTo(actor.pos.toFloat());
                        if (overshoot > 0)
                            actor.pos += (overshoot * face->facePlane.normal).toShort();
                        actor.yawAngle = TrigLUT.atan2(actor.speed.x, actor.speed.y);
                    }
                }
            }
        }

        actor.speed.x = fixpoint_mul(58500, actor.speed.x);
        actor.speed.y = fixpoint_mul(58500, actor.speed.y);
        actor.speed.z = fixpoint_mul(58500, actor.speed.z);
    }
}
