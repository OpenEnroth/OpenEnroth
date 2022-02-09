#include "Collisions.h"

#include <algorithm>

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
 * @param move_distance[out]            Move distance along the `dir` axis required to touch the provided polygon.
 *                                      Always non-negative. This parameter is not set if the function returns false.
 *                                      Note that "touching" in this context means that the distance from the actor's
 *                                      center to the polygon equals actor's radius.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param face_points                   Face vertices access function.
 * @return                              Whether the actor, basically modeled as a sphere, can actually collide with the
 *                                      polygon if moving along the `dir` axis.
 */
template<class FacePointAccessor>
static bool CollideSphereWithFace(BLVFace *face, const Vec3_int_ &pos, int radius, const Vec3_int_ &dir,
                                  int *move_distance, bool ignore_ethereal, const FacePointAccessor& face_points) {
    if (ignore_ethereal && face->Ethereal())
        return false;

    // _fp suffix => that's a fixpoint number

    // dot_product(dir, normal) is a cosine of an angle between them.
    int cos_dir_normal_fp =
        fixpoint_mul(dir.x, face->pFacePlane_old.vNormal.x) +
        fixpoint_mul(dir.y, face->pFacePlane_old.vNormal.y) +
        fixpoint_mul(dir.z, face->pFacePlane_old.vNormal.z);

    int pos_face_distance_fp = face->pFacePlane_old.SignedDistanceToAsFixpoint(pos.x, pos.y, pos.z);
    int radius_fp = radius << 16;

    int64_t overshoot;
    signed int move_distance_fp;

    // How deep into the model that the face belongs to we already are,
    // positive value => actor's sphere already intersects the model.
    int overshoot_fp = -pos_face_distance_fp + radius_fp;
    if (abs(overshoot_fp) < radius_fp) {
        // We got here => we're not that deep into the model. Can just push us back a little.
        overshoot = abs(overshoot_fp) >> 16;
        move_distance_fp = 0;
    } else {
        // We got here => we're already inside the model. Or way outside.
        int overshoot_x4 = abs(overshoot_fp) >> 14;
        if (overshoot_x4 > abs(cos_dir_normal_fp))
            return false; // Moving perpendicular to the plane is OK for some reason.

        // We just say we overshot by radius. No idea why.
        overshoot = radius;

        // Then this is a correction needed to bring us to the point where actor's sphere is just touching the face.
        move_distance_fp = fixpoint_div(overshoot_fp, cos_dir_normal_fp);
    }

    Vec3_short_ new_pos;
    new_pos.x = pos.x + ((fixpoint_mul(move_distance_fp, dir.x) - overshoot * face->pFacePlane_old.vNormal.x) >> 16);
    new_pos.y = pos.y + ((fixpoint_mul(move_distance_fp, dir.y) - overshoot * face->pFacePlane_old.vNormal.y) >> 16);
    new_pos.z = pos.z + ((fixpoint_mul(move_distance_fp, dir.z) - overshoot * face->pFacePlane_old.vNormal.z) >> 16);

    if (!IsProjectedPointInsideFace(face, new_pos, face_points))
        return false; // We've just managed to slide past the face, so pretend no collision happened.

    if (move_distance_fp < 0) {
        *move_distance = 0;
    } else {
        *move_distance = move_distance_fp >> 16;
    }

    return true;
}

/**
 * Original offset 0x475D85, 0x475F30 (two functions merged into one).
 *
 * @param face                          Polygon to check collision against.
 * @param pos                           Actor position to check.
 * @param dir                           Movement direction as a unit vector in fixpoint format.
 * @param move_distance[in,out]         Current movement distance along the `dir` axis. This parameter is not touched
 *                                      when the function returns false. If the function returns true, then the
 *                                      distance required to hit the polygon is stored here. Note that this effectively
 *                                      means that this function can only decrease `move_distance`, but never increase
 *                                      it.
 * @param face_points                   Face vertices access function.
 * @return                              Whether the actor, modeled as a point, hits the provided polygon if moving from
 *                                      `pos` along the `dir` axis by at most `move_distance`.
 */
