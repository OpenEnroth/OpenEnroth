#include "Collisions.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/MM7.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"

CollisionState collision_state;

void CollideIndoorWithGeometry(bool ignore_ethereal) {
    std::array<int, 10> pSectorsArray;  // [sp+30h] [bp-28h]@1
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
            int pFloor = pSector->pFloors[j];
            BLVFace *pFace = &pIndoor->pFaces[pSector->pFloors[j]];

            if (pFace->Portal() || !collision_state.bbox.Intersects(pFace->pBounding))
                continue;

            if (pFloor == collision_state.field_84)
                continue;

            int distance_lo_old = pFace->pFacePlane_old.SignedDistanceTo(collision_state.position_lo);
            int distance_lo_new = pFace->pFacePlane_old.SignedDistanceTo(collision_state.new_position_lo);
            if (distance_lo_old > 0 &&
                (distance_lo_old <= collision_state.radius_lo || distance_lo_new <= collision_state.radius_lo) &&
                distance_lo_new <= distance_lo_old) {
                bool have_collision = false;
                int move_distance = collision_state.move_distance;
                if (CollideIndoorWithFace(pFace, collision_state.position_lo, collision_state.radius_lo,
                                         collision_state.direction, &move_distance, ignore_ethereal)) {
                    have_collision = true;
                } else {
                    move_distance = collision_state.move_distance + collision_state.radius_lo;
                    if (CollidePointIndoorWithFace(pFace, &collision_state.position_lo, &collision_state.direction,
                                                   &move_distance)) {
                        have_collision = true;
                        move_distance -= collision_state.radius_lo;
                    }
                }

                if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                    collision_state.adjusted_move_distance = move_distance;
                    collision_state.pid = PID(OBJECT_BModel, pSector->pFloors[j]);
                }
            }

            // TODO: it's quite clear radius_lo does not belong in checks below, but here it is. Probably a bug in
            // the original code?

            int distance_hi_old = pFace->pFacePlane_old.SignedDistanceTo(collision_state.position_hi);
            int distance_hi_new = pFace->pFacePlane_old.SignedDistanceTo(collision_state.new_position_hi);
            if ((collision_state.check_hi & 1) &&
                distance_hi_old > 0 &&
                (distance_hi_old <= collision_state.radius_lo || distance_hi_new <= collision_state.radius_lo) &&
                distance_hi_new <= distance_hi_old) {
                bool have_collision = false;
                int move_distance = collision_state.move_distance;
                if (CollideIndoorWithFace(pFace, collision_state.position_hi, collision_state.radius_hi,
                                         collision_state.direction, &move_distance, ignore_ethereal)) {
                    have_collision = true;
                } else {
                    move_distance = collision_state.move_distance + collision_state.radius_hi;
                    if (CollidePointIndoorWithFace(pFace, &collision_state.position_hi, &collision_state.direction,
                                                   &move_distance)) {
                        have_collision = true;
                        move_distance -= collision_state.radius_lo;
                    }
                }

                if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                    collision_state.adjusted_move_distance = move_distance;
                    collision_state.pid = PID(OBJECT_BModel, pSector->pFloors[j]);
                }
            }
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
            face.pFacePlane_old.vNormal.x = mface.pFacePlaneOLD.vNormal.x;
            face.pFacePlane_old.vNormal.y = mface.pFacePlaneOLD.vNormal.y;
            face.pFacePlane_old.vNormal.z = mface.pFacePlaneOLD.vNormal.z;

            // TODO: need fixpoint_to_float here
            face.pFacePlane_old.dist = mface.pFacePlaneOLD.dist;  // incorrect

            face.uAttributes = mface.uAttributes;

            face.pBounding.x1 = mface.pBoundingBox.x1;
            face.pBounding.y1 = mface.pBoundingBox.y1;
            face.pBounding.z1 = mface.pBoundingBox.z1;

            face.pBounding.x2 = mface.pBoundingBox.x2;
            face.pBounding.y2 = mface.pBoundingBox.y2;
            face.pBounding.z2 = mface.pBoundingBox.z2;

            face.zCalc1 = mface.zCalc1;
            face.zCalc2 = mface.zCalc2;
            face.zCalc3 = mface.zCalc3;

            face.pXInterceptDisplacements = mface.pXInterceptDisplacements;
            face.pYInterceptDisplacements = mface.pYInterceptDisplacements;
            face.pZInterceptDisplacements = mface.pZInterceptDisplacements;

            face.uPolygonType = (PolygonType)mface.uPolygonType;

            face.uNumVertices = mface.uNumVertices;

            // face.uBitmapID = model.pFaces[j].uTextureID;
            face.resource = mface.resource;

            face.pVertexIDs = mface.pVertexIDs;

            if (face.Ethereal() || face.Portal()) // TODO: this doesn't respect ignore_ethereal parameter
                continue;

            int distance_lo_old = face.pFacePlane_old.SignedDistanceTo(collision_state.position_lo);
            int distance_lo_new = face.pFacePlane_old.SignedDistanceTo(collision_state.new_position_lo);
            if (distance_lo_old > 0 &&
                (distance_lo_old <= collision_state.radius_lo || distance_lo_new <= collision_state.radius_lo) &&
                distance_lo_new <= distance_lo_old) {
                bool have_collision = false;
                int move_distance = collision_state.move_distance;
                if (CollideOutdoorWithFace(collision_state.radius_lo, &move_distance, collision_state.position_lo,
                                           collision_state.direction, &face, model.index, ignore_ethereal)) {
                    have_collision = true;
                } else {
                    move_distance = collision_state.move_distance + collision_state.radius_lo;
                    if (CollidePointOutdoorWithFace(&move_distance, &face, collision_state.position_lo,
                                                    collision_state.direction, model.index)) {
                        have_collision = true;
                        move_distance -= collision_state.radius_lo;
                    }
                }

                if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                    collision_state.adjusted_move_distance = move_distance;
                    collision_state.pid = PID(OBJECT_BModel, (mface.index | (model.index << 6)));
                }
            }

            int distance_hi_old = face.pFacePlane_old.SignedDistanceTo(collision_state.position_hi);
            int distance_hi_new = face.pFacePlane_old.SignedDistanceTo(collision_state.new_position_hi);
            if ((collision_state.check_hi & 1) &&
                distance_hi_old > 0 &&
                (distance_hi_old <= collision_state.radius_lo || distance_hi_new <= collision_state.radius_lo) &&
                distance_hi_new <= distance_hi_old) {
                bool have_collision = false;
                int move_distance = collision_state.move_distance;
                if (CollideOutdoorWithFace(collision_state.radius_hi, &move_distance, collision_state.position_hi,
                                           collision_state.direction, &face, model.index, ignore_ethereal)) {
                    have_collision = true;
                } else {
                    move_distance = collision_state.move_distance + collision_state.radius_hi;
                    if (CollidePointOutdoorWithFace(&move_distance, &face, collision_state.position_hi,
                                                    collision_state.direction, model.index)) {
                        have_collision = true;
                        move_distance -= collision_state.radius_lo;
                    }
                }

                if (have_collision && move_distance < collision_state.adjusted_move_distance) {
                    collision_state.adjusted_move_distance = move_distance;
                    collision_state.pid = PID(OBJECT_BModel, (mface.index | (model.index << 6)));
                }
            }
        }
    }
}

