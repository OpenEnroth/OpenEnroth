#include "Engine/Party.h"

#include <stdlib.h>
#include <climits>

#include "Engine/Engine.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;
using Io::Mouse;

Party *pParty = nullptr;

struct ActionQueue *pPartyActionQueue = new ActionQueue;

std::array<bool, 4> playerAlreadyPicked;  // byte_AE3368_
char PickedPlayer2_unused;                // byte_AE3369_
char PickedPlayer3_unused;                // byte_AE336A_
char PickedPlayer4_unused;                // byte_AE336B_

struct {
    UIAnimation _pUIAnim_Food;
    UIAnimation _pUIAnim_Gold;
    UIAnimation _pUIAnum_Torchlight;
    UIAnimation _pUIAnim_WizardEye;
} _uianim;

UIAnimation *pUIAnim_Food = &_uianim._pUIAnim_Food;
UIAnimation *pUIAnim_Gold = &_uianim._pUIAnim_Gold;
UIAnimation *pUIAnum_Torchlight = &_uianim._pUIAnum_Torchlight;
UIAnimation *pUIAnim_WizardEye = &_uianim._pUIAnim_WizardEye;

std::array<class UIAnimation *, 4>
    pUIAnims =  // was struct byt defined as class
    {&_uianim._pUIAnim_Food, &_uianim._pUIAnim_Gold,
     &_uianim._pUIAnum_Torchlight, &_uianim._pUIAnim_WizardEye};

//----- (0044A56A) --------------------------------------------------------
void Party::CountHirelings() {  // non hired followers
    cNonHireFollowers = 0;

    for (unsigned int i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        NPCData *npc = &pNPCStats->pNewNPCData[i];
        if (npc->Hired() &&
            (!pHirelings[0].pName || strcmp(npc->pName, pHirelings[0].pName))) {
            if (!pHirelings[1].pName || strcmp(npc->pName, pHirelings[1].pName))
                ++cNonHireFollowers;
        }
    }
}

// inlined
//----- (mm6c::004858D0) --------------------------------------------------
void Party::Zero() {
    uFlags2 = 0;
    uNumGoldInBank = 0;

    uCurrentYear = 0;
    uCurrentMonth = 0;
    uCurrentMonthWeek = 0;
    uCurrentDayOfMonth = 0;
    uCurrentHour = 0;
    uCurrentMinute = 0;
    uCurrentTimeSecond = 0;

    _6FC_water_lava_timer = 0;
    days_played_without_rest = 0;
    vPosition.x = 0;
    vPosition.y = 0;
    vPosition.z = 0;
    uFallStartY = 0;
    sRotationZ = 0;
    sRotationX = 0;
    uFallSpeed = 0;
    field_28 = 0;
    uDefaultPartyHeight = 192;
    field_14_radius = 37;
    y_rotation_granularity = 25;
    y_rotation_speed = 90;

    uWalkSpeed = 384;
    walk_sound_timer = 0;

    field_24 = 5;
    _6FC_water_lava_timer = 0;
    field_708 = 15;
    field_0 = 25;

    uNumDeaths = 0;
    uNumPrisonTerms = 0;
    uNumBountiesCollected = 0;
    monster_for_hunting_killed.fill(0);
    monster_id_for_hunting.fill(0);
    memset(_quest_bits, 0, sizeof(_quest_bits));
    pArcomageWins.fill(0);
    uNumArenaPageWins = 0;
    uNumArenaSquireWins = 0;
    uNumArenaKnightWins = 0;
    uNumArenaLordWins = 0;

    for (int y = 0; y < 4; y++) {
        auto player = &pPlayers[y];
        for (int z = 0; z < player->vBeacons.size(); z++) {
            player->vBeacons[z].image->Release();
        }
        player->vBeacons.clear();
    }
}

// inlined
//----- (mm6c::0045BE90) --------------------------------------------------
void ActionQueue::Reset() { uNumActions = 0; }

//----- (004760C1) --------------------------------------------------------
void ActionQueue::Add(PartyAction action) {
    if (uNumActions < 30) pActions[uNumActions++] = action;
}

//----- (00497FC5) --------------------------------------------------------
bool Party::_497FC5_check_party_perception_against_level() {
    int uMaxPerception;  // edi@1
    signed int v5;       // eax@3
    bool result;         // eax@7

    uMaxPerception = 0;
    for (int i = 0; i < 4; i++) {
        if (this->pPlayers[i].CanAct()) {
            v5 = this->pPlayers[i].GetPerception();
            if (v5 > uMaxPerception) uMaxPerception = v5;
        }
    }
    if (uLevelMapStatsID && (signed int)uLevelMapStatsID < 77)
        result = uMaxPerception >= 2 * pMapStats->pInfos[uLevelMapStatsID]._per;
    else
        result = 0;
    return result;
}

//----- (004936E1) --------------------------------------------------------
void Party::SetHoldingItem(ItemGen *pItem) {
    PickedItem_PlaceInInventory_or_Drop();
    memcpy(&pPickedItem, pItem, sizeof(pPickedItem));
    mouse->SetCursorBitmapFromItemID(pPickedItem.uItemID);
}

int Party::GetFirstCanAct() {  // added to fix some nzi problems entering shops
    for (int i = 0; i < 4; ++i) {
        if (this->pPlayers[i].CanAct()) return i + 1;
    }

    __debugbreak();  // should not get here
    return 1;
}

