#include "Collisions.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "Engine/Evt/Processor.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/Random/Random.h"

#include "Utility/Math/Float.h"
#include "Utility/Math/TrigLut.h"

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
    Vec3f sphereToVertex = p1 - pos;
    float edgeLengthSqr = edge.lengthSqr();
    float edgeDotDir = dot(edge, dir);
    float edgeDotSphereToVertex = dot(edge, sphereToVertex);
    float sphereToVertexlengthsqr = sphereToVertex.lengthSqr();

    // distance from pos to line p1->p2 = ||(p1 - pos) X (p2 - p1)|| / ||(p2 - p1)||
    // but our pos is moving  pos = startpos + direction * distance
    // at collision, distance from pos to line will be radius
    // square for simplicty - expand with vector quadruple product
    // rearrange with respect to distance to form Ax^2 + Bx + C = 0

    float a = edgeLengthSqr * -dir.lengthSqr() + (edgeDotDir * edgeDotDir);
    float b = edgeLengthSqr * (2.0f * dot(dir, sphereToVertex)) - (2.0f * edgeDotDir * edgeDotSphereToVertex);
    float c = edgeLengthSqr * (radius * radius - sphereToVertexlengthsqr) + (edgeDotSphereToVertex * edgeDotSphereToVertex);

    if (hasShorterSolution(a, b, c, currentmovedist, newmovedist, inside)) {
        // Collision point will be perpendicular to edge
        // Project the position at point of collision onto the edge
        float f = (edgeDotDir * *newmovedist - edgeDotSphereToVertex) / edgeLengthSqr;
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
 * @param dir                           Movement direction as a unit vector.
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
    bool sphereInPlane = false;
    if (fuzzyIsNull(dir_normal_projection, COLLISIONS_EPS)) {
        if (fabs(center_face_distance) >= radius) {
            return false; // can never hit face
        } else {
            sphereInPlane = true; // Sphere is already touching the infinite plane
        }
    } else {
        // how far do we need to move the sphere to touch infinite plane
        move_distance = (center_face_distance - radius) / -dir_normal_projection;
        if (move_distance < -radius) {
            // this can happen when we are already closer than the radius
            // we are interested edge collisions up to the point where the sphere center is touching the plane
            return false;
        }
        if (move_distance > 65536.0f) return false; // moving almost parallal - TODO(pskelton): should probably tweak EPS when finished moving to floats
        projected_pos += move_distance * dir - radius * face->facePlane.normal;
    }

    if (!sphereInPlane) {
        // projected pos of collsion should now be on the faceplace
        assert(fuzzyIsNull(face->facePlane.signedDistanceTo(projected_pos), COLLISIONS_EPS)); // TODO(captainurist): move into face->Contains.

        // collision point is in face so can return
        if (face->Contains(projected_pos, model_idx)) {
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
    bool collidingWithFace = false;

    // now collide with vertices - point sphere collision
    a = dir.lengthSqr();
    for (int i = 0; i < face->uNumVertices; ++i) {
        Vec3f vertPos;
        if (model_idx == MODEL_INDOOR) {
            vertPos = pIndoor->vertices[face->pVertexIDs[i]];
        } else {
            vertPos = pOutdoor->pBModels[model_idx].vertices[face->pVertexIDs[i]];
        }

        b = 2.0f * (dot(dir, pos - vertPos));
        c = (vertPos - pos).lengthSqr() - radius * radius;

        if (hasShorterSolution(a, b, c, startingDist, &newDist)) {
            startingDist = newDist;
            collidingWithFace = true;
            new_collision_pos = vertPos;
        }
    }

    // now collide with edges
    for (int i = 0; i < face->uNumVertices; ++i) {
        Vec3f vert1, vert2;
        int i2 = (i + 1) % face->uNumVertices;
        if (model_idx == MODEL_INDOOR) {
            vert1 = pIndoor->vertices[face->pVertexIDs[i]];
            vert2 = pIndoor->vertices[face->pVertexIDs[i2]];
        } else {
            vert1 = pOutdoor->pBModels[model_idx].vertices[face->pVertexIDs[i]];
            vert2 = pOutdoor->pBModels[model_idx].vertices[face->pVertexIDs[i2]];
        }

        // collide with line between the two verts
        float intersectionDist;
        if (CollideWithLine(vert1, vert2, radius, startingDist, &newDist, &intersectionDist, false)) {
            startingDist = newDist;
            collidingWithFace = true;
            new_collision_pos = vert1 + intersectionDist * (vert2 - vert1);
        }
    }

    if (collidingWithFace) {
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
 * @param dir                           Movement direction as a unit vector.
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

    if (!face->Contains(new_pos, model_idx))
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
    auto collide_once = [&](const Vec3f &old_pos, const Vec3f &new_pos, const Vec3f &dir, int radius, float height) {
        float distance_old = face->facePlane.signedDistanceTo(old_pos);
        float distance_new = face->facePlane.signedDistanceTo(new_pos);
        if (distance_old > 0 && (distance_old <= radius || distance_new <= radius) && distance_new <= distance_old) {
            bool have_collision = false;
            float move_distance = collision_state.move_distance;
            Vec3f col_pos;
            if (CollideSphereWithFace(face, old_pos, radius, dir, &move_distance, &col_pos, ignore_ethereal, model_idx)) {
                have_collision = true;
            }

            if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                // We allow for a bit of negative movement in case we are already too close to the surface and need pushback
                if (move_distance > allowedCollisionOvershoot) {
                    collision_state.adjusted_move_distance = move_distance;
                    collision_state.collisionPos = col_pos;
                    collision_state.pid = face_pid;
                    collision_state.heightOffset = height;
                }
            }
        }
    };

    collide_once(collision_state.position_lo, collision_state.new_position_lo, collision_state.direction, collision_state.radius_lo, 0.0f);

    if (!collision_state.check_hi)
        return;

    collide_once(collision_state.position_hi, collision_state.new_position_hi, collision_state.direction, collision_state.radius_hi, collision_state.position_hi.z - collision_state.position_lo.z);

    // Test some more spheres in the middle of the body to catch some edge cases
    Vec3f midPos = (collision_state.position_lo + collision_state.position_hi) / 2;
    Vec3f newMidPos = (collision_state.new_position_lo + collision_state.new_position_hi) / 2;
    collide_once(midPos, newMidPos, collision_state.direction, collision_state.radius_hi, midPos.z - collision_state.position_lo.z);

    // Try and test the center of the face if its within our cylinder and not too close to the midpoint
    float zCent = face->pBounding.center().z;
    if (zCent > collision_state.position_lo.z && zCent < collision_state.position_hi.z && std::abs(midPos.z - zCent) > 10) {
        float diff = zCent - collision_state.position_lo.z;
        midPos.z = zCent;
        newMidPos.z = collision_state.new_position_lo.z + diff;
        collide_once(midPos, newMidPos, collision_state.direction, collision_state.radius_hi, diff);
    }
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
        Vec3f newPos = collision_state.position_lo + dir * newdist;
        Vec3f dirC = center_lo - newPos;
        dirC.normalize();
        Vec3f colPos = newPos + dirC * collision_state.radius_lo;
        collision_state.collisionPos = colPos;

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

    CollideWithCylinder(decor->vPosition, desc->uRadius, desc->uDecorationHeight, Pid(OBJECT_Decoration, id), false);
}


//
// Public API.
//

bool CollisionState::PrepareAndCheckIfStationary(Duration dt) {
    float dtf = dt ? dt.realtimeMillisecondsFloat() : pEventTimer->dt().realtimeMillisecondsFloat();

    this->speed = this->velocity.length();
    if (fuzzyIsNull(this->speed, COLLISIONS_EPS))
        return true;

    this->direction = this->velocity / this->speed;

    this->move_distance = dtf * this->speed - this->total_move_distance;
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
    BLVSector *pSector = &pIndoor->sectors[collision_state.uSectorID];
    for (uint16_t portalId : pSector->portalIds) {
        BLVFace *pFace = &pIndoor->faces[portalId];
        if (!collision_state.bbox.intersects(pFace->pBounding))
            continue;

        float distance = std::abs(pFace->facePlane.signedDistanceTo(collision_state.position_lo));
        if (distance > collision_state.move_distance + collision_state.radius_lo)
            continue;

        pSectorsArray[totalSectors++] =
            pFace->uSectorID == collision_state.uSectorID ? pFace->uBackSectorID : pFace->uSectorID;
        break;
    }

    for (int i = 0; i < totalSectors; i++) {
        pSector = &pIndoor->sectors[pSectorsArray[i]];
        for (uint16_t face_id : std::array{pSector->floorIds, pSector->wallIds, pSector->ceilingIds} | std::views::join) {
            BLVFace *face = &pIndoor->faces[face_id];
            if (face->isPortal() || !collision_state.bbox.intersects(face->pBounding))
                continue;

            // TODO(pskelton): Modify game data face attribs to ethereal eventually - hack so that secret tunnel under prison bed can be accessed
            if (engine->_currentLoadedMapId == MAP_CASTLE_HARMONDALE)
                if (face_id == 385 || face_id == 405 || face_id == 4602 || face_id == 4606)
                    continue;
            if (engine->_currentLoadedMapId == MAP_TEMPLE_OF_THE_LIGHT) // For #1706 glitch on waterway
                if (face_id == 1181)
                    continue;

            CollideBodyWithFace(face, Pid(OBJECT_Face, face_id), ignore_ethereal, MODEL_INDOOR);
        }
    }
}

void CollideOutdoorWithModels(bool ignore_ethereal) {
    for (BSPModel &model : pOutdoor->pBModels) {
        if (!collision_state.bbox.intersects(model.boundingBox))
            continue;

        for (ODMFace &mface : model.faces) {
            if (!collision_state.bbox.intersects(mface.boundingBox))
                continue;

            // TODO: we should really either merge two face classes, or template the functions down the chain call here.
            BLVFace face;
            face.FromODM(&mface);

            if (face.Ethereal() || face.isPortal()) // TODO: this doesn't respect ignore_ethereal parameter
                continue;

            Pid pid = Pid::odmFace(model.index, mface.index);
            CollideBodyWithFace(&face, pid, ignore_ethereal, model.index);
        }
    }
}

void CollideIndoorWithDecorations() {
    BLVSector *sector = &pIndoor->sectors[collision_state.uSectorID];
    for (uint16_t decorationId : sector->decorationIds)
        CollideWithDecoration(decorationId);
}

void CollideOutdoorWithDecorations(Vec2i gridPos) {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return;

    int grid_index = gridPos.x + (gridPos.y << 7);
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
    // TODO(pskelton): disable this for time being - this appears to be a obselete legacy collision remnant
    // Was meant to handle portal crossing to update sector id during movement
    // Causes issue where portal "collision" overrides actual wall collision
    return true;

    int portal_id = 0;            // [sp+10h] [bp-4h]@15
    float min_move_distance = std::numeric_limits<float>::max();
    for (uint16_t portalFaceId : pIndoor->sectors[collision_state.uSectorID].portalIds) {
        BLVFace *face = &pIndoor->faces[portalFaceId];
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
            portal_id = portalFaceId;
        }
    }

    if (collision_state.adjusted_move_distance >= min_move_distance && min_move_distance <= collision_state.move_distance) {
        if (pIndoor->faces[portal_id].uSectorID == collision_state.uSectorID) {
            collision_state.uSectorID = pIndoor->faces[portal_id].uBackSectorID;
        } else {
            collision_state.uSectorID = pIndoor->faces[portal_id].uSectorID;
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

    return CollideWithCylinder(actor->pos, radius, actor->height, Pid(OBJECT_Actor, actor_idx), true);
}

void _46ED8A_collide_against_sprite_objects(Pid pid) {
    for (unsigned i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID == 0)
            continue;

        ObjectDesc *object = &pObjectList->pObjects[pSpriteObjects[i].uObjectDescID];
        if (object->uFlags & OBJECT_DESC_NO_COLLISION)
            continue;

        // This code is very close to what we have in CollideWithCylinder, but factoring out common parts just
        // seemed not worth it.

        BBoxf bbox = BBoxf::forCylinder(pSpriteObjects[i].vPosition, object->uRadius, object->uHeight);
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
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = actor.radius;
    collision_state.radius_hi = actor.radius;
    collision_state.check_hi = true;
    collision_state.uSectorID = actor.sectorId;
    // Dont bother with hi check if lo radius covers actor height anyway
    if (actor.radius * 2 > actor.height) collision_state.check_hi = false;

    for (int attempt = 0; attempt < 5; attempt++) {
        collision_state.position_lo = actor.pos + Vec3f(0, 0, actor.radius);
        collision_state.position_hi = actor.pos + Vec3f(0, 0, actor.height - actor.radius);
        collision_state.velocity = actor.velocity;

        if (collision_state.PrepareAndCheckIfStationary())
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

        if (collision_state.adjusted_move_distance > collision_state.move_distance) collision_state.adjusted_move_distance = collision_state.move_distance;
        Vec3f adjusted_pos = actor.pos + (collision_state.adjusted_move_distance - closestdist) * collision_state.direction;
        collision_state.collisionPos -= closestdist * collision_state.direction;

        int newFaceID = -1;
        float newFloorZ = GetIndoorFloorZ(adjusted_pos, &collision_state.uSectorID, &newFaceID);

        if (newFloorZ == -30000 || newFloorZ - actor.pos.z > 128)
            break; // New pos is out of bounds, running more iterations won't help.

        if (pIndoor->faces[newFaceID].uAttributes & FACE_INDOOR_SKY) {
            if (actor.aiState == Dead) {
                actor.aiState = Removed;
                break; // Actor removed, no point in running more iterations.
            }

            if (!isAboveGround && !isFlying) {
                if (actor.monsterInfo.hostilityType == HOSTILITY_FRIENDLY || isInCrowd)
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0_ticks, nullptr);

                break; // Trying to walk into indoor sky, bad idea!
            }
        }

        // Prevent actors from falling off ledges.
        if (actor.currentActionAnimation == ANIM_Walking && newFloorZ < actor.pos.z - 100 && !isAboveGround && !isFlying) {
            if (grng->randomBool()) {
                actor.yawAngle += 100;
            } else {
                actor.yawAngle -= 100;
            }
            break; // We'll try again in the next frame.
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        actor.pos = adjusted_pos;
        actor.sectorId = collision_state.uSectorID;

        // break if weve moved far enough now
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) break; // And we're done with collisions.

        // React to collisions
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.hostilityType == HOSTILITY_FRIENDLY;
                bool otherFriendly = pActors[id].monsterInfo.hostilityType == HOSTILITY_FRIENDLY;
                if (isInCrowd) {
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0_ticks, nullptr);
                } else if (isFriendly && otherFriendly) {
                    Actor::AI_FaceObject(actor.id, collision_state.pid, nullptr);
                } else {
                    Actor::AI_Flee(actor.id, collision_state.pid, 0_ticks, nullptr);
                }
            }
        }

        if (type == OBJECT_Character) {
            if (actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
                actor.velocity.y = 0;
                actor.velocity.x = 0;

                if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                }
            } else {
                Actor::AI_FaceObject(actor.id, collision_state.pid, nullptr);
            }
        }

        if (type == OBJECT_Decoration) {
            int speed = integer_sqrt(actor.velocity.x * actor.velocity.x + actor.velocity.y * actor.velocity.y);
            int angle = TrigLUT.atan2(actor.pos.x - pLevelDecorations[id].vPosition.x, actor.pos.y - pLevelDecorations[id].vPosition.y); // Face away from the decoration.
            actor.velocity.x = TrigLUT.cos(angle) * speed;
            actor.velocity.y = TrigLUT.sin(angle) * speed;
        }

        if (type == OBJECT_Face) {
            BLVFace *face = &pIndoor->faces[id];
            bool bFaceSlopeTooSteep = face->facePlane.normal.z > 0.0f && face->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            // TODO(pskelton): Do actors need same exclusions as party?

            // TODO(pskelton): This 'catch all' is probably unsafe - would be better as above
            if (bFaceSlopeTooSteep && face->Invisible() && face->uPolygonType == PolygonType::POLYGON_InBetweenFloorAndWall)
                bFaceSlopeTooSteep = false;

            // new sliding plane - drag collision down to correct level for slide direction
            Vec3f slidePlaneOrigin = collision_state.collisionPos - Vec3f(0, 0, collision_state.heightOffset);
            Vec3f slidePlaneNormal = adjusted_pos + Vec3f(0, 0, collision_state.radius_lo) - slidePlaneOrigin;
            slidePlaneNormal.normalize();
            float destPlaneDist = dot(collision_state.new_position_lo - slidePlaneOrigin, slidePlaneNormal);
            Vec3f newDestination = collision_state.new_position_lo - destPlaneDist * slidePlaneNormal;
            Vec3f newDirection = newDestination - slidePlaneOrigin;

            // Cant push uphill on steep faces
            if (bFaceSlopeTooSteep && newDirection.z > 0)
                newDirection.z = 0;

            newDirection.normalize();

            // Push away from the surface and add a touch down for better slide
            if (bFaceSlopeTooSteep)
                actor.velocity += Vec3f(face->facePlane.normal.x, face->facePlane.normal.y, -2) * 10;

            // set movement speed along sliding plane
            actor.velocity = newDirection * dot(newDirection, actor.velocity);

            if (pIndoor->faces[id].uAttributes & FACE_TriggerByMonster)
                eventProcessor(pIndoor->faceExtras[pIndoor->faces[id].uFaceExtraID].uEventID, Pid(), 1);

            if (pIndoor->faces[id].uPolygonType == POLYGON_Floor) {
                float new_floor_z_tmp = pIndoor->vertices[*face->pVertexIDs].z;
                // We dont collide with the rear of faces so hitting a floor poly with upwards direction means that
                // weve collided with its edge and we should step up onto its level.
                if (actor.velocity.z > 0.0f && (new_floor_z_tmp - actor.pos.z) < 128)
                    actor.pos.z = new_floor_z_tmp;

                if (actor.velocity.lengthSqr() < 400) {
                    actor.velocity.x = 0;
                    actor.velocity.y = 0;
                }
            }
        }

        actor.velocity *= 0.89263916f; // was 58500 fp
    }
}

