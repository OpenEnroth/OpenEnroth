#pragma once

#include "Utility/Flags.h"

enum class TurnEngineFlag {
    TE_FLAG_1 = 1,
    TE_HAVE_PENDING_ACTIONS = 2,
    TE_PLAYER_TURN = 4,
    TE_FLAG_8_finished = 8
};
using enum TurnEngineFlag;
MM_DECLARE_FLAGS(TurnEngineFlags, TurnEngineFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(TurnEngineFlags)

enum class TurnEngineAiAction {
    TE_AI_STAND = 0,
    TE_AI_RANGED_ATTACK = 1,
    TE_AI_PURSUE = 2,
    TE_AI_MELEE_ATTACK = 3,
    TE_AI_FLEE = 4
};
using enum TurnEngineAiAction;

enum class TurnEngineStep {
    TE_NONE = 0,
    TE_WAIT = 1, // Monsters' turn.
    TE_ATTACK = 2, // Party is attacking.
    TE_MOVEMENT = 3, // Party is moving.
};
using enum TurnEngineStep;
