#pragma once

#include <optional>

#include "Engine/MapEnums.h"

#include "Library/Geometry/Vec.h"

/**
 * Where the party ends up, in absolute map coordinates.
 */
struct PartyPlacement {
    PartyPlacement() = default;
    PartyPlacement(const Vec3f &pos, int yaw, int pitch, int zSpeed) : pos(pos), yaw(yaw), pitch(pitch), zSpeed(zSpeed) {}

    Vec3f pos;
    int yaw = -1; // -1 keeps the current yaw. Shipped scripts use it for the in-map teleporters in the Erathian
                  // Sewers and the Hall under the Hill, which move the party without turning it.
    // TODO(captainurist): pitch is assigned even when yaw is -1, so those teleporters do level the view. Every
    //                     shipped MoveToMap record carries pitch 0, so the data can't tell us whether -1 was
    //                     meant to keep both angles. Investigate.
    int pitch = 0;
    int zSpeed = 0;
};

/**
 * Where a script or the UI wants the party to go.
 */
struct MapDestination {
    MapDestination() = default;
    MapDestination(MapId map, MapStartPoint startPoint) : map(map), startPoint(startPoint) {}
    MapDestination(MapId map, const PartyPlacement &placement) : map(map), placement(placement) {}

    MapId map = MAP_INVALID; // MAP_INVALID means stay on the current map.
    MapStartPoint startPoint = MAP_START_POINT_PARTY;
    std::optional<PartyPlacement> placement; // Unset means don't move the party.
};

/**
 * Moves the party. Refuses positions the map can't hold, logging an error.
 */
void placeParty(const PartyPlacement &placement);

/**
 * @param point                         Starting point to look up in the currently loaded map.
 * @return                              Placement of the map's starting point decoration, grounded on the floor.
 *                                      Maps without a matching decoration give the party's current placement.
 */
PartyPlacement placementForStartPoint(MapStartPoint point);
