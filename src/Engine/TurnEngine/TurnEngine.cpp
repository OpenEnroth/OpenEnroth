#include <stdlib.h>

#include <utility>

#include "Engine/Engine.h"
#include "Engine/Time.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/Graphics/Viewport.h"

#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/IconFrameTable.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

#include "Engine/Party.h"
#include "Engine/AttackList.h"

struct stru262_TurnBased *pTurnEngine = new stru262_TurnBased;

//----- (00404544) --------------------------------------------------------
void stru262_TurnBased::SortTurnQueue() {
    int active_actors;
    TurnBased_QueueElem *current_top;   // eax@16
    TurnBased_QueueElem *test_element;  // ecx@18
    int i, j;
    ObjectType p_type;
    unsigned int p_id;

    active_actors = this->uActorQueueSize;
    // set non active actors in queue initiative that not allow them to
    // paticipate
    for (i = 0; i < uActorQueueSize; ++i) {
        p_type = PID_TYPE(pQueue[i].uPackedID);
        p_id = PID_ID(pQueue[i].uPackedID);

        if (p_type == OBJECT_Actor) {
            pActors[p_id].uAttributes |= ACTOR_STAND_IN_QUEUE;  // 0x80
            if (!pActors[p_id].CanAct()) {
                --active_actors;
                pQueue[i].actor_initiative = 1001;
                pActors[p_id].ResetQueue();
            }
        } else if (p_type == OBJECT_Player) {
            if (!pParty->pPlayers[p_id].CanAct()) {
                --active_actors;
                pQueue[i].actor_initiative = 1001;
            }
        }
    }
    // sort
    if (uActorQueueSize > 0) {
        for (i = 0; i < uActorQueueSize - 1; ++i) {
            current_top = &pQueue[i];
            for (j = i + 1; j < uActorQueueSize; ++j) {
                test_element = &pQueue[j];
                if (test_element->actor_initiative <
                        current_top
                            ->actor_initiative ||  // if less initiative -> top
                    ((test_element->actor_initiative ==
                      current_top->actor_initiative) &&
                     (((PID_TYPE(test_element->uPackedID) == OBJECT_Player) &&
                       (PID_TYPE(current_top->uPackedID) ==
                        OBJECT_Actor)) ||  // player preferable
                      ((PID_TYPE(test_element->uPackedID) ==
                        PID_TYPE(current_top->uPackedID)) &&
                       (PID_ID(test_element->uPackedID) <
                        PID_ID(
                            current_top->uPackedID)))))) {  // less id preferable
                    std::swap(*current_top, *test_element);
                }
            }
        }
    }
    uActorQueueSize = active_actors;
    if (PID_TYPE(pQueue[0].uPackedID) == OBJECT_Player) {  // we have player at queue top
        pParty->setActiveCharacter(PID_ID(pQueue[0].uPackedID) + 1);
        flags |= TE_PLAYER_TURN;
    } else {
        pParty->setActiveCharacter(0);
        flags &= ~TE_PLAYER_TURN;
    }
    for (i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) ==
            OBJECT_Player)  // set recovery times
            pParty->pPlayers[PID_ID(pQueue[i].uPackedID)].uTimeToRecovery =
                (uint16_t)((double)pQueue[i].actor_initiative *
                                   0.46875);
    }
}
//----- (0040471C) --------------------------------------------------------
void stru262_TurnBased::ApplyPlayerAction() {
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage == TE_ATTACK) _406457(0);
    }
}

