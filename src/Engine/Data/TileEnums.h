#pragma once

#include <cstdint>

#include "Utility/Flags.h"
#include "Utility/Segment.h"

// TODO(captainurist): #enum rename
enum class TILE_DESC_FLAG {
    TILE_DESC_NULL = 0,
    TILE_DESC_BURN = 0x1,
    TILE_DESC_WATER = 0x2, // Literally water.
    TILE_DESC_BLOCK = 0x4,
    TILE_DESC_REPULSE = 0x8,
    TILE_DESC_FLAT = 0x10,
    TILE_DESC_WAVY = 0x20,
    TILE_DESC_DONT_DRAW = 0x40,
    TILE_DESC_WATER_2 = 0x100, // Shore tile that's drawn on top of the water.
    TILE_DESC_TRANSITION = 0x200, // Transition tile, e.g. dirt-sand, dirt-water, grass-dirt, etc. All road tiles have this set.
    TILE_DESC_SCROLL_DOWN = 0x400,
    TILE_DESC_SCROLL_UP = 0x800,
    TILE_DESC_SCROLL_LEFT = 0x1000,
    TILE_DESC_SCROLL_RIGHT = 0x2000
};
using enum TILE_DESC_FLAG;
MM_DECLARE_FLAGS(TILE_DESC_FLAGS, TILE_DESC_FLAG)
MM_DECLARE_OPERATORS_FOR_FLAGS(TILE_DESC_FLAGS)

// TODO(captainurist): #enum rename
enum class TILE_SECT {
    TILE_SECT_NULL = -1,
    TILE_SECT_Start = -2,
    TILE_SECT_Base1 = 0, // Base tile. For roads - crossing.
    TILE_SECT_Base2_NS = 1, // Base variation (swamp has one). For roads - NS.
    TILE_SECT_Base3_EW = 2, // Base variation (swamp has one). For roads - EW.
    TILE_SECT_Base4_N_E = 3, // Base variation (not sure if this is used). For roads - NE turn.
    TILE_SECT_Special1_N_W = 4, // For roads - NW turn.
    TILE_SECT_Special2_S_E = 5, // For roads - SE turn.
    TILE_SECT_Special3_S_W = 6, // For roads - SW turn.
    TILE_SECT_Special4_NS_E = 7, // For roads - T intersection, NS/E.
    TILE_SECT_Special5_NS_W = 8, // For roads - T intersection, NS/W.
    TILE_SECT_Special6_EW_N = 9, // For roads - T intersection, EW/N.
    TILE_SECT_Special7_EW_S = 0xA, // For roads - T intersection, EW/S.
    TILE_SECT_Special8_NCAP = 0xB, // For roads - N road end.
    TILE_SECT_NE1_SE1_ECAP = 0xC, // NE/SE transition to 3 x dirt. For roads - E road end.
    TILE_SECT_SCAP = 0xD, // For roads - S road end.
    TILE_SECT_NW1_SW1_WCAP = 0xE, // NW/SW transition to 3 x dirt. For roads - W road end.
    TILE_SECT_DN = 0xF, // For roads - Y intersection, N.
    TILE_SECT_E1_DS = 0x10, // E transition to dirt. For roads - Y intersection, S.
    TILE_SECT_W1_DW = 0x11, // W transition to dirt. For roads - Y intersection, W.
    TILE_SECT_N1_DE = 0x12, // N transition to dirt. For roads - Y intersection, E.
    TILE_SECT_S1_DSW = 0x13, // S transition to dirt. For roads - diagonal road, SW.
    TILE_SECT_XNE1_XSE1_DNE = 0x14, // NE/SE transition to 1 x dirt. For roads - diagonal road, NE.
    TILE_SECT_DSE = 0x15, // For roads - diagonal road, SE.
    TILE_SECT_XNW1_XSW1_DNW = 0x16, // NW/SW transition to 1 x dirt. For roads - diagonal road, NW.

    TILE_SECT_FIRST_SPECIAL = TILE_SECT_Special1_N_W,
    TILE_SECT_LAST_SPECIAL = TILE_SECT_Special8_NCAP,
};
using enum TILE_SECT;

inline Segment<TILE_SECT> allSpecialTileSects() {
    return {TILE_SECT_FIRST_SPECIAL, TILE_SECT_LAST_SPECIAL};
}

// TODO(captainurist): #enum rename
// TODO(captainurist): rename enum values properly, City is Sand.
// Most of these tilesets don't exist in mm7 data, see comments. Everything's mapped to dirt.
#pragma warning(push)
#pragma warning(disable : 4341)
enum class Tileset : int16_t {
    // Tile 0 has Tileset = 255.
    Tileset_Grass = 0,
    Tileset_Snow = 1,
    Tileset_Desert = 2, // Sand.
    Tileset_CooledLava = 3, // Somehow this tileset is all dirt in the data files.
    Tileset_Dirt = 4, // This one has only 3 tiles.
    Tileset_Water = 5, // Water tile & shoreline tiles.
    Tileset_Badlands = 6, // Looks like Deyja.
    Tileset_Swamp = 7,
    Tileset_Tropical = 8, // This is all dirt.
    Tileset_City = 9, // This is sand too, lol.
    Tileset_RoadGrassCobble = 10, // Cobble road on dirt actually.
    Tileset_RoadGrassDirt = 11, // This is all dirt.
    Tileset_RoadSnowCobble = 12, // This is all dirt.
    Tileset_RoadSnowDirt = 13, // This is all dirt.
    Tileset_RoadSandCobble = 14, // This doesn't exist in mm7 tiles at all.
    Tileset_RoadSandDirt = 15, // This doesn't exist in mm7 tiles at all.
    Tileset_RoadVolcanoCobble = 16, // This is all dirt.
    Tileset_RoadVolcanoDirt = 17, // This is all dirt.
    Tileset_RoadCrackedCobble = 22, // This is all dirt.
    Tileset_RoadCrackedDirt = 23, // This is all dirt.
    Tileset_RoadSwampCobble = 24, // This is all dirt.
    Tileset_RoadSwampDir = 25, // This is all dirt.
    Tileset_RoadTropicalCobble = 26, // This is all dirt.
    Tileset_RoadTropicalDirt = 27, // This is all dirt.
    Tileset_RoadCityStone = 28, // This is all dirt.
    Tileset_NULL = -1,
    Tileset_Start = -2
};
using enum Tileset;
#pragma warning(pop)