//----- (0049370F) --------------------------------------------------------
int Party::GetNextActiveCharacter() {
    int v2;         // eax@4
    signed int v8;  // esi@23
    int v12;        // [sp+Ch] [bp-4h]@1

    if (uActiveCharacter > 0 && this->pPlayers[uActiveCharacter - 1].CanAct() &&
        this->pPlayers[uActiveCharacter - 1].uTimeToRecovery <
            1)  // avoid switching away from char that can act
        return uActiveCharacter;

    v12 = 0;
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage != TE_ATTACK || PID_TYPE(pTurnEngine->pQueue[0].uPackedID) != OBJECT_Player) {
            return 0;
        }
        v2 = PID_ID(pTurnEngine->pQueue[0].uPackedID);
        return v2 + 1;
    }

    if (playerAlreadyPicked[0] && playerAlreadyPicked[1] &&
        playerAlreadyPicked[2] && playerAlreadyPicked[3])
        playerAlreadyPicked.fill(false);

    for (int i = 0; i < 4; i++) {
        if (!this->pPlayers[i].CanAct() ||
            this->pPlayers[i].uTimeToRecovery > 0) {
            playerAlreadyPicked[i] = true;
        } else if (!playerAlreadyPicked[i]) {
            playerAlreadyPicked[i] = true;
            if (i >
                0)  // TODO(_) check if this condition really should be here. it is
                    // equal to the original source but still seems kind of weird
                return i + 1;
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (this->pPlayers[i].CanAct() &&
            this->pPlayers[i].uTimeToRecovery == 0) {
            if (v12 == 0 || this->pPlayers[i].uSpeedBonus > v8) {
                v8 = this->pPlayers[i].uSpeedBonus;
                v12 = i + 1;
            }
        }
    }
    return v12;
}

//----- (00493244) --------------------------------------------------------
bool Party::HasItem(unsigned int uItemID) {
    for (int player = 0; player < 4; player++) {
        for (int itemPos = 0; itemPos < 138; itemPos++) {
            if (pParty->pPlayers[player].pOwnItems[itemPos].uItemID == uItemID)
                return true;
        }
    }
    return false;
}

void ui_play_gold_anim() {
    pUIAnim_Gold->uAnimTime = 0;
    pUIAnim_Gold->uAnimLength = pUIAnim_Gold->icon->GetAnimLength();
    pAudioPlayer->PlaySound(SOUND_gold01, 0, 0, -1, 0, 0);
}

void ui_play_food_anim() {
    pUIAnim_Food->uAnimTime = 0;
    pUIAnim_Food->uAnimLength = pUIAnim_Food->icon->GetAnimLength();
    // pAudioPlayer->PlaySound(SOUND_eat, 0, 0, -1, 0, 0);
}

//----- (00492AD5) --------------------------------------------------------
void Party::SetFood(int amount) {
    pParty->uNumFoodRations = amount;
    ui_play_food_anim();
}

//----- (00492B03) --------------------------------------------------------
void Party::TakeFood(int amount) {
    if (pParty->uNumFoodRations <= amount)
        pParty->uNumFoodRations = 0;
    else
        pParty->uNumFoodRations -= amount;

    ui_play_food_anim();
}

//----- (00492B42) --------------------------------------------------------
void Party::GiveFood(int amount) {
    pParty->uNumFoodRations += amount;

    if (pParty->uNumFoodRations > 0xFFFF)
        Party::SetFood(0xFFFFu);

    ui_play_food_anim();
}

int Party::GetGold() const {
    if (engine->config->debug_infinite_gold) {
        return 99999;
    }

    return uNumGold;
}

int Party::GetFood() const {
    if (engine->config->debug_infinite_food) {
        return 99999;
    }

    return uNumFoodRations;
}

//----- (00492B70) --------------------------------------------------------
void Party::SetGold(int amount) {
    pParty->uNumGold = amount;

    ui_play_gold_anim();
}

void Party::AddGold(int amount) {
    SetGold(pParty->GetGold() + amount);
}

//----- (00492BB6) --------------------------------------------------------
void Party::TakeGold(int amount) {
    if (amount <= pParty->uNumGold)
        pParty->uNumGold -= amount;
    else
        pParty->uNumGold = 0;

    ui_play_gold_anim();
}

//----- (0049135E) --------------------------------------------------------
unsigned int Party::GetPartyFame() {
    unsigned __int64 total_exp = 0;
    for (uint i = 0; i < 4; ++i) total_exp += pPlayers[i].uExperience;
    return std::min(
        (unsigned int)(total_exp / 1000),
        UINT_MAX);  // min wasn't present, but could be incorrect without it
}

