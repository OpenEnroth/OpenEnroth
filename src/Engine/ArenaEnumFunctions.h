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
    case ARENA_LEVEL_PAGE:      return AWARD_ARENA_PAGE_WINS;
    case ARENA_LEVEL_SQUIRE:    return AWARD_ARENA_SQUIRE_WINS;
    case ARENA_LEVEL_KNIGHT:    return AWARD_ARENA_KNIGHT_WINS;
    case ARENA_LEVEL_LORD:      return AWARD_ARENA_LORD_WINS;
    }
}
