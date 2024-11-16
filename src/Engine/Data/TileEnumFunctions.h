#include "TileEnums.h"

#include "Media/Audio/SoundEnums.h"

#include "Utility/Segment.h"

inline Segment<TileVariant> allSpecialTileVariants() {
    return {TILE_VARIANT_FIRST_SPECIAL, TILE_VARIANT_LAST_SPECIAL};
}

/**
 * @param tileset                       Tileset to get walk/run sound for.
 * @param isRunning                     Run flag.
 * @return                              Sound id to use.
 * @offset 0x47EE49
 */
SoundId walkSoundForTileset(Tileset tileset, bool isRunning);

int foodRequiredForTileset(Tileset tileset);