//----- (0049137D) --------------------------------------------------------
void Party::CreateDefaultParty(bool bDebugGiveItems) {
    Player *pCharacter;      // esi@3
    int uSkillIdx;           // eax@11
    unsigned int v16;        // [sp-4h] [bp-44h]@26
    signed int uNumPlayers;  // [sp+18h] [bp-28h]@1
    ItemGen Dst;             // [sp+1Ch] [bp-24h]@10

    pHireling1Name[0] = 0;
    pHireling2Name[0] = 0;
    this->hirelingScrollPosition = 0;
    memset(&pHirelings, 0, sizeof(pHirelings));

    strcpy(this->pPlayers[0].pName, localization->GetString(509));  // Zoltan
    this->pPlayers[0].uPrevFace = 17;
    this->pPlayers[0].uCurrentFace = 17;
    this->pPlayers[0].uPrevVoiceID = 17;
    this->pPlayers[0].uVoiceID = 17;
    this->pPlayers[0].uMight = 30;
    this->pPlayers[0].uIntelligence = 5;
    this->pPlayers[0].uWillpower = 5;
    this->pPlayers[0].uEndurance = 13;
    this->pPlayers[0].uAccuracy = 13;
    this->pPlayers[0].uSpeed = 14;
    this->pPlayers[0].uLuck = 7;
    this->pPlayers[0].pActiveSkills[PLAYER_SKILL_LEATHER] = 1;
    this->pPlayers[0].pActiveSkills[PLAYER_SKILL_ARMSMASTER] = 1;
    this->pPlayers[0].pActiveSkills[PLAYER_SKILL_BOW] = 1;
    this->pPlayers[0].pActiveSkills[PLAYER_SKILL_SWORD] = 1;

    strcpy(this->pPlayers[1].pName, localization->GetString(506));  // Roderic
    this->pPlayers[1].uPrevFace = 3;
    this->pPlayers[1].uCurrentFace = 3;
    this->pPlayers[1].uPrevVoiceID = 3;
    this->pPlayers[1].uVoiceID = 3;
    this->pPlayers[1].uMight = 13;
    this->pPlayers[1].uIntelligence = 9;
    this->pPlayers[1].uWillpower = 9;
    this->pPlayers[1].uEndurance = 13;
    this->pPlayers[1].uAccuracy = 13;
    this->pPlayers[1].uSpeed = 13;
    this->pPlayers[1].uLuck = 13;
    this->pPlayers[1].pActiveSkills[PLAYER_SKILL_LEATHER] = 1;
    this->pPlayers[1].pActiveSkills[PLAYER_SKILL_STEALING] = 1;
    this->pPlayers[1].pActiveSkills[PLAYER_SKILL_DAGGER] = 1;
    this->pPlayers[1].pActiveSkills[PLAYER_SKILL_TRAP_DISARM] = 1;

    strcpy(this->pPlayers[2].pName, localization->GetString(508));  // Serena
    this->pPlayers[2].uPrevFace = 14;
    this->pPlayers[2].uCurrentFace = 14;
    this->pPlayers[2].uPrevVoiceID = 14;
    this->pPlayers[2].uVoiceID = 14;
    this->pPlayers[2].uMight = 12;
    this->pPlayers[2].uIntelligence = 9;
    this->pPlayers[2].uWillpower = 20;
    this->pPlayers[2].uEndurance = 22;
    this->pPlayers[2].uAccuracy = 7;
    this->pPlayers[2].uSpeed = 13;
    this->pPlayers[2].uLuck = 7;
    this->pPlayers[2].pActiveSkills[PLAYER_SKILL_ALCHEMY] = 1;
    this->pPlayers[2].pActiveSkills[PLAYER_SKILL_LEATHER] = 1;
    this->pPlayers[2].pActiveSkills[PLAYER_SKILL_BODY] = 1;
    this->pPlayers[2].pActiveSkills[PLAYER_SKILL_MACE] = 1;

    strcpy(this->pPlayers[3].pName, localization->GetString(507));  // Alexis
    this->pPlayers[3].uPrevFace = 10;
    this->pPlayers[3].uCurrentFace = 10;
    this->pPlayers[3].uEndurance = 13;
    this->pPlayers[3].uAccuracy = 13;
    this->pPlayers[3].uSpeed = 13;
    this->pPlayers[3].uPrevVoiceID = 10;
    this->pPlayers[3].uVoiceID = 10;
    this->pPlayers[3].uMight = 5;
    this->pPlayers[3].uIntelligence = 30;
    this->pPlayers[3].uWillpower = 9;
    this->pPlayers[3].uLuck = 7;
    this->pPlayers[3].pActiveSkills[PLAYER_SKILL_LEATHER] = 1;
    this->pPlayers[3].pActiveSkills[PLAYER_SKILL_AIR] = 1;
    this->pPlayers[3].pActiveSkills[PLAYER_SKILL_FIRE] = 1;
    this->pPlayers[3].pActiveSkills[PLAYER_SKILL_STAFF] = 1;

    for (uNumPlayers = 0; uNumPlayers < 4; uNumPlayers++) {
        pCharacter = &pParty->pPlayers[uNumPlayers];

        if (pCharacter->classType == PLAYER_CLASS_KNIGHT)
            pCharacter->sResMagicBase = 10;

        pCharacter->lastOpenedSpellbookPage = 0;
        for (int i = 0; i < 9; i++) {  // for Magic Book
            if (pPlayers[uNumPlayers].pActiveSkills[12 + i]) {
                pCharacter->lastOpenedSpellbookPage = i;
                break;
            }
        }

        pCharacter->uExpressionTimePassed = 0;

        if (bDebugGiveItems) {
            Dst.Reset();
            pItemsTable->GenerateItem(2, 40, &Dst);  // ring
            pCharacter->AddItem2(-1, &Dst);
            for (uSkillIdx = 0; uSkillIdx < 36; uSkillIdx++) {
                if (pCharacter->pActiveSkills[uSkillIdx]) {
                    switch (uSkillIdx) {
                        case PLAYER_SKILL_STAFF:
                            pCharacter->WearItem(ITEM_STAFF_1);
                            break;
                        case PLAYER_SKILL_SWORD:
                            pCharacter->WearItem(ITEM_LONGSWORD_1);
                            break;
                        case PLAYER_SKILL_DAGGER:
                            pCharacter->WearItem(ITEM_DAGGER_1);
                            break;
                        case PLAYER_SKILL_AXE:
                            pCharacter->WearItem(ITEM_AXE_1);
                            break;
                        case PLAYER_SKILL_SPEAR:
                            pCharacter->WearItem(ITEM_SPEAR_1);
                            break;
                        case PLAYER_SKILL_BOW:
                            pCharacter->WearItem(ITEM_CROSSBOW_1);
                            break;
                        case PLAYER_SKILL_MACE:
                            pCharacter->WearItem(ITEM_MACE_1);
                            break;
                        case PLAYER_SKILL_SHIELD:
                            pCharacter->WearItem(ITEM_BUCKLER_1);
                            break;
                        case PLAYER_SKILL_LEATHER:
                            pCharacter->WearItem(ITEM_LEATHER_1);
                            break;
                        case PLAYER_SKILL_CHAIN:
                            pCharacter->WearItem(ITEM_CHAINMAIL_1);
                            break;
                        case PLAYER_SKILL_PLATE:
                            pCharacter->WearItem(ITEM_PLATE_1);
                            break;
                        case PLAYER_SKILL_FIRE:
                            pCharacter->AddItem(-1, ITEM_SPELLBOOK_FIRE_STRIKE);
                            break;
                        case PLAYER_SKILL_AIR:
                            pCharacter->AddItem(
                                -1, ITEM_SPELLBOOK_AIR_FEATHER_FALL);
                            break;
                        case PLAYER_SKILL_WATER:
                            pCharacter->AddItem(
                                -1, ITEM_SPELLBOOK_WATER_POISON_SPRAY);
                            break;
                        case PLAYER_SKILL_EARTH:
                            pCharacter->AddItem(-1, ITEM_SPELLBOOK_EARTH_SLOW);
                            break;
                        case PLAYER_SKILL_SPIRIT:
                            pCharacter->AddItem(-1,
                                                ITEM_SPELLBOOK_SPIRIT_BLESS);
                            break;
                        case PLAYER_SKILL_MIND:
                            pCharacter->AddItem(-1,
                                                ITEM_SPELLBOOK_MIND_MIND_BLAST);
                            break;
                        case PLAYER_SKILL_BODY:
                            pCharacter->AddItem(-1,
                                                ITEM_SPELLBOOK_BODY_FIRST_AID);
                            break;
                        case PLAYER_SKILL_ITEM_ID:
                        case PLAYER_SKILL_REPAIR:
                        case PLAYER_SKILL_MEDITATION:
                        case PLAYER_SKILL_PERCEPTION:
                        case PLAYER_SKILL_DIPLOMACY:
                        case PLAYER_SKILL_TRAP_DISARM:
                        case PLAYER_SKILL_LEARNING:
                            pCharacter->AddItem(-1, ITEM_POTION_BOTTLE);
                            v16 = 5 * (rand() % 3 + 40);  // simple reagent
                            pCharacter->AddItem(-1, v16);
                            break;
                        case PLAYER_SKILL_DODGE:
                            pCharacter->AddItem(-1, ITEM_BOOTS_1);
                            break;
                        case PLAYER_SKILL_UNARMED:
                            pCharacter->AddItem(-1, ITEM_GAUNTLETS_1);
                            break;
                        default:
                            break;
                    }
                }
            }
            for (int i = 0; i < 138; i++) {
                if (pCharacter->pInventoryItemList[i].uItemID != 0)
                    pCharacter->pInventoryItemList[i].SetIdentified();
            }
        }

        pCharacter->sHealth = pCharacter->GetMaxHealth();
        pCharacter->sMana = pCharacter->GetMaxMana();
    }
}

