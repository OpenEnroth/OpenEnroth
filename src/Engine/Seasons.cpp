#include "Seasons.h"

#include <cassert>

#include "Engine/GameType.h"

Tileset tilesetForSeason(Tileset tileset, int month) {
    // MM8's Dagger Wound Island is a permanently-tropical setting - a fresh MM8 new game
    // starts in month 0 ("winter" under MM7's month->season mapping below), yet the original
    // renders plain grass there, not the grass->snow reskin MM7 applies.
    if (gameType() == GAME_TYPE_MM8)
        return tileset;

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
    // Same MM7-only seasonal-reskin mechanic as `tilesetForSeason` above, applied to
    // level-decoration sprites (flowers nulled out, some trees swapped) instead of terrain tiles.
    // MM8's decoration sprite table doesn't share MM7's id assignment, so nulling
    // out or offsetting whatever MM8 decoration happens to sit at one of these MM7 ids -
    // e.g. a building's sign - renders it as a missing/null placeholder sprite.
    if (gameType() == GAME_TYPE_MM8)
        return spriteId;

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
