#include "EnumSnapshots.h"

#include <string>

#include "Engine/Data/TileEnums.h"

void reconstruct(const TileVariant_MM7 &src, TileVariant *dst, ContextTag<bool> isRoad, ContextTag<std::string> name) {
    if (*isRoad) {
        switch (src) {
        case TILE_VARIANT_MM7_BASE1_NSEW:       *dst = TILE_VARIANT_ROAD_N_S_E_W; break;
        case TILE_VARIANT_MM7_BASE2_NS:         *dst = TILE_VARIANT_ROAD_N_S; break;
        case TILE_VARIANT_MM7_BASE3_EW:         *dst = TILE_VARIANT_ROAD_E_W; break;
        case TILE_VARIANT_MM7_BASE4_NE:         *dst = TILE_VARIANT_ROAD_N_E; break;
        case TILE_VARIANT_MM7_SPECIAL1_NW:      *dst = TILE_VARIANT_ROAD_N_W; break;
        case TILE_VARIANT_MM7_SPECIAL2_SE:      *dst = TILE_VARIANT_ROAD_S_E; break;
        case TILE_VARIANT_MM7_SPECIAL3_SW:      *dst = TILE_VARIANT_ROAD_S_W; break;
        case TILE_VARIANT_MM7_SPECIAL4_NS_E:    *dst = TILE_VARIANT_ROAD_N_S_E; break;
        case TILE_VARIANT_MM7_SPECIAL5_NS_W:    *dst = TILE_VARIANT_ROAD_N_S_W; break;
        case TILE_VARIANT_MM7_SPECIAL6_EW_N:    *dst = TILE_VARIANT_ROAD_N_E_W; break;
        case TILE_VARIANT_MM7_SPECIAL7_EW_S:    *dst = TILE_VARIANT_ROAD_S_E_W; break;
        case TILE_VARIANT_MM7_SPECIAL8_NCAP:    *dst = TILE_VARIANT_ROAD_N; break;
        case TILE_VARIANT_MM7_NE1_SE1_ECAP:     *dst = TILE_VARIANT_ROAD_E; break;
        case TILE_VARIANT_MM7_SCAP:             *dst = TILE_VARIANT_ROAD_S; break;
        case TILE_VARIANT_MM7_NW1_SW1_WCAP:     *dst = TILE_VARIANT_ROAD_W; break;
        case TILE_VARIANT_MM7_DN:               *dst = TILE_VARIANT_ROAD_N_E_W; break;
        case TILE_VARIANT_MM7_E1_DS:            *dst = TILE_VARIANT_ROAD_S_E_W; break;
        case TILE_VARIANT_MM7_W1_DW:            *dst = TILE_VARIANT_ROAD_N_S_W; break;
        case TILE_VARIANT_MM7_N1_DE:            *dst = TILE_VARIANT_ROAD_N_S_E; break;
        case TILE_VARIANT_MM7_S1_DSW:           *dst = TILE_VARIANT_ROAD_D_S_W; break;
        case TILE_VARIANT_MM7_XNE1_XSE1_DNE:    *dst = TILE_VARIANT_ROAD_D_N_E; break;
        case TILE_VARIANT_MM7_DSE:              *dst = TILE_VARIANT_ROAD_D_S_E; break;
        case TILE_VARIANT_MM7_XNW1_XSW1_DNW:    *dst = TILE_VARIANT_ROAD_D_N_W; break;
        default:                                *dst = TILE_VARIANT_INVALID; break;
        }
    } else {
        switch (src) {
        case TILE_VARIANT_MM7_BASE1_NSEW:       *dst = TILE_VARIANT_BASE1; break;
        case TILE_VARIANT_MM7_BASE2_NS:         *dst = TILE_VARIANT_BASE2; break;
        case TILE_VARIANT_MM7_BASE3_EW:         *dst = TILE_VARIANT_BASE3; break;
        case TILE_VARIANT_MM7_BASE4_NE:         *dst = TILE_VARIANT_BASE4; break;
        case TILE_VARIANT_MM7_SPECIAL1_NW:      *dst = TILE_VARIANT_SPECIAL1; break;
        case TILE_VARIANT_MM7_SPECIAL2_SE:      *dst = TILE_VARIANT_SPECIAL2; break;
        case TILE_VARIANT_MM7_SPECIAL3_SW:      *dst = TILE_VARIANT_SPECIAL3; break;
        case TILE_VARIANT_MM7_SPECIAL4_NS_E:    *dst = TILE_VARIANT_SPECIAL4; break;
        case TILE_VARIANT_MM7_SPECIAL5_NS_W:    *dst = TILE_VARIANT_SPECIAL5; break;
        case TILE_VARIANT_MM7_SPECIAL6_EW_N:    *dst = TILE_VARIANT_SPECIAL6; break;
        case TILE_VARIANT_MM7_SPECIAL7_EW_S:    *dst = TILE_VARIANT_SPECIAL7; break;
        case TILE_VARIANT_MM7_SPECIAL8_NCAP:    *dst = TILE_VARIANT_SPECIAL8; break;
        case TILE_VARIANT_MM7_NE1_SE1_ECAP:     *dst = name->ends_with("ne") ? TILE_VARIANT_TRANSITION_N_E : TILE_VARIANT_TRANSITION_S_E; break;
        case TILE_VARIANT_MM7_SCAP:             *dst = TILE_VARIANT_INVALID; break;
        case TILE_VARIANT_MM7_NW1_SW1_WCAP:     *dst = name->ends_with("nw") ? TILE_VARIANT_TRANSITION_N_W : TILE_VARIANT_TRANSITION_S_W; break;
        case TILE_VARIANT_MM7_DN:               *dst = TILE_VARIANT_INVALID; break;
        case TILE_VARIANT_MM7_E1_DS:            *dst = TILE_VARIANT_TRANSITION_E; break;
        case TILE_VARIANT_MM7_W1_DW:            *dst = TILE_VARIANT_TRANSITION_W; break;
        case TILE_VARIANT_MM7_N1_DE:            *dst = TILE_VARIANT_TRANSITION_N; break;
        case TILE_VARIANT_MM7_S1_DSW:           *dst = TILE_VARIANT_TRANSITION_S; break;
        case TILE_VARIANT_MM7_XNE1_XSE1_DNE:    *dst = name->ends_with("xne") ? TILE_VARIANT_TRANSITION_NE : TILE_VARIANT_TRANSITION_SE; break;
        case TILE_VARIANT_MM7_DSE:              *dst = TILE_VARIANT_INVALID; break;
        case TILE_VARIANT_MM7_XNW1_XSW1_DNW:    *dst = name->ends_with("xnw") ? TILE_VARIANT_TRANSITION_NW : TILE_VARIANT_TRANSITION_SW; break;
        default:                                *dst = TILE_VARIANT_INVALID; break;
        }
    }
}

