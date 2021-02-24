#include "Engine/Objects/Player.h"

#include <algorithm>
#include <functional>

#include "Engine/Autonotes.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Events2D.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/stru123.h"
#include "Engine/stru298.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

using EngineIoc = Engine_::IocContainer;

static DecalBuilder *decal_builder = EngineIoc::ResolveDecalBuilder();
static SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

NZIArray<struct Player*, 5> pPlayers;

enum PlayerSpeech PlayerSpeechID;

// Race Stat Points Bonus/ Penalty
struct PlayerCreation_AttributeProps {
    unsigned char uBaseValue;
    unsigned char uMaxValue;
    unsigned char uDroppedStep;
    unsigned char uBaseStep;
};

PlayerCreation_AttributeProps
    StatTable[4][7] =  // [human , elf, goblin, dwarf] [might, int, per , end,
                       // acc, speed, luck]
    {{
         {11, 25, 1, 1},
         {11, 25, 1, 1},
         {11, 25, 1, 1},
         {9, 25, 1, 1},
         {11, 25, 1, 1},
         {11, 25, 1, 1},
         {9, 25, 1, 1},
     },
     {
         {7, 15, 2, 1},
         {14, 30, 1, 2},
         {11, 25, 1, 1},
         {7, 15, 2, 1},
         {14, 30, 1, 2},
         {11, 25, 1, 1},
         {9, 20, 1, 1},
     },
     {
         {14, 30, 1, 2},
         {7, 15, 2, 1},
         {7, 15, 2, 1},
         {11, 25, 1, 1},
         {11, 25, 1, 1},
         {14, 30, 1, 2},
         {9, 20, 1, 1},
     },
     {{14, 30, 1, 2},
      {11, 25, 1, 1},
      {11, 25, 1, 1},
      {14, 30, 1, 2},
      {7, 15, 2, 1},
      {7, 15, 2, 1},
      {9, 20, 1, 1}}};

std::array<int, 5> StealingMasteryBonuses = {
    0, 100, 200, 300,
    500};  // dword_4EDEA0        //the zeroth element isn't accessed, it just
           // helps avoid -1 indexing, originally 4 element array off by one
std::array<int, 5> StealingRandomBonuses = {-200, -100, 0, 100,
                                            200};  // dword_4EDEB4
std::array<int, 5> StealingEnchantmentBonusForSkill = {
    0, 2, 4, 6,
    10};  // dword_4EDEC4      //the zeroth element isn't accessed, it just
          // helps avoid -1 indexing, originally 4 element array off by one

// available skills per class ( 9 classes X 37 skills )
// 0 - not available
// 1 - available
// 2 - primary skill
unsigned char pSkillAvailabilityPerClass[9][37] = {  // byte[] @ MM7.exe::004ED820
    {0, 2, 0, 1, 1, 1, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 2, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 2, 1, 0},
    {1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 2, 2, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 0, 0, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0,
      0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 1, 1, 2, 0, 0, 0, 1, 0, 0, 1, 2, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {0, 1, 1, 2, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 2,
      0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 2, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 2, 1, 0, 1,
      0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
    {2, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 1, 1, 1, 0, 0, 0,
      0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0}  // some of these
                                                            // are started
                                                            // at 4ED94C,
                                                            // but needs to
                                                            // be here
};

unsigned char pEquipTypeToBodyAnchor[21] = {  // 4E8398
    1,   // EQUIP_SINGLE_HANDED
    1,   // EQUIP_TWO_HANDED
    2,   // EQUIP_BOW
    3,   // EQUIP_ARMOUR
    0,   // EQUIP_SHIELD
    4,   // EQUIP_HELMET
    5,   // EQUIP_BELT
    6,   // EQUIP_CLOAK
    7,   // EQUIP_GAUNTLETS
    8,   // EQUIP_BOOTS
    10,  // EQUIP_RING
    9,   // EQUIP_AMULET
    1,   // EQUIP_WAND
    0,   // EQUIP_REAGENT
    0,   // EQUIP_POTION
    0,   // EQUIP_SPELL_SCROLL
    0,   // EQUIP_BOOK
    0,   // EQUIP_MESSAGE_SCROLL
    0,   // EQUIP_GOLD
    0,   // EQUIP_GEM
    0    // EQUIP_NONE
};

unsigned char pBaseHealthByClass[12] = {40, 35, 35, 30, 30, 30,
                                        25, 20, 20, 0,  0,  0};
unsigned char pBaseManaByClass[12] = {0, 0, 0, 5, 5, 0, 10, 10, 15, 0, 0, 0};
unsigned char pBaseHealthPerLevelByClass[36] = {
    5, 7, 9, 9, 4, 6, 8, 8, 5, 6, 8, 8, 4, 5, 6, 6, 3, 4,
    6, 6, 4, 5, 6, 6, 2, 3, 4, 4, 2, 3, 4, 4, 2, 3, 3, 3};
unsigned char pBaseManaPerLevelByClass[36] = {
    0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 2, 3, 3, 1, 2,
    3, 3, 0, 2, 3, 3, 3, 4, 5, 5, 3, 4, 5, 5, 3, 4, 6, 6};

unsigned char pConditionAttributeModifier[7][19] = {
    {100, 100, 100, 120, 50, 200, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100,
     100, 100, 100},  // Might
    {100, 100, 100, 50, 25, 10, 100, 100, 75, 60, 50, 30, 100, 100, 100, 100,
     100, 1, 100},  // Intelligence
    {100, 100, 100, 50, 25, 10, 100, 100, 75, 60, 50, 30, 100, 100, 100, 100,
     100, 1, 100},  // Willpower
    {100, 100, 100, 100, 50, 150, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100,
     100, 100, 100},  // Endurance
    {100, 100, 100, 50, 10, 100, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100,
     100, 50, 100},  // Accuracy
    {100, 100, 100, 120, 20, 120, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100,
     100, 50, 100},  // Speed
    {100, 100, 100, 100, 200, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
     100, 100, 100, 100}};  // Luck

unsigned char pAgingAttributeModifier[7][4] = {
    {100, 75, 40, 10},      // Might
    {100, 150, 100, 10},    // Intelligence
    {100, 150, 100, 10},    // Willpower
    {100, 75, 40, 10},      // Endurance
    {100, 100, 40, 10},     // Accuracy
    {100, 100, 40, 10},     // Speed
    {100, 100, 100, 100}};  // Luck

unsigned int pAgeingTable[4] = {50, 100, 150, 0xFFFF};

std::array<unsigned int, 18> pConditionImportancyTable = {
    {16, 15, 14, 17, 13, 2, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 1, 0}};

short param_to_bonus_table[29] = {
    500, 400, 350, 300, 275, 250, 225, 200, 175, 150, 125, 100, 75, 50, 40,
    35,  30,  25,  21,  19,  17,  15,  13,  11,  9,   7,   5,   3,  0};
signed int parameter_to_bonus_value[29] = {
    30, 25, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8,
    7,  6,  5,  4,  3,  2,  1,  0,  -1, -2, -3, -4, -5, -6};

unsigned short base_recovery_times_per_weapon_type[12] = {
    100,  // PLAYER_SKILL_STAFF   && Unarmed withoud skill
    90,   // PLAYER_SKILL_SWORD   && Unarmed with skill
    60,   // PLAYER_SKILL_DAGGER
    100,  // PLAYER_SKILL_AXE
    80,   // PLAYER_SKILL_SPEAR
    100,  // PLAYER_SKILL_BOW
    80,   // PLAYER_SKILL_MACE
    30,   // PLAYER_SKILL_BLASTER
    10,   // PLAYER_SKILL_SHIELD
    10,   // PLAYER_SKILL_LEATHER
    20,   // PLAYER_SKILL_CHAIN
    30    // PLAYER_SKILL_PLATE
};

//----- (00490913) --------------------------------------------------------
int PlayerCreation_GetUnspentAttributePointCount() {
    int CurrentStatValue = 50;
    int RemainingStatPoints = 50;
    int raceId;
    int StatBaseValue;
    int PenaltyMult;
    int BonusMult;

    for (int playerNum = 0; playerNum < 4; playerNum++) {
        raceId = pParty->pPlayers[playerNum].GetRace();

        for (int statNum = 0; statNum <= 6; statNum++) {
            switch (statNum) {
                case 0:
                    CurrentStatValue = pParty->pPlayers[playerNum].uMight;
                    break;
                case 1:
                    CurrentStatValue =
                        pParty->pPlayers[playerNum].uIntelligence;
                    break;
                case 2:
                    CurrentStatValue = pParty->pPlayers[playerNum].uWillpower;
                    break;
                case 3:
                    CurrentStatValue = pParty->pPlayers[playerNum].uEndurance;
                    break;
                case 4:
                    CurrentStatValue = pParty->pPlayers[playerNum].uAccuracy;
                    break;
                case 5:
                    CurrentStatValue = pParty->pPlayers[playerNum].uSpeed;
                    break;
                case 6:
                    CurrentStatValue = pParty->pPlayers[playerNum].uLuck;
                    break;
            }

            StatBaseValue = StatTable[raceId][statNum].uBaseValue;

            if (CurrentStatValue >=
                StatBaseValue) {  // bonus or penalty increase
                PenaltyMult = StatTable[raceId][statNum].uDroppedStep;
                BonusMult = StatTable[raceId][statNum].uBaseStep;
            } else {  // less than base
                PenaltyMult = StatTable[raceId][statNum].uBaseStep;
                BonusMult = StatTable[raceId][statNum].uDroppedStep;
            }

            RemainingStatPoints +=
                PenaltyMult * (StatBaseValue - CurrentStatValue) / BonusMult;
        }
    }

    return RemainingStatPoints;
}

//----- (00427730) --------------------------------------------------------
bool Player::CanCastSpell(unsigned int uRequiredMana) {
    if (engine->config->debug_all_magic) return true;
    if (sMana >= uRequiredMana) {  // enough mana
        sMana -= uRequiredMana;    // removes mana
        return true;
    }

    // not enough mana
    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
    return false;
}

//----- (004BE2DD) --------------------------------------------------------
void Player::SalesProcess(unsigned int inventory_idnx, int item_index, int _2devent_idx) {
    float shop_mult = p2DEvents[_2devent_idx - 1].fPriceMultiplier;
    int sell_price = GetPriceSell(pOwnItems[item_index], shop_mult);

    // remove item and add gold
    RemoveItemAtInventoryIndex(inventory_idnx);
    Party::AddGold(sell_price);
}

//----- (0043EEF3) --------------------------------------------------------
bool Player::NothingOrJustBlastersEquipped() {
    signed int item_idx;
    signed int item_id;

    // scan through all equipped items
    for (int i = 0; i < 16; ++i) {
        item_idx = pEquipment.pIndices[i];

        if (item_idx) {
            item_id = pOwnItems[item_idx - 1].uItemID;

            if (item_id != ITEM_BLASTER &&
                item_id != ITEM_LASER_RIFLE)  // soemthing other than blaster&
                                              // blaster rifle
                return false;
        }
    }

    return true;  // nothing or just blaster equipped
}

//----- (004B8040) --------------------------------------------------------
int Player::GetConditionDaysPassed(
    unsigned int
        condition) {  // PS - CHECK ?? is this the intedned behavior - RETURN
                      // NUMBER OF DAYS CONDITION HAS BEEN ACTIVE FOR

    if (this->conditions_times[condition].Valid() == false) return 0;

    GameTime playtime = pParty->GetPlayingTime();
    GameTime condtime = this->conditions_times[condition];
    GameTime diff = playtime - condtime;

    return diff.GetDays() + 1;
}

//----- (004B807C) --------------------------------------------------------
int Player::GetTempleHealCostModifier(float price_multi) {
    unsigned int conditionIdx = GetMajorConditionIdx();  // get worse condition
    int conditionTimeMultiplier = 1;
    int baseConditionMultiplier =
        1;  // condition good unless otherwise , base price for health and mana
    int high_mult;
    int result;

    if (conditionIdx >= 14 &&
        conditionIdx <= 16) {  // dead, petri, erad - serious
        if (conditionIdx <= 15)
            baseConditionMultiplier = 5;  // dead or petri
        else
            baseConditionMultiplier = 10;  // erad

        conditionTimeMultiplier = GetConditionDaysPassed(conditionIdx);
    } else if (conditionIdx < 14) {  // all other conditions
        for (int i = 0; i <= 13; i++) {
            high_mult = GetConditionDaysPassed(i);

            if (high_mult >
                conditionTimeMultiplier)  // get worst other condition
                conditionTimeMultiplier = high_mult;
        }
    }

    result = (int)((double)conditionTimeMultiplier *
                   (double)baseConditionMultiplier *
                   price_multi);  // calc heal price

    if (result < 1)  // min cost
        result = 1;

    if (result > 10000)  // max cost
        result = 10000;

    return result;
}

//----- (004B8102) --------------------------------------------------------
int Player::GetPriceSell(ItemGen itemx, float price_multiplier) {
    int uRealValue = itemx.GetValue();
    signed int result =
        (signed int)(((double)uRealValue / (price_multiplier + 2.0)) +
                     uRealValue * GetMerchant() / 100);

    if (result > uRealValue) result = uRealValue;

    if (itemx.IsBroken()) result = 1;

    if (result < 1) result = 1;

    return result;
}

//----- (004B8142) --------------------------------------------------------
int Player::GetBuyingPrice(unsigned int uRealValue, float price_multiplier) {
    uint price =
        (uint)(((100 - GetMerchant()) * (uRealValue * price_multiplier)) / 100);

    if (price < uRealValue)  // price should always be at least item value
        price = uRealValue;

    return price;
}

//----- (004B8179) --------------------------------------------------------
int Player::GetPriceIdentification(float price_multiplier) {
    int basecost = (int)(price_multiplier * 50.0f);
    int actcost = basecost * (100 - GetMerchant()) / 100;

    if (actcost < basecost / 3)  // minimum price
        actcost = basecost / 3;

    if (actcost > 1)
        return actcost;
    else
        return 1;
}

//----- (004B81C3) --------------------------------------------------------
int Player::GetPriceRepair(int uRealValue, float price_multiplier) {
    int basecost = (int)(uRealValue / (6.0f - price_multiplier));
    int actcost = basecost * (100 - GetMerchant()) / 100;

    if (actcost < basecost / 3)  // min price
        actcost = basecost / 3;

    if (actcost > 1)
        return actcost;
    else
        return 1;
}

//----- (004B8213) --------------------------------------------------------
int Player::GetBaseSellingPrice(int uRealValue, float price_multiplier) {
    int basecost = (int)(uRealValue / (price_multiplier + 2.0f));

    if (basecost < 1)  // min price
        basecost = 1;

    return basecost;
}

//----- (004B8233) --------------------------------------------------------
int Player::GetBaseBuyingPrice(int uRealValue, float price_multiplier) {
    int basecost = (int)(uRealValue * price_multiplier);

    if (basecost < 1)  // min price
        basecost = 1;

    return basecost;
}

//----- (004B824B) --------------------------------------------------------
int Player::GetBaseIdentifyPrice(float price_multiplier) {
    int basecost = (int)(price_multiplier * 50.0f);

    if (basecost < 1)  // min price
        basecost = 1;

    return basecost;
}

//----- (004B8265) --------------------------------------------------------
int Player::GetBaseRepairPrice(int uRealValue, float price_multiplier) {
    int basecost = (int)(uRealValue / (6.0f - price_multiplier));

    if (basecost < 1)  // min price
        basecost = 1;

    return basecost;
}

//----- (004B6FF9) --------------------------------------------------------
bool Player::IsPlayerHealableByTemple() {
    if (this->sHealth >= GetMaxHealth() && this->sMana >= GetMaxMana() &&
        GetMajorConditionIdx() == Condition_Good) {
        return false;  // fully healthy
    } else {
        if (GetMajorConditionIdx() == Condition_Zombie) {
            if ((window_SpeakInHouse->par1C == 78 ||
                 window_SpeakInHouse->par1C == 81 ||
                 window_SpeakInHouse->par1C == 82))
                return false;  // zombie cant be healed at these tmeples
            else
                return true;
        } else {
            return true;
        }
    }
}

ItemGen* Player::GetItemAtInventoryIndex(int inout_item_cell) {
    int inventory_index = this->GetItemListAtInventoryIndex(inout_item_cell);

    if (!inventory_index) {
        return nullptr;
    }

    return &this->pInventoryItemList[inventory_index - 1];
}

//----- (00421E75) --------------------------------------------------------
unsigned int Player::GetItemListAtInventoryIndex(int inout_item_cell) {
    int cell_idx = inout_item_cell;
    if (cell_idx > 125 || cell_idx < 0) return 0;

    int inventory_index = this->pInventoryMatrix[cell_idx];
    if (inventory_index < 0) {  // not pointed to main item cell so redirect
        inventory_index = this->pInventoryMatrix[-1 - inventory_index];
    }

    return inventory_index;  // returns item list position + 1
}

//----- (004160CA) --------------------------------------------------------
void Player::ItemsPotionDmgBreak(int enchant_count) {
    int avalible_items = 0;

    __int16 item_index_tabl[138];                         // table holding items
    memset(item_index_tabl, 0, sizeof(item_index_tabl));  // set to zero

    for (int i = 0; i < 138; ++i) {  // scan through and log in table
        if ((pOwnItems[i].uItemID > 0) && (pOwnItems[i].uItemID <= 134))
            item_index_tabl[avalible_items++] = i;
    }

    if (avalible_items) {  // is there anything to break
        if (enchant_count) {
            for (int i = 0; i < enchant_count; ++i) {
                int indexbreak =
                    item_index_tabl[rand() % avalible_items];  // random item

                if (!(pInventoryItemList[indexbreak].uAttributes &
                      ITEM_HARDENED))  // if its not hardened
                    pInventoryItemList[indexbreak].uAttributes |=
                        ITEM_BROKEN;  // break it
            }
        } else {
            for (int i = 0; i < avalible_items; ++i) {  // break everything
                pInventoryItemList[item_index_tabl[i]].uAttributes |=
                    ITEM_BROKEN;
            }
        }
    }
}

//----- (00492C0B) --------------------------------------------------------
bool Player::CanAct() {
    if (this->IsAsleep() || this->IsParalyzed() || this->IsUnconcious() ||
        this->IsDead() || this->IsPertified() || this->IsEradicated())

        return false;
    else
        return true;
}

//----- (00492C40) --------------------------------------------------------
bool Player::CanSteal() {
    return GetActualSkillLevel(PLAYER_SKILL_STEALING) != 0;
}

//----- (00492C4E) --------------------------------------------------------
bool Player::CanEquip_RaceAndAlignmentCheck(unsigned int uItemID) {
    switch (uItemID) {
        case ITEM_RELIC_ETHRICS_STAFF:
        case ITEM_RELIC_OLD_NICK:
        case ITEM_RELIC_TWILIGHT:
            return pParty->IsPartyEvil();
            break;

        case ITEM_RELIC_TALEDONS_HELM:
        case ITEM_RELIC_JUSTICE:
            return pParty->IsPartyGood();
            break;

        case ITEM_ARTIFACT_ELFBANE:
            return IsRaceGoblin();
            break;

        case ITEM_ARTIFACT_MINDS_EYE:
            return IsRaceHuman();
            break;

        case ITEM_ELVEN_CHAINMAIL:
            return IsRaceElf();
            break;

        case ITEM_FORGE_GAUNTLETS:
            return IsRaceDwarf();
            break;

        case ITEM_ARTIFACT_HEROS_BELT:
            return IsMale();
            break;

        case ITEM_ARTIFACT_LADYS_ESCORT:
            return IsFemale();
            break;

        case ITEM_WETSUIT:
            return NothingOrJustBlastersEquipped();
            break;

        default:
            return 1;
            break;
    }
}

//----- (00492D65) --------------------------------------------------------
void Player::SetCondition(unsigned int uConditionIdx, int blockable) {
    if (conditions_times[uConditionIdx])  // cant get the same condition twice
        return;

    if (!ConditionProcessor::IsPlayerAffected(this, uConditionIdx,
                                              blockable)) {  // block check
        return;
    }

    switch (uConditionIdx) {  // conditions noises
        case Condition_Cursed:
            PlaySound(SPEECH_30, 0);
            break;
        case Condition_Weak:
            PlaySound(SPEECH_25, 0);
            break;
        case Condition_Sleep:
            break;  // nosound
        case Condition_Fear:
            PlaySound(SPEECH_26, 0);
            break;
        case Condition_Drunk:
            PlaySound(SPEECH_31, 0);
            break;
        case Condition_Insane:
            PlaySound(SPEECH_29, 0);
            break;

        case Condition_Poison_Weak:
        case Condition_Poison_Medium:
        case Condition_Poison_Severe:
            PlaySound(SPEECH_27, 0);
            break;

        case Condition_Disease_Weak:
        case Condition_Disease_Medium:
        case Condition_Disease_Severe:
            PlaySound(SPEECH_28, 0);
            break;

        case Condition_Paralyzed:
            break;  // nosound

        case Condition_Unconcious:
            PlaySound(SPEECH_32, 0);
            if (sHealth > 0) sHealth = 0;
            break;

        case Condition_Dead:
            PlaySound(SPEECH_33, 0);
            if (sHealth > 0) sHealth = 0;
            if (sMana > 0) sMana = 0;
            break;

        case Condition_Pertified:
            PlaySound(SPEECH_34, 0);
            break;

        case Condition_Eradicated:
            PlaySound(SPEECH_35, 0);
            if (sHealth > 0) sHealth = 0;
            if (sMana > 0) sMana = 0;
            break;

        case Condition_Zombie:
            if (classType == PLAYER_CLASS_LICH || IsEradicated() ||
                IsZombie() || !IsDead())  // cant zombified
                return;

            conditions_times.fill(GameTime(0));
            sHealth = GetMaxHealth();
            sMana = 0;
            uPrevFace = uCurrentFace;
            uPrevVoiceID = uVoiceID;

            if (IsMale()) {
                uCurrentFace = 23;
                uVoiceID = 23;
            } else {
                uCurrentFace = 24;
                uVoiceID = 24;
            }

            PlaySound(SPEECH_99, 0);
            break;
    }

    int players_before = 0;
    for (int i = 1; i < 5;
         ++i) {  // count active players veofre activating condition
        if (pPlayers[i]->CanAct()) ++players_before;
    }

    conditions_times[uConditionIdx] =
        pParty->GetPlayingTime();  // set ocndition

    int remainig_player = 0;  // who is left now
    int players_after = 0;
    for (int i = 1; i < 5; ++i) {
        if (pPlayers[i]->CanAct()) {
            remainig_player = i;
            ++players_after;
        }
    }

    if ((players_before == 2) &&
        (players_after ==
         1))  // if was 2 and now down to 1 - "its just you and me now"
        pPlayers[remainig_player]->PlaySound(
            SPEECH_107, 0);  //скорее всего обнадёжывающий возглас последнего

    return;
}

//----- (00492528) --------------------------------------------------------
bool Player::CanFitItem(unsigned int uSlot, unsigned int uItemID) {
    auto img = assets->GetImage_ColorKey(pItemsTable->pItems[uItemID].pIconName,
                                         0x7FF);
    unsigned int slotWidth = GetSizeInInventorySlots(img->GetWidth());
    unsigned int slotHeight = GetSizeInInventorySlots(img->GetHeight());

    Assert(slotHeight > 0 && slotWidth > 0,
           "Items should have nonzero dimensions");
    if ((slotWidth + uSlot % INVETORYSLOTSWIDTH) <= INVETORYSLOTSWIDTH &&
        (slotHeight + uSlot / INVETORYSLOTSWIDTH) <= INVETORYSLOTSHEIGHT) {
        for (unsigned int x = 0; x < slotWidth; x++) {
            for (unsigned int y = 0; y < slotHeight; y++) {
                if (pInventoryMatrix[y * INVETORYSLOTSWIDTH + x + uSlot] != 0) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

//----- (004925E6) --------------------------------------------------------
int Player::FindFreeInventoryListSlot() {
    for (int i = 0; i < 126; i++) {
        if (pInventoryItemList[i].uItemID == ITEM_NULL) {
            return i;  // space at i
        }
    }

    return -1;  // no room
}

//----- (00492600) --------------------------------------------------------
int Player::CreateItemInInventory(unsigned int uSlot, unsigned int uItemID) {
    signed int freeSlot = FindFreeInventoryListSlot();

    if (freeSlot == -1) {  // no room
        if (uActiveCharacter)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);

        return 0;
    } else {  // place items
        PutItemArInventoryIndex(uItemID, freeSlot, uSlot);
        this->pInventoryItemList[freeSlot].uItemID = uItemID;
    }

    return freeSlot + 1;  // return slot no + 1
}

//----- (00492700) --------------------------------------------------------
int Player::HasSkill(unsigned int uSkillType) {
    if (uSkillType >= 37 || this->pActiveSkills[uSkillType]) {
        return 1;
    } else {
        GameUI_StatusBar_OnEvent(localization->FormatString(67, this->pName));  // *** does not have the skill
        return 0;
    }
}

//----- (00492745) --------------------------------------------------------
void Player::WearItem(unsigned int uItemID) {
    int item_body_anch;
    int item_indx;
    item_indx = FindFreeInventoryListSlot();

    if (item_indx != -1) {
        pInventoryItemList[item_indx].uItemID = uItemID;
        item_body_anch =
            pEquipTypeToBodyAnchor[pItemsTable->pItems[uItemID].uEquipType];
        pEquipment.pIndices[item_body_anch] = item_indx + 1;
        pInventoryItemList[item_indx].uBodyAnchor = item_body_anch + 1;
    }
}

//----- (004927A8) --------------------------------------------------------
int Player::AddItem(int index, unsigned int uItemID) {
    if (index == -1) {  // no location specified - search for space
        for (int xcoord = 0; xcoord < INVETORYSLOTSWIDTH; xcoord++) {
            for (int ycoord = 0; ycoord < INVETORYSLOTSHEIGHT; ycoord++) {
                if (CanFitItem(ycoord * INVETORYSLOTSWIDTH + xcoord,
                               uItemID)) {  // found space
                    return CreateItemInInventory(
                        ycoord * INVETORYSLOTSWIDTH + xcoord, uItemID);
                }
            }
        }

        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        return 0;  // no space cant add item
    }

    if (!CanFitItem(index, uItemID)) {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        return 0;  // cant fit item
    }

    return CreateItemInInventory(index, uItemID);  // return location
}

//----- (00492826) --------------------------------------------------------
int Player::AddItem2(int index, ItemGen* Src) {  // are both required - check
    pItemsTable->SetSpecialBonus(Src);

    if (index == -1) {  // no loaction specified
        for (int xcoord = 0; xcoord < INVETORYSLOTSWIDTH; xcoord++) {
            for (int ycoord = 0; ycoord < INVETORYSLOTSHEIGHT; ycoord++) {
                if (CanFitItem(ycoord * INVETORYSLOTSWIDTH + xcoord,
                               Src->uItemID)) {  // found space
                    return CreateItemInInventory2(
                        ycoord * INVETORYSLOTSWIDTH + xcoord, Src);
                }
            }
        }

        return 0;
    }

    if (!CanFitItem(index, Src->uItemID)) return 0;

    return CreateItemInInventory2(index, Src);
}

//----- (0049289C) --------------------------------------------------------
int Player::CreateItemInInventory2(unsigned int index,
                                   ItemGen* Src) {  // are both required - check
    signed int freeSlot = FindFreeInventoryListSlot();
    int result;

    if (freeSlot == -1) {  // no room
        result = 0;
    } else {
        PutItemArInventoryIndex(Src->uItemID, freeSlot, index);
        memcpy(&pInventoryItemList[freeSlot], Src, sizeof(ItemGen));
        result = freeSlot + 1;
    }

    return result;
}

//----- (0049298B) --------------------------------------------------------
void Player::PutItemArInventoryIndex(
    int uItemID, int itemListPos,
    int index) {  // originally accepted ItemGen* but needed only its uItemID

    auto img = assets->GetImage_ColorKey(pItemsTable->pItems[uItemID].pIconName,
                                         0x7FF);
    unsigned int slot_width = GetSizeInInventorySlots(img->GetWidth());
    unsigned int slot_height = GetSizeInInventorySlots(img->GetHeight());

    if (slot_width > 0) {
        int* pInvPos = &pInventoryMatrix[index];
        for (unsigned int i = 0; i < slot_height; i++) {
            memset32(pInvPos, -1 - index,
                     slot_width);  // TODO(_): try to come up with a better
                                   // solution. negative values are used when
                                   // drawing the inventory - nothing is drawn
            pInvPos += INVETORYSLOTSWIDTH;
        }
    }

    pInventoryMatrix[index] = itemListPos + 1;
}

//----- (00492A36) --------------------------------------------------------
void Player::RemoveItemAtInventoryIndex(unsigned int index) {
    ItemGen* item_in_slot = this->GetItemAtInventoryIndex(index);

    auto img = assets->GetImage_ColorKey(item_in_slot->GetIconName(), 0x7FF);
    unsigned int slot_width = GetSizeInInventorySlots(img->GetWidth());
    unsigned int slot_height = GetSizeInInventorySlots(img->GetHeight());

    item_in_slot->Reset();  // must get img details before reset

    int inventory_index = this->pInventoryMatrix[index];
    if (inventory_index < 0) {  // not pointed to main item cell so redirect
        index = (-1 - inventory_index);
    }

    if (slot_width > 0) {
        int* pInvPos = &pInventoryMatrix[index];
        for (unsigned int i = 0; i < slot_height; i++) {
            memset32(pInvPos, 0, slot_width);
            pInvPos += INVETORYSLOTSWIDTH;
        }
    }
}

//----- (0049107D) --------------------------------------------------------
int Player::GetBodybuilding() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_BODYBUILDING);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_BODYBUILDING, 1, 2, 3, 5);

    return multiplier * skill;
}

//----- (004910A8) --------------------------------------------------------
int Player::GetMeditation() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_MEDITATION);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_MEDITATION, 1, 2, 3, 5);

    return multiplier * skill;
}

//----- (004910D3) --------------------------------------------------------
bool Player::CanIdentify(ItemGen* pItem) {
    int skill = GetActualSkillLevel(PLAYER_SKILL_ITEM_ID);
    int skillmaster = GetActualSkillMastery(PLAYER_SKILL_ITEM_ID);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_ITEM_ID, 1, 2, 3, 5);

    if (CheckHiredNPCSpeciality(Scholar) ||
        skillmaster == 4)  // always identify
        return true;

    // check item level against skill
    bool result = (multiplier * skill) >=
                  pItemsTable->pItems[pItem->uItemID].uItemID_Rep_St;

    return result;
}

//----- (00491151) --------------------------------------------------------
bool Player::CanRepair(ItemGen* pItem) {
    int skill = GetActualSkillLevel(PLAYER_SKILL_REPAIR);
    int skillmaster = GetActualSkillMastery(PLAYER_SKILL_REPAIR);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_REPAIR, 1, 2, 3, 5);
    ITEM_EQUIP_TYPE equipType = pItem->GetItemEquipType();

    if (CheckHiredNPCSpeciality(Smith) && equipType <= 2 ||
        CheckHiredNPCSpeciality(Armorer) && equipType >= 3 && equipType <= 9 ||
        CheckHiredNPCSpeciality(Alchemist) && equipType >= 9)
        return true;  // check against hired help

    if (skillmaster == 4)  // gm repair
        return true;

    // check item level against skill
    bool result = (multiplier * skill) >=
                  pItemsTable->pItems[pItem->uItemID].uItemID_Rep_St;

    return result;
}

//----- (004911F3) --------------------------------------------------------
int Player::GetMerchant() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_MERCHANT);
    int skillmaster = GetActualSkillMastery(PLAYER_SKILL_MERCHANT);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_MERCHANT, 1, 2, 3, 5);

    if (skillmaster == 4)  // gm merchant
        return 10000;

    int rep = pParty->GetPartyReputation();
    int bonus = multiplier * skill;

    if (bonus == 0)  // no skill so trading on rep alone
        return -rep;

    return bonus - rep + 7;
}

//----- (0049125A) --------------------------------------------------------
int Player::GetPerception() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_PERCEPTION);
    int skillmaster = GetActualSkillMastery(PLAYER_SKILL_PERCEPTION);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_PERCEPTION, 1, 2, 3, 5);

    if (skillmaster == 4)  // gm percept
        return 10000;

    return multiplier * skill;
}

//----- (004912B0) --------------------------------------------------------
int Player::GetDisarmTrap() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_TRAP_DISARM);
    int skillmaster = GetActualSkillMastery(PLAYER_SKILL_TRAP_DISARM);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_TRAP_DISARM, 1, 2, 3, 5);

    if (skillmaster == 4)  // gm disarm
        return 10000;

    if (HasEnchantedItemEquipped(35))  // item has increased disarm
        multiplier++;

    return multiplier * skill;
}