template<class FacePointAccessor>
static bool CollidePointWithFace(BLVFace *face, const Vec3_int_ &pos, const Vec3_int_ &dir, int *move_distance,
                                 const FacePointAccessor &face_points) {
    // _fp suffix => that's a fixpoint number

    // dot_product(dir, normal) is a cosine of an angle between them.
    int cos_dir_normal_fp =
        fixpoint_mul(dir.x, face->pFacePlane_old.vNormal.x) +
        fixpoint_mul(dir.y, face->pFacePlane_old.vNormal.y) +
        fixpoint_mul(dir.z, face->pFacePlane_old.vNormal.z);

    if (cos_dir_normal_fp == 0)
        return false; // dir is perpendicular to face normal.

    if (face->uAttributes & FACE_ETHEREAL)
        return false;

    if (cos_dir_normal_fp > 0 && !face->Portal())
        return false; // We're facing away && face is not a portal.

    int pos_face_distance_fp = face->pFacePlane_old.SignedDistanceToAsFixpoint(pos);

    if (cos_dir_normal_fp < 0 && pos_face_distance_fp < 0)
        return false; // Facing towards the face but already inside the model.

    if (cos_dir_normal_fp > 0 && pos_face_distance_fp > 0)
        return false; // Facing away from the face and outside the model.

    int pos_face_distance_x4 = abs(-(pos_face_distance_fp)) >> 14;

    // How far we need to move along the `dir` axis to hit face.
    int move_distance_fp = fixpoint_div(-pos_face_distance_fp, cos_dir_normal_fp);

    Vec3_short_ new_pos;
    new_pos.x = pos.x + ((fixpoint_mul(move_distance_fp, dir.x) + 0x8000) >> 16);
    new_pos.y = pos.y + ((fixpoint_mul(move_distance_fp, dir.y) + 0x8000) >> 16);
    new_pos.z = pos.z + ((fixpoint_mul(move_distance_fp, dir.z) + 0x8000) >> 16);

    if (pos_face_distance_x4 > abs(cos_dir_normal_fp))
        return false; // Moving perpendicular to the plane.

    if (move_distance_fp > *move_distance << 16)
        return false; // No correction needed.

    if (!IsProjectedPointInsideFace(face, new_pos, face_points))
        return false;

    *move_distance = move_distance_fp >> 16;
    return true;
}

static bool CollidePointIndoorWithFace(BLVFace *face, const Vec3_int_ &pos, const Vec3_int_ &dir, int *move_distance) {
    return CollidePointWithFace(face, pos, dir, move_distance, [&](int index) -> const auto & {
        return pIndoor->pVertices[face->pVertexIDs[index]];
    });
}

/**
 * Original offset 0x475665, 0x4759C9 (two functions merged into one).
 *
 * @param face                          Face to check.
 * @param point                         Point to check.
 * @param face_points                   Face vertices access function.
 * @returns                             Projects the provided point and face onto the face's main plane (XY, YZ or ZX)
 *                                      and returns whether the resulting point lies inside the resulting polygon.
 */
template<class FacePointAccessor>
static bool IsProjectedPointInsideFace(BLVFace *face, const Vec3_short_ &point, const FacePointAccessor& face_points) {
    if (face->uNumVertices <= 0)
        return false;

    std::array<int16_t, 104> edges_u;
    std::array<int16_t, 104> edges_v;

    int u;
    int v;
    if (face->uAttributes & FACE_XY_PLANE) {
        u = point.x;
        v = point.y;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[i] = face_points(i).x;
            edges_v[i] = face_points(i).y;
        }
    } else if (face->uAttributes & FACE_XZ_PLANE) {
        u = point.x;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[i] = face_points(i).x;
            edges_v[i] = face_points(i).z;
        }
    } else {
        u = point.y;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[i] = face_points(i).y;
            edges_v[i] = face_points(i).z;
        }
    }

    // The polygons we're dealing with are convex, so instead of the usual ray casting algorithm we can simply
    // check that the point in question lies on the same side relative to all of the polygon's edges.
    int sign = 0;
    for (int i = 0; i < face->uNumVertices; i++) {
        int j = (i + 1) % face->uNumVertices;

        int a_u = edges_u[j] - edges_u[i];
        int a_v = edges_v[j] - edges_v[i];
        int b_u = u - edges_u[i];
        int b_v = v - edges_v[i];
        int cross_product = a_u * b_v - a_v * b_u; // That's |a| * |b| * sin(a,b)
        if (cross_product == 0)
            continue;

        int cross_sign = static_cast<int>(cross_product > 0) * 2 - 1;

        if (sign == 0) {
            sign = cross_sign;
        } else if (sign != cross_sign) {
            return false;
        }
    }
    return true;
}

