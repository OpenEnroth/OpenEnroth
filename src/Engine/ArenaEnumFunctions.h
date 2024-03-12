#pragma once

#include <cassert>

#include "ArenaEnums.h"

#include "GUI/GUIDialogues.h"

inline ArenaLevel arenaLevelForDialogue(DialogueId type) {
    switch (type) {
    default:                            assert(false); [[fallthrough]];
    case DIALOGUE_ARENA_SELECT_PAGE:    return ARENA_LEVEL_PAGE;
    case DIALOGUE_ARENA_SELECT_SQUIRE:  return ARENA_LEVEL_SQUIRE;
    case DIALOGUE_ARENA_SELECT_KNIGHT:  return ARENA_LEVEL_KNIGHT;
    case DIALOGUE_ARENA_SELECT_LORD:    return ARENA_LEVEL_LORD;
    }
}