//----- (004917CE) --------------------------------------------------------
void Party::Reset() {
    Zero();

    field_708 = 15;
    sEyelevel = 160;
    uNumGold = 200;
    uNumFoodRations = 7;

    alignment = PartyAlignment::PartyAlignment_Neutral;
    SetUserInterface(alignment, true);

    // game begins at 9 am
    this->playing_time = GameTime(0, 0, 9);
    this->last_regenerated = GameTime(0, 0, 9);
    this->uCurrentHour = 9;

    bTurnBasedModeOn = false;

    uActiveCharacter = 1;
    ::pPlayers.ZerothIndex() = &pPlayers[0];
    for (uint i = 0; i < 4; ++i) ::pPlayers[i + 1] = &pPlayers[i];

    pPlayers[0].Reset(PLAYER_CLASS_KNIGHT);
    pPlayers[0].uCurrentFace = 17;
    pPlayers[0].uPrevVoiceID = 17;
    pPlayers[0].uVoiceID = 17;
    pPlayers[0].SetInitialStats();

    pPlayers[0].uSex = pPlayers[0].GetSexByVoice();
    pPlayers[0].RandomizeName();
    strcpy(pPlayers[0].pName, localization->GetString(509));

    pPlayers[1].Reset(PLAYER_CLASS_THIEF);
    pPlayers[1].uCurrentFace = 3;
    pPlayers[1].uPrevVoiceID = 3;
    pPlayers[1].uVoiceID = 3;
    pPlayers[1].SetInitialStats();
    pPlayers[1].uSex = pPlayers[1].GetSexByVoice();
    pPlayers[1].RandomizeName();
    strcpy(pPlayers[1].pName, localization->GetString(506));

    pPlayers[2].Reset(PLAYER_CLASS_CLERIC);
    pPlayers[2].uCurrentFace = 14;
    pPlayers[2].uPrevVoiceID = 14;
    pPlayers[2].uVoiceID = 14;
    pPlayers[2].SetInitialStats();
    pPlayers[2].uSex = pPlayers[3].GetSexByVoice();
    pPlayers[2].RandomizeName();
    strcpy(pPlayers[2].pName, localization->GetString(508));

    pPlayers[3].Reset(PLAYER_CLASS_SORCERER);
    pPlayers[3].uCurrentFace = 10;
    pPlayers[3].uPrevVoiceID = 10;
    pPlayers[3].uVoiceID = 10;
    pPlayers[3].SetInitialStats();
    pPlayers[3].uSex = pPlayers[3].GetSexByVoice();
    pPlayers[3].RandomizeName();
    strcpy(pPlayers[3].pName, localization->GetString(507));

    for (uint i = 0; i < 4; ++i) {
        pPlayers[i].uTimeToRecovery = 0;
        for (uint j = 0; j < 20; ++j) pPlayers[i].conditions_times[j].Reset();

        for (uint j = 0; j < 24; ++j) pPlayers[i].pPlayerBuffs[j].Reset();

        pPlayers[i].expression = CHARACTER_EXPRESSION_1;
        pPlayers[i].uExpressionTimePassed = 0;
        pPlayers[i].uExpressionTimeLength = rand() % 256 + 128;
    }

    for (uint i = 0; i < 20; ++i) pPartyBuffs[i].Reset();

    current_character_screen_window =
        WINDOW_CharacterWindow_Stats;  // default character ui - stats
    uFlags = 0;
    memset(_autonote_bits, 0, sizeof(_autonote_bits));
    memset(_quest_bits, 0, sizeof(_quest_bits));
    pIsArtifactFound.fill(0);

    PartyTimes._shop_ban_times.fill(GameTime(0));

    memcpy(pNPCStats->pNewNPCData, pNPCStats->pNPCData, sizeof(pNPCStats->pNPCData));
    memcpy(pNPCStats->pGroups_copy, pNPCStats->pGroups, 0x66u);
    pNPCStats->pNewNPCData[3].uFlags |= 128;  //|= 0x80u; Lady Margaret
    _494035_timed_effects__water_walking_damage__etc();
    pEventTimer->Pause();

    this->pPickedItem.uItemID = 0;
}

//----- (0043AD34) --------------------------------------------------------
void Party::Yell() {
    Actor *v0;  // esi@5
    int v1;     // edi@9
    int v2;     // ebx@9
    int v3;     // eax@9

    if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
    }

    if (!pParty->bTurnBasedModeOn) {
        for (unsigned int i = 0; i < uNumActors; i++) {
            v0 = &pActors[i];
            if (v0->Actor::CanAct() &&
                v0->pMonsterInfo.uHostilityType !=
                    MonsterInfo::Hostility_Long &&
                v0->pMonsterInfo.uMovementType !=
                    MONSTER_MOVEMENT_TYPE_STAIONARY) {
                v1 = abs(v0->vPosition.x - pParty->vPosition.x);
                v2 = abs(v0->vPosition.y - pParty->vPosition.y);
                v3 = abs(v0->vPosition.z - pParty->vPosition.z);
                if (int_get_vector_length(v1, v2, v3) < 512)
                    Actor::AI_Flee(i, 4, 0, 0);
            }
        }
    }
}