void CollideIndoorWithDecorations() {
    BLVSector *sector = &pIndoor->pSectors[collision_state.uSectorID];
    for (unsigned int i = 0; i < sector->uNumDecorations; ++i) {
        LevelDecoration *decor = &pLevelDecorations[sector->pDecorationIDs[i]];
        if (decor->uFlags & LEVEL_DECORATION_INVISIBLE)
            continue;

        DecorationDesc *decor_desc = pDecorationList->GetDecoration(decor->uDecorationDescID);
        if (decor_desc->CanMoveThrough())
            continue;

        BBox_int_ bbox;
        bbox.x1 = decor->vPosition.x - decor_desc->uRadius;
        bbox.x2 = decor->vPosition.x + decor_desc->uRadius;
        bbox.y1 = decor->vPosition.y - decor_desc->uRadius;
        bbox.y2 = decor->vPosition.y + decor_desc->uRadius;
        bbox.z1 = decor->vPosition.z;
        bbox.z2 = decor->vPosition.z + decor_desc->uDecorationHeight;
        if (!collision_state.bbox.Intersects(bbox))
            continue;

        // dist vector points from position center into decoration center.
        int dist_x = decor->vPosition.x - collision_state.position_lo.x;
        int dist_y = decor->vPosition.y - collision_state.position_lo.y;
        int sum_radius = collision_state.radius_lo + decor_desc->uRadius;

        // Area of the parallelogram formed by dist and collision_state.direction. Direction is a unit vector,
        // thus this actually is length(dist) * sin(dist, collision_state.direction).
        // This in turn is the distance from decoration center to the line of actor's movement.
        int closest_dist = (dist_x * collision_state.direction.y - dist_y * collision_state.direction.x) >> 16;
        if (abs(closest_dist) > sum_radius)
            continue; // No chance to collide.

        // Length of dist vector projected onto collision_state.direction.
        int dist_dot_dir = (dist_x * collision_state.direction.x + dist_y * collision_state.direction.y) >> 16;
        if (dist_dot_dir <= 0)
            continue; // We're moving away from the decoration.

        // Z-coordinate of the actor at the point closest to the decoration in XY plane.
        int closest_z = collision_state.position_lo.z + fixpoint_mul(collision_state.direction.z, dist_dot_dir);
        if (closest_z < bbox.z1 || closest_z > bbox.z2)
            continue;

        // That's how far can we go along the collision_state.direction axis until the actor touches the decoration,
        // i.e. distance between them goes below sum_radius.
        int move_distance = dist_dot_dir - integer_sqrt(sum_radius * sum_radius - closest_dist * closest_dist);
        if (move_distance < 0)
            move_distance = 0;

        if (move_distance < collision_state.adjusted_move_distance) {
            collision_state.adjusted_move_distance = move_distance;
            collision_state.pid = PID(OBJECT_Decoration, sector->pDecorationIDs[i]);
        }
    }
}