void ProcessActorCollisionsODM(Actor &actor, bool isFlying) {
    int actorRadius = !isFlying ? 40 : actor.radius;

    collision_state.total_move_distance = 0;
    collision_state.check_hi = true;
    collision_state.radius_hi = actorRadius;
    collision_state.radius_lo = actorRadius;

    for (int attempt = 0; attempt < 100; ++attempt) {
        collision_state.position_lo = actor.pos + Vec3f(0, 0, actorRadius + 1);
        collision_state.position_hi = actor.pos + Vec3f(0, 0, actor.height - actorRadius - 1);
        collision_state.position_hi.z = std::max(collision_state.position_hi.z, collision_state.position_lo.z);
        collision_state.velocity = actor.velocity;
        collision_state.uSectorID = 0;
        if (collision_state.PrepareAndCheckIfStationary())
            break;

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(worldToGrid(actor.pos));
        CollideWithParty(false);
        _46ED8A_collide_against_sprite_objects(Pid(OBJECT_Actor, actor.id));

        int actorCollisions = 0;
        for (int i = 0; i < ai_arrays_size; i++)
            if (ai_near_actors_ids[i] != actor.id && CollideWithActor(ai_near_actors_ids[i], 40))
                actorCollisions++;
        int isInCrowd = actorCollisions > 1;

        //if (collision_state.adjusted_move_distance < collision_state.move_distance)
        //    Slope_High = collision_state.adjusted_move_distance * collision_state.direction.z;

        Vec3f newPos = actor.pos + collision_state.adjusted_move_distance * collision_state.direction;
        bool isOnWater = false;
        int modelPid = 0;
        float newFloorZ = ODM_GetFloorLevel(newPos, &isOnWater, &modelPid);
        if (isOnWater) {
            if (actor.pos.z < newFloorZ + 60) {
                if (actor.aiState == Dead || actor.aiState == Dying ||
                    actor.aiState == Removed || actor.aiState == Disabled) {
                    SpriteObject::createSplashObject(Vec3f(actor.pos.x, actor.pos.y, modelPid ? newFloorZ + 30 : newFloorZ + 60));
                    actor.aiState = Removed;
                    break;
                }
            }
        }

        actor.pos = newPos;
        if (fuzzyEquals(collision_state.adjusted_move_distance, collision_state.move_distance))
            break; // No collision happened.

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        int id = collision_state.pid.id();
        ObjectType type = collision_state.pid.type();

        if (type == OBJECT_Actor) {
            if (!pParty->bTurnBasedModeOn || (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT)) {
                bool isFriendly = actor.monsterInfo.hostilityType == HOSTILITY_FRIENDLY;
                bool otherFriendly = pActors[id].monsterInfo.hostilityType == HOSTILITY_FRIENDLY;
                if (isInCrowd) {
                    Actor::AI_StandOrBored(actor.id, Pid(OBJECT_Character, 0), 0_ticks, nullptr);
                } else if (isFriendly && otherFriendly) {
                    Actor::AI_FaceObject(actor.id, collision_state.pid, nullptr);
                } else {
                    Actor::AI_Flee(actor.id, collision_state.pid, 0_ticks, nullptr);
                }
            }
        }

        if (type == OBJECT_Character) {
            if (actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
                actor.velocity.y = 0;
                actor.velocity.x = 0;

                if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                }
            } else {
                Actor::AI_FaceObject(actor.id, collision_state.pid, nullptr);
            }
        }

        if (type == OBJECT_Decoration) {
            int speed = integer_sqrt(actor.velocity.x * actor.velocity.x + actor.velocity.y * actor.velocity.y);
            int angle = TrigLUT.atan2(actor.pos.x - pLevelDecorations[id].vPosition.x, actor.pos.y - pLevelDecorations[id].vPosition.y);
            actor.velocity.x = TrigLUT.cos(angle) * speed;
            actor.velocity.y = TrigLUT.sin(angle) * speed;
        }

        if (type == OBJECT_Face) {
            const ODMFace *face = &pOutdoor->face(collision_state.pid);

            if (!face->Ethereal()) {
                if (face->polygonType == POLYGON_Floor) {
                    if (actor.velocity.z < 0) actor.velocity.z = 0;
                    actor.pos.z = newFloorZ;
                    if (actor.velocity.lengthSqr() < 400) {
                        actor.velocity.y = 0;
                        actor.velocity.x = 0;
                    }
                } else {
                    float velocityDotNormal = dot(face->facePlane.normal, actor.velocity);
                    velocityDotNormal = std::max(std::abs(velocityDotNormal), collision_state.speed / 8);
                    actor.velocity += velocityDotNormal * face->facePlane.normal;

                    if (face->polygonType != POLYGON_InBetweenFloorAndWall) {
                        float overshoot = collision_state.radius_lo - face->facePlane.signedDistanceTo(actor.pos);
                        if (overshoot > 0)
                            actor.pos += overshoot * face->facePlane.normal;
                        actor.yawAngle = TrigLUT.atan2(actor.velocity.x, actor.velocity.y);
                    }
                }
            }
        }

        actor.velocity *= 0.89263916f; // was 58500 fp
    }
}