//----- (00491317) --------------------------------------------------------
char Player::GetLearningPercent() {
    int skill = GetActualSkillLevel(PLAYER_SKILL_LEARNING);
    int multiplier =
        GetMultiplierForSkillLevel(PLAYER_SKILL_LEARNING, 1, 2, 3, 5);

    return multiplier * skill + 9;
}

//----- (0048C855) --------------------------------------------------------
int Player::GetBaseStrength() {
    return this->uMight + GetItemsBonus(CHARACTER_ATTRIBUTE_STRENGTH);
}

//----- (0048C86C) --------------------------------------------------------
int Player::GetBaseIntelligence() {
    return this->uIntelligence +
           GetItemsBonus(CHARACTER_ATTRIBUTE_INTELLIGENCE);
}

//----- (0048C883) --------------------------------------------------------
int Player::GetBaseWillpower() {
    return this->uWillpower + GetItemsBonus(CHARACTER_ATTRIBUTE_WILLPOWER);
}

//----- (0048C89A) --------------------------------------------------------
int Player::GetBaseEndurance() {
    return this->uEndurance + GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE);
}

//----- (0048C8B1) --------------------------------------------------------
int Player::GetBaseAccuracy() {
    return this->uAccuracy + GetItemsBonus(CHARACTER_ATTRIBUTE_ACCURACY);
}

//----- (0048C8C8) --------------------------------------------------------
int Player::GetBaseSpeed() {
    return this->uSpeed + GetItemsBonus(CHARACTER_ATTRIBUTE_SPEED);
}

//----- (0048C8DF) --------------------------------------------------------
int Player::GetBaseLuck() {
    return this->uLuck + GetItemsBonus(CHARACTER_ATTRIBUTE_LUCK);
}

//----- (0048C8F6) --------------------------------------------------------
int Player::GetBaseLevel() {
    return this->uLevel + GetItemsBonus(CHARACTER_ATTRIBUTE_LEVEL);
}

//----- (0048C90D) --------------------------------------------------------
int Player::GetActualLevel() {
    return uLevel + sLevelModifier +
           GetMagicalBonus(CHARACTER_ATTRIBUTE_LEVEL) +
           GetItemsBonus(CHARACTER_ATTRIBUTE_LEVEL);
}

//----- (0048C93C) --------------------------------------------------------
int Player::GetActualMight() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_STRENGTH, &Player::uMight,
                              &Player::uMightBonus);
}

//----- (0048C9C2) --------------------------------------------------------
int Player::GetActualIntelligence() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_INTELLIGENCE,
                              &Player::uIntelligence,
                              &Player::uIntelligenceBonus);
}

//----- (0048CA3F) --------------------------------------------------------
int Player::GetActualWillpower() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_WILLPOWER,
                              &Player::uWillpower, &Player::uWillpowerBonus);
}

//----- (0048CABC) --------------------------------------------------------
int Player::GetActualEndurance() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_ENDURANCE,
                              &Player::uEndurance, &Player::uEnduranceBonus);
}

//----- (0048CB39) --------------------------------------------------------
int Player::GetActualAccuracy() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_ACCURACY, &Player::uAccuracy,
                              &Player::uAccuracyBonus);
}

//----- (0048CBB6) --------------------------------------------------------
int Player::GetActualSpeed() {
    return GetActualAttribute(CHARACTER_ATTRIBUTE_SPEED, &Player::uSpeed,
                              &Player::uSpeedBonus);
}

//----- (0048CC33) --------------------------------------------------------
int Player::GetActualLuck() {
    signed int npc_luck_bonus = 0;

    if (CheckHiredNPCSpeciality(Fool)) npc_luck_bonus = 5;

    if (CheckHiredNPCSpeciality(ChimneySweep)) npc_luck_bonus += 20;

    if (CheckHiredNPCSpeciality(Psychic)) npc_luck_bonus += 10;

    return GetActualAttribute(CHARACTER_ATTRIBUTE_LUCK, &Player::uLuck,
                              &Player::uLuckBonus) +
           npc_luck_bonus;
}

//----- (new function) --------------------------------------------------------
int Player::GetActualAttribute(CHARACTER_ATTRIBUTE_TYPE attrId,
                               unsigned short Player::*attrValue,
                               unsigned short Player::*attrBonus) {
    uint uActualAge = this->sAgeModifier + GetBaseAge();
    uint uAgeingMultiplier = 100;

    for (uint i = 0; i < 4; ++i) {
        if (uActualAge >=
            pAgeingTable[i])  // is the player old enough to need attrib adjust
            uAgeingMultiplier = pAgingAttributeModifier[attrId][i];
        else
            break;
    }

    uchar uConditionMult = pConditionAttributeModifier
        [attrId][GetMajorConditionIdx()];  // weak from disease or poison ect
    int magicBonus = GetMagicalBonus(attrId);
    int itemBonus = GetItemsBonus(attrId);

    return uConditionMult * uAgeingMultiplier * this->*attrValue / 100 / 100 +
           magicBonus + itemBonus + this->*attrBonus;
}

//----- (0048CCF5) --------------------------------------------------------
int Player::GetActualAttack(bool onlyMainHandDmg) {
    int parbonus = GetParameterBonus(
        GetActualAccuracy());  // bonus points for steps of accuracy level
    int atkskillbonus = GetSkillBonus(
        CHARACTER_ATTRIBUTE_ATTACK);  // bonus for skill with weapon
    int weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_ATTACK,
                                  onlyMainHandDmg);  // how good is weapon

    return parbonus + atkskillbonus + weapbonus +
           GetMagicalBonus(CHARACTER_ATTRIBUTE_ATTACK) +
           this->_some_attack_bonus;
}

//----- (0048CD45) --------------------------------------------------------
int Player::GetMeleeDamageMinimal() {
    int parbonus = GetParameterBonus(GetActualMight());
    int weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MIN) + parbonus;
    int atkskillbonus =
        GetSkillBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) + weapbonus;

    int result = _melee_dmg_bonus +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) +
                 atkskillbonus;

    if (result < 1) result = 1;

    return result;
}

//----- (0048CD90) --------------------------------------------------------
int Player::GetMeleeDamageMaximal() {
    int parbonus = GetParameterBonus(GetActualMight());
    int weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MAX) + parbonus;
    int atkskillbonus =
        GetSkillBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) + weapbonus;

    int result = this->_melee_dmg_bonus +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) +
                 atkskillbonus;

    if (result < 1) result = 1;

    return result;
}

//----- (0048CDDB) --------------------------------------------------------
int Player::CalculateMeleeDamageTo(bool ignoreSkillBonus, bool ignoreOffhand,
                                   unsigned int uTargetActorID) {
    int mainWpnDmg = 0;
    int offHndWpnDmg = 0;

    if (IsUnarmed()) {  // no weapons
        mainWpnDmg = rand() % 3 + 1;
    } else {
        if (HasItemEquipped(EQUIP_TWO_HANDED)) {  // misleading - checks main
                                                  // hand has something
            ItemGen* mainHandItemGen = this->GetMainHandItem();
            int itemId = mainHandItemGen->uItemID;
            bool addOneDice = false;
            if (pItemsTable->pItems[itemId].uSkillType == PLAYER_SKILL_SPEAR &&
                !this->pEquipment
                     .uShield)  // using spear in two hands adds a dice roll
                addOneDice = true;

            mainWpnDmg = CalculateMeleeDmgToEnemyWithWeapon(
                mainHandItemGen, uTargetActorID, addOneDice);
        }

        if (!ignoreOffhand) {
            if (this->HasItemEquipped(
                    EQUIP_SINGLE_HANDED)) {  // has second hand got a weapon
                                             // that not a shield
                ItemGen* offHandItemGen =
                    (ItemGen*)&this
                        ->pInventoryItemList[this->pEquipment.uShield - 1];

                if (offHandItemGen->GetItemEquipType() != EQUIP_SHIELD) {
                    offHndWpnDmg = CalculateMeleeDmgToEnemyWithWeapon(
                        offHandItemGen, uTargetActorID, false);
                }
            }
        }
    }

    int dmgSum = mainWpnDmg + offHndWpnDmg;

    if (!ignoreSkillBonus) {
        int mightBonus = GetParameterBonus(GetActualMight());
        int mightAndSkillbonus =
            GetSkillBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) + mightBonus;
        dmgSum += this->_melee_dmg_bonus +
                  GetMagicalBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) +
                  mightAndSkillbonus;
    }

    if (dmgSum < 1) dmgSum = 1;

    return dmgSum;
}

int Player::CalculateMeleeDmgToEnemyWithWeapon(ItemGen* weapon,
                                               unsigned int uTargetActorID,
                                               bool addOneDice) {
    int itemId = weapon->uItemID;
    int diceCount = pItemsTable->pItems[itemId].uDamageDice;

    if (addOneDice) diceCount++;

    int diceSides = pItemsTable->pItems[itemId].uDamageRoll;
    int diceResult = 0;

    for (int i = 0; i < diceCount; i++) {  // roll dice
        diceResult += rand() % diceSides + 1;
    }

    int totalDmg =
        pItemsTable->pItems[itemId].uDamageMod + diceResult;  // add modifer

    if (uTargetActorID > 0) {  // if an actor has been provided
        int enchType =
            weapon->special_enchantment;  // check against enchantments

        if (MonsterStats::BelongsToSupertype(uTargetActorID,
                                             MONSTER_SUPERTYPE_UNDEAD) &&
            (enchType == 64 || itemId == ITEM_ARTIFACT_GHOULSBANE ||
             itemId == ITEM_ARTIFACT_GIBBET || itemId == ITEM_RELIC_JUSTICE)) {
            totalDmg *= 2;  // double damage vs undead
        } else if (MonsterStats::BelongsToSupertype(
                       uTargetActorID, MONSTER_SUPERTYPE_KREEGAN) &&
                   (enchType == 39 || itemId == ITEM_ARTIFACT_GIBBET)) {
            totalDmg *= 2;  // double damage vs devils
        } else if (MonsterStats::BelongsToSupertype(uTargetActorID,
                                                    MONSTER_SUPERTYPE_DRAGON) &&
                   (enchType == 40 || itemId == ITEM_ARTIFACT_GIBBET)) {
            totalDmg *= 2;  // double damage vs dragons
        } else if (MonsterStats::BelongsToSupertype(uTargetActorID,
                                                    MONSTER_SUPERTYPE_ELF) &&
                   (enchType == 63 || itemId == ITEM_RELIC_OLD_NICK)) {
            totalDmg *= 2;  // double damage vs elf
        } else if (MonsterStats::BelongsToSupertype(uTargetActorID,
                                                    MONSTER_SUPERTYPE_TITAN) &&
                   (enchType == 65)) {
            totalDmg *= 2;  // double damage vs titan
        }
    }

    if (GetActualSkillMastery(PLAYER_SKILL_DAGGER) >= 3 &&
        pItemsTable->pItems[itemId].uSkillType == 2 &&
        rand() % 100 < 10)  // master dagger
        totalDmg *= 3;      // triple damage backstab

    return totalDmg;
}

//----- (0048D0B9) --------------------------------------------------------
int Player::GetRangedAttack() {
    int result;
    int weapbonus;
    int skillbonus;

    ItemGen* mainHandItem = GetMainHandItem();

    if (mainHandItem != nullptr &&
        (mainHandItem->uItemID < ITEM_BLASTER ||
         mainHandItem->uItemID > ITEM_LASER_RIFLE)) {  // no blasters
        weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_RANGED_ATTACK) +
                    GetParameterBonus(GetActualAccuracy());
        skillbonus =
            GetSkillBonus(CHARACTER_ATTRIBUTE_RANGED_ATTACK) + weapbonus;
        result = this->_ranged_atk_bonus +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_RANGED_ATTACK) +
                 skillbonus;
    } else {
        result = GetActualAttack(true);
    }

    return result;
}

//----- (0048D124) --------------------------------------------------------
int Player::GetRangedDamageMin() {
    int weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_MIN);
    int skillbonus =
        GetSkillBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS) + weapbonus;
    int result = this->_ranged_dmg_bonus +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS) +
                 skillbonus;

    if (result < 0)  // cant be less than 0
        result = 0;

    return result;
}

//----- (0048D191) --------------------------------------------------------
int Player::GetRangedDamageMax() {
    int weapbonus = GetItemsBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_MAX);
    int skillbonus =
        GetSkillBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS) + weapbonus;
    int result = this->_ranged_dmg_bonus +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS) +
                 skillbonus;

    if (result < 0) result = 0;

    return result;
}

//----- (0048D1FE) --------------------------------------------------------
int Player::CalculateRangedDamageTo(int uMonsterInfoID) {
    if (!HasItemEquipped(EQUIP_BOW))  // no bow
        return 0;

    ItemGen* bow =
        (ItemGen*)&this->pInventoryItemList[this->pEquipment.uBow - 1];
    int itemenchant = bow->special_enchantment;

    signed int dmgperroll = pItemsTable->pItems[bow->uItemID].uDamageRoll;
    int damagefromroll = 0;
    int damage = 0;

    for (int i = 0; i < pItemsTable->pItems[bow->uItemID].uDamageDice;
         i++) {  // roll damage dice
        damagefromroll += ((rand() % dmgperroll) + 1);
    }

    damage = pItemsTable->pItems[bow->uItemID].uDamageMod +
             damagefromroll;  // total damage

    if (uMonsterInfoID) {  // check against bow enchantments
        if (itemenchant == 64 &&
            MonsterStats::BelongsToSupertype(
                uMonsterInfoID,
                MONSTER_SUPERTYPE_UNDEAD)) {  // double damage vs undead
            damage *= 2;
        } else if (itemenchant == 39 &&
                   MonsterStats::BelongsToSupertype(
                       uMonsterInfoID,
                       MONSTER_SUPERTYPE_KREEGAN)) {  // double vs devils
            damage *= 2;
        } else if (itemenchant == 40 &&
                   MonsterStats::BelongsToSupertype(
                       uMonsterInfoID,
                       MONSTER_SUPERTYPE_DRAGON)) {  // double vs dragons
            damage *= 2;
        } else if (itemenchant == 63 &&
                   MonsterStats::BelongsToSupertype(
                       uMonsterInfoID,
                       MONSTER_SUPERTYPE_ELF)) {  // double vs elf
            damage *= 2;
        }
    }

    return damage + this->GetSkillBonus(CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS);
}

//----- (0048D2EA) --------------------------------------------------------
String Player::GetMeleeDamageString() {
    int min_damage;
    int max_damage;

    ItemGen* mainHandItem = GetMainHandItem();

    if (mainHandItem != nullptr && (mainHandItem->uItemID >= ITEM_WAND_FIRE) &&
        (mainHandItem->uItemID <= ITEM_WAND_INCENERATION)) {
        return String(localization->GetString(595));  // Wand
    } else if (mainHandItem != nullptr &&
               (mainHandItem->uItemID == ITEM_BLASTER ||
                mainHandItem->uItemID == ITEM_LASER_RIFLE)) {
        min_damage =
            GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MIN, true);  // blasters
        max_damage = GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MAX, true);
    } else {
        min_damage = GetMeleeDamageMinimal();  // weapons
        max_damage = GetMeleeDamageMaximal();
    }

    if (min_damage == max_damage) {
        return StringPrintf("%d", min_damage);  // build string
    } else {
        return StringPrintf("%d - %d", min_damage, max_damage);  // build string
    }
}

//----- (0048D396) --------------------------------------------------------
String Player::GetRangedDamageString() {
    int min_damage;
    int max_damage;

    ItemGen* mainHandItem = GetMainHandItem();

    if (mainHandItem != nullptr && (mainHandItem->uItemID >= 135) &&
        (mainHandItem->uItemID <= 159)) {
        return String(localization->GetString(595));  // Wand
    } else if (mainHandItem != nullptr &&
               (mainHandItem->uItemID == ITEM_BLASTER ||
                mainHandItem->uItemID == ITEM_LASER_RIFLE)) {
        min_damage =
            GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MIN, true);  // blasters
        max_damage = GetItemsBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_MAX, true);
    } else {
        min_damage = GetRangedDamageMin();  // weaposn
        max_damage = GetRangedDamageMax();
    }

    if (max_damage > 0) {
        if (min_damage == max_damage) {
            return StringPrintf("%d", min_damage);  // build string
        } else {
            return StringPrintf("%d - %d", min_damage,
                                max_damage);  // build string
        }
    } else {
        return String("N/A");  // no bow equipped
    }
}

//----- (0048D45A) --------------------------------------------------------
bool Player::CanTrainToNextLevel() {
    int lvl = this->uLevel + 1;
    int neededExp = ((lvl * (lvl - 1)) / 2 * 1000);
    return this->uExperience >= neededExp;
}

//----- (0048D498) --------------------------------------------------------
unsigned int Player::GetExperienceDisplayColor() {
    if (CanTrainToNextLevel())
        return ui_character_bonus_text_color;
    else
        return ui_character_default_text_color;
}

//----- (0048D4B3) --------------------------------------------------------
int Player::CalculateIncommingDamage(DAMAGE_TYPE dmg_type, int dmg) {
    if (classType == PLAYER_CLASS_LICH &&
        (dmg_type == CHARACTER_ATTRIBUTE_RESIST_MIND ||
         dmg_type == CHARACTER_ATTRIBUTE_RESIST_BODY ||
         dmg_type == CHARACTER_ATTRIBUTE_RESIST_SPIRIT))  // TODO(_): determine if spirit
                                                          // resistance should be handled
                                                          // by body res. modifier
        return 0;  // liches are not affected by self magics

    int resist_value = 0;

    switch (dmg_type) {  // get resistance
        case DMGT_FIRE:
            resist_value = GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE);
            break;
        case DMGT_ELECTR:
            resist_value = GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_AIR);
            break;
        case DMGT_COLD:
            resist_value =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_WATER);
            break;
        case DMGT_EARTH:
            resist_value =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH);
            break;

        case DMGT_SPIRIT:
            resist_value =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_SPIRIT);
            break;
        case DMGT_MIND:
            resist_value = GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND);
            break;
        case DMGT_BODY:
            resist_value = GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY);
            break;
    }

    int player_luck = GetActualLuck();
    signed int res_rand_divider =
        GetParameterBonus(player_luck) + resist_value + 30;

    if (GetParameterBonus(player_luck) + resist_value > 0) {
        for (int i = 0; i < 4; i++) {
            if (rand() % res_rand_divider >= 30)
                dmg /= 2;  // damage reduction on successful check
            else
                break;
        }
    }

    ItemGen* equippedArmor = GetArmorItem();
    if ((dmg_type == DMGT_PHISYCAL) &&
        (equippedArmor != nullptr)) {      // physical damage and wearing armour
        if (!equippedArmor->IsBroken()) {  // armour isnt broken
            int armor_skill = equippedArmor->GetPlayerSkillType();

            if (armor_skill == PLAYER_SKILL_PLATE) {
                if (GetActualSkillMastery(PLAYER_SKILL_PLATE) >=
                    3)  // master and above half incoming damage
                    return dmg / 2;
            }

            if (armor_skill == PLAYER_SKILL_CHAIN) {
                if (GetActualSkillMastery(PLAYER_SKILL_CHAIN) ==
                    4)  // gm cahin reducing damage
                    return dmg * 2 / 3;
            }
        }
    }

    return dmg;
}

//----- (0048D62C) --------------------------------------------------------
ITEM_EQUIP_TYPE Player::GetEquippedItemEquipType(ITEM_EQUIP_TYPE uEquipSlot) {
    return GetNthEquippedIndexItem(uEquipSlot)->GetItemEquipType();
}

//----- (0048D651) --------------------------------------------------------
PLAYER_SKILL_TYPE Player::GetEquippedItemSkillType(ITEM_EQUIP_TYPE uEquipSlot) {
    return (PLAYER_SKILL_TYPE)GetNthEquippedIndexItem(uEquipSlot)
        ->GetPlayerSkillType();
}

//----- (0048D676) --------------------------------------------------------
bool Player::IsUnarmed() {
    return !HasItemEquipped(EQUIP_TWO_HANDED) &&
           (!HasItemEquipped(EQUIP_SINGLE_HANDED) ||
            GetOffHandItem()->GetItemEquipType() == EQUIP_SHIELD);
}

//----- (0048D6AA) --------------------------------------------------------
bool Player::HasItemEquipped(ITEM_EQUIP_TYPE uEquipIndex) {
    uint i = pEquipment.pIndices[uEquipIndex];
    if (i)
        return !pOwnItems[i - 1].IsBroken();
    else
        return false;
}

//----- (0048D6D0) --------------------------------------------------------
bool Player::HasEnchantedItemEquipped(int uEnchantment) {
    for (uint i = 0; i < 16; ++i) {  // search over equipped inventory
        if (HasItemEquipped((ITEM_EQUIP_TYPE)i) &&
            GetNthEquippedIndexItem(i)->special_enchantment == uEnchantment)
            return true;  // check item equipped and is enchanted
    }

    return false;  // no echanted items equipped
}

//----- (0048D709) --------------------------------------------------------
bool Player::WearsItem(int item_id, ITEM_EQUIP_TYPE equip_type) {
    // check aginst specific item and slot
    return (HasItemEquipped(equip_type) &&
            GetNthEquippedIndexItem(equip_type)->uItemID == item_id);
}

bool Player::WearsItemAnyWhere(int item_id) {
    for (int i = 0; i < 16; i++) {  // check over equipped inventory
        if (WearsItem(item_id, (ITEM_EQUIP_TYPE)i)) {
            return true;  // foudn item
        }
    }

    return false;  // item not equipped
}

//----- (0048D76C) --------------------------------------------------------
int Player::StealFromShop(
    ItemGen* itemToSteal, int extraStealDifficulty, int reputation,
    int extraStealFine,
    int* fineIfFailed) {  // returns an int, but is the return value is compared
                          // to zero, so might change to bool

    // fineiffailed is changed!

    if (!itemToSteal || !CanAct()) {
        return 0;  // no item or cant act - no stealing
    } else {
        unsigned int stealskill =
            this->GetActualSkillLevel(PLAYER_SKILL_STEALING);
        unsigned int stealmaster =
            this->GetActualSkillMastery(PLAYER_SKILL_STEALING);
        unsigned int itemvalue = itemToSteal->GetValue();
        int equiptype = itemToSteal->GetItemEquipType();

        if (equiptype == EQUIP_SINGLE_HANDED || equiptype == EQUIP_TWO_HANDED ||
            equiptype == EQUIP_BOW)
            itemvalue *= 3;

        int currMaxItemValue = StealingRandomBonuses[rand() % 5] +
                               stealskill * StealingMasteryBonuses[stealmaster];
        *fineIfFailed = 100 * (reputation + extraStealDifficulty) + itemvalue;

        if (extraStealFine) {
            *fineIfFailed += 500;
        }

        if (rand() % 100 >= 5) {
            if (*fineIfFailed > currMaxItemValue) {
                if (*fineIfFailed - currMaxItemValue < 500) {
                    return 1;  // fail with item
                } else {
                    return 0;  // failed no item
                }
            } else {
                return 2;  // perfect crime
            }
        } else {
            return 0;  // failed no item
        }
    }
}

//----- (0048D88B) --------------------------------------------------------
int Player::StealFromActor(
    unsigned int uActorID, int _steal_perm,
    int reputation) {  // returns not used - should luck attribute affect

    Actor* actroPtr;
    actroPtr = &pActors[uActorID];

    if (!actroPtr || !CanAct()) {  // no actor to steal from or player cant act
        return 0;
    }

    if (!actroPtr->ActorHasItem())  // if actor does not have an item
        actroPtr->SetRandomGoldIfTheresNoItem();  // add some gold

    unsigned int stealskill = this->GetActualSkillLevel(PLAYER_SKILL_STEALING);
    unsigned int stealingMastery =
        this->GetActualSkillMastery(PLAYER_SKILL_STEALING);
    int currMaxItemValue = StealingRandomBonuses[rand() % 5] +
                           stealskill * StealingMasteryBonuses[stealingMastery];
    int fineIfFailed =
        actroPtr->pMonsterInfo.uLevel + 100 * (_steal_perm + reputation);

    if (rand() % 100 < 5 || fineIfFailed > currMaxItemValue ||
        actroPtr->ActorEnemy()) {  // busted
        Actor::AggroSurroundingPeasants(uActorID, 1);
        GameUI_StatusBar_OnEvent(localization->FormatString(
            376, this->pName));  // %s was caught stealing!
        return 0;                // no item fail
    } else {
        int random = rand();

        if (random % 100 >= 70) {  // stealing gold
            if (actroPtr->ActorHasItems[3].GetItemEquipType() !=
                EQUIP_GOLD) {  // no gold to steal fail
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    377, this->pName));  // %s failed to steal anything
                return 1;
            }

            unsigned int enchBonusSum = 0;  // how much to steal
            for (int i = 0; i < stealskill; i++)
                enchBonusSum +=
                    rand() % StealingEnchantmentBonusForSkill[stealingMastery] +
                    1;  // add random stealing bonuses

            int* enchTypePtr;
            enchTypePtr = (int*)&actroPtr->ActorHasItems[3]
                              .special_enchantment;  // this is the amount of
                                                     // gold the actor has

            if ((int)enchBonusSum >= *enchTypePtr) {  // steal all the gold
                enchBonusSum = *enchTypePtr;
                actroPtr->ActorHasItems[3].uItemID = ITEM_NULL;
                *enchTypePtr = 0;
            } else {
                *enchTypePtr -= enchBonusSum;  // steal some of the gold
            }

            if (enchBonusSum) {
                pParty->PartyFindsGold(enchBonusSum, 2);
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    302, this->pName, enchBonusSum));  // %s stole %d gold
            } else {
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    377, this->pName));  // %s failed to steal anything
            }

            return 2;                     // stole gold
        } else if (random % 100 >= 40) {  // stealing an item
            ItemGen tempItem;
            tempItem.Reset();

            int randslot = rand() % 4;
            unsigned int carriedItemId =
                actroPtr
                    ->uCarriedItemID;  // carried items are special items the
                                       // ncp carries (ie lute from bard)

            if (carriedItemId != 0 ||
                actroPtr->ActorHasItems[randslot].uItemID != 0 &&
                    actroPtr->ActorHasItems[randslot].GetItemEquipType() !=
                        EQUIP_GOLD) {  // check we have an item to steal
                if (carriedItemId != 0) {  // load item into tempitem
                    actroPtr->uCarriedItemID = 0;
                    tempItem.uItemID = carriedItemId;
                    if (pItemsTable->pItems[carriedItemId].uEquipType ==
                        EQUIP_WAND)
                        tempItem.uNumCharges =
                            rand() % 6 +
                            pItemsTable->pItems[carriedItemId].uDamageMod + 1;
                    else if (pItemsTable->pItems[carriedItemId].uEquipType ==
                                 EQUIP_POTION &&
                             carriedItemId != ITEM_POTION_BOTTLE)
                        tempItem.uEnchantmentType = 2 * rand() % 4 + 2;
                } else {
                    ItemGen* itemToSteal = &actroPtr->ActorHasItems[randslot];
                    memcpy(&tempItem, itemToSteal, sizeof(tempItem));
                    itemToSteal->Reset();
                    carriedItemId = tempItem.uItemID;
                }

                if (carriedItemId !=
                    0) {  // looks odd in current context, but avoids accessing
                          // zeroth element of pItemsTable->pItems

                    GameUI_StatusBar_OnEvent(localization->FormatString(
                        304,  // Official                   //TODO: add a normal
                              // "%d stole %d" message
                        this->pName,
                        pItemsTable->pItems[carriedItemId].pUnidentifiedName));

                    pParty->PickedItem_PlaceInInventory_or_Drop();  // drop or place picked item

                    memcpy(
                        &pParty->pPickedItem, &tempItem,
                        sizeof(ItemGen));  // copy item in to mouse picked item
                    mouse->SetCursorBitmapFromItemID(carriedItemId);
                    return 2;  // stole item
                }
            }
        }

        GameUI_StatusBar_OnEvent(localization->FormatString(
            377, this->pName));  // %s failed to steal anything
        return 1;
    }
}

//----- (0048DBB9) --------------------------------------------------------
void Player::Heal(int amount) {
    if (!IsEradicated() && !IsDead()) {  // cant heal
        int max_health = GetMaxHealth();

        if (IsZombie())  // zombie health is halved
            max_health /= 2;

        sHealth += amount;         // add health
        if (sHealth > max_health)  // limits check
            sHealth = max_health;

        if (IsUnconcious()) {
            if (sHealth > 0) {  // wake up if health rises above 0
                SetUnconcious(GameTime(0));
            }
        }
    }
}

//----- (0048DC1E) --------------------------------------------------------
int Player::ReceiveDamage(signed int amount, DAMAGE_TYPE dmg_type) {
    SetAsleep(GameTime(0));  // wake up if asleep
    signed int recieved_dmg = CalculateIncommingDamage(dmg_type, amount);  // get damage
    // for no damage cheat - moved from elsewhere
    if (!engine->config->NoDamage()) {
        sHealth -= recieved_dmg;     // reduce health
    }

    if (sHealth < 1) {  // player unconscious or if too hurt - dead
        if ((sHealth + uEndurance +
                 GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) >=
             1) ||
            pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active()) {
            SetCondUnconsciousWithBlockCheck(false);
        } else {
            SetCondDeadWithBlockCheck(false);
        }

        if (sHealth <= -10) {  // break armor if health has dropped below -10
            ItemGen* equippedArmor = GetArmorItem();
            if (equippedArmor != nullptr) {  // check there is some armor
                if (!(equippedArmor->uAttributes &
                      ITEM_HARDENED)) {          // if its not hardened
                    equippedArmor->SetBroken();  // break it
                }
            }
        }
    }

    if (recieved_dmg && CanAct()) PlaySound(SPEECH_24, 0);  // oww

    return recieved_dmg;
}

//----- (0048DCF6) --------------------------------------------------------
int Player::ReceiveSpecialAttackEffect(
    int attType,
    struct Actor* pActor) {  // long function - consider breaking into two??

    SPECIAL_ATTACK_TYPE attTypeCast = (SPECIAL_ATTACK_TYPE)attType;

    int statcheck;
    int statcheckbonus;
    int luckstat = GetActualLuck();
    signed int itemstobreakcounter = 0;
    char itemstobreaklist[140];
    ItemGen* itemtocheck = nullptr;
    ItemGen* itemtobreak = nullptr;
    unsigned int itemtostealinvindex = 0;

    switch (attTypeCast) {
        case SPECIAL_ATTACK_CURSE:
            statcheck = GetActualWillpower();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        case SPECIAL_ATTACK_WEAK:
        case SPECIAL_ATTACK_SLEEP:
        case SPECIAL_ATTACK_DRUNK:
        case SPECIAL_ATTACK_DISEASE_WEAK:
        case SPECIAL_ATTACK_DISEASE_MEDIUM:
        case SPECIAL_ATTACK_DISEASE_SEVERE:
        case SPECIAL_ATTACK_UNCONSCIOUS:
        case SPECIAL_ATTACK_AGING:
            statcheck = GetActualEndurance();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        case SPECIAL_ATTACK_INSANE:
        case SPECIAL_ATTACK_PARALYZED:
        case SPECIAL_ATTACK_FEAR:
            statcheckbonus =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND);
            break;

        case SPECIAL_ATTACK_PETRIFIED:
            statcheckbonus =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH);
            break;

        case SPECIAL_ATTACK_POISON_WEAK:
        case SPECIAL_ATTACK_POISON_MEDIUM:
        case SPECIAL_ATTACK_POISON_SEVERE:
        case SPECIAL_ATTACK_DEAD:
        case SPECIAL_ATTACK_ERADICATED:
            statcheckbonus =
                GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY);
            break;

        case SPECIAL_ATTACK_MANA_DRAIN:
            statcheckbonus = (GetParameterBonus(GetActualIntelligence()) +
                              GetParameterBonus(GetActualWillpower())) /
                             2;
            break;

        case SPECIAL_ATTACK_BREAK_ANY:
            for (int i = 0; i < 138; i++) {
                if (i < 126) {
                    itemtocheck = &this->pInventoryItemList[i];
                } else {
                    itemtocheck = &this->pEquippedItems[i - 126];
                }

                if (itemtocheck->uItemID > 0 && itemtocheck->uItemID <= 134 &&
                    !itemtocheck->IsBroken())
                    itemstobreaklist[itemstobreakcounter++] = i;
            }

            if (!itemstobreakcounter) return 0;

            itemtobreak = &this->pInventoryItemList
                               [itemstobreaklist[rand() % itemstobreakcounter]];
            statcheckbonus =
                3 * (pItemsTable->pItems[itemtobreak->uItemID].uMaterial +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_BREAK_ARMOR:
            for (int i = 0; i < 16; i++) {
                if (HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    if (i == EQUIP_ARMOUR)
                        itemstobreaklist[itemstobreakcounter++] =
                            this->pEquipment.uArmor - 1;

                    if ((i == EQUIP_SINGLE_HANDED || i == EQUIP_TWO_HANDED) &&
                        GetEquippedItemEquipType((ITEM_EQUIP_TYPE)i) ==
                            EQUIP_SHIELD)
                        itemstobreaklist[itemstobreakcounter++] =
                            this->pEquipment.pIndices[i] - 1;
                }
            }

            if (!itemstobreakcounter) return 0;

            itemtobreak = &this->pInventoryItemList
                               [itemstobreaklist[rand() % itemstobreakcounter]];
            statcheckbonus =
                3 * (pItemsTable->pItems[itemtobreak->uItemID].uMaterial +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_BREAK_WEAPON:
            for (int i = 0; i < 16; i++) {
                if (HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    if (i == EQUIP_BOW)
                        itemstobreaklist[itemstobreakcounter++] =
                            (unsigned char)(this->pEquipment.uBow) - 1;

                    if ((i == EQUIP_SINGLE_HANDED || i == EQUIP_TWO_HANDED) &&
                        (GetEquippedItemEquipType((ITEM_EQUIP_TYPE)i) ==
                             EQUIP_SINGLE_HANDED ||
                         GetEquippedItemEquipType((ITEM_EQUIP_TYPE)i) ==
                             EQUIP_TWO_HANDED))
                        itemstobreaklist[itemstobreakcounter++] =
                            this->pEquipment.pIndices[i] - 1;
                }
            }

            if (!itemstobreakcounter) return 0;

            itemtobreak = &this->pInventoryItemList
                               [itemstobreaklist[rand() % itemstobreakcounter]];
            statcheckbonus =
                3 * (pItemsTable->pItems[itemtobreak->uItemID].uMaterial +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_STEAL:
            for (int i = 0; i < 126; i++) {
                int ItemPosInList = this->pInventoryMatrix[i];

                if (ItemPosInList > 0) {
                    itemtocheck = &this->pInventoryItemList[ItemPosInList - 1];

                    if (itemtocheck->uItemID > 0 &&
                        itemtocheck->uItemID <= 134) {
                        itemstobreaklist[itemstobreakcounter++] = i;
                    }
                }
            }

            if (!itemstobreakcounter) return 0;

            itemtostealinvindex =
                itemstobreaklist[rand() % itemstobreakcounter];
            statcheck = GetActualAccuracy();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        default:
            statcheckbonus = 0;
            break;
    }

    signed int savecheck = GetParameterBonus(luckstat) + statcheckbonus + 30;
    signed int whichplayer;

    if (rand() % savecheck >= 30) {  // saving throw if attacke is avoided
        return 0;
    } else {
        for (whichplayer = 0; whichplayer < 4; whichplayer++) {
            if (this == pPlayers[whichplayer + 1]) break;
        }

        // pass this to new fucntion??
        // atttypecast - whichplayer - itemtobreak - itemtostealinvindex

        switch (attTypeCast) {
            case SPECIAL_ATTACK_CURSE:
                SetCondition(Condition_Cursed, 1);
                pAudioPlayer->PlaySound(SOUND_star1, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_WEAK:
                SetCondition(Condition_Weak, 1);
                pAudioPlayer->PlaySound(SOUND_star1, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_SLEEP:
                SetCondition(Condition_Sleep, 1);
                pAudioPlayer->PlaySound(SOUND_star1, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DRUNK:
                SetCondition(Condition_Drunk, 1);
                pAudioPlayer->PlaySound(SOUND_star1, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_INSANE:
                SetCondition(Condition_Insane, 1);
                pAudioPlayer->PlaySound(SOUND_star4, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_WEAK:
                SetCondition(Condition_Poison_Weak, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_MEDIUM:
                SetCondition(Condition_Poison_Medium, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_SEVERE:
                SetCondition(Condition_Poison_Severe, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_WEAK:
                SetCondition(Condition_Disease_Weak, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_MEDIUM:
                SetCondition(Condition_Disease_Medium, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_SEVERE:
                SetCondition(Condition_Disease_Severe, 1);
                pAudioPlayer->PlaySound(SOUND_star2, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_PARALYZED:
                SetCondition(Condition_Paralyzed, 1);
                pAudioPlayer->PlaySound(SOUND_star4, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_UNCONSCIOUS:
                SetCondition(Condition_Unconcious, 1);
                pAudioPlayer->PlaySound(SOUND_star4, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DEAD:
                SetCondition(Condition_Dead, 1);
                pAudioPlayer->PlaySound(SOUND_eradicate, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_PETRIFIED:
                SetCondition(Condition_Pertified, 1);
                pAudioPlayer->PlaySound(SOUND_eradicate, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_ERADICATED:
                SetCondition(Condition_Eradicated, 1);
                pAudioPlayer->PlaySound(SOUND_eradicate, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_BREAK_ANY:
            case SPECIAL_ATTACK_BREAK_ARMOR:
            case SPECIAL_ATTACK_BREAK_WEAPON:
                if (!(itemtobreak->uAttributes & ITEM_HARDENED)) {
                    PlaySound(SPEECH_40, 0);
                    itemtobreak->SetBroken();
                    pAudioPlayer->PlaySound(SOUND_metal_vs_metal03h, 0, 0, -1, 0, 0);
                }
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_STEAL:
                PlaySound(SPEECH_40, 0);
                void* actoritems;
                actoritems = pActor->ActorHasItems;
                if (pActor->ActorHasItems[0].uItemID) {
                    actoritems = &pActor->ActorHasItems[1];
                    if (pActor->ActorHasItems[1].uItemID) {
                        spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                        return 1;
                    }
                }

                memcpy(actoritems,
                       &this->pInventoryItemList[this->pInventoryMatrix[itemtostealinvindex] - 1],
                       0x24u);
                RemoveItemAtInventoryIndex(itemtostealinvindex);
                pAudioPlayer->PlaySound(SOUND_metal_vs_metal03h, 0, 0, -1,
                                        0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_AGING:
                PlaySound(SPEECH_42, 0);
                ++this->sAgeModifier;
                pAudioPlayer->PlaySound(SOUND_eleccircle, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_MANA_DRAIN:
                PlaySound(SPEECH_41, 0);
                this->sMana = 0;
                pAudioPlayer->PlaySound(SOUND_eleccircle, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_FEAR:
                SetCondition(Condition_Fear, 1);
                pAudioPlayer->PlaySound(SOUND_star1, 0, 0, -1, 0, 0);
                spell_fx_renderer->SetPlayerBuffAnim(0x99u, whichplayer);
                return 1;
                break;

            default:
                return 0;
        }
    }
}

// 48DCF6: using guessed type char var_94[140];

//----- (0048E1A3) --------------------------------------------------------
unsigned int Player::GetSpellSchool(unsigned int uSpellID) {
    return pSpellStats->pInfos[uSpellID].uSchool;
}

//----- (0048E1B5) --------------------------------------------------------
int Player::GetAttackRecoveryTime(bool bRangedAttack) {
    ItemGen* weapon = nullptr;
    uint weapon_recovery = base_recovery_times_per_weapon_type[0];
    if (bRangedAttack) {
        if (HasItemEquipped(EQUIP_BOW)) {
            weapon = GetBowItem();
            weapon_recovery =
                base_recovery_times_per_weapon_type[weapon
                                                        ->GetPlayerSkillType()];
        }
    } else if (IsUnarmed() == 1 && GetActualSkillLevel(PLAYER_SKILL_UNARMED) > 0) {
        weapon_recovery = base_recovery_times_per_weapon_type[1];
    } else if (HasItemEquipped(EQUIP_TWO_HANDED)) {
        weapon = GetMainHandItem();
        if (weapon->GetItemEquipType() == EQUIP_WAND) {
            // __debugbreak();  // looks like offset in player's inventory and
                             // wand_lut much like case in 0042ECB5
            // __debugbreak();  // looks like wands were two-handed weapons once,
                             // or supposed to be. should not get here now
            weapon_recovery = pSpellDatas[wand_spell_ids[weapon->uItemID - ITEM_WAND_FIRE]].uExpertLevelRecovery;
        } else {
            weapon_recovery = base_recovery_times_per_weapon_type[weapon->GetPlayerSkillType()];
        }
    }
    if (HasItemEquipped(EQUIP_SINGLE_HANDED) &&
        GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) != EQUIP_SHIELD) {
    // ADD: shield check because shield recovery is added later and can be
    // accidentally doubled
        if (base_recovery_times_per_weapon_type[GetOffHandItem()
                                                    ->GetPlayerSkillType()] >
            weapon_recovery) {
            weapon = GetOffHandItem();
            weapon_recovery =
                base_recovery_times_per_weapon_type[weapon
                                                        ->GetPlayerSkillType()];
        }
    }

    uint armour_recovery = 0;
    if (HasItemEquipped(EQUIP_ARMOUR)) {
        uchar armour_skill_type = GetArmorItem()->GetPlayerSkillType();
        uint base_armour_recovery =
            base_recovery_times_per_weapon_type[armour_skill_type];
        float multiplier;

        if (armour_skill_type == PLAYER_SKILL_LEATHER) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(
                armour_skill_type, 1.0f, 0, 0, 0);
        } else if (armour_skill_type == PLAYER_SKILL_CHAIN) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(
                armour_skill_type, 1.0f, 0.5f, 0, 0);
        } else if (armour_skill_type == PLAYER_SKILL_PLATE) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(
                armour_skill_type, 1.0f, 0.5f, 0.5f, 0);
        } else {
            // PLAYER_SKILL_MISC
            // any others?
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(
                armour_skill_type, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        armour_recovery = (uint)(base_armour_recovery * multiplier);
    }

    uint shield_recovery = 0;
    if (HasItemEquipped(EQUIP_SINGLE_HANDED) &&
        GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) == EQUIP_SHIELD) {
        uchar skill_type = GetOffHandItem()->GetPlayerSkillType();

        uint shield_base_recovery =
            base_recovery_times_per_weapon_type[skill_type];
        float multiplier =
            GetArmorRecoveryMultiplierFromSkillLevel(skill_type, 1.0f, 0, 0, 0);
        shield_recovery = (uint)(shield_base_recovery * multiplier);
    }

    uint player_speed_recovery_reduction = GetParameterBonus(GetActualSpeed()),
         sword_axe_bow_recovery_reduction = 0;
    bool shooting_laser = false;
    if (weapon != nullptr) {
        if (GetActualSkillLevel(
                (PLAYER_SKILL_TYPE)weapon->GetPlayerSkillType()) &&
            (weapon->GetPlayerSkillType() == PLAYER_SKILL_SWORD ||
             weapon->GetPlayerSkillType() == PLAYER_SKILL_AXE ||
             weapon->GetPlayerSkillType() == PLAYER_SKILL_BOW)) {
            if (SkillToMastery(pActiveSkills[weapon->GetPlayerSkillType()]) >=
                2)  // Expert   Sword, Axe & Bow   reduce recovery
                sword_axe_bow_recovery_reduction =
                    pActiveSkills[weapon->GetPlayerSkillType()] & 0x3F;
        }
        if (weapon->GetPlayerSkillType() == PLAYER_SKILL_BLASTER)
            shooting_laser = true;
    }

    uint armsmaster_recovery_reduction = 0;
    if (!bRangedAttack && !shooting_laser) {
        if (uint armsmaster_level =
                GetActualSkillLevel(PLAYER_SKILL_ARMSMASTER)) {
            armsmaster_recovery_reduction = armsmaster_level & 0x3F;
            if (SkillToMastery(armsmaster_level) >= 4)
                armsmaster_recovery_reduction *= 2;
        }
    }

    uint hasteRecoveryReduction = 0;
    if (pPlayerBuffs[PLAYER_BUFF_HASTE].Active()) hasteRecoveryReduction = 25;
    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Active()) hasteRecoveryReduction = 25;

    uint weapon_enchantment_recovery_reduction = 0;
    if (weapon) {
        if (weapon->special_enchantment == 59 ||
            weapon->special_enchantment == 41 ||
            weapon->special_enchantment == 500)
            weapon_enchantment_recovery_reduction = 20;
    }

    int recovery = weapon_recovery + armour_recovery + shield_recovery -
                   armsmaster_recovery_reduction -
                   weapon_enchantment_recovery_reduction -
                   hasteRecoveryReduction - sword_axe_bow_recovery_reduction -
                   player_speed_recovery_reduction;

    if (bRangedAttack || shooting_laser) {
        if (recovery < 5) recovery = 5;
    } else {
        if (recovery < 30) recovery = 30;
    }

    return recovery;
}

//----- new --------------------------------------------------------
float Player::GetArmorRecoveryMultiplierFromSkillLevel(
    unsigned char armour_skill_type, float mult1, float mult2, float mult3,
    float mult4) {
    uint skill_mastery = SkillToMastery(pActiveSkills[armour_skill_type]);

    switch (skill_mastery) {
        case 1:
            return mult1;
            break;
        case 2:
            return mult2;
            break;
        case 3:
            return mult3;
            break;
        case 4:
            return mult4;
            break;
    }
    Error("Unexpected input value: %d", armour_skill_type);
    return 0;
}

//----- (0048E4F8) --------------------------------------------------------
int Player::GetMaxHealth() {
    int endbonus = GetParameterBonus(GetActualEndurance());
    int healthbylevel =
        pBaseHealthPerLevelByClass[classType] * (GetActualLevel() + endbonus);
    int itembonus = GetItemsBonus(CHARACTER_ATTRIBUTE_HEALTH) + healthbylevel;
    int maxhealth = uFullHealthBonus + pBaseHealthByClass[classType / 4] +
                    GetSkillBonus(CHARACTER_ATTRIBUTE_HEALTH) + itembonus;

    if (maxhealth < 0)  // min zero
        maxhealth = 0;

    return maxhealth;
}

//----- (0048E565) --------------------------------------------------------
int Player::GetMaxMana() {
    int mainmanastat;
    int statbonus;
    int addmanastat;

    switch (classType) {
        case PLAYER_CLASS_ROGUE:
        case PLAYER_CLASS_SPY:
        case PLAYER_CLASS_ASSASSIN:
        case PLAYER_CLASS_ARCHER:
        case PLAYER_CLASS_WARRIOR_MAGE:
        case PLAYER_CLASS_MASTER_ARCHER:
        case PLAYER_CLASS_SNIPER:
        case PLAYER_CLASS_SORCERER:
        case PLAYER_CLASS_WIZARD:
        case PLAYER_CLASS_ARCHMAGE:
        case PLAYER_CLASS_LICH:
            // intelligence based mana
            mainmanastat = GetActualIntelligence();
            statbonus = GetParameterBonus(mainmanastat);
            break;

        case PLAYER_CLASS_INITIATE:
        case PLAYER_CLASS_MASTER:
        case PLAYER_CLASS_NINJA:
        case PLAYER_CLASS_PALADIN:
        case PLAYER_CLASS_CRUSADER:
        case PLAYER_CLASS_HERO:
        case PLAYER_CLASS_VILLIAN:
        case PLAYER_CLASS_CLERIC:
        case PLAYER_CLASS_PRIEST:
        case PLAYER_CLASS_PRIEST_OF_SUN:
        case PLAYER_CLASS_PRIEST_OF_MOON:
            // personality based mana
            mainmanastat = GetActualWillpower();
            statbonus = GetParameterBonus(mainmanastat);
            break;

        case PLAYER_CLASS_HUNTER:
        case PLAYER_CLASS_RANGER_LORD:
        case PLAYER_CLASS_BOUNTY_HUNTER:
        case PLAYER_CLASS_DRUID:
        case PLAYER_CLASS_GREAT_DRUID:
        case PLAYER_CLASS_ARCH_DRUID:
        case PLAYER_CLASS_WARLOCK:
            // mixed base mana
            mainmanastat = GetActualWillpower();
            statbonus = GetParameterBonus(mainmanastat);
            addmanastat = GetActualIntelligence();
            statbonus += GetParameterBonus(addmanastat);
            break;

        default:  // no magic
            return 0;
            break;
    }

    int manabylevel =
        pBaseManaPerLevelByClass[classType] * (GetActualLevel() + statbonus);
    int itembonus = GetItemsBonus(CHARACTER_ATTRIBUTE_MANA) + manabylevel;
    int maxmana = uFullManaBonus + pBaseManaByClass[classType / 4] +
                  GetSkillBonus(CHARACTER_ATTRIBUTE_MANA) + itembonus;

    if (maxmana < 0)  // min of 0
        maxmana = 0;

    return maxmana;
}

//----- (0048E656) --------------------------------------------------------
int Player::GetBaseAC() {
    int acc = GetActualAccuracy();
    int accbonus = GetParameterBonus(acc);
    int itembonus = GetItemsBonus(CHARACTER_ATTRIBUTE_AC_BONUS) + accbonus;
    int skillbonus = GetSkillBonus(CHARACTER_ATTRIBUTE_AC_BONUS) + itembonus;

    if (skillbonus < 0)  // min zero
        skillbonus = 0;

    return skillbonus;
}

//----- (0048E68F) --------------------------------------------------------
int Player::GetActualAC() {
    int acc = GetActualAccuracy();
    int accbonus = GetParameterBonus(acc);
    int itembonus = GetItemsBonus(CHARACTER_ATTRIBUTE_AC_BONUS) + accbonus;
    int skillbonus = GetSkillBonus(CHARACTER_ATTRIBUTE_AC_BONUS) + itembonus;

    int result = this->sACModifier +
                 GetMagicalBonus(CHARACTER_ATTRIBUTE_AC_BONUS) + skillbonus;

    if (result < 0)  // min zero
        result = 0;

    return result;
}

//----- (0048E6DC) --------------------------------------------------------
unsigned int Player::GetBaseAge() {
    return pParty->GetPlayingTime().GetYears() - this->uBirthYear + game_starting_year;
}

//----- (0048E72C) --------------------------------------------------------
unsigned int Player::GetActualAge() {
    return this->sAgeModifier + GetBaseAge();
}

//----- (0048E73F) --------------------------------------------------------
int Player::GetBaseResistance(enum CHARACTER_ATTRIBUTE_TYPE a2) {
    int v7;  // esi@20
    int racialBonus = 0;
    __int16* resStat;
    int result;

    switch (a2) {
        case CHARACTER_ATTRIBUTE_RESIST_FIRE:
            resStat = &sResFireBase;
            if (IsRaceGoblin()) racialBonus = 5;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_AIR:
            resStat = &sResAirBase;
            if (IsRaceGoblin()) racialBonus = 5;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_WATER:
            resStat = &sResWaterBase;
            if (IsRaceDwarf()) racialBonus = 5;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_EARTH:
            resStat = &sResEarthBase;
            if (IsRaceDwarf()) racialBonus = 5;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_MIND:
            resStat = &sResMindBase;
            if (IsRaceElf()) racialBonus = 10;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_BODY:
        case CHARACTER_ATTRIBUTE_RESIST_SPIRIT:
            resStat = &sResBodyBase;
            if (IsRaceHuman()) racialBonus = 5;
            break;
        default:
            Error("Unknown attribute");
    }
    v7 = GetItemsBonus(a2) + racialBonus;
    result = v7 + *resStat;
    if (classType == PLAYER_CLASS_LICH) {
        if (result > 200) result = 200;
    }
    return result;
}

//----- (0048E7D0) --------------------------------------------------------
int Player::GetActualResistance(enum CHARACTER_ATTRIBUTE_TYPE a2) {
    signed int v10 = 0;  // [sp+14h] [bp-4h]@1
    __int16* resStat;
    int result;
    int baseRes;

    int leatherArmorSkillLevel = GetActualSkillLevel(PLAYER_SKILL_LEATHER);

    if (CheckHiredNPCSpeciality(Enchanter)) v10 = 20;
    if ((a2 == CHARACTER_ATTRIBUTE_RESIST_FIRE ||
         a2 == CHARACTER_ATTRIBUTE_RESIST_AIR ||
         a2 == CHARACTER_ATTRIBUTE_RESIST_WATER ||
         a2 == CHARACTER_ATTRIBUTE_RESIST_EARTH) &&
        GetActualSkillMastery(PLAYER_SKILL_LEATHER) == 4 &&
        HasItemEquipped(EQUIP_ARMOUR) &&
        GetEquippedItemSkillType(EQUIP_ARMOUR) == PLAYER_SKILL_LEATHER)
        v10 += leatherArmorSkillLevel;  // &0x3F;

    switch (a2) {
        case CHARACTER_ATTRIBUTE_RESIST_FIRE:
            resStat = &sResFireBonus;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_AIR:
            resStat = &sResAirBonus;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_WATER:
            resStat = &sResWaterBonus;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_EARTH:
            resStat = &sResEarthBonus;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_MIND:
            resStat = &sResMindBonus;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_BODY:
        case CHARACTER_ATTRIBUTE_RESIST_SPIRIT:
            resStat = &sResBodyBonus;
            break;
        default:
            Error("Unexpected attribute");
    }
    baseRes = GetBaseResistance(a2);
    result = v10 + GetMagicalBonus(a2) + baseRes + *(resStat);
    if (classType == PLAYER_CLASS_LICH) {
        if (result > 200) result = 200;
    }
    return result;
}

//----- (0048E8F5) --------------------------------------------------------
bool Player::Recover(int dt) {
    int timepassed =
        (int)(dt * GetSpecialItemBonus(ITEM_ENCHANTMENT_OF_RECOVERY) * 0.01 +
              dt);

    if (uTimeToRecovery > timepassed) {  // need more time till recovery
        uTimeToRecovery -= timepassed;
        return true;
    } else {
        uTimeToRecovery = 0;  // recovered
        viewparams->bRedrawGameUI = true;

        if (!uActiveCharacter)  // set recoverd char as active
            uActiveCharacter = pParty->GetNextActiveCharacter();

        return false;
    }
}

//----- (0048E96A) --------------------------------------------------------
void Player::SetRecoveryTime(signed int rec) {
    Assert(rec >= 0);

    if (rec > uTimeToRecovery) uTimeToRecovery = rec;

    if (uActiveCharacter != 0 && pPlayers[uActiveCharacter] == this &&
        !some_active_character)
        uActiveCharacter = pParty->GetNextActiveCharacter();

    viewparams->bRedrawGameUI = true;
}

//----- (0048E9B7) --------------------------------------------------------
void Player::RandomizeName() {
    if (!uExpressionTimePassed)
        strcpy(
            pName,
            pNPCStats->pNPCNames[rand() % pNPCStats->uNumNPCNames[uSex]][uSex]);
}

//----- (0048E9F4) --------------------------------------------------------
unsigned int Player::GetMajorConditionIdx() {
    for (uint i = 0; i < 18; ++i) {
        if (conditions_times[pConditionImportancyTable[i]].Valid())
            return pConditionImportancyTable[i];  // return worst condition
    }
    return 18;  // condition good
}

//----- (0048EA1B) --------------------------------------------------------
int Player::GetParameterBonus(int player_parameter) {
    int i;  // eax@1
    i = 0;
    while (param_to_bonus_table[i]) {
        if (player_parameter >= param_to_bonus_table[i]) break;
        ++i;
    }
    return parameter_to_bonus_value[i];
}

//----- (0048EA46) --------------------------------------------------------
int Player::GetSpecialItemBonus(ITEM_ENCHANTMENT enchantment) {
    for (int i = EQUIP_SINGLE_HANDED; i < EQUIP_BOOK; ++i) {
        if (HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
            if (enchantment == ITEM_ENCHANTMENT_OF_RECOVERY) {
                if (GetNthEquippedIndexItem(i)->special_enchantment ==
                        ITEM_ENCHANTMENT_OF_RECOVERY ||
                    (GetNthEquippedIndexItem(i)->uItemID ==
                     ITEM_ELVEN_CHAINMAIL))
                    return 50;
            }
            if (enchantment == ITEM_ENCHANTMENT_OF_FORCE) {
                if (GetNthEquippedIndexItem(i)->special_enchantment ==
                    ITEM_ENCHANTMENT_OF_FORCE)
                    return 5;
            }
        }
    }
    return 0;
}

//----- (0048EAAE) --------------------------------------------------------
int Player::GetItemsBonus(enum CHARACTER_ATTRIBUTE_TYPE attr,
                          bool getOnlyMainHandDmg /*= false*/) {
    int v5;                     // edi@1
    int v9;                     // eax@49
    int v14;                    // ecx@58
    int v15;                    // eax@58
    int v17;                    // eax@62
    int v22;                    // eax@76
    int v25;                    // ecx@80
    int v26;                    // edi@80
    int v32;                    // eax@98
    int v56;                    // eax@365
    signed int v58;             // [sp-4h] [bp-20h]@10
    int v61;                    // [sp+10h] [bp-Ch]@1
    int v62;                    // [sp+14h] [bp-8h]@1
    ItemGen* currEquippedItem;  // [sp+20h] [bp+4h]@101
    bool no_skills;

    v5 = 0;
    v62 = 0;
    v61 = 0;

    no_skills = false;
    switch (attr) {
        case CHARACTER_ATTRIBUTE_SKILL_ALCHEMY:
            v58 = PLAYER_SKILL_ALCHEMY;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_STEALING:
            v58 = PLAYER_SKILL_STEALING;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM:
            v58 = PLAYER_SKILL_TRAP_DISARM;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_ITEM_ID:
            v58 = PLAYER_SKILL_ITEM_ID;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID:
            v58 = PLAYER_SKILL_MONSTER_ID;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER:
            v58 = PLAYER_SKILL_ARMSMASTER;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_DODGE:
            v58 = PLAYER_SKILL_DODGE;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_UNARMED:
            v58 = PLAYER_SKILL_UNARMED;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_FIRE:
            v58 = PLAYER_SKILL_FIRE;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_AIR:
            v58 = PLAYER_SKILL_AIR;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_WATER:
            v58 = PLAYER_SKILL_WATER;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_EARTH:
            v58 = PLAYER_SKILL_EARTH;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_SPIRIT:
            v58 = PLAYER_SKILL_SPIRIT;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_MIND:
            v58 = PLAYER_SKILL_MIND;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_BODY:
            v58 = PLAYER_SKILL_BODY;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_LIGHT:
            v58 = PLAYER_SKILL_LIGHT;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_DARK:
            v58 = PLAYER_SKILL_DARK;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_MEDITATION:
            v58 = PLAYER_SKILL_MEDITATION;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_BOW:
            v58 = PLAYER_SKILL_BOW;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_SHIELD:
            v58 = PLAYER_SKILL_SHIELD;
            break;
        case CHARACTER_ATTRIBUTE_SKILL_LEARNING:
            v58 = PLAYER_SKILL_LEARNING;
            break;
        default:
            no_skills = true;
    }
    if (!no_skills) {
        if (!this->pActiveSkills[v58]) return 0;
    }

    switch (attr) {  // TODO(_) would be nice to move these into separate functions
        case CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS:
        case CHARACTER_ATTRIBUTE_RANGED_ATTACK:
            if (HasItemEquipped(EQUIP_BOW)) v5 = GetBowItem()->GetDamageMod();
            return v5;
            break;

        case CHARACTER_ATTRIBUTE_RANGED_DMG_MIN:
            if (!HasItemEquipped(EQUIP_BOW)) return 0;
            v5 = GetBowItem()->GetDamageMod();
            v56 = GetBowItem()->GetDamageDice();
            return v5 + v56;
            break;

        case CHARACTER_ATTRIBUTE_RANGED_DMG_MAX:
            if (!HasItemEquipped(EQUIP_BOW)) return 0;
            v5 = GetBowItem()->GetDamageDice() * GetBowItem()->GetDamageRoll();
            v56 = GetBowItem()->GetDamageMod();
            return v5 + v56;

        case CHARACTER_ATTRIBUTE_LEVEL:
            if (!Player::HasEnchantedItemEquipped(25)) return 0;
            return 5;
            break;

        case CHARACTER_ATTRIBUTE_MELEE_DMG_MAX:
            if (IsUnarmed()) {
                return 3;
            } else {
                if (this->HasItemEquipped(EQUIP_TWO_HANDED)) {
                    v22 = this->GetEquippedItemEquipType(EQUIP_TWO_HANDED);
                    if (v22 >= 0 && v22 <= 2) {
                        ItemGen* mainHandItem = GetMainHandItem();
                        v26 = mainHandItem->GetDamageRoll();
                        if (GetOffHandItem() != nullptr ||
                            mainHandItem->GetPlayerSkillType() != 4) {
                            v25 = mainHandItem->GetDamageDice();
                        } else {
                            v25 = mainHandItem->GetDamageDice() + 1;
                        }
                        v5 = mainHandItem->GetDamageMod() + v25 * v26;
                    }
                }
                if (getOnlyMainHandDmg ||
                    !this->HasItemEquipped(EQUIP_SINGLE_HANDED) ||
                    (GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) < 0 ||
                     GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) > 2)) {
                    return v5;
                } else {
                    ItemGen* offHandItem = GetOffHandItem();
                    v15 = offHandItem->GetDamageMod();
                    v14 = offHandItem->GetDamageDice() *
                          offHandItem->GetDamageRoll();
                    return v5 + v15 + v14;
                }
            }
            break;

        case CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS:
        case CHARACTER_ATTRIBUTE_ATTACK:
            if (IsUnarmed()) {
                return 0;
            }
            if (this->HasItemEquipped(EQUIP_TWO_HANDED)) {
                v17 = this->GetEquippedItemEquipType(EQUIP_TWO_HANDED);
                if (v17 >= 0 && v17 <= 2) {
                    v5 = GetMainHandItem()->GetDamageMod();
                }
            }
            if (getOnlyMainHandDmg ||
                !this->HasItemEquipped(EQUIP_SINGLE_HANDED) ||
                (this->GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) < 0) ||
                this->GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) > 2) {
                return v5;
            } else {
                v56 = GetOffHandItem()->GetDamageMod();
                return v5 + v56;
            }
            break;

        case CHARACTER_ATTRIBUTE_MELEE_DMG_MIN:
            if (IsUnarmed()) {
                return 1;
            }
            if (this->HasItemEquipped(EQUIP_TWO_HANDED)) {
                v9 = this->GetEquippedItemEquipType(EQUIP_TWO_HANDED);
                if (v9 >= 0 && v9 <= 2) {
                    ItemGen* mainHandItem = GetMainHandItem();
                    v5 = mainHandItem->GetDamageDice() +
                         mainHandItem->GetDamageMod();
                    if (GetOffHandItem() == nullptr &&
                        mainHandItem->GetPlayerSkillType() == 4) {
                        ++v5;
                    }
                }
            }

            if (getOnlyMainHandDmg ||
                !this->HasItemEquipped(EQUIP_SINGLE_HANDED) ||
                (this->GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) < 0) ||
                this->GetEquippedItemEquipType(EQUIP_SINGLE_HANDED) > 2) {
                return v5;
            } else {
                ItemGen* offHandItem = GetOffHandItem();
                v14 = offHandItem->GetDamageMod();
                v15 = offHandItem->GetDamageDice();
                return v5 + v15 + v14;
            }
            break;

        case CHARACTER_ATTRIBUTE_STRENGTH:
        case CHARACTER_ATTRIBUTE_INTELLIGENCE:
        case CHARACTER_ATTRIBUTE_WILLPOWER:
        case CHARACTER_ATTRIBUTE_ENDURANCE:
        case CHARACTER_ATTRIBUTE_ACCURACY:
        case CHARACTER_ATTRIBUTE_SPEED:
        case CHARACTER_ATTRIBUTE_LUCK:
        case CHARACTER_ATTRIBUTE_HEALTH:
        case CHARACTER_ATTRIBUTE_MANA:
        case CHARACTER_ATTRIBUTE_AC_BONUS:

        case CHARACTER_ATTRIBUTE_RESIST_FIRE:
        case CHARACTER_ATTRIBUTE_RESIST_AIR:
        case CHARACTER_ATTRIBUTE_RESIST_WATER:
        case CHARACTER_ATTRIBUTE_RESIST_EARTH:
        case CHARACTER_ATTRIBUTE_RESIST_MIND:
        case CHARACTER_ATTRIBUTE_RESIST_BODY:
        case CHARACTER_ATTRIBUTE_RESIST_SPIRIT:

        case CHARACTER_ATTRIBUTE_SKILL_ALCHEMY:
        case CHARACTER_ATTRIBUTE_SKILL_STEALING:
        case CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM:
        case CHARACTER_ATTRIBUTE_SKILL_ITEM_ID:
        case CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID:
        case CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER:
        case CHARACTER_ATTRIBUTE_SKILL_DODGE:
        case CHARACTER_ATTRIBUTE_SKILL_UNARMED:

        case CHARACTER_ATTRIBUTE_SKILL_FIRE:
        case CHARACTER_ATTRIBUTE_SKILL_AIR:
        case CHARACTER_ATTRIBUTE_SKILL_WATER:
        case CHARACTER_ATTRIBUTE_SKILL_EARTH:
        case CHARACTER_ATTRIBUTE_SKILL_SPIRIT:
        case CHARACTER_ATTRIBUTE_SKILL_MIND:
        case CHARACTER_ATTRIBUTE_SKILL_BODY:
        case CHARACTER_ATTRIBUTE_SKILL_LIGHT:
        case CHARACTER_ATTRIBUTE_SKILL_DARK:
        case CHARACTER_ATTRIBUTE_SKILL_MEDITATION:
        case CHARACTER_ATTRIBUTE_SKILL_BOW:
        case CHARACTER_ATTRIBUTE_SKILL_SHIELD:
        case CHARACTER_ATTRIBUTE_SKILL_LEARNING:
            for (int i = 0; i < 16; i++) {
                if (HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    currEquippedItem = GetNthEquippedIndexItem(i);
                    if (attr == CHARACTER_ATTRIBUTE_AC_BONUS) {
                        v32 = currEquippedItem->GetItemEquipType();
                        if (v32 >= 3 && v32 <= 11) {
                            v5 += currEquippedItem->GetDamageDice() +
                                  currEquippedItem->GetDamageMod();
                        }
                    }
                    if (pItemsTable->IsMaterialNonCommon(currEquippedItem) &&
                        !pItemsTable->IsMaterialSpecial(currEquippedItem)) {
                        currEquippedItem->GetItemBonusArtifact(this, attr,
                                                               &v62);
                    } else if (currEquippedItem->uEnchantmentType != 0) {
                        if (this->pInventoryItemList
                                    [this->pEquipment.pIndices[i] - 1]
                                        .uEnchantmentType -
                                1 == attr) {  // if
                                   // (currEquippedItem->IsRegularEnchanmentForAttribute(attr))
                            if (attr > CHARACTER_ATTRIBUTE_RESIST_BODY &&
                                v5 < currEquippedItem
                                         ->m_enchantmentStrength)  // for skills
                                                                   // bonuses
                                v5 = currEquippedItem->m_enchantmentStrength;
                            else  // for resists and attributes bonuses
                                v5 += currEquippedItem->m_enchantmentStrength;
                        }
                    } else {
                        currEquippedItem->GetItemBonusSpecialEnchantment(this, attr, &v5, &v61);
                    }
                }
            }
            return v5 + v62 + v61;
            break;
        default:
            return 0;
    }
}

//----- (0048F73C) --------------------------------------------------------
int Player::GetMagicalBonus(enum CHARACTER_ATTRIBUTE_TYPE a2) {
    int v3 = 0;  // eax@4
    int v4 = 0;  // ecx@5

    switch (a2) {
        case CHARACTER_ATTRIBUTE_RESIST_FIRE:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_FIRE].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_FIRE].uPower;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_AIR:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_AIR].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_AIR].uPower;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_BODY:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_BODY].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_BODY].uPower;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_WATER:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_WATER].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_WATER].uPower;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_EARTH:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_EARTH].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_EARTH].uPower;
            break;
        case CHARACTER_ATTRIBUTE_RESIST_MIND:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_RESIST_MIND].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_MIND].uPower;
            break;
        case CHARACTER_ATTRIBUTE_ATTACK:
        case CHARACTER_ATTRIBUTE_RANGED_ATTACK:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_BLESS]
                     .uPower;  // only player effect spell in both VI and VII
            break;
        case CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_HEROISM].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_HEROISM].uPower;
            break;
        case CHARACTER_ATTRIBUTE_STRENGTH:
            v3 = pPlayerBuffs[PLAYER_BUFF_STRENGTH].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_INTELLIGENCE:
            v3 = pPlayerBuffs[PLAYER_BUFF_INTELLIGENCE].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_WILLPOWER:
            v3 = pPlayerBuffs[PLAYER_BUFF_WILLPOWER].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_ENDURANCE:
            v3 = pPlayerBuffs[PLAYER_BUFF_ENDURANCE].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_ACCURACY:
            v3 = pPlayerBuffs[PLAYER_BUFF_ACCURACY].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_SPEED:
            v3 = pPlayerBuffs[PLAYER_BUFF_SPEED].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_LUCK:
            v3 = pPlayerBuffs[PLAYER_BUFF_LUCK].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].uPower;
            break;
        case CHARACTER_ATTRIBUTE_AC_BONUS:
            v3 = this->pPlayerBuffs[PLAYER_BUFF_STONESKIN].uPower;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_STONE_SKIN].uPower;
            break;
    }
    return v3 + v4;
}