//----- (004059DB) --------------------------------------------------------
void stru262_TurnBased::Start() {
    int v17;          // edx@22
    AIDirection v30;  // [sp+Ch] [bp-68h]@10
    AIDirection v31;  // [sp+28h] [bp-4Ch]@10
    AIDirection a3;   // [sp+44h] [bp-30h]@10
    int activ_players[4] {};
    int players_recovery_time[4] {};
    int a_players_count;
    int i, j;
    int temp;

    pTurnEngine->flags &= ~TE_HAVE_PENDING_ACTIONS;
    pEventTimer->TrackGameTime();
    pAudioPlayer->playUISound(SOUND_batllest);
    // pPlayer = pParty->pPlayers.data();
    dword_50C998_turnbased_icon_1A =
        pIconsFrameTable->GetIcon(uIconID_TurnStart)->GetAnimLength();
    dword_50C994 = 0;

    this->turn_initiative = 100;
    this->turns_count = 0;
    this->ai_turn_timer = 64;
    this->turn_stage = TE_WAIT;
    this->uActorQueueSize = 0;

    for (uint pl_id = 0; pl_id < 4; ++pl_id) {
        if (pParty->pPlayers[pl_id].CanAct()) {
            this->pQueue[this->uActorQueueSize].uPackedID =
                PID(OBJECT_Player, pl_id);
            this->pQueue[this->uActorQueueSize].AI_action_type = TE_AI_PURSUE;
            this->pQueue[this->uActorQueueSize].uActionLength = 0;
            pParty->pTurnBasedPlayerRecoveryTimes[this->uActorQueueSize] = 0;
            ++this->uActorQueueSize;
        }
    }

    for (int i = 0; i < ai_arrays_size; ++i) {
        if (ai_near_actors_ids[i] == 10) continue;
        if (pActors[ai_near_actors_ids[i]].CanAct()) {
            if (pActors[ai_near_actors_ids[i]].ActorNearby()) {
                pActors[ai_near_actors_ids[i]].uAttributes |=
                    ACTOR_STAND_IN_QUEUE;  // 0x80
                Actor::GetDirectionInfo(
                    PID(OBJECT_Actor, ai_near_actors_ids[i]),
                    ai_near_actors_targets_pid[ai_near_actors_ids[i]], &v31, 0);
                v30 = v31;
                Actor::AI_StandOrBored(ai_near_actors_ids[i], 4, 32, &v30);
                this->pQueue[this->uActorQueueSize].uPackedID =
                    PID(OBJECT_Actor, ai_near_actors_ids[i]);
                this->pQueue[this->uActorQueueSize].AI_action_type =
                    TE_AI_PURSUE;
                this->pQueue[this->uActorQueueSize].uActionLength = 0;
                ++this->uActorQueueSize;
            }
        }
    }

    a_players_count = 0;
    for (int k = 0; k < this->uActorQueueSize; ++k) {
        // set initial initiative for turn actors
        if (PID_TYPE(this->pQueue[k].uPackedID) == OBJECT_Player) {
            if (pPlayers[PID_ID(this->pQueue[k].uPackedID) + 1]
                ->uTimeToRecovery != 0) {
                this->pQueue[k].actor_initiative =
                    (int)((double)pPlayers
                        [PID_ID(this->pQueue[k].uPackedID) + 1]
                ->uTimeToRecovery *
                    0.46875);
            } else {
                activ_players[a_players_count] = k;
                ++a_players_count;
            }
        } else if (PID_TYPE(this->pQueue[k].uPackedID) == OBJECT_Actor) {
            v17 = grng->random(99);
            if (v17 < 33)
                this->pQueue[k].actor_initiative = 1;
            else
                this->pQueue[k].actor_initiative = (v17 >= 66) ? 5 : 3;
        } else {  // fot non player and actor
            this->pQueue[k].actor_initiative = 666;
        }
        this->pQueue[k].actor_initiative += 16;
    }

    if (a_players_count > 0) {
        for (i = 0; i < a_players_count; ++i)
            players_recovery_time[i] =
                pParty
                    ->pPlayers[PID_ID(this->pQueue[activ_players[i]].uPackedID)]
                    .GetAttackRecoveryTime(false);
        // sort players by recovery time
        for (i = 0; i < a_players_count - 1; ++i) {
            for (j = i + 1; j < a_players_count; ++j) {
                if (players_recovery_time[j] <
                    players_recovery_time[i]) {  // swap values
                    temp = players_recovery_time[i];
                    players_recovery_time[i] = players_recovery_time[j];
                    players_recovery_time[j] = temp;
                    temp = activ_players[i];
                    activ_players[i] = activ_players[j];
                    activ_players[j] = temp;
                }
            }
        }
        for (i = 0; i < a_players_count; ++i)
            this->pQueue[activ_players[i]].actor_initiative = i + 2;
    }
    this->SortTurnQueue();
}

//----- (00405CFF) --------------------------------------------------------
void stru262_TurnBased::End(bool bPlaySound) {
    ObjectType objType;  // eax@13
    int objID;           // esi@13
    int i;

    this->turn_stage = TE_NONE;
    for (i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor)
            pActors[PID_ID(pQueue[i].uPackedID)].ResetQueue();
    }

    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uAttributes & SPRITE_HALT_TURN_BASED)
            pSpriteObjects[i].uAttributes &= ~SPRITE_HALT_TURN_BASED;
    }

    for (i = 0; i < uActorQueueSize; ++i) {
        objType = (ObjectType)PID_TYPE(pQueue[i].uPackedID);
        objID = PID_ID(pQueue[i].uPackedID);
        if (objType == OBJECT_Player)
            pPlayers[objID + 1]->uTimeToRecovery =
                (uint16_t)((double)pQueue[i].actor_initiative *
                                   flt_debugrecmod3);
        else if (objType == OBJECT_Actor)
            pActors[objID].pMonsterInfo.uRecoveryTime =
                (uint16_t)((double)pQueue[i].actor_initiative *
                                   flt_debugrecmod3);
    }
    if (bPlaySound != 0)
        pAudioPlayer->playUISound(SOUND_batlleen);
    pTurnEngine->flags &= ~TE_HAVE_PENDING_ACTIONS;
    pEventTimer->StopGameTime();
    dword_50C994 = 0;
    dword_50C998_turnbased_icon_1A = 0;
}
// 50C994: using guessed type int dword_50C994;
// 50C998: using guessed type int dword_50C998_turnbased_icon_1A;