//----- (00491BF9) --------------------------------------------------------
void Party::ResetPosMiscAndSpellBuffs() {
    this->vPosition.y = 0;
    this->vPosition.z = 0;
    this->vPosition.x = 0;
    this->uFallStartY = 0;
    this->sRotationZ = 0;
    this->sRotationX = 0;
    this->uFallSpeed = 0;
    this->field_28 = 0;
    this->uDefaultPartyHeight = 120;
    this->field_14_radius = 37;
    this->y_rotation_granularity = 25;
    this->uWalkSpeed = 384;
    this->y_rotation_speed = 90;
    this->field_24 = 5;
    this->_6FC_water_lava_timer = 0;
    this->field_708 = 15;
    this->field_0 = 25;

    for (int playerId = 0; playerId < 4; playerId++) {
        for (int buffId = 0; buffId < 24; buffId++) {
            this->pPlayers[playerId].pPlayerBuffs[buffId].Reset();
        }
    }
    for (int buffId = 0; buffId < 20; buffId++) {
        this->pPartyBuffs[buffId].Reset();
    }
}

//----- (004909F4) --------------------------------------------------------
void Party::UpdatePlayersAndHirelingsEmotions() {
    int v4;  // edx@27

    if (pParty->cNonHireFollowers < 0) pParty->CountHirelings();

    for (int i = 0; i < 4; ++i) {
        Player *player = &pPlayers[i];
        player->uExpressionTimePassed +=
            (unsigned short)pMiscTimer->uTimeElapsed;

        uint condition = player->GetMajorConditionIdx();
        if (condition == Condition_Good || condition == Condition_Zombie) {
            if (player->uExpressionTimePassed < player->uExpressionTimeLength)
                continue;

            player->uExpressionTimePassed = 0;
            if (player->expression != 1 || rand() % 5) {
                player->expression = CHARACTER_EXPRESSION_1;
                player->uExpressionTimeLength = rand() % 256 + 32;
            } else {
                v4 = rand() % 100;
                if (v4 < 25)
                    player->expression = CHARACTER_EXPRESSION_13;
                else if (v4 < 31)
                    player->expression = CHARACTER_EXPRESSION_14;
                else if (v4 < 37)
                    player->expression = CHARACTER_EXPRESSION_15;
                else if (v4 < 43)
                    player->expression = CHARACTER_EXPRESSION_16;
                else if (v4 < 46)
                    player->expression = CHARACTER_EXPRESSION_17;
                else if (v4 < 52)
                    player->expression = CHARACTER_EXPRESSION_18;
                else if (v4 < 58)
                    player->expression = CHARACTER_EXPRESSION_19;
                else if (v4 < 64)
                    player->expression = CHARACTER_EXPRESSION_20;
                else if (v4 < 70)
                    player->expression = CHARACTER_EXPRESSION_54;
                else if (v4 < 76)
                    player->expression = CHARACTER_EXPRESSION_55;
                else if (v4 < 82)
                    player->expression = CHARACTER_EXPRESSION_56;
                else if (v4 < 88)
                    player->expression = CHARACTER_EXPRESSION_57;
                else if (v4 < 94)
                    player->expression = CHARACTER_EXPRESSION_29;
                else
                    player->expression = CHARACTER_EXPRESSION_30;
            }

            for (unsigned int j = 0; j < pPlayerFrameTable->uNumFrames; ++j) {
                PlayerFrame *frame = &pPlayerFrameTable->pFrames[j];
                if (frame->expression == player->expression) {
                    player->uExpressionTimeLength = 8 * frame->uAnimLength;
                    break;
                }
            }
        } else if (player->expression != CHARACTER_EXPRESSION_DMGRECVD_MINOR &&
                   player->expression != CHARACTER_EXPRESSION_DMGRECVD_MODERATE &&
                   player->expression != CHARACTER_EXPRESSION_DMGRECVD_MAJOR ||
                   player->uExpressionTimePassed >= player->uExpressionTimeLength) {
            player->uExpressionTimeLength = 0;
            player->uExpressionTimePassed = 0;

            switch (condition) {
                case Condition_Dead:
                    player->expression = CHARACTER_EXPRESSION_DEAD;
                    break;
                case Condition_Pertified:
                    player->expression = CHARACTER_EXPRESSION_PERTIFIED;
                    break;
                case Condition_Eradicated:
                    player->expression = CHARACTER_EXPRESSION_ERADICATED;
                    break;
                case Condition_Cursed:
                    player->expression = CHARACTER_EXPRESSION_CURSED;
                    break;
                case Condition_Weak:
                    player->expression = CHARACTER_EXPRESSION_WEAK;
                    break;
                case Condition_Sleep:
                    player->expression = CHARACTER_EXPRESSION_SLEEP;
                    break;
                case Condition_Fear:
                    player->expression = CHARACTER_EXPRESSION_FEAR;
                    break;
                case Condition_Drunk:
                    player->expression = CHARACTER_EXPRESSION_DRUNK;
                    break;
                case Condition_Insane:
                    player->expression = CHARACTER_EXPRESSION_INSANE;
                    break;
                case Condition_Poison_Weak:
                case Condition_Poison_Medium:
                case Condition_Poison_Severe:
                    player->expression = CHARACTER_EXPRESSION_POISONED;
                    break;
                case Condition_Disease_Weak:
                case Condition_Disease_Medium:
                case Condition_Disease_Severe:
                    player->expression = CHARACTER_EXPRESSION_DISEASED;
                    break;
                case Condition_Paralyzed:
                    player->expression = CHARACTER_EXPRESSION_PARALYZED;
                    break;
                case Condition_Unconcious:
                    player->expression = CHARACTER_EXPRESSION_UNCONCIOUS;
                    break;
                default:
                    Error("Invalid condition: %u", condition);
            }
        }
    }

    for (int i = 0; i < 2; ++i) {
        NPCData *hireling = &pParty->pHirelings[i];
        if (!hireling->evt_C) continue;

        hireling->evt_B += pMiscTimer->uTimeElapsed;
        if (hireling->evt_B >= hireling->evt_C) {
            hireling->evt_A = 0;
            hireling->evt_B = 0;
            hireling->evt_C = 0;

            Assert(sizeof(NPCData) == 0x4C);
            memset(hireling, 0, sizeof(*hireling));

            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            viewparams->bRedrawGameUI = true;
        }
    }
}

