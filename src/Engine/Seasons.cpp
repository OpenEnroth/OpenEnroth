#include "Seasons.h"

#include <cassert>

int tileIdForSeason(int tileId, int month) {
    switch (month) {
    case 11:
    case 0:
    case 1: // winter
        if (tileId >= 90 && tileId <= 113) { // TILESET_GRASS
            if (tileId <= 95)  // some grastyl entries
                return 348;
            return 348 + (tileId - 96);
        }
        return tileId;

    case 2:
    case 3:
    case 4: // spring
    case 8:
    case 9:
    case 10: // autumn
        if (tileId >= 90 && tileId <= 113)  // just convert all TILESET_GRASS to dirt
            return 1;
        return tileId;

    default:
        assert(false);
        [[fallthrough]];
    case 5:
    case 6:
    case 7: // summer
        // All tiles are green grass by default.
        return tileId;
    }
}

Tileset tilesetForSeason(Tileset tileset, int month) {
    switch (month) {
    case 11:
    case 0:
    case 1: // winter
        return tileset == TILESET_GRASS ? TILESET_SNOW : tileset;

    case 2:
    case 3:
    case 4: // spring
    case 8:
    case 9:
    case 10: // autumn
        return tileset == TILESET_GRASS ? TILESET_DIRT : tileset;

    default:
        assert(false);
        [[fallthrough]];
    case 5:
    case 6:
    case 7: // summer
        return tileset;
    }
}

int spriteIdForSeason(int spriteId, int month) {
    switch (month) {
    // case 531 (tree60), 536 (tree65), 537 (tree66) have no autumn/winter
    // sprites
    case 11:
    case 0:
    case 1: // winter
        switch (spriteId) {
        // case 468: // bush02    grows on swamps, which are evergreeen actually
        case 548: // flower10
        case 547: // flower09
        case 541: // flower03
        case 539: // flower01
            return 0; // null sprite

        case 483:  // tree01
        case 486:  // tree04
        case 492:  // tree10
            return spriteId + 2;

        default:
            return spriteId;
        }

    case 2:
    case 3:
    case 4: // spring
        return spriteId;

    case 8:
    case 9:
    case 10: // autumn
        switch (spriteId) {
        // case 468: // bush02 grows on swamps, which are evergreeen actually
        case 548: // flower10
        case 547: // flower09
        case 541: // flower03
        case 539: // flower01
            return 0; // null sprite

        case 483: // tree01
        case 486: // tree04
        case 492: // tree10
            return spriteId + 1;

        default:
            return spriteId;
        }

    default:
        assert(false);
        [[fallthrough]];
    case 5:
    case 6:
    case 7:
        return spriteId;
    }
}