//----- (0048F882) --------------------------------------------------------
int Player::GetActualSkillLevel(
    PLAYER_SKILL_TYPE uSkillType)  { // bitwise check & 0x3F when using this can
                                     // now be removed ?? must check behavior
    int bonus_value;        // esi@1
    uint16_t skill_value;  // ax@126
    int result;                    // al@127

    bonus_value = 0;
    switch (uSkillType) {
        case PLAYER_SKILL_MONSTER_ID: {
            if (CheckHiredNPCSpeciality(Hunter)) bonus_value = 6;
            if (CheckHiredNPCSpeciality(Sage)) bonus_value += 6;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID);
        } break;

        case PLAYER_SKILL_ARMSMASTER: {
            if (CheckHiredNPCSpeciality(Armsmaster)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Weaponsmaster)) bonus_value += 3;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER);
        } break;

        case PLAYER_SKILL_STEALING: {
            if (CheckHiredNPCSpeciality(Burglar)) bonus_value = 8;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_STEALING);
        } break;

        case PLAYER_SKILL_ALCHEMY: {
            if (CheckHiredNPCSpeciality(Herbalist)) bonus_value = 4;
            if (CheckHiredNPCSpeciality(Apothecary)) bonus_value += 8;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_ALCHEMY);
        } break;

        case PLAYER_SKILL_LEARNING: {
            if (CheckHiredNPCSpeciality(Teacher)) bonus_value = 10;
            if (CheckHiredNPCSpeciality(Instructor)) bonus_value += 15;
            if (CheckHiredNPCSpeciality(Scholar)) bonus_value += 5;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_LEARNING);
        } break;

        case PLAYER_SKILL_UNARMED: {
            if (CheckHiredNPCSpeciality(Monk)) bonus_value = 2;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_UNARMED);
        } break;

        case PLAYER_SKILL_DODGE: {
            if (CheckHiredNPCSpeciality(Monk)) bonus_value = 2;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_DODGE);
        } break;

        case PLAYER_SKILL_BOW:
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_BOW);
            break;
        case PLAYER_SKILL_SHIELD:
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_SHIELD);
            break;

        case PLAYER_SKILL_EARTH:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus_value += 4;
            if (classType == PLAYER_CLASS_WARLOCK && PartyHasDragon())
                bonus_value += 3;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_EARTH);
            break;
        case PLAYER_SKILL_FIRE:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus_value += 4;
            if (classType == PLAYER_CLASS_WARLOCK && PartyHasDragon())
                bonus_value += 3;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_FIRE);
            break;
        case PLAYER_SKILL_AIR:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus_value += 4;
            if (classType == PLAYER_CLASS_WARLOCK && PartyHasDragon())
                bonus_value += 3;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_AIR);
            break;
        case PLAYER_SKILL_WATER:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus_value += 4;
            if (classType == PLAYER_CLASS_WARLOCK && PartyHasDragon())
                bonus_value += 3;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_WATER);
            break;
        case PLAYER_SKILL_SPIRIT:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus_value += 4;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_SPIRIT);
            break;
        case PLAYER_SKILL_MIND:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus_value += 4;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_MIND);
            break;
        case PLAYER_SKILL_BODY:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus_value = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus_value += 4;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_BODY);
            break;
        case PLAYER_SKILL_LIGHT:
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_LIGHT);
            break;
        case PLAYER_SKILL_DARK: {
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_DARK);
        } break;

        case PLAYER_SKILL_MERCHANT: {
            if (CheckHiredNPCSpeciality(Trader)) bonus_value = 4;
            if (CheckHiredNPCSpeciality(Merchant)) bonus_value += 6;
            if (CheckHiredNPCSpeciality(Gypsy)) bonus_value += 3;
            if (CheckHiredNPCSpeciality(Duper)) bonus_value += 8;
        } break;

        case PLAYER_SKILL_PERCEPTION: {
            if (CheckHiredNPCSpeciality(Scout)) bonus_value = 6;
            if (CheckHiredNPCSpeciality(Psychic)) bonus_value += 5;
        } break;

        case PLAYER_SKILL_ITEM_ID:
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_ITEM_ID);
            break;
        case PLAYER_SKILL_MEDITATION:
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_MEDITATION);
            break;
        case PLAYER_SKILL_TRAP_DISARM: {
            if (CheckHiredNPCSpeciality(Tinker)) bonus_value = 4;
            if (CheckHiredNPCSpeciality(Locksmith)) bonus_value += 6;
            if (CheckHiredNPCSpeciality(Burglar)) bonus_value += 8;
            bonus_value += GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM);
        } break;
    }

    if (uSkillType == PLAYER_SKILL_CLUB) {
        // some items loaded in as clubs
        uSkillType = PLAYER_SKILL_MACE;
    }

    // cap skill and bonus at 60
    if (uSkillType == PLAYER_SKILL_MISC) {
        skill_value = 0;
    } else {
        skill_value = pActiveSkills[uSkillType] & 0x3F;
    }

    result = bonus_value + skill_value;

    if (result > 60) result = 60;

    // if ( bonus_value + (skill_value & 0x3F) < 60 )
    // /  result =
    // else
    //   result = skill_value & 0xFFFC | 0x3C; //al
    return result;
}

int Player::GetActualSkillMastery(PLAYER_SKILL_TYPE uSkillType) {
    switch (pActiveSkills[uSkillType] & 0x1C0) {  // could chnage to /64 + 1 ?
        case 0x100:
            return 4;  // Grandmaster
        case 0x80:
            return 3;  // Master
        case 0x40:
            return 2;  // Expert
        case 0x00:
            return 1;  // Normal
    }

    assert(false);  // should never get here
    return 0;
}

//----- (0048FC00) --------------------------------------------------------
int Player::GetSkillBonus(
    enum CHARACTER_ATTRIBUTE_TYPE
        inSkill) {  // TODO(_): move the individual implementations to attribute
                    // classes once possible ?? check
    int armsMasterBonus;

    armsMasterBonus = 0;
    int armmaster_skill = GetActualSkillLevel(PLAYER_SKILL_ARMSMASTER);
    if (armmaster_skill > 0) {
        int multiplier = 0;
        if (inSkill == CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS) {
            multiplier =
                GetMultiplierForSkillLevel(PLAYER_SKILL_ARMSMASTER, 0, 0, 1, 2);
        } else if (inSkill == CHARACTER_ATTRIBUTE_ATTACK) {
            multiplier =
                GetMultiplierForSkillLevel(PLAYER_SKILL_ARMSMASTER, 0, 1, 1, 2);
        }
        armsMasterBonus = multiplier * (armmaster_skill);  //& 0x3F);
    }

    switch (inSkill) {
        case CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS:
            if (HasItemEquipped(EQUIP_BOW)) {
                int bowSkillLevel = GetActualSkillLevel(PLAYER_SKILL_BOW);
                int multiplier =
                    GetMultiplierForSkillLevel(PLAYER_SKILL_BOW, 0, 0, 0, 1);
                return multiplier * (bowSkillLevel & 0x3F);
            }
            return 0;
            break;
        case CHARACTER_ATTRIBUTE_HEALTH: {
            int base_value = pBaseHealthPerLevelByClass[classType];
            int attrib_modif = GetBodybuilding();
            return base_value * attrib_modif;
        } break;
        case CHARACTER_ATTRIBUTE_MANA: {
            int base_value = pBaseManaPerLevelByClass[classType];
            int attrib_modif = GetMeditation();
            return base_value * attrib_modif;
        } break;
        case CHARACTER_ATTRIBUTE_AC_BONUS: {
            bool wearingArmor = false;
            bool wearingLeather = false;
            unsigned int ACSum = 0;

            for (int j = 0; j < 16; ++j) {
                ItemGen* currItem = GetNthEquippedIndexItem(j);
                if (currItem != nullptr && (!currItem->IsBroken())) {
                    PLAYER_SKILL_TYPE itemSkillType =
                        (PLAYER_SKILL_TYPE)currItem->GetPlayerSkillType();
                    int currArmorSkillLevel = 0;
                    int multiplier = 0;
                    switch (itemSkillType) {
                        case PLAYER_SKILL_STAFF:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 0, 1, 1, 1);
                            break;
                        case PLAYER_SKILL_SWORD:
                        case PLAYER_SKILL_SPEAR:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 0, 0, 0, 1);
                            break;
                        case PLAYER_SKILL_SHIELD:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            wearingArmor = true;
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 1, 1, 2, 2);
                            break;
                        case PLAYER_SKILL_LEATHER:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            wearingLeather = true;
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 1, 1, 2, 2);
                            break;
                        case PLAYER_SKILL_CHAIN:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            wearingArmor = true;
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 1, 1, 1, 1);
                            break;
                        case PLAYER_SKILL_PLATE:
                            currArmorSkillLevel =
                                GetActualSkillLevel(itemSkillType);
                            wearingArmor = true;
                            multiplier = GetMultiplierForSkillLevel(
                                itemSkillType, 1, 1, 1, 1);
                            break;
                    }
                    ACSum += multiplier * (currArmorSkillLevel & 0x3F);
                }
            }

            int dodgeSkillLevel = GetActualSkillLevel(PLAYER_SKILL_DODGE);
            int dodgeMastery = GetActualSkillMastery(PLAYER_SKILL_DODGE);
            int multiplier =
                GetMultiplierForSkillLevel(PLAYER_SKILL_DODGE, 1, 2, 3, 3);
            if (!wearingArmor && (!wearingLeather || dodgeMastery == 4)) {
                ACSum += multiplier * (dodgeSkillLevel & 0x3F);
            }
            return ACSum;
        } break;
        case CHARACTER_ATTRIBUTE_ATTACK:
            if (this->IsUnarmed()) {
                int unarmedSkill =
                    this->GetActualSkillLevel(PLAYER_SKILL_UNARMED);
                if (!unarmedSkill) {
                    return 0;
                }
                int multiplier = GetMultiplierForSkillLevel(
                    PLAYER_SKILL_UNARMED, 0, 1, 2, 2);
                return armsMasterBonus + multiplier * (unarmedSkill & 0x3F);
            }
            for (int i = 0; i < 16; ++i) {  // ?? what eh check behaviour
                if (this->HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    ItemGen* currItem = GetNthEquippedIndexItem(i);
                    if (currItem->GetItemEquipType() <= EQUIP_TWO_HANDED) {
                        PLAYER_SKILL_TYPE currItemSkillType =
                            (PLAYER_SKILL_TYPE)currItem->GetPlayerSkillType();
                        int currentItemSkillLevel =
                            this->GetActualSkillLevel(currItemSkillType);
                        if (currItemSkillType == PLAYER_SKILL_BLASTER) {
                            int multiplier = GetMultiplierForSkillLevel(
                                currItemSkillType, 1, 2, 3, 5);
                            return multiplier * (currentItemSkillLevel & 0x3F);
                        } else if (currItemSkillType == PLAYER_SKILL_STAFF &&
                                   this->GetActualSkillLevel(
                                       PLAYER_SKILL_UNARMED) > 0) {
                            int unarmedSkillLevel =
                                this->GetActualSkillLevel(PLAYER_SKILL_UNARMED);
                            int multiplier = GetMultiplierForSkillLevel(
                                PLAYER_SKILL_UNARMED, 1, 1, 2, 2);
                            return multiplier * (unarmedSkillLevel & 0x3F) +
                                   armsMasterBonus +
                                   (currentItemSkillLevel & 0x3F);
                        } else {
                            return armsMasterBonus +
                                   (currentItemSkillLevel & 0x3F);
                        }
                    }
                }
            }
            return 0;
            break;

        case CHARACTER_ATTRIBUTE_RANGED_ATTACK:
            for (int i = 0; i < 16; i++) {
                if (this->HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    ItemGen* currItemPtr = GetNthEquippedIndexItem(i);
                    if (currItemPtr->GetItemEquipType() == EQUIP_TWO_HANDED ||
                        currItemPtr->GetItemEquipType() ==
                            EQUIP_SINGLE_HANDED) {
                        PLAYER_SKILL_TYPE currentItemSkillType =
                            (PLAYER_SKILL_TYPE)GetNthEquippedIndexItem(i)
                                ->GetPlayerSkillType();
                        int currentItemSkillLevel =
                            this->GetActualSkillLevel(currentItemSkillType);
                        if (currentItemSkillType == PLAYER_SKILL_BOW) {
                            int multiplier = GetMultiplierForSkillLevel(
                                PLAYER_SKILL_BOW, 1, 1, 1, 1);
                            return multiplier * (currentItemSkillLevel & 0x3F);
                        } else if (currentItemSkillType ==
                                   PLAYER_SKILL_BLASTER) {
                            int multiplier = GetMultiplierForSkillLevel(
                                PLAYER_SKILL_BLASTER, 1, 2, 3, 5);
                            return multiplier * (currentItemSkillLevel & 0x3F);
                        }
                    }
                }
            }
            return 0;
            break;

        case CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS:
            if (this->IsUnarmed()) {
                int unarmedSkillLevel =
                    this->GetActualSkillLevel(PLAYER_SKILL_UNARMED);
                if (!unarmedSkillLevel) {
                    return 0;
                }
                int multiplier = GetMultiplierForSkillLevel(
                    PLAYER_SKILL_UNARMED, 0, 1, 2, 2);
                return multiplier * (unarmedSkillLevel & 0x3F);
            }
            for (int i = 0; i < 16; i++) {
                if (this->HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    ItemGen* currItemPtr = GetNthEquippedIndexItem(i);
                    if (currItemPtr->GetItemEquipType() == EQUIP_TWO_HANDED ||
                        currItemPtr->GetItemEquipType() ==
                            EQUIP_SINGLE_HANDED) {
                        PLAYER_SKILL_TYPE currItemSkillType =
                            (PLAYER_SKILL_TYPE)
                                currItemPtr->GetPlayerSkillType();
                        int currItemSkillLevel =
                            this->GetActualSkillLevel(currItemSkillType);
                        int baseSkillBonus;
                        int multiplier;
                        switch (currItemSkillType) {
                            case PLAYER_SKILL_STAFF:
                                if (this->GetActualSkillMastery(
                                        PLAYER_SKILL_STAFF) >= 4 &&
                                    this->GetActualSkillLevel(
                                        PLAYER_SKILL_UNARMED) > 0) {
                                    int unarmedSkillLevel =
                                        this->GetActualSkillLevel(
                                            PLAYER_SKILL_UNARMED);
                                    int multiplier = GetMultiplierForSkillLevel(
                                        PLAYER_SKILL_UNARMED, 0, 1, 2, 2);
                                    return multiplier *
                                           (unarmedSkillLevel & 0x3F);
                                } else {
                                    return armsMasterBonus;
                                }
                                break;

                            case PLAYER_SKILL_DAGGER:
                                multiplier = GetMultiplierForSkillLevel(
                                    PLAYER_SKILL_DAGGER, 0, 0, 0, 1);
                                baseSkillBonus =
                                    multiplier * (currItemSkillLevel & 0x3F);
                                return armsMasterBonus + baseSkillBonus;
                                break;
                            case PLAYER_SKILL_SWORD:
                                multiplier = GetMultiplierForSkillLevel(
                                    PLAYER_SKILL_SWORD, 0, 0, 0, 0);
                                baseSkillBonus =
                                    multiplier * (currItemSkillLevel & 0x3F);
                                return armsMasterBonus + baseSkillBonus;
                                break;
                            case PLAYER_SKILL_MACE:
                            case PLAYER_SKILL_SPEAR:
                                multiplier = GetMultiplierForSkillLevel(
                                    currItemSkillType, 0, 1, 1, 1);
                                baseSkillBonus =
                                    multiplier * (currItemSkillLevel & 0x3F);
                                return armsMasterBonus + baseSkillBonus;
                                break;
                            case PLAYER_SKILL_AXE:
                                multiplier = GetMultiplierForSkillLevel(
                                    PLAYER_SKILL_AXE, 0, 0, 1, 1);
                                baseSkillBonus =
                                    multiplier * (currItemSkillLevel & 0x3F);
                                return armsMasterBonus + baseSkillBonus;
                                break;
                        }
                    }
                }
            }
            return 0;
            break;
        default:
            return 0;
    }
}

unsigned int Player::GetMultiplierForSkillLevel(
    PLAYER_SKILL_TYPE uSkillType, int mult1, int mult2, int mult3,
    int mult4) {  // ?? needs changing - check behavious
    int masteryLvl = GetActualSkillMastery(uSkillType);
    switch (masteryLvl) {
        case 1:
            return mult1;
        case 2:
            return mult2;
        case 3:
            return mult3;
        case 4:
            return mult4;
    }
    Error("(%u)", masteryLvl);
    return 0;
}
//----- (00490109) --------------------------------------------------------
// faces are:  0  1  2  3   human males
//             4  5  6  7   human females
//                   8  9   elf males
//                  10 11   elf females
//                  12 13   dwarf males
//                  14 15   dwarf females
//                  16 17   goblin males
//                  18 19   goblin females
//                     20   lich male
//                     21   lich female
//                     22   underwater suits (unused)
//                     23   zombie male
//                     24   zombie female
enum CHARACTER_RACE Player::GetRace() const {
    if (uCurrentFace <= 7) {
        return CHARACTER_RACE_HUMAN;
    } else if (uCurrentFace <= 11) {
        return CHARACTER_RACE_ELF;
    } else if (uCurrentFace <= 15) {
        return CHARACTER_RACE_DWARF;
    } else if (uCurrentFace <= 19) {
        return CHARACTER_RACE_GOBLIN;
    } else {
        return CHARACTER_RACE_HUMAN;
    }
}

String Player::GetRaceName() const {
    switch (GetRace()) {
        case 0:
            return localization->GetString(LSTR_RACE_HUMAN);
        case 1:
            return localization->GetString(LSTR_RACE_DWARF);
        case 2:
            return localization->GetString(LSTR_RACE_GOBLIN);
        case 3:
            return localization->GetString(LSTR_RACE_ELF);
    }
}

//----- (00490141) --------------------------------------------------------
PLAYER_SEX Player::GetSexByVoice() {
    switch (this->uVoiceID) {
        case 0u:
        case 1u:
        case 2u:
        case 3u:
        case 8u:
        case 9u:
        case 0xCu:
        case 0xDu:
        case 0x10u:
        case 0x11u:
        case 0x14u:
        case 0x17u:
            return SEX_MALE;

        case 4u:
        case 5u:
        case 6u:
        case 7u:
        case 0xAu:
        case 0xBu:
        case 0xEu:
        case 0xFu:
        case 0x12u:
        case 0x13u:
        case 0x15u:
        case 0x18u:
            return SEX_FEMALE;
    }
    Error("(%u)", this->uVoiceID);
    return SEX_MALE;
}

//----- (00490188) --------------------------------------------------------
void Player::SetInitialStats() {
    CHARACTER_RACE race = GetRace();
    uMight = StatTable[race][0].uBaseValue;
    uIntelligence = StatTable[race][1].uBaseValue;
    uWillpower = StatTable[race][2].uBaseValue;
    uEndurance = StatTable[race][3].uBaseValue;
    uAccuracy = StatTable[race][4].uBaseValue;
    uSpeed = StatTable[race][5].uBaseValue;
    uLuck = StatTable[race][6].uBaseValue;
}

//----- (004901FC) --------------------------------------------------------
void Player::SetSexByVoice() {
    switch (this->uVoiceID) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 0xC:
        case 0xD:
        case 0x10:
        case 0x11:
        case 0x14:
        case 0x17:
            this->uSex = SEX_MALE;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 0xA:
        case 0xB:
        case 0xE:
        case 0xF:
        case 0x12:
        case 0x13:
        case 0x15:
        case 0x18:
            this->uSex = SEX_FEMALE;
            break;
        default:
            Error("(%u)", this->uVoiceID);
            break;
    }
}

//----- (0049024A) --------------------------------------------------------
void Player::Reset(PLAYER_CLASS_TYPE cls) {
    sLevelModifier = 0;
    sAgeModifier = 0;

    classType = cls;
    uLuckBonus = 0;
    uSpeedBonus = 0;
    uAccuracyBonus = 0;
    uEnduranceBonus = 0;
    uWillpowerBonus = 0;
    uIntelligenceBonus = 0;
    uMightBonus = 0;
    uLevel = 1;
    uExperience = 251 + rand() % 100;
    uSkillPoints = 0;
    uBirthYear = 1147 - rand() % 6;
    pActiveSkills.fill(0);
    memset(_achieved_awards_bits, 0, sizeof(_achieved_awards_bits));
    memset(&spellbook, 0, sizeof(spellbook));

    for (uint i = 0; i < 37; ++i) {
        if (pSkillAvailabilityPerClass[classType / 4][i] != 2) continue;
        pActiveSkills[i] = 1;
    }

    memset(&pEquipment, 0, sizeof(PlayerEquipment));
    pInventoryMatrix.fill(0);
    for (uint i = 0; i < 126; ++i) pInventoryItemList[i].Reset();
    for (uint i = 0; i < 12; ++i) pEquippedItems[i].Reset();

    sHealth = GetMaxHealth();
    sMana = GetMaxMana();
}

//----- (004903C9) --------------------------------------------------------
PLAYER_SKILL_TYPE Player::GetSkillIdxByOrder(signed int order) {
    int counter;  // edx@5
    bool canBeInactive;
    unsigned char requiredValue;
    signed int offset;

    if (order <= 1) {
        canBeInactive = false;
        requiredValue = 2;  // 2 - primary skill
        offset = 0;
    } else if (order <= 3) {
        canBeInactive = false;
        requiredValue = 1;  // 1 - available
        offset = 2;
    } else if (order <= 12) {
        canBeInactive = true;
        requiredValue = 1;  // 1 - available
        offset = 4;
    } else {
        return (PLAYER_SKILL_TYPE)37;
    }
    counter = 0;
    for (int i = 0; i < 37; i++) {
        if ((this->pActiveSkills[i] || canBeInactive) &&
            pSkillAvailabilityPerClass[classType / 4][i] == requiredValue) {
            if (counter == order - offset) return (PLAYER_SKILL_TYPE)i;
            ++counter;
        }
    }

    return (PLAYER_SKILL_TYPE)37;
}

//----- (0049048D) --------------------------------------------------------
// unsigned __int16 PartyCreation_BtnMinusClick(Player *_this, int eAttribute)
void Player::DecreaseAttribute(int eAttribute) {
    int pBaseValue;    // ecx@1
    int pDroppedStep;  // ebx@1
    int pStep;         // esi@1
    int uMinValue;     // [sp+Ch] [bp-4h]@1

    int raceId = GetRace();
    pBaseValue = StatTable[raceId][eAttribute].uBaseValue;
    pDroppedStep = StatTable[raceId][eAttribute].uDroppedStep;
    uMinValue = pBaseValue - 2;
    pStep = StatTable[raceId][eAttribute].uBaseStep;
    unsigned short* AttrToChange = nullptr;
    switch (eAttribute) {
        case CHARACTER_ATTRIBUTE_STRENGTH:
            AttrToChange = &this->uMight;
            break;
        case CHARACTER_ATTRIBUTE_INTELLIGENCE:
            AttrToChange = &this->uIntelligence;
            break;
        case CHARACTER_ATTRIBUTE_WILLPOWER:
            AttrToChange = &this->uWillpower;
            break;
        case CHARACTER_ATTRIBUTE_ENDURANCE:
            AttrToChange = &this->uEndurance;
            break;
        case CHARACTER_ATTRIBUTE_ACCURACY:
            AttrToChange = &this->uAccuracy;
            break;
        case CHARACTER_ATTRIBUTE_SPEED:
            AttrToChange = &this->uSpeed;
            break;
        case CHARACTER_ATTRIBUTE_LUCK:
            AttrToChange = &this->uLuck;
            break;
    }
    if (*AttrToChange <= pBaseValue) pStep = pDroppedStep;
    if (*AttrToChange - pStep >= uMinValue) *AttrToChange -= pStep;
}

//----- (004905F5) --------------------------------------------------------
// signed int  PartyCreation_BtnPlusClick(Player *this, int eAttribute)
void Player::IncreaseAttribute(int eAttribute) {
    int raceId;              // eax@1
    int maxValue;            // ebx@1
    signed int baseStep;     // edi@1
    signed int tmp;          // eax@17
    signed int result;       // eax@18
    int baseValue;           // [sp+Ch] [bp-8h]@1
    signed int droppedStep;  // [sp+10h] [bp-4h]@1
    unsigned short* statToChange;

    raceId = GetRace();
    maxValue = StatTable[raceId][eAttribute].uMaxValue;
    baseStep = StatTable[raceId][eAttribute].uBaseStep;
    baseValue = StatTable[raceId][eAttribute].uBaseValue;
    droppedStep = StatTable[raceId][eAttribute].uDroppedStep;
    PlayerCreation_GetUnspentAttributePointCount();
    switch (eAttribute) {
        case 0:
            statToChange = &this->uMight;
            break;
        case 1:
            statToChange = &this->uIntelligence;
            break;
        case 2:
            statToChange = &this->uWillpower;
            break;
        case 3:
            statToChange = &this->uEndurance;
            break;
        case 4:
            statToChange = &this->uAccuracy;
            break;
        case 5:
            statToChange = &this->uSpeed;
            break;
        case 6:
            statToChange = &this->uLuck;
            break;
        default:
            Error("(%u)", eAttribute);
            break;
    }
    if (*statToChange < baseValue) {
        tmp = baseStep;
        baseStep = droppedStep;
        droppedStep = tmp;
    }
    result = PlayerCreation_GetUnspentAttributePointCount();
    if (result >= droppedStep) {
        if (baseStep + *statToChange <= maxValue) *statToChange += baseStep;
    }
}

//----- (0049070F) --------------------------------------------------------
void Player::Zero() {
    this->sLevelModifier = 0;
    this->sACModifier = 0;
    this->uLuckBonus = 0;
    this->uAccuracyBonus = 0;
    this->uSpeedBonus = 0;
    this->uEnduranceBonus = 0;
    this->uWillpowerBonus = 0;
    this->uIntelligenceBonus = 0;
    this->uMightBonus = 0;
    this->field_100 = 0;
    this->field_FC = 0;
    this->field_F8 = 0;
    this->field_F4 = 0;
    this->field_F0 = 0;
    this->field_EC = 0;
    this->field_E8 = 0;
    this->field_E4 = 0;
    this->field_E0 = 0;
    memset(&this->sResFireBonus, 0, 0x16u);
    this->field_1A97 = 0;
    this->_ranged_dmg_bonus = 0;
    this->field_1A95 = 0;
    this->_ranged_atk_bonus = 0;
    this->field_1A93 = 0;
    this->_melee_dmg_bonus = 0;
    this->field_1A91 = 0;
    this->_some_attack_bonus = 0;
    this->_mana_related = 0;
    this->uFullManaBonus = 0;
    this->_health_related = 0;
    this->uFullHealthBonus = 0;
}

//----- (004907E7) --------------------------------------------------------
unsigned int Player::GetStatColor(int uStat) {
    int attribute_value;  // edx@1

    int base_attribute_value = StatTable[GetRace()][uStat].uBaseValue;
    switch (uStat) {
        case 0:
            attribute_value = uMight;
            break;
        case 1:
            attribute_value = uIntelligence;
            break;
        case 2:
            attribute_value = uWillpower;
            break;
        case 3:
            attribute_value = uEndurance;
            break;
        case 4:
            attribute_value = uAccuracy;
            break;
        case 5:
            attribute_value = uSpeed;
            break;
        case 6:
            attribute_value = uLuck;
            break;
        default:
            Error("Unexpected attribute");
    }

    if (attribute_value == base_attribute_value)
        return ui_character_stat_default_color;
    else if (attribute_value > base_attribute_value)
        return ui_character_stat_buffed_color;
    else
        return ui_character_stat_debuffed_color;
}

//----- (004908A8) --------------------------------------------------------
bool Player::DiscardConditionIfLastsLongerThan(unsigned int uCondition,
                                               GameTime time) {
    if (conditions_times[uCondition] && (time < conditions_times[uCondition])) {
        conditions_times[uCondition].Reset();
        return true;
    } else {
        return false;
    }
}