/**
 * Helper function that performs several collision checks between both the "feet" and the "head" spheres of the
 * collision state, and the provided face.
 *
 * @param face                          Face to check.
 * @param face_pid                      Pid of the provided face.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param face_points                   Face vertices access function.
*/
template<class FacePointAccessor>
static void CollideBodyWithFace(BLVFace *face, int face_pid, bool ignore_ethereal,
                                const FacePointAccessor &face_points) {
    auto collide_once = [&](const Vec3_int_ &old_pos, const Vec3_int_ &new_pos, const Vec3_int_ &dir, int radius) {
        int distance_old = face->pFacePlane_old.SignedDistanceTo(old_pos);
        int distance_new = face->pFacePlane_old.SignedDistanceTo(new_pos);
        if (distance_old > 0 && (distance_old <= radius || distance_new <= radius) && distance_new <= distance_old) {
            bool have_collision = false;
            int move_distance = collision_state.move_distance;
            if (CollideSphereWithFace(face, old_pos, radius, dir, &move_distance, ignore_ethereal, face_points)) {
                have_collision = true;
            } else {
                move_distance = collision_state.move_distance + radius;
                if (CollidePointWithFace(face, old_pos, dir, &move_distance, face_points)) {
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

    collide_once(collision_state.position_lo, collision_state.new_position_lo, collision_state.direction,
                 collision_state.radius_lo);

    if(collision_state.check_hi)
        collide_once(collision_state.position_hi, collision_state.new_position_hi, collision_state.direction,
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
static bool CollideWithCylinder(const Vec3_int_ &center_lo, int radius, int height, int pid, bool jagged_top) {
    BBox_int_ bbox;
    bbox.x1 = center_lo.x - radius;
    bbox.x2 = center_lo.x + radius;
    bbox.y1 = center_lo.y - radius;
    bbox.y2 = center_lo.y + radius;
    bbox.z1 = center_lo.z;
    bbox.z2 = center_lo.z + height;
    if (!collision_state.bbox.Intersects(bbox))
        return false;

    // dist vector points from position center into cylinder center.
    int dist_x = center_lo.x - collision_state.position_lo.x;
    int dist_y = center_lo.y - collision_state.position_lo.y;
    int sum_radius = collision_state.radius_lo + radius;

    // Area of the parallelogram formed by dist and collision_state.direction. Direction is a unit vector,
    // thus this actually is length(dist) * sin(dist, collision_state.direction).
    // This in turn is the distance from cylinder center to the line of actor's movement.
    int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
    if (abs(closest_dist) > sum_radius)
        return false; // No chance to collide.

    // Length of dist vector projected onto collision_state.direction.
    int dist_dot_dir = (dist_x * collision_state.direction.x + dist_y * collision_state.direction.y) >> 16;
    if (dist_dot_dir <= 0)
        return false; // We're moving away from the cylinder.

    // Z-coordinate of the actor at the point closest to the cylinder in XY plane.
    int closest_z = collision_state.position_lo.z + fixpoint_mul(collision_state.direction.z, dist_dot_dir);
    if (closest_z < bbox.z1 || (closest_z > bbox.z2 && !jagged_top))
        return false;

    // That's how far can we go along the collision_state.direction axis until the actor touches the cylinder,
    // i.e. distance between them goes below sum_radius.
    int move_distance = dist_dot_dir - integer_sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
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

    CollideWithCylinder(decor->vPosition, desc->uRadius, desc->uDecorationHeight, PID(OBJECT_Decoration, id), false);
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

        int distance = abs(pFace->pFacePlane_old.SignedDistanceTo(collision_state.position_lo));
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

            auto face_points = [&](int index) -> const auto & {
                return pIndoor->pVertices[face->pVertexIDs[index]];
            };
            CollideBodyWithFace(face, PID(OBJECT_BModel, face_id), ignore_ethereal, face_points);
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

            BLVFace face;
            face.pFacePlane_old = mface.pFacePlaneOLD;
            face.uAttributes = mface.uAttributes;
            face.pBounding = mface.pBoundingBox;
            face.zCalc1 = mface.zCalc1;
            face.zCalc2 = mface.zCalc2;
            face.zCalc3 = mface.zCalc3;
            face.pXInterceptDisplacements = mface.pXInterceptDisplacements;
            face.pYInterceptDisplacements = mface.pYInterceptDisplacements;
            face.pZInterceptDisplacements = mface.pZInterceptDisplacements;
            face.uPolygonType = (PolygonType)mface.uPolygonType;
            face.uNumVertices = mface.uNumVertices;
            face.resource = mface.resource;
            face.pVertexIDs = mface.pVertexIDs;

            if (face.Ethereal() || face.Portal()) // TODO: this doesn't respect ignore_ethereal parameter
                continue;

            auto face_points = [&](int index) -> const auto &{
                return pOutdoor->pBModels[model.index].pVertices.pVertices[face.pVertexIDs[index]];
            };

            int pid = PID(OBJECT_BModel, (mface.index | (model.index << 6)));
            CollideBodyWithFace(&face, pid, ignore_ethereal, face_points);
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
    uint16_t *pid_list = &pOutdoor->pFaceIDLIST[pOutdoor->pOMAP[grid_index]];
    if (!pid_list)
        return;

    for(; *pid_list != 0; pid_list++) {
        uint16_t pid = *pid_list;
        if (PID_TYPE(pid) != OBJECT_Decoration)
            continue;

        CollideWithDecoration(PID_ID(pid));
    }
}

bool CollideIndoorWithPortals() {
    int portal_id = 0;            // [sp+10h] [bp-4h]@15
    unsigned int min_move_distance = 0xFFFFFF;
    for (unsigned int i = 0; i < pIndoor->pSectors[collision_state.uSectorID].uNumPortals; ++i) {
        BLVFace *face = &pIndoor->pFaces[pIndoor->pSectors[collision_state.uSectorID].pPortals[i]];
        if (!collision_state.bbox.Intersects(face->pBounding))
            continue;

        int distance_lo_old = face->pFacePlane_old.SignedDistanceTo(collision_state.position_lo);
        int distance_lo_new = face->pFacePlane_old.SignedDistanceTo(collision_state.new_position_lo);
        int move_distance = collision_state.move_distance;
        if ((distance_lo_old < collision_state.radius_lo || distance_lo_new < collision_state.radius_lo) &&
            (distance_lo_old > -collision_state.radius_lo || distance_lo_new > -collision_state.radius_lo) &&
            CollidePointIndoorWithFace(face, collision_state.position_lo, collision_state.direction, &move_distance) &&
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

    int radius = actor->uActorRadius;
    if (override_radius != 0)
        radius = override_radius;

    return CollideWithCylinder(actor->vPosition, radius, actor->uActorHeight, PID(OBJECT_Actor, actor_idx), true);
}

void _46ED8A_collide_against_sprite_objects(unsigned int _this) {
    for (uint i = 0; i < uNumSpriteObjects; ++i) {
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

        int dist_x = pSpriteObjects[i].vPosition.x - collision_state.position_lo.x;
        int dist_y = pSpriteObjects[i].vPosition.y - collision_state.position_lo.y;
        int sum_radius = object->uHeight + collision_state.radius_lo;

        int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
        if (abs(closest_dist) > sum_radius)
            continue;

        int dist_dot_dir = (dist_x * collision_state.direction.x + dist_y * collision_state.direction.y) >> 16;
        if (dist_dot_dir <= 0)
            continue;

        int closest_z = collision_state.position_lo.z + fixpoint_mul(collision_state.direction.z, dist_dot_dir);
        if (closest_z < bbox.z1 - collision_state.radius_lo || closest_z > bbox.z2 + collision_state.radius_lo)
            continue;

        if (dist_dot_dir < collision_state.adjusted_move_distance)
            sub_46DEF2(_this, i);
    }
}

void CollideWithParty(bool jagged_top) {
    CollideWithCylinder(pParty->vPosition, 2 * pParty->radius, pParty->uPartyHeight, 4, jagged_top);
}