//----- (00405E14) --------------------------------------------------------
void stru262_TurnBased::AITurnBasedAction() {
    AIDirection v6;     // esi@21
    AIDirection a3;     // [sp+4h] [bp-68h]@21
    AIDirection v14;    // [sp+20h] [bp-4Ch]@21
    AIDirection v15;    // [sp+3Ch] [bp-30h]@21
    Actor *curr_actor;  // [sp+58h] [bp-14h]@2
    int target_pid;     // [sp+5Ch] [bp-10h]@6
    int shrinked;
    int j;

    for (uint i = 0; i < pActors.size(); ++i) {
        curr_actor = &pActors[i];
        shrinked = pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].Active();
        for (ACTOR_BUFF_INDEX j : pActors[i].pActorBuffs.indices())
            if (j != ACTOR_BUFF_MASS_DISTORTION)
                pActors[i].pActorBuffs[j].IsBuffExpiredToTime(pParty->GetPlayingTime());
        if (shrinked && pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].Expired())
            pActors[i].uActorHeight =
                pMonsterList->pMonsters[pActors[i].pMonsterInfo.uID - 1]
                    .uMonsterHeight;

        if (!(curr_actor->uAttributes & ACTOR_STAND_IN_QUEUE) &&
            !curr_actor->pActorBuffs[ACTOR_BUFF_STONED].Expired() &&
            !curr_actor->pActorBuffs[ACTOR_BUFF_PARALYZED].Expired()) {
            curr_actor->uCurrentActionTime += pMiscTimer->uTimeElapsed;
            if (curr_actor->uCurrentActionTime >=
                curr_actor->uCurrentActionLength) {
                target_pid = ai_near_actors_targets_pid[i];
                Actor::GetDirectionInfo(PID(OBJECT_Actor, i), target_pid, &v6,
                                        0);
                memcpy(&v15, &v6, sizeof(AIDirection));
                memcpy(&v14, &v15, sizeof(AIDirection));
                if (curr_actor->uAIState == Dying) {
                    curr_actor->uCurrentActionTime = 0;
                    curr_actor->uCurrentActionLength = 0;
                    curr_actor->uAIState = Dead;
                    curr_actor->UpdateAnimation();
                } else if ((curr_actor->uAIState > Removed) &&
                    (curr_actor->uAIState < Disabled)) {
                    Actor::AI_StandOrBored(i, target_pid, 32, &v14);
                }
            }
        }
    }
    if (turn_stage == TE_WAIT) {
        if (ai_turn_timer == 64) {
            ActorAISetMovementDecision();
        } else if (ai_turn_timer > 0) {
            ActorAIDoAdditionalMove();
        } else {
            ActorAIStopMovement();
            turn_initiative = 100;
        }
        ai_turn_timer -= pEventTimer->uTimeElapsed;
    } else if (turn_stage == TE_ATTACK) {
        if (!(flags & TE_FLAG_1)) {
            if (turn_initiative == 100) {
                StartTurn();
                SetAIRecoveryTimes();
                return;
            }
            if (turn_initiative > 0 || pQueue[0].actor_initiative <= 0) {
                _4065B0();
                SetAIRecoveryTimes();
                return;
            }
        }
        NextTurn();
    } else if (turn_stage == TE_MOVEMENT) {
        if ((uActionPointsLeft > 0) && (!(flags & TE_FLAG_8_finished))) {
            ActorAIChooseNewTargets();
        } else {
            flags &= ~TE_FLAG_8_finished;
            turn_stage = TE_WAIT;
            ai_turn_timer = 64;
        }
    }
}

//----- (00406051) --------------------------------------------------------
void stru262_TurnBased::StartTurn() {
    int player_num, actor_num, i, j;

    pending_actions = 0;
    // add player to queue if he can act
    for (player_num = 0; player_num < 4; ++player_num) {
        for (j = 0; j < uActorQueueSize; ++j) {
            if (PID_TYPE(pQueue[j].uPackedID) == OBJECT_Player) {
                if (pPlayers[PID_ID(pQueue[j].uPackedID) + 1]->CanAct() &&
                    (player_num != PID_ID(pQueue[j].uPackedID)))
                    break;
            }
        }
        if (j == uActorQueueSize) {
            pQueue[uActorQueueSize].uPackedID = PID(OBJECT_Player, player_num);
            pQueue[uActorQueueSize].actor_initiative = 100;
            pQueue[uActorQueueSize].uActionLength = 0;
            pQueue[uActorQueueSize].AI_action_type = TE_AI_STAND;
            ++uActorQueueSize;
        }
    }
    // add new arrived actors
    for (actor_num = 0; actor_num < ai_arrays_size; ++actor_num) {
        for (j = 0; j < uActorQueueSize; ++j) {
            if ((PID_TYPE(pQueue[j].uPackedID) == OBJECT_Actor) &&
                ai_near_actors_ids[actor_num] == PID_ID(pQueue[j].uPackedID))
                break;
        }
        if (j == uActorQueueSize) {
            pQueue[uActorQueueSize].uPackedID =
                PID(OBJECT_Actor, ai_near_actors_ids[actor_num]);
            pQueue[uActorQueueSize].actor_initiative = 1;
            pQueue[uActorQueueSize].uActionLength = 0;
            pQueue[uActorQueueSize].AI_action_type = TE_AI_STAND;
            ++uActorQueueSize;
        }
    }
    ++turns_count;
    turn_initiative = 100;
    for (i = 0; i < uActorQueueSize; ++i) {
        if (pQueue[i].actor_initiative == 0) pQueue[i].actor_initiative = 100;
    }
    StepTurnQueue();
    for (i = 0; i < uActorQueueSize; ++i) {
        if ((PID_TYPE(pQueue[i].uPackedID) == OBJECT_Player) ||
            (pQueue[i].actor_initiative > 0))
            break;
        AI_Action_(i);
    }
}
// 4F75D8: using guessed type int ai_arrays_size;

