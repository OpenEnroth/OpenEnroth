#pragma once

#include <string>

#include "Engine/MapEnums.h"

#include "Library/Geometry/Vec.h"

/**
 * Describes target teleportation point of the party.
 */
class TeleportPoint {
 public:
    /**
     * Checks validity of teleportation point
     */
    bool isValid() { return _teleportValid; }

    /**
     * Mark teleporation point as invalid.
     * Invalid points cannot be used for teleportation.
     */
    void invalidate();

    /**
     * Set teleportation target within the map.
     * Updates teleportation point validity if any of the set values is not zero (or -1 for yaw).
     *
     * @param pos     Party position after teleportation.
     * @param yaw     Camera yaw after teleportation, if set to -1 then yaw will not be changed after teleport.
     * @param pitch   Camera pitch after teleportation.
     * @param zSpeed  Vertical velocity of party after teleportation.
     */
    void setTeleportTarget(Vec3f pos, int yaw, int pitch, int zSpeed);

    /**
     * Set target map name for teleportation.
     * Map name is not used by doTeleport function because map loading processing is required in this case.
     * It is used separately in game message processing instead to change current map to previosly prepared one.
     *
     * @param mapName  Name of the target map.
     */
    void setTeleportMap(std::string_view mapName) { _targetMap = mapName; }

    /**
     * Get target map name.
     */
    const std::string &teleportMap() { return _targetMap; }

    /**
     * Perform teleportation using current valid point.
     * Changes effective party position, speed and camera position.
     * Target map is not used.
     *
     * @param keepOnZero    Do not change speed/positions if corresponding parameter is set to 0.
     *                      Does not affect yaw - it never updated if set to -1, and updated on any other value.
     */
    void doTeleport(bool keepOnZero);

    /**
     * Resolves the pending target against the given starting point decoration of the currently loaded map,
     * making it fully absolute. Doesn't teleport. Zero components (-1 for yaw) take the value the decoration
     * placement provides - the decoration position with z grounded on the floor, the decoration yaw, zero pitch
     * and vertical speed. An invalid pending target resolves to the decoration placement itself, so the point is
     * always valid afterwards. A map without a matching decoration resolves against the current party state
     * instead. Does nothing if `point` doesn't name a known decoration type.
     *
     * @param point         Starting point type to resolve against.
     */
    void adjustToStartingPoint(MapStartPoint point);

 private:
    bool _teleportValid = false;
    std::string _targetMap;
    Vec3f _pos;
    int _yaw = 0;
    int _pitch = 0;
    int _zSpeed = 0;
};

extern MapStartPoint uLevel_StartingPointType;
