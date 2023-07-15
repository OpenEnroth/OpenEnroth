#pragma once

#include <cstdint>

#include "Library/Serialization/SerializationFwd.h"

enum MAP_TYPE : uint32_t {
    MAP_INVALID = 0,
    MAP_EMERALD_ISLE = 1,
    MAP_HARMONDALE = 2,
    MAP_STEADWICK = 3,
    MAP_PIERPONT = 4,
    MAP_DEYJA = 5,
    MAP_BRAKADA_DESERT = 6,
    MAP_CELESTIA = 7,
    MAP_THE_PIT = 8,
    MAP_EVENMORN_ISLE = 9,
    MAP_MOUNT_NIGHON = 10,
    MAP_BARROW_DOWNS = 11,
    MAP_LAND_OF_GIANTS = 12,
    MAP_TATALIA = 13,
    MAP_AVLEE = 14,
    MAP_SHOALS = 15,
    MAP_CASTLE_HARMONDALE = 21,
    //...
    MAP_ARENA = 76,
    //...
};

/**
 * Map treasure / danger level, determines how treasure levels of random items are handled by the game.
 *
 * Higher map treasure level => better random loot given the same treasure level of a random item.
 *
 * @see RemapTreasureLevel
 */
enum class MAP_TREASURE_LEVEL : int8_t {
    MAP_TREASURE_LEVEL_1 = 0,
    MAP_TREASURE_LEVEL_2 = 1,
    MAP_TREASURE_LEVEL_3 = 2,
    MAP_TREASURE_LEVEL_4 = 3,
    MAP_TREASURE_LEVEL_5 = 4,
    MAP_TREASURE_LEVEL_6 = 5,
    MAP_TREASURE_LEVEL_7 = 6,

    MAP_TREASURE_LEVEL_FIRST = MAP_TREASURE_LEVEL_1,
    MAP_TREASURE_LEVEL_LAST = MAP_TREASURE_LEVEL_7
};
using enum MAP_TREASURE_LEVEL;

enum MapStartPoint : uint32_t {
    MapStartPoint_Party = 0,
    MapStartPoint_North = 1,
    MapStartPoint_South = 2,
    MapStartPoint_East = 3,
    MapStartPoint_West = 4
};
MM_DECLARE_SERIALIZATION_FUNCTIONS(MapStartPoint)