//----- (004061CA) --------------------------------------------------------
void stru262_TurnBased::NextTurn() {
    int v13;         // [sp+10h] [bp-4h]@7
    int monster_id;  // eax@5

    SortTurnQueue();
    if (PID_TYPE(pQueue[0].uPackedID) == OBJECT_Player)
        pParty->setActiveCharacter(PID_ID(pQueue[0].uPackedID) + 1);
    else
        pParty->setActiveCharacter(0);

    if (pending_actions) {
        pTurnEngine->flags |= TE_HAVE_PENDING_ACTIONS;
        return;
    }
    pTurnEngine->flags &= ~TE_HAVE_PENDING_ACTIONS;
    if (pQueue[0].actor_initiative <= 0) return;

    v13 = 0;
    if (uActorQueueSize > 0) {
        for (int i = 0; i < uActorQueueSize; ++i) {
            if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
                monster_id = PID_ID(pQueue[i].uPackedID);
                if ((pActors[monster_id].uAIState == Dying) ||
                    (pActors[monster_id].uAIState == Stunned) ||
                    (pActors[monster_id].uAIState == AttackingMelee) ||
                    (pActors[monster_id].uAIState == AttackingRanged1) ||
                    (pActors[monster_id].uAIState == AttackingRanged2) ||
                    (pActors[monster_id].uAIState == AttackingRanged3) ||
                    (pActors[monster_id].uAIState == AttackingRanged4) ||
                    (pActors[monster_id].uAIState == Summoned)) {
                    pActors[monster_id].uCurrentActionTime +=
                        pEventTimer->uTimeElapsed;
                    if (pActors[monster_id].uCurrentActionTime <
                        pActors[monster_id].uCurrentActionLength) {
                        v13 = 1;
                    } else if (pActors[monster_id].uAIState == Dying) {  // Dying
                        pActors[monster_id].uAIState = Dead;
                        pActors[monster_id].uCurrentActionTime = 0;
                        pActors[monster_id].uCurrentActionLength = 0;
                        pActors[monster_id].UpdateAnimation();
                    } else {
                        if (pActors[monster_id].uAIState == Stunned)  // Stunned
                            Actor::AI_StandOrBored(
                                monster_id,
                                ai_near_actors_targets_pid[monster_id], 32, 0);
                    }
                }
            }
        }
        if (v13 != 0) {
            flags |= TE_FLAG_1;
            return;
        }
    }

    flags &= ~TE_FLAG_1;
    // set all actors to stay
    for (int i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
            monster_id = PID_ID(pQueue[i].uPackedID);
            if ((pActors[monster_id].uAIState != Dead) &&
                (pActors[monster_id].uAIState != Dying) &&
                (pActors[monster_id].uAIState != Removed) &&
                (pActors[monster_id].uAIState != Summoned) &&
                (pActors[monster_id].uAIState != Disabled)) {
                pQueue[i].uActionLength = 0;
                Actor::AI_StandOrBored(monster_id,
                                       ai_near_actors_targets_pid[monster_id],
                                       32, nullptr);
            }
        }
    }

    // turn tick
    turn_stage = TE_MOVEMENT;
    pParty->GetPlayingTime().value += 213;
    _494035_timed_effects__water_walking_damage__etc();
    uActionPointsLeft = 130;
}

//----- (004063A1) --------------------------------------------------------
bool stru262_TurnBased::StepTurnQueue() {
    AIState v9;  // dx@12
    int j;

    SortTurnQueue();
    if (pQueue[0].actor_initiative != 0) {
        if (PID_TYPE(pQueue[0].uPackedID) == OBJECT_Player) {
            do {
                for (j = 0; j < uActorQueueSize; ++j)
                    --pQueue[j].actor_initiative;
                --turn_initiative;
                if (turn_initiative == 0) return true;
            } while (pQueue[0].actor_initiative != 0);
        } else {
            if (pQueue[0].actor_initiative > 0) {
                v9 = pActors[PID_ID(pQueue[0].uPackedID)].uAIState;
                if (!(v9 == Dying || v9 == Dead || v9 == Disabled ||
                      v9 == Removed)) {
                    do {
                        for (j = 0; j < uActorQueueSize; ++j) {
                            --pQueue[j].actor_initiative;
                            if (pQueue[j].actor_initiative == 0)
                                pQueue[j].uActionLength = 0;
                        }
                        --turn_initiative;
                        if (turn_initiative == 0) return true;
                    } while (pQueue[0].actor_initiative > 0);
                }
            }
        }
    }
    return false;
}

//----- (00406457) --------------------------------------------------------
void stru262_TurnBased::_406457(int a2) {
    signed int v4;  // ecx@2
    signed int v6;  // eax@2
    int i;
    v6 = 0;
    if (PID_TYPE(pQueue[a2].uPackedID) == OBJECT_Player) {
        v4 = PID_ID(pQueue[a2].uPackedID);
        if (pParty->pTurnBasedPlayerRecoveryTimes[v4]) {
            v6 = pParty->pTurnBasedPlayerRecoveryTimes[v4];
            pParty->pTurnBasedPlayerRecoveryTimes[v4] = 0;
        } else {
            v6 = pPlayers[v4 + 1]->GetAttackRecoveryTime(false);
        }
        if (v6 < 30) v6 = 30;
    } else {
        v6 =
            pMonsterStats
            ->pInfos[pActors[PID_ID(pQueue[a2].uPackedID)].pMonsterInfo.uID]
            .uRecoveryTime;
    }

    pQueue[a2].actor_initiative = v6;
    SortTurnQueue();
    if (PID_TYPE(pQueue[0].uPackedID) == OBJECT_Player)
        pParty->setActiveCharacter(PID_ID(pQueue[0].uPackedID) + 1);
    else
        pParty->setActiveCharacter(0);
    while ((pQueue[0].actor_initiative > 0) && (turn_initiative > 0)) {
        for (i = 0; i < uActorQueueSize; ++i) {
            --pQueue[i].actor_initiative;
            if (pQueue[i].actor_initiative == 0) pQueue[i].uActionLength = 0;
        }
        --turn_initiative;
    }
}

//----- (0040652A) --------------------------------------------------------
void stru262_TurnBased::SetAIRecoveryTimes() {
    int i;
    AIState monster_ai_state;
    Actor *monster;  // eax@5

    for (i = 0; i < uActorQueueSize; ++i) {
        if (pQueue[i].actor_initiative == 0) {
            if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Player) break;
            monster = &pActors[PID_ID(pQueue[i].uPackedID)];
            monster_ai_state = monster->uAIState;
            if (monster_ai_state == Standing || monster_ai_state == Fleeing ||
                monster_ai_state == Fidgeting) {
                pQueue[i].actor_initiative =
                    pMonsterStats->pInfos[monster->pMonsterInfo.uID]
                        .uRecoveryTime;
                if (monster->pActorBuffs[ACTOR_BUFF_SLOWED].Active())
                    pQueue[i].actor_initiative *= 2;
            }
        }
    }
}

