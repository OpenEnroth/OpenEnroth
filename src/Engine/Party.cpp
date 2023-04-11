#include "Engine/Party.h"

#include <cstdlib>
#include <climits>
#include <algorithm>
#include <string>
#include <limits>

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
#include "GUI/UI/UIRest.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

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
int Party::CountHirelings() {  // non hired followers
    cNonHireFollowers = 0;

    for (unsigned int i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        NPCData *npc = &pNPCStats->pNewNPCData[i];
        if (npc->Hired() && npc->pName != pHirelings[0].pName && npc->pName != pHirelings[1].pName)
            ++cNonHireFollowers;
    }

    return cNonHireFollowers;
}

// inlined
//----- (mm6c::004858D0) --------------------------------------------------
void Party::Zero() {
    field_0_set25_unused = 25;
    uPartyHeight = uDefaultPartyHeight = engine->config->gameplay.PartyHeight.value();
    sEyelevel = uDefaultEyelevel = engine->config->gameplay.PartyEyeLevel.value();
    radius = 37;
    _yawGranularity = 25;
    uWalkSpeed = engine->config->gameplay.PartyWalkSpeed.value();
    _yawRotationSpeed = 90;
    jump_strength = 5;
    field_28_set0_unused = 0;
    playing_time = GameTime(0, 0, 0);
    last_regenerated = GameTime(0, 0, 0);
    PartyTimes.bountyHunting_next_generation_time.fill(GameTime(0));
    PartyTimes.CounterEventValues.fill(GameTime(0));
    PartyTimes.HistoryEventTimes.fill(GameTime(0));
    PartyTimes.Shops_next_generation_time.fill(GameTime(0));
    PartyTimes._shop_ban_times.fill(GameTime(0));
    PartyTimes._s_times.fill(GameTime(0));
    vPosition.x = vPrevPosition.x = 0;
    vPosition.y = vPrevPosition.y = 0;
    vPosition.z = vPrevPosition.z = 0;
    _viewYaw = _viewPrevYaw = 0;
    _viewPitch = _viewPrevPitch = 0;
    sPrevEyelevel = 0;
    field_6E0_set0_unused = 0;
    field_6E4_set0_unused = 0;
    uFallSpeed = 0;
    field_6EC_set0_unused = 0;
    sPartySavedFlightZ = 0;
    floor_face_pid = 0;
    walk_sound_timer = 0;
    _6FC_water_lava_timer = 0;
    uFallStartZ = 0;
    bFlying = 0;
    field_708_set15_unused = 15;
    hirelingScrollPosition = 0;
    cNonHireFollowers = 0;
    field_70B_set0_unused = 0;
    uCurrentYear = 0;
    uCurrentMonth = 0;
    uCurrentMonthWeek = 0;
    uCurrentDayOfMonth = 0;
    uCurrentHour = 0;
    uCurrentMinute = 0;
    uCurrentTimeSecond = 0;
    uNumFoodRations = 0;
    field_72C_set0_unused = 0;
    field_730_set0_unused = 0;
    uNumGold = 0;
    uNumGoldInBank = 0;
    uNumDeaths = 0;
    field_740_set0_unused = 0;
    uNumPrisonTerms = 0;
    uNumBountiesCollected = 0;
    field_74C_set0_unused = 0;
    monster_id_for_hunting.fill(0);
    monster_for_hunting_killed.fill(0);
    days_played_without_rest = 0;
    memset(_quest_bits, 0, sizeof(_quest_bits));
    pArcomageWins.fill(0);
    field_7B5_in_arena_quest = 0;
    uNumArenaWins.fill(0);
    pIsArtifactFound.fill(0);
    field_7d7_set0_unused.fill(0);
    memset(_autonote_bits, 0, sizeof(_autonote_bits));
    field_818_set0_unused.fill(0);
    random_order_num_unused.fill(0);
    uNumArcomageWins = 0;
    uNumArcomageLoses = 0;
    bTurnBasedModeOn = false;
    field_880_set0_unused = 0;
    uFlags2 = 0;
    alignment = PartyAlignment::PartyAlignment_Neutral;
    for (uint i = 0; i < 20; ++i) pPartyBuffs[i].Reset();
    pPickedItem.Reset();
    uFlags = 0;

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            StandartItemsInShops[i][j].Reset();

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            SpecialItemsInShops[i][j].Reset();

    for (unsigned int i = 0; i < 32; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            SpellBooksInGuilds[i][j].Reset();

    field_1605C_set0_unused.fill(0);
    pHireling1Name[0] = 0;
    pHireling2Name[0] = 0;
    armageddon_timer = 0;
    armageddonDamage = 0;
    pTurnBasedPlayerRecoveryTimes.fill(0);
    InTheShopFlags.fill(0);
    uFine = 0;
    flt_TorchlightColorR = 0.0f;
    flt_TorchlightColorG = 0.0f;
    flt_TorchlightColorB = 0.0f;
    TorchLightLastIntensity = 0.0f;

    _roundingDt = 0;

    // players
    for (Player &player : this->pPlayers) {
        player.Zero();
        player.sResFireBase = 0;
        player.sResAirBase = 0;
        player.sResWaterBase = 0;
        player.sResEarthBase = 0;
        player.sResPhysicalBase = 0;
        player.sResMagicBase = 0;
        player.sResSpiritBase = 0;
        player.sResMindBase = 0;
        player.sResBodyBase = 0;
        player.sResLightBase = 0;
        player.sResDarkBase = 0;

        for (int z = 0; z < player.vBeacons.size(); z++) {
            player.vBeacons[z].image->Release();
        }
        player.vBeacons.clear();
    }

    // hirelings
    pHirelings.fill(NPCData());

    playerAlreadyPicked.fill(false); // TODO(captainurist): belongs in a different place?
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
    for (Player &player : this->pPlayers) {
        if (player.CanAct()) {
            v5 = player.GetPerception();
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

void Party::setActiveToFirstCanAct() {  // added to fix some nzi problems entering shops
    for (int i = 0; i < this->pPlayers.size(); ++i) {
        if (this->pPlayers[i].CanAct()) {
            _activeCharacter = i + 1;
            return;
        }
    }

    assert(false);  // should not get here
}

//----- (0049370F) --------------------------------------------------------
void Party::switchToNextActiveCharacter() {
    // avoid switching away from char that can act
    if (hasActiveCharacter() && this->pPlayers[_activeCharacter - 1].CanAct() &&
        this->pPlayers[_activeCharacter - 1].uTimeToRecovery < 1)
        return;

    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage != TE_ATTACK || PID_TYPE(pTurnEngine->pQueue[0].uPackedID) != OBJECT_Player) {
            _activeCharacter = 0;
        } else {
            _activeCharacter = PID_ID(pTurnEngine->pQueue[0].uPackedID) + 1;
        }
        return;
    }

    if (playerAlreadyPicked[0] && playerAlreadyPicked[1] &&
        playerAlreadyPicked[2] && playerAlreadyPicked[3])
        playerAlreadyPicked.fill(false);

    for (int i = 0; i < this->pPlayers.size(); i++) {
        if (!this->pPlayers[i].CanAct() ||
            this->pPlayers[i].uTimeToRecovery > 0) {
            playerAlreadyPicked[i] = true;
        } else if (!playerAlreadyPicked[i]) {
            playerAlreadyPicked[i] = true;
            if (i > 0) { // TODO(_) check if this condition really should be here. it is
                // equal to the original source but still seems kind of weird
                _activeCharacter = i + 1;
                return;
            }
            break;
        }
    }

    int v12{};
    uint v8{};
    for (int i = 0; i < this->pPlayers.size(); i++) {
        if (this->pPlayers[i].CanAct() &&
            this->pPlayers[i].uTimeToRecovery == 0) {
            if (v12 == 0 || this->pPlayers[i].uSpeedBonus > v8) {
                v8 = this->pPlayers[i].uSpeedBonus;
                v12 = i + 1;
            }
        }
    }
    _activeCharacter = v12;
    return;
}

bool Party::hasItem(ITEM_TYPE uItemID) {
    for (Player &player : this->pPlayers) {
        for (ItemGen &item : player.pOwnItems) {
            if (item.uItemID == uItemID)
                return true;
        }
    }
    return false;
}

void ui_play_gold_anim() {
    pUIAnim_Gold->uAnimTime = 0;
    pUIAnim_Gold->uAnimLength = pUIAnim_Gold->icon->GetAnimLength();
    pAudioPlayer->playUISound(SOUND_gold01);
}

void ui_play_food_anim() {
    pUIAnim_Food->uAnimTime = 0;
    pUIAnim_Food->uAnimLength = pUIAnim_Food->icon->GetAnimLength();
    // pAudioPlayer->PlaySound(SOUND_eat, 0, 0, -1, 0, 0);
}

//----- (00492AD5) --------------------------------------------------------
void Party::SetFood(int amount) {
    if (amount > 65535)
        amount = 65535;
    else if (amount < 0)
        amount = 0;

    uNumFoodRations = amount;

    ui_play_food_anim();
}

//----- (00492B03) --------------------------------------------------------
void Party::TakeFood(int amount) {
    SetFood(GetFood() - amount);
}

//----- (00492B42) --------------------------------------------------------
void Party::GiveFood(int amount) {
    SetFood(GetFood() + amount);
}

int Party::GetFood() const {
    if (engine->config->debug.InfiniteFood.value()) {
        return 99999;
    }

    return uNumFoodRations;
}

int Party::GetGold() const {
    if (engine->config->debug.InfiniteGold.value()) {
        return 99999;
    }

    return uNumGold;
}

//----- (00492B70) --------------------------------------------------------
void Party::SetGold(int amount) {
    if (amount < 0)
        amount = 0;

    uNumGold = amount;

    ui_play_gold_anim();
}

void Party::AddGold(int amount) {
    SetGold(GetGold() + amount);
}

//----- (00492BB6) --------------------------------------------------------
void Party::TakeGold(int amount) {
    SetGold(GetGold() - amount);
}

int Party::GetBankGold() const {
    return uNumGoldInBank;
}

void Party::SetBankGold(int amount) {
    if (amount < 0)
        amount = 0;

    uNumGoldInBank = amount;
}

void Party::AddBankGold(int amount) {
    SetBankGold(GetBankGold() + amount);
}

void Party::TakeBankGold(int amount) {
    SetBankGold(GetBankGold() - amount);
}

int Party::GetFine() const {
    return uFine;
}

void Party::SetFine(int amount) {
    if (amount < 0)
        amount = 0;

    uFine = amount;
}

void Party::AddFine(int amount) {
    SetFine(GetFine() + amount);
}

void Party::TakeFine(int amount) {
    SetFine(GetFine() - amount);
}

//----- (0049135E) --------------------------------------------------------
unsigned int Party::getPartyFame() {
    uint64_t total_exp = 0;
    for (Player &player : this->pPlayers) {
        total_exp += player.uExperience;
    }
    return std::min(
        (unsigned int)(total_exp / 1000),
        UINT_MAX);  // min wasn't present, but could be incorrect without it
}

void Party::createDefaultParty(bool bDebugGiveItems) {
    signed int uNumPlayers;  // [sp+18h] [bp-28h]@1
    ItemGen Dst;             // [sp+1Ch] [bp-24h]@10

    pHireling1Name[0] = 0;
    pHireling2Name[0] = 0;
    this->hirelingScrollPosition = 0;
    pHirelings.fill(NPCData());

    this->pPlayers[0].pName = localization->GetString(LSTR_PC_NAME_ZOLTAN);
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

    this->pPlayers[1].pName = localization->GetString(LSTR_PC_NAME_RODERIC);
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

    this->pPlayers[2].pName = localization->GetString(LSTR_PC_NAME_SERENA);
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

    this->pPlayers[3].pName = localization->GetString(LSTR_PC_NAME_ALEXIS);
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

    for (Player &pCharacter : pPlayers) {
        if (pCharacter.classType == PLAYER_CLASS_KNIGHT)
            pCharacter.sResMagicBase = 10;

        pCharacter.lastOpenedSpellbookPage = 0;
        int count = 0;
        for (PLAYER_SKILL_TYPE skill : MagicSkills()) {  // for Magic Book
            if (pCharacter.pActiveSkills[skill]) {
                pCharacter.lastOpenedSpellbookPage = count;
                break;
            }

            count++;
        }

        pCharacter.uExpressionTimePassed = 0;

        if (bDebugGiveItems) {
            Dst.Reset();
            pItemTable->GenerateItem(ITEM_TREASURE_LEVEL_2, 40, &Dst);  // ring
            pCharacter.AddItem2(-1, &Dst);
            for (int uSkillIdx = 0; uSkillIdx < 36; uSkillIdx++) {
                PLAYER_SKILL_TYPE skill = (PLAYER_SKILL_TYPE)uSkillIdx;
                if (pCharacter.pActiveSkills[skill]) {
                    switch (skill) {
                        case PLAYER_SKILL_STAFF:
                            pCharacter.WearItem(ITEM_STAFF);
                            break;
                        case PLAYER_SKILL_SWORD:
                            pCharacter.WearItem(ITEM_CRUDE_LONGSWORD);
                            break;
                        case PLAYER_SKILL_DAGGER:
                            pCharacter.WearItem(ITEM_DAGGER);
                            break;
                        case PLAYER_SKILL_AXE:
                            pCharacter.WearItem(ITEM_CRUDE_AXE);
                            break;
                        case PLAYER_SKILL_SPEAR:
                            pCharacter.WearItem(ITEM_CRUDE_SPEAR);
                            break;
                        case PLAYER_SKILL_BOW:
                            pCharacter.WearItem(ITEM_CROSSBOW);
                            break;
                        case PLAYER_SKILL_MACE:
                            pCharacter.WearItem(ITEM_MACE);
                            break;
                        case PLAYER_SKILL_SHIELD:
                            pCharacter.WearItem(ITEM_WOODEN_BUCKLER);
                            break;
                        case PLAYER_SKILL_LEATHER:
                            pCharacter.WearItem(ITEM_LEATHER_ARMOR);
                            break;
                        case PLAYER_SKILL_CHAIN:
                            pCharacter.WearItem(ITEM_CHAIN_MAIL);
                            break;
                        case PLAYER_SKILL_PLATE:
                            pCharacter.WearItem(ITEM_PLATE_ARMOR);
                            break;
                        case PLAYER_SKILL_FIRE:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_FIRE_BOLT);
                            break;
                        case PLAYER_SKILL_AIR:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_FEATHER_FALL);
                            break;
                        case PLAYER_SKILL_WATER:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_POISON_SPRAY);
                            break;
                        case PLAYER_SKILL_EARTH:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_SLOW);
                            break;
                        case PLAYER_SKILL_SPIRIT:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_BLESS);
                            break;
                        case PLAYER_SKILL_MIND:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_MIND_BLAST);
                            break;
                        case PLAYER_SKILL_BODY:
                            pCharacter.AddItem(-1, ITEM_SPELLBOOK_HEAL);
                            break;
                        case PLAYER_SKILL_ITEM_ID:
                        case PLAYER_SKILL_REPAIR:
                        case PLAYER_SKILL_MEDITATION:
                        case PLAYER_SKILL_PERCEPTION:
                        case PLAYER_SKILL_DIPLOMACY:
                        case PLAYER_SKILL_TRAP_DISARM:
                        case PLAYER_SKILL_LEARNING:
                            pCharacter.AddItem(-1, ITEM_POTION_BOTTLE);
                            pCharacter.AddItem(-1, grng->randomSample(Level1Reagents())); // Add simple reagent.
                            break;
                        case PLAYER_SKILL_DODGE:
                            pCharacter.AddItem(-1, ITEM_LEATHER_BOOTS);
                            break;
                        case PLAYER_SKILL_UNARMED:
                            pCharacter.AddItem(-1, ITEM_GAUNTLETS);
                            break;
                        default:
                            break;
                    }
                }
            }
            for (int i = 0; i < Player::INVENTORY_SLOT_COUNT; i++) {
                if (pCharacter.pInventoryItemList[i].uItemID != ITEM_NULL) {
                    pCharacter.pInventoryItemList[i].SetIdentified();
                }
            }
            for (int i = 0; i < Player::ADDITIONAL_SLOT_COUNT; i++) {
                if (pCharacter.pEquippedItems[i].uItemID != ITEM_NULL) {
                    pCharacter.pEquippedItems[i].SetIdentified();
                }
            }
        }

        pCharacter.sHealth = pCharacter.GetMaxHealth();
        pCharacter.sMana = pCharacter.GetMaxMana();
    }
}

