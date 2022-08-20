#include "Collisions.h"

#include <algorithm>
#include <limits>

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/MM7.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

CollisionState collision_state;

/* =================
 * Helper functions.
 */

/**
 * Original offsets 0x47531C, 0x4754BF (two functions merged into one).
 *
 * @param face                          Polygon to check collision against.
 * @param pos                           Actor position to check.
 * @param radius                        Actor radius.
 * @param dir                           Movement direction as a unit vector in fixpoint format.
 * @param out_move_distance[out]        Move distance along the `dir` axis required to touch the provided polygon.
 *                                      Always non-negative. This parameter is not set if the function returns false.
 *                                      Note that "touching" in this context means that the distance from the actor's
 *                                      center to the polygon equals actor's radius.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
 * @return                              Whether the actor, basically modeled as a sphere, can actually collide with the
 *                                      polygon if moving along the `dir` axis.
 */
static bool CollideSphereWithFace(BLVFace *face, const Vec3_float_ &pos, float radius, const Vec3_float_ &dir,
                                  float *out_move_distance, bool ignore_ethereal, int model_idx) {
    if (ignore_ethereal && face->Ethereal())
        return false;

    // dot_product(dir, normal) is a cosine of an angle between them.
    float cos_dir_normal = Dot(dir, face->pFacePlane.vNormal);

    float pos_face_distance = face->pFacePlane.SignedDistanceTo(pos);

    // How deep into the model that the face belongs to we already are,
    // positive value => actor's sphere already intersects the model.
    float overshoot = -pos_face_distance + radius;
    float move_distance = 0;
    if (abs(overshoot) < radius) {
        // We got here => we're not that deep into the model. Can just push us back a little.
        move_distance = 0;
    } else {
        // We got here => we're already inside the model. Or way outside.
        // We just say we overshot by radius. No idea why.
        overshoot = radius;

        // Then this is a correction needed to bring us to the point where actor's sphere is just touching the face.
        move_distance = overshoot / cos_dir_normal;
    }

    Vec3_float_ new_pos =
        pos + move_distance * dir - overshoot * face->pFacePlane.vNormal;

    if (!face->Contains(ToIntVector(new_pos), model_idx))
        return false; // We've just managed to slide past the face, so pretend no collision happened.

    if (move_distance < 0) {
        *out_move_distance = 0;
    } else {
        *out_move_distance = move_distance;
    }

    return true;
}

/**
 * Original offset 0x475D85, 0x475F30 (two functions merged into one).
 *
 * @param face                          Polygon to check collision against.
 * @param pos                           Actor position to check.
 * @param dir                           Movement direction as a unit vector in fixpoint format.
 * @param out_move_distance[in,out]     Current movement distance along the `dir` axis. This parameter is not touched
 *                                      when the function returns false. If the function returns true, then the
 *                                      distance required to hit the polygon is stored here. Note that this effectively
 *                                      means that this function can only decrease `move_distance`, but never increase
 *                                      it.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
 * @return                              Whether the actor, modeled as a point, hits the provided polygon if moving from
 *                                      `pos` along the `dir` axis by at most `move_distance`.
 */
