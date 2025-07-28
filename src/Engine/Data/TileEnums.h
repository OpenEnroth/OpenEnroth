#pragma once

#include "Utility/Flags.h"

#include "Library/Serialization/SerializationFwd.h"

enum class TileFlag {
    TILE_BURN = 0x1,
    TILE_WATER = 0x2, // Literally water.
    TILE_BLOCK = 0x4,
    TILE_REPULSE = 0x8,
    TILE_FLAT = 0x10,
    TILE_WAVY = 0x20,
    TILE_DONT_DRAW = 0x40,
    TILE_SHORE = 0x100, // Shore tile that's drawn on top of the water.
    TILE_TRANSITION = 0x200, // Transition tile, e.g. dirt-sand, dirt-water, grass-dirt, etc. All road tiles have this set.
    TILE_SCROLL_DOWN = 0x400,
    TILE_SCROLL_UP = 0x800,
    TILE_SCROLL_LEFT = 0x1000,
    TILE_SCROLL_RIGHT = 0x2000,
    TILE_GENERATED_TRANSITION = 0x4000, // OE-specific, transition tile that was auto-generated, tile id might not be
                                        // stable between runs.
};
using enum TileFlag;
MM_DECLARE_FLAGS(TileFlags, TileFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(TileFlags)

/**
 * Tile variants inside a single tile set.
 */
enum class TileVariant {
    TILE_VARIANT_INVALID = -1,

    // Base tile variants. Only swamp has variants in MM7.
    TILE_VARIANT_BASE1 = 0,
    TILE_VARIANT_BASE2,
    TILE_VARIANT_BASE3,
    TILE_VARIANT_BASE4,

    // Special tile variants. Don't exist in MM7.
    TILE_VARIANT_SPECIAL1,
    TILE_VARIANT_SPECIAL2,
    TILE_VARIANT_SPECIAL3,
    TILE_VARIANT_SPECIAL4,
    TILE_VARIANT_SPECIAL5,
    TILE_VARIANT_SPECIAL6,
    TILE_VARIANT_SPECIAL7,
    TILE_VARIANT_SPECIAL8,

    // Road tile variants.
    TILE_VARIANT_ROAD_N_S_E_W, // Intersection.
    TILE_VARIANT_ROAD_N_S, // Straight road.
    TILE_VARIANT_ROAD_E_W,
    TILE_VARIANT_ROAD_N_E, // 90 deg turns.
    TILE_VARIANT_ROAD_N_W,
    TILE_VARIANT_ROAD_S_E,
    TILE_VARIANT_ROAD_S_W,
    TILE_VARIANT_ROAD_N_S_E, // T intersections.
    TILE_VARIANT_ROAD_N_S_W,
    TILE_VARIANT_ROAD_N_E_W,
    TILE_VARIANT_ROAD_S_E_W,
    TILE_VARIANT_ROAD_N, // Road ends.
    TILE_VARIANT_ROAD_S,
    TILE_VARIANT_ROAD_E,
    TILE_VARIANT_ROAD_W,
    TILE_VARIANT_ROAD_Y_N_S_E, // Y intersections with 135 deg turns.
    TILE_VARIANT_ROAD_Y_N_S_W,
    TILE_VARIANT_ROAD_Y_N_E_W,
    TILE_VARIANT_ROAD_Y_S_E_W,
    TILE_VARIANT_ROAD_D_N_E, // Diagonal roads.
    TILE_VARIANT_ROAD_D_N_W,
    TILE_VARIANT_ROAD_D_S_E,
    TILE_VARIANT_ROAD_D_S_W,

    // Transition tile variants follow, 46 total.

    // ?X?
    // ...
    // ...
    TILE_VARIANT_TRANSITION_N,
    TILE_VARIANT_TRANSITION_S,
    TILE_VARIANT_TRANSITION_E,
    TILE_VARIANT_TRANSITION_W,

    // ..X
    // ...
    // ...
    TILE_VARIANT_TRANSITION_NE,
    TILE_VARIANT_TRANSITION_NW,
    TILE_VARIANT_TRANSITION_SE,
    TILE_VARIANT_TRANSITION_SW,

    // ?X?
    // ..X
    // ..?
    TILE_VARIANT_TRANSITION_N_E,
    TILE_VARIANT_TRANSITION_N_W,
    TILE_VARIANT_TRANSITION_S_E,
    TILE_VARIANT_TRANSITION_S_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // ?X?
    TILE_VARIANT_TRANSITION_N_S,
    TILE_VARIANT_TRANSITION_E_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // ..X
    // ?X?
    TILE_VARIANT_TRANSITION_N_S_E,
    TILE_VARIANT_TRANSITION_N_S_W,
    TILE_VARIANT_TRANSITION_N_E_W,
    TILE_VARIANT_TRANSITION_S_E_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // X.X
    // ?X?
    TILE_VARIANT_TRANSITION_N_S_E_W,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // ...
    TILE_VARIANT_TRANSITION_NE_NW,
    TILE_VARIANT_TRANSITION_NE_SE,
    TILE_VARIANT_TRANSITION_NW_SW,
    TILE_VARIANT_TRANSITION_SE_SW,

    // Doesn't exist in MM7 data:
    // ..X
    // ...
    // X..
    TILE_VARIANT_TRANSITION_NE_SW,
    TILE_VARIANT_TRANSITION_NW_SE,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // ..X
    TILE_VARIANT_TRANSITION_NE_NW_SE,
    TILE_VARIANT_TRANSITION_NE_NW_SW,
    TILE_VARIANT_TRANSITION_NE_SE_SW,
    TILE_VARIANT_TRANSITION_NW_SE_SW,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // X.X
    TILE_VARIANT_TRANSITION_NE_NW_SE_SW,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // X.X
    TILE_VARIANT_TRANSITION_N_SE_SW,
    TILE_VARIANT_TRANSITION_S_NE_NW,
    TILE_VARIANT_TRANSITION_E_NW_SW,
    TILE_VARIANT_TRANSITION_W_NE_SE,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // ..X
    TILE_VARIANT_TRANSITION_N_SE,
    TILE_VARIANT_TRANSITION_N_SW,
    TILE_VARIANT_TRANSITION_S_NE,
    TILE_VARIANT_TRANSITION_S_NW,
    TILE_VARIANT_TRANSITION_E_NW,
    TILE_VARIANT_TRANSITION_E_SW,
    TILE_VARIANT_TRANSITION_W_NE,
    TILE_VARIANT_TRANSITION_W_SE,

    // Doesn't exist in MM7 data:
    // ?X?
    // ..X
    // X.?
    TILE_VARIANT_TRANSITION_N_E_SW,
    TILE_VARIANT_TRANSITION_N_W_SE,
    TILE_VARIANT_TRANSITION_S_E_NW,
    TILE_VARIANT_TRANSITION_S_W_NE,

    TILE_VARIANT_FIRST_SPECIAL = TILE_VARIANT_SPECIAL1,
    TILE_VARIANT_LAST_SPECIAL = TILE_VARIANT_SPECIAL8,
    TILE_VARIANT_FIRST_TRANSITION = TILE_VARIANT_TRANSITION_N,
    TILE_VARIANT_LAST_TRANSITION = TILE_VARIANT_TRANSITION_S_W_NE,
    TILE_VARIANT_FIRST_GENERATED = TILE_VARIANT_TRANSITION_N_S,
    TILE_VARIANT_LAST_GENERATED = TILE_VARIANT_TRANSITION_S_W_NE,
};
using enum TileVariant;
MM_DECLARE_SERIALIZATION_FUNCTIONS(TileVariant)

/**
 * Tile set id.
 *
 * Most of these tile sets don't exist in mm7 data, see comments.
 */
enum class Tileset {
    TILESET_INVALID,
    TILESET_GRASS,
    TILESET_SNOW,
    TILESET_DESERT,
    TILESET_DIRT, // This one has only 3 tiles.
    TILESET_WATER, // Water tile & shoreline tiles.
    TILESET_BADLANDS, // Looks like Deyja.
    TILESET_SWAMP,
    TILESET_COBBLE_ROAD, // Cobble road on dirt.

    TILESET_FIRST_TERRAIN = TILESET_GRASS,
    TILESET_LAST_TERRAIN = TILESET_SWAMP,
    TILESET_FIRST_ROAD = TILESET_COBBLE_ROAD,
    TILESET_LAST_ROAD = TILESET_COBBLE_ROAD,
};
using enum Tileset;
MM_DECLARE_SERIALIZATION_FUNCTIONS(Tileset)

enum class Direction {
    DIRECTION_NONE = 0,
    DIRECTION_N = 0x1,
    DIRECTION_S = 0x2,
    DIRECTION_E = 0x4,
    DIRECTION_W = 0x8,
    DIRECTION_NE = 0x10,
    DIRECTION_NW = 0x20,
    DIRECTION_SE = 0x40,
    DIRECTION_SW = 0x80,
};
using enum Direction;
MM_DECLARE_FLAGS(Directions, Direction)
MM_DECLARE_OPERATORS_FOR_FLAGS(Directions)

