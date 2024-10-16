#pragma once

#include <cassert>

#include "Engine/Data/AwardEnums.h"
#include "GUI/GUIDialogues.h"

#include "ArenaEnums.h"

inline ArenaLevel arenaLevelForDialogue(DialogueId dialogueId) {
    switch (dialogueId) {
    default:                            assert(false); [[fallthrough]];
    case DIALOGUE_ARENA_SELECT_PAGE:    return ARENA_LEVEL_PAGE;
    case DIALOGUE_ARENA_SELECT_SQUIRE:  return ARENA_LEVEL_SQUIRE;
    case DIALOGUE_ARENA_SELECT_KNIGHT:  return ARENA_LEVEL_KNIGHT;
    case DIALOGUE_ARENA_SELECT_LORD:    return ARENA_LEVEL_LORD;
    }
}

inline DialogueId dialogueForArenaLevel(ArenaLevel level) {
    switch (level) {
    default:                  assert(false); [[fallthrough]];
    case ARENA_LEVEL_PAGE:    return DIALOGUE_ARENA_SELECT_PAGE;
    case ARENA_LEVEL_SQUIRE:  return DIALOGUE_ARENA_SELECT_SQUIRE;
    case ARENA_LEVEL_KNIGHT:  return DIALOGUE_ARENA_SELECT_KNIGHT;
    case ARENA_LEVEL_LORD:    return DIALOGUE_ARENA_SELECT_LORD;
    }
}

inline AwardType awardTypeForArenaLevel(ArenaLevel level) {
    switch (level) {
    default:                    assert(false); [[fallthrough]];
    case ARENA_LEVEL_PAGE:      return Award_Arena_PageWins;
    case ARENA_LEVEL_SQUIRE:    return Award_Arena_SquireWins;
    case ARENA_LEVEL_KNIGHT:    return Award_Arena_KnightWins;
    case ARENA_LEVEL_LORD:      return Award_Arena_LordWins;
    }
}