//----- (004917CE) --------------------------------------------------------
void Party::Reset() {
    Zero();

    field_708_set15_unused = 15;
    sEyelevel = engine->config->gameplay.PartyEyeLevel.value();
    uNumGold = engine->config->gameplay.NewGameGold.value();
    uNumFoodRations = engine->config->gameplay.NewGameFood.value();

    alignment = PartyAlignment::PartyAlignment_Neutral;
    SetUserInterface(alignment, true);

    // game begins at 9 am
    this->playing_time = GameTime(0, 0, 9);
    this->last_regenerated = GameTime(0, 0, 9);
    this->uCurrentHour = 9;

    bTurnBasedModeOn = false;

    pParty->_activeCharacter = 1;
    for (int i = 0; i < pPlayers.size(); ++i) {
        ::pPlayers[i + 1] = &pPlayers[i];
    }

    pPlayers[0].Reset(PLAYER_CLASS_KNIGHT);
    pPlayers[0].uCurrentFace = 17;
    pPlayers[0].uPrevVoiceID = 17;
    pPlayers[0].uVoiceID = 17;
    pPlayers[0].SetInitialStats();
    pPlayers[0].uSex = pPlayers[0].GetSexByVoice();
    pPlayers[0].pName = localization->GetString(LSTR_PC_NAME_ZOLTAN);

    pPlayers[1].Reset(PLAYER_CLASS_THIEF);
    pPlayers[1].uCurrentFace = 3;
    pPlayers[1].uPrevVoiceID = 3;
    pPlayers[1].uVoiceID = 3;
    pPlayers[1].SetInitialStats();
    pPlayers[1].uSex = pPlayers[1].GetSexByVoice();
    pPlayers[1].pName = localization->GetString(LSTR_PC_NAME_RODERIC);

    pPlayers[2].Reset(PLAYER_CLASS_CLERIC);
    pPlayers[2].uCurrentFace = 14;
    pPlayers[2].uPrevVoiceID = 14;
    pPlayers[2].uVoiceID = 14;
    pPlayers[2].SetInitialStats();
    pPlayers[2].uSex = pPlayers[3].GetSexByVoice();
    pPlayers[2].pName = localization->GetString(LSTR_PC_NAME_SERENA);

    pPlayers[3].Reset(PLAYER_CLASS_SORCERER);
    pPlayers[3].uCurrentFace = 10;
    pPlayers[3].uPrevVoiceID = 10;
    pPlayers[3].uVoiceID = 10;
    pPlayers[3].SetInitialStats();
    pPlayers[3].uSex = pPlayers[3].GetSexByVoice();
    pPlayers[3].pName = localization->GetString(LSTR_PC_NAME_ALEXIS);

    for (Player &player : this->pPlayers) {
        player.uTimeToRecovery = 0;
        player.conditions.ResetAll();

        for (SpellBuff &buff : player.pPlayerBuffs) {
            buff.Reset();
        }

        player.expression = CHARACTER_EXPRESSION_1;
        player.uExpressionTimePassed = 0;
        player.uExpressionTimeLength = vrng->random(256) + 128;
    }

    for (SpellBuff &buff : this->pPartyBuffs) {
        buff.Reset();
    }

    current_character_screen_window = WINDOW_CharacterWindow_Stats;  // default character ui - stats
    uFlags = 0;
    memset(_autonote_bits, 0, sizeof(_autonote_bits));
    memset(_quest_bits, 0, sizeof(_quest_bits));
    pIsArtifactFound.fill(0);

    PartyTimes._shop_ban_times.fill(GameTime(0));

    pNPCStats->pNewNPCData = pNPCStats->pNPCData;
    memcpy(pNPCStats->pGroups_copy, pNPCStats->pGroups, 0x66u);
    pNPCStats->pNewNPCData[3].uFlags |= 128;  //|= 0x80u; Lady Margaret
    _494035_timed_effects__water_walking_damage__etc();
    pEventTimer->Pause();

    this->pPickedItem.uItemID = ITEM_NULL;
}

