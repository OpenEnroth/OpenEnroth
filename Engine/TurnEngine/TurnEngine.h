#pragma once


enum TURN_ENGINE_FLAGS
{
  TE_FLAG_1               = 1,
  TE_HAVE_PENDING_ACTIONS = 2,
  TE_PLAYER_TURN          = 4,
  TE_FLAG_8               = 8
};

enum TURN_ENGINE_AI_ACTION
{
  TE_AI_STAND         = 0,
  TE_AI_RANGED_ATTACK = 1,
  TE_AI_PURSUE        = 2,
  TE_AI_MELEE_ATTACK  = 3,
  TE_AI_FLEE          = 4
};

enum TURN_ENGINE_TURN_STAGE
{
  TE_NONE             = 0,
  TE_WAIT             = 1,
  TE_ATTACK           = 2,
  TE_MOVEMENT         = 3,
};

/*  299 */
#pragma pack(push, 1)
struct TurnBased_QueueElem
{
  inline TurnBased_QueueElem()
  {
    uPackedID = 0;
    actor_initiative = 0;
    uActionLength = 0;
    AI_action_type = 0;
  }
  int uPackedID;
  int actor_initiative;  //act first who have less
  int uActionLength;
  int AI_action_type;
};
#pragma pack(pop)

/*  254 */
#pragma pack(push, 1)
struct stru262_TurnBased
{
  inline stru262_TurnBased()
  {
    turns_count = 0;
    turn_stage = 0;
    ai_turn_timer = 0;
    uActorQueueSize = 0;
    turn_initiative = 0;
    uActionPointsLeft = 0;
    field_18 = 0;
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
  int turn_stage; //if = 2 - action
  int ai_turn_timer;
  int uActorQueueSize; //c
  int turn_initiative;
  int uActionPointsLeft; //14
  int field_18;
  int pending_actions;
  TurnBased_QueueElem pQueue[530]; //20
};
#pragma pack(pop)



extern struct stru262_TurnBased *pTurnEngine;