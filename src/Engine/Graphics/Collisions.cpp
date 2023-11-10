#include "Collisions.h"

#include <algorithm>
#include <limits>
#include <utility>

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
#include "Engine/Engine.h"

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
 * @param p1                            Starting point of line.
 * @param p2                            End point of line.
 * @param radius                        Radius to use.
 * @param currentmovedist               Current largest movement distance before a collision.
 * @param[out] newmovedist              Move distance along the `dir` axis required to touch the provided line with provided radius.
 *                                      Always non-negative. This parameter is not set if the function returns false.
 * @param[out] intersection             How far along the line p1->p2 the collision will occur in the range [0 - 1]. Not set if the function
 *                                      returns false.
 * @param inside                        Whether collisions should happen when inside radius
 *
 * @return                              Whether the sphere of radius at position of collision state 'lo', can collide with the
 *                                      line p1 to p2 if moving along the `dir` axis AND the distance required to move for that
 *                                      collision is less than the current distance.
 */
static bool CollideWithLine(const Vec3f p1, const Vec3f p2, const float radius, const float currentmovedist, float* newmovedist, float* intersection, bool inside) {
    Vec3f pos = collision_state.position_lo;
    Vec3f dir = collision_state.direction;
    Vec3f edge = p2 - p1;
    Vec3f spherepostovertex = p1 - pos;
    float edgelengthsqr = edge.lengthSqr();
    float edgedotdir = dot(edge, dir);
    float edgedotspherepostovertex = dot(edge, spherepostovertex);
    float spherepostovertexlengthsqr = spherepostovertex.lengthSqr();

    // distance from pos to line p1->p2 = ||(p1 - pos) X (p2 - p1)|| / ||(p2 - p1)||
    // but our pos is moving  pos = startpos + direction * distance
    // at collision, distance from pos to line will be radius
    // square for simplicty - expand with vector quadruple product
    // rearrange with respect to distance to form Ax^2 + Bx + C = 0

    float a = edgelengthsqr * -dir.lengthSqr() + (edgedotdir * edgedotdir);
    float b = edgelengthsqr * (2.0f * dot(dir, spherepostovertex)) - (2.0f * edgedotdir * edgedotspherepostovertex);
    float c = edgelengthsqr * (radius * radius - spherepostovertexlengthsqr) + (edgedotspherepostovertex * edgedotspherepostovertex);

    if (hasShorterSolution(a, b, c, currentmovedist, newmovedist, inside)) {
        // Collision point will be perpendicular to edge
        // Project the position at point of collision onto the edge
        float f = (edgedotdir * *newmovedist - edgedotspherepostovertex) / edgelengthsqr;
        // is the collision within the points of the line
        if (f >= 0.0f && f <= 1.0f) {
            *intersection = f;
            return true;
        }
    }

    return false;
}

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
 * @param[out] out_collision_point      Point at which collision between sphere and face occurs.
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 * @param model_idx                     Model index, or `MODEL_INDOOR`.
 * @return                              Whether the actor, basically modeled as a sphere, can actually collide with the
 *                                      polygon if moving along the `dir` axis.
 */