void Party::yell() {
    if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
    }

    if (!pParty->bTurnBasedModeOn) {
        for (int i = 0; i < pActors.size(); i++) {
            Actor &actor = pActors[i];
            if (actor.CanAct() &&
                actor.pMonsterInfo.uHostilityType != MonsterInfo::Hostility_Long &&
                actor.pMonsterInfo.uMovementType != MONSTER_MOVEMENT_TYPE_STATIONARY) {
                if ((actor.vPosition - pParty->vPosition).length() < 512) {
                    Actor::AI_Flee(i, 4, 0, 0);
                }
            }
        }
    }
}

//----- (00491BF9) --------------------------------------------------------
void Party::ResetPosMiscAndSpellBuffs() {
    this->vPosition.y = 0;
    this->vPosition.z = 0;
    this->vPosition.x = 0;
    this->uFallStartZ = 0;
    this->_viewYaw = 0;
    this->_viewPitch = 0;
    this->uFallSpeed = 0;
    this->field_28_set0_unused = 0;
    this->uDefaultPartyHeight = engine->config->gameplay.PartyHeight.value(); // was 120?
    this->radius = 37;
    this->_yawGranularity = 25;
    this->uWalkSpeed = engine->config->gameplay.PartyWalkSpeed.value();
    this->_yawRotationSpeed = 90;
    this->jump_strength = 5;
    this->_6FC_water_lava_timer = 0;
    this->field_708_set15_unused = 15;
    this->field_0_set25_unused = 25;

    for (Player &player : this->pPlayers) {
        for (SpellBuff &buff : player.pPlayerBuffs) {
            buff.Reset();
        }
    }
    for (SpellBuff &buff : this->pPartyBuffs) {
        buff.Reset();
    }
}