void reconstruct(const Tileset_MM7 &src, Tileset *dst) {
    switch (src) {
    case TILESET_MM7_INVALID:               *dst = TILESET_INVALID; break;
    case TILESET_MM7_GRASS:                 *dst = TILESET_GRASS; break;
    case TILESET_MM7_SNOW:                  *dst = TILESET_SNOW; break;
    case TILESET_MM7_DESERT:                *dst = TILESET_DESERT; break;
    case TILESET_MM7_DIRT:                  *dst = TILESET_DIRT; break;
    case TILESET_MM7_WATER:                 *dst = TILESET_WATER; break;
    case TILESET_MM7_BADLANDS:              *dst = TILESET_BADLANDS; break;
    case TILESET_MM7_SWAMP:                 *dst = TILESET_SWAMP; break;
    case TILESET_MM7_ROAD_GRASS_COBBLE:     *dst = TILESET_COBBLE_ROAD; break;

    // These don't exist so we map them to what's in the tile data.
    case TILESET_MM7_COOLED_LAVA:           *dst = TILESET_DIRT; break;
    case TILESET_MM7_TROPICAL:              *dst = TILESET_DIRT; break;
    case TILESET_MM7_CITY:                  *dst = TILESET_DESERT; break;
    case TILESET_MM7_ROAD_GRASS_DIRT:       *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_SNOW_COBBLE:      *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_SNOW_DIRT:        *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_SAND_COBBLE:      *dst = TILESET_INVALID; break;
    case TILESET_MM7_ROAD_SAND_DIRT:        *dst = TILESET_INVALID; break;
    case TILESET_MM7_ROAD_VOLCANO_COBBLE:   *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_VOLCANO_DIRT:     *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_CRACKED_COBBLE:   *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_CRACKED_DIRT:     *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_SWAMP_COBBLE:     *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_SWAMP_DIRT:       *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_TROPICAL_COBBLE:  *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_TROPICAL_DIRT:    *dst = TILESET_DIRT; break;
    case TILESET_MM7_ROAD_CITY_STONE:       *dst = TILESET_DIRT; break;
    default:                                *dst = TILESET_INVALID; break;
    }
}