//----- (004680ED) --------------------------------------------------------
void Player::UseItem_DrinkPotion_etc(signed int player_num, int a3) {
    Player* playerAffected;  // esi@1
    signed int v5;           // eax@17
    int v8;                  // edx@39
    // const char *v13; // eax@45
    signed int v15;        // edi@68
    int v16;               // edx@73
    unsigned __int16 v17;  // edi@73
    unsigned int v18;      // eax@73
                           //    const char *v22; // eax@84
    int scroll_id;         // esi@96
    int v25;               // eax@109
    int v26;               // eax@113
    int new_mana_val;      // edi@114
    __int64 v30;           // edi@137
    __int64 v32;           // ST3C_4@137
    __int64 v34;           // ST34_4@137
    unsigned __int16 v50;  // [sp-Ch] [bp-38h]@120
    signed int v67;        // [sp-4h] [bp-30h]@77
    char v72;              // [sp+20h] [bp-Ch]@68
    signed int v73;        // [sp+24h] [bp-8h]@1
    const char* v74;       // [sp+24h] [bp-8h]@23
    // Player *thisb; // [sp+28h] [bp-4h]@1
    unsigned int thisa;  // [sp+28h] [bp-4h]@22

    // thisb = this;
    playerAffected = &pParty->pPlayers[player_num - 1];
    v73 = 1;
    if (pParty->bTurnBasedModeOn &&
        (pTurnEngine->turn_stage == TE_WAIT ||
         pTurnEngine->turn_stage == TE_MOVEMENT))
        return;
    if (pParty->pPickedItem.GetItemEquipType() == EQUIP_REAGENT) {
        if (pParty->pPickedItem.uItemID == ITEM_161) {
            playerAffected->SetCondition(Condition_Poison_Weak, 1);
        } else if (pParty->pPickedItem.uItemID == ITEM_161) {
            new_mana_val = playerAffected->sMana;
            new_mana_val += 2;
            if (new_mana_val > playerAffected->GetMaxMana())
                new_mana_val = playerAffected->GetMaxMana();
            playerAffected->PlaySound(SPEECH_36, 0);
        } else if (pParty->pPickedItem.uItemID == ITEM_162) {
            playerAffected->Heal(2);
            playerAffected->PlaySound(SPEECH_36, 0);
        } else {
            GameUI_StatusBar_OnEvent(localization->FormatString(36, pParty->pPickedItem.GetDisplayName().c_str()));  // %s can not be used that way

            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }
        pAudioPlayer->PlaySound(SOUND_eat, 0, 0, -1, 0, 0);

        if (pGUIWindow_CurrentMenu &&
            pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
        }
        if (v73) {
            if (pParty->bTurnBasedModeOn) {
                pParty->pTurnBasedPlayerRecoveryTimes[player_num - 1] = 100;
                this->SetRecoveryTime(100);
                pTurnEngine->ApplyPlayerAction();
            } else {
                this->SetRecoveryTime((int)(flt_6BE3A4_debug_recmod1 * 213.3333333333333));
            }
        }
        mouse->RemoveHoldingItem();
        return;
    }

    if (pParty->pPickedItem.GetItemEquipType() == EQUIP_POTION) {
        switch (pParty->pPickedItem.uItemID) {
            case 221:  // Catalyst
                playerAffected->SetCondition(Condition_Poison_Weak, 1);
                break;

            case 222:  // Cure Wounds
                v25 = pParty->pPickedItem.uEnchantmentType + 10;
                playerAffected->Heal(v25);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 223:  // Magic Potion
                v26 = pParty->pPickedItem.uEnchantmentType + 10;
                new_mana_val = playerAffected->sMana;
                new_mana_val += v26;
                if (new_mana_val > playerAffected->GetMaxMana())
                    new_mana_val = playerAffected->GetMaxMana();
                playerAffected->PlaySound(SPEECH_36, 0);
                playerAffected->sMana = new_mana_val;
                break;

            case 224:  // Cure Weakness
                playerAffected->conditions_times[Condition_Weak].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 225:  // Cure Disease
                playerAffected->conditions_times[Condition_Disease_Severe]
                    .Reset();
                playerAffected->conditions_times[Condition_Disease_Medium]
                    .Reset();
                playerAffected->conditions_times[Condition_Disease_Weak]
                    .Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 226:  // Cure Poison
                playerAffected->conditions_times[Condition_Poison_Severe]
                    .Reset();
                playerAffected->conditions_times[Condition_Poison_Medium]
                    .Reset();
                playerAffected->conditions_times[Condition_Poison_Weak].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 227:  // Awaken
                playerAffected->conditions_times[Condition_Sleep].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 228:  // Haste
                if (!playerAffected->conditions_times[Condition_Weak]) {
                    auto duration =
                        GameTime(0, TIME_SECONDS_PER_QUANT *
                                        pParty->pPickedItem.uEnchantmentType);
                    playerAffected->pPlayerBuffs[PLAYER_BUFF_HASTE].Apply(
                        pParty->GetPlayingTime() + duration, 3, 5, 0, 0);
                    playerAffected->PlaySound(SPEECH_36, 0);
                }
                break;

            case 229:  // Heroism
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_HEROISM].Apply(
                    pParty->GetPlayingTime() + duration, 3, 5, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 230:  // Bless
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_BLESS].Apply(
                    pParty->GetPlayingTime() + duration, 3, 5, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 231:  // Preservation
            {
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 232:  // Shield
            {
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_SHIELD].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 234:  // Stoneskin
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Apply(
                    pParty->GetPlayingTime() + duration, 3, 5, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 235:  // Water Breathing
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_WATER_WALK].Apply(
                    pParty->GetPlayingTime() + duration, 3, 5, 0, 0);
                break;
            }

            case 237:  // Remove Fear
                playerAffected->conditions_times[Condition_Fear].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 238:  // Remove Curse
                playerAffected->conditions_times[Condition_Cursed].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 239:  // Cure Insanity
                playerAffected->conditions_times[Condition_Insane].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 240:  // Might Boost
            {
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                playerAffected->pPlayerBuffs[PLAYER_BUFF_STRENGTH].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 241:  // Intellect Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_INTELLIGENCE].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 242:  // Personality Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_WILLPOWER].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 243:  // Endurance Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_ENDURANCE].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 244:  // Speed Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_SPEED].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 245:  // Accuracy Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_ACCURACY].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 251:  // Cure Paralysis
                playerAffected->conditions_times[Condition_Paralyzed].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 252:  // Divine Restoration
                v30 = playerAffected->conditions_times[Condition_Dead];
                v32 = playerAffected->conditions_times[Condition_Pertified];
                v34 = playerAffected->conditions_times[Condition_Eradicated];
                conditions_times.fill(GameTime(0));
                playerAffected->conditions_times[Condition_Dead] = GameTime(v30);
                playerAffected->conditions_times[Condition_Pertified] = GameTime(v32);
                playerAffected->conditions_times[Condition_Eradicated] = GameTime(v34);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 253:  // Divine Cure
                v25 = 5 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->Heal(v25);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 254:  // Divine Power
                v26 = 5 * pParty->pPickedItem.uEnchantmentType;
                new_mana_val = playerAffected->sMana;
                new_mana_val += v26;
                if (new_mana_val > playerAffected->GetMaxMana())
                    new_mana_val = playerAffected->GetMaxMana();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 255:  // Luck Boost
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_LUCK].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 256:  // Fire Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_FIRE].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 257:  // Air Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_AIR].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 258:  // Water Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_WATER].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 259:  // Earth Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_EARTH].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 260:  // Mind Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_MIND].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 261:  // Body Resistance
            {
                auto duration =
                    GameTime(0, TIME_SECONDS_PER_QUANT *
                                    pParty->pPickedItem.uEnchantmentType);
                v50 = 3 * pParty->pPickedItem.uEnchantmentType;
                playerAffected->pPlayerBuffs[PLAYER_BUFF_RESIST_BODY].Apply(
                    pParty->GetPlayingTime() + duration, 0, v50, 0, 0);
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            }

            case 262:  // Stone to Flesh
                playerAffected->conditions_times[Condition_Pertified].Reset();
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            case 264:  // Pure Luck
                if (!playerAffected->pure_luck_used) {
                    playerAffected->uLuck += 50;
                    playerAffected->pure_luck_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 265:  // Pure Speed
                if (!playerAffected->pure_speed_used) {
                    playerAffected->uSpeed += 50;
                    playerAffected->pure_speed_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 266:  // Pure Intellect
                if (!playerAffected->pure_intellect_used) {
                    playerAffected->uIntelligence += 50;
                    playerAffected->pure_intellect_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 267:  // Pure Endurance
                if (!playerAffected->pure_endurance_used) {
                    playerAffected->uEndurance += 50;
                    playerAffected->pure_endurance_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 268:  // Pure Personality
                if (!playerAffected->pure_willpower_used) {
                    playerAffected->uWillpower += 50;
                    playerAffected->pure_willpower_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 269:  // Pure Accuracy
                if (!playerAffected->pure_accuracy_used) {
                    playerAffected->uAccuracy += 50;
                    playerAffected->pure_accuracy_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 270:  // Pure Might
                if (!playerAffected->pure_might_used) {
                    playerAffected->uMight += 50;
                    playerAffected->pure_might_used = 1;
                }
                playerAffected->PlaySound(SPEECH_36, 0);
                break;
            case 271:  // Rejuvenation
                playerAffected->sAgeModifier = 0;
                playerAffected->PlaySound(SPEECH_36, 0);
                break;

            default:
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    36, pParty->pPickedItem.GetDisplayName().c_str()));  // %s can not be used that way

                pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                return;
        }
        pAudioPlayer->PlaySound(SOUND_drink, 0, 0, -1, 0, 0);
        if (pGUIWindow_CurrentMenu &&
            pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
            //         if ( !v73 ) v73 is always 1 at this point
            //         {
            //           pMouse->RemoveHoldingItem();
            //           return;
            //         }
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
        }
        if (v73) {
            if (pParty->bTurnBasedModeOn) {
                pParty->pTurnBasedPlayerRecoveryTimes[player_num - 1] = 100;
                this->SetRecoveryTime(100);
                pTurnEngine->ApplyPlayerAction();
            } else {
                this->SetRecoveryTime((int)(flt_6BE3A4_debug_recmod1 * 213.3333333333333));
            }
        }
        mouse->RemoveHoldingItem();
        return;
    }

    if (pParty->pPickedItem.GetItemEquipType() == EQUIP_SPELL_SCROLL) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_CASTING) return;
        if (!playerAffected->CanAct()) {
            auto str = localization->FormatString(
                382, localization->GetCharacterConditionName(
                         playerAffected->GetMajorConditionIdx()));
            GameUI_StatusBar_OnEvent(str);

            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }
        if (engine->IsUnderwater()) {
            GameUI_StatusBar_OnEvent(localization->GetString(652));  // You can not do that while you are underwater!
            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }

        scroll_id = pParty->pPickedItem.uItemID - 299;
        if (scroll_id == 30 || scroll_id == 4 || scroll_id == 91 ||
            scroll_id == 28) {  // Enchant Item scroll, Vampiric Weapon scroll
                                // ,Recharge Item ,Fire Aura
            mouse->RemoveHoldingItem();
            pGUIWindow_CurrentMenu->Release();
            current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
            viewparams->bRedrawGameUI = 1;
            _42777D_CastSpell_UseWand_ShootArrow((SPELL_TYPE)scroll_id, player_num - 1, 0x85u, 1, 0);
        } else {
            mouse->RemoveHoldingItem();
            pMessageQueue_50C9E8->AddGUIMessage(UIMSG_SpellScrollUse, scroll_id, player_num - 1);
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME && pGUIWindow_CurrentMenu &&
                (pGUIWindow_CurrentMenu->eWindowType != WINDOW_null)) {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
            }
        }
        return;
    }

    if (pParty->pPickedItem.GetItemEquipType() == EQUIP_BOOK) {
        v15 = pParty->pPickedItem.uItemID - 400;
        v72 = playerAffected->spellbook.bHaveSpell[pParty->pPickedItem.uItemID - 400];
        // (char *)&v3->pConditions[0] +
        // pParty->pPickedItem.uItemID + 2;
        if (v72) {
            auto str = localization->FormatString(
                380, pParty->pPickedItem.GetDisplayName().c_str());  // You already know the %s spell
            GameUI_StatusBar_OnEvent(str);

            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }
        if (!playerAffected->CanAct()) {
            auto str = localization->FormatString(
                382,  // That player is %s
                localization->GetCharacterConditionName(
                    playerAffected->GetMajorConditionIdx()));
            GameUI_StatusBar_OnEvent(str);

            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }
        v16 = v15 % 11 + 1;
        v17 = playerAffected->pActiveSkills[v15 / 11 + 12];
        v18 = SkillToMastery(v17) - 1;
        switch (v18) {
            case 0:
                v67 = 4;
                break;
            case 1:
                v67 = 7;
                break;
            case 2:
                v67 = 10;
                break;
            case 3:
                v67 = 11;
                break;
            default:
                v67 = player_num;
        }

        if (v16 > v67 || !v17) {
            auto str = localization->FormatString(
                381, pParty->pPickedItem.GetDisplayName()
                         .c_str());  // You don't have the skill to learn %s
            GameUI_StatusBar_OnEvent(str);

            playerAffected->PlaySound((PlayerSpeech)20, 0);
            return;
        }
        playerAffected->spellbook
            .bHaveSpell[pParty->pPickedItem.uItemID - 400] = 1;
        playerAffected->PlaySound(SPEECH_21, 0);
        v73 = 0;

        if (pGUIWindow_CurrentMenu &&
            pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
            if (!v73) {
                mouse->RemoveHoldingItem();
                return;
            }
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
        }
        //       if ( v73 )                                                v73
        //       is always 0 at this point
        //       {
        //         if ( pParty->bTurnBasedModeOn )
        //         {
        //           pParty->pTurnBasedPlayerRecoveryTimes[player_num-1] = 100;
        //           thisb->SetRecoveryTime(100);
        //           pTurnEngine->ApplyPlayerAction();
        //         }
        //         else
        //         {
        //           thisb->SetRecoveryTime(flt_6BE3A4_debug_recmod1 * 213.3333333333333);
        //         }
        //       }
        mouse->RemoveHoldingItem();
        return;
    }

    if (pParty->pPickedItem.GetItemEquipType() == EQUIP_MESSAGE_SCROLL) {
        if (playerAffected->CanAct()) {
            CreateMsgScrollWindow(pParty->pPickedItem.uItemID);
            playerAffected->PlaySound(SPEECH_37, 0);
            return;
        }

        auto str = localization->FormatString(
            382,  // That player is %s
            localization->GetCharacterConditionName(
                playerAffected->GetMajorConditionIdx()));
        GameUI_StatusBar_OnEvent(str);

        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        return;
    } else {
        if (pParty->pPickedItem.uItemID == ITEM_GENIE_LAMP) {
            thisa = pParty->uCurrentMonthWeek + 1;
            if (pParty->uCurrentMonth >= 7)
                v74 = nullptr;
            else
                v74 = localization->GetAttirubteName(pParty->uCurrentMonth);

            String status;
            switch (pParty->uCurrentMonth) {
                case 0:
                    playerAffected->uMight += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 1:
                    playerAffected->uIntelligence += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 2:
                    playerAffected->uWillpower += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 3:
                    playerAffected->uEndurance += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 4:
                    playerAffected->uAccuracy += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 5:
                    playerAffected->uSpeed += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 6:
                    playerAffected->uLuck += thisa;
                    status = StringPrintf(
                        "+%u %s %s", thisa, v74,
                        localization->GetString(121));  // "Permanent"
                    break;
                case 7:
                    pParty->PartyFindsGold(1000 * thisa, 0);
                    status =
                        StringPrintf("+%u %s", 1000 * thisa,
                                     localization->GetString(97));  // "Gold"
                    break;
                case 8:
                    Party::GiveFood(5 * thisa);
                    status =
                        StringPrintf("+%u %s", 5 * thisa,
                                     localization->GetString(653));  // "Food"
                    break;
                case 9u:
                    playerAffected->uSkillPoints += 2 * thisa;
                    status = StringPrintf(
                        "+%u %s", 2 * thisa,
                        localization->GetString(207));  // Skill Points
                    break;
                case 10:
                    playerAffected->uExperience += 2500 * thisa;
                    status = StringPrintf(
                        "+%u %s", 2500 * thisa,
                        localization->GetString(83));  // Experience
                    break;
                case 11: {
                    v8 = rand() % 6;

                    auto spell_school_name =
                        localization->GetSpellSchoolName(v8 == 5 ? v8 + 1 : v8);

                    switch (v8) {
                        case 0:  // Fire
                            playerAffected->sResFireBase += thisa;
                            break;
                        case 1:  // Air
                            playerAffected->sResAirBase += thisa;
                            break;
                        case 2:  // Water
                            playerAffected->sResWaterBase += thisa;
                            break;
                        case 3:  // Earth
                            playerAffected->sResEarthBase += thisa;
                            break;
                        case 4:  // Mind
                            playerAffected->sResMindBase += thisa;
                            break;
                        case 5:  // Body
                            playerAffected->sResBodyBase += thisa;
                            break;
                        default:
                            ("Unexpected attribute");
                            return;
                    }
                    status = StringPrintf(
                        "+%u %s %s", thisa, spell_school_name,
                        localization->GetString(121));  // Permanent
                    break;
                }
            }
            GameUI_StatusBar_OnEvent(status);

            mouse->RemoveHoldingItem();
            spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, player_num - 1);
            playerAffected->PlaySound(SPEECH_93, 0);
            pAudioPlayer->PlaySound(SOUND_chimes, 0, 0, -1, 0, 0);
            if (pParty->uCurrentDayOfMonth == 6 ||
                pParty->uCurrentDayOfMonth == 20) {
                playerAffected->SetCondition(Condition_Eradicated, 0);
                pAudioPlayer->PlaySound(SOUND_gong, 0, 0, -1, 0, 0);
            } else if (pParty->uCurrentDayOfMonth == 12 ||
                       pParty->uCurrentDayOfMonth == 26) {
                playerAffected->SetCondition(Condition_Dead, 0);
                pAudioPlayer->PlaySound(SOUND_gong, 0, 0, -1, 0, 0);
            } else if (pParty->uCurrentDayOfMonth == 4 ||
                       pParty->uCurrentDayOfMonth == 25) {
                playerAffected->SetCondition(Condition_Pertified, 0);
                pAudioPlayer->PlaySound(SOUND_gong, 0, 0, -1, 0, 0);
            }
            return;
        } else if (pParty->pPickedItem.uItemID == ITEM_RED_APPLE) {
            Party::GiveFood(1);
            pAudioPlayer->PlaySound(SOUND_eat, 0, 0, -1, 0, 0);
        } else if (pParty->pPickedItem.uItemID == ITEM_LUTE) {
            pAudioPlayer->PlaySound(SOUND_luteguitar, 0, 0, -1, 0, 0);
            return;
        } else if (pParty->pPickedItem.uItemID == ITEM_FAERIE_PIPES) {
            pAudioPlayer->PlaySound(SOUND_panflute, 0, 0, -1, 0, 0);
            return;
        } else if (pParty->pPickedItem.uItemID == ITEM_GRYPHONHEARTS_TRUMPET) {
            pAudioPlayer->PlaySound(SOUND_trumpet, 0, 0, -1, 0, 0);
            return;
        } else if (pParty->pPickedItem.uItemID == ITEM_HORSESHOE) {
            spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, player_num - 1);
            v5 = PID(OBJECT_Player, player_num + 49);
            pAudioPlayer->PlaySound(SOUND_quest, v5, 0, -1, 0, 0);
            playerAffected->AddVariable(VAR_NumSkillPoints, 2);
        } else if (pParty->pPickedItem.uItemID == ITEM_TEMPLE_IN_A_BOTTLE) {
            TeleportToNWCDungeon();
            return;
        } else {
            auto str = localization->FormatString(
                36, pParty->pPickedItem.GetDisplayName().c_str());  // %s can not be used that way
            GameUI_StatusBar_OnEvent(str);

            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
            return;
        }

        mouse->RemoveHoldingItem();
        return;
    }
}

bool CmpSkillValue(int valToCompare, int skillValue) {
    int v4;
    if (valToCompare <= 63)
        v4 = skillValue & 0x3F;
    else
        v4 = skillValue & skillValue;
    return v4 >= valToCompare;
}

//----- (00449BB4) --------------------------------------------------------
bool Player::CompareVariable(enum VariableType VarNum, int pValue) {
    // in some cases this calls only calls v4 >= pValue, which i've
    // changed to return false, since these values are supposed to
    // be positive and v4 was -1 by default
    Assert(pValue >= 0, "Compare variable shouldn't have negative arguments");

    signed int v4;                         // edi@1
    unsigned __int8 test_bit_value;        // eax@25
    unsigned __int8 byteWithRequestedBit;  // cl@25
    DDM_DLV_Header* v19;                   // eax@122
    DDM_DLV_Header* v21;                   // eax@126
    int actStat;                           // ebx@161
    int baseStat;                          // eax@161

    if ((signed int)VarNum >= VAR_MapPersistentVariable_0 &&
        VarNum <= VAR_MapPersistentVariable_74)
        return (unsigned __int8)stru_5E4C90_MapPersistVars
                   .field_0[VarNum - VAR_MapPersistentVariable_0] >
               0;  // originally (unsigned __int8)byte_5E4C15[VarNum];
    if ((signed int)VarNum >= VAR_MapPersistentVariable_75 &&
        VarNum <= VAR_MapPersistentVariable_99)
        return (unsigned __int8)stru_5E4C90_MapPersistVars
                   ._decor_events[VarNum - VAR_MapPersistentVariable_75] >
               0;  // not really sure whether the number gets up to 99, but
                   // can't ignore the possibility

    switch (VarNum) {
        case VAR_Sex:
            return (pValue == this->uSex);
        case VAR_Class:
            return (pValue == this->classType);
        case VAR_Race:
            return pValue == GetRace();
        case VAR_CurrentHP:
            return this->sHealth >= pValue;
        case VAR_MaxHP:
            return (this->sHealth >= GetMaxHealth());
        case VAR_CurrentSP:
            return this->sMana >= pValue;
        case VAR_MaxSP:
            return (this->sMana >= GetMaxMana());
        case VAR_ActualAC:
            return GetActualAC() >= pValue;
        case VAR_ACModifier:
            return this->sACModifier >= pValue;
        case VAR_BaseLevel:
            return this->uLevel >= pValue;
        case VAR_LevelModifier:
            return this->sLevelModifier >= pValue;
        case VAR_Age:
            return GetActualAge() >= (unsigned int)pValue;
        case VAR_Award:
            return _449B57_test_bit(this->_achieved_awards_bits, pValue);
        case VAR_Experience:
            return this->uExperience >=
                   pValue;  // TODO(_) change pValue to long long
        case VAR_QBits_QuestsDone:
            return _449B57_test_bit(pParty->_quest_bits, pValue);
        case VAR_PlayerItemInHands:
            // for (int i = 0; i < 138; i++)
            for (int i = 0; i < 126; i++) {
                if (pInventoryItemList[i].uItemID == pValue) {
                    return true;
                }
            }
            return pParty->pPickedItem.uItemID == pValue;

        case VAR_Hour:
            return pParty->GetPlayingTime().GetHoursOfDay() == pValue;

        case VAR_DayOfYear:
            return pParty->GetPlayingTime().GetDays() % 336 + 1 == pValue;

        case VAR_DayOfWeek:
            return pParty->GetPlayingTime().GetDays() % 7 == pValue;

        case VAR_FixedGold:
            return pParty->GetGold() >= pValue;
        case VAR_FixedFood:
            return pParty->GetFood() >= pValue;
        case VAR_MightBonus:
            return this->uMightBonus >= pValue;
        case VAR_IntellectBonus:
            return this->uIntelligenceBonus >= pValue;
        case VAR_PersonalityBonus:
            return this->uWillpowerBonus >= pValue;
        case VAR_EnduranceBonus:
            return this->uEnduranceBonus >= pValue;
        case VAR_SpeedBonus:
            return this->uSpeedBonus >= pValue;
        case VAR_AccuracyBonus:
            return this->uAccuracyBonus >= pValue;
        case VAR_LuckBonus:
            return this->uLuckBonus >= pValue;
        case VAR_BaseMight:
            return this->uMight >= pValue;
        case VAR_BaseIntellect:
            return this->uIntelligence >= pValue;
        case VAR_BasePersonality:
            return this->uWillpower >= pValue;
        case VAR_BaseEndurance:
            return this->uEndurance >= pValue;
        case VAR_BaseSpeed:
            return this->uSpeed >= pValue;
        case VAR_BaseAccuracy:
            return this->uAccuracy >= pValue;
        case VAR_BaseLuck:
            return this->uLuck >= pValue;
        case VAR_ActualMight:
            return GetActualMight() >= pValue;
        case VAR_ActualIntellect:
            return GetActualIntelligence() >= pValue;
        case VAR_ActualPersonality:
            return GetActualWillpower() >= pValue;
        case VAR_ActualEndurance:
            return GetActualEndurance() >= pValue;
        case VAR_ActualSpeed:
            return GetActualSpeed() >= pValue;
        case VAR_ActualAccuracy:
            return GetActualAccuracy() >= pValue;
        case VAR_ActualLuck:
            return GetActualLuck() >= pValue;
        case VAR_FireResistance:
            return this->sResFireBase >= pValue;
        case VAR_AirResistance:
            return this->sResAirBase >= pValue;
        case VAR_WaterResistance:
            return this->sResWaterBase >= pValue;
        case VAR_EarthResistance:
            return this->sResEarthBase >= pValue;
        case VAR_SpiritResistance:
            return this->sResSpiritBase >= pValue;
        case VAR_MindResistance:
            return this->sResMindBase >= pValue;
        case VAR_BodyResistance:
            return this->sResBodyBase >= pValue;
        case VAR_LightResistance:
            return this->sResLightBase >= pValue;
        case VAR_DarkResistance:
            return this->sResDarkBase >= pValue;
        case VAR_PhysicalResistance:
            Error("Physical resistance isn't used in events");
            return false;
        case VAR_MagicResistance:
            return this->sResMagicBase >= pValue;
        case VAR_FireResistanceBonus:
            return this->sResFireBonus >= pValue;
        case VAR_AirResistanceBonus:
            return this->sResAirBonus >= pValue;
        case VAR_WaterResistanceBonus:
            return this->sResWaterBonus >= pValue;
        case VAR_EarthResistanceBonus:
            return this->sResEarthBonus >= pValue;
        case VAR_SpiritResistanceBonus:
            return this->sResSpiritBonus >= pValue;
        case VAR_MindResistanceBonus:
            return this->sResMindBonus >= pValue;
        case VAR_BodyResistanceBonus:
            return this->sResBodyBonus >= pValue;
        case VAR_LightResistanceBonus:
            return this->sResLightBonus >= pValue;
        case VAR_DarkResistanceBonus:
            return this->sResDarkBonus >= pValue;
        case VAR_MagicResistanceBonus:
            return this->sResMagicBonus >= pValue;
        case VAR_StaffSkill:
            return CmpSkillValue(pValue, this->skillStaff);
        case VAR_SwordSkill:
            return CmpSkillValue(pValue, this->skillSword);
        case VAR_DaggerSkill:
            return CmpSkillValue(pValue, this->skillDagger);
        case VAR_AxeSkill:
            return CmpSkillValue(pValue, this->skillAxe);
        case VAR_SpearSkill:
            return CmpSkillValue(pValue, this->skillSpear);
        case VAR_BowSkill:
            return CmpSkillValue(pValue, this->skillBow);
        case VAR_MaceSkill:
            return CmpSkillValue(pValue, this->skillMace);
        case VAR_BlasterSkill:
            return CmpSkillValue(pValue, this->skillBlaster);
        case VAR_ShieldSkill:
            return CmpSkillValue(pValue, this->skillShield);
        case VAR_LeatherSkill:
            return CmpSkillValue(pValue, this->skillLeather);
        case VAR_SkillChain:
            return CmpSkillValue(pValue, this->skillChain);
        case VAR_PlateSkill:
            return CmpSkillValue(pValue, this->skillPlate);
        case VAR_FireSkill:
            return CmpSkillValue(pValue, this->skillFire);
        case VAR_AirSkill:
            return CmpSkillValue(pValue, this->skillAir);
        case VAR_WaterSkill:
            return CmpSkillValue(pValue, this->skillWater);
        case VAR_EarthSkill:
            return CmpSkillValue(pValue, this->skillEarth);
        case VAR_SpiritSkill:
            return CmpSkillValue(pValue, this->skillSpirit);
        case VAR_MindSkill:
            return CmpSkillValue(pValue, this->skillMind);
        case VAR_BodySkill:
            return CmpSkillValue(pValue, this->skillBody);
        case VAR_LightSkill:
            return CmpSkillValue(pValue, this->skillLight);
        case VAR_DarkSkill:
            return CmpSkillValue(pValue, this->skillDark);
        case VAR_IdentifyItemSkill:
            return CmpSkillValue(pValue, this->skillItemId);
        case VAR_MerchantSkill:
            return CmpSkillValue(pValue, this->skillMerchant);
        case VAR_RepairSkill:
            return CmpSkillValue(pValue, this->skillRepair);
        case VAR_BodybuildingSkill:
            return CmpSkillValue(pValue, this->skillBodybuilding);
        case VAR_MeditationSkill:
            return CmpSkillValue(pValue, this->skillMeditation);
        case VAR_PerceptionSkill:
            return CmpSkillValue(pValue, this->skillPerception);
        case VAR_DiplomacySkill:
            return CmpSkillValue(pValue, this->skillDiplomacy);
        case VAR_ThieverySkill:
            Error("Thievery isn't used in events");
            return false;
        case VAR_DisarmTrapSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillDisarmTrap);
        case VAR_DodgeSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillDodge);
        case VAR_UnarmedSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillUnarmed);
        case VAR_IdentifyMonsterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillMonsterId);
        case VAR_ArmsmasterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillArmsmaster);
        case VAR_StealingSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillStealing);
        case VAR_AlchemySkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->skillAlchemy);
        case VAR_LearningSkill:
            return CmpSkillValue(pValue, this->skillLearning);
        case VAR_Cursed:
            return conditions_times[Condition_Cursed].Valid();
        case VAR_Weak:
            return conditions_times[Condition_Weak].Valid();
        case VAR_Asleep:
            return conditions_times[Condition_Sleep].Valid();
        case VAR_Afraid:
            return conditions_times[Condition_Fear].Valid();
        case VAR_Drunk:
            return conditions_times[Condition_Drunk].Valid();
        case VAR_Insane:
            return conditions_times[Condition_Insane].Valid();
        case VAR_PoisonedGreen:
            return conditions_times[Condition_Poison_Weak].Valid();
        case VAR_DiseasedGreen:
            return conditions_times[Condition_Disease_Weak].Valid();
        case VAR_PoisonedYellow:
            return conditions_times[Condition_Poison_Medium].Valid();
        case VAR_DiseasedYellow:
            return conditions_times[Condition_Disease_Medium].Valid();
        case VAR_PoisonedRed:
            return conditions_times[Condition_Poison_Severe].Valid();
        case VAR_DiseasedRed:
            return conditions_times[Condition_Disease_Severe].Valid();
        case VAR_Paralyzed:
            return conditions_times[Condition_Paralyzed].Valid();
        case VAR_Unconsious:
            return conditions_times[Condition_Unconcious].Valid();
        case VAR_Dead:
            return conditions_times[Condition_Dead].Valid();
        case VAR_Stoned:
            return conditions_times[Condition_Pertified].Valid();
        case VAR_Eradicated:
            return conditions_times[Condition_Eradicated].Valid();
        case VAR_MajorCondition:
            v4 = GetMajorConditionIdx();
            if (v4 != 18) {
                return v4 >= pValue;
            }
            return true;
        case VAR_AutoNotes:  // TODO(_): find out why the double subtraction. or
                             // whether this is even used
            test_bit_value = 0x80u >> (pValue - 2) % 8;
            byteWithRequestedBit = pParty->_autonote_bits[(pValue - 2) / 8];
            return (test_bit_value & byteWithRequestedBit) != 0;
        case VAR_IsMightMoreThanBase:
            actStat = GetActualMight();
            baseStat = GetBaseStrength();
            return (actStat >= baseStat);
        case VAR_IsIntellectMoreThanBase:
            actStat = GetActualIntelligence();
            baseStat = GetBaseIntelligence();
            return (actStat >= baseStat);
        case VAR_IsPersonalityMoreThanBase:
            actStat = GetActualWillpower();
            baseStat = GetBaseWillpower();
            return (actStat >= baseStat);
        case VAR_IsEnduranceMoreThanBase:
            actStat = GetActualEndurance();
            baseStat = GetBaseEndurance();
            return (actStat >= baseStat);
        case VAR_IsSpeedMoreThanBase:
            actStat = GetActualSpeed();
            baseStat = GetBaseSpeed();
            return (actStat >= baseStat);
        case VAR_IsAccuracyMoreThanBase:
            actStat = GetActualAccuracy();
            baseStat = GetBaseAccuracy();
            return (actStat >= baseStat);
        case VAR_IsLuckMoreThanBase:
            actStat = GetActualLuck();
            baseStat = GetBaseLuck();
            return (actStat >= baseStat);
        case VAR_PlayerBits:
            test_bit_value = 0x80u >> ((signed __int16)pValue - 1) % 8;
            byteWithRequestedBit =
                this->playerEventBits[((signed __int16)pValue - 1) / 8];
            return (test_bit_value & byteWithRequestedBit) != 0;
        case VAR_NPCs2:
            return pNPCStats->pNewNPCData[pValue].Hired();
        case VAR_IsFlying:
            if (pParty->bFlying && pParty->pPartyBuffs[PARTY_BUFF_FLY].Active())
                return true;
            return false;
        case VAR_HiredNPCHasSpeciality:
            return CheckHiredNPCSpeciality(pValue);
        case VAR_CircusPrises:  // isn't used in MM6 since 0x1D6u is a book of
                                // regeneration
            v4 = 0;
            for (int playerNum = 0; playerNum < 4; playerNum++) {
                for (int invPos = 0; invPos < 138; invPos++) {
                    int itemId = pParty->pPlayers[playerNum]
                                     .pInventoryItemList[invPos]
                                     .uItemID;
                    switch (itemId) {
                        case 0x1D6u:
                            ++v4;
                            break;
                        case 0x1D7u:
                            v4 += 3;
                            break;
                        case 0x1DDu:
                            v4 += 5;
                            break;
                    }
                }
            }
            return v4 >= pValue;
        case VAR_NumSkillPoints:
            return this->uSkillPoints >= (unsigned int)pValue;
        case VAR_MonthIs:
            return (pParty->uCurrentMonth == (unsigned int)pValue);
        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
            if (pParty->PartyTimes.CounterEventValues[VarNum - VAR_Counter1]
                    .Valid()) {
                return (pParty->PartyTimes
                            .CounterEventValues[VarNum - VAR_Counter1] +
                        GameTime::FromHours(pValue)) <=
                       pParty->GetPlayingTime();
            }
            break;

        case VAR_ReputationInCurrentLocation:
            v19 = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v19 = &pIndoor->dlv;
            return (v19->uReputation >= pValue);

        case VAR_Unknown1:
            v21 = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v21 = &pIndoor->dlv;
            return (v21->field_C_alert == pValue);  // yes, equality, not >=

        case VAR_GoldInBank:
            return pParty->uNumGoldInBank >= (unsigned int)pValue;

        case VAR_NumDeaths:
            return pParty->uNumDeaths >= (unsigned int)pValue;

        case VAR_NumBounties:
            return pParty->uNumBountiesCollected >= (unsigned int)pValue;

        case VAR_PrisonTerms:
            return pParty->uNumPrisonTerms >= pValue;
        case VAR_ArenaWinsPage:
            return (unsigned __int8)pParty->uNumArenaPageWins >= pValue;
        case VAR_ArenaWinsSquire:
            return (unsigned __int8)pParty->uNumArenaSquireWins >= pValue;
        case VAR_ArenaWinsKnight:
            return (unsigned __int8)pParty->uNumArenaKnightWins >= pValue;
        case VAR_ArenaWinsLord:
            return pParty->uNumArenaLordWins >= pValue;
        case VAR_Invisible:
            return pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active();
        case VAR_ItemEquipped:
            for (int i = 0; i < 16; i++) {
                if (HasItemEquipped((ITEM_EQUIP_TYPE)i) &&
                    GetNthEquippedIndexItem(i)->uItemID == pValue) {
                    return true;
                }
            }
            return false;
    }

    return false;
}

