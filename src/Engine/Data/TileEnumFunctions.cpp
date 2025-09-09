#include "TileEnumFunctions.h"

#include <cassert>
#include <unordered_map>

#include "Utility/MapAccess.h"

static const std::unordered_map<TileVariant, Directions> transitionDirectionsByTileVariant = {
    {TILE_VARIANT_BASE1,                    DIRECTION_NONE},
    {TILE_VARIANT_TRANSITION_N,             DIRECTION_N},
    {TILE_VARIANT_TRANSITION_S,             DIRECTION_S},
    {TILE_VARIANT_TRANSITION_E,             DIRECTION_E},
    {TILE_VARIANT_TRANSITION_W,             DIRECTION_W},
    {TILE_VARIANT_TRANSITION_NE,            DIRECTION_NE},
    {TILE_VARIANT_TRANSITION_NW,            DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_SE,            DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_SW,            DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_N_E,           DIRECTION_N | DIRECTION_E},
    {TILE_VARIANT_TRANSITION_N_W,           DIRECTION_N | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_S_E,           DIRECTION_S | DIRECTION_E},
    {TILE_VARIANT_TRANSITION_S_W,           DIRECTION_S | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_N_S,           DIRECTION_N | DIRECTION_S},
    {TILE_VARIANT_TRANSITION_E_W,           DIRECTION_E | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_N_S_E,         DIRECTION_N | DIRECTION_S | DIRECTION_E},
    {TILE_VARIANT_TRANSITION_N_S_W,         DIRECTION_N | DIRECTION_S | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_N_E_W,         DIRECTION_N | DIRECTION_E | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_S_E_W,         DIRECTION_S | DIRECTION_E | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_N_S_E_W,       DIRECTION_N | DIRECTION_S | DIRECTION_E | DIRECTION_W},
    {TILE_VARIANT_TRANSITION_NE_NW,         DIRECTION_NE | DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_NE_SE,         DIRECTION_NE | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_NW_SW,         DIRECTION_NW | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_SE_SW,         DIRECTION_SE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_NE_SW,         DIRECTION_NE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_NW_SE,         DIRECTION_NW | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_NE_NW_SE,      DIRECTION_NE | DIRECTION_NW | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_NE_NW_SW,      DIRECTION_NE | DIRECTION_NW | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_NE_SE_SW,      DIRECTION_NE | DIRECTION_SE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_NW_SE_SW,      DIRECTION_NW | DIRECTION_SE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_NE_NW_SE_SW,   DIRECTION_NE | DIRECTION_NW | DIRECTION_SE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_N_SE_SW,       DIRECTION_N | DIRECTION_SE | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_S_NE_NW,       DIRECTION_S | DIRECTION_NE | DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_E_NW_SW,       DIRECTION_E | DIRECTION_NW | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_W_NE_SE,       DIRECTION_W | DIRECTION_NE | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_N_SE,          DIRECTION_N | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_N_SW,          DIRECTION_N | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_S_NE,          DIRECTION_S | DIRECTION_NE},
    {TILE_VARIANT_TRANSITION_S_NW,          DIRECTION_S | DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_E_NW,          DIRECTION_E | DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_E_SW,          DIRECTION_E | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_W_NE,          DIRECTION_W | DIRECTION_NE},
    {TILE_VARIANT_TRANSITION_W_SE,          DIRECTION_W | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_N_E_SW,        DIRECTION_N | DIRECTION_E | DIRECTION_SW},
    {TILE_VARIANT_TRANSITION_N_W_SE,        DIRECTION_N | DIRECTION_W | DIRECTION_SE},
    {TILE_VARIANT_TRANSITION_S_E_NW,        DIRECTION_S | DIRECTION_E | DIRECTION_NW},
    {TILE_VARIANT_TRANSITION_S_W_NE,        DIRECTION_S | DIRECTION_W | DIRECTION_NE}
};
static const std::unordered_map<Directions, TileVariant> tileVariantByTransitionDirections = inverted(transitionDirectionsByTileVariant);

Directions transitionDirectionsForTileVariant(TileVariant tileVariant) {
    return valueOr(transitionDirectionsByTileVariant, tileVariant, DIRECTION_NONE);
}

SoundId walkSoundForTileset(Tileset tileset, bool isRunning) {
    switch (tileset) {
    default:
        assert(false);
        [[fallthrough]];
    case TILESET_INVALID:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case TILESET_GRASS:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILESET_SNOW:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case TILESET_DESERT:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    // Leaving this code here for now, commented out b/c we've dropped TILESET_COOLED_LAVA.
    // case TILESET_COOLED_LAVA:
    //     return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case TILESET_DIRT:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt; // Water sounds were used.
    case TILESET_WATER:
        return isRunning ? SOUND_RunWater : SOUND_WalkWater; // Dirt sounds were used.
    case TILESET_BADLANDS:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case TILESET_SWAMP:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case TILESET_COBBLE_ROAD:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    }
}

int foodRequiredForTileset(Tileset tileset) {
    switch (tileset) {
    case TILESET_GRASS:
        return 1;
    case TILESET_SNOW:
    case TILESET_SWAMP:
        return 3;
    // Leaving this code here for now, commented out b/c we've dropped TILESET_COOLED_LAVA.
    // case TILESET_COOLED_LAVA:
    case TILESET_BADLANDS:
        return 4;
    case TILESET_DESERT:
        return 5;
    default:
        return 2;
    }
}

TileVariant tileVariantForTransitionDirections(Directions directions) {
    // First we need to canonize the directions a bit.
    if (directions & DIRECTION_N)
        directions &= ~(DIRECTION_NE | DIRECTION_NW);
    if (directions & DIRECTION_S)
        directions &= ~(DIRECTION_SE | DIRECTION_SW);
    if (directions & DIRECTION_E)
        directions &= ~(DIRECTION_NE | DIRECTION_SE);
    if (directions & DIRECTION_W)
        directions &= ~(DIRECTION_NW | DIRECTION_SW);

    // Then map. Value should always exist.
    return *valuePtr(tileVariantByTransitionDirections, directions);
}
