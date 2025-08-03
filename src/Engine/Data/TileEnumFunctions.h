#include <span>
#include <cassert>

#include "TileEnums.h"

#include "Media/Audio/SoundEnums.h"

#include "Library/Geometry/Point.h"

#include "Utility/Segment.h"


//
// TileVariant
//

inline Segment<TileVariant> allSpecialTileVariants() {
    return {TILE_VARIANT_FIRST_SPECIAL, TILE_VARIANT_LAST_SPECIAL};
}

inline Segment<TileVariant> allTransitionTileVariants() {
    return {TILE_VARIANT_FIRST_TRANSITION, TILE_VARIANT_LAST_TRANSITION};
}

inline Segment<TileVariant> allGeneratedTileVariants() {
    return {TILE_VARIANT_FIRST_GENERATED, TILE_VARIANT_LAST_GENERATED};
}

Directions transitionDirectionsForTileVariant(TileVariant tileVariant);


//
// Tileset
//

inline bool isRoad(Tileset tileset) {
    return tileset >= TILESET_FIRST_ROAD && tileset <= TILESET_LAST_ROAD;
}

inline bool isTerrain(Tileset tileset) {
    return tileset >= TILESET_FIRST_TERRAIN && tileset <= TILESET_LAST_TERRAIN;
}

inline Segment<Tileset> allTerrainTilesets() {
    return {TILESET_FIRST_TERRAIN, TILESET_LAST_TERRAIN};
}

/**
 * @param tileset                       Tileset to get walk/run sound for.
 * @param isRunning                     Run flag.
 * @return                              Sound id to use.
 * @offset 0x47EE49
 */
SoundId walkSoundForTileset(Tileset tileset, bool isRunning);

int foodRequiredForTileset(Tileset tileset);


//
// Direction
//

inline std::span<const Direction> allDirections() {
    static constexpr std::array result = {DIRECTION_N, DIRECTION_S, DIRECTION_E, DIRECTION_W, DIRECTION_NE, DIRECTION_NW, DIRECTION_SE, DIRECTION_SW};
    return result;
}

inline Pointi offsetForDirection(Direction direction) {
    switch (direction) {
    case DIRECTION_N:       return {0, -1};
    case DIRECTION_S:       return {0, 1};
    case DIRECTION_E:       return {1, 0};
    case DIRECTION_W:       return {-1, 0};
    case DIRECTION_NE:      return {1, -1};
    case DIRECTION_NW:      return {-1, -1};
    case DIRECTION_SE:      return {1, 1};
    case DIRECTION_SW:      return {-1, 1};
    default:                assert(false); [[fallthrough]];
    case DIRECTION_NONE:    return {0, 0};
    }
}

TileVariant tileVariantForTransitionDirections(Directions directions);