//----- (004909F4) --------------------------------------------------------
void Party::UpdatePlayersAndHirelingsEmotions() {
    int v4;  // edx@27

    if (pParty->cNonHireFollowers < 0) {
        pParty->CountHirelings();
    }

    for (Player &player : this->pPlayers) {
        player.uExpressionTimePassed += (unsigned short)pMiscTimer->uTimeElapsed;

        Condition condition = player.GetMajorConditionIdx();
        if (condition == Condition_Good || condition == Condition_Zombie) {
            if (player.uExpressionTimePassed < player.uExpressionTimeLength)
                continue;

            player.uExpressionTimePassed = 0;
            if (player.expression != 1 || vrng->random(5)) {
                player.expression = CHARACTER_EXPRESSION_1;
                player.uExpressionTimeLength = vrng->random(256) + 32;
            } else {
                v4 = vrng->random(100);
                if (v4 < 25)
                    player.expression = CHARACTER_EXPRESSION_BLINK;
                else if (v4 < 31)
                    player.expression = CHARACTER_EXPRESSION_WINK;
                else if (v4 < 37)
                    player.expression = CHARACTER_EXPRESSION_MOUTH_OPEN_RANDOM;
                else if (v4 < 43)
                    player.expression = CHARACTER_EXPRESSION_PURSE_LIPS_RANDOM;
                else if (v4 < 46)
                    player.expression = CHARACTER_EXPRESSION_LOOK_UP;
                else if (v4 < 52)
                    player.expression = CHARACTER_EXPRESSION_LOOK_RIGHT;
                else if (v4 < 58)
                    player.expression = CHARACTER_EXPRESSION_LOOK_LEFT;
                else if (v4 < 64)
                    player.expression = CHARACTER_EXPRESSION_LOOK_DOWN;
                else if (v4 < 70)
                    player.expression = CHARACTER_EXPRESSION_54;
                else if (v4 < 76)
                    player.expression = CHARACTER_EXPRESSION_55;
                else if (v4 < 82)
                    player.expression = CHARACTER_EXPRESSION_56;
                else if (v4 < 88)
                    player.expression = CHARACTER_EXPRESSION_57;
                else if (v4 < 94)
                    player.expression = CHARACTER_EXPRESSION_PURSE_LIPS_1;
                else
                    player.expression = CHARACTER_EXPRESSION_PURSE_LIPS_2;
            }

            for (unsigned int j = 0; j < pPlayerFrameTable->uNumFrames; ++j) {
                PlayerFrame *frame = &pPlayerFrameTable->pFrames[j];
                if (frame->expression == player.expression) {
                    player.uExpressionTimeLength = 8 * frame->uAnimLength;
                    break;
                }
            }
        } else if (player.expression != CHARACTER_EXPRESSION_DMGRECVD_MINOR &&
                   player.expression != CHARACTER_EXPRESSION_DMGRECVD_MODERATE &&
                   player.expression != CHARACTER_EXPRESSION_DMGRECVD_MAJOR ||
                   player.uExpressionTimePassed >= player.uExpressionTimeLength) {
            player.uExpressionTimeLength = 0;
            player.uExpressionTimePassed = 0;

            switch (condition) {
                case Condition_Dead:
                    player.expression = CHARACTER_EXPRESSION_DEAD;
                    break;
                case Condition_Petrified:
                    player.expression = CHARACTER_EXPRESSION_PERTIFIED;
                    break;
                case Condition_Eradicated:
                    player.expression = CHARACTER_EXPRESSION_ERADICATED;
                    break;
                case Condition_Cursed:
                    player.expression = CHARACTER_EXPRESSION_CURSED;
                    break;
                case Condition_Weak:
                    player.expression = CHARACTER_EXPRESSION_WEAK;
                    break;
                case Condition_Sleep:
                    player.expression = CHARACTER_EXPRESSION_SLEEP;
                    break;
                case Condition_Fear:
                    player.expression = CHARACTER_EXPRESSION_FEAR;
                    break;
                case Condition_Drunk:
                    player.expression = CHARACTER_EXPRESSION_DRUNK;
                    break;
                case Condition_Insane:
                    player.expression = CHARACTER_EXPRESSION_INSANE;
                    break;
                case Condition_Poison_Weak:
                case Condition_Poison_Medium:
                case Condition_Poison_Severe:
                    player.expression = CHARACTER_EXPRESSION_POISONED;
                    break;
                case Condition_Disease_Weak:
                case Condition_Disease_Medium:
                case Condition_Disease_Severe:
                    player.expression = CHARACTER_EXPRESSION_DISEASED;
                    break;
                case Condition_Paralyzed:
                    player.expression = CHARACTER_EXPRESSION_PARALYZED;
                    break;
                case Condition_Unconscious:
                    player.expression = CHARACTER_EXPRESSION_UNCONCIOUS;
                    break;
                default:
                    Error("Invalid condition: %u", condition);
            }
        }
    }

    for (int i = 0; i < 2; ++i) {
        NPCData *hireling = &pParty->pHirelings[i];
        if (!hireling->dialogue_3_evt_id) continue;

        hireling->dialogue_2_evt_id += pMiscTimer->uTimeElapsed;
        if (hireling->dialogue_2_evt_id >= hireling->dialogue_3_evt_id) {
            hireling->dialogue_1_evt_id = 0;
            hireling->dialogue_2_evt_id = 0;
            hireling->dialogue_3_evt_id = 0;

            *hireling = NPCData();

            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
        }
    }
}

