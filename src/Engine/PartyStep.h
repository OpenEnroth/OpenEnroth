#pragma once

#include <functional>

#include "Engine/Pid.h"

#include "Library/Geometry/Vec.h"

/**
 * A dry run of the party's own movement tick.
 *
 * `simulatePartySteps` runs the very same `ODM_ProcessPartyActions` / `BLV_ProcessPartyActions` the game runs every
 * frame - the same input queue, the same floor/slope/water/collision/gravity code, the same `dt` - from a pose of
 * the caller's choosing, with the party's motion state saved before and restored after, and with every effect the
 * tick would have had on anything BUT the party's pose held back and reported instead of applied. The answer is what
 * a player would get by pressing these keys for this many frames from that pose, and nothing else changes.
 *
 * This is the one primitive for "can the party get from here to there".
 */

/** What a movement tick does to the world besides moving the party. Recorded, not applied, during a dry run. */
struct PartyMotionEffects {
    int faceEvent = 0;          // Pressure-plate / trigger face event the tick would have fired (last one wins).
    int fallDamageDistance = 0; // `Party::giveFallDamage` argument, 0 when no damaging landing happened (max seen).
    bool fallingScream = false; // The party would have screamed (a fall from far above the ground).
    bool splash = false;        // The party would have splashed into water.
};

/**
 * Keys held for every simulated movement tick - the keys, not the actions: the forward key runs or walks by the
 * party's own running flag (`PARTY_FLAGS_2_RUNNING`), exactly as `KeyboardInputHandler` resolves it for a player.
 */
struct PartyStepInput {
    bool forward = false;
    bool backward = false;
    bool jump = false;
    bool ignoreActors = false;  // Walk the geometry only: bodies standing about do not stop the party (a map-scale
                                // flood or a stair run is a fact about the level, not about who is in the way today).
};

struct PartyStepResult {
    Vec3f start{};
    Vec3f end{};
    int ticks = 0;             // Movement ticks actually simulated (input ticks + settle ticks).
    float expectedTravelPerTick = 0.0f;  // What one held-key tick covers on open ground by the engine's own speed
                                         // formula (walk speed, run multiplier, this frame's dt) - the yardstick
                                         // for "did the party get anywhere near as far as the keys asked".
    bool noFloor = false;      // `start` lies outside the level's floor geometry - nothing was simulated.
    bool leftMap = false;      // The walk reached the outdoor map's edge (`maxPartyAxisDistance`, where the game
                               // clamps the party or offers travel); the keys were released there.
    bool airborne = false;     // `Party::isAirborne()` after the last tick - the engine's own notion, no threshold.
    bool onWater = false;      // The engine's floor query at `end` says the party stands on water.
    int floorFaceId = -1;      // Floor face under `end` (outdoor: model face id, -1 = terrain; indoor: BLV face).
    int sectorId = 0;          // Indoor sector at `end`, 0 outdoors.
    Vec3f collidedAt{};        // Where the party was on the frame `collidedWith` was first pressed against.
    Pid collidedWith{};        // What stopped the party: the first thing its HORIZONTAL move pressed against while the
                               // keys were held (`collision_state.horizontalPid`; the floor it stands on never counts;
                               // empty if nothing did).
    PartyMotionEffects effects;

    float planarTravel() const;
};

/**
 * Called after every simulated tick. Returning false during the input ticks releases the keys there - the settle
 * ticks still run, so a fall that starts on the last held frame is still followed to its landing.
 */
using PartyStepTraceFn = std::function<bool(int tick, const Vec3f &pos, bool airborne, const PartyMotionEffects &soFar)>;

/**
 * @param from          Pose to simulate from. The party is placed here at rest (no velocity, not airborne), on the
 *                      floor face the engine resolves under it, so a pressure plate already under `from` does not fire.
 * @param viewYaw       Facing, in the party's own yaw units (0 = +X, 512 = +Y).
 * @param input         Keys held for each of the `ticks` movement ticks.
 * @param ticks         Movement ticks with `input` held.
 * @param settleTicks   Extra ticks with no input afterwards, the way a real key release is followed by frames. Stops
 *                      early once the party has been on the ground for two consecutive ticks (the tick after a landing
 *                      is the one that charges fall damage). Pass enough to let a fall land, e.g. 30.
 * @param trace         Optional per-tick observer, see `PartyStepTraceFn`.
 */
PartyStepResult simulatePartySteps(const Vec3f &from, int viewYaw, const PartyStepInput &input, int ticks,
                                   int settleTicks = 0, const PartyStepTraceFn &trace = nullptr);

/** `pos` moved half a unit off an exact outdoor tile corner, where the terrain height query asserts; unchanged otherwise. */
Vec3f offTileCorner(const Vec3f &pos);

/** True while `simulatePartySteps` is running the movement tick. */
bool partyMotionDryRun();
/**
 * While set, `GetIndoorFloorZ` hands a party standing on a seam between two sectors to the sector the position
 * lies in when the remembered one only has a pit far below. Off by default.
 */
void setSeamFloorResolve(bool on);
bool seamFloorResolve();
/** True while a dry run asked to walk the geometry only (`PartyStepInput::ignoreActors`) - `CollideWithActor` reads it. */
bool partyMotionDryRunIgnoresActors();

// The seam `ODM_ProcessPartyActions` / `BLV_ProcessPartyActions` route their world effects through. In the game: applied
// directly. Dry run: recorded into the running simulation's `PartyMotionEffects`.
void partyMotionFaceEvent(int eventId);
void partyMotionFallDamage(int distance);
void partyMotionFallingScream();
void partyMotionSplash();