void CollideOutdoorWithDecorations(int grid_x, int grid_y) {
    int v12;                // ebp@15
    int v13;                // ebx@15
    int v14;                // esi@16
    int v15;                // edi@17
    int v16;                // eax@17
    int v17;                // esi@19
    char v18;               // zf@23
    int v19;                // [sp+0h] [bp-10h]@15
    int v21;                // [sp+8h] [bp-8h]@15

    if (grid_x < 0 || grid_x > 127 || grid_y < 0 || grid_y > 127)
        return;

    int grid_index = grid_x + (grid_y << 7);
    unsigned __int16 *v3 = &pOutdoor->pFaceIDLIST[pOutdoor->pOMAP[grid_index]];
    unsigned __int16 *v20 = &pOutdoor->pFaceIDLIST[pOutdoor->pOMAP[grid_index]];
    if (!v3)
        return;

    do {
        unsigned __int16 v4 = *v3;
        if (PID_TYPE(v4) == OBJECT_Decoration) {
            LevelDecoration *v5 = &pLevelDecorations[PID_ID(v4)];
            if (!(v5->uFlags & LEVEL_DECORATION_INVISIBLE)) {
                DecorationDesc *v6 = pDecorationList->GetDecoration(v5->uDecorationDescID);
                if (!v6->CanMoveThrough()) {
                    BBox_int_ bbox;
                    bbox.x1 = v5->vPosition.x - v6->uRadius;
                    bbox.x2 = v5->vPosition.x + v6->uRadius;
                    bbox.y1 = v5->vPosition.y - v6->uRadius;
                    bbox.y2 = v5->vPosition.y + v6->uRadius;
                    bbox.z1 = v5->vPosition.z;
                    bbox.z2 = v5->vPosition.z + v6->uDecorationHeight;
                    if (collision_state.bbox.Intersects(bbox)) {
                        v12 = v5->vPosition.x - collision_state.position_lo.x;
                        v19 = v5->vPosition.y - collision_state.position_lo.y;
                        v13 = collision_state.radius_lo + v6->uRadius;
                        v21 = ((v5->vPosition.x - collision_state.position_lo.x) *
                            collision_state.direction.y - (v5->vPosition.y - collision_state.position_lo.y) *
                            collision_state.direction.x) >> 16;
                        if (abs(v21) <= collision_state.radius_lo + v6->uRadius) {
                            v14 = (v12 * collision_state.direction.x +
                                v19 * collision_state.direction.y) >> 16;
                            if (v14 > 0) {
                                v15 = v5->vPosition.z;
                                v16 = collision_state.position_lo.z +
                                    fixpoint_mul(collision_state.direction.z, v14);
                                if (v16 >= v15) {
                                    if (v16 <= v6->uDecorationHeight + v15) {
                                        v17 = v14 - integer_sqrt(v13 * v13 - v21 * v21);
                                        if (v17 < 0)
                                            v17 = 0;
                                        if (v17 < collision_state.adjusted_move_distance) {
                                            collision_state.adjusted_move_distance = v17;
                                            collision_state.pid = *v20;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        v3 = v20 + 1;
        v18 = *v20 == 0;
        ++v20;
    } while (!v18);
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
            CollidePointIndoorWithFace(face, &collision_state.position_lo, &collision_state.direction, &move_distance) &&
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

int _46EF01_collision_chech_player(int a1) {
    int result;  // eax@1
    int v3;      // ebx@7
    int v4;      // esi@7
    int v5;      // edi@8
    int v6;      // ecx@9
    int v7;      // edi@12
    int v10;     // [sp+14h] [bp-8h]@7
    int v11;     // [sp+18h] [bp-4h]@7

    result = pParty->vPosition.x;
    // device_caps = pParty->uPartyHeight;
    if (collision_state.bbox.x1 <= pParty->vPosition.x + (2 * pParty->radius) &&
        collision_state.bbox.x2 >= pParty->vPosition.x - (2 * pParty->radius) &&
        collision_state.bbox.y1 <= pParty->vPosition.y + (2 * pParty->radius) &&
        collision_state.bbox.y2 >= pParty->vPosition.y - (2 * pParty->radius) &&
        collision_state.bbox.z1 <= (pParty->vPosition.z + (int)pParty->uPartyHeight) &&
        collision_state.bbox.z2 >= pParty->vPosition.z) {
        v3 = collision_state.radius_lo + (2 * pParty->radius);
        v11 = pParty->vPosition.x - collision_state.position_lo.x;
        v4 = ((pParty->vPosition.x - collision_state.position_lo.x) *
            collision_state.direction.y -
            (pParty->vPosition.y - collision_state.position_lo.y) *
            collision_state.direction.x) >>
            16;
        v10 = pParty->vPosition.y - collision_state.position_lo.y;
        result = abs(((pParty->vPosition.x - collision_state.position_lo.x) *
            collision_state.direction.y -
            (pParty->vPosition.y - collision_state.position_lo.y) *
            collision_state.direction.x) >>
            16);
        if (result <= collision_state.radius_lo + (2 * pParty->radius)) {
            result = v10 * collision_state.direction.y;
            v5 = (v10 * collision_state.direction.y +
                v11 * collision_state.direction.x) >>
                16;
            if (v5 > 0) {
                v6 = fixpoint_mul(collision_state.direction.z, v5) +
                    collision_state.position_lo.z;
                result = pParty->vPosition.z;
                if (v6 >= pParty->vPosition.z) {
                    result = pParty->uPartyHeight + pParty->vPosition.z;
                    if (v6 <= (signed int)(pParty->uPartyHeight +
                        pParty->vPosition.z) ||
                        a1) {
                        result = integer_sqrt(v3 * v3 - v4 * v4);
                        v7 = v5 - integer_sqrt(v3 * v3 - v4 * v4);
                        if (v7 < 0) v7 = 0;
                        if (v7 < collision_state.adjusted_move_distance) {
                            collision_state.adjusted_move_distance = v7;
                            collision_state.pid = 4;
                        }
                    }
                }
            }
        }
    }
    return result;
}

bool CollideIndoorWithFace(BLVFace *face, const Vec3_int_ &pos, int radius, const Vec3_int_ &dir,
                          int *move_distance, bool ignore_ethereal) {
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

    if (!IsProjectedPointInsideIndoorFace(face, new_pos))
        return false; // We've just managed to slide past the face, so pretend no collision happened.

    if (move_distance_fp < 0) {
        *move_distance = 0;
    } else {
        *move_distance = move_distance_fp >> 16;
    }

    return true;
}

bool CollideOutdoorWithFace(int radius, int* move_distance, const Vec3_int_ &pos, const Vec3_int_ &dir,
                BLVFace* face, int model_index, bool ignore_ethereal) {
    int v12;      // ST1C_4@3
    int a7a;      // [sp+30h] [bp+18h]@7
    int a1b;      // [sp+38h] [bp+20h]@3
    int a11b;     // [sp+40h] [bp+28h]@3

    if (ignore_ethereal && face->Ethereal())
        return false;

    // _fp suffix => that's a fixpoint number

    int cos_dir_normal_fp =
        fixpoint_mul(dir.x, face->pFacePlane_old.vNormal.x) +
        fixpoint_mul(dir.y, face->pFacePlane_old.vNormal.y) +
        fixpoint_mul(dir.z, face->pFacePlane_old.vNormal.z);

    int pos_face_distance_fp = face->pFacePlane_old.SignedDistanceToAsFixpoint(pos.x, pos.y, pos.z);
    int radius_fp = radius << 16;

    int move_distance_fp;
    int64_t overshoot;

    int overshoot_fp = radius_fp - pos_face_distance_fp;
    if (abs(radius_fp - pos_face_distance_fp) < radius_fp) {
        move_distance_fp = 0;
        overshoot = abs(overshoot_fp) >> 16;
    } else {
        int overshoot_x4 = abs(overshoot_fp) >> 14;
        if (overshoot_x4 > abs(cos_dir_normal_fp))
            return false;

        overshoot = radius;
        move_distance_fp = fixpoint_div(overshoot_fp, cos_dir_normal_fp);
    }

    Vec3_short_ new_pos;
    new_pos.x = pos.x + ((fixpoint_mul(move_distance_fp, dir.x) - overshoot * face->pFacePlane_old.vNormal.x) >> 16);
    new_pos.y = pos.y + ((fixpoint_mul(move_distance_fp, dir.y) - overshoot * face->pFacePlane_old.vNormal.y) >> 16);
    new_pos.z = pos.z + ((fixpoint_mul(move_distance_fp, dir.z) - overshoot * face->pFacePlane_old.vNormal.z) >> 16);

    if (!IsProjectedPointInsideOutdoorFace(face, model_index, new_pos))
        return false;

    if (move_distance_fp < 0) {
        *move_distance = 0;
    } else {
        *move_distance = move_distance_fp >> 16;
    }
    return true;
}

bool CollidePointIndoorWithFace(BLVFace *face, Vec3_int_ *pos, Vec3_int_ *dir, int *move_distance) {
    // _fp suffix => that's a fixpoint number

    // dot_product(dir, normal) is a cosine of an angle between them.
    int cos_dir_normal_fp =
        fixpoint_mul(dir->x, face->pFacePlane_old.vNormal.x) +
        fixpoint_mul(dir->y, face->pFacePlane_old.vNormal.y) +
        fixpoint_mul(dir->z, face->pFacePlane_old.vNormal.z);

    if (cos_dir_normal_fp == 0)
        return false; // dir is perpendicular to face normal.

    if (face->uAttributes & FACE_ETHEREAL)
        return false;

    if (cos_dir_normal_fp > 0 && !face->Portal())
        return false; // We're facing away && face is not a portal.

    int pos_face_distance_fp = face->pFacePlane_old.SignedDistanceToAsFixpoint(*pos);

    if (cos_dir_normal_fp < 0 && pos_face_distance_fp < 0)
        return false; // Facing towards the face but already inside the model.

    if (cos_dir_normal_fp > 0 && pos_face_distance_fp > 0)
        return false; // Facing away from the face and outside the model.

    int pos_face_distance_x4 = abs(-(pos_face_distance_fp)) >> 14;

    // How far we need to move along the `dir` axis to hit face.
    int move_distance_fp = fixpoint_div(-pos_face_distance_fp, cos_dir_normal_fp);

    Vec3_short_ new_pos;
    new_pos.x = pos->x + ((fixpoint_mul(move_distance_fp, dir->x) + 0x8000) >> 16);
    new_pos.y = pos->y + ((fixpoint_mul(move_distance_fp, dir->y) + 0x8000) >> 16);
    new_pos.z = pos->z + ((fixpoint_mul(move_distance_fp, dir->z) + 0x8000) >> 16);

    if (pos_face_distance_x4 > abs(cos_dir_normal_fp))
        return false; // Moving perpendicular to the plane.

    if (move_distance_fp > *move_distance << 16)
        return false; // No correction needed.

    if (!IsProjectedPointInsideIndoorFace(face, new_pos))
        return false;

    *move_distance = move_distance_fp >> 16;
    return true;
}

bool CollidePointOutdoorWithFace(int *move_distance, BLVFace *face, const Vec3_int_ &pos, const Vec3_int_ &dir, int model_index) {
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

    if (!IsProjectedPointInsideOutdoorFace(face, model_index, new_pos))
        return false;

    *move_distance = move_distance_fp >> 16;
    return true;
}

bool IsProjectedPointInsideIndoorFace(BLVFace* face, const Vec3_short_ &point) {
    std::array<int16_t, 104> edges_u;
    std::array<int16_t, 104> edges_v;

    int u;
    int v;
    if (face->uAttributes & FACE_XY_PLANE) {
        u = point.x;
        v = point.y;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].x;
            edges_v[2 * i] = face->pYInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].y;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].x;
            edges_v[2 * i + 1] = face->pYInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].y;
        }
    } else if (face->uAttributes & FACE_XZ_PLANE) {
        u = point.x;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].x;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].z;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].x;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].z;
        }
    } else {
        u = point.y;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; i++) {
            edges_u[2 * i] = face->pYInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].y;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] + pIndoor->pVertices[face->pVertexIDs[i]].z;
            edges_u[2 * i + 1] = face->pYInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].y;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] + pIndoor->pVertices[face->pVertexIDs[i + 1]].z;
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