void ProcessPartyCollisionsBLV(int sectorId, int min_party_move_delta_sqr, int *faceId, int *faceEvent) {
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius;
    collision_state.check_hi = true;
    collision_state.uSectorID = sectorId;
    for (unsigned i = 0; i < 5; i++) {
        collision_state.position_lo = pParty->pos + Vec3f(0, 0, collision_state.radius_lo);
        collision_state.position_hi = pParty->pos + Vec3f(0, 0, pParty->height - collision_state.radius_lo);
        collision_state.velocity = pParty->velocity;

        Duration dt; // zero means use actual dt
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            dt = 26_ticks;

        if (collision_state.PrepareAndCheckIfStationary(dt))
            break;

        for (unsigned j = 0; j < 100; ++j) {
            CollideIndoorWithGeometry(true);
            CollideIndoorWithDecorations();
            // TODO(captainurist): why there is no call to _46ED8A_collide_against_sprite_objects?
            //                     See ProcessPartyCollisionsODM.
            // pskelton - probably because there are no/ very few sprite objects in BLV. The only ones i can think of are the trees in the fairy hill.
            if (!engine->config->gameplay.NoPartyActorCollisions.value()) {
                for (int k = 0; k < pActors.size(); ++k)
                    CollideWithActor(k, 0);
            }
            if (CollideIndoorWithPortals())
                break; // No portal collisions => can break.
        }

        // Set new position but moved back slightly so we never touch the face
        if (collision_state.adjusted_move_distance > collision_state.move_distance) {
            collision_state.adjusted_move_distance = collision_state.move_distance;
        }
        Vec3f adjusted_pos = pParty->pos + (collision_state.adjusted_move_distance - closestdist) * collision_state.direction;
        // Adjust the collision position with the same offset
        collision_state.collisionPos -= closestdist * collision_state.direction;

        float adjusted_floor_z = GetIndoorFloorZ(adjusted_pos + Vec3f(0, 0, collision_state.radius_lo), &collision_state.uSectorID, faceId);
        if (adjusted_floor_z == -30000 || adjusted_floor_z - pParty->pos.z > 128) {
            // intended world position isnt valid so dont move there
            return; // TODO: whaaa?
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        pParty->pos = adjusted_pos;
        pBLVRenderParams->uPartySectorID = collision_state.uSectorID;
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            break; // And we're done with collisions.
        }

        if (collision_state.pid.type() == OBJECT_Actor) {
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset(); // Break invisibility when running into a monster.
        }

        if (collision_state.pid.type() == OBJECT_Decoration) {
            // TODO(pskelton): common to odm/blv so extract
            Vec3f newDirection;
            if (collision_state.adjusted_move_distance > 0.0f) {
                // Create new sliding plane from collision
                Vec3f slidePlaneOrigin = collision_state.collisionPos;
                Vec3f dirC = pLevelDecorations[collision_state.pid.id()].vPosition - slidePlaneOrigin;
                Vec3f slidePlaneNormal = Vec3f(-dirC.x, -dirC.y, 0);
                slidePlaneNormal.normalize();

                // Form a sliding vector that is parallel to sliding movement
                // Take where you wouldve ended up without collisions and move that onto the slide plane by adding the normal
                // Start point to new destination is a vector along the slide plane
                float destPlaneDist = dot(collision_state.new_position_lo - slidePlaneOrigin, slidePlaneNormal);
                Vec3f newDestination = collision_state.new_position_lo - destPlaneDist * slidePlaneNormal;
                newDirection = newDestination - collision_state.collisionPos;
                newDirection.z = 0;
                newDirection.normalize();
            }

            // Set party to move along this new sliding vector
            pParty->velocity = newDirection * dot(newDirection, pParty->velocity);
            // Skip reducing party speed
            continue;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            BLVFace *pFace = &pIndoor->faces[collision_state.pid.id()];
            bool bFaceSlopeTooSteep = pFace->facePlane.normal.z > 0.0f && pFace->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            // TODO(pskelton): Better way to do this? Maybe add a climbable attribute
            if (engine->_currentLoadedMapId == MAP_TIDEWATER_CAVERNS) {  // Special case for steep staircase in tidewater
                if (collision_state.pid.id() == 650)
                    bFaceSlopeTooSteep = false;
            }
            if (engine->_currentLoadedMapId == MAP_CASTLE_GLOAMING) { // Special case for exiting teleport boats
                if (collision_state.pid.id() == 551 || collision_state.pid.id() == 1990 || collision_state.pid.id() == 2217)
                    bFaceSlopeTooSteep = false;
            }
            if (engine->_currentLoadedMapId == MAP_CASTLE_HARMONDALE) {
                if (collision_state.pid.id() == 398) // Secret tunnel under prison bed
                    bFaceSlopeTooSteep = false;
            }
            if (engine->_currentLoadedMapId == MapId::MAP_HALL_OF_THE_PIT) {
                if (collision_state.pid.id() == 787 || collision_state.pid.id() == 832 || collision_state.pid.id() == 790)
                    bFaceSlopeTooSteep = false;
            }
            if (engine->_currentLoadedMapId == MAP_CASTLE_GLOAMING) {
                if (collision_state.pid.id() == 2439 || collision_state.pid.id() == 2438 || collision_state.pid.id() == 2437 || collision_state.pid.id() == 2436) // gloaming
                    bFaceSlopeTooSteep = false;
            }

            // TODO(pskelton): This 'catch all' is probably unsafe - would be better as above
            if (bFaceSlopeTooSteep && pFace->Invisible() && pFace->uPolygonType == PolygonType::POLYGON_InBetweenFloorAndWall)
                bFaceSlopeTooSteep = false;

            // new sliding plane - drag collision down to correct level for slide direction
            Vec3f slidePlaneOrigin = collision_state.collisionPos - Vec3f(0, 0, collision_state.heightOffset);
            Vec3f slidePlaneNormal = adjusted_pos + Vec3f(0, 0, collision_state.radius_lo) - slidePlaneOrigin;
            slidePlaneNormal.normalize();
            float destPlaneDist = dot(collision_state.new_position_lo - slidePlaneOrigin, slidePlaneNormal);
            Vec3f newDestination = collision_state.new_position_lo - destPlaneDist * slidePlaneNormal;
            Vec3f newDirection = newDestination - slidePlaneOrigin;

            // Cant push uphill on steep faces
            if (bFaceSlopeTooSteep && newDirection.z > 0)
                newDirection.z = 0;

            newDirection.normalize();

            // Push away from the surface and add a touch down for better slide
            if (bFaceSlopeTooSteep)
                pParty->velocity += Vec3f(pFace->facePlane.normal.x, pFace->facePlane.normal.y, -2) * 10;

            // set movement speed along sliding plane
            pParty->velocity = newDirection * dot(newDirection, pParty->velocity);

            if (pParty->floor_face_id != collision_state.pid.id() && pFace->Pressure_Plate())
                *faceEvent = pIndoor->faceExtras[pFace->uFaceExtraID].uEventID;

            if (pFace->uPolygonType == POLYGON_Floor) {
                float new_party_z_tmp = pIndoor->vertices[*pFace->pVertexIDs].z;
                // We dont collide with the rear of faces so hitting a floor poly with upwards direction means that
                // weve collided with its edge and we should step up onto its level.
                if (pParty->velocity.z > 0.0f && (new_party_z_tmp - pParty->pos.z) < 128)
                    pParty->pos.z = new_party_z_tmp;
                if (pParty->uFallStartZ - new_party_z_tmp < 512)
                    pParty->uFallStartZ = new_party_z_tmp;
            }
            continue;
        }

        // ~0.9x reduce party speed and try again
        pParty->velocity *= 0.89263916f; // was 58500 fp
    }
}