//----- (00490D02) --------------------------------------------------------
void Party::RestAndHeal() {
    Player *pPlayer;         // esi@4
    bool have_vessels_soul;  // [sp+10h] [bp-8h]@10

    for (SpellBuff &buff : pParty->pPartyBuffs) {
        buff.Reset();
    }

    for (int pPlayerID = 0; pPlayerID < this->pPlayers.size(); ++pPlayerID) {
        pPlayer = &pParty->pPlayers[pPlayerID];
        for (uint i = 0; i < 20; ++i) pPlayer->pPlayerBuffs[i].Reset();

        pPlayer->Zero();
        if (pPlayer->conditions.HasAny({Condition_Dead, Condition_Petrified, Condition_Eradicated})) {
            continue;
        }

        pPlayer->conditions.Reset(Condition_Unconscious);
        pPlayer->conditions.Reset(Condition_Drunk);
        pPlayer->conditions.Reset(Condition_Fear);
        pPlayer->conditions.Reset(Condition_Sleep);
        pPlayer->conditions.Reset(Condition_Weak);

        pPlayer->uTimeToRecovery = 0;
        pPlayer->sHealth = pPlayer->GetMaxHealth();
        pPlayer->sMana = pPlayer->GetMaxMana();
        if (pPlayer->classType == PLAYER_CLASS_LICH) {
            have_vessels_soul = false;
            for (uint i = 0; i < Player::INVENTORY_SLOT_COUNT; i++) {
                if (pPlayer->pInventoryItemList[i].uItemID == ITEM_QUEST_LICH_JAR_FULL && pPlayer->pInventoryItemList[i].uHolderPlayer == pPlayerID)
                    have_vessels_soul = true;
            }
            if (!have_vessels_soul) {
                pPlayer->sHealth = pPlayer->GetMaxHealth() / 2;
                pPlayer->sMana = pPlayer->GetMaxMana() / 2;
            }
        }

        if (pPlayer->conditions.Has(Condition_Zombie)) {
            pPlayer->sMana = 0;
            pPlayer->sHealth /= 2;
        } else if (pPlayer->conditions.HasAny({Condition_Poison_Severe, Condition_Disease_Severe})) {
            pPlayer->sHealth /= 4;
            pPlayer->sMana /= 4;
        } else if (pPlayer->conditions.HasAny({Condition_Poison_Medium, Condition_Disease_Medium})) {
            pPlayer->sHealth /= 3;
            pPlayer->sMana /= 3;
        } else if (pPlayer->conditions.HasAny({Condition_Poison_Weak, Condition_Disease_Weak})) {
            pPlayer->sHealth /= 2;
            pPlayer->sMana /= 2;
        }
        if (pPlayer->conditions.Has(Condition_Insane))
            pPlayer->sMana = 0;
        UpdatePlayersAndHirelingsEmotions();
    }
    pParty->days_played_without_rest = 0;
}