//----- (004065B0) --------------------------------------------------------
void stru262_TurnBased::_4065B0() {
    int i;

    SortTurnQueue();
    if (pQueue[0].actor_initiative <= 0) {
        for (i = 0; i < uActorQueueSize; ++i) {
            if ((PID_TYPE(pQueue[i].uPackedID) == OBJECT_Player) ||
                (pQueue[i].actor_initiative > 0))
                break;
            if ((pQueue[i].uActionLength <= 0) &&
                (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor))
                AI_Action_(i);
        }
    } else {
        StepTurnQueue();
        if (PID_TYPE(pQueue[0].uPackedID) == OBJECT_Player)
            pParty->setActiveCharacter(PID_ID(pQueue[0].uPackedID) + 1);
        else
            pParty->setActiveCharacter(0);
    }
    for (i = 0; i < uActorQueueSize; ++i) AIAttacks(i);
}

//----- (00406648) --------------------------------------------------------
void stru262_TurnBased::AIAttacks(unsigned int queue_index) {
    // TurnBased_QueueElem *v1; // ecx@1
    // int v3; // eax@1
    unsigned int actor_id;  // ebx@2
    // Actor *v5; // esi@2
    ABILITY_INDEX v19;        // al@24
    AIDirection a3;  // [sp+Ch] [bp-3Ch]@2
    AIDirection a4;  // [sp+28h] [bp-20h]@2
    // TurnBased_QueueElem *v28; // [sp+44h] [bp-4h]@1
    // unsigned int a2a; // [sp+50h] [bp+8h]@2

    // v1 = &pQueue[queue_index];
    // v28 = v1;
    // v3 = pQueue[queue_index].uPackedID;
    if (PID_TYPE(pQueue[queue_index].uPackedID) == OBJECT_Actor) {
        actor_id = PID_ID(pQueue[queue_index].uPackedID);
        // a2a = ai_near_actors_targets_pid[v4];
        Actor::GetDirectionInfo(pQueue[queue_index].uPackedID,
                                ai_near_actors_targets_pid[actor_id], &a3, 0);
        memcpy(&a4, &a3, sizeof(a4));
        // v5 = &pActors[v4];
        // LOWORD(v3) = v5->uAIState;
        if ((pActors[actor_id].uAIState != Dead) &&
            (pActors[actor_id].uAIState != Disabled) &&
            (pActors[actor_id].uAIState != Removed)) {
            pActors[actor_id].uCurrentActionTime += pEventTimer->uTimeElapsed;
            if ((signed int)pActors[actor_id].uCurrentActionTime >= pActors[actor_id].uCurrentActionLength) {
                switch (pActors[actor_id].uAIState) {
                    case AttackingMelee:
                        v19 = pActors[actor_id].special_ability_use_check(actor_id);
                        pushMeleeAttack(pQueue[queue_index].uPackedID, pActors[actor_id].vPosition + Vec3i(0, 0, pActors[actor_id].uActorHeight / 2), v19);
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    case AttackingRanged1:
                        Actor::AI_RangedAttack(actor_id, &a4, pActors[actor_id].pMonsterInfo.uMissleAttack1Type, ABILITY_ATTACK1);
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    case Dying:
                        pActors[actor_id].uCurrentActionTime = 0;
                        pActors[actor_id].uCurrentActionLength = 0;
                        pActors[actor_id].uAIState = Dead;
                        pActors[actor_id].UpdateAnimation();
                        break;
                    case Stunned:
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    case AttackingRanged2:
                        Actor::AI_RangedAttack(actor_id, &a4, pActors[actor_id].pMonsterInfo.uMissleAttack2Type, ABILITY_ATTACK2);
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    case AttackingRanged3:
                        Actor::AI_SpellAttack(actor_id, &a4, pActors[actor_id].pMonsterInfo.uSpell1ID,
                                              ABILITY_SPELL1, pActors[actor_id].pMonsterInfo.uSpellSkillAndMastery1);
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    case AttackingRanged4:
                        Actor::AI_SpellAttack(actor_id, &a4, pActors[actor_id].pMonsterInfo.uSpell2ID,
                                              ABILITY_SPELL2, pActors[actor_id].pMonsterInfo.uSpellSkillAndMastery2);
                        Actor::AI_Stand(actor_id, ai_near_actors_targets_pid[actor_id], 0, &a4);
                        break;
                    default:
                        if (!grng->random(2))
                            Actor::AI_Bored(actor_id, ai_near_actors_targets_pid[actor_id], &a4);
                        else
                            Actor::AI_Stand( actor_id, ai_near_actors_targets_pid[actor_id], 64, &a4);
                }
            }
        }
    }
}