void ProcessPartyCollisionsODM(Vec3f *partyNewPos, Vec3f *partyInputSpeed, int *floorFaceId, bool *partyNotOnModel, bool *partyHasHitModel, int *triggerID) {
    // --(Collisions)-------------------------------------------------------------------
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius;
    collision_state.check_hi = true;

    // make 5 attempts to satisfy collisions
    for (unsigned i = 0; i < 5; i++) {
        collision_state.position_hi = *partyNewPos + Vec3f(0, 0, pParty->height - collision_state.radius_lo);
        collision_state.position_lo = *partyNewPos + Vec3f(0, 0, collision_state.radius_lo);
        collision_state.velocity = *partyInputSpeed;
        collision_state.uSectorID = 0;

        Duration frame_movement_dt;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            frame_movement_dt = 26_ticks;
        if (collision_state.PrepareAndCheckIfStationary(frame_movement_dt)) {
            break;
        }

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(worldToGrid(pParty->pos));
        _46ED8A_collide_against_sprite_objects(Pid::character(0));
        if (!engine->config->gameplay.NoPartyActorCollisions.value()) {
            for (size_t actor_id = 0; actor_id < pActors.size(); ++actor_id)
                CollideWithActor(actor_id, 0);
        }

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

        bool isOnWater = false;
        float allnewfloor = ODM_GetFloorLevel(newPosLow, &isOnWater, floorFaceId);
        int party_y_pid;
        float x_advance_floor = ODM_GetFloorLevel(Vec3f(newPosLow.x, partyNewPos->y, newPosLow.z), &isOnWater, &party_y_pid);
        int party_x_pid;
        float y_advance_floor = ODM_GetFloorLevel(Vec3f(partyNewPos->x, newPosLow.y, newPosLow.z), &isOnWater, &party_x_pid);
        bool terr_slope_advance_x = pOutdoor->pTerrain.isSlopeTooHighByPos(Vec3f(newPosLow.x, partyNewPos->y, 0.0f));
        bool terr_slope_advance_y = pOutdoor->pTerrain.isSlopeTooHighByPos(Vec3f(partyNewPos->x, newPosLow.y, 0.0f));

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
                if (pOutdoor->pTerrain.isSlopeTooHighByPos(newPosLow) && allnewfloor <= partyNewPos->z) {
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
            Vec3f newDirection;
            if (collision_state.adjusted_move_distance > 0.0f) {
                // Create new sliding plane from collision
                Vec3f slidePlaneOrigin = collision_state.collisionPos;
                Vec3f dirC = pLevelDecorations[collision_state.pid.id()].vPosition - slidePlaneOrigin;
                Vec3f slidePlaneNormal = Vec3f(-dirC.x, -dirC.y, 0);
                slidePlaneNormal.normalize();

                // Form a sliding vector that is parallel to sliding movement
                // Take where you wouldve ended up without collisions and move that onto the slide plane by adding the normal
                // Start point to new destination is a vector along the slide plane
                float destPlaneDist = dot(collision_state.new_position_lo - slidePlaneOrigin, slidePlaneNormal);
                Vec3f newDestination = collision_state.new_position_lo - destPlaneDist * slidePlaneNormal;
                newDirection = newDestination - collision_state.collisionPos;
                newDirection.z = 0;
                newDirection.normalize();
            }

            // Set party to move along this new sliding vector
            *partyInputSpeed = newDirection * dot(newDirection, *partyInputSpeed);
            // Skip reducing party speed
            continue;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            const ODMFace* pODMFace = &pOutdoor->face(collision_state.pid);
            bool bFaceSlopeTooSteep = pODMFace->facePlane.normal.z > 0.0f && pODMFace->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            if (bFaceSlopeTooSteep) { // make small slopes walkable
                if (pODMFace->boundingBox.z2 - pODMFace->boundingBox.z1 < 128)
                    bFaceSlopeTooSteep = false;
            }

            if (pODMFace->facePlane.normal.z > 0 && !bFaceSlopeTooSteep)
                *partyHasHitModel = true;

            if (engine->IsUnderwater())
                bFaceSlopeTooSteep = false;

            if (pParty->floor_face_id != collision_state.pid.id() && pODMFace->Pressure_Plate()) {
                pParty->floor_face_id = collision_state.pid.id();
                *triggerID = pODMFace->eventId;  // this one triggers tour events / traps
            }

            // new sliding plane - drag collision down to correct level for slide direction
            Vec3f slidePlaneOrigin = collision_state.collisionPos - Vec3f(0, 0, collision_state.heightOffset);
            Vec3f slidePlaneNormal = newPosLow + Vec3f(0, 0, collision_state.radius_lo) - slidePlaneOrigin;
            slidePlaneNormal.normalize();
            float destPlaneDist = dot(collision_state.new_position_lo - slidePlaneOrigin, slidePlaneNormal);
            Vec3f newDestination = collision_state.new_position_lo - destPlaneDist * slidePlaneNormal;
            Vec3f newDirection = newDestination - slidePlaneOrigin;

            // Cant push uphill on steep faces
            if (bFaceSlopeTooSteep && newDirection.z > 0)
                newDirection.z = 0;

            newDirection.normalize();

            // Push away from the surface and add a touch down for better slide
            if (bFaceSlopeTooSteep)
                *partyInputSpeed += Vec3f(pODMFace->facePlane.normal.x, pODMFace->facePlane.normal.y, -2) * 10;

            // set movement speed along sliding plane
            *partyInputSpeed = newDirection * dot(newDirection, *partyInputSpeed);

            if (pODMFace->polygonType == POLYGON_Floor || pODMFace->polygonType == POLYGON_InBetweenFloorAndWall) {
                pParty->bFlying = false;
                pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
            }

            if (pODMFace->polygonType == POLYGON_Floor) {
                // We dont collide with the rear of faces so hitting a floor poly with upwards direction means that
                // weve collided with its edge and we should step up onto its level.
                float newZ = pOutdoor->pBModels[collision_state.pid.id() >> 6].vertices[pODMFace->vertexIds[0]].z;
                if (pParty->velocity.z > 0.0f && (newZ - pParty->pos.z) < 128)
                    pParty->pos.z = newZ;
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

    d = std::sqrt(d);
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
        // TODO(pskelton): inside cylinder collisions (eg actor actor overlap) cause issues - disable for now.
        // Consider if theres any instances where this could be reintroduced and useful.
        return false;
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