//----- (0044A5CB) --------------------------------------------------------
void Player::SetVariable(enum VariableType var_type, signed int var_value) {
    unsigned int v6;      // esi@13
    unsigned int v7;      // esi@14
    signed int v11;       // eax@30
    DDM_DLV_Header* v24;  // ecx@148
    ItemGen item;         // [sp+Ch] [bp-28h]@52

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[var_type - VAR_History_0]) {
            pParty->PartyTimes.HistoryEventTimes[var_type - VAR_History_0] =
                pParty->GetPlayingTime();
            if (pStorylineText->StoreLine[var_type - VAR_History_0].pText) {
                bFlashHistoryBook = 1;
                PlayAwardSound();
            }
        }
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 &&
        var_type <= VAR_MapPersistentVariable_99) {
        if (var_type >= VAR_MapPersistentVariable_0 &&
            var_type <= VAR_MapPersistentVariable_74)
            stru_5E4C90_MapPersistVars
                .field_0[var_type - VAR_MapPersistentVariable_0] =
                (char)var_value;
        if (var_type >= VAR_MapPersistentVariable_75 &&
            var_type <= VAR_MapPersistentVariable_99)
            stru_5E4C90_MapPersistVars
                ._decor_events[var_type - VAR_MapPersistentVariable_75] =
                (unsigned char)
                    var_value;  // not really sure whether the number gets up to
                                // 99, but can't ignore the possibility
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 &&
        var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[var_type - VAR_UnknownTimeEvent0] =
            pParty->GetPlayingTime();
        PlayAwardSound();
        return;
    }

    switch (var_type) {
        case VAR_Sex:
            this->uSex = (PLAYER_SEX)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_Class:
            this->classType = (PLAYER_CLASS_TYPE)var_value;
            if ((PLAYER_CLASS_TYPE)var_value == PLAYER_CLASS_LICH) {
                for (int i = 0; i < 138; i++) {
                    if (this->pOwnItems[i].uItemID == ITEM_LICH_JAR_EMPTY) {
                        this->pOwnItems[i].uItemID = ITEM_LICH_JAR_FULL;
                        this->pOwnItems[i].uHolderPlayer = GetPlayerIndex() + 1;
                    }
                }
                if (this->sResFireBase < 20) this->sResFireBase = 20;
                if (this->sResAirBase < 20) this->sResAirBase = 20;
                if (this->sResWaterBase < 20) this->sResWaterBase = 20;
                if (this->sResEarthBase < 20) this->sResEarthBase = 20;
                this->sResMindBase = 200;
                this->sResBodyBase = 200;
                v11 = this->GetSexByVoice();
                this->uPrevVoiceID = this->uVoiceID;
                this->uPrevFace = this->uCurrentFace;
                if (v11) {
                    this->uCurrentFace = 21;
                    this->uVoiceID = 21;
                } else {
                    this->uCurrentFace = 20;
                    this->uVoiceID = 20;
                }
                GameUI_ReloadPlayerPortraits(GetPlayerIndex(),
                                             this->uCurrentFace);
            }
            PlayAwardSound_Anim();
            return;
        case VAR_CurrentHP:
            this->sHealth = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_MaxHP:
            this->sHealth = GetMaxHealth();
            return;
        case VAR_CurrentSP:
            this->sMana = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_MaxSP:
            this->sMana = GetMaxMana();
            return;
        case VAR_ACModifier:
            this->sACModifier = (unsigned __int8)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_BaseLevel:
            this->uLevel = (unsigned __int8)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier = (unsigned __int8)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_Age:
            this->sAgeModifier = var_value;
            return;
        case VAR_Award:
            if (!_449B57_test_bit(this->_achieved_awards_bits, var_value) &&
                pAwards[var_value].pText) {
                PlayAwardSound_Anim();
                this->PlaySound(SPEECH_96, 0);
            }
            _449B7E_toggle_bit(this->_achieved_awards_bits, var_value, 1u);
            return;
        case VAR_Experience:
            this->uExperience = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_QBits_QuestsDone:
            if (!_449B57_test_bit(pParty->_quest_bits, var_value) &&
                pQuestTable[var_value - 1]) {
                bFlashQuestBook = 1;
                spell_fx_renderer->SetPlayerBuffAnim(0x96u, GetPlayerIndex());
                PlayAwardSound();
                this->PlaySound(SPEECH_93, 0);
            }
            _449B7E_toggle_bit(pParty->_quest_bits, var_value, 1u);
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.uItemID = var_value;
            item.uAttributes = 1;
            pParty->SetHoldingItem(&item);
            if (var_value >= ITEM_ARTIFACT_PUCK &&
                var_value <= ITEM_RELIC_MEKORIGS_HAMMER)
                pParty->pIsArtifactFound[var_value - 500] = 1;
            return;
        case VAR_FixedGold:
            Party::SetGold(var_value);
            return;
        case VAR_RandomGold:
            v6 = rand() % var_value + 1;
            Party::SetGold(v6);
            GameUI_StatusBar_OnEvent(
                localization->FormatString(500, v6));  // You have %lu gold
            GameUI_DrawFoodAndGold();
            return;
        case VAR_FixedFood:
            Party::SetFood(var_value);
            PlayAwardSound_Anim();
            return;
        case VAR_RandomFood:
            v7 = rand() % var_value + 1;
            Party::SetFood(v7);
            GameUI_StatusBar_OnEvent(
                localization->FormatString(501, v7));  // You have %lu food
            GameUI_DrawFoodAndGold();
            PlayAwardSound_Anim();
            return;
        case VAR_BaseMight:
            this->uMight = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BaseIntellect:
            this->uIntelligence = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BasePersonality:
            this->uWillpower = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BaseEndurance:
            this->uEndurance = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BaseSpeed:
            this->uSpeed = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BaseAccuracy:
            this->uAccuracy = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BaseLuck:
            this->uLuck = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->uMightBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->uIntelligenceBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->uWillpowerBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->uEnduranceBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->uSpeedBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->uAccuracyBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->uLuckBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_FireResistance:
            this->sResFireBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_AirResistance:
            this->sResAirBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_MindResistance:
            this->sResMindBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_LightResistance:
            this->sResLightBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_92);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_PhysicalResistanceBonus:
            Error("Physical res. bonus not used");
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus = (unsigned __int8)var_value;
            PlayAwardSound_Anim_Face(SPEECH_91);
            return;
        case VAR_Cursed:
            this->SetCondition(Condition_Cursed, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Weak:
            this->SetCondition(Condition_Weak, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Asleep:
            this->SetCondition(Condition_Sleep, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Afraid:
            this->SetCondition(Condition_Fear, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Drunk:
            this->SetCondition(Condition_Drunk, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Insane:
            this->SetCondition(Condition_Insane, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedGreen:
            this->SetCondition(Condition_Poison_Weak, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedGreen:
            this->SetCondition(Condition_Disease_Weak, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedYellow:
            this->SetCondition(Condition_Poison_Medium, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedYellow:
            this->SetCondition(Condition_Disease_Medium, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedRed:
            this->SetCondition(Condition_Poison_Severe, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedRed:
            this->SetCondition(Condition_Disease_Severe, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Paralyzed:
            this->SetCondition(Condition_Paralyzed, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Unconsious:
            this->SetCondition(Condition_Unconcious, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Dead:
            this->SetCondition(Condition_Dead, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Stoned:
            this->SetCondition(Condition_Pertified, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Eradicated:
            this->SetCondition(Condition_Eradicated, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_MajorCondition:
            conditions_times.fill(GameTime(0));
            PlayAwardSound_Anim();
            return;
        case VAR_AutoNotes:
            if (!_449B57_test_bit(pParty->_autonote_bits, var_value) &&
                pAutonoteTxt[var_value - 1].pText) {
                spell_fx_renderer->SetPlayerBuffAnim(0x96u, GetPlayerIndex());
                this->PlaySound(SPEECH_96, 0);
                bFlashAutonotesBook = 1;
                _506568_autonote_type = pAutonoteTxt[var_value - 1].eType;  // dword_72371C[2 * a3];
            }
            _449B7E_toggle_bit(pParty->_autonote_bits, var_value, 1u);
            PlayAwardSound();
            return;
        case VAR_PlayerBits:
            _449B7E_toggle_bit((unsigned char*)playerEventBits, var_value, 1u);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNewNPCData[var_value].uFlags |= 0x80u;
            pParty->CountHirelings();
            viewparams->bRedrawGameUI = true;
            return;
        case VAR_NumSkillPoints:
            this->uSkillPoints = var_value;
            return;

        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
            pParty->PartyTimes.CounterEventValues[var_type - VAR_Counter1] =
                pParty->GetPlayingTime();
            return;

        case VAR_ReputationInCurrentLocation:
            v24 = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v24 = &pIndoor->dlv;
            v24->uReputation = var_value;
            if (var_value > 10000) v24->uReputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank = var_value;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths = var_value;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected = var_value;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms = var_value;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaPageWins = var_value;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaSquireWins = var_value;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaKnightWins = var_value;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaLordWins = var_value;
            return;
        case VAR_StaffSkill:
            SetSkillByEvent(&Player::skillStaff, var_value);
            return;
        case VAR_SwordSkill:
            SetSkillByEvent(&Player::skillSword, var_value);
            return;
        case VAR_DaggerSkill:
            SetSkillByEvent(&Player::skillDagger, var_value);
            return;
        case VAR_AxeSkill:
            SetSkillByEvent(&Player::skillAxe, var_value);
            return;
        case VAR_SpearSkill:
            SetSkillByEvent(&Player::skillSpear, var_value);
            return;
        case VAR_BowSkill:
            SetSkillByEvent(&Player::skillBow, var_value);
            return;
        case VAR_MaceSkill:
            SetSkillByEvent(&Player::skillMace, var_value);
            return;
        case VAR_BlasterSkill:
            SetSkillByEvent(&Player::skillBlaster, var_value);
            return;
        case VAR_ShieldSkill:
            SetSkillByEvent(&Player::skillShield, var_value);
            return;
        case VAR_LeatherSkill:
            SetSkillByEvent(&Player::skillLeather, var_value);
            return;
        case VAR_SkillChain:
            SetSkillByEvent(&Player::skillChain, var_value);
            return;
        case VAR_PlateSkill:
            SetSkillByEvent(&Player::skillPlate, var_value);
            return;
        case VAR_FireSkill:
            SetSkillByEvent(&Player::skillFire, var_value);
            return;
        case VAR_AirSkill:
            SetSkillByEvent(&Player::skillAir, var_value);
            return;
        case VAR_WaterSkill:
            SetSkillByEvent(&Player::skillWater, var_value);
            return;
        case VAR_EarthSkill:
            SetSkillByEvent(&Player::skillEarth, var_value);
            return;
        case VAR_SpiritSkill:
            SetSkillByEvent(&Player::skillSpirit, var_value);
            return;
        case VAR_MindSkill:
            SetSkillByEvent(&Player::skillMind, var_value);
            return;
        case VAR_BodySkill:
            SetSkillByEvent(&Player::skillBody, var_value);
            return;
        case VAR_LightSkill:
            SetSkillByEvent(&Player::skillLight, var_value);
            return;
        case VAR_DarkSkill:
            SetSkillByEvent(&Player::skillDark, var_value);
            return;
        case VAR_IdentifyItemSkill:
            SetSkillByEvent(&Player::skillItemId, var_value);
            return;
        case VAR_MerchantSkill:
            SetSkillByEvent(&Player::skillMerchant, var_value);
            return;
        case VAR_RepairSkill:
            SetSkillByEvent(&Player::skillRepair, var_value);
            return;
        case VAR_BodybuildingSkill:
            SetSkillByEvent(&Player::skillBodybuilding, var_value);
            return;
        case VAR_MeditationSkill:
            SetSkillByEvent(&Player::skillMeditation, var_value);
            return;
        case VAR_PerceptionSkill:
            SetSkillByEvent(&Player::skillPerception, var_value);
            return;
        case VAR_DiplomacySkill:
            SetSkillByEvent(&Player::skillDiplomacy, var_value);
            return;
        case VAR_ThieverySkill:
            Error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            SetSkillByEvent(&Player::skillDisarmTrap, var_value);
            return;
        case VAR_DodgeSkill:
            SetSkillByEvent(&Player::skillDodge, var_value);
            return;
        case VAR_UnarmedSkill:
            SetSkillByEvent(&Player::skillUnarmed, var_value);
            return;
        case VAR_IdentifyMonsterSkill:
            SetSkillByEvent(&Player::skillMonsterId, var_value);
            return;
        case VAR_ArmsmasterSkill:
            SetSkillByEvent(&Player::skillArmsmaster, var_value);
            return;
        case VAR_StealingSkill:
            SetSkillByEvent(&Player::skillStealing, var_value);
            return;
        case VAR_AlchemySkill:
            SetSkillByEvent(&Player::skillAlchemy, var_value);
            return;
        case VAR_LearningSkill:
            SetSkillByEvent(&Player::skillLearning, var_value);
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound() {
    int playerIndex = GetPlayerIndex();
    int v25 = 8 * playerIndex + 400;
    HEXRAYS_LOBYTE(v25) = PID(OBJECT_Player, playerIndex - 112);
    pAudioPlayer->PlaySound(SOUND_quest, v25, 0, -1, 0, 0);
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim() {
    int playerIndex = GetPlayerIndex();
    spell_fx_renderer->SetPlayerBuffAnim(0x96u, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim_Face(PlayerSpeech speech) {
    this->PlaySound(speech, 0);
    PlayAwardSound_Anim();
}

//----- (new function) --------------------------------------------------------
void Player::SetSkillByEvent(unsigned __int16 Player::*skillToSet,
                             unsigned __int16 skillValue) {
    unsigned __int16 currSkillValue = this->*skillToSet;
    if (skillValue > 63) {  // the original had the condition reversed which was probably wrong
        this->*skillToSet = skillValue | currSkillValue & 63;
    } else {
        this->*skillToSet = skillValue | currSkillValue & 0xC0;
    }
    int playerIndex = GetPlayerIndex();
    spell_fx_renderer->SetPlayerBuffAnim(0x96u, playerIndex);
    PlayAwardSound();
}

//----- (0044AFFB) --------------------------------------------------------
void Player::AddVariable(enum VariableType var_type, signed int val) {
    int v6;               // eax@15
    unsigned int v7;      // esi@18
    DDM_DLV_Header* v27;  // eax@153
    ItemGen item;         // [sp+Ch] [bp-2Ch]@45

    if (var_type >= VAR_Counter1 && var_type <= VAR_Counter10) {
        pParty->PartyTimes.CounterEventValues[var_type - VAR_Counter1] =
            pParty->GetPlayingTime();
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 &&
        var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[var_type - VAR_UnknownTimeEvent0] =
            pParty->GetPlayingTime();
        PlayAwardSound();
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 &&
        var_type <= VAR_MapPersistentVariable_99) {
        if (var_type >= VAR_MapPersistentVariable_0 &&
            var_type <= VAR_MapPersistentVariable_74) {
            if (255 - val >
                stru_5E4C90_MapPersistVars
                    .field_0[var_type - VAR_MapPersistentVariable_0])
                stru_5E4C90_MapPersistVars
                    .field_0[var_type - VAR_MapPersistentVariable_0] += val;
            else
                stru_5E4C90_MapPersistVars
                    .field_0[var_type - VAR_MapPersistentVariable_0] = 255;
        }
        if ((signed int)var_type >= VAR_MapPersistentVariable_75 &&
            var_type <= VAR_MapPersistentVariable_99) {
            if (255 - val >
                stru_5E4C90_MapPersistVars
                    ._decor_events[var_type - VAR_MapPersistentVariable_75])
                stru_5E4C90_MapPersistVars
                    ._decor_events[var_type - VAR_MapPersistentVariable_75] +=
                    val;
            else
                stru_5E4C90_MapPersistVars
                    ._decor_events[var_type - VAR_MapPersistentVariable_75] =
                    255;
        }
        return;
    }

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[var_type - VAR_History_0]) {
            pParty->PartyTimes.HistoryEventTimes[var_type - VAR_History_0] =
                pParty->GetPlayingTime();
            if (pStorylineText->StoreLine[var_type - VAR_History_0].pText == 0) {
                bFlashHistoryBook = 1;
                PlayAwardSound();
            }
        }
        return;
    }

    switch (var_type) {
        case VAR_RandomGold:
            if (val == 0) val = 1;
            v6 = rand();
            pParty->PartyFindsGold(v6 % val + 1, 1);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_RandomFood:
            if (val == 0) val = 1;
            v7 = rand() % val + 1;
            Party::GiveFood(v7);
            GameUI_StatusBar_OnEvent(
                localization->FormatString(502, v7));  // You find %lu food

            GameUI_DrawFoodAndGold();
            PlayAwardSound();
            return;
        case VAR_Sex:
            this->uSex = (PLAYER_SEX)val;
            PlayAwardSound_Anim97();
            return;
        case VAR_Class:
            this->classType = (PLAYER_CLASS_TYPE)val;
            PlayAwardSound_Anim97();
            return;
        case VAR_CurrentHP:
            this->sHealth = std::min(this->sHealth + val, this->GetMaxHealth());
            PlayAwardSound_Anim97();
            return;
        case VAR_MaxHP:
            this->_health_related = 0;
            this->uFullHealthBonus = 0;
            this->sHealth = this->GetMaxHealth();
            return;
        case VAR_CurrentSP:
            this->sMana = std::min(this->sMana + val, this->GetMaxMana());
            PlayAwardSound_Anim97();
            return;
        case VAR_MaxSP:
            this->_mana_related = 0;
            this->uFullManaBonus = 0;
            this->sMana = GetMaxMana();
            return;
        case VAR_ACModifier:
            this->sACModifier = std::min(this->sACModifier + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_BaseLevel:
            this->uLevel = std::min(this->uLevel + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier = std::min(this->sLevelModifier + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_Age:
            this->sAgeModifier += val;
            return;
        case VAR_Award:
            if (_449B57_test_bit(this->_achieved_awards_bits, val) &&
                pAwards[val].pText) {
                PlayAwardSound_Anim97_Face(SPEECH_96);
            }
            _449B7E_toggle_bit(this->_achieved_awards_bits, val, 1);
            return;
        case VAR_Experience:
            this->uExperience = std::min((unsigned __int64)(this->uExperience + val), 4000000000ull);
            PlayAwardSound_Anim97();
            return;
        case VAR_QBits_QuestsDone:
            if (!_449B57_test_bit(pParty->_quest_bits, val) &&
                pQuestTable[val]) {
                bFlashQuestBook = 1;
                PlayAwardSound_Anim97_Face(SPEECH_93);
            }
            _449B7E_toggle_bit(pParty->_quest_bits, val, 1);
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.uAttributes = 1;
            item.uItemID = val;
            if (val >= ITEM_ARTIFACT_PUCK && val <= ITEM_RELIC_MEKORIGS_HAMMER) {
                pParty->pIsArtifactFound[val - 500] = 1;
            } else if (val >= ITEM_WAND_FIRE && val <= ITEM_WAND_INCENERATION) {
                item.uNumCharges = rand() % 6 + item.GetDamageMod() + 1;
                item.uMaxCharges = item.uNumCharges;
            }
            pParty->SetHoldingItem(&item);
            return;
        case VAR_FixedGold:
            pParty->PartyFindsGold(val, 1);
            return;
        case VAR_BaseMight:
            this->uMight = std::min(this->uMight + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BaseIntellect:
            this->uIntelligence = std::min(this->uIntelligence + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BasePersonality:
            this->uWillpower = std::min(this->uWillpower + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BaseEndurance:
            this->uEndurance = std::min(this->uEndurance + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BaseSpeed:
            this->uSpeed = std::min(this->uSpeed + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BaseAccuracy:
            this->uAccuracy = std::min(this->uAccuracy + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BaseLuck:
            this->uLuck = std::min(this->uLuck + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_FixedFood:
            Party::GiveFood(val);
            GameUI_StatusBar_OnEvent(
                localization->FormatString(502, val));  // You find %lu food

            PlayAwardSound();
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->uMightBonus = std::min(this->uMightBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->uIntelligenceBonus = std::min(this->uIntelligenceBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->uWillpowerBonus = std::min(this->uWillpowerBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->uEnduranceBonus = std::min(this->uEnduranceBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->uSpeedBonus = std::min(this->uSpeedBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->uAccuracyBonus = std::min(this->uAccuracyBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->uLuckBonus = std::min(this->uLuckBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_FireResistance:
            this->sResFireBase = std::min(this->sResFireBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_AirResistance:
            this->sResAirBase = std::min(this->sResAirBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase = std::min(this->sResWaterBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase = std::min(this->sResEarthBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase = std::min(this->sResSpiritBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_MindResistance:
            this->sResMindBase = std::min(this->sResMindBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase = std::min(this->sResBodyBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_LightResistance:
            this->sResLightBase = std::min(this->sResLightBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase = std::min(this->sResDarkBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase = std::min(this->sResMagicBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_92);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus = std::min(this->sResFireBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus = std::min(this->sResAirBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus = std::min(this->sResWaterBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus = std::min(this->sResEarthBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus = std::min(this->sResSpiritBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus = std::min(this->sResMindBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus = std::min(this->sResBodyBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus = std::min(this->sResLightBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus = std::min(this->sResDarkBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus = std::min(this->sResMagicBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_91);
            return;
        case VAR_Cursed:
            this->SetCondition(Condition_Cursed, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Weak:
            this->SetCondition(Condition_Weak, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Asleep:
            this->SetCondition(Condition_Sleep, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Afraid:
            this->SetCondition(Condition_Fear, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Drunk:
            this->SetCondition(Condition_Drunk, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Insane:
            this->SetCondition(Condition_Insane, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedGreen:
            this->SetCondition(Condition_Poison_Weak, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedGreen:
            this->SetCondition(Condition_Disease_Weak, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedYellow:
            this->SetCondition(Condition_Poison_Medium, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedYellow:
            this->SetCondition(Condition_Disease_Medium, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedRed:
            this->SetCondition(Condition_Poison_Severe, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedRed:
            this->SetCondition(Condition_Disease_Severe, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Paralyzed:
            this->SetCondition(Condition_Paralyzed, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Unconsious:
            this->SetCondition(Condition_Unconcious, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Dead:
            this->SetCondition(Condition_Dead, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Stoned:
            this->SetCondition(Condition_Pertified, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Eradicated:
            this->SetCondition(Condition_Eradicated, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_MajorCondition:
            conditions_times.fill(GameTime(0));
            PlayAwardSound_Anim97();
            return;
        case VAR_AutoNotes:
            if (!_449B57_test_bit(pParty->_autonote_bits, val) &&
                pAutonoteTxt[val].pText) {
                this->PlaySound(SPEECH_96, 0);
                bFlashAutonotesBook = 1;
                _506568_autonote_type = pAutonoteTxt[val].eType;
                spell_fx_renderer->SetPlayerBuffAnim(0x97u, GetPlayerIndex());
            }
            _449B7E_toggle_bit(pParty->_autonote_bits, val, 1);
            PlayAwardSound();
            return;
        case VAR_PlayerBits:
            _449B7E_toggle_bit((unsigned char*)this->playerEventBits, val, 1u);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNewNPCData[val].uFlags |= 0x80;
            pParty->CountHirelings();
            viewparams->bRedrawGameUI = true;
            return;
        case VAR_NumSkillPoints:
            this->uSkillPoints += val;
            return;
        case VAR_ReputationInCurrentLocation:
            v27 = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v27 = &pIndoor->dlv;
            v27->uReputation += val;
            if (v27->uReputation > 10000) v27->uReputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank += val;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths += val;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected += val;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms += val;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaPageWins += val;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaSquireWins += val;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaKnightWins += val;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaLordWins += val;
            return;
        case VAR_StaffSkill:
            AddSkillByEvent(&Player::skillStaff, val);
            return;
        case VAR_SwordSkill:
            AddSkillByEvent(&Player::skillSword, val);
            return;
        case VAR_DaggerSkill:
            AddSkillByEvent(&Player::skillDagger, val);
            return;
        case VAR_AxeSkill:
            AddSkillByEvent(&Player::skillAxe, val);
            return;
        case VAR_SpearSkill:
            AddSkillByEvent(&Player::skillSpear, val);
            return;
        case VAR_BowSkill:
            AddSkillByEvent(&Player::skillBow, val);
            return;
        case VAR_MaceSkill:
            AddSkillByEvent(&Player::skillMace, val);
            return;
        case VAR_BlasterSkill:
            AddSkillByEvent(&Player::skillBlaster, val);
            return;
        case VAR_ShieldSkill:
            AddSkillByEvent(&Player::skillShield, val);
            return;
        case VAR_LeatherSkill:
            AddSkillByEvent(&Player::skillLeather, val);
            return;
        case VAR_SkillChain:
            AddSkillByEvent(&Player::skillChain, val);
            return;
        case VAR_PlateSkill:
            AddSkillByEvent(&Player::skillPlate, val);
            return;
        case VAR_FireSkill:
            AddSkillByEvent(&Player::skillFire, val);
            return;
        case VAR_AirSkill:
            AddSkillByEvent(&Player::skillAir, val);
            return;
        case VAR_WaterSkill:
            AddSkillByEvent(&Player::skillWater, val);
            return;
        case VAR_EarthSkill:
            AddSkillByEvent(&Player::skillEarth, val);
            return;
        case VAR_SpiritSkill:
            AddSkillByEvent(&Player::skillSpirit, val);
            return;
        case VAR_MindSkill:
            AddSkillByEvent(&Player::skillMind, val);
            return;
        case VAR_BodySkill:
            AddSkillByEvent(&Player::skillBody, val);
            return;
        case VAR_LightSkill:
            AddSkillByEvent(&Player::skillLight, val);
            return;
        case VAR_DarkSkill:
            AddSkillByEvent(&Player::skillDark, val);
            return;
        case VAR_IdentifyItemSkill:
            AddSkillByEvent(&Player::skillItemId, val);
            return;
        case VAR_MerchantSkill:
            AddSkillByEvent(&Player::skillMerchant, val);
            return;
        case VAR_RepairSkill:
            AddSkillByEvent(&Player::skillRepair, val);
            return;
        case VAR_BodybuildingSkill:
            AddSkillByEvent(&Player::skillBodybuilding, val);
            return;
        case VAR_MeditationSkill:
            AddSkillByEvent(&Player::skillMeditation, val);
            return;
        case VAR_PerceptionSkill:
            AddSkillByEvent(&Player::skillPerception, val);
            return;
        case VAR_DiplomacySkill:
            AddSkillByEvent(&Player::skillDiplomacy, val);
            return;
        case VAR_ThieverySkill:
            Error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            AddSkillByEvent(&Player::skillDisarmTrap, val);
            return;
        case VAR_DodgeSkill:
            AddSkillByEvent(&Player::skillDodge, val);
            return;
        case VAR_UnarmedSkill:
            AddSkillByEvent(&Player::skillUnarmed, val);
            return;
        case VAR_IdentifyMonsterSkill:
            AddSkillByEvent(&Player::skillMonsterId, val);
            return;
        case VAR_ArmsmasterSkill:
            AddSkillByEvent(&Player::skillArmsmaster, val);
            return;
        case VAR_StealingSkill:
            AddSkillByEvent(&Player::skillStealing, val);
            return;
        case VAR_AlchemySkill:
            AddSkillByEvent(&Player::skillAlchemy, val);
            return;
        case VAR_LearningSkill:
            AddSkillByEvent(&Player::skillLearning, val);
            return;
        default:
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim97() {
    int playerIndex = GetPlayerIndex();
    spell_fx_renderer->SetPlayerBuffAnim(0x97u, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim97_Face(PlayerSpeech speech) {
    this->PlaySound(speech, 0);
    PlayAwardSound_Anim97();
}

//----- (new function) --------------------------------------------------------
void Player::AddSkillByEvent(unsigned __int16 Player::*skillToSet,
                             unsigned __int16 addSkillValue) {
    if (addSkillValue > 63) {
        this->*skillToSet =
            (unsigned __int8)addSkillValue | this->*skillToSet & 63;
    } else {
        this->*skillToSet = std::min(this->*skillToSet + addSkillValue, 60) |
                            this->*skillToSet & 0xC0;
    }
    PlayAwardSound_Anim97();
    return;
}

//----- (0044B9C4) --------------------------------------------------------
void Player::SubtractVariable(enum VariableType VarNum, signed int pValue) {
    DDM_DLV_Header* locationHeader;  // eax@90
    int randGold;
    int randFood;
    int npcIndex;

    if (VarNum >= VAR_MapPersistentVariable_0 &&
        VarNum <= VAR_MapPersistentVariable_99) {
        if (VarNum >= VAR_MapPersistentVariable_0 &&
            VarNum <= VAR_MapPersistentVariable_74) {
            stru_5E4C90_MapPersistVars
                .field_0[VarNum - VAR_MapPersistentVariable_0] -= pValue;
        }
        if ((signed int)VarNum >= VAR_MapPersistentVariable_75 &&
            VarNum <= VAR_MapPersistentVariable_99) {
            stru_5E4C90_MapPersistVars
                ._decor_events[VarNum - VAR_MapPersistentVariable_75] -= pValue;
        }
        return;
    }

    switch (VarNum) {
        case VAR_CurrentHP:
            ReceiveDamage((signed int)pValue, DMGT_PHISYCAL);
            PlayAwardSound_Anim98();
            return;
        case VAR_CurrentSP:
            this->sMana = std::max(this->sMana - pValue, 0);
            PlayAwardSound_Anim98();
            return;
        case VAR_ACModifier:
            this->sACModifier -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_BaseLevel:
            this->uLevel -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_Age:
            this->sAgeModifier -= (signed __int16)pValue;
            return;
        case VAR_Award:
            _449B7E_toggle_bit(this->_achieved_awards_bits,
                               (signed __int16)pValue, 0);
            return;
        case VAR_Experience:
            this->uExperience -= pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_QBits_QuestsDone:
            _449B7E_toggle_bit(pParty->_quest_bits, (__int16)pValue, 0);
            this->PlaySound(SPEECH_96, 0);
            return;
        case VAR_PlayerItemInHands:
            for (uint i = 0; i < 16; ++i) {
                int id_ = this->pEquipment.pIndices[i];
                if (id_ > 0) {
                    if (this->pInventoryItemList[this->pEquipment.pIndices[i] -
                                                 1]
                            .uItemID == pValue) {
                        this->pEquipment.pIndices[i] = 0;
                    }
                }
            }
            for (int i = 0; i < 126; i++) {
                int id_ = this->pInventoryMatrix[i];
                if (id_ > 0) {
                    if (this->pInventoryItemList[id_ - 1].uItemID == pValue) {
                        RemoveItemAtInventoryIndex(i);
                        return;
                    }
                }
            }
            if (pParty->pPickedItem.uItemID == pValue) {
                mouse->RemoveHoldingItem();
                return;
            }
            return;
        case VAR_FixedGold:
            if (pValue > pParty->GetGold()) {
                dword_5B65C4_cancelEventProcessing = 1;
                return;
            }
            Party::TakeGold(pValue);
            return;
        case VAR_RandomGold:
            randGold = rand() % (signed int)pValue + 1;
            if (randGold > pParty->GetGold())
                randGold = pParty->GetGold();
            Party::TakeGold(randGold);
            GameUI_StatusBar_OnEvent(localization->FormatString(503, randGold));
            GameUI_DrawFoodAndGold();
            return;
        case VAR_FixedFood:
            Party::TakeFood(pValue);
            PlayAwardSound_Anim98();
            return;
        case VAR_RandomFood:
            randFood = rand() % (signed int)pValue + 1;
            if (randFood > pParty->GetFood())
                randFood = pParty->GetFood();
            Party::TakeFood(randFood);
            GameUI_StatusBar_OnEvent(localization->FormatString(504, randFood));
            GameUI_DrawFoodAndGold();
            PlayAwardSound_Anim98();
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->uMightBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->uIntelligenceBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->uWillpowerBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->uEnduranceBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->uSpeedBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->uAccuracyBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->uLuckBonus -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_BaseMight:
            this->uMight -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BaseIntellect:
            this->uIntelligence -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BasePersonality:
            this->uWillpower -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BaseEndurance:
            this->uEndurance -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BaseSpeed:
            this->uSpeed -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BaseAccuracy:
            this->uAccuracy -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BaseLuck:
            this->uLuck -= (unsigned __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_FireResistance:
            this->sResFireBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_AirResistance:
            this->sResAirBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_MindResistance:
            this->sResMindBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_LightResistance:
            this->sResLightBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_92);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus -= (signed __int16)pValue;
            this->PlayAwardSound_Anim98_Face(SPEECH_91);
            return;
        case VAR_StaffSkill:
            this->skillStaff -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_SwordSkill:
            this->skillSword -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_DaggerSkill:
            this->skillDagger -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_AxeSkill:
            this->skillAxe -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_SpearSkill:
            this->skillSpear -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_BowSkill:
            this->skillBow -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_MaceSkill:
            this->skillMace -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_BlasterSkill:
            this->skillBlaster -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_ShieldSkill:
            this->skillShield -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_LeatherSkill:
            this->skillLearning -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_SkillChain:
            this->skillChain -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_PlateSkill:
            this->skillPlate -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_FireSkill:
            this->skillFire -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_AirSkill:
            this->skillAir -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_WaterSkill:
            this->skillWater -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_EarthSkill:
            this->skillEarth -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_SpiritSkill:
            this->skillSpirit -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_MindSkill:
            this->skillMind -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_BodySkill:
            this->skillBody -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_LightSkill:
            this->skillLight -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_DarkSkill:
            this->skillDark -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_IdentifyItemSkill:
            this->skillItemId -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_MerchantSkill:
            this->skillMerchant -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_RepairSkill:
            this->skillRepair -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_BodybuildingSkill:
            this->skillBodybuilding -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_MeditationSkill:
            this->skillMeditation -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_PerceptionSkill:
            this->skillPerception -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_DiplomacySkill:
            this->skillDiplomacy -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_ThieverySkill:
            Error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            this->skillDisarmTrap -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_DodgeSkill:
            this->skillDodge -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_UnarmedSkill:
            this->skillUnarmed -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_IdentifyMonsterSkill:
            this->skillMonsterId -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_ArmsmasterSkill:
            this->skillArmsmaster -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_StealingSkill:
            this->skillStealing -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_AlchemySkill:
            this->skillAlchemy -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_LearningSkill:
            this->skillLearning -= (unsigned __int8)pValue;
            PlayAwardSound_Anim98();
            return;
        case VAR_Cursed:
            this->conditions_times[Condition_Cursed].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Weak:
            this->conditions_times[Condition_Weak].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Asleep:
            this->conditions_times[Condition_Sleep].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Afraid:
            this->conditions_times[Condition_Fear].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Drunk:
            this->conditions_times[Condition_Drunk].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Insane:
            this->conditions_times[Condition_Insane].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_PoisonedGreen:
            this->conditions_times[Condition_Poison_Weak].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_DiseasedGreen:
            this->conditions_times[Condition_Disease_Weak].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_PoisonedYellow:
            this->conditions_times[Condition_Poison_Medium].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_DiseasedYellow:
            this->conditions_times[Condition_Disease_Medium].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_PoisonedRed:
            this->conditions_times[Condition_Poison_Severe].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_DiseasedRed:
            this->conditions_times[Condition_Disease_Severe].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Paralyzed:
            this->conditions_times[Condition_Paralyzed].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Unconsious:
            this->conditions_times[Condition_Unconcious].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Dead:
            this->conditions_times[Condition_Dead].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Stoned:
            this->conditions_times[Condition_Pertified].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_Eradicated:
            this->conditions_times[Condition_Eradicated].Reset();
            PlayAwardSound_Anim98();
            return;
        case VAR_AutoNotes:
            _449B7E_toggle_bit(pParty->_autonote_bits, pValue - 1, 0);
            return;
        case VAR_NPCs2:
            npcIndex = 0;
            GetNewNPCData(sDialogue_SpeakingActorNPC_ID, &npcIndex);
            if (npcIndex == pValue) {
                npcIdToDismissAfterDialogue = pValue;
            } else {
                npcIdToDismissAfterDialogue = 0;
                pParty->hirelingScrollPosition = 0;
                pNPCStats->pNewNPCData[(int)pValue].uFlags &= 0xFFFFFF7F;
                pParty->CountHirelings();
                viewparams->bRedrawGameUI = true;
            }
            return;
        case VAR_HiredNPCHasSpeciality:
            for (unsigned int i = 0; i < pNPCStats->uNumNewNPCs; i++) {
                if (pNPCStats->pNewNPCData[i].uProfession == pValue) {
                    pNPCStats->pNewNPCData[(int)pValue].uFlags &= 0xFFFFFF7F;
                }
            }
            if (pParty->pHirelings[0].uProfession == pValue)
                memset(&pParty->pHirelings[0], 0, sizeof(NPCData));
            if (pParty->pHirelings[1].uProfession == pValue)
                memset(&pParty->pHirelings[1], 0, sizeof(NPCData));
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            if ((unsigned int)pValue <= this->uSkillPoints) {
                this->uSkillPoints -= pValue;
            } else {
                this->uSkillPoints = 0;
            }
            return;
        case VAR_ReputationInCurrentLocation:
            locationHeader = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor)
                locationHeader = &pIndoor->dlv;
            locationHeader->uReputation -= pValue;
            if (locationHeader->uReputation < -10000)
                locationHeader->uReputation = -10000;
            return;
        case VAR_GoldInBank:
            if ((unsigned int)pValue <= pParty->uNumGoldInBank) {
                pParty->uNumGoldInBank -= (unsigned int)pValue;
            } else {
                dword_5B65C4_cancelEventProcessing = 1;
            }
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths -= (unsigned int)pValue;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected -= (unsigned int)pValue;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms -= (int)pValue;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaPageWins -= (char)pValue;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaSquireWins -= (char)pValue;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaKnightWins -= (char)pValue;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaLordWins -= (char)pValue;
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim98() {
    int playerIndex = GetPlayerIndex();
    spell_fx_renderer->SetPlayerBuffAnim(0x98u, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Player::PlayAwardSound_Anim98_Face(PlayerSpeech speech) {
    this->PlaySound(speech, 0);
    PlayAwardSound_Anim98();
}

//----- (00467E7F) --------------------------------------------------------
void Player::EquipBody(ITEM_EQUIP_TYPE uEquipType) {
    int itemAnchor;          // ebx@1
    int itemInvLocation;     // edx@1
    int freeSlot;            // eax@3
    ItemGen tempPickedItem;  // [sp+Ch] [bp-30h]@1

    tempPickedItem.Reset();
    itemAnchor = pEquipTypeToBodyAnchor[uEquipType];
    itemInvLocation =
        pPlayers[uActiveCharacter]->pEquipment.pIndices[itemAnchor];
    if (itemInvLocation) {  //переодеться в другую вещь
        memcpy(&tempPickedItem, &pParty->pPickedItem, sizeof(tempPickedItem));
        pPlayers[uActiveCharacter]
            ->pInventoryItemList[itemInvLocation - 1]
            .uBodyAnchor = 0;
        pParty->pPickedItem.Reset();
        pParty->SetHoldingItem(&pPlayers[uActiveCharacter]
                                    ->pInventoryItemList[itemInvLocation - 1]);
        tempPickedItem.uBodyAnchor = itemAnchor + 1;
        memcpy(&pPlayers[uActiveCharacter]
                    ->pInventoryItemList[itemInvLocation - 1],
               &tempPickedItem, sizeof(ItemGen));
        pPlayers[uActiveCharacter]->pEquipment.pIndices[itemAnchor] =
            itemInvLocation;
    } else {  // одеть вещь
        freeSlot = pPlayers[uActiveCharacter]->FindFreeInventoryListSlot();
        if (freeSlot >= 0) {
            pParty->pPickedItem.uBodyAnchor = itemAnchor + 1;
            memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[freeSlot],
                   &pParty->pPickedItem, sizeof(ItemGen));
            pPlayers[uActiveCharacter]->pEquipment.pIndices[itemAnchor] =
                freeSlot + 1;
            mouse->RemoveHoldingItem();
        }
    }
}

//----- (0049387A) --------------------------------------------------------
int CycleCharacter(bool backwards) {
    const int PARTYSIZE = 4;
    int valToAdd = backwards ? (PARTYSIZE - 2) : 0;
    int mult = backwards ? -1 : 1;

    for (int i = 0; i < (PARTYSIZE - 1); i++) {
        int currCharId =
            ((uActiveCharacter + mult * i + valToAdd) % PARTYSIZE) + 1;
        if (pPlayers[currCharId]->uTimeToRecovery == 0) {
            return currCharId;
        }
    }
    return uActiveCharacter;
}

//----- (0043EE77) --------------------------------------------------------
bool Player::HasUnderwaterSuitEquipped() {
    // the original function took the
    // player number as a parameter. if it
    // was 0, the whole party was checked.
    // calls with the parameter 0 have been
    // changed to calls to this for every
    // player
    if (GetArmorItem() == nullptr || GetArmorItem()->uItemID != 604) {
        return false;
    }
    return true;
}

//----- (0043EE15) --------------------------------------------------------
bool Player::HasItem(unsigned int uItemID, bool checkHeldItem) {
    if (!checkHeldItem || pParty->pPickedItem.uItemID != uItemID) {
        for (uint i = 0; i < 126; ++i) {
            if (this->pInventoryMatrix[i] > 0) {
                if ((unsigned int)this
                        ->pInventoryItemList[this->pInventoryMatrix[i] - 1]
                        .uItemID == uItemID)
                    return true;
            }
        }
        for (uint i = 0; i < 16; ++i) {
            if (this->pEquipment.pIndices[i]) {
                if ((unsigned int)this
                        ->pInventoryItemList[this->pEquipment.pIndices[i] - 1]
                        .uItemID == uItemID)
                    return true;
            }
        }
        return false;
    } else {
        return true;
    }
}
//----- (0043EDB9) --------------------------------------------------------
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this) {
    CHARACTER_RACE race;  // edi@2
    PLAYER_SEX sex;       // eax@2

    for (int i = 1; i <= 4; i++) {
        race = pPlayers[i]->GetRace();
        sex = pPlayers[i]->GetSexByVoice();
        switch (_this) {
            case 0:
                if ((race == CHARACTER_RACE_HUMAN ||
                     race == CHARACTER_RACE_ELF ||
                     race == CHARACTER_RACE_GOBLIN) &&
                    sex == SEX_MALE)
                    return true;
                break;
            case 1:
                if ((race == CHARACTER_RACE_HUMAN ||
                     race == CHARACTER_RACE_ELF ||
                     race == CHARACTER_RACE_GOBLIN) &&
                    sex == SEX_FEMALE)
                    return true;
                break;
            case 2:
                if (race == CHARACTER_RACE_DWARF && sex == SEX_MALE)
                    return true;
                break;
            case 3:
                if (race == CHARACTER_RACE_DWARF && sex == SEX_FEMALE)
                    return true;
                break;
        }
    }
    return false;
}

//----- (0043ED6F) --------------------------------------------------------
bool IsDwarfPresentInParty(bool a1) {
    for (uint i = 0; i < 4; ++i) {
        CHARACTER_RACE race = pParty->pPlayers[i].GetRace();

        if (race == CHARACTER_RACE_DWARF && a1)
            return true;
        else if (race != CHARACTER_RACE_DWARF && !a1)
            return true;
    }
    return false;
}

//----- (00439FCB) --------------------------------------------------------
void DamagePlayerFromMonster(unsigned int uObjID, int dmgSource, Vec3_int_* pPos, signed int targetchar) {
    // target player? if any

    Player* playerPtr;            // ebx@3
    Actor* actorPtr;              // esi@3
    int spellId;                  // eax@38
    signed int recvdMagicDmg;     // eax@139
    int v72[4];                   // [sp+30h] [bp-24h]@164
    int healthBeforeRecvdDamage;  // [sp+48h] [bp-Ch]@3


    unsigned int uActorID = PID_ID(uObjID);
    int pidtype = PID_TYPE(uObjID);

    /*    OBJECT_Any = 0x0,
    OBJECT_BLVDoor = 0x1,
    OBJECT_Item = 0x2,
    OBJECT_Actor = 0x3,
    OBJECT_Player = 0x4,
    OBJECT_Decoration = 0x5,
    OBJECT_BModel = 0x6,*/

    if (pidtype != 2) {  // not an item
        // hit by monster
        if (pidtype != 3) __debugbreak();

        if (targetchar == -1) __debugbreak();

        // test
        // if (/*uActorType == OBJECT_Player &&*/ !_A750D8_player_speech_timer) {
        //    _A750D8_player_speech_timer = 256;
        //    PlayerSpeechID = SPEECH_18;
        //    uSpeakingCharacter = 1;
        // }
        // test

        playerPtr = &pParty->pPlayers[targetchar];
        actorPtr = &pActors[uActorID];
        healthBeforeRecvdDamage = playerPtr->sHealth;
        if (PID_TYPE(uObjID) != 3 || !actorPtr->ActorHitOrMiss(playerPtr))
            return;

        // GM unarmed 1% chance to evade attacks per skill point
        if (playerPtr->GetActualSkillMastery(PLAYER_SKILL_UNARMED) >= 4 &&
            rand() % 100 < playerPtr->GetActualSkillLevel(PLAYER_SKILL_UNARMED)) {
            auto str = localization->FormatString(637, playerPtr->pName);  // evades damage
            GameUI_StatusBar_OnEvent(str);
            playerPtr->PlaySound(SPEECH_6, 0);
            return;
        }

        // play hit sound
        ItemGen* equippedArmor = playerPtr->GetArmorItem();
        SoundID soundToPlay;
        if (!equippedArmor || equippedArmor->IsBroken() ||
            (equippedArmor->GetPlayerSkillType() != PLAYER_SKILL_CHAIN &&
             equippedArmor->GetPlayerSkillType() != PLAYER_SKILL_PLATE)) {
            int randVal = rand() % 4;
            switch (randVal) {
                case 0:
                    soundToPlay = (SoundID)108;
                    break;
                case 1:
                    soundToPlay = (SoundID)109;
                    break;
                case 2:
                    soundToPlay = (SoundID)110;
                    break;
                case 3:
                    soundToPlay = (SoundID)44;
                    break;
                default:
                    Error("Unexpected sound value");
            }
        } else {
            int randVal = rand() % 4;
            switch (randVal) {
                case 0:
                    soundToPlay = (SoundID)105;
                    break;
                case 1:
                    soundToPlay = (SoundID)106;
                    break;
                case 2:
                    soundToPlay = (SoundID)107;
                    break;
                case 3:
                    soundToPlay = (SoundID)45;
                    break;
                default:
                    Error("Unexpected sound value");
            }
        }
        pAudioPlayer->PlaySound(soundToPlay, PID(OBJECT_Player, targetchar + 80), 0, -1, 0, 0);

        // calc damage
        int dmgToReceive = actorPtr->_43B3E0_CalcDamage(dmgSource);
        if (actorPtr->pActorBuffs[ACTOR_BUFF_SHRINK].Active()) {
            __int16 spellPower = actorPtr->pActorBuffs[ACTOR_BUFF_SHRINK].uPower;
            if (spellPower > 0)
                dmgToReceive /= spellPower;
        }

        int damageType;
        switch (dmgSource) {
            case 0:
                damageType = actorPtr->pMonsterInfo.uAttack1Type;
                break;
            case 1:
                damageType = actorPtr->pMonsterInfo.uAttack2Type;
                break;
            case 2:
                spellId = actorPtr->pMonsterInfo.uSpell1ID;
                damageType = pSpellStats->pInfos[spellId].uSchool;
                break;
            case 3:
                spellId = actorPtr->pMonsterInfo.uSpell2ID;
                damageType = pSpellStats->pInfos[spellId].uSchool;
                break;
            case 4:
                damageType = actorPtr->pMonsterInfo.field_3C_some_special_attack;
                break;
            default:
            case 5:
                damageType = 4;  // DMGT_PHISYCAL
                break;
        }

        // calc damage
        dmgToReceive = playerPtr->ReceiveDamage(dmgToReceive, (DAMAGE_TYPE)damageType);

        // pain reflection back on attacker
        if (playerPtr->pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION].Active()) {
            int actorState = actorPtr->uAIState;
            if (actorState != Dying && actorState != Dead) {
                int reflectedDamage = actorPtr->CalcMagicalDamageToActor((DAMAGE_TYPE)damageType, dmgToReceive);
                actorPtr->sCurrentHP -= reflectedDamage;
                if (reflectedDamage >= 0) {
                    if (actorPtr->sCurrentHP >= 1) {
                        Actor::AI_Stun(uActorID, PID(OBJECT_Player, targetchar), 0);  // todo extract this branch to a function
                                    // once Actor::functions are changed to
                                    // nonstatic actor functions
                        Actor::AggroSurroundingPeasants(uActorID, 1);
                    } else {
                        // actor has died from retaliation
                        // add bloodsplat
                        if (pMonsterStats->pInfos[actorPtr->pMonsterInfo.uID].bQuestMonster & 1 && !engine->config->NoBloodsplats()) {
                            int splatRadius = _4D864C_force_sw_render_rules && !engine->config->NoHugeBloodsplats()
                                    ? 10 * actorPtr->uActorRadius
                                    : actorPtr->uActorRadius;
                            decal_builder->AddBloodsplat(
                                actorPtr->vPosition.x,
                                actorPtr->vPosition.y,
                                actorPtr->vPosition.z, 1.0, 0.0, 0.0,
                                (float)splatRadius, 0, 0);
                        }
                        Actor::Die(uActorID);
                        Actor::ApplyFineForKillingPeasant(uActorID);
                        Actor::AggroSurroundingPeasants(uActorID, 1);
                        if (actorPtr->pMonsterInfo.uExp)
                            pParty->GivePartyExp(pMonsterStats->pInfos[actorPtr->pMonsterInfo.uID].uExp);

                        // kill speech
                        int speechToPlay = SPEECH_51;
                        if (rand() % 100 < 20)
                            speechToPlay = actorPtr->pMonsterInfo.uHP >= 100 ? 2 : 1;
                        playerPtr->PlaySound((PlayerSpeech)speechToPlay, 0);
                    }
                }
            }
        }

        // special attack trigger
        if (!engine->config->NoDamage() && actorPtr->pMonsterInfo.uSpecialAttackType &&
            rand() % 100 < actorPtr->pMonsterInfo.uLevel *
                                actorPtr->pMonsterInfo.uSpecialAttackLevel) {
            playerPtr->ReceiveSpecialAttackEffect(actorPtr->pMonsterInfo.uSpecialAttackType, actorPtr);
        }

        // add recovery after being hit
        if (!pParty->bTurnBasedModeOn) {
            int actEndurance = playerPtr->GetActualEndurance();
            int recoveryTime = (int)((20 - playerPtr->GetParameterBonus(actEndurance)) *
                      flt_6BE3A4_debug_recmod1 * 2.133333333333333);
            playerPtr->SetRecoveryTime(recoveryTime);
        }

        // badly hurt speech
        int yellThreshold = playerPtr->GetMaxHealth() / 4;
        if (yellThreshold > playerPtr->sHealth &&
            yellThreshold <= healthBeforeRecvdDamage &&
            playerPtr->sHealth > 0) {
            playerPtr->PlaySound(SPEECH_48, 0);
        }

        viewparams->bRedrawGameUI = 1;
        return;
    } else {  // is an item
        SpriteObject* spritefrom = &pSpriteObjects[uActorID];
        int uActorType = PID_TYPE(spritefrom->spell_caster_pid);
        int uActorID = PID_ID(spritefrom->spell_caster_pid);

        if (uActorType == 2) {  // item
            Player* playerPtr;  // eax@81

            // select char target or pick random
            if (targetchar != -1) {
                playerPtr = &pParty->pPlayers[targetchar];
            } else {
                int activePlayerCounter = 0;
                for (int i = 1; i <= 4; i++) {
                    if (pPlayers[i]->CanAct()) {
                        v72[activePlayerCounter] = i;
                        activePlayerCounter++;
                    }
                }
                if (activePlayerCounter) {
                    playerPtr = &pParty->pPlayers[v72[rand() % activePlayerCounter] - 1];
                }
            }

            int damage;
            int damagetype;
            if (uActorType != OBJECT_Player ||spritefrom->spell_id != SPELL_BOW_ARROW) {
                int playerMaxHp = playerPtr->GetMaxHealth();
                damage = _43AFE3_calc_spell_damage(spritefrom->spell_id, spritefrom->spell_level,
                                                spritefrom->spell_skill, playerMaxHp);
                damagetype = pSpellStats->pInfos[spritefrom->spell_id].uSchool;
            } else {
                damage = pParty->pPlayers[uActorID].CalculateRangedDamageTo(0);
                damagetype = 0;
            }
            playerPtr->ReceiveDamage(damage, (DAMAGE_TYPE)damagetype);
            if (uActorType == OBJECT_Player && !_A750D8_player_speech_timer) {
                _A750D8_player_speech_timer = 256;
                PlayerSpeechID = SPEECH_44;
                uSpeakingCharacter = uActorID + 1;
            }
            return;
        } else if (uActorType == 3) {  // missile fired by actor
            Actor* actorPtr = &pActors[uActorID];
            if (targetchar == -1) targetchar = stru_50C198.which_player_to_attack(actorPtr);
            Player* playerPtr = &pParty->pPlayers[targetchar];
            int dmgToReceive = actorPtr->_43B3E0_CalcDamage(dmgSource);
            unsigned __int16 spriteType = spritefrom->uType;

            if (spritefrom->uType == 545) {  // arrows
                // GM unarmed 1% chance to evade attack per skill point
                logger->Info("Arrpow");
                if (playerPtr->GetActualSkillMastery(PLAYER_SKILL_UNARMED) >= 4 &&
                    rand() % 100 < playerPtr->GetActualSkillLevel(PLAYER_SKILL_UNARMED)) {
                    auto str = localization->FormatString(637, playerPtr->pName);  // evades damage
                    GameUI_StatusBar_OnEvent(str);
                    playerPtr->PlaySound(SPEECH_6, 0);
                    return;
                }
            } else if (spriteType == 555 || spriteType == 510 ||  // dragonflies firebolt
                       spriteType == 500 || spriteType == 515 ||
                       spriteType == 505 || spriteType == 530 ||  // all missile types?
                       spriteType == 525 || spriteType == 520 ||
                       spriteType == 535 || spriteType == 540) {
                // reduce missle damage with skills / armour
                if (!actorPtr->ActorHitOrMiss(playerPtr)) return;
                if (playerPtr->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active()) dmgToReceive >>= 1;
                if (playerPtr->HasEnchantedItemEquipped(36)) dmgToReceive >>= 1;
                if (playerPtr->HasEnchantedItemEquipped(69)) dmgToReceive >>= 1;
                if (playerPtr->HasItemEquipped(EQUIP_ARMOUR) &&
                    playerPtr->GetArmorItem()->uItemID ==
                        ITEM_ARTIFACT_GOVERNORS_ARMOR)
                    dmgToReceive >>= 1;
                if (playerPtr->HasItemEquipped(EQUIP_TWO_HANDED)) {
                    ItemGen* mainHandItem = playerPtr->GetMainHandItem();
                    if (mainHandItem->uItemID == ITEM_RELIC_KELEBRIM ||
                        mainHandItem->uItemID == ITEM_ARTIFACT_ELFBANE ||
                        (mainHandItem->GetItemEquipType() == EQUIP_SHIELD &&
                         playerPtr->GetActualSkillMastery(
                             PLAYER_SKILL_SHIELD) == 4))
                        dmgToReceive >>= 1;
                }
                if (playerPtr->HasItemEquipped(EQUIP_SINGLE_HANDED)) {
                    ItemGen* offHandItem = playerPtr->GetOffHandItem();
                    if (offHandItem->uItemID == ITEM_RELIC_KELEBRIM ||
                        offHandItem->uItemID == ITEM_ARTIFACT_ELFBANE ||
                        (offHandItem->GetItemEquipType() == EQUIP_SHIELD &&
                         playerPtr->GetActualSkillMastery(
                             PLAYER_SKILL_SHIELD) == 4))
                        dmgToReceive >>= 1;
                }
            }

            if (actorPtr->pActorBuffs[ACTOR_BUFF_SHRINK].Active()) {
                int spellPower = actorPtr->pActorBuffs[ACTOR_BUFF_SHRINK].uPower;
                if (spellPower > 0) dmgToReceive /= spellPower;
            }

            int damageType;
            switch (dmgSource) {
                case 0:
                    damageType = actorPtr->pMonsterInfo.uAttack1Type;
                    break;
                case 1:
                    damageType = actorPtr->pMonsterInfo.uAttack2Type;
                    break;
                case 2:
                    spellId = actorPtr->pMonsterInfo.uSpell1ID;
                    damageType = pSpellStats->pInfos[spellId].uSchool;
                    break;
                case 3:
                    spellId = actorPtr->pMonsterInfo.uSpell2ID;
                    damageType = pSpellStats->pInfos[spellId].uSchool;
                    break;
                case 4:
                    damageType = actorPtr->pMonsterInfo.field_3C_some_special_attack;
                    break;
                case 5:
                    damageType = 4;
                    break;
            }

            int reflectedDmg = playerPtr->ReceiveDamage(dmgToReceive, (DAMAGE_TYPE)damageType);
            if (playerPtr->pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION].Active()) {
                unsigned __int16 actorState = actorPtr->uAIState;
                if (actorState != Dying && actorState != Dead) {
                    recvdMagicDmg = actorPtr->CalcMagicalDamageToActor((DAMAGE_TYPE)damageType, reflectedDmg);
                    actorPtr->sCurrentHP -= recvdMagicDmg;

                    if (recvdMagicDmg >= 0) {
                        if (actorPtr->sCurrentHP >= 1) {
                            Actor::AI_Stun(uActorID, PID(OBJECT_Player, targetchar), 0);
                            Actor::AggroSurroundingPeasants(uActorID, 1);
                        } else {
                            // actor killed by retaliation
                            if (pMonsterStats->pInfos[actorPtr->pMonsterInfo.uID].bQuestMonster & 1 &&
                                !engine->config->NoBloodsplats()) {
                                int splatRadius = _4D864C_force_sw_render_rules && !engine->config->NoHugeBloodsplats()
                                        ? 10 * actorPtr->uActorRadius
                                        : actorPtr->uActorRadius;
                                decal_builder->AddBloodsplat(
                                    actorPtr->vPosition.x,
                                    actorPtr->vPosition.y,
                                    actorPtr->vPosition.z, 1.0, 0.0, 0.0,
                                    (float)splatRadius, 0, 0);
                            }

                            Actor::Die(uActorID);
                            Actor::ApplyFineForKillingPeasant(uActorID);
                            Actor::AggroSurroundingPeasants(uActorID, 1);
                            if (actorPtr->pMonsterInfo.uExp)
                                pParty->GivePartyExp(pMonsterStats->pInfos[actorPtr->pMonsterInfo.uID].uExp);

                            int speechToPlay = SPEECH_51;
                            if (rand() % 100 < 20)
                                speechToPlay = actorPtr->pMonsterInfo.uHP >= 100 ? 2 : 1;
                            playerPtr->PlaySound((PlayerSpeech)speechToPlay, 0);
                        }
                    }
                }
            }

            // special attack trigger
            if (!dmgSource && !engine->config->NoDamage() &&
                actorPtr->pMonsterInfo.uSpecialAttackType &&
                rand() % 100 < actorPtr->pMonsterInfo.uLevel *
                                   actorPtr->pMonsterInfo.uSpecialAttackLevel) {
                playerPtr->ReceiveSpecialAttackEffect(actorPtr->pMonsterInfo.uSpecialAttackType, actorPtr);
            }

            // set recovery after hit
            if (!pParty->bTurnBasedModeOn) {
                int actEnd = playerPtr->GetActualEndurance();
                int recTime =
                    (int)((20 - playerPtr->GetParameterBonus(actEnd)) *
                          flt_6BE3A4_debug_recmod1 * 2.133333333333333);
                playerPtr->SetRecoveryTime(recTime);
            }
            return;
        } else {
            // party hits self
            Player* playerPtr = &pParty->pPlayers[targetchar];
            int damage;
            int damagetype;
            if (uActorType != OBJECT_Player ||
                spritefrom->spell_id != SPELL_BOW_ARROW) {
                int playerMaxHp = playerPtr->GetMaxHealth();
                damage = _43AFE3_calc_spell_damage(spritefrom->spell_id, spritefrom->spell_level,
                    spritefrom->spell_skill, playerMaxHp);
                damagetype = pSpellStats->pInfos[spritefrom->spell_id].uSchool;
            } else {
                damage = pParty->pPlayers[uActorID].CalculateRangedDamageTo(0);
                damagetype = 0;
            }

            playerPtr->ReceiveDamage(damage, (DAMAGE_TYPE)damagetype);
            if (uActorType == OBJECT_Player && !_A750D8_player_speech_timer) {
                _A750D8_player_speech_timer = 256;
                PlayerSpeechID = SPEECH_44;
                uSpeakingCharacter = uActorID + 1;
            }

            return;
        }
    }
}

void Player::OnInventoryLeftClick() {
    unsigned int pickedItemId;  // esi@12
    unsigned int invItemIndex;  // eax@12
    unsigned int itemPos;       // eax@18
    ItemGen tmpItem;            // [sp+Ch] [bp-3Ch]@1

    CastSpellInfo* pSpellInfo;

    if (current_character_screen_window == WINDOW_CharacterWindow_Inventory) {
        unsigned int pY;
        unsigned int pX;
        mouse->GetClickPos(&pX, &pY);

        int inventoryYCoord = (pY - 17) / 32;
        int inventoryXCoord = (pX - 14) / 32;
        int invMatrixIndex = inventoryXCoord + (INVETORYSLOTSWIDTH * inventoryYCoord);

        if (inventoryYCoord >= 0 && inventoryYCoord < INVETORYSLOTSHEIGHT &&
            inventoryXCoord >= 0 && inventoryXCoord < INVETORYSLOTSWIDTH) {
            if (_50C9A0_IsEnchantingInProgress) {
                unsigned int enchantedItemPos =
                    this->GetItemListAtInventoryIndex(invMatrixIndex);

                if (enchantedItemPos) {
                    /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
                     *0x7Fu;
                     *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
                     *uActiveCharacter - 1;
                     *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                     *enchantedItemPos - 1;
                     *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                     *invMatrixIndex;*/
                    pSpellInfo =
                        (CastSpellInfo*)pGUIWindow_CastTargetedSpell->ptr_1C;
                    pSpellInfo->uFlags &= 0x7F;
                    pSpellInfo->uPlayerID_2 = uActiveCharacter - 1;
                    pSpellInfo->spell_target_pid = enchantedItemPos;  // - 1;
                    pSpellInfo->field_6 = (-1 - enchantedItemPos);    // check
                    ptr_50C9A4_ItemToEnchant =
                        &this->pInventoryItemList[enchantedItemPos - 1];
                    _50C9A0_IsEnchantingInProgress = 0;

                    pMessageQueue_50CBD0->Flush();

                    mouse->SetCursorImage("MICON1");
                    _50C9D0_AfterEnchClickEventId = 113;
                    _50C9D4_AfterEnchClickEventSecondParam = 0;
                    _50C9D8_AfterEnchClickEventTimeout = 256;
                }

                return;
            }

            if (ptr_50C9A4_ItemToEnchant) return;

            pickedItemId = pParty->pPickedItem.uItemID;
            invItemIndex = this->GetItemListAtInventoryIndex(invMatrixIndex);

            if (!pickedItemId) {  // no hold item
                if (!invItemIndex) {
                    return;
                } else {
                    memcpy(&pParty->pPickedItem,
                           &this->pInventoryItemList[invItemIndex - 1],
                           sizeof(pParty->pPickedItem));
                    this->RemoveItemAtInventoryIndex(invMatrixIndex);
                    pickedItemId = pParty->pPickedItem.uItemID;
                    mouse->SetCursorImage(
                        pItemsTable->pItems[pickedItemId].pIconName);
                    return;
                }
            } else {  // hold item
                if (invItemIndex) {
                    ItemGen* invItemPtr =
                        &this->pInventoryItemList[invItemIndex - 1];
                    memcpy(&tmpItem, invItemPtr, sizeof(tmpItem));
                    this->RemoveItemAtInventoryIndex(invMatrixIndex);
                    int emptyIndex =
                        this->AddItem2(invMatrixIndex, &pParty->pPickedItem);

                    if (!emptyIndex) {
                        emptyIndex = this->AddItem2(-1, &pParty->pPickedItem);
                        if (!emptyIndex) {
                            this->PutItemArInventoryIndex(tmpItem.uItemID,
                                                          invItemIndex - 1,
                                                          invMatrixIndex);
                            memcpy(invItemPtr, &tmpItem, sizeof(ItemGen));
                            return;
                        }
                    }

                    memcpy(&pParty->pPickedItem, &tmpItem, sizeof(ItemGen));
                    mouse->SetCursorImage(pParty->pPickedItem.GetIconName());
                    return;
                } else {
                    itemPos = this->AddItem(invMatrixIndex, pickedItemId);

                    if (itemPos) {
                        memcpy(&this->pInventoryItemList[itemPos - 1],
                               &pParty->pPickedItem, sizeof(ItemGen));
                        mouse->RemoveHoldingItem();
                        return;
                    }

                    // itemPos = this->AddItem(-1, pickedItemId);

                    // if ( itemPos ) {
                    //    memcpy(&this->pInventoryItemList[itemPos-1],
                    // &pParty->pPickedItem, sizeof(ItemGen));
                    //    pMouse->RemoveHoldingItem();
                    //       return;
                    // }
                }
            }  // held item or no
        }  // limits
    }      // char wind
}  // func

bool Player::IsWeak() const {
    return this->conditions_times[Condition_Weak].Valid();
}

bool Player::IsDead() const {
    return this->conditions_times[Condition_Dead].Valid();
}

bool Player::IsEradicated() const {
    return this->conditions_times[Condition_Eradicated].Valid();
}

bool Player::IsZombie() const {
    return this->conditions_times[Condition_Zombie].Valid();
}

bool Player::IsCursed() const {
    return this->conditions_times[Condition_Cursed].Valid();
}

bool Player::IsPertified() const {
    return this->conditions_times[Condition_Pertified].Valid();
}

bool Player::IsUnconcious() const {
    return this->conditions_times[Condition_Unconcious].Valid();
}

bool Player::IsAsleep() const {
    return this->conditions_times[Condition_Sleep].Valid();
}

bool Player::IsParalyzed() const {
    return this->conditions_times[Condition_Paralyzed].Valid();
}

bool Player::IsDrunk() const {
    return this->conditions_times[Condition_Drunk].Valid();
}

void Player::SetCursed(GameTime time) {
    this->conditions_times[Condition_Cursed] = time;
}

void Player::SetWeak(GameTime time) {
    this->conditions_times[Condition_Weak] = time;
}

void Player::SetAsleep(GameTime time) {
    this->conditions_times[Condition_Sleep] = time;
}

void Player::SetAfraid(GameTime time) {
    this->conditions_times[Condition_Fear] = time;
}

void Player::SetDrunk(GameTime time) {
    this->conditions_times[Condition_Drunk] = time;
}

void Player::SetInsane(GameTime time) {
    this->conditions_times[Condition_Insane] = time;
}

void Player::SetPoisonWeak(GameTime time) {
    this->conditions_times[Condition_Poison_Weak] = time;
}

void Player::SetDiseaseWeak(GameTime time) {
    this->conditions_times[Condition_Disease_Weak] = time;
}

void Player::SetPoisonMedium(GameTime time) {
    this->conditions_times[Condition_Poison_Medium] = time;
}

void Player::SetDiseaseMedium(GameTime time) {
    this->conditions_times[Condition_Disease_Medium] = time;
}

void Player::SetPoisonSevere(GameTime time) {
    this->conditions_times[Condition_Poison_Severe] = time;
}

void Player::SetDiseaseSevere(GameTime time) {
    this->conditions_times[Condition_Disease_Severe] = time;
}

void Player::SetParalyzed(GameTime time) {
    this->conditions_times[Condition_Paralyzed] = time;
}

void Player::SetUnconcious(GameTime time) {
    this->conditions_times[Condition_Unconcious] = time;
}

void Player::SetDead(GameTime time) {
    this->conditions_times[Condition_Dead] = time;
}

void Player::SetPertified(GameTime time) {
    this->conditions_times[Condition_Pertified] = time;
}

void Player::SetEradicated(GameTime time) {
    this->conditions_times[Condition_Eradicated] = time;
}

void Player::SetZombie(GameTime time) {
    this->conditions_times[Condition_Zombie] = time;
}

void Player::SetCondWeakWithBlockCheck(int blockable) {
    SetCondition(Condition_Weak, blockable);
}

void Player::SetCondInsaneWithBlockCheck(int blockable) {
    SetCondition(Condition_Insane, blockable);
}

void Player::SetCondDeadWithBlockCheck(int blockable) {
    SetCondition(Condition_Dead, blockable);
}

void Player::SetCondUnconsciousWithBlockCheck(int blockable) {
    SetCondition(Condition_Dead, blockable);
}

ItemGen* Player::GetOffHandItem() { return GetItem(&PlayerEquipment::uShield); }

ItemGen* Player::GetMainHandItem() {
    return GetItem(&PlayerEquipment::uMainHand);
}

ItemGen* Player::GetBowItem() { return GetItem(&PlayerEquipment::uBow); }

ItemGen* Player::GetArmorItem() { return GetItem(&PlayerEquipment::uArmor); }

ItemGen* Player::GetHelmItem() { return GetItem(&PlayerEquipment::uHelm); }

ItemGen* Player::GetBeltItem() { return GetItem(&PlayerEquipment::uBelt); }

ItemGen* Player::GetCloakItem() { return GetItem(&PlayerEquipment::uCloak); }

ItemGen* Player::GetGloveItem() { return GetItem(&PlayerEquipment::uGlove); }

ItemGen* Player::GetBootItem() { return GetItem(&PlayerEquipment::uBoot); }

ItemGen* Player::GetAmuletItem() { return GetItem(&PlayerEquipment::uAmulet); }

ItemGen* Player::GetNthRingItem(int ringNum) {
    return GetNthEquippedIndexItem(ringNum + 10);
}

ItemGen* Player::GetNthEquippedIndexItem(int index) {
    if (this->pEquipment.pIndices[index] == 0) {
        return nullptr;
    }

    return &this->pInventoryItemList[this->pEquipment.pIndices[index] - 1];
}

ItemGen* Player::GetItem(unsigned int PlayerEquipment::*itemPos) {
    if (this->pEquipment.*itemPos == 0) {
        return nullptr;
    }

    return &this->pInventoryItemList[this->pEquipment.*itemPos - 1];
}

int Player::GetPlayerIndex() {  // PS - RETURN PLAYER INDEX
    int uPlayerIdx = 0;

    if (this == pPlayers[1])
        uPlayerIdx = 0;
    else if (this == pPlayers[2])
        uPlayerIdx = 1;
    else if (this == pPlayers[3])
        uPlayerIdx = 2;
    else if (this == pPlayers[4])
        uPlayerIdx = 3;
    else
        Error("Unexpected player pointer");

    return uPlayerIdx;
}

//----- (004272F5) --------------------------------------------------------
bool Player::PlayerHitOrMiss(Actor* pActor, int distancemod,
                             int skillmod) {  // PS - RETURN IF ATTACK WILL HIT
    int naturalArmor = pActor->pMonsterInfo.uAC;  // actor usual armour
    int armorBuff = 0;

    if (pActor->pActorBuffs[ACTOR_BUFF_SOMETHING_THAT_HALVES_AC]
            .Active())  // gm axe effect??
        naturalArmor /= 2;

    if (pActor->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        armorBuff = pActor->pActorBuffs[ACTOR_BUFF_SHIELD].uPower;

    if (pActor->pActorBuffs[ACTOR_BUFF_STONESKIN].Active() &&
        pActor->pActorBuffs[ACTOR_BUFF_STONESKIN].uPower > armorBuff)
        armorBuff = pActor->pActorBuffs[ACTOR_BUFF_STONESKIN].uPower;

    int effectiveActorArmor = armorBuff + naturalArmor;

    int attBonus;  // player attack bonus
    if (distancemod)
        attBonus = this->GetRangedAttack();  // range
    else
        attBonus = this->GetActualAttack(false);  // melee

    int attPositiveMod =
        skillmod + rand() % (effectiveActorArmor + 2 * attBonus +
                             30);  // positive effects to hit on attack

    int attNegativeMod;  // negative effects to hit on attack

    if (distancemod == 2) {  // medium range
        attNegativeMod =
            ((effectiveActorArmor + 15) / 2) + effectiveActorArmor + 15;
    } else if (distancemod == 3) {  // far range
        attNegativeMod = 2 * effectiveActorArmor + 30;
    } else {  // close range
        attNegativeMod = effectiveActorArmor + 15;
    }

    return (attPositiveMod > attNegativeMod);  // do the pos outweight the negs
}

//----- (0042ECB5) --------------------------------------------------------
void Player::_42ECB5_PlayerAttacksActor() {
    //  char *v5; // eax@8
    //  unsigned int v9; // ecx@21
    //  char *v11; // eax@26
    //  unsigned int v12; // eax@47
    //  SoundID v24; // [sp-4h] [bp-40h]@58

    // result = pParty->pPlayers[uActiveCharacter-1].CanAct();
    Player* player = &pParty->pPlayers[uActiveCharacter - 1];
    if (!player->CanAct()) return;

    CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress();
    // v3 = 0;
    if (pParty->Invisible())
        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();

    // v31 = player->pEquipment.uBow;
    int bow_idx = player->pEquipment.uBow;
    if (bow_idx && player->pInventoryItemList[bow_idx - 1].IsBroken())
        bow_idx = 0;

    // v32 = 0;
    int wand_item_id = 0;
    // v33 = 0;

    int laser_weapon_item_id = 0;

    int main_hand_idx = player->pEquipment.uMainHand;
    if (main_hand_idx) {
        ItemGen* item = &player->pInventoryItemList[main_hand_idx - 1];
        // v5 = (char *)v1 + 36 * v4;
        if (!item->IsBroken()) {
            // v28b = &v1->pInventoryItems[v4].uItemID;
            // v6 = v1->pInventoryItems[v4].uItemID;//*((int *)v5 + 124);
            if (item->GetItemEquipType() == EQUIP_WAND) {
                if (item->uNumCharges <= 0)
                    player->pEquipment.uMainHand =
                        0;  // wand discharged - unequip
                else
                    wand_item_id = item->uItemID;  // *((int *)v5 + 124);
            } else if (item->uItemID == ITEM_BLASTER ||
                item->uItemID == ITEM_LASER_RIFLE) {
                laser_weapon_item_id = item->uItemID;  // *((int *)v5 + 124);
            }
        }
    }

    // v30 = 0;
    // v29 = 0;
    // v28 = 0;
    // v7 = pMouse->uPointingObjectID;

    int target_pid = mouse->uPointingObjectID;
    int target_type = PID_TYPE(target_pid), target_id = PID_ID(target_pid);
    if (target_type != OBJECT_Actor || !pActors[target_id].CanAct()) {
        target_pid = stru_50C198.FindClosestActor(5120, 0, 0);
        target_type = PID_TYPE(target_pid);
        target_id = PID_ID(target_pid);
    }

    Actor* actor = nullptr;
    if (target_id < 500) {
        actor = &pActors[target_id];  // prevent crash
    }

    int actor_distance = 0;
    if (target_type == OBJECT_Actor) {
        int distance_x = actor->vPosition.x - pParty->vPosition.x,
            distance_y = actor->vPosition.y - pParty->vPosition.y,
            distance_z = actor->vPosition.z - pParty->vPosition.z;
        actor_distance =
            integer_sqrt(distance_x * distance_x + distance_y * distance_y +
                         distance_z * distance_z) -
            actor->uActorRadius;
        if (actor_distance < 0) actor_distance = 0;
    }

    bool shooting_bow = false, shotting_laser = false, shooting_wand = false,
         melee_attack = false;
    if (laser_weapon_item_id) {
        shotting_laser = true;
        _42777D_CastSpell_UseWand_ShootArrow(SPELL_LASER_PROJECTILE,
                                             uActiveCharacter - 1, 0, 0,
                                             uActiveCharacter + 8);
    } else if (wand_item_id) {
        shooting_wand = true;

        int main_hand_idx = player->pEquipment.uMainHand;
        _42777D_CastSpell_UseWand_ShootArrow(
            wand_spell_ids[player->pInventoryItemList[main_hand_idx - 1]
                               .uItemID -
                           ITEM_WAND_FIRE],
            uActiveCharacter - 1, 8, 0, uActiveCharacter + 8);

        if (!--player->pInventoryItemList[main_hand_idx - 1].uNumCharges)
            player->pEquipment.uMainHand = 0;
    } else if (target_type == OBJECT_Actor && actor_distance <= 407.2) {
        melee_attack = true;

        Vec3_int_ a3;
        a3.x = actor->vPosition.x - pParty->vPosition.x;
        a3.y = actor->vPosition.y - pParty->vPosition.y;
        a3.z = actor->vPosition.z - pParty->vPosition.z;
        Vec3_int_::Normalize(&a3.x, &a3.y, &a3.z);

        Actor::DamageMonsterFromParty(PID(OBJECT_Player, uActiveCharacter - 1),
                                      target_id, &a3);
        if (player->WearsItem(ITEM_ARTIFACT_SPLITTER, EQUIP_TWO_HANDED) ||
            player->WearsItem(ITEM_ARTIFACT_SPLITTER, EQUIP_SINGLE_HANDED))
            _42FA66_do_explosive_impact(
                actor->vPosition.x, actor->vPosition.y,
                actor->vPosition.z + actor->uActorHeight / 2, 0, 512,
                uActiveCharacter);
    } else if (bow_idx) {
        shooting_bow = true;
        _42777D_CastSpell_UseWand_ShootArrow(SPELL_BOW_ARROW,
                                             uActiveCharacter - 1, 0, 0, 0);
    } else {
        melee_attack = true;
        // ; // actor out of range or no actor; no ranged weapon so melee
        // attacking air
    }

    if (!pParty->bTurnBasedModeOn &&
        melee_attack) {  // wands, bows & lasers will add recovery while shooting
                         // spell effect
        int recovery = player->GetAttackRecoveryTime(false);
        if (recovery < 30) recovery = 30;
        player->SetRecoveryTime(flt_6BE3A4_debug_recmod1 * (double)recovery *
                                2.133333333333333);
    }

    int v34 = 0;
    if (shooting_wand) {
        return;
    } else if (shooting_bow) {
        v34 = 5;
        player->PlaySound(SPEECH_50, 0);
    } else if (shotting_laser) {
        v34 = 7;
    } else {
        int main_hand_idx = player->pEquipment.uMainHand;
        if (player->HasItemEquipped(EQUIP_TWO_HANDED) & main_hand_idx)
            v34 = player->pInventoryItemList[main_hand_idx - 1].GetPlayerSkillType();

        pTurnEngine->ApplyPlayerAction();
    }

    switch (v34) {
        case 0:
            pAudioPlayer->PlaySound(SOUND_swing_with_blunt_weapon01, 0, 0, -1, 0, 0);
            break;
        case 1:
            pAudioPlayer->PlaySound(SOUND_swing_with_sword01, 0, 0, -1, 0, 0);
            break;
        case 2:
            pAudioPlayer->PlaySound(SOUND_swing_with_sword02, 0, 0, -1, 0, 0);
            break;
        case 3:
            pAudioPlayer->PlaySound(SOUND_swing_with_axe01, 0, 0, -1, 0, 0);
            break;
        case 4:
            pAudioPlayer->PlaySound(SOUND_swing_with_axe03, 0, 0, -1, 0, 0);
            break;
        case 5:
            pAudioPlayer->PlaySound(SOUND_shoot_bow01, 0, 0, -1, 0, 0);
            break;
        case 6:
            pAudioPlayer->PlaySound(SOUND_swing_with_blunt_weapon03, 0, 0, -1, 0, 0);
            break;
        case 7:
            pAudioPlayer->PlaySound(SOUND_shoot_blaster01, 0, 0, -1, 0, 0);
            break;
    }
}

//----- (0042FA66) --------------------------------------------------------
void Player::_42FA66_do_explosive_impact(int xpos, int ypos, int zpos, int a4,
                                         __int16 a5, signed int actchar) {
        // EXPLOSIVE IMPACT OF ARTIFACT SPLITTER

    // a5 is range?

    SpriteObject a1a;
    a1a.uType = SPRITE_OBJECT_EXPLODE;
    a1a.containing_item.Reset();
    a1a.spell_id = SPELL_FIRE_FIREBALL;
    a1a.spell_level = 8;
    a1a.spell_skill = 3;
    a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
    a1a.vPosition.x = xpos;
    a1a.vPosition.y = ypos;
    a1a.vPosition.z = zpos;
    a1a.uAttributes = 0;
    a1a.uSectorID = pIndoor->GetSector(xpos, ypos, zpos);
    a1a.uSpriteFrameID = 0;
    a1a.spell_target_pid = 0;
    a1a.field_60_distance_related_prolly_lod = 0;
    a1a.uFacing = 0;
    a1a.uSoundID = 0;

    if (actchar >= 1 || actchar <= 4)
        a1a.spell_caster_pid = PID(OBJECT_Player, actchar - 1);
    else
        a1a.spell_caster_pid = 0;

    int id = a1a.Create(0, 0, 0, 0);
    if (id != -1)
        AttackerInfo.Add(PID(OBJECT_Item, id), a5, (short)a1a.vPosition.x,
                         (short)a1a.vPosition.y, (short)a1a.vPosition.z, 0, 0);
}

//----- (00458244) --------------------------------------------------------
unsigned int SkillToMastery(unsigned int skill_value) {
    // attempt to depreciate ?
    // use GetActualSkillMastery instead

    switch (skill_value & 0x1C0) {
        case 0x100:
            return 4;  // Grandmaster
        case 0x80:
            return 3;  // Master
        case 0x40:
            return 2;  // Expert
        case 0x00:
            return 1;  // Normal
    }

    assert(false);  // should not get here
    return 0;
}

//----- (004948B1) --------------------------------------------------------
void Player::PlaySound(PlayerSpeech speech,
                       int a3) {         // ?? need to swith over to openal
    int speechCount = 0;                 // esi@4
    int expressionCount = 0;             // esi@4
    int pickedVariant;                   // esi@10
    CHARACTER_EXPRESSION_ID expression;  // ebx@17
    int pSoundID;                        // ecx@19
    int speechVariantArray[5];           // [sp+Ch] [bp-1Ch]@7
    int expressionVariantArray[5];
    unsigned int expressionDuration = 0;

    unsigned int pickedSoundID = 0;
    if (engine->config->voice_level > 0) {
        for (int i = 0; i < 2; i++) {
            if (SoundSetAction[speech][i]) {
                speechVariantArray[speechCount] = SoundSetAction[speech][i];
                speechCount++;
            }
        }
        if (speechCount) {
            pickedVariant = speechVariantArray[rand() % speechCount];
            int numberOfSubvariants = byte_4ECF08[pickedVariant - 1][uVoiceID];
            if (numberOfSubvariants > 0) {
                pickedSoundID = rand() % numberOfSubvariants +
                                2 * (pickedVariant + 50 * uVoiceID) + 4998;
                pAudioPlayer->PlaySound(
                    (SoundID)pickedSoundID,
                    PID(OBJECT_Player, uActiveCharacter + 39), 0, -1, 0, 0);
            }
        }
    }

    for (int i = 0; i < 5; i++) {
        if (SoundSetAction[speech][i + 3]) {
            expressionVariantArray[expressionCount] =
                SoundSetAction[speech][i + 3];
            expressionCount++;
        }
    }
    if (expressionCount) {
        expression = (CHARACTER_EXPRESSION_ID)
            expressionVariantArray[rand() % expressionCount];
        if (expression == CHARACTER_EXPRESSION_21 && pickedSoundID) {
            pSoundID = pickedSoundID;
            if (pSoundID >= 0) {
                expressionDuration = (sLastTrackLengthMS << 7) / 1000;
            }
        }
        PlayEmotion(expression, expressionDuration);
    }
}

//----- (00494A25) --------------------------------------------------------
void Player::PlayEmotion(CHARACTER_EXPRESSION_ID new_expression, int duration) {
    // 38 - sparkles 1 player?

    unsigned int currexpr = expression;

    if (expression == CHARACTER_EXPRESSION_DEAD ||
        expression == CHARACTER_EXPRESSION_ERADICATED) {
        return;  // no react
    } else if (expression == CHARACTER_EXPRESSION_PERTIFIED &&
               new_expression != CHARACTER_EXPRESSION_FALLING) {
        return;  // no react
    } else {
        if (expression != CHARACTER_EXPRESSION_SLEEP ||
            new_expression != CHARACTER_EXPRESSION_FALLING) {
            if (currexpr >= 2 && currexpr <= 11 && currexpr != 8 &&
                !(new_expression == CHARACTER_EXPRESSION_DMGRECVD_MINOR ||
                  new_expression == CHARACTER_EXPRESSION_DMGRECVD_MODERATE ||
                  new_expression == CHARACTER_EXPRESSION_DMGRECVD_MAJOR)) {
                return;  // no react
            }
        }
    }

    this->uExpressionTimePassed = 0;

    if (!duration) {
        this->uExpressionTimeLength =
            8 * pPlayerFrameTable->pFrames[duration].uAnimLength;
    } else {
        this->uExpressionTimeLength = 0;
    }

    expression = new_expression;
    viewparams->bRedrawGameUI = 1;
}

//----- (0049327B) --------------------------------------------------------
bool Player::ProfessionOrGuildFlagsCorrect(unsigned int uClass, int a3) {
    if (this->classType == uClass) {
        return true;
    } else {
        if (!a3) {
            return false;
        }
        switch (uClass) {
            case PLAYER_CLASS_PRIEST_OF_SUN:
                return (_449B57_test_bit(
                    (unsigned __int8*)this->_achieved_awards_bits, 65));
            case PLAYER_CLASS_PRIEST_OF_MOON:
                return (_449B57_test_bit(
                    (unsigned __int8*)this->_achieved_awards_bits, 67));
            case PLAYER_CLASS_ARCHMAGE:
                return (_449B57_test_bit(
                    (unsigned __int8*)this->_achieved_awards_bits, 77));
            case PLAYER_CLASS_LICH:
                return (_449B57_test_bit(
                    (unsigned __int8*)this->_achieved_awards_bits, 79));
                break;
            default:
                Error("Should not be able to get here (%u)", uClass);
                break;
        }
        return false;
    }
}

//----- (00490EEE) --------------------------------------------------------
int Player::SelectPhrasesTransaction(
    ItemGen* pItem, int building_type, int BuildID_2Events,
    int ShopMenuType) {  // TODO(_): probably move this somewhere else, not really
                         // Player:: stuff
    unsigned int idemId;   // edx@1
    signed int equipType;  // esi@1
    float multiplier;      // ST04_4@26
    int price;             // edi@26
    int merchantLevel;     // [sp+10h] [bp-8h]@1
    int itemValue;

    merchantLevel = GetActualSkillLevel(PLAYER_SKILL_MERCHANT);
    idemId = pItem->uItemID;
    equipType = pItem->GetItemEquipType();
    itemValue = pItem->GetValue();

    switch (building_type) {
        case BuildingType_WeaponShop:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS) return 5;
            if (equipType > EQUIP_BOW) return 4;
            break;
        case BuildingType_ArmorShop:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS) return 5;
            if (equipType < EQUIP_ARMOUR || equipType > EQUIP_BOOTS) return 4;
            break;
        case BuildingType_MagicShop:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS) return 5;
            if (pItemsTable->pItems[idemId].uSkillType != PLAYER_SKILL_MISC)
                return 4;
            break;
        case BuildingType_AlchemistShop:
            if ((idemId >= ITEM_ARTIFACT_HERMES_SANDALS &&
                 idemId < ITEM_RECIPE_REJUVENATION) ||
                idemId > ITEM_RECIPE_BODY_RESISTANCE)
                return 5;
            if (!(equipType == EQUIP_REAGENT || equipType == EQUIP_POTION ||
                  equipType == EQUIP_MESSAGE_SCROLL))
                return 4;
            break;
        default:
            Error("(%u)", building_type);
            break;
    }
    if (pItem->IsStolen()) return 6;

    multiplier = p2DEvents[BuildID_2Events - 1].fPriceMultiplier;
    switch (ShopMenuType) {
        case 2:
            price = GetBuyingPrice(itemValue, multiplier);
            break;
        case 3:
            // if (pItem->IsBroken())
            // price = 1;
            // else
            price = this->GetPriceSell(*pItem,
                                       multiplier);  // itemValue, multiplier);
            break;
        case 4:
            price = this->GetPriceIdentification(multiplier);
            break;
        case 5:
            price = this->GetPriceRepair(itemValue, multiplier);
            break;
        default:
            Error("(%u)", ShopMenuType);
            break;
    }
    if (merchantLevel) {
        if (price == itemValue) {
            return 3;
        } else {
            return 2;
        }
    } else {
        return 1;
    }
}

//----- (0048C6AF) --------------------------------------------------------
Player::Player() {
    memset(&pEquipment, 0, sizeof(PlayerEquipment));
    pInventoryMatrix.fill(0);
    for (uint i = 0; i < 126; ++i) pInventoryItemList[i].Reset();
    for (uint i = 0; i < 12; ++i) pEquippedItems[i].Reset();

    for (uint i = 0; i < 24; ++i) {
        pPlayerBuffs[i].uSkill = 0;
        pPlayerBuffs[i].uSkill = 0;
        pPlayerBuffs[i].uPower = 0;
        pPlayerBuffs[i].expire_time.Reset();
        pPlayerBuffs[i].uCaster = 0;
        pPlayerBuffs[i].uFlags = 0;
    }

    pName[0] = 0;
    uCurrentFace = 0;
    uVoiceID = 0;
    conditions_times.fill(GameTime(0));

    field_BB = 0;

    uMight = uMightBonus = 0;
    uIntelligence = uIntelligenceBonus = 0;
    uWillpower = uWillpowerBonus = 0;
    uEndurance = uEnduranceBonus = 0;
    uSpeed = uSpeedBonus = 0;
    uAccuracy = uAccuracyBonus = 0;
    uLuck = uLuckBonus = 0;
    uLevel = sLevelModifier = 0;
    sAgeModifier = 0;
    sACModifier = 0;

    //  memset(field_1F5, 0, 30);
    pure_luck_used = 0;
    pure_speed_used = 0;
    pure_intellect_used = 0;
    pure_endurance_used = 0;
    pure_willpower_used = 0;
    pure_accuracy_used = 0;
    pure_might_used = 0;

    sResFireBase = sResFireBonus = 0;
    sResAirBase = sResAirBonus = 0;
    sResWaterBase = sResWaterBonus = 0;
    sResEarthBase = sResEarthBonus = 0;
    sResMagicBase = sResMagicBonus = 0;
    sResSpiritBase = sResSpiritBonus = 0;
    sResMindBase = sResMindBonus = 0;
    sResBodyBase = sResBodyBonus = 0;
    sResLightBase = sResLightBonus = 0;
    sResDarkBase = sResDarkBonus = 0;

    uTimeToRecovery = 0;

    uSkillPoints = 0;

    sHealth = 0;
    uFullHealthBonus = 0;
    _health_related = 0;

    sMana = 0;
    uFullManaBonus = 0;
    _mana_related = 0;

    uQuickSpell = 0;

    _some_attack_bonus = 0;
    field_1A91 = 0;
    _melee_dmg_bonus = 0;
    field_1A93 = 0;
    _ranged_atk_bonus = 0;
    field_1A95 = 0;
    _ranged_dmg_bonus = 0;
    field_1A97 = 0;

    expression = CHARACTER_EXPRESSION_INVALID;
    uExpressionTimePassed = 0;
    uExpressionTimeLength = 0;

    uNumDivineInterventionCastsThisDay = 0;
    uNumArmageddonCasts = 0;
    uNumFireSpikeCasts = 0;

    memset(field_1988, 0, sizeof(field_1988));
    memset(playerEventBits, 0, sizeof(playerEventBits));

    field_E0 = 0;
    field_E4 = 0;
    field_E8 = 0;
    field_EC = 0;
    field_F0 = 0;
    field_F4 = 0;
    field_F8 = 0;
    field_FC = 0;
    field_100 = 0;
    field_104 = 0;

    _expression21_animtime = 0;
    _expression21_frameset = 0;

    lastOpenedSpellbookPage = 0;
}

void Player::CleanupBeacons() {
    struct delete_beacon {
        bool operator()(const LloydBeacon &beacon) const {
            return (beacon.uBeaconTime < pParty->GetPlayingTime());
        }
    };
    vBeacons.erase(std::remove_if(vBeacons.begin(), vBeacons.end(),
        [](const LloydBeacon &beacon) {
            return (beacon.uBeaconTime < pParty->GetPlayingTime());
        }), vBeacons.end()
    );
}

bool Player::SetBeacon(size_t index, size_t power) {
    int file_index = pGames_LOD->GetSubNodeIndex(pCurrentMapName);
    if (file_index < 0) {
        return false;
    }

    LloydBeacon beacon;

    beacon.image = render->TakeScreenshot(92, 68);
    beacon.uBeaconTime = GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(power));
    beacon.PartyPos_X = pParty->vPosition.x;
    beacon.PartyPos_Y = pParty->vPosition.y;
    beacon.PartyPos_Z = pParty->vPosition.z;
    beacon.PartyRot_X = pParty->sRotationZ;
    beacon.PartyRot_Y = pParty->sRotationX;
    beacon.SaveFileID = file_index;

    if (index < vBeacons.size()) {
        // overwrite so clear image
        vBeacons[index].image->Release();
        vBeacons[index] = beacon;
    } else {
        vBeacons.push_back(beacon);
    }

    return true;
}