void Rest(GameTime restTime) {
    if (restTime > GameTime::FromHours(4)) {
        Actor::InitializeActors();
    }

    pParty->GetPlayingTime() += restTime;

    for (Player &player : pParty->pPlayers) {
        player.Recover(restTime);  // ??
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

    for (Player &player : pParty->pPlayers) {
        player.uTimeToRecovery = 0;
        player.uNumDivineInterventionCastsThisDay = 0;
        player.uNumArmageddonCasts = 0;
        player.uNumFireSpikeCasts = 0;
        player.field_1B3B_set0_unused = 0;
    }

    pParty->UpdatePlayersAndHirelingsEmotions();
}
void Party::restOneFrame() {
    // Before each frame party rested for 6 minutes but that caused
    // resting to be too fast on high FPS
    // Now resting speed is roughly 6 game hours per second
    GameTime restTick = GameTime::FromMinutes(3 * pEventTimer->uTimeElapsed);

    if (remainingRestTime < restTick) {
        restTick = remainingRestTime;
    }

    if (restTick.Valid()) {
        Rest(restTick);
        remainingRestTime -= restTick;
        OutdoorLocation::LoadActualSkyFrame();
    }

    if (!remainingRestTime.Valid()) {
        if (currentRestType == REST_HEAL) {
            // Close rest screen when healing is done.
            // Resting type is reset on Escape processing
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
        } else if (currentRestType == REST_WAIT) {
            // Reset resting type after waiting is done.
            currentRestType = REST_NONE;
        }
    }
}

bool TestPartyQuestBit(PARTY_QUEST_BITS bit) {
    return _449B57_test_bit(pParty->_quest_bits, bit);
}

//----- (0047752B) --------------------------------------------------------
int Party::GetPartyReputation() {
    DDM_DLV_Header *ddm_dlv = &pOutdoor->ddm;
    if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) ddm_dlv = &pIndoor->dlv;

    int npcRep = 0;
    if (CheckHiredNPCSpeciality(Pirate)) npcRep += 5;
    if (CheckHiredNPCSpeciality(Burglar)) npcRep += 5;
    if (CheckHiredNPCSpeciality(Gypsy)) npcRep += 5;
    if (CheckHiredNPCSpeciality(Duper)) npcRep += 5;
    if (CheckHiredNPCSpeciality(FallenWizard)) npcRep += 5;
    return npcRep + ddm_dlv->uReputation;
}

//----- (004269A2) --------------------------------------------------------
void Party::GivePartyExp(unsigned int pEXPNum) {
    signed int pActivePlayerCount;  // ecx@1
    int pLearningPercent;           // eax@13
    int playermodexp;

    if (pEXPNum > 0) {
        pActivePlayerCount = 0;
        for (Player &player : this->pPlayers) {
            if (player.conditions.HasNone({Condition_Unconscious, Condition_Dead, Condition_Petrified, Condition_Eradicated})) {
                pActivePlayerCount++;
            }
        }
        if (pActivePlayerCount) {
            pEXPNum = pEXPNum / pActivePlayerCount;
            for (Player &player : this->pPlayers) {
                if (player.conditions.HasNone({Condition_Unconscious, Condition_Dead, Condition_Petrified, Condition_Eradicated})) {
                    pLearningPercent = player.getLearningPercent();
                    playermodexp = pEXPNum + pEXPNum * pLearningPercent / 100;
                    player.uExperience += playermodexp;
                    if (player.uExperience > 4000000000) {
                        player.uExperience = 0;
                    }
                }
            }
        }
    }
}

