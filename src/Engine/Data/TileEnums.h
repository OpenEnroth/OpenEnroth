#pragma once

#include "Utility/Flags.h"

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
    TILE_SCROLL_RIGHT = 0x2000
};
using enum TileFlag;
MM_DECLARE_FLAGS(TileFlags, TileFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(TileFlags)

/**
 * Tile variants inside a single tile set.
 *
 * There are two different tile set types in mm7:
 * 1. Normal tile sets.
 * 2. Road tile sets.
 *
 * The only road tile set we have in mm7 is cobble road over dirt.
 */
enum class TileVariant {
    TILE_VARIANT_INVALID = 255, // Tile with id 0 has variant = 255.
    TILE_VARIANT_BASE1 = 0, // Base tile. For roads - crossing.
    TILE_VARIANT_BASE2_NS = 1, // Base variation (swamp has one). For roads - NS.
    TILE_VARIANT_BASE3_EW = 2, // Base variation (swamp has one). For roads - EW.
    TILE_VARIANT_BASE4_NE = 3, // Base variation (not sure if this is used). For roads - NE turn.
    TILE_VARIANT_SPECIAL1_NW = 4, // For roads - NW turn.
    TILE_VARIANT_SPECIAL2_SE = 5, // For roads - SE turn.
    TILE_VARIANT_SPECIAL3_SW = 6, // For roads - SW turn.
    TILE_VARIANT_SPECIAL4_NS_E = 7, // For roads - T intersection, NS/E.
    TILE_VARIANT_SPECIAL5_NS_W = 8, // For roads - T intersection, NS/W.
    TILE_VARIANT_SPECIAL6_EW_N = 9, // For roads - T intersection, EW/N.
    TILE_VARIANT_SPECIAL7_EW_S = 0xA, // For roads - T intersection, EW/S.
    TILE_VARIANT_SPECIAL8_NCAP = 0xB, // For roads - N road end.
    TILE_VARIANT_NE1_SE1_ECAP = 0xC, // NE/SE transition to 3 x dirt. For roads - E road end.
    TILE_VARIANT_SCAP = 0xD, // For roads - S road end.
    TILE_VARIANT_NW1_SW1_WCAP = 0xE, // NW/SW transition to 3 x dirt. For roads - W road end.
    TILE_VARIANT_DN = 0xF, // For roads - Y intersection, N.
    TILE_VARIANT_E1_DS = 0x10, // E transition to dirt. For roads - Y intersection, S.
    TILE_VARIANT_W1_DW = 0x11, // W transition to dirt. For roads - Y intersection, W.
    TILE_VARIANT_N1_DE = 0x12, // N transition to dirt. For roads - Y intersection, E.
    TILE_VARIANT_S1_DSW = 0x13, // S transition to dirt. For roads - diagonal road, SW.
    TILE_VARIANT_XNE1_XSE1_DNE = 0x14, // NE/SE transition to 1 x dirt. For roads - diagonal road, NE.
    TILE_VARIANT_DSE = 0x15, // For roads - diagonal road, SE.
    TILE_VARIANT_XNW1_XSW1_DNW = 0x16, // NW/SW transition to 1 x dirt. For roads - diagonal road, NW.

    TILE_VARIANT_FIRST_SPECIAL = TILE_VARIANT_SPECIAL1_NW,
    TILE_VARIANT_LAST_SPECIAL = TILE_VARIANT_SPECIAL8_NCAP,
};
using enum TileVariant;

/**
 * Tile set id.
 *
 * Most of these tile sets don't exist in mm7 data, see comments.
 */
enum class Tileset {
    TILESET_INVALID = 255, // Tile with id 0 has tile set = 255.
    TILESET_GRASS = 0,
    TILESET_SNOW = 1,
    TILESET_DESERT = 2, // Sand.
    TILESET_COOLED_LAVA = 3, // Somehow this tileset is all dirt in the data files.
    TILESET_DIRT = 4, // This one has only 3 tiles.
    TILESET_WATER = 5, // Water tile & shoreline tiles.
    TILESET_BADLANDS = 6, // Looks like Deyja.
    TILESET_SWAMP = 7,
    TILESET_TROPICAL = 8, // This is all dirt.
    TILESET_CITY = 9, // This is sand too, lol.
    TILESET_ROAD_GRASS_COBBLE = 10, // Cobble road on dirt actually.
    TILESET_ROAD_GRASS_DIRT = 11, // This is all dirt.
    TILESET_ROAD_SNOW_COBBLE = 12, // This is all dirt.
    TILESET_ROAD_SNOW_DIRT = 13, // This is all dirt.
    TILESET_ROAD_SAND_COBBLE = 14, // This doesn't exist in mm7 tiles at all.
    TILESET_ROAD_SAND_DIRT = 15, // This doesn't exist in mm7 tiles at all.
    TILESET_ROAD_VOLCANO_COBBLE = 16, // This is all dirt.
    TILESET_ROAD_VOLCANO_DIRT = 17, // This is all dirt.
    TILESET_ROAD_CRACKED_COBBLE = 22, // This is all dirt.
    TILESET_ROAD_CRACKED_DIRT = 23, // This is all dirt.
    TILESET_ROAD_SWAMP_COBBLE = 24, // This is all dirt.
    TILESET_ROAD_SWAMP_DIRT = 25, // This is all dirt.
    TILESET_ROAD_TROPICAL_COBBLE = 26, // This is all dirt.
    TILESET_ROAD_TROPICAL_DIRT = 27, // This is all dirt.
    TILESET_ROAD_CITY_STONE = 28, // This is all dirt.
};
using enum Tileset;
