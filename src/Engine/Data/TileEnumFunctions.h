#include "TileEnums.h"

#include "Media/Audio/SoundEnums.h"

#include "Utility/Segment.h"

inline Segment<TileVariant> allSpecialTileVariants() {
    return {TILE_VARIANT_FIRST_SPECIAL, TILE_VARIANT_LAST_SPECIAL};
}

/**
 * @param tileSet                       Tileset to get walk/run sound for,
 * @param isRunning                     Run flag.
 * @return                              Sound id to use.
 * @offset 0x47EE49
 */
SoundId walkSoundForTileSet(TileSet tileSet, bool isRunning);
