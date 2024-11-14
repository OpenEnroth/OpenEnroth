#include "TileEnums.h"

#include "Utility/Segment.h"

inline Segment<TileVariant> allSpecialTileVariants() {
    return {TILE_VARIANT_FIRST_SPECIAL, TILE_VARIANT_LAST_SPECIAL};
}
