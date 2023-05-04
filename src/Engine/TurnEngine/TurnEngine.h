#pragma once

#include "Utility/Flags.h"

enum class TURN_ENGINE_FLAG {
    TE_FLAG_1 = 1,
    TE_HAVE_PENDING_ACTIONS = 2,
    TE_PLAYER_TURN = 4,
    TE_FLAG_8_finished = 8
};
using enum TURN_ENGINE_FLAG;
MM_DECLARE_FLAGS(TURN_ENGINE_FLAGS, TURN_ENGINE_FLAG)
MM_DECLARE_OPERATORS_FOR_FLAGS(TURN_ENGINE_FLAGS)

enum class TURN_ENGINE_AI_ACTION {
    TE_AI_STAND = 0,
    TE_AI_RANGED_ATTACK = 1,
    TE_AI_PURSUE = 2,
    TE_AI_MELEE_ATTACK = 3,
    TE_AI_FLEE = 4
};
using enum TURN_ENGINE_AI_ACTION;

enum class TURN_ENGINE_TURN_STAGE {
    TE_NONE = 0,
    TE_WAIT = 1,
    TE_ATTACK = 2,
    TE_MOVEMENT = 3,
};
using enum TURN_ENGINE_TURN_STAGE;

struct TurnBased_QueueElem {
    inline TurnBased_QueueElem() {
        uPackedID = 0;
        actor_initiative = 0;
        uActionLength = 0;
        AI_action_type = TE_AI_STAND;
    }
    int uPackedID;
    int actor_initiative;  // act first who have less
    int uActionLength;
    TURN_ENGINE_AI_ACTION AI_action_type;
};

struct stru262_TurnBased {
    inline stru262_TurnBased() {
        turns_count = 0;
        turn_stage = TE_NONE;
        ai_turn_timer = 0;
        uActorQueueSize = 0;
        turn_initiative = 0;
        uActionPointsLeft = 0;
        flags = 0;
        pending_actions = 0;
    }

    void SortTurnQueue();
    void ApplyPlayerAction();
    void Start();
    void End(bool bPlaySound);
    void AITurnBasedAction();
    void StartTurn();
    void NextTurn();
    bool StepTurnQueue();
    void _406457(int a2);
    void SetAIRecoveryTimes();
    void _4065B0();
    void AIAttacks(unsigned int queue_index);
    void AI_Action_(int queue_index);
    void ActorAISetMovementDecision();
    void ActorAIStopMovement();
    void ActorAIDoAdditionalMove();
    bool ActorMove(signed int a2);
    void ActorAIChooseNewTargets();

    int turns_count;
    TURN_ENGINE_TURN_STAGE turn_stage;  // if = 2 - action
    int ai_turn_timer;
    int uActorQueueSize;  // c
    int turn_initiative;
    int uActionPointsLeft;  // 14
    TURN_ENGINE_FLAGS flags;
    int pending_actions;
    TurnBased_QueueElem pQueue[530];  // 20
};

extern struct stru262_TurnBased *pTurnEngine;
