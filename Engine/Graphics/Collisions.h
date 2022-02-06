#pragma once

#include "Engine/VectorTypes.h"

struct BLVFace;

struct CollisionState {
    /**
     * Prepares this struct by filling all necessary fields, and checks whether there is actually no movement.
     *
     * @param dt                        Time delta, in fixpoint seconds.
     * @return                          True if there is no movement, false otherwise.
     */
    bool PrepareAndCheckIfStationary(int dt);

    // actor is modeled as two spheres, basically "feet" & "head". Collisions are then done for both spheres.

    int check_hi;  // Check the hi sphere collisions. If not set, only the lo sphere is checked.
    int radius_lo;   // radius of the lo ("feet") sphere.
    int radius_hi;  // radius of the hi ("head") sphere.
    Vec3_int_ position_lo; // center of the lo sphere.
    Vec3_int_ position_hi; // center of the hi sphere.
    Vec3_int_ new_position_lo; // desired new position for the center of the lo sphere.
    Vec3_int_ new_position_hi; // desired new position for the center of the hi sphere.
    Vec3_int_ velocity;  // Movement vector.
    Vec3_int_ direction;  // Movement direction, basically velocity as a unit vector.
    int speed = 0;  // Velocity magnitude.
    int total_move_distance;  // Total move distance, accumulated between collision iterations, starts at 0.
    int move_distance;  // Desired movement distance for current iteration, minus the distance already covered.
    int adjusted_move_distance;  // Movement distance for current iteration, adjusted after collision checks.
    unsigned int uSectorID = 0;  // Indoor sector id.
    unsigned int pid;  // PID of the object that we're collided with.
    int ignored_face_id;  // Don't check collisions with this face.
    BBox_int_ bbox = { 0, 0, 0, 0, 0, 0 };
};

extern CollisionState collision_state;

/**
 * Original offset 0x46E44E.
 *
 * Performs collisions with level geometry in indoor levels. Updates `collision_state`.
 *
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 */
void CollideIndoorWithGeometry(bool ignore_ethereal);

/**
 * Original offset 0x46E889.
 *
 * Performs collisions with models in outdoor levels. Updates `collision_state`.
 *
 * @param ignore_ethereal               Whether ethereal faces should be ignored by this function.
 */
void CollideOutdoorWithModels(bool ignore_ethereal);

/**
 * Original offset 0x46E0B2.
 */
void CollideIndoorWithDecorations();

/**
 * Original offset 0x46E26D.
 *
 * @param grid_x
 * @param grid_y
 */
void CollideOutdoorWithDecorations(int grid_x, int grid_y);

/**
 * Original offset 0x46F04E.
 *
 * Performs collision checks with portals. Updates `collision_state`. If the collision did happen, then
 * `adjusted_move_distance` member is set to `0xFFFFFF` (basically a large number).
 *
 * @return                              True if there were no collisions with portals.
 */
bool CollideIndoorWithPortals();

/**
 * Original offset 0x46DF1A.
 *
 * @param actor_idx                 Actor index.
 * @param override_radius           Override actor's radius. Pass zero to use original radius.
 * @return                          Whether the collision is possible.
 */
bool CollideWithActor(int actor_idx, int override_radius);

void _46ED8A_collide_against_sprite_objects(unsigned int _this);

int _46EF01_collision_chech_player(int a1);