void Party::partyFindsGold(int amount, GoldReceivePolicy policy) {
    int hirelingSalaries = 0;
    unsigned int goldToGain = amount;

    std::string status;
    if (policy == GOLD_RECEIVE_NOSHARE_SILENT) {
    } else if (policy == GOLD_RECEIVE_NOSHARE_MSG) {
        status = localization->FormatString(LSTR_FMT_YOU_FOUND_D_GOLD, amount);
    } else {
        FlatHirelings buf;
        buf.Prepare();

        for (int i = 0; i < buf.Size(); i++) {
            NPCProf prof = buf.Get(i)->profession;
            if (prof != NoProfession) {
                hirelingSalaries += pNPCStats->pProfessions[prof].uHirePrice;
            }
        }
        if (CheckHiredNPCSpeciality(Factor)) {
            goldToGain += (signed int)(10 * goldToGain) / 100;
        }
        if (CheckHiredNPCSpeciality(Banker)) {
            goldToGain += (signed int)(20 * goldToGain) / 100;
        }
        if (CheckHiredNPCSpeciality(Pirate)) {
            goldToGain += (signed int)(10 * goldToGain) / 100;
        }
        if (hirelingSalaries) {
            hirelingSalaries = (signed int)(goldToGain * hirelingSalaries / 100) / 100;
            if (hirelingSalaries < 1) {
                hirelingSalaries = 1;
            }
            status = localization->FormatString(LSTR_FMT_YOU_FOUND_D_GOLD_FOLLOWERS, goldToGain, hirelingSalaries);
        } else {
            status = localization->FormatString(LSTR_FMT_YOU_FOUND_D_GOLD, amount);
        }
    }
    AddGold(goldToGain - hirelingSalaries);
    if (status.length() > 0) {
        GameUI_SetStatusBar(status);
    }
}