//----- (00490D02) --------------------------------------------------------
void Party::RestAndHeal() {
    Player *pPlayer;         // esi@4
    bool have_vessels_soul;  // [sp+10h] [bp-8h]@10

    for (uint i = 0; i < 20; ++i) pParty->pPartyBuffs[i].Reset();

    for (int pPlayerID = 0; pPlayerID < 4; ++pPlayerID) {
        pPlayer = &pParty->pPlayers[pPlayerID];
        for (uint i = 0; i < 20; ++i) pPlayer->pPlayerBuffs[i].Reset();

        pPlayer->Zero();
        if (pPlayer->conditions_times[Condition_Dead] ||
            pPlayer->conditions_times[Condition_Pertified] ||
            pPlayer->conditions_times[Condition_Eradicated]) {
            continue;
        }

        pPlayer->conditions_times[Condition_Unconcious].Reset();
        pPlayer->conditions_times[Condition_Drunk].Reset();
        pPlayer->conditions_times[Condition_Fear].Reset();
        pPlayer->conditions_times[Condition_Sleep].Reset();
        pPlayer->conditions_times[Condition_Weak].Reset();

        pPlayer->uTimeToRecovery = 0;
        pPlayer->sHealth = pPlayer->GetMaxHealth();
        pPlayer->sMana = pPlayer->GetMaxMana();
        if (pPlayer->classType == PLAYER_CLASS_LICH) {
            have_vessels_soul = false;
            for (uint i = 0; i < 126; i++) {
                if (pPlayer->pInventoryItemList[i].uItemID ==
                        ITEM_LICH_JAR_FULL &&
                    pPlayer->pInventoryItemList[i].uHolderPlayer ==
                        pPlayerID + 1)
                    have_vessels_soul = true;
            }
            if (!have_vessels_soul) {
                pPlayer->sHealth = pPlayer->GetMaxHealth() / 2;
                pPlayer->sMana = pPlayer->GetMaxMana() / 2;
            }
        }

        if (pPlayer->conditions_times[Condition_Zombie]) {
            pPlayer->sMana = 0;
            pPlayer->sHealth /= 2;
        } else if (pPlayer->conditions_times[Condition_Poison_Severe] ||
                   pPlayer->conditions_times[Condition_Disease_Severe]) {
            pPlayer->sHealth /= 4;
            pPlayer->sMana /= 4;
        } else if (pPlayer->conditions_times[Condition_Poison_Medium] ||
                   pPlayer->conditions_times[Condition_Disease_Medium]) {
            pPlayer->sHealth /= 3;
            pPlayer->sMana /= 3;
        } else if (pPlayer->conditions_times[Condition_Poison_Weak] ||
                   pPlayer->conditions_times[Condition_Disease_Weak]) {
            pPlayer->sHealth /= 2;
            pPlayer->sMana /= 2;
        }
        if (pPlayer->conditions_times[Condition_Insane]) pPlayer->sMana = 0;
        UpdatePlayersAndHirelingsEmotions();
    }
    pParty->days_played_without_rest = 0;
}

//----- (004938D1) --------------------------------------------------------
void Rest(unsigned int uMinsToRest) {  // this is passed mins not hours
    auto rest_time = GameTime(0, uMinsToRest);

    if (uMinsToRest > 240) Actor::InitializeActors();

    pParty->GetPlayingTime() += rest_time;

    for (int i = 1; i <= 4; i++) {
        pPlayers[i]->Recover(rest_time);  // ??
    }

    _494035_timed_effects__water_walking_damage__etc();
}

//----- (004B1BDB) --------------------------------------------------------
void RestAndHeal(int minutes) {
    pParty->GetPlayingTime().AddMinutes(minutes);

    pParty->pHirelings[0].bHasUsedTheAbility = false;
    pParty->pHirelings[1].bHasUsedTheAbility = false;

    pParty->uCurrentTimeSecond = pParty->GetPlayingTime().GetSecondsFraction();
    pParty->uCurrentMinute = pParty->GetPlayingTime().GetMinutesFraction();
    pParty->uCurrentHour = pParty->GetPlayingTime().GetHoursOfDay();
    pParty->uCurrentMonthWeek = pParty->GetPlayingTime().GetWeeksOfMonth();
    pParty->uCurrentDayOfMonth = pParty->GetPlayingTime().GetDaysOfMonth();
    pParty->uCurrentMonth = pParty->GetPlayingTime().GetMonthsOfYear();
    pParty->uCurrentYear = pParty->GetPlayingTime().GetYears() + game_starting_year;
    pParty->RestAndHeal();

    for (int i = 0; i < 4; i++) {
        pParty->pPlayers[i].uTimeToRecovery = 0;
        pParty->pPlayers[i].uNumDivineInterventionCastsThisDay = 0;
        pParty->pPlayers[i].uNumArmageddonCasts = 0;
        pParty->pPlayers[i].uNumFireSpikeCasts = 0;
        pParty->pPlayers[i].field_1B3B = 0;
    }

    pParty->UpdatePlayersAndHirelingsEmotions();
}
//----- (0041F5BE) --------------------------------------------------------
void Party::Sleep8Hours() {
    if (_506F18_num_minutes_to_sleep < 6) {
        if (_506F18_num_minutes_to_sleep) {
            Rest(_506F18_num_minutes_to_sleep);
            _506F18_num_minutes_to_sleep = 0;
            OutdoorLocation::LoadActualSkyFrame();
        }
        if (_506F14_resting_stage == 2) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
        }
    } else {
        Rest(6u);
        _506F18_num_minutes_to_sleep -= 6;
        OutdoorLocation::LoadActualSkyFrame();
    }
    viewparams->bRedrawGameUI = 1;
}