//----- (0040680F) --------------------------------------------------------
void stru262_TurnBased::AI_Action_(int queue_index) {
    unsigned int actor_id;  // edi@2
    AIDirection v7;         // esi@10
    int v9;                 // ecx@10
    signed int v10;         // eax@13
    ABILITY_INDEX v14;                // eax@29
    AIDirection a3;         // [sp+Ch] [bp-44h]@10
    AIDirection v18;        // [sp+28h] [bp-28h]@10
    signed int v22;         // [sp+58h] [bp+8h]@10

    pQueue[queue_index].uActionLength = 0;
    if (PID_TYPE(pQueue[queue_index].uPackedID) == OBJECT_Actor) {
        actor_id = PID_ID(pQueue[queue_index].uPackedID);
        if (!(pActors[actor_id].uAIState == Dying ||
              pActors[actor_id].uAIState == Dead ||
              pActors[actor_id].uAIState == Summoned ||
              pActors[actor_id].uAIState == Disabled ||
              pActors[actor_id].uAIState == Removed)) {
            Actor::_SelectTarget(actor_id,
                                 &ai_near_actors_targets_pid[actor_id], true);
            v22 = ai_near_actors_targets_pid[actor_id];
            if (pActors[actor_id].pMonsterInfo.uHostilityType && !v22)
                pActors[actor_id].pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Friendly;
            Actor::GetDirectionInfo(PID(OBJECT_Actor, actor_id), v22, &v7, 0);
            memcpy(&a3, &v7, sizeof(AIDirection));
            memcpy(&v18, &a3, sizeof(AIDirection));
            v9 = a3.uDistance - pActors[actor_id].uActorRadius;
            if (v9 < 0) v9 = 0;
            if (PID_TYPE(v22) == OBJECT_Actor)
                // v10 = (uint8_t)*(&byte_5C8D1A[89 *
                // (pMonsterStats->pInfos[pActors[PID_ID(v22)].pMonsterInfo.uID].uID
                // - 1) / 3] + (v5->pMonsterInfo.uID - 1) / 3);
                v10 = pFactionTable->relations
                          [(pMonsterStats
                                ->pInfos[pActors[PID_ID(v22)].pMonsterInfo.uID]
                                .uID) /
                               3 +
                           1][(pActors[actor_id].pMonsterInfo.uID - 1) / 3 + 1];
            else
                v10 = 4;
            switch (v10) {
                case 1:
                    if ((double)(signed int)v9 < 307.2)
                        pActors[actor_id].pMonsterInfo.uHostilityType =
                            MonsterInfo::Hostility_Long;
                    break;
                case 2:
                    if (v9 < 1024)
                        pActors[actor_id].pMonsterInfo.uHostilityType =
                            MonsterInfo::Hostility_Long;
                    break;
                case 3:
                    if (v9 < 2560)
                        pActors[actor_id].pMonsterInfo.uHostilityType =
                            MonsterInfo::Hostility_Long;
                    break;
                case 4:
                    if (v9 < 5120)
                        pActors[actor_id].pMonsterInfo.uHostilityType =
                            MonsterInfo::Hostility_Long;
                    break;
            }
            if (pActors[actor_id].pMonsterInfo.uHostilityType == 4 && v22 &&
                (signed int)v9 < 5120) {
                v14 = pActors[actor_id].special_ability_use_check(actor_id);
                pQueue[queue_index].AI_action_type = TE_AI_STAND;
                switch (v14) {
                    case ABILITY_ATTACK1:
                        if (pActors[actor_id].pMonsterInfo.uMissleAttack1Type) {
                            Actor::AI_MissileAttack1(actor_id, v22, &v18);
                            pQueue[queue_index].AI_action_type =
                                    TE_AI_RANGED_ATTACK;
                        }
                    case ABILITY_ATTACK2:
                        if (pActors[actor_id].pMonsterInfo.uMissleAttack2Type) {
                            Actor::AI_MissileAttack2(actor_id, v22, &v18);
                            pQueue[queue_index].AI_action_type =
                                TE_AI_RANGED_ATTACK;
                        }
                        break;
                    case ABILITY_SPELL1:
                        if (pActors[actor_id].pMonsterInfo.uSpell1ID != SPELL_NONE) {
                            Actor::AI_SpellAttack1(actor_id, v22, &v18);
                            pQueue[queue_index].AI_action_type =
                                TE_AI_RANGED_ATTACK;
                        }
                        break;
                    case ABILITY_SPELL2:
                        if (pActors[actor_id].pMonsterInfo.uSpell2ID != SPELL_NONE) {
                            Actor::AI_SpellAttack2(actor_id, v22, &v18);
                            pQueue[queue_index].AI_action_type =
                                TE_AI_RANGED_ATTACK;
                        }
                        break;
                    default:
                        Assert(false && "Unreachable");
                }
                // if (!pQueue[queue_index].AI_action_type)
                if ((double)v9 < 307.2) {
                    Actor::AI_MeleeAttack(actor_id, v22, &v18);
                    pQueue[queue_index].AI_action_type = TE_AI_MELEE_ATTACK;
                    pQueue[queue_index].uActionLength =
                        pActors[actor_id].uCurrentActionLength;
                    return;
                } else {
                    Actor::AI_Stand(actor_id, v22, 64, &v18);
                    pQueue[queue_index].AI_action_type = TE_AI_STAND;
                    pQueue[queue_index].uActionLength =
                        pActors[actor_id].uCurrentActionLength;
                    return;
                }
            } else {
                Actor::AI_Stand(actor_id, v22, 64, &v18);
                pQueue[queue_index].AI_action_type = TE_AI_STAND;
            }
            pQueue[queue_index].uActionLength =
                pActors[actor_id].uCurrentActionLength;
        }
    }
}

//----- (00406A63) --------------------------------------------------------
void stru262_TurnBased::ActorAISetMovementDecision() {
    AIDirection a3;           // [sp+8h] [bp-44h]@5
    AIDirection v7;           // [sp+24h] [bp-28h]@5
    unsigned int target_pid;  // [sp+40h] [bp-Ch]@5
    int i;

    this->ai_turn_timer = 64;
    dword_50C994 = 0;
    pParty->setActiveCharacter(0);
    for (i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
            target_pid =
                ai_near_actors_targets_pid[PID_ID(pQueue[i].uPackedID)];
            Actor::GetDirectionInfo(pQueue[i].uPackedID, target_pid, &v7, 0);
            if (!ActorMove(i))
                Actor::AI_Stand(PID_ID(pQueue[i].uPackedID), target_pid, 32,
                                &v7);
        }
    }
}
// 50C994: using guessed type int dword_50C994;