static bool CollideSphereWithFace(BLVFace* face, const Vec3f& pos, float radius, const Vec3f& dir,
    float* out_move_distance, Vec3f* out_collision_point, bool ignore_ethereal, int model_idx) {
    if (ignore_ethereal && face->Ethereal())
        return false;

    if (face->uNumVertices < 3)
        return false; // Apparently this happens.

    float dir_normal_projection = dot(dir, face->facePlane.normal);
    // This is checked by the caller, we should be moving into the face or sideways, so projection of dir onto the
    // face normal should either be negative or close to zero. IE never collide with rear of face
    assert(dir_normal_projection < COLLISIONS_EPS);

    if (dir_normal_projection > 0.0f) {
        // hitting backface
        return false;
    }

    float center_face_distance = face->facePlane.signedDistanceTo(pos);
    float move_distance = 0.0f;
    Vec3f projected_pos = pos;
    bool sphereinplane = false;
    if (fuzzyIsNull(dir_normal_projection, COLLISIONS_EPS)) {
        if (fabs(center_face_distance) >= radius) {
            return false; // can never hit face
        } else {
            sphereinplane = true; // Sphere is already touching the infinite plane
        }
    } else {
        // how far do we need to move the sphere to touch infinite plane
        move_distance = (center_face_distance - radius) / -dir_normal_projection;
        if (move_distance < -100.0f) {
            // this can happen when we are already closer than the radius
            return false;
        }
        projected_pos += move_distance * dir - radius * face->facePlane.normal;
    }

    if (!sphereinplane) {
        // projected pos of collsion should now be on the faceplace
        assert(fuzzyIsNull(face->facePlane.signedDistanceTo(projected_pos), COLLISIONS_EPS)); // TODO(captainurist): move into face->Contains.

        // collision point is in face so can return
        if (face->Contains(projected_pos.toInt(), model_idx)) {
            *out_move_distance = move_distance;
            *out_collision_point = projected_pos;
            //logger->warning("Error: collide with face md: {}", move_distance);
            return true;
        }
    }

    // We may not be colliding with the surface of the face but could still be hitting its vertices or edges
    float a, b, c;
    float startingDist = *out_move_distance, newDist = 0.0f;
    Vec3f new_collision_pos;
    bool collidingwithface = false;

    // now collide with vertices - point sphere collision
    a = dir.lengthSqr();
    for (int i = 0; i < face->uNumVertices; ++i) {
        Vec3f vertpos;
        if (model_idx == MODEL_INDOOR) {
            vertpos = pIndoor->pVertices[face->pVertexIDs[i]].toFloat();
        } else {
            vertpos = pOutdoor->pBModels[model_idx].pVertices[face->pVertexIDs[i]].toFloat();
        }

        b = 2.0f * (dot(dir, pos - vertpos));
        c = (vertpos - pos).lengthSqr() - radius * radius;

        if (hasShorterSolution(a, b, c, startingDist, &newDist)) {
            startingDist = newDist;
            collidingwithface = true;
            new_collision_pos = vertpos;
        }
    }

    // now collide with edges
    for (int i = 0; i < face->uNumVertices; ++i) {
        Vec3f vert1, vert2;
        int i2 = (i + 1) % face->uNumVertices;
        if (model_idx == MODEL_INDOOR) {
            vert1 = pIndoor->pVertices[face->pVertexIDs[i]].toFloat();
            vert2 = pIndoor->pVertices[face->pVertexIDs[i2]].toFloat();
        } else {
            vert1 = pOutdoor->pBModels[model_idx].pVertices[face->pVertexIDs[i]].toFloat();
            vert2 = pOutdoor->pBModels[model_idx].pVertices[face->pVertexIDs[i2]].toFloat();
        }

        // collide with line between the two verts
        float intersectiondist;
        if (CollideWithLine(vert1, vert2, radius, startingDist, &newDist, &intersectiondist, false)) {
            startingDist = newDist;
            collidingwithface = true;
            new_collision_pos = vert1 + intersectiondist * (vert2 - vert1);
        }
    }

    if (collidingwithface) {
        *out_move_distance = startingDist;
        *out_collision_point = new_collision_pos;
        return true;
    }

    return false; // No collision happened.
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
            Vec3f col_pos;
            if (CollideSphereWithFace(face, old_pos, radius, dir, &move_distance, &col_pos, ignore_ethereal, model_idx)) {
                have_collision = true;
            } else {
                move_distance = collision_state.move_distance + radius;
                if (CollidePointWithFace(face, old_pos, dir, &move_distance, model_idx)) {
                    have_collision = true;
                    col_pos = move_distance * dir + old_pos;
                    move_distance -= radius;
                }
            }

            if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                collision_state.adjusted_move_distance = move_distance;
                collision_state.collisionPos = col_pos;
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

    float dist_x = center_lo.x - collision_state.position_lo.x;
    float dist_y = center_lo.y - collision_state.position_lo.y;
    Vec3f dir = collision_state.direction;

    //// Length of dist vector projected onto collision_state.direction.
    float dist_dot_dir = dist_x * dir.x + dist_y * dir.y;
    if (dist_dot_dir <= 0.0f) {
        return false; // We're moving away from the cylinder.
    }

    Vec3f pos = collision_state.position_lo;
    radius += collision_state.radius_lo;
    // add radius to treat bottom of collison state as flat
    Vec3f vert1 = center_lo, vert2 = center_lo + Vec3f(0, 0, height + collision_state.radius_lo);

    float newdist, intersection;
    if (CollideWithLine(vert1, vert2, radius, collision_state.adjusted_move_distance, &newdist, &intersection, true)) {
        // form a normal on the cylinder for use later
        Vec3f newpos = collision_state.position_lo + dir * newdist;
        Vec3f dir = center_lo - newpos;
        dir.normalize();
        Vec3f colpos = newpos + dir * collision_state.radius_lo;
        collision_state.collisionPos = colpos;
        Vec3f colnorm = Vec3f(-dir.x, -dir.y, 0);
        colnorm.normalize();
        collision_state.collisionNorm = colnorm;

        // set collision paramas
        collision_state.adjusted_move_distance = newdist;
        collision_state.pid = pid;

        return true;
    }

    return false;
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
    // Why x2? on radius?? - vanilla behaviour
    CollideWithCylinder(pParty->pos, 2 * pParty->radius, pParty->height, Pid::character(0), jagged_top);
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
                if (actor.monsterInfo.uHostilityType == HOSTILITY_FRIENDLY || isInCrowd)
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

        actor.pos = newPos.toInt();
        actor.sectorId = collision_state.uSectorID;
        if (fuzzyEquals(collision_state.adjusted_move_distance, collision_state.move_distance))
            break; // No collisions happened.

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.uHostilityType == HOSTILITY_FRIENDLY;
                bool otherFriendly = pActors[id].monsterInfo.uHostilityType == HOSTILITY_FRIENDLY;
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
            if (actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
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
                actor.speed += (velocityDotNormal * face->facePlane.normal).toInt();
                if (face->uPolygonType != POLYGON_InBetweenFloorAndWall && face->uPolygonType != POLYGON_Floor) {
                    float overshoot = collision_state.radius_lo - face->facePlane.signedDistanceTo(actor.pos.toFloat());
                    if (overshoot > 0)
                        actor.pos += (overshoot * pIndoor->pFaces[id].facePlane.normal).toInt();
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

        actor.pos = newPos.toInt();
        if (fuzzyEquals(collision_state.adjusted_move_distance, collision_state.move_distance))
            break; // No collision happened.

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.uHostilityType == HOSTILITY_FRIENDLY;
                bool otherFriendly = pActors[id].monsterInfo.uHostilityType == HOSTILITY_FRIENDLY;
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
            if (actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
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

                    actor.speed += (velocityDotNormal * face->facePlane.normal).toInt();
                    if (face->uPolygonType != POLYGON_InBetweenFloorAndWall) {
                        float overshoot = collision_state.radius_lo - face->facePlane.signedDistanceTo(actor.pos.toFloat());
                        if (overshoot > 0)
                            actor.pos += (overshoot * face->facePlane.normal).toInt();
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

void ProcessPartyCollisionsBLV(int sectorId, int min_party_move_delta_sqr, int *faceId, int *faceEvent) {
    constexpr float closestdist = 1.5f; // Closest allowed approach to collision surface - needs adjusting

    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius;
    collision_state.check_hi = true;
    for (uint i = 0; i < 5; i++) {
        collision_state.position_hi = pParty->pos + Vec3f(0, 0, pParty->height - collision_state.radius_lo);
        collision_state.position_lo = pParty->pos + Vec3f(0, 0, collision_state.radius_lo);
        collision_state.velocity = pParty->speed;

        collision_state.uSectorID = sectorId;
        int dt = 0; // zero means use actual dt
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            dt = 13312; // fixpoint(13312) = 0.203125

        if (collision_state.PrepareAndCheckIfStationary(dt))
            break;

        for (uint j = 0; j < 100; ++j) {
            CollideIndoorWithGeometry(true);
            CollideIndoorWithDecorations();
            for (int k = 0; k < pActors.size(); ++k)
                CollideWithActor(k, 0);
            if (CollideIndoorWithPortals())
                break; // No portal collisions => can break.
        }

        Vec3f adjusted_pos;
        // Set new position but moved back slightly so we never touch the face
        adjusted_pos = pParty->pos + (collision_state.adjusted_move_distance - closestdist) * collision_state.direction;
        // Adjust the collision position with the same offset
        collision_state.collisionPos -= closestdist * collision_state.direction;

        int adjusted_floor_z = GetIndoorFloorZ((adjusted_pos + Vec3f(0, 0, 40)).toInt(), &collision_state.uSectorID, faceId);
        if (adjusted_floor_z == -30000 || adjusted_floor_z - pParty->pos.z > 128) {
            // intended world position isnt valid so dont move there
            int testadjusted_floor_z = GetIndoorFloorZ(pParty->pos.toInt(), &collision_state.uSectorID, faceId);
            return; // TODO: whaaa?
        }

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            pParty->pos = (collision_state.new_position_lo - Vec3f(0, 0, collision_state.radius_lo));
            break; // And we're done with collisions.
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;

        pParty->pos.x = adjusted_pos.x;
        pParty->pos.y = adjusted_pos.y;
        float new_party_z_tmp = adjusted_pos.z;

        if (collision_state.pid.type() == OBJECT_Actor) {
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset(); // Break invisibility when running into a monster.
        }

        if (collision_state.pid.type() == OBJECT_Decoration) {
            // TODO(pskelton): common to odm/blv so extract
            Vec3f newdirection;
            if (collision_state.adjusted_move_distance > 0.0f) {
                // Create new sliding plane from collisio
                Vec3f slideplaneorigin = collision_state.collisionPos;
                Vec3f slideplanenormal = collision_state.collisionNorm;
                float slideplanedist = -(dot(slideplaneorigin, slideplanenormal));

                // Form a sliding vector that is parallel to sliding movement
                // Take where you wouldve ended up without collisions and move that onto the slide plane by adding the normal
                // Start point to new destination is a vector along the slide plane
                float destplanedist = dot(collision_state.new_position_lo, slideplanenormal) + slideplanedist;
                Vec3f newdestination = collision_state.new_position_lo - destplanedist * slideplanenormal;
                newdirection = newdestination - collision_state.collisionPos;
                newdirection.z = 0;
                newdirection.normalize();
            }

            // Set party to move along this new sliding vector
            pParty->speed = newdirection * dot(newdirection, pParty->speed);
            // Skip reducing party speed
            continue;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            BLVFace *pFace = &pIndoor->pFaces[collision_state.pid.id()];
            bool bFaceSlopeTooSteep = pFace->facePlane.normal.z > 0.0f && pFace->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            // new sliding plane
            Vec3f slideplaneorigin = collision_state.collisionPos;
            Vec3f slideplanenormal = adjusted_pos + Vec3f(0, 0, collision_state.radius_lo) - slideplaneorigin;
            slideplanenormal.normalize();
            float slideplanedist = -(dot(slideplaneorigin, slideplanenormal));
            float distfromdestpointtoplane = dot(collision_state.new_position_lo, slideplanenormal) + slideplanedist;
            Vec3f newdestination = collision_state.new_position_lo - distfromdestpointtoplane * slideplanenormal;
            Vec3f newdirection = newdestination - collision_state.collisionPos;

            // Cant push uphill on steep faces
            if (bFaceSlopeTooSteep && newdirection.z > 0)
                newdirection.z = 0;

            newdirection.normalize();

            // Push away from the surface and add a touch down for better slide
            if (bFaceSlopeTooSteep)
                pParty->speed += Vec3f(pFace->facePlane.normal.x, pFace->facePlane.normal.y, -2) * 10;

            // set movement speed along sliding plane
            pParty->speed = newdirection * dot(newdirection, pParty->speed);

            if (pParty->floor_face_id != collision_state.pid.id() && pFace->Pressure_Plate())
                *faceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;

            if (pFace->uPolygonType == POLYGON_Floor) {
                new_party_z_tmp = pIndoor->pVertices[*pFace->pVertexIDs].z + 1;
                if (pParty->uFallStartZ - new_party_z_tmp < 512)
                    pParty->uFallStartZ = new_party_z_tmp;
            }
            continue;
        }

        // ~0.9x reduce party speed and try again
        pParty->speed *= 0.89263916f; // was 58500 fp
    }
}

void ProcessPartyCollisionsODM(Vec3f *partyNewPos, Vec3f *partyInputSpeed, bool *partyIsOnWater, int *floorFaceId, bool *partyNotOnModel, bool *partyHasHitModel, int *triggerID) {
    constexpr float closestdist = 0.5f;  // Closest allowed approach to collision surface - needs adjusting

    // --(Collisions)-------------------------------------------------------------------
    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius;
    collision_state.check_hi = true;

    // make 5 attempts to satisfy collisions
    for (uint i = 0; i < 5; i++) {
        collision_state.position_hi = *partyNewPos + Vec3f(0, 0, pParty->height - collision_state.radius_lo);
        collision_state.position_lo = *partyNewPos + Vec3f(0, 0, collision_state.radius_lo);
        collision_state.velocity = *partyInputSpeed;
        collision_state.uSectorID = 0;

        int frame_movement_dt = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            frame_movement_dt = 13312;
        if (collision_state.PrepareAndCheckIfStationary(frame_movement_dt)) {
            break;
        }

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(WorldPosToGridCellX(pParty->pos.x), WorldPosToGridCellY(pParty->pos.y));
        _46ED8A_collide_against_sprite_objects(Pid::character(0));
        for (size_t actor_id = 0; actor_id < pActors.size(); ++actor_id)
            CollideWithActor(actor_id, 0);

        Vec3f newPosLow = {};
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            // Moved far enough so reset foot position for exit
            newPosLow.x = collision_state.new_position_lo.x;
            newPosLow.y = collision_state.new_position_lo.y;
            newPosLow.z = collision_state.new_position_lo.z - collision_state.radius_lo;
        } else {
            // Set new position but moved back slightly so we never touch the face
            newPosLow = *partyNewPos + (collision_state.adjusted_move_distance - closestdist) * collision_state.direction;
            // Adjust the collision position with the same offset
            collision_state.collisionPos -= closestdist * collision_state.direction;
        }

        int allnewfloor = ODM_GetFloorLevel(newPosLow.toInt(), pParty->height, partyIsOnWater, floorFaceId, 0);
        int party_y_pid;
        int x_advance_floor = ODM_GetFloorLevel(Vec3i(newPosLow.x, partyNewPos->y, newPosLow.z), pParty->height, partyIsOnWater, &party_y_pid, 0);
        int party_x_pid;
        int y_advance_floor = ODM_GetFloorLevel(Vec3i(partyNewPos->x, newPosLow.y, newPosLow.z), pParty->height, partyIsOnWater, &party_x_pid, 0);
        bool terr_slope_advance_x = IsTerrainSlopeTooHigh(newPosLow.x, partyNewPos->y);
        bool terr_slope_advance_y = IsTerrainSlopeTooHigh(partyNewPos->x, newPosLow.y);

        *partyNotOnModel = false;
        if (!party_y_pid && !party_x_pid && !*floorFaceId) *partyNotOnModel = true;

        bool move_in_y = true;
        bool move_in_x = true;
        if (engine->IsUnderwater() || !*partyNotOnModel) {
            partyNewPos->x = newPosLow.x;
            partyNewPos->y = newPosLow.y;
        } else {
            if (terr_slope_advance_x && x_advance_floor > partyNewPos->z) move_in_x = false;
            if (terr_slope_advance_y && y_advance_floor > partyNewPos->z) move_in_y = false;

            if (move_in_x) {
                partyNewPos->x = newPosLow.x;
                if (move_in_y) partyNewPos->y = newPosLow.y;
            } else if (move_in_y) {
                partyNewPos->y = newPosLow.y;
            } else {
                if (IsTerrainSlopeTooHigh(newPosLow.x, newPosLow.y) && allnewfloor <= partyNewPos->z) {
                    // move down the hill is allowed
                    partyNewPos->x = newPosLow.x;
                    partyNewPos->y = newPosLow.y;
                }
            }
        }

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            if (!*partyNotOnModel) {
                partyNewPos->x = collision_state.new_position_lo.x;
                partyNewPos->y = collision_state.new_position_lo.y;
            }
            partyNewPos->z = collision_state.new_position_lo.z - collision_state.radius_lo;
            break;
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        *partyNewPos = newPosLow;

        if (collision_state.pid.type() == OBJECT_Actor) {
            if (pParty->Invisible())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (collision_state.pid.type() == OBJECT_Decoration) {
            // TODO(pskelton): common to odm/blv so extract
            Vec3f newdirection;
            if (collision_state.adjusted_move_distance > 0.0f) {
                // Create new sliding plane from collisio
                Vec3f slideplaneorigin = collision_state.collisionPos;
                Vec3f slideplanenormal = collision_state.collisionNorm;
                float slideplanedist = -(dot(slideplaneorigin, slideplanenormal));

                // Form a sliding vector that is parallel to sliding movement
                // Take where you wouldve ended up without collisions and move that onto the slide plane by adding the normal
                // Start point to new destination is a vector along the slide plane
                float destplanedist = dot(collision_state.new_position_lo, slideplanenormal) + slideplanedist;
                Vec3f newdestination = collision_state.new_position_lo - destplanedist * slideplanenormal;
                newdirection = newdestination - collision_state.collisionPos;
                newdirection.z = 0;
                newdirection.normalize();
            }

            // Set party to move along this new sliding vector
            *partyInputSpeed = newdirection * dot(newdirection, *partyInputSpeed);
            // Skip reducing party speed
            continue;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            const ODMFace* pODMFace = &pOutdoor->face(collision_state.pid);
            bool bFaceSlopeTooSteep = pODMFace->facePlane.normal.z > 0.0f && pODMFace->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            if (pODMFace->facePlane.normal.z > 0 && !bFaceSlopeTooSteep)
                *partyHasHitModel = true;

            if (engine->IsUnderwater())
                bFaceSlopeTooSteep = false;

            if (pParty->floor_face_id != collision_state.pid.id() && pODMFace->Pressure_Plate()) {
                pParty->floor_face_id = collision_state.pid.id();
                *triggerID = pODMFace->sCogTriggeredID;  // this one triggers tour events / traps
            }

            // new sliding plane
            Vec3f slideplaneorigin = collision_state.collisionPos;
            Vec3f slideplanenormal = newPosLow + Vec3f(0, 0, collision_state.radius_lo) - slideplaneorigin;
            slideplanenormal.normalize();
            float slideplanedist = -(dot(slideplaneorigin, slideplanenormal));
            float distfromdestpointtoplane = dot(collision_state.new_position_lo, slideplanenormal) + slideplanedist;
            Vec3f newdestination = collision_state.new_position_lo - distfromdestpointtoplane * slideplanenormal;
            Vec3f newdirection = newdestination - collision_state.collisionPos;

            // Cant push uphill on steep faces
            if (bFaceSlopeTooSteep && newdirection.z > 0)
                newdirection.z = 0;

            newdirection.normalize();

            // Push away from the surface and add a touch down for better slide
            if (bFaceSlopeTooSteep)
                *partyInputSpeed += Vec3f(pODMFace->facePlane.normal.x, pODMFace->facePlane.normal.y, -2) * 10;

            // set movement speed along sliding plane
            *partyInputSpeed = newdirection * dot(newdirection, *partyInputSpeed);

            if (pODMFace->uPolygonType == POLYGON_Floor || pODMFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                pParty->bFlying = false;
                pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
            }

            continue;
        }

        // ~0.9x reduce party speed and try again
        *partyInputSpeed *= 0.89263916f; // was 58500 fp
    }
}


bool hasShorterSolution(const float a, const float b, const float c, const float curSoln, float* outNewSoln, bool inside) {
    float d = b * b - 4.0f * a * c;
    if (d < 0.0f) {
        return false;  // no real solutions - No intersection points.
    }

    d = sqrt(d);
    float alpha1 = (-b - d) / (2 * a);
    float alpha2 = (-b + d) / (2 * a);

    // Deal with smaller solution first
    if (alpha1 > alpha2) {
        std::swap(alpha1, alpha2);
    }
    // May be negative if the collision point was behind us - so ignore
    if (alpha1 > 0.0f && alpha1 < curSoln) {
        *outNewSoln = alpha1;
        return true;  // this new collision is shorter than old
    }

    if (inside) {
        // We are inside and colliding - for cylinder
        if (alpha1 < 0.0f && alpha2 >= 0.0f) {
            *outNewSoln = 0.0f;
            return true;
        }
    }

    if (alpha2 > 0.0f && alpha2 < curSoln) {
        *outNewSoln = alpha2;
        return true;  // this new collision is shorter than old
    }

    // No valid or better solutions
    return false;
}
