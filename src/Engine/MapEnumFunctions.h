#pragma once

#include "MapEnums.h"

/**
 * Is map an outdoor map?
 *
 * MM8's outdoor maps (`MAP_MM8_DAGGER_WOUND_ISLAND`..`MAP_MM8_RAVAGE_ROAMING`, plus the isolated
 * `MAP_MM8_REGNA`) count too. Its four plane maps (85-88) sit in the same id range but are not
 * outdoor maps.
 */
inline bool isMapOutdoor(MapId mapid) {
    if (mapid >= MAP_EMERALD_ISLAND && mapid <= MAP_SHOALS && mapid != MAP_PIT && mapid != MAP_CELESTE)
        return true;
    return (mapid >= MAP_MM8_DAGGER_WOUND_ISLAND && mapid <= MAP_MM8_RAVAGE_ROAMING) || mapid == MAP_MM8_REGNA;
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

inline Segment<MapId> allMaps() {
    return {MAP_FIRST, MAP_LAST};
}
