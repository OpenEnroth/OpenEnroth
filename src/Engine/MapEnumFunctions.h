#pragma once

#include "MapEnums.h"

/**
 * Is map an outdoor map?
 */
inline bool isMapOutdoor(MapId mapid) {
    return mapid >= MAP_EMERALD_ISLAND && mapid <= MAP_SHOALS && mapid != MAP_PIT && mapid != MAP_CELESTE;
}

/**
 * Is map an indoor map?
 */
inline bool isMapIndoor(MapId mapid) {
    return (mapid >= MAP_DRAGON_CAVES && mapid <= MAP_ARENA) || mapid == MAP_PIT || mapid == MAP_CELESTE;
}

/**
 * Is map an outdoor underwater map (requires wetsuit etc.)?
 */
inline bool isMapUnderwater(MapId mapid) {
    return mapid == MAP_SHOALS;
}

/**
 * Is hirelings interactions are forbidden on this map?
 */
inline bool isHirelingsBlockedOnMap(MapId mapid) {
    return (mapid == MAP_SHOALS) || (mapid == MAP_LINCOLN);
}