bool IsProjectedPointInsideOutdoorFace(BLVFace* face, int model_index, const Vec3_short_ &point) {
    std::array<int16_t, 104> edges_u;
    std::array<int16_t, 104> edges_v;

    int u;
    int v;
    if (face->uAttributes & FACE_XY_PLANE) {
        u = point.x;
        v = point.y;
        for (int i = 0; i < face->uNumVertices; ++i) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].x;
            edges_v[2 * i] = face->pYInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].y;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].x;
            edges_v[2 * i + 1] = face->pYInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].y;
        }
    } else if (face->uAttributes & FACE_XZ_PLANE) {
        u = point.x;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; ++i) {
            edges_u[2 * i] = face->pXInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].x;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].z;
            edges_u[2 * i + 1] = face->pXInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].x;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].z;
        }
    } else {
        u = point.y;
        v = point.z;
        for (int i = 0; i < face->uNumVertices; ++i) {
            edges_u[2 * i] = face->pYInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].y;
            edges_v[2 * i] = face->pZInterceptDisplacements[i] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i]].z;
            edges_u[2 * i + 1] = face->pYInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].y;
            edges_v[2 * i + 1] = face->pZInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[model_index].pVertices.pVertices[face->pVertexIDs[i + 1]].z;
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