void Party::PickedItem_PlaceInInventory_or_Drop() {
    // no picked item
    if (pParty->pPickedItem.uItemID == ITEM_NULL)
        return;

    auto texture = assets->GetImage_ColorKey(pParty->pPickedItem.GetIconName());

    // check if active player has room in inventory
    int inventIndex = ::pPlayers[pParty->_activeCharacter]->AddItem(-1, pParty->pPickedItem.uItemID);
    if (pParty->_activeCharacter && inventIndex != 0) {
        ::pPlayers[pParty->_activeCharacter]->pInventoryItemList[inventIndex - 1] = pParty->pPickedItem;
        mouse->RemoveHoldingItem();
    } else {
        // see if any other char has room
        bool dropItem = true;
        for (Player &player : pParty->pPlayers) {
            inventIndex = player.AddItem(-1, pParty->pPickedItem.uItemID);
            if (inventIndex) {
                // found room so give char item
                player.pInventoryItemList[inventIndex - 1] = pParty->pPickedItem;
                mouse->RemoveHoldingItem();
                dropItem = false;
                break;
            }
        }

        // no chars have room so drop
        if (dropItem) {
            SpriteObject object;
            object.uType = (SPRITE_OBJECT_TYPE)pItemTable->pItems[pParty->pPickedItem.uItemID].uSpriteID;
            object.spell_caster_pid = PID(OBJECT_Player, 0);
            object.uObjectDescID = pObjectList->ObjectIDByItemID(object.uType);
            object.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->sEyelevel);
            object.uSoundID = 0;
            object.uFacing = 0;
            object.uAttributes = SPRITE_DROPPED_BY_PLAYER;
            object.uSpriteFrameID = 0;
            object.uSectorID = pIndoor->GetSector(object.vPosition);
            object.containing_item = pParty->pPickedItem;
            object.Create(pParty->_viewYaw, 184, 200, 0);
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
    int v10;        // eax@11

    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->_activeCharacter = 0 and crash

    if (!pItemTable->pItems[pItem->uItemID].uItemID_Rep_St) {
        pItem->SetIdentified();
    }

    char *iconName = pItemTable->pItems[pItem->uItemID].pIconName;
    if (iconName) {
        auto texture = assets->GetImage_ColorKey(iconName);
        int current_player = pParty->_activeCharacter - 1;
        for (int i = 0; i < pPlayers.size(); i++, current_player++) {
            if (current_player >= pPlayers.size()) {
                current_player = 0;
            }
            int itemIndex = pPlayers[current_player].AddItem(-1, pItem->uItemID);
            if (itemIndex) {
                pPlayers[current_player].pInventoryItemList[itemIndex - 1] = *pItem;
                pItem->Reset();
                pAudioPlayer->playUISound(SOUND_gold01);
                pPlayers[current_player].playReaction(SPEECH_FoundItem);

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
        logger->warning("Invalid picture_name detected ::addItem()");
    }
    return false;
}

bool Party::isPartyEvil() { return _449B57_test_bit(_quest_bits, QBIT_DARK_PATH); }

bool Party::isPartyGood() { return _449B57_test_bit(_quest_bits, QBIT_LIGHT_PATH); }

size_t Party::immolationAffectedActors(int *affected, size_t affectedArrSize, size_t effectRange) {
    int x, y, z;
    int affectedCount = 0;

    for (size_t i = 0; i < pActors.size(); ++i) {
        x = abs(pActors[i].vPosition.x - this->vPosition.x);
        y = abs(pActors[i].vPosition.y - this->vPosition.y);
        z = abs(pActors[i].vPosition.z - this->vPosition.z);
        if (int_get_vector_length(x, y, z) <= effectRange) {
            if (pActors[i].uAIState != Dead && pActors[i].uAIState != Dying &&
                pActors[i].uAIState != Removed &&
                pActors[i].uAIState != Disabled &&
                pActors[i].uAIState != Summoned) {
                affected[affectedCount] = i;

                affectedCount++;
                if (affectedCount >= affectedArrSize)
                    break;
            }
        }
    }

    return affectedCount;
}

int Party::getSharedSkillStrongestEffect(PLAYER_SKILL_TYPE skillType, std::any param) {
    int result = 0;
    for (Player &player : pParty->pPlayers) {
        if (!player.CanAct()) continue;
        switch (skillType) {
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_MERCHANT:
                result = std::max(result, player.GetMerchant());
                break;
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_ITEM_ID:
                result = result ? true :
                         player.CanIdentify(std::any_cast<ItemGen *>(param));
                break;
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_REPAIR:
                result = result ? true :
                         player.CanRepair(std::any_cast<ItemGen *>(param));
                break;
        default:
                Error("Unknown skill type %d", skillType);
        }
    }
    return result;
}

int Party::getOptionallySharedSkillStrongestEffect(PLAYER_SKILL_TYPE skillType,
                                                   int playerIndex,
                                                   std::any param) {
    if (engine->config->gameplay.SharedMiscSkills.value()) {
        return getSharedSkillStrongestEffect(skillType, param);
    } else {
        if (playerIndex == -1) {
            assert(hasActiveCharacter());
            playerIndex = getActiveCharacter();
        }
        Player &player = pParty->pPlayers[playerIndex];
        switch (skillType) {
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_MERCHANT:
                return player.GetMerchant();
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_ITEM_ID:
                return player.CanIdentify(std::any_cast<ItemGen *>(param));
        case PLAYER_SKILL_TYPE::PLAYER_SKILL_REPAIR:
                return player.CanRepair(std::any_cast<ItemGen *>(param));
        default:
                Error("Unknown skill type %d", skillType);
        }
    }
}
int Party::getItemTreatmentStrongestEffect(ShopItemTreatment itemTreatment,
                                           std::any param1,
                                           std::any param2 /*= std::any()*/) {
    int result = (itemTreatment == ShopItemTreatment::ITEM_TREATMENT_SELL
                      ? 0
                      : std::numeric_limits<int>::max());
    for (Player &player : pPlayers) {
        if (!player.CanAct()) continue;
        switch (itemTreatment) {
        case ShopItemTreatment::ITEM_TREATMENT_BUY:
                result = std::min(
                    result,
                    player.GetBuyingPrice(std::any_cast<unsigned int>(param1),
                                          std::any_cast<float>(param2)));
                break;
        case ShopItemTreatment::ITEM_TREATMENT_SELL:
                result = std::max(
                    result,
                    player.GetPriceSell(std::any_cast<ItemGen>(param1),
                                          std::any_cast<float>(param2)));
                break;
        case ShopItemTreatment::ITEM_TREATMENT_IDENTIFY:
                result = std::min(result, player.GetPriceIdentification(
                                              std::any_cast<float>(param1)));
                break;
        case ShopItemTreatment::ITEM_TREATMENT_REPAIR:
                result = std::min(
                    result, player.GetPriceRepair(std::any_cast<int>(param1),
                                                  std::any_cast<float>(param2)));
                break;
        default:
                Error("Unknown item treatment type %d", static_cast<int>(itemTreatment));
        }
    }
    return result;
}

int Party::getItemTreatmentOptionallyStrongestEffect(
    ShopItemTreatment itemTreatment, std::any param1,
    std::any param2 /*= std::any()*/, int playerIndex /*= -1*/) {
    if (engine->config->gameplay.SharedMiscSkills.value()) {
        return getItemTreatmentStrongestEffect(itemTreatment, param1, param2);
    } else {
        if (playerIndex == -1) {
            assert(hasActiveCharacter());
            playerIndex = getActiveCharacter();
        }
        Player &player = pPlayers[playerIndex];
        switch (itemTreatment) {
            case ShopItemTreatment::ITEM_TREATMENT_BUY:
                return player.GetBuyingPrice(
                    std::any_cast<unsigned int>(param1),
                    std::any_cast<float>(param2));
            case ShopItemTreatment::ITEM_TREATMENT_SELL:
                return player.GetPriceSell(std::any_cast<ItemGen>(param1),
                                           std::any_cast<float>(param2));
            case ShopItemTreatment::ITEM_TREATMENT_IDENTIFY:
                return player.GetPriceIdentification(
                    std::any_cast<float>(param1));
            case ShopItemTreatment::ITEM_TREATMENT_REPAIR:
                return player.GetPriceRepair(std::any_cast<int>(param1),
                                             std::any_cast<float>(param2));
            default:
                Error("Unknown item treatment type %d",
                      static_cast<int>(itemTreatment));
        }
    }
}
int getTravelTime() {
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
bool _449B57_test_bit(uint8_t *a1, int16_t a2) {
    return (a1[(a2 - 1) >> 3] & (0x80 >> (a2 - 1) % 8)) != 0;
}

//----- (00449B7E) --------------------------------------------------------
void _449B7E_toggle_bit(unsigned char *pArray, int16_t a2,
                        uint16_t bToggle) {
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

void Party::giveFallDamage(int distance) {
    for (Player &player : pParty->pPlayers) {  // receive falling damage
        if (!player.HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
            !player.WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS)) {
            player.ReceiveDamage((int)((distance) *
                    (uint64_t)(player.GetMaxHealth() / 10)) / 256, DMGT_PHISYCAL);
            int bonus = 20 - player.GetParameterBonus(player.GetActualEndurance());
            player.SetRecoveryTime(bonus * debug_non_combat_recovery_mul * flt_debugrecmod3);
        }
    }
}