bool TestPartyQuestBit(PARTY_QUEST_BITS bit) {
    return _449B57_test_bit(pParty->_quest_bits, bit);
}

//----- (0047752B) --------------------------------------------------------
int Party::GetPartyReputation() {
    DDM_DLV_Header *v0;  // ebx@1
    signed int v1;       // esi@3

    v0 = &pOutdoor->ddm;
    if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v0 = &pIndoor->dlv;
    v1 = 0;
    if (CheckHiredNPCSpeciality(Pirate)) v1 += 5;
    if (CheckHiredNPCSpeciality(Burglar)) v1 += 5;
    if (CheckHiredNPCSpeciality(Gypsy)) v1 += 5;
    if (CheckHiredNPCSpeciality(Duper)) v1 += 5;
    if (CheckHiredNPCSpeciality(FallenWizard)) v1 += 5;
    return v1 + v0->uReputation;
}

//----- (004269A2) --------------------------------------------------------
void Party::GivePartyExp(unsigned int pEXPNum) {
    signed int pActivePlayerCount;  // ecx@1
    int pLearningPercent;           // eax@13
    int playermodexp;

    if (pEXPNum > 0) {
        pActivePlayerCount = 0;
        for (uint i = 0; i < 4; ++i) {
            if (!pParty->pPlayers[i].conditions_times[Condition_Unconcious] &&
                !pParty->pPlayers[i].conditions_times[Condition_Dead] &&
                !pParty->pPlayers[i].conditions_times[Condition_Pertified] &&
                !pParty->pPlayers[i].conditions_times[Condition_Eradicated]) {
                pActivePlayerCount++;
            }
        }
        if (pActivePlayerCount) {
            pEXPNum = pEXPNum / pActivePlayerCount;
            for (uint i = 0; i < 4; ++i) {
                if (!pParty->pPlayers[i]
                         .conditions_times[Condition_Unconcious] &&
                    !pParty->pPlayers[i].conditions_times[Condition_Dead] &&
                    !pParty->pPlayers[i]
                         .conditions_times[Condition_Pertified] &&
                    !pParty->pPlayers[i]
                         .conditions_times[Condition_Eradicated]) {
                    pLearningPercent = pParty->pPlayers[i].GetLearningPercent();
                    playermodexp = pEXPNum + pEXPNum * pLearningPercent / 100;
                    pParty->pPlayers[i].uExperience += playermodexp;
                    if (pParty->pPlayers[i].uExperience > 4000000000) {
                        pParty->pPlayers[i].uExperience = 0;
                    }
                }
            }
        }
    }
}

//----- (00420C05) --------------------------------------------------------
void Party::PartyFindsGold(
    unsigned int amount,
    int _1_dont_share_with_followers___2_the_same_but_without_a_message__else_normal) {
    NPCData *v12;              // ecx@21
    unsigned int v13;          // ecx@23
    int hirelingCount;  // [sp+Ch] [bp-4h]@6

    int hirelingSalaries = 0;
    unsigned int goldToGain = amount;

    String status;
    if (_1_dont_share_with_followers___2_the_same_but_without_a_message__else_normal ==
      2) {
    } else if (
        _1_dont_share_with_followers___2_the_same_but_without_a_message__else_normal ==
        1) {
        status =
            localization->FormatString(467, amount);  // You found %lu gold!
    } else {
        unsigned char buf[1024];
        hirelingCount = 0;
        for (int i = 0; i < 2; i++) {
            if (this->pHirelings[i].pName) {
                hirelingCount++;
                buf[hirelingCount] = i;
            }
        }
        for (uint i = 0; i < pNPCStats->uNumNewNPCs; i++) {
            if (pNPCStats->pNewNPCData[i].uFlags & 0x80 &&
                (!this->pHirelings[0].pName ||
                 strcmp(pNPCStats->pNewNPCData[i].pName,
                        this->pHirelings[0].pName)) &&
                (!this->pHirelings[1].pName ||
                 strcmp(pNPCStats->pNewNPCData[i].pName,
                        this->pHirelings[1].pName))) {
                hirelingCount++;
                buf[hirelingCount] = i + 2;
            }
        }
        for (int i = 0; i < hirelingCount; i++) {
            uchar thisBufId = buf[i];
            if (thisBufId < 2)
                v12 = &this->pHirelings[thisBufId];
            else
                v12 = &pNPCStats->pNPCData[thisBufId + 499];
            v13 = v12->uProfession;
            if (v13)
                hirelingSalaries +=
                    pNPCStats->pProfessions[v13]
                        .uHirePrice;
        }
        if (CheckHiredNPCSpeciality(Factor))
            goldToGain += (signed int)(10 * goldToGain) / 100;
        if (CheckHiredNPCSpeciality(Banker))
            goldToGain += (signed int)(20 * goldToGain) / 100;
        if (CheckHiredNPCSpeciality(Pirate))
            goldToGain += (signed int)(10 * goldToGain) / 100;
        if (hirelingSalaries) {
            hirelingSalaries =
                (signed int)(goldToGain * hirelingSalaries / 100) / 100;
            if (hirelingSalaries < 1) hirelingSalaries = 1;
            status = localization->FormatString(
                466, goldToGain,
                hirelingSalaries);  // You found %lu gold (followers take %lu)!
        } else {
            status = localization->FormatString(467, amount);  // You found %lu gold!
        }
    }
    AddGold(goldToGain - hirelingSalaries);
    if (status.length() > 0) GameUI_StatusBar_OnEvent(status.c_str(), 2u);
}