//----- (00406AFE) --------------------------------------------------------
void stru262_TurnBased::ActorAIStopMovement() {
    AIDirection a3;  // [sp+4h] [bp-48h]@5
    AIDirection v7;  // [sp+20h] [bp-2Ch]@5
    unsigned int target_pid;
    int i;

    for (i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
            target_pid =
                ai_near_actors_targets_pid[PID_ID(pQueue[i].uPackedID)];
            Actor::GetDirectionInfo(pQueue[i].uPackedID, target_pid, &v7, 0);
            Actor::AI_Stand(PID_ID(pQueue[i].uPackedID), target_pid, 32, &v7);
            pQueue[i].AI_action_type = TE_AI_STAND;
            pQueue[i].uActionLength = 0;
        }
    }
    turn_stage = TE_ATTACK;
    ai_turn_timer = 100;
}

//----- (00406B9F) --------------------------------------------------------
void stru262_TurnBased::ActorAIDoAdditionalMove() {
    AIDirection a3;    // [sp+0h] [bp-50h]@15
    AIDirection v9;    // [sp+1Ch] [bp-34h]@15
    unsigned int v13;  // [sp+44h] [bp-Ch]@8
    unsigned int monster_id;

    for (int i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
            monster_id = PID_ID(pQueue[i].uPackedID);
            if (!(pActors[monster_id].pActorBuffs[ACTOR_BUFF_STONED].Active() ||
                  pActors[monster_id]
                      .pActorBuffs[ACTOR_BUFF_PARALYZED]
                      .Active() ||
                  pActors[monster_id].uAIState == Dead ||
                  pActors[monster_id].uAIState == Removed ||
                  pActors[monster_id].uAIState == Disabled)) {
                v13 = ai_near_actors_targets_pid[PID_ID(pQueue[i].uPackedID)];
                Actor::GetDirectionInfo(pQueue[i].uPackedID, v13, &v9, 0);
                if (pActors[monster_id].uAIState == Pursuing ||
                    pActors[monster_id].uAIState == Tethered) {
                    if ((double)(signed int)v9.uDistance < 307.2)
                        Actor::AI_Stand(PID_ID(pQueue[i].uPackedID), v13, 32,
                                        &v9);
                } else {
                    pActors[monster_id].uCurrentActionTime +=
                        pEventTimer->uTimeElapsed;
                    if (pActors[monster_id].uCurrentActionTime >
                        pActors[monster_id].uCurrentActionLength) {
                        if (pActors[monster_id].uAIState == Dying) {
                            pActors[monster_id].uCurrentActionTime = 0;
                            pActors[monster_id].uCurrentActionLength = 0;
                            pActors[monster_id].uAIState = Dead;
                            pActors[monster_id].UpdateAnimation();
                        }
                        if (!ActorMove(i))
                            Actor::AI_Stand(PID_ID(pQueue[i].uPackedID), v13,
                                            32, &v9);
                    }
                }
            }
        }
    }
}