static bool CollidePointWithFace(BLVFace *face, const Vec3_float_ &pos, const Vec3_float_ &dir, float *out_move_distance,
                                 int model_idx) {
    // _fp suffix => that's a fixpoint number

    // dot_product(dir, normal) is a cosine of an angle between them.
    float cos_dir_normal = Dot(dir, face->pFacePlane.vNormal);

    if (FuzzyIsNull(cos_dir_normal))
        return false; // dir is perpendicular to face normal.

    if (face->uAttributes & FACE_ETHEREAL)
        return false;

    if (cos_dir_normal > 0 && !face->Portal())
        return false; // We're facing away && face is not a portal.

    float pos_face_distance = face->pFacePlane.SignedDistanceTo(pos);

    if (cos_dir_normal < 0 && pos_face_distance < 0)
        return false; // Facing towards the face but already inside the model.

    if (cos_dir_normal > 0 && pos_face_distance > 0)
        return false; // Facing away from the face and outside the model.

    // How far we need to move along the `dir` axis to hit face.
    float move_distance = -pos_face_distance / cos_dir_normal;

    Vec3_float_ new_pos = pos + move_distance * dir;

    if (move_distance > *out_move_distance)
        return false; // No correction needed.

    if (!face->Contains(ToIntVector(new_pos), model_idx))
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
static void CollideBodyWithFace(BLVFace *face, int face_pid, bool ignore_ethereal, int model_idx) {
    auto collide_once = [&](const Vec3_float_ &old_pos, const Vec3_float_ &new_pos, const Vec3_float_ &dir, int radius) {
        float distance_old = face->pFacePlane.SignedDistanceTo(old_pos);
        float distance_new = face->pFacePlane.SignedDistanceTo(new_pos);
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

    collide_once(
        ToFloatVector(collision_state.position_lo),
        ToFloatVector(collision_state.new_position_lo),
        ToFloatVectorFromFixpoint(collision_state.direction),
        collision_state.radius_lo);

    if(collision_state.check_hi)
        collide_once(
            ToFloatVector(collision_state.position_hi),
            ToFloatVector(collision_state.new_position_hi),
            ToFloatVectorFromFixpoint(collision_state.direction),
            collision_state.radius_hi);
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
static bool CollideWithCylinder(const Vec3_float_ &center_lo, float radius, float height, int pid, bool jagged_top) {
    BBox_float_ bbox;
    bbox.x1 = center_lo.x - radius;
    bbox.x2 = center_lo.x + radius;
    bbox.y1 = center_lo.y - radius;
    bbox.y2 = center_lo.y + radius;
    bbox.z1 = center_lo.z;
    bbox.z2 = center_lo.z + height;
    if (!collision_state.bbox.Intersects(bbox))
        return false;

    // dist vector points from position center into cylinder center.
    float dist_x = center_lo.x - collision_state.position_lo.x;
    float dist_y = center_lo.y - collision_state.position_lo.y;
    float sum_radius = collision_state.radius_lo + radius;

    // Area of the parallelogram formed by dist and collision_state.direction. Direction is a unit vector,
    // thus this actually is length(dist) * sin(dist, collision_state.direction).
    // This in turn is the distance from cylinder center to the line of actor's movement.
    Vec3_float_ dir = ToFloatVectorFromFixpoint(collision_state.direction);
    float closest_dist = dist_x * dir.y - dist_y * dir.x;
    if (abs(closest_dist) > sum_radius)
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

    DecorationDesc *desc = pDecorationList->GetDecoration(decor->uDecorationDescID);
    if (desc->CanMoveThrough())
        return;

    CollideWithCylinder(
        ToFloatVector(decor->vPosition), desc->uRadius, desc->uDecorationHeight, PID(OBJECT_Decoration, id), false);
}



/* =============================
 * Implementation of public API.
 */
bool CollisionState::PrepareAndCheckIfStationary(int dt) {
    this->speed = integer_sqrt(
        this->velocity.z * this->velocity.z +
        this->velocity.y * this->velocity.y +
        this->velocity.x * this->velocity.x);

    if (this->speed != 0) {
        this->direction.x = fixpoint_div(this->velocity.x, this->speed);
        this->direction.y = fixpoint_div(this->velocity.y, this->speed);
        this->direction.z = fixpoint_div(this->velocity.z, this->speed);
    } else {
        this->direction.x = 0;
        this->direction.y = 0;
        this->direction.z = 65536;
    }

    if (!dt)
        dt = pEventTimer->dt_fixpoint;

    this->move_distance = fixpoint_mul(dt, this->speed) - this->total_move_distance;
    if (this->move_distance <= 0)
        return true;

    this->new_position_hi.x = fixpoint_mul(this->move_distance, this->direction.x) + this->position_lo.x;
    this->new_position_lo.x = fixpoint_mul(this->move_distance, this->direction.x) + this->position_lo.x;

    this->new_position_hi.y = fixpoint_mul(this->move_distance, this->direction.y) + this->position_lo.y;
    this->new_position_lo.y = fixpoint_mul(this->move_distance, this->direction.y) + this->position_lo.y;

    this->new_position_hi.z = fixpoint_mul(this->move_distance, this->direction.z) + this->position_hi.z;
    this->new_position_lo.z = fixpoint_mul(this->move_distance, this->direction.z) + this->position_lo.z;

    this->bbox.x1 = std::min(this->position_lo.x, this->new_position_lo.x) - this->radius_lo;
    this->bbox.x2 = std::max(this->position_lo.x, this->new_position_lo.x) + this->radius_lo;
    this->bbox.y1 = std::min(this->position_lo.y, this->new_position_lo.y) - this->radius_lo;
    this->bbox.y2 = std::max(this->position_lo.y, this->new_position_lo.y) + this->radius_lo;
    this->bbox.z1 = std::min(this->position_lo.z, this->new_position_lo.z) - this->radius_lo;
    this->bbox.z2 = std::max(this->position_hi.z, this->new_position_hi.z) + this->radius_hi;

    this->pid = 0;
    this->adjusted_move_distance = 0xFFFFFFu;

    return  false;
}

void CollideIndoorWithGeometry(bool ignore_ethereal) {
    std::array<int, 10> pSectorsArray;
    pSectorsArray[0] = collision_state.uSectorID;
    int totalSectors = 1;

    // See if we're intersection portals. If we do, we need to add corresponding sectors to the sectors array.
    BLVSector *pSector = &pIndoor->pSectors[collision_state.uSectorID];
    for (int j = 0; j < pSector->uNumPortals; ++j) {
        BLVFace *pFace = &pIndoor->pFaces[pSector->pPortals[j]];
        if (!collision_state.bbox.Intersects(pFace->pBounding))
            continue;

        float distance = abs(pFace->pFacePlane.SignedDistanceTo(ToFloatVector(collision_state.position_lo)));
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
            if (face->Portal() || !collision_state.bbox.Intersects(face->pBounding))
                continue;

            int face_id = pSector->pFloors[j];
            if (face_id == collision_state.ignored_face_id)
                continue;

            CollideBodyWithFace(face, PID(OBJECT_BModel, face_id), ignore_ethereal, MODEL_INDOOR);
        }
    }
}

void CollideOutdoorWithModels(bool ignore_ethereal) {
    for (BSPModel &model : pOutdoor->pBModels) {
        if (!collision_state.bbox.Intersects(model.pBoundingBox))
            continue;

        for (ODMFace &mface : model.pFaces) {
            if (!collision_state.bbox.Intersects(mface.pBoundingBox))
                continue;

            // TODO: we should really either merge two face classes, or template the functions down the chain call here.
            BLVFace face;
            face.pFacePlane_old = mface.pFacePlaneOLD;
            face.pFacePlane = mface.pFacePlane;
            face.uAttributes = mface.uAttributes;
            face.pBounding = mface.pBoundingBox;
            face.zCalc = mface.zCalc;
            face.uPolygonType = (PolygonType)mface.uPolygonType;
            face.uNumVertices = mface.uNumVertices;
            face.resource = mface.resource;
            face.pVertexIDs = mface.pVertexIDs.data();

            if (face.Ethereal() || face.Portal()) // TODO: this doesn't respect ignore_ethereal parameter
                continue;

            int pid = PID(OBJECT_BModel, (mface.index | (model.index << 6)));
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
        uint16_t pid = pOutdoor->pFaceIDLIST[i];
        if (!pid)
            break;

        if (PID_TYPE(pid) != OBJECT_Decoration)
            continue;

        CollideWithDecoration(PID_ID(pid));
    }
}

bool CollideIndoorWithPortals() {
    int portal_id = 0;            // [sp+10h] [bp-4h]@15
    float min_move_distance = std::numeric_limits<float>::max();
    for (unsigned int i = 0; i < pIndoor->pSectors[collision_state.uSectorID].uNumPortals; ++i) {
        BLVFace *face = &pIndoor->pFaces[pIndoor->pSectors[collision_state.uSectorID].pPortals[i]];
        if (!collision_state.bbox.Intersects(face->pBounding))
            continue;

        float distance_lo_old = face->pFacePlane.SignedDistanceTo(ToFloatVector(collision_state.position_lo));
        float distance_lo_new = face->pFacePlane.SignedDistanceTo(ToFloatVector(collision_state.new_position_lo));
        float move_distance = collision_state.move_distance;
        if ((distance_lo_old < collision_state.radius_lo || distance_lo_new < collision_state.radius_lo) &&
            (distance_lo_old > -collision_state.radius_lo || distance_lo_new > -collision_state.radius_lo) &&
            CollidePointWithFace(face, ToFloatVector(collision_state.position_lo),
                ToFloatVector(collision_state.direction), &move_distance, MODEL_INDOOR) &&
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
        collision_state.adjusted_move_distance = 0xFFFFFFF;
        return false;
    }

    return true;
}

bool CollideWithActor(int actor_idx, int override_radius) {
    Actor *actor = &pActors[actor_idx];
    if (actor->uAIState == Removed || actor->uAIState == Dying || actor->uAIState == Disabled ||
        actor->uAIState == Dead || actor->uAIState == Summoned)
        return false;

    float radius = actor->uActorRadius;
    if (override_radius != 0)
        radius = override_radius;

    return CollideWithCylinder(
        ToFloatVector(actor->vPosition), radius, actor->uActorHeight, PID(OBJECT_Actor, actor_idx), true);
}

void _46ED8A_collide_against_sprite_objects(unsigned int _this) {
    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID == 0)
            continue;

        ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
        if (object->uFlags & OBJECT_DESC_NO_COLLISION)
            continue;

        // This code is very close to what we have in CollideWithCylinder, but factoring out common parts just
        // seemed not worth it.

        BBox_int_ bbox;
        bbox.x1 = pSpriteObjects[i].vPosition.x - object->uRadius;
        bbox.x2 = pSpriteObjects[i].vPosition.x + object->uRadius;
        bbox.y1 = pSpriteObjects[i].vPosition.y - object->uRadius;
        bbox.y2 = pSpriteObjects[i].vPosition.y + object->uRadius;
        bbox.z1 = pSpriteObjects[i].vPosition.z;
        bbox.z2 = pSpriteObjects[i].vPosition.z + object->uHeight;
        if (!collision_state.bbox.Intersects(bbox))
            continue;

        float dist_x = pSpriteObjects[i].vPosition.x - collision_state.position_lo.x;
        float dist_y = pSpriteObjects[i].vPosition.y - collision_state.position_lo.y;
        float sum_radius = object->uHeight + collision_state.radius_lo;

        Vec3_float_ dir = ToFloatVectorFromFixpoint(collision_state.direction);
        float closest_dist = dist_x * dir.y - dist_y * dir.x;
        if (abs(closest_dist) > sum_radius)
            continue;

        float dist_dot_dir = dist_x * dir.x + dist_y * dir.y;
        if (dist_dot_dir <= 0)
            continue;

        float closest_z = collision_state.position_lo.z + dir.z * dist_dot_dir;
        if (closest_z < bbox.z1 - collision_state.radius_lo || closest_z > bbox.z2 + collision_state.radius_lo)
            continue;

        if (dist_dot_dir < collision_state.adjusted_move_distance)
            sub_46DEF2(_this, i);
    }
}

void CollideWithParty(bool jagged_top) {
    CollideWithCylinder(ToFloatVector(pParty->vPosition), 2 * pParty->radius, pParty->uPartyHeight, 4, jagged_top);
}

