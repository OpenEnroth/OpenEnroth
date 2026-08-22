#pragma once

#include <cassert>
#include <optional>
#include <variant>

#include "Engine/MapEnums.h"

#include "Library/Geometry/Vec.h"

/**
 * Where the party ends up and which way it faces, in absolute map coordinates.
 */
struct PartyPlacement {
    PartyPlacement() = default;
    PartyPlacement(const Vec3f &pos, int yaw, int pitch, int zSpeed)
        : pos(pos), yaw(yaw), pitch(pitch), zSpeed(zSpeed) {}

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
 * Where a script or the UI wants the party to go. Default-constructed one stays on the current map and doesn't move
 * the party.
 */
class MapDestination {
 public:
    MapDestination() = default;
    MapDestination(MapId map, MapStartPoint startPoint) : _map(map), _arrival(startPoint) {
        assert(map != MAP_INVALID); // Start points name a spot in the map being entered.
    }
    MapDestination(MapId map, const PartyPlacement &placement) : _map(map), _arrival(placement) {}

    [[nodiscard]] MapId map() const { return _map; } // MAP_INVALID means stay on the current map.

    /**
     * A `MapStartPoint` arrival names a decoration in the map that's being entered, so this can only be called once
     * that map is loaded. A `PartyPlacement` arrival is already in absolute coordinates and has no such requirement.
     *
     * @return                          Where the party ends up, or empty if it shouldn't be moved at all. A map
     *                                  that has no such start point decoration leaves the party where it is.
     */
    [[nodiscard]] std::optional<PartyPlacement> resolvePlacement() const;

 private:
    MapId _map = MAP_INVALID;
    std::variant<std::monostate, MapStartPoint, PartyPlacement> _arrival; // monostate means don't move the party.
};

/**
 * Moves the party, or leaves it where it is and logs an error if the target is unusable - indoors that means a
 * position outside every sector, outdoors one past the map bounds. An outdoor position below the floor is only
 * warned about, the next update drops the party down onto it.
 *
 * @param placement                     Where to put the party, in the currently loaded map.
 */
void placeParty(const PartyPlacement &placement);
