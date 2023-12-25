#pragma once

#include <vector>

#include "Engine/Pid.h"

#include "TurnEngineEnums.h"

struct TurnBased_QueueElem {
    inline TurnBased_QueueElem() {
        uPackedID = Pid();
        actor_initiative = 0;
        uActionLength = Duration::zero();
        AI_action_type = TE_AI_STAND;
    }
    Pid uPackedID;
    int actor_initiative;  // act first who have less
    Duration uActionLength;
    TurnEngineAiAction AI_action_type;
};

struct stru262_TurnBased {
    inline stru262_TurnBased() {
        turns_count = 0;
        turn_stage = TE_NONE;
        ai_turn_timer = Duration::zero();
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
    TurnEngineStep turn_stage;  // if = 2 - action
    Duration ai_turn_timer;
    int turn_initiative;
    int uActionPointsLeft;  // 14
    TurnEngineFlags flags;
    int pending_actions;
    std::vector<TurnBased_QueueElem> pQueue;  // 20
};

extern struct stru262_TurnBased *pTurnEngine;
