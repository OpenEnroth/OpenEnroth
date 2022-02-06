#include "Collisions.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/MM7.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

CollisionState collision_state;

/*
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
    std::array<int16_t, 104> edges_u;
    std::array<int16_t, 104> edges_v;

    int u;
    int v;
    if (face->uAttributes & FACE_XY_PLANE) {
        u = point.x;
        v = point.y;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] + face_points(i).x;
            edges_v[2 * i] = face->pYInterceptDisplacements[i] + face_points(i).y;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] + face_points(i + 1).x;
            edges_v[2 * i + 1] = face->pYInterceptDisplacements[i + 1] + face_points(i + 1).y;
        }
    } else if (face->uAttributes & FACE_XZ_PLANE) {
        u = point.x;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] + face_points(i).x;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] + face_points(i).z;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] + face_points(i + 1).x;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] + face_points(i + 1).z;
        }
    } else {
        u = point.y;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pYInterceptDisplacements[i] + face_points(i).y;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] + face_points(i).z;
            edges_u[2 * i + 1] = face->pYInterceptDisplacements[i + 1] + face_points(i + 1).y;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] + face_points(i + 1).z;
        }
    }
    edges_u[2 * face->uNumVertices] = edges_u[0];
    edges_v[2 * face->uNumVertices] = edges_v[0];

    if (2 * face->uNumVertices <= 0)
        return 0;

    int counter = 0;
    for (int i = 0; i < 2 * face->uNumVertices; ++i) {
        if (counter >= 2)
            break;

        // Check that we're inside the bounding band in v coordinate
        if ((edges_v[i] >= v) == (edges_v[i + 1] >= v))
            continue;

        // If we're to the left then we surely have an intersection
        if ((edges_u[i] >= u) && (edges_u[i + 1] >= u)) {
            ++counter;
            continue;
        }

        // We're not to the left? Then we must be inside the bounding band in u coordinate
        if ((edges_u[i] >= u) == (edges_u[i + 1] >= u))
            continue;

        // Calculate the intersection point of v=const line with the edge.
        int line_intersection_u = edges_u[i] +
            static_cast<__int64>(edges_u[i + 1] - edges_u[i]) * (v - edges_v[i]) / (edges_v[i + 1] - edges_v[i]);

        // We need ray intersections, so consider only one halfplane.
        if (line_intersection_u >= u)
            ++counter;
    }

    return counter == 1;
}

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

    if(collision_state.check_hi & 1)
        collide_once(collision_state.position_hi, collision_state.new_position_hi, collision_state.direction,
                     collision_state.radius_hi);
}

static void CollideWithDecoration(int id) {
    LevelDecoration *decor = &pLevelDecorations[id];
    if (decor->uFlags & LEVEL_DECORATION_INVISIBLE)
        return;

    DecorationDesc *decor_desc = pDecorationList->GetDecoration(decor->uDecorationDescID);
    if (decor_desc->CanMoveThrough())
        return;

    BBox_int_ bbox;
    bbox.x1 = decor->vPosition.x - decor_desc->uRadius;
    bbox.x2 = decor->vPosition.x + decor_desc->uRadius;
    bbox.y1 = decor->vPosition.y - decor_desc->uRadius;
    bbox.y2 = decor->vPosition.y + decor_desc->uRadius;
    bbox.z1 = decor->vPosition.z;
    bbox.z2 = decor->vPosition.z + decor_desc->uDecorationHeight;
    if (!collision_state.bbox.Intersects(bbox))
        return;

    // dist vector points from position center into decoration center.
    int dist_x = decor->vPosition.x - collision_state.position_lo.x;
    int dist_y = decor->vPosition.y - collision_state.position_lo.y;
    int sum_radius = collision_state.radius_lo + decor_desc->uRadius;

    // Area of the parallelogram formed by dist and collision_state.direction. Direction is a unit vector,
    // thus this actually is length(dist) * sin(dist, collision_state.direction).
    // This in turn is the distance from decoration center to the line of actor's movement.
    int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
    if (abs(closest_dist) > sum_radius)
        return; // No chance to collide.

    // Length of dist vector projected onto collision_state.direction.
    int dist_dot_dir = (dist_x * collision_state.direction.x + dist_y * collision_state.direction.y) >> 16;
    if (dist_dot_dir <= 0)
        return; // We're moving away from the decoration.

    // Z-coordinate of the actor at the point closest to the decoration in XY plane.
    int closest_z = collision_state.position_lo.z + fixpoint_mul(collision_state.direction.z, dist_dot_dir);
    if (closest_z < bbox.z1 || closest_z > bbox.z2)
        return;

    // That's how far can we go along the collision_state.direction axis until the actor touches the decoration,
    // i.e. distance between them goes below sum_radius.
    int move_distance = dist_dot_dir - integer_sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
    if (move_distance < 0)
        move_distance = 0;

    if (move_distance < collision_state.adjusted_move_distance) {
        collision_state.adjusted_move_distance = move_distance;
        collision_state.pid = PID(OBJECT_Decoration, id);
    }
}


/*
 * Implementation of public API.
 */

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
        unsigned __int16 pid = *pid_list;
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

    BBox_int_ bbox;
    bbox.x1 = actor->vPosition.x - radius;
    bbox.x2 = actor->vPosition.x + radius;
    bbox.y1 = actor->vPosition.y - radius;
    bbox.y2 = actor->vPosition.y + radius;
    bbox.z1 = actor->vPosition.z;
    bbox.z2 = actor->vPosition.z + actor->uActorHeight;

    if (!collision_state.bbox.Intersects(bbox))
        return false;

    // dist vector points from position center into actor's center.
    int dist_x = actor->vPosition.x - collision_state.position_lo.x;
    int dist_y = actor->vPosition.y - collision_state.position_lo.y;
    int sum_radius = collision_state.radius_lo + radius;

    // Distance from actor's center to the line of movement.
    int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
    if (abs(closest_dist) > sum_radius)
        return false; // No chance to collide.

    // Length of dist vector projected onto collision_state.direction.
    int dist_dot_dir = (dist_x * collision_state.direction.x + dist_y * collision_state.direction.y) >> 16;
    if (dist_dot_dir <= 0)
        return false; // We're moving away from the actor.

    // Z-coordinate at the point closest to the actor in XY plane.
    int closest_z = collision_state.position_lo.z + fixpoint_mul(collision_state.direction.z, dist_dot_dir);
    if (closest_z < actor->vPosition.z)
        return false; // We're below.
    // TODO: and where's a check for being above?

    int move_distance = dist_dot_dir - integer_sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
    if (move_distance < 0)
        move_distance = 0;

    if (move_distance < collision_state.adjusted_move_distance) {
        collision_state.adjusted_move_distance = move_distance;
        collision_state.pid = PID(OBJECT_Actor, actor_idx);
    }
    return true;
}