void Party::PickedItem_PlaceInInventory_or_Drop() {
    // no picked item
    if (!pParty->pPickedItem.uItemID) {
        return;
    }

    auto texture = assets->GetImage_ColorKey(pParty->pPickedItem.GetIconName(), 0x7FF);

    // check if active player has room in inventory
    int InventIndex = ::pPlayers[uActiveCharacter]->AddItem(-1, pParty->pPickedItem.uItemID);
    if (uActiveCharacter && InventIndex != 0) {
        memcpy(&::pPlayers[uActiveCharacter]->pInventoryItemList[InventIndex - 1], &pParty->pPickedItem, 0x24u);
        mouse->RemoveHoldingItem();
    } else {
        // see if any other char has room
        int CharIndex = 0;
        for (CharIndex = 0; CharIndex < 4; CharIndex++) {
            int InventIndex = pParty->pPlayers[CharIndex].AddItem(-1, pParty->pPickedItem.uItemID);
            if (InventIndex) {
                // found room so give char item
                memcpy(&pParty->pPlayers[CharIndex].pInventoryItemList[InventIndex - 1], &pParty->pPickedItem, sizeof(ItemGen));
                mouse->RemoveHoldingItem();
                break;
            }
        }

        // no chars have room so drop
        if (CharIndex == 4) {
            SpriteObject object;
            object.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[pParty->pPickedItem.uItemID].uSpriteID;
            object.spell_caster_pid = OBJECT_Player;
            object.uObjectDescID = pObjectList->ObjectIDByItemID(object.uType);
            object.vPosition.y = pParty->vPosition.y;
            object.vPosition.x = pParty->vPosition.x;
            object.vPosition.z = pParty->sEyelevel + pParty->vPosition.z;
            object.uSoundID = 0;
            object.uFacing = 0;
            object.uAttributes = 8;
            object.uSpriteFrameID = 0;
            object.uSectorID = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                                  pParty->sEyelevel + pParty->vPosition.z);
            memcpy(&object.containing_item, &pParty->pPickedItem, sizeof(object.containing_item));
            object.Create(pParty->sRotationZ, 184, 200, 0);
            mouse->RemoveHoldingItem();
        }
    }

    if (texture) {
        texture->Release();
        texture = nullptr;
    }
}

//----- (0048C6F6) --------------------------------------------------------
bool Party::AddItemToParty(ItemGen *pItem) {
    unsigned int v2;  // eax@1
    char *v5;         // eax@8
    // Texture_MM7 *v7; // ebx@10
    signed int v8;  // esi@10
    Player *v9;     // edi@11
    int v10;        // eax@11
    // int v21; // [sp+24h] [bp-4h]@10

    v2 = pItem->uItemID;
    if (!pItemsTable->pItems[v2].uItemID_Rep_St) pItem->SetIdentified();

    v5 = pItemsTable->pItems[v2].pIconName;
    if (v5) {
        auto texture = assets->GetImage_ColorKey(v5, 0x7FF);
        v8 = 0;
        uint current_player = uActiveCharacter;
        for (int i = 0; i < 4; i++) {
            current_player = current_player + i;
            if (current_player > 4) current_player = current_player - 4;
            v9 = ::pPlayers[current_player];
            v10 = v9->AddItem(-1, pItem->uItemID);
            if (v10) {
                memcpy(&v9->pInventoryItemList[v10 - 1], pItem, 0x24u);
                pItem->Reset();
                pAudioPlayer->PlaySound(SOUND_gold01, 0, 0, -1, 0, 0);
                v9->PlaySound(SPEECH_60, 0);

                if (texture) {
                    texture->Release();
                }
                return true;
            }
        }
        if (texture) {
            texture->Release();
        }
    } else {
        logger->Warning("Invalid picture_name detected ::addItem()");
    }
    return false;
}

bool Party::IsPartyEvil() { return _449B57_test_bit(_quest_bits, 100); }

bool Party::IsPartyGood() { return _449B57_test_bit(_quest_bits, 99); }

//----- (0046A89E) --------------------------------------------------------
int Party::_46A89E_immolation_effect(int *affected, int affectedArrSize,
                                     int effectRange) {
    int v5;             // ebx@3
    int v17;            // [sp+Ch] [bp-10h]@3
    int v18;            // [sp+10h] [bp-Ch]@3
    int affectedCount;  // [sp+18h] [bp-4h]@1

    affectedCount = 0;
    for (size_t i = 0; i < uNumActors; ++i) {
        v5 = abs(pActors[i].vPosition.x - this->vPosition.x);
        v17 = abs(pActors[i].vPosition.y - this->vPosition.y);
        v18 = abs(pActors[i].vPosition.z - this->vPosition.z);
        if (int_get_vector_length(v5, v17, v18) <= effectRange) {
            if (pActors[i].uAIState != Dead && pActors[i].uAIState != Dying &&
                pActors[i].uAIState != Removed &&
                pActors[i].uAIState != Disabled &&
                pActors[i].uAIState != Summoned) {
                affected[affectedCount] = i;
                affectedCount++;
                if (affectedCount >= affectedArrSize) break;
            }
        }
    }
    return affectedCount;
}

//----- (00444D80) --------------------------------------------------------
int GetTravelTime() {
    signed int new_travel_time;  // esi@1

    new_travel_time = uDefaultTravelTime_ByFoot;
    if (CheckHiredNPCSpeciality(Guide)) --new_travel_time;
    if (CheckHiredNPCSpeciality(Tracker)) new_travel_time -= 2;
    if (CheckHiredNPCSpeciality(Pathfinder)) new_travel_time -= 3;
    if (CheckHiredNPCSpeciality(Explorer)) --new_travel_time;
    if (new_travel_time < 1) new_travel_time = 1;
    return new_travel_time;
}
// 6BD07C: using guessed type int uDefaultTravelTime_ByFoot;

//----- (00449B57) --------------------------------------------------------
bool _449B57_test_bit(unsigned __int8 *a1, __int16 a2) {
    return (a1[(a2 - 1) >> 3] & (0x80 >> (a2 - 1) % 8)) != 0;
}

//----- (00449B7E) --------------------------------------------------------
void _449B7E_toggle_bit(unsigned char *pArray, __int16 a2,
                        unsigned __int16 bToggle) {
    signed int v3;          // esi@1
    unsigned char set_bit;  // edx@1

    v3 = a2 - 1;
    set_bit = 0x80 >> v3 % 8;
    if (bToggle)
        pArray[v3 / 8] |= set_bit;
    else
        pArray[v3 / 8] &= ~set_bit;
}

//----- (004760D5) --------------------------------------------------------
PartyAction ActionQueue::Next() {
    if (!uNumActions) return PARTY_INVALID;

    PartyAction result = pActions[0];
    for (unsigned int i = 0; i < uNumActions - 1; ++i)
        pActions[i] = pActions[i + 1];
    --uNumActions;

    return result;
}