//----- (00406D10) --------------------------------------------------------
bool stru262_TurnBased::ActorMove(signed int queue_position) {
    AIDirection v9;                // esi@10
    int v11;                       // ecx@10
    uint8_t pHostileType;  // al@12
    AIDirection a3;                // [sp+Ch] [bp-48h]@10
    AIDirection pDir;              // [sp+28h] [bp-2Ch]@10
    unsigned int uActorID;         // [sp+50h] [bp-4h]@2

    if (PID_TYPE(pQueue[queue_position].uPackedID) == OBJECT_Player) return 0;
    uActorID = PID_ID(pQueue[queue_position].uPackedID);
    if (pActors[uActorID].uAIState == Dead ||
        pActors[uActorID].uAIState == Dying ||
        pActors[uActorID].uAIState == Removed ||
        pActors[uActorID].uAIState == Disabled ||
        pActors[uActorID].uAIState == Summoned)
        return 1;
    Actor::_SelectTarget(uActorID, &ai_near_actors_targets_pid[uActorID], true);
    if (pActors[uActorID].pMonsterInfo.uHostilityType &&
        !ai_near_actors_targets_pid[uActorID])
        pActors[uActorID].pMonsterInfo.uHostilityType =
            MonsterInfo::Hostility_Friendly;
    Actor::GetDirectionInfo(pQueue[queue_position].uPackedID,
                            ai_near_actors_targets_pid[uActorID], &v9, 0);
    memcpy(&a3, &v9, sizeof(AIDirection));
    memcpy(&pDir, &a3, sizeof(AIDirection));
    v11 = a3.uDistance - pActors[uActorID].uActorRadius;
    if (v11 < 0) v11 = 0;
    pHostileType = pActors[uActorID].pMonsterInfo.uHostilityType;
    switch (pHostileType) {
        case 1:
            if ((double)v11 < 307.2)
                pActors[uActorID].pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
            break;
        case 2:
            if (v11 < 1024)
                pActors[uActorID].pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
            break;
        case 3:
            if (v11 < 2560)
                pActors[uActorID].pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
            break;
    }
    if (pActors[uActorID].pActorBuffs[ACTOR_BUFF_AFRAID].Active()) {
        if (v11 < 10240) {
            Actor::AI_Flee(uActorID, ai_near_actors_targets_pid[uActorID], 0,
                           &pDir);
            pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_FLEE;
        } else {
            Actor::AI_RandomMove(uActorID, ai_near_actors_targets_pid[uActorID],
                                 1024, 0);
            pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_PURSUE;
        }
        pTurnEngine->pQueue[queue_position].uActionLength =
            pActors[uActorID].uCurrentActionLength;
        return true;
    }
    if (pActors[uActorID].pMonsterInfo.uHostilityType ==
        MonsterInfo::Hostility_Long) {
        if (!(pActors[uActorID].uAttributes & ACTOR_FLEEING) ||
            pActors[uActorID].pMonsterInfo.uAIType == 1) {
            if (pActors[uActorID].pMonsterInfo.uAIType == 1) {
                if (pActors[uActorID].pMonsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY)
                    Actor::AI_Stand(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                else
                    Actor::AI_Flee(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_FLEE;
                pTurnEngine->pQueue[queue_position].uActionLength = pActors[uActorID].uCurrentActionLength;
                return true;
            }
            if (pActors[uActorID].pMonsterInfo.uAIType == 2) {
                if (((double)pActors[uActorID].pMonsterInfo.uHP * 0.2) >
                        (double)pActors[uActorID].sCurrentHP &&
                    (v11 < 10240)) {
                    if (pActors[uActorID].pMonsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY)
                        Actor::AI_Stand(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                    else
                        Actor::AI_Flee(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                    pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_FLEE;
                    pTurnEngine->pQueue[queue_position].uActionLength = pActors[uActorID].uCurrentActionLength;
                    return true;
                }
            }
            if (pActors[uActorID].pMonsterInfo.uAIType == 3) {
                if (((double)pActors[uActorID].pMonsterInfo.uHP * 0.1) >
                        (double)pActors[uActorID].sCurrentHP &&
                    (v11 < 10240)) {
                    if (pActors[uActorID].pMonsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY)
                        Actor::AI_Stand(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                    else
                        Actor::AI_Flee(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
                    pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_FLEE;
                    pTurnEngine->pQueue[queue_position].uActionLength = pActors[uActorID].uCurrentActionLength;
                    return true;
                }
            }
        }
        if ((double)(signed int)v11 < 307.2) return 0;
        if ((signed int)v11 < 5120) {
            if (pActors[uActorID].pMonsterInfo.uMissleAttack1Type &&
                (signed int)v11 < 1024)
                Actor::AI_Pursue1(uActorID, ai_near_actors_targets_pid[uActorID], uActorID, 32, &pDir);
            else
                Actor::AI_Pursue2(uActorID, ai_near_actors_targets_pid[uActorID], 32, &pDir, 307);
            pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_PURSUE;
            pTurnEngine->pQueue[queue_position].uActionLength = pActors[uActorID].uCurrentActionLength;
            return true;
        }
    }
    switch (pActors[uActorID].pMonsterInfo.uMovementType) {
        case MONSTER_MOVEMENT_TYPE_SHORT:
            Actor::AI_RandomMove(uActorID, ai_near_actors_targets_pid[uActorID], 1024, 32);
            break;
        case MONSTER_MOVEMENT_TYPE_MEDIUM:
            Actor::AI_RandomMove(uActorID, ai_near_actors_targets_pid[uActorID], 2560, 32);
            break;
        case MONSTER_MOVEMENT_TYPE_LONG:
            Actor::AI_RandomMove(uActorID, ai_near_actors_targets_pid[uActorID], 5120, 32);
            break;
        case MONSTER_MOVEMENT_TYPE_FREE:
            Actor::AI_RandomMove(uActorID, ai_near_actors_targets_pid[uActorID], 10240, 32);
            break;
        case MONSTER_MOVEMENT_TYPE_STATIONARY:
            Actor::AI_Stand(uActorID, ai_near_actors_targets_pid[uActorID], 32, 0);
            break;
        default:
            return true;
    }
    pTurnEngine->pQueue[queue_position].AI_action_type = TE_AI_PURSUE;
    pTurnEngine->pQueue[queue_position].uActionLength = pActors[uActorID].uCurrentActionLength;
    return true;
}

//----- (00406FA8) --------------------------------------------------------
void stru262_TurnBased::ActorAIChooseNewTargets() {
    Actor *curr_acror;        // ebx@4
    AIDirection a3;           // [sp+Ch] [bp-6Ch]@8
    AIDirection v9;           // [sp+28h] [bp-50h]@8
    AIDirection a4;           // [sp+44h] [bp-34h]@8
    unsigned int target_pid;  // [sp+60h] [bp-18h]@1
    int uActorID;             // [sp+68h] [bp-10h]@4
    int i;

    for (i = 0; i < uActorQueueSize; ++i) {
        if (PID_TYPE(pQueue[i].uPackedID) == OBJECT_Actor) {
            uActorID = PID_ID(pQueue[i].uPackedID);
            curr_acror = &pActors[uActorID];
            if (!(curr_acror->uAIState == Summoned ||
                  curr_acror->uAIState == Dead ||
                  curr_acror->uAIState == Removed ||
                  curr_acror->uAIState == Disabled)) {
                target_pid = ai_near_actors_targets_pid[uActorID];
                Actor::_SelectTarget(
                    uActorID, &ai_near_actors_targets_pid[uActorID], true);
                Actor::GetDirectionInfo(pQueue[i].uPackedID, target_pid, &v9,
                                        0);
                memcpy(&a4, &v9, sizeof(AIDirection));
                curr_acror->uCurrentActionTime += pEventTimer->uTimeElapsed;
                if (curr_acror->uCurrentActionTime >
                    curr_acror->uCurrentActionLength) {
                    if (curr_acror->uAIState == Dying) {
                        curr_acror->uCurrentActionTime = 0;
                        curr_acror->uCurrentActionLength = 0;
                        curr_acror->uAIState = Dead;
                        curr_acror->UpdateAnimation();
                        break;
                    }
                    if (grng->random(2))
                        Actor::AI_Stand(uActorID, target_pid, 64, &a4);
                    else
                        Actor::AI_Bored(uActorID, target_pid, &a4);
                }
            }
        }
    }
}