void _46ED8A_collide_against_sprite_objects(unsigned int _this) {
    ObjectDesc *object;  // edx@4
    int v10;             // ecx@12
    int v11;             // esi@13

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
            if (!(object->uFlags & OBJECT_DESC_NO_COLLISION)) {
                if (collision_state.bbox.x1 <= pSpriteObjects[i].vPosition.x + object->uRadius &&
                    collision_state.bbox.x2 >= pSpriteObjects[i].vPosition.x - object->uRadius &&
                    collision_state.bbox.y1 <= pSpriteObjects[i].vPosition.y + object->uRadius &&
                    collision_state.bbox.y2 >= pSpriteObjects[i].vPosition.y - object->uRadius &&
                    collision_state.bbox.z1 <= pSpriteObjects[i].vPosition.z + object->uHeight &&
                    collision_state.bbox.z2 >= pSpriteObjects[i].vPosition.z) {
                    if (abs(((pSpriteObjects[i].vPosition.x -
                              collision_state.position_lo.x) *
                                 collision_state.direction.y -
                             (pSpriteObjects[i].vPosition.y -
                              collision_state.position_lo.y) *
                                 collision_state.direction.x) >>
                            16) <=
                        object->uHeight + collision_state.radius_lo) {
                        v10 = ((pSpriteObjects[i].vPosition.x -
                                collision_state.position_lo.x) *
                                   collision_state.direction.x +
                               (pSpriteObjects[i].vPosition.y -
                                collision_state.position_lo.y) *
                                   collision_state.direction.y) >>
                              16;
                        if (v10 > 0) {
                            v11 = collision_state.position_lo.z +
                                  ((unsigned __int64)(collision_state.direction.z *
                                                      (signed __int64)v10) >>
                                   16);
                            if (v11 >= pSpriteObjects[i].vPosition.z -
                                           collision_state.radius_lo) {
                                if (v11 <= object->uHeight +
                                               collision_state.radius_lo +
                                               pSpriteObjects[i].vPosition.z) {
                                    if (v10 < collision_state.adjusted_move_distance) {
                                        sub_46DEF2(_this, i);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void _46EF01_collision_chech_player(bool infinite_height) {
    BBox_int_ bbox;
    bbox.x1 = pParty->vPosition.x - (2 * pParty->radius);
    bbox.x2 = pParty->vPosition.x + (2 * pParty->radius);
    bbox.y1 = pParty->vPosition.y - (2 * pParty->radius);
    bbox.y2 = pParty->vPosition.y + (2 * pParty->radius);
    bbox.z1 = pParty->vPosition.z;
    bbox.z2 = pParty->vPosition.z + pParty->uPartyHeight;
    if (!collision_state.bbox.Intersects(bbox))
        return;

    int sum_radius = collision_state.radius_lo + (2 * pParty->radius);
    int dist_x = pParty->vPosition.x - collision_state.position_lo.x;
    int dist_y = pParty->vPosition.y - collision_state.position_lo.y;

    int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
    if (abs(closest_dist) > sum_radius)
        return;

    int dist_dot_dir = (dist_y * collision_state.direction.y + dist_x * collision_state.direction.x) >> 16;
    if (dist_dot_dir <= 0)
        return;

    int closest_z = fixpoint_mul(collision_state.direction.z, dist_dot_dir) + collision_state.position_lo.z;
    if (closest_z < bbox.z1 || (closest_z > bbox.z2 && !infinite_height))
        return;

    int move_distance = dist_dot_dir - integer_sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
    if (move_distance < 0)
        move_distance = 0;

    if (move_distance < collision_state.adjusted_move_distance) {
        collision_state.adjusted_move_distance = move_distance;
        collision_state.pid = 4;
    }
}


