#pragma once

#include <cstdint>

#include "Library/Serialization/SerializationFwd.h"

/**
 * Enum of all maps in the game.
 *
 * DON'T EDIT, THIS IS AUTOGENERATED CODE.
 *
 * @see runMapIdCodeGen
 */
enum class MapId : uint32_t {
    MAP_INVALID = 0,
    MAP_EMERALD_ISLAND = 1,
    MAP_HARMONDALE = 2,
    MAP_ERATHIA = 3,
    MAP_TULAREAN_FOREST = 4,
    MAP_DEYJA = 5,
    MAP_BRACADA_DESERT = 6,
    MAP_CELESTE = 7,
    MAP_PIT = 8,
    MAP_EVENMORN_ISLAND = 9,
    MAP_MOUNT_NIGHON = 10,
    MAP_BARROW_DOWNS = 11,
    MAP_LAND_OF_THE_GIANTS = 12,
    MAP_TATALIA = 13,
    MAP_AVLEE = 14,
    MAP_SHOALS = 15,
    MAP_DRAGON_CAVES = 16,
    MAP_LORD_MARKHAMS_MANOR = 17,
    MAP_BANDIT_CAVES = 18,
    MAP_HAUNTED_MANSION = 19,
    MAP_TEMPLE_OF_THE_MOON = 20,
    MAP_CASTLE_HARMONDALE = 21,
    MAP_WHITE_CLIFF_CAVE = 22,
    MAP_ERATHIAN_SEWERS = 23,
    MAP_FORT_RIVERSTRIDE = 24,
    MAP_TULAREAN_CAVES = 25,
    MAP_CLANKERS_LABORATORY = 26,
    MAP_HALL_OF_THE_PIT = 27,
    MAP_WATCHTOWER_6 = 28,
    MAP_SCHOOL_OF_SORCERY = 29,
    MAP_RED_DWARF_MINES = 30,
    MAP_WALLS_OF_MIST = 31,
    MAP_TEMPLE_OF_THE_LIGHT = 32,
    MAP_BREEDING_ZONE = 33,
    MAP_TEMPLE_OF_THE_DARK = 34,
    MAP_GRAND_TEMPLE_OF_THE_MOON = 35,
    MAP_GRAND_TEMPLE_OF_THE_SUN = 36,
    MAP_THUNDERFIST_MOUNTAIN = 37,
    MAP_MAZE = 38,
    MAP_STONE_CITY = 39,
    MAP_COLONY_ZOD = 40,
    MAP_MERCENARY_GUILD = 41,
    MAP_TIDEWATER_CAVERNS = 42,
    MAP_WINE_CELLAR = 43,
    MAP_TITANS_STRONGHOLD = 44,
    MAP_TEMPLE_OF_BAA = 45,
    MAP_HALL_UNDER_THE_HILL = 46,
    MAP_LINCOLN = 47,
    MAP_CASTLE_GRYPHONHEART = 48,
    MAP_CASTLE_NAVAN = 49,
    MAP_CASTLE_LAMBENT = 50,
    MAP_CASTLE_GLOAMING = 51,
    MAP_DRAGONS_LAIR = 52,
    MAP_BARROW_VII = 53,
    MAP_BARROW_IV = 54,
    MAP_BARROW_II = 55,
    MAP_BARROW_XIV = 56,
    MAP_BARROW_III = 57,
    MAP_BARROW_IX = 58,
    MAP_BARROW_VI = 59,
    MAP_BARROW_I = 60,
    MAP_BARROW_VIII = 61,
    MAP_BARROW_XIII = 62,
    MAP_BARROW_X = 63,
    MAP_BARROW_XII = 64,
    MAP_BARROW_V = 65,
    MAP_BARROW_XI = 66,
    MAP_BARROW_XV = 67,
    MAP_ZOKARRS_TOMB = 68,
    MAP_NIGHON_TUNNELS = 69,
    MAP_TUNNELS_TO_EEOFOL = 70,
    MAP_WILLIAM_SETAGS_TOWER = 71,
    MAP_WROMTHRAXS_CAVE = 72,
    MAP_HIDDEN_TOMB = 73,
    MAP_STRANGE_TEMPLE = 74,
    MAP_SMALL_HOUSE = 75,
    MAP_ARENA = 76,

    MAP_FIRST = MAP_EMERALD_ISLAND,
    MAP_LAST = MAP_ARENA,
};
using enum MapId;

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

enum class MapStartPoint : uint32_t {
    MAP_START_POINT_PARTY = 0,
    MAP_START_POINT_NORTH = 1,
    MAP_START_POINT_SOUTH = 2,
    MAP_START_POINT_EAST = 3,
    MAP_START_POINT_WEST = 4
};
using enum MapStartPoint;
MM_DECLARE_SERIALIZATION_FUNCTIONS(MapStartPoint)
