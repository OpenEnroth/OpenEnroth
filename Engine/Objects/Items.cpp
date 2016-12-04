#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string>

#include "Engine/Engine.h"

#include "..\..\GUI\UI\UIHouses.h"
#include "GUI/GUIButton.h"

#include "Items.h"
#include "../MapInfo.h"
#include "GUI/GUIWindow.h"
#include "Chest.h"
#include "../LOD.h"
#include "Monsters.h"
#include "../Party.h"
#include "../Tables/FactionTable.h"
#include "../Tables/StorylineTextTable.h"
#include "../texts.h"
#include "../OurMath.h"



struct ITEM_VARIATION
	{
	unsigned __int16 treasure_level;
	unsigned __int16 item_class[4];
	};


std::array<const char, 5> uItemsAmountPerShopType={ 0, 6, 8, 12, 12};

const ITEM_VARIATION shopWeap_variation_ord[15] ={
	{ 0, { 0, 0, 0, 0 }},
	{ 1, { 23, 27, 20, 20 }},
	{ 1, { 23, 24, 28, 20 }},
	{ 2, { 23, 24, 25, 20 }},
	{ 2, { 27, 27, 26, 26 }},
	{ 4, { 24, 30, 25, 27 }},
	{ 4, { 24, 30, 25, 27 }},
	{ 3, { 30, 24, 20, 20 }},
	{ 2, { 20, 20, 20, 20 }},
	{ 3, { 27, 27, 26, 26 }},
	{ 3, { 28, 28, 25, 25 }},
	{ 2, { 23, 23, 24, 24 }},
	{ 3, { 23, 23, 26, 26 }},
	{ 2, { 30, 26, 26, 26 }},
	{ 2, { 28, 25, 28, 29 }}};

const ITEM_VARIATION shopArmr_variation_ord[28] ={
	{ 1, { 35, 35, 38, 38 }},
	{ 1, { 31, 31, 31, 34 }},
	{ 1, { 35, 35, 38, 38 }},
	{ 1, { 31, 31, 32, 34 }},
	{ 2, { 35, 35, 38, 38 }},
	{ 2, { 31, 32, 32, 33 }},
	{ 2, { 35, 35, 38, 38 }},
	{ 2, { 31, 31, 32, 32 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 31, 32, 33, 34 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 31, 32, 33, 34 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 31, 31, 31 }},
	{ 2, { 35, 35, 38, 38 }},
	{ 2, { 31, 32, 34, 34 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 31, 32, 32 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 32, 32, 32, 33 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 31, 31, 32 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 33, 31, 32, 34 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 33, 31, 32, 34 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 33, 31, 32, 34 }}};



const unsigned __int16 shopMagic_treasure_lvl[14]= {0, 1, 1, 2, 2, 4, 4, 3, 2, 2, 2, 2, 2, 2};
const unsigned __int16 shopAlch_treasure_lvl[13] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 2, 2, 2, 2};

const ITEM_VARIATION shopWeap_variation_spc[15]={
	{ 0, { 0, 0, 0, 0 }},
	{ 2, { 25, 30, 20, 20}},
	{ 2, { 23, 24, 28, 20}},
	{ 3, { 23, 24, 25, 20}},
	{ 3, { 27, 27, 26, 26}},
	{ 5, { 23, 26, 28, 27}},
	{ 5, { 23, 26, 28, 27}},
	{ 4, { 30, 24, 20, 20}},
	{ 3, { 20, 20, 20, 20}},
	{ 4, { 27, 27, 26, 26}},
	{ 4, { 28, 28, 25, 25}},
	{ 4, { 23, 23, 24, 24}},
	{ 4, { 24, 24, 27, 20}},
	{ 4, { 30, 26, 26, 26}},
	{ 4, { 28, 25, 28, 29}}};

const ITEM_VARIATION shopArmr_variation_spc[28]={
	{ 2, { 35, 35, 38, 38 }},
	{ 2, { 31, 31, 31, 34 }},
	{ 2, { 35, 35, 38, 38 }},
	{ 2, { 31, 31, 32, 34 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 32, 32, 33 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 31, 32, 32 }},
	{ 5, { 35, 35, 38, 38 }},
	{ 5, { 31, 32, 33, 34 }},
	{ 5, { 35, 35, 38, 38 }},
	{ 5, { 31, 32, 33, 34 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 31, 31, 31, 31 }},
	{ 3, { 35, 35, 38, 38 }},
	{ 3, { 31, 32, 34, 34 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 31, 31, 32, 33 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 32, 32, 33, 34 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 31, 31, 31, 32 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 32, 32, 32, 32 }},
	{ 4, { 35, 35, 38, 38 }},
	{ 4, { 34, 34, 34, 34 }},
	{ 5, { 35, 35, 38, 38 }},
	{ 5, { 33, 33, 33, 33 }}
	};

const unsigned __int16 shopMagicSpc_treasure_lvl[14]  =  {0, 2, 2, 3, 3, 5, 5, 4, 3, 3, 3, 3, 3, 3};
const unsigned __int16 shopAlchSpc_treasure_lvl[13]   =  {0, 2, 2, 3, 3, 4, 4, 5, 5, 3, 2, 2, 2};


std::array< std::array<char, 14>, 7> byte_4E8168={{  //byte_4E8178
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
    { 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    { 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    { 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4},
    { 2, 2, 2, 2, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5},
    { 2, 2, 2, 2, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6},
    { 2, 2, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}}};

int sub_4BE571_AddItemToSet(int valueToAdd, int *outPutSet, int elemsAlreadyPresent, int elemsNeeded);
int dword_F8B1DC_currentShopOption; // weak

ItemGen *ptr_50C9A4_ItemToEnchant;

struct ItemsTable *pItemsTable; // 005D29E0





//----- (00439DF3) --------------------------------------------------------
int ItemGen::_439DF3_get_additional_damage(DAMAGE_TYPE *damage_type, bool *draintargetHP)
{
    *draintargetHP = false;
    *damage_type = DMGT_FIRE;
    if (uItemID == ITEM_NULL)
        return 0;

    UpdateTempBonus(pParty->uTimePlayed);
    if (uItemID == ITEM_ARTIFACT_IRON_FEATHER)
    {
        *damage_type = DMGT_ELECTR;
        return rand() % 10 + 6;
    }
    if (uItemID == ITEM_ARTIFACT_GHOULSBANE)
    {
        *damage_type = DMGT_FIRE;
        return rand() % 16 + 3;
    }
    if (uItemID == ITEM_ARTEFACT_ULLYSES)
    {
        *damage_type = DMGT_COLD;
        return rand() % 4 + 9;
    }
    if (uItemID == ITEM_RELIC_OLD_NICK)
    {
        *damage_type = DMGT_BODY;
        return 8;
    }

    switch (special_enchantment)
    {
    case ITEM_ENCHANTMENT_OF_COLD:
        *damage_type = DMGT_COLD;
        return rand() % 2 + 3;
        break;
    case ITEM_ENCHANTMENT_OF_FROST:
        *damage_type = DMGT_COLD;
        return rand() % 3 + 6;
        break;
    case ITEM_ENCHANTMENT_OF_ICE:
        *damage_type = DMGT_COLD;
        return rand() % 4 + 9;
        break;
    case ITEM_ENCHANTMENT_OF_SPARKS:
        *damage_type = DMGT_ELECTR;
        return rand() % 4 + 2;
        break;
    case ITEM_ENCHANTMENT_OF_LIGHTNING:
        *damage_type = DMGT_ELECTR;
        return rand() % 7 + 4;
        break;
    case ITEM_ENCHANTMENT_OF_THUNDERBOLTS:
        *damage_type = DMGT_ELECTR;
        return rand() % 10 + 6;
        break;
    case ITEM_ENCHANTMENT_OF_FIRE:
        *damage_type = DMGT_FIRE;
        return GetDiceResult(1, 6);
        break;
    case ITEM_ENCHANTMENT_OF_FLAME:
        *damage_type = DMGT_FIRE;
        return GetDiceResult(2, 6);
        break;
    case ITEM_ENCHANTMENT_OF_INFERNOS:
        *damage_type = DMGT_FIRE;
        return GetDiceResult(3, 6);
        break;
    case ITEM_ENCHANTMENT_OF_POISON:
        *damage_type = DMGT_BODY;
        return 5;
        break;
    case ITEM_ENCHANTMENT_OF_VENOM:
        *damage_type = DMGT_BODY;
        return 8;
        break;
    case ITEM_ENCHANTMENT_OF_ACID:
        *damage_type = DMGT_BODY;
        return 12;
        break;
    case ITEM_ENCHANTMENT_VAMPIRIC:
    case ITEM_ENCHANTMENT_OF_DARKNESS:
        *damage_type = DMGT_DARK;
        *draintargetHP = true;
        return 0;
        break;

    case ITEM_ENCHANTMENT_OF_DRAGON:
        *damage_type = DMGT_FIRE;
        return rand() % 11 + 10;
        break;
    default:
        *damage_type = DMGT_FIRE;
        return 0;
    }
}


//----- (00402F07) --------------------------------------------------------
void ItemGen::Reset()
{
  this->uHolderPlayer = 0;
  this->uAttributes = 0;
  this->uNumCharges = 0;
  this->special_enchantment = ITEM_ENCHANTMENT_NULL;
  this->m_enchantmentStrength = 0;
  this->uEnchantmentType = 0;
  this->uItemID = ITEM_NULL;
  this->uBodyAnchor = 0;
  this->uExpireTime = 0i64;
}

//----- (00458260) --------------------------------------------------------
void ItemGen::UpdateTempBonus(__int64 uTimePlayed)
{
  if ( this->uAttributes & ITEM_TEMP_BONUS )
  {
    if ( uTimePlayed > (signed __int64)this->uExpireTime )
    {
      this->uEnchantmentType = 0;
      this->special_enchantment = ITEM_ENCHANTMENT_NULL;
      this->uAttributes = this->uAttributes&(~ITEM_TEMP_BONUS);
    }
  }
}

//----- (0045814E) --------------------------------------------------------
void ItemsTable::Release()
{
  free(pMonstersTXT_Raw);
  free(pMonsterPlacementTXT_Raw);
  free(pSkillDescTXT_Raw);
  free(pSpcItemsTXT_Raw);
  free(pStdItemsTXT_Raw);
  free(pRndItemsTXT_Raw);
  free(pItemsTXT_Raw);
  free(pHostileTXT_Raw);
  free(pHistoryTXT_Raw);
  free(pPotionsTXT_Raw);
  free(pPotionNotesTXT_Raw);
  pMonstersTXT_Raw = nullptr;
  pMonsterPlacementTXT_Raw = nullptr;
  pSpcItemsTXT_Raw = nullptr;
  pSkillDescTXT_Raw = nullptr;
  pStdItemsTXT_Raw = nullptr;
  pRndItemsTXT_Raw = nullptr;
  pItemsTXT_Raw = nullptr;
  pHostileTXT_Raw = nullptr;
  pHistoryTXT_Raw = nullptr;
  pPotionsTXT_Raw = nullptr;
  pPotionNotesTXT_Raw = nullptr;
}


//----- (00456D84) --------------------------------------------------------
void ItemsTable::Initialize()
{
    std::map<std::string, ITEM_EQUIP_TYPE, ci_less> equipStatMap;
    equipStatMap["weapon"] = EQUIP_SINGLE_HANDED;
    equipStatMap["weapon2"] = EQUIP_TWO_HANDED;
    equipStatMap["weapon1or2"] = EQUIP_SINGLE_HANDED;
    equipStatMap["missile"] = EQUIP_BOW;
    equipStatMap["bow"] = EQUIP_BOW;
    equipStatMap["armor"] = EQUIP_ARMOUR;
    equipStatMap["shield"] = EQUIP_SHIELD;
    equipStatMap["helm"] = EQUIP_HELMET;
    equipStatMap["belt"] = EQUIP_BELT;
    equipStatMap["cloak"] = EQUIP_CLOAK;
    equipStatMap["gauntlets"] = EQUIP_GAUNTLETS;
    equipStatMap["boots"] = EQUIP_BOOTS;
    equipStatMap["ring"] = EQUIP_RING;
    equipStatMap["amulet"] = EQUIP_AMULET;
    equipStatMap["weaponw"] = EQUIP_WAND;
    equipStatMap["herb"] = EQUIP_REAGENT;
    equipStatMap["reagent"] = EQUIP_REAGENT;
    equipStatMap["bottle"] = EQUIP_POTION;
    equipStatMap["sscroll"] = EQUIP_SPELL_SCROLL;
    equipStatMap["book"] = EQUIP_BOOK;
    equipStatMap["mscroll"] = EQUIP_MESSAGE_SCROLL;
    equipStatMap["gold"] = EQUIP_GOLD;
    equipStatMap["gem"] = EQUIP_GEM;

    std::map<std::string, PLAYER_SKILL_TYPE, ci_less> equipSkillMap;
    equipSkillMap["staff"] = PLAYER_SKILL_STAFF;
    equipSkillMap["sword"] = PLAYER_SKILL_SWORD;
    equipSkillMap["dagger"] = PLAYER_SKILL_DAGGER;
    equipSkillMap["axe"] = PLAYER_SKILL_AXE;
    equipSkillMap["spear"] = PLAYER_SKILL_SPEAR;
    equipSkillMap["bow"] = PLAYER_SKILL_BOW;
    equipSkillMap["mace"] = PLAYER_SKILL_MACE;
    equipSkillMap["blaster"] = PLAYER_SKILL_BLASTER;
    equipSkillMap["shield"] = PLAYER_SKILL_SHIELD;
    equipSkillMap["leather"] = PLAYER_SKILL_LEATHER;
    equipSkillMap["chain"] = PLAYER_SKILL_CHAIN;
    equipSkillMap["plate"] = PLAYER_SKILL_PLATE;
    equipSkillMap["club"] = PLAYER_SKILL_CLUB;

    std::map<std::string, ITEM_MATERIAL, ci_less> materialMap;
    materialMap["artifact"] = MATERIAL_ARTEFACT;
    materialMap["relic"] = MATERIAL_RELIC;
    materialMap["special"] = MATERIAL_SPECIAL;

    char* test_string;
    int item_counter;

    pMapStats = new MapStats;
    pMapStats->Initialize();

    pMonsterStats = new MonsterStats;
    pMonsterStats->Initialize();
    pMonsterStats->InitializePlacements();

    pSpellStats = new SpellStats;
    pSpellStats->Initialize();

    LoadPotions();
    LoadPotionNotes();

    pFactionTable = new FactionTable;
    pFactionTable->Initialize();

    pStorylineText = new StorylineText;
    pStorylineText->Initialize();

    pStdItemsTXT_Raw = (char *)pEvents_LOD->LoadRaw("stditems.txt", 0);
    strtok(pStdItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    //Standard Bonuses by Group	
    for (int i = 0; i < 24; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        pEnchantments[i].pBonusStat = RemoveQuotes(tokens[0]);
        pEnchantments[i].pOfName = RemoveQuotes(tokens[1]);
        for (int j = 0; j < 9; j++)
        {
            pEnchantments[i].to_item[j] = atoi(tokens[j + 2]);
        }
    }

    memset(&pEnchantmentsSumm, 0, 36);
    for (int i = 0; i < 9; ++i)
    {
        for (int j = 0; j < 24; ++j)
            pEnchantmentsSumm[i] += pEnchantments[j].to_item[i];
    }

    //Bonus range for Standard by Level
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 6; ++i) //counted from 1
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() == 4, "Invalid number of tokens");
        bonus_ranges[i].minR = atoi(tokens[2]);
        bonus_ranges[i].maxR = atoi(tokens[3]);
    }


    pSpcItemsTXT_Raw = (char *)pEvents_LOD->LoadRaw("spcitems.txt", 0);
    strtok(pSpcItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 72; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() >= 17, "Invalid number of tokens");
        pSpecialEnchantments[i].pBonusStatement = RemoveQuotes(tokens[0]);
        pSpecialEnchantments[i].pNameAdd = RemoveQuotes(tokens[1]);
        for (int j = 0; j < 12; j++)
        {
            pSpecialEnchantments[i].to_item_apply[j] = atoi(tokens[j + 2]);
        }
        int res;
        res = atoi(tokens[14]);
        if (!res)
        {
            ++tokens[14];
            while (*tokens[14] == ' ')//fix X 2 case
                ++tokens[14];
            res = atoi(tokens[14]);
        }
        pSpecialEnchantments[i].iValue = res;
        pSpecialEnchantments[i].iTreasureLevel = tolower(tokens[15][0]) - 97;
    }

    pSpecialEnchantments_count = 71;
    memset(&pSpecialEnchantmentsSumm, 0, 96);
    for (int i = 0; i < 12; ++i)
    {
        for (unsigned int j = 0; j <= pSpecialEnchantments_count; ++j)
            pSpecialEnchantmentsSumm[i] += pSpecialEnchantments[j].to_item_apply[i];
    }

    InitializeBuildingResidents();

    pItemsTXT_Raw = (char*)pEvents_LOD->LoadRaw("items.txt", 0);
    strtok(pItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    uAllItemsCount = 0;
    item_counter = 0;
    while (item_counter < 800)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        item_counter = atoi(tokens[0]);
        uAllItemsCount = item_counter;
        pItems[item_counter].pIconName = RemoveQuotes(tokens[1]);
        pItems[item_counter].pName = RemoveQuotes(tokens[2]);
        pItems[item_counter].uValue = atoi(tokens[3]);
        auto findResult = equipStatMap.find(tokens[4]);
        pItems[item_counter].uEquipType = findResult == equipStatMap.end() ? EQUIP_NONE : findResult->second;
        auto findResult2 = equipSkillMap.find(tokens[5]);
        pItems[item_counter].uSkillType = findResult2 == equipSkillMap.end() ? PLAYER_SKILL_MISC : findResult2->second;
        auto tokens2 = Tokenize(tokens[6], 'd');
        if (tokens2.size() == 2)
        {
            pItems[item_counter].uDamageDice = atoi(tokens2[0]);
            pItems[item_counter].uDamageRoll = atoi(tokens2[1]);
        }
        else if (tolower(tokens2[0][0]) != 's')
        {
            pItems[item_counter].uDamageDice = atoi(tokens2[0]);
            pItems[item_counter].uDamageRoll = 1;
        }
        else
        {
            pItems[item_counter].uDamageDice = 0;
            pItems[item_counter].uDamageRoll = 0;
        }
        pItems[item_counter].uDamageMod = atoi(tokens[7]);
        auto findResult3 = materialMap.find(tokens[8]);
        pItems[item_counter].uMaterial = findResult3 == materialMap.end() ? MATERIAL_COMMON : findResult3->second;
        pItems[item_counter].uItemID_Rep_St = atoi(tokens[9]);
        pItems[item_counter].pUnidentifiedName = RemoveQuotes(tokens[10]);
        pItems[item_counter].uSpriteID = atoi(tokens[11]);

        pItems[item_counter]._additional_value = 0;
        pItems[item_counter]._bonus_type = 0;
        if (pItems[item_counter].uMaterial == MATERIAL_SPECIAL)
        {
            for (int ii = 0; ii < 24; ++ii)
            {
                if (!_stricmp(tokens[12], pEnchantments[ii].pOfName))
                {
                    pItems[item_counter]._bonus_type = ii + 1;
                    break;
                }
            }
            if (!pItems[item_counter]._bonus_type)
            {
                for (int ii = 0; ii < 72; ++ii)
                {
                    if (!_stricmp(tokens[12], pSpecialEnchantments[ii].pNameAdd))
                    {
                        pItems[item_counter]._additional_value = ii + 1;
                    }
                }
            }
        }

        if ((pItems[item_counter].uMaterial == MATERIAL_SPECIAL) && (pItems[item_counter]._bonus_type))
        {
            char b_s = atoi(tokens[13]);
            if (b_s)
                pItems[item_counter]._bonus_strength = b_s;
            else
                pItems[item_counter]._bonus_strength = 1;
        }
        else
            pItems[item_counter]._bonus_strength = 0;
        pItems[item_counter].uEquipX = atoi(tokens[14]);
        pItems[item_counter].uEquipY = atoi(tokens[15]);
        pItems[item_counter].pDescription = RemoveQuotes(tokens[16]);
        item_counter++;
    }

    uAllItemsCount = item_counter;
    pRndItemsTXT_Raw = (char *)pEvents_LOD->LoadRaw("rnditems.txt", 0);
    strtok(pRndItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (item_counter = 0; item_counter < 619; item_counter++)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() > 7, "Invalid number of tokens");
        item_counter = atoi(tokens[0]);
        pItems[item_counter].uChanceByTreasureLvl1 = atoi(tokens[2]);
        pItems[item_counter].uChanceByTreasureLvl2 = atoi(tokens[3]);
        pItems[item_counter].uChanceByTreasureLvl3 = atoi(tokens[4]);
        pItems[item_counter].uChanceByTreasureLvl4 = atoi(tokens[5]);
        pItems[item_counter].uChanceByTreasureLvl5 = atoi(tokens[6]);
        pItems[item_counter].uChanceByTreasureLvl6 = atoi(tokens[7]);
    }

    //ChanceByTreasureLvl Summ - to calculate chance
    memset(&uChanceByTreasureLvlSumm, 0, 24);
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 1; j < item_counter; ++j)
            uChanceByTreasureLvlSumm[i] += pItems[j].uChanceByTreasureLvl[i];
    }

    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 3; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() > 7, "Invalid number of tokens");
        switch (i)
        {
        case 0:
            uBonusChanceStandart[0] = atoi(tokens[2]);
            uBonusChanceStandart[1] = atoi(tokens[3]);
            uBonusChanceStandart[2] = atoi(tokens[4]);
            uBonusChanceStandart[3] = atoi(tokens[5]);
            uBonusChanceStandart[4] = atoi(tokens[6]);
            uBonusChanceStandart[5] = atoi(tokens[7]);
            break;
        case 1:
            uBonusChanceSpecial[0] = atoi(tokens[2]);
            uBonusChanceSpecial[1] = atoi(tokens[3]);
            uBonusChanceSpecial[2] = atoi(tokens[4]);
            uBonusChanceSpecial[3] = atoi(tokens[5]);
            uBonusChanceSpecial[4] = atoi(tokens[6]);
            uBonusChanceSpecial[5] = atoi(tokens[7]);
            break;
        case 2:
            uBonusChanceWpSpecial[0] = atoi(tokens[2]);
            uBonusChanceWpSpecial[1] = atoi(tokens[3]);
            uBonusChanceWpSpecial[2] = atoi(tokens[4]);
            uBonusChanceWpSpecial[3] = atoi(tokens[5]);
            uBonusChanceWpSpecial[4] = atoi(tokens[6]);
            uBonusChanceWpSpecial[5] = atoi(tokens[7]);
            break;
        }
    }
    free(pRndItemsTXT_Raw);
    pRndItemsTXT_Raw = nullptr;

    pSkillDescTXT_Raw = (char *)pEvents_LOD->LoadRaw("skilldes.txt", 0);
    strtok(pSkillDescTXT_Raw, "\r");
    for (int i = 0; i < 37; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() >= 6, "Invalid number of tokens");
        pSkillDesc[i] = RemoveQuotes(tokens[1]);
        pNormalSkillDesc[i] = RemoveQuotes(tokens[2]);
        pExpertSkillDesc[i] = RemoveQuotes(tokens[3]);
        pMasterSkillDesc[i] = RemoveQuotes(tokens[4]);
        pGrandSkillDesc[i] = RemoveQuotes(tokens[5]);
    }

    pStatsTXT_Raw = (char *)pEvents_LOD->LoadRaw("stats.txt", 0);
    strtok(pStatsTXT_Raw, "\r");
    for (int i = 0; i < 26; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() == 2, "Invalid number of tokens");
        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            pAttributeDescriptions[i] = RemoveQuotes(tokens[1]);
            break;
        case 7:
            pHealthPointsAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 8:
            pArmourClassAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 9:
            pSpellPointsAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 10:
            pPlayerConditionAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 11:
            pFastSpellAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 12:
            pPlayerAgeAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 13:
            pPlayerLevelAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 14:
            pPlayerExperienceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 15:
            pAttackBonusAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 16:
            pAttackDamageAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 17:
            pMissleBonusAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 18:
            pMissleDamageAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 19:
            pFireResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 20:
            pAirResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 21:
            pWaterResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 22:
            pEarthResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 23:
            pMindResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 24:
            pBodyResistanceAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        case 25:
            pSkillPointsAttributeDescription = RemoveQuotes(tokens[1]);
            break;
        }
    }

    pClassTXT_Raw = 0;
    pClassTXT_Raw = (char *)pEvents_LOD->LoadRaw("class.txt", 0);
    strtok(pClassTXT_Raw, "\r");
    for (int i = 0; i < 36; ++i)
    {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() == 3, "Invalid number of tokens");
        pClassDescriptions[i] = RemoveQuotes(tokens[1]);
    }



    ItemGen::PopulateSpecialBonusMap();
    ItemGen::PopulateArtifactBonusMap();
    ItemGen::PopulateRegularBonusMap();


}

//----- (00456D17) --------------------------------------------------------
void ItemsTable::SetSpecialBonus(ItemGen *pItem)
{
  if ( pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL )
  {
    pItem->uEnchantmentType = pItems[pItem->uItemID]._bonus_type;
    pItem->special_enchantment = (ITEM_ENCHANTMENT)pItems[pItem->uItemID]._additional_value;
    pItem->m_enchantmentStrength = pItems[pItem->uItemID]._bonus_strength;
  }
}

//----- (00456D43) --------------------------------------------------------
bool ItemsTable::IsMaterialSpecial(ItemGen *pItem)
{
  return this->pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL;
}

//----- (00456D5E) --------------------------------------------------------
bool ItemsTable::IsMaterialNonCommon(ItemGen *pItem)
{
  return pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL ||
	      pItems[pItem->uItemID].uMaterial == MATERIAL_RELIC || 
		  pItems[pItem->uItemID].uMaterial == MATERIAL_ARTEFACT;
}


//----- (00453B3C) --------------------------------------------------------
void ItemsTable::LoadPotions()
{

  CHAR Text[90]; 
  char* test_string;
  unsigned int uRow;
  unsigned int uColumn;
  unsigned __int8 potion_value;

  free(pPotionNotesTXT_Raw);
  auto tokens = Tokenize("", '\t');
  char* pPotionsTXT_Raw = (char *)pEvents_LOD->LoadRaw("potion.txt", 0);
  test_string = strtok(pPotionsTXT_Raw ,"\r") + 1;
  while (test_string)
  {
    tokens = Tokenize(test_string, '\t');
    if (!strcmp(tokens[0], "222"))    
      break;
    test_string = strtok(NULL ,"\r") + 1;
  }
  if (!test_string)
  {
    MessageBoxA(0, "Error Pre-Parsing Potion Table", "Load Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
    return;
  }

  for (uRow = 0;uRow < 50; ++uRow)
  {
    if (tokens.size() < 50)
    {
      wsprintfA(Text, "Error Parsing Potion Table at Row: %d Column: %d", uRow, tokens.size());
      MessageBoxA(0, Text, "Parsing Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
      return;
    }
    for (uColumn = 0; uColumn < 50; ++uColumn)
    {
      char* currValue = tokens[uColumn + 7];
      potion_value = atoi(currValue);
      if ( !potion_value && tolower(currValue[0]) == 'e')
      {
        potion_value = atoi(currValue + 1);
      }      
      this->potion_data[uRow][uColumn]=potion_value;
    }

    test_string = strtok(NULL ,"\r") + 1;
    if (!test_string)
    {
      wsprintfA(Text, "Error Parsing Potion Table at Row: %d Column: %d", uRow, 0);
      MessageBoxA(0, Text, "Parsing Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
      return;
    }
    tokens = Tokenize(test_string, '\t');
  }
}

//----- (00453CE5) --------------------------------------------------------
void ItemsTable::LoadPotionNotes()
{

  CHAR Text[90]; 
  char* test_string;
  unsigned int uRow;
  unsigned int uColumn;
  unsigned __int8 potion_note;

	free(pPotionNotesTXT_Raw);
  auto tokens = Tokenize("", '\t');
  char* pPotionNotesTXT_Raw = (char *)pEvents_LOD->LoadRaw("potnotes.txt", 0);
  test_string = strtok(pPotionNotesTXT_Raw ,"\r") + 1;
  while (test_string)
  {
    tokens = Tokenize(test_string, '\t');
    if (!strcmp(tokens[0], "222"))    
      break;
    test_string = strtok(NULL ,"\r") + 1;
  }
  if (!test_string)
  {
    MessageBoxA(0, "Error Pre-Parsing Potion Table", "Load Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
    return;
  }

  for (uRow = 0;uRow < 50; ++uRow)
  {
    if (tokens.size() < 50)
    {
      wsprintfA(Text, "Error Parsing Potion Table at Row: %d Column: %d", uRow, tokens.size());
      MessageBoxA(0, Text, "Parsing Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
      return;
    }
    for (uColumn = 0; uColumn < 50; ++uColumn)
    {
      char* currValue = tokens[uColumn + 7];
      potion_note = atoi(currValue);
      if ( !potion_note && tolower(currValue[0]) == 'e')
      {
          potion_note = atoi(currValue + 1);
      }      
      this->potion_note[uRow][uColumn]=potion_note;
    }

    test_string = strtok(NULL ,"\r") + 1;
    if (!test_string)
    {
      wsprintfA(Text, "Error Parsing Potion Table at Row: %d Column: %d", uRow, 0);
      MessageBoxA(0, Text, "Parsing Error", MB_ICONHAND|MB_CANCELTRYCONTINUE);
      return;
    }
    tokens = Tokenize(test_string, '\t');
  }


	}


//----- (00456442) --------------------------------------------------------
unsigned int ItemGen::GetValue()
	{
	unsigned int uBaseValue; // edi@1
	unsigned int bonus;

	uBaseValue = pItemsTable->pItems[this->uItemID].uValue;
	if ( this->uAttributes & ITEM_TEMP_BONUS || pItemsTable->IsMaterialNonCommon(this) )
		return uBaseValue;
	if (uEnchantmentType )
		return uBaseValue + 100 * m_enchantmentStrength;;
	if (special_enchantment )
		{
		bonus = pItemsTable->pSpecialEnchantments[special_enchantment].iTreasureLevel;
		if ( bonus > 10 )
			return uBaseValue + bonus;
		else
			return uBaseValue * bonus;
		} 
	return uBaseValue;
	}

//----- (00456499) --------------------------------------------------------
const char *ItemGen::GetDisplayName()
{
  if (IsIdentified())
    return GetIdentifiedName();
  else
    return pItemsTable->pItems[uItemID].pUnidentifiedName;
}

//----- (004564B3) --------------------------------------------------------
const char *ItemGen::GetIdentifiedName()
{
  unsigned __int8 equip_type; 
  const char *player_name; 
  const char *nameModificator; 
  const char *format_str; 

  equip_type = GetItemEquipType();
  if ( (equip_type == EQUIP_REAGENT) || (equip_type == EQUIP_POTION) || (equip_type == EQUIP_GOLD) )
  {
    sprintf(item__getname_buffer.data(), "%s", pItemsTable->pItems[uItemID].pName);
    return item__getname_buffer.data();
  }
  sprintf(item__getname_buffer.data(), "%s", pItemsTable->pItems[uItemID].pName);
  if ( uItemID == ITEM_LICH_JAR_FULL )  //Lich Jar
  {
    if ( (uHolderPlayer >0 )&& (uHolderPlayer <= 4) )
      {
        player_name = pPlayers[uHolderPlayer]->pName;
        if ( player_name[strlen(player_name) - 1] == 's' )
          format_str = pGlobalTXT_LocalizationStrings[655]; //"%s' Jar"
        else
          format_str = pGlobalTXT_LocalizationStrings[654]; //"%s's Jar"
		sprintf(item__getname_buffer.data(), format_str, pPlayers[uHolderPlayer]->pName);
		return item__getname_buffer.data();
      }
  }
  if ( !pItemsTable->IsMaterialNonCommon(this) )
  {
    if ( uEnchantmentType )
    {
      strcat(item__getname_buffer.data(), " ");
      nameModificator = pItemsTable->pEnchantments[uEnchantmentType-1].pOfName;
    }
    else
    {
      if ( !special_enchantment )
        return item__getname_buffer.data();
      if ( special_enchantment == 16 //Drain Hit Points from target.
        || special_enchantment == 39 //Double damage vs Demons.
        || special_enchantment == 40 //Double damage vs Dragons
        || special_enchantment == 45 //+5 Speed and Accuracy
        || special_enchantment == 56 //+5 Might and Endurance.
        || special_enchantment == 57 //+5 Intellect and Personality.
        || special_enchantment == 58 //Increased Value.
        || special_enchantment == 60 //+3 Unarmed and Dodging skills
        || special_enchantment == 61 //+3 Stealing and Disarm skills.
        || special_enchantment == 59  //Increased Weapon speed.
        || special_enchantment == 63 //Double Damage vs. Elves.
        || special_enchantment == 64 //Double Damage vs. Undead.
        || special_enchantment == 67 //Adds 5 points of Body damage and +2 Disarm skill.
        || special_enchantment == 68 ) //Adds 6-8 points of Cold damage and +5 Armor Class.
      {  //enchantment and name positions inverted!
        sprintf( item__getname_buffer.data(), "%s %s",
				  pItemsTable->pSpecialEnchantments[special_enchantment-1].pNameAdd,
				  pItemsTable->pItems[uItemID].pName);
        return item__getname_buffer.data();
      }
      strcat(item__getname_buffer.data(), " ");
	  nameModificator = pItemsTable->pSpecialEnchantments[special_enchantment-1].pNameAdd;
    }
    strcat(item__getname_buffer.data(), nameModificator);
  }
  return item__getname_buffer.data();
}


//----- (00456620) --------------------------------------------------------
void ItemsTable::GenerateItem(int treasure_level, unsigned int uTreasureType, ItemGen *out_item)
{
    int treasureLevelMinus1; // ebx@3
    int current_chance; // ebx@43
    int tmp_chance; // ecx@47
    int v17; // ebx@57
    int v18; // edx@62
    unsigned int special_chance; // edx@86
    unsigned int v26; // edx@89
    unsigned int v27; // eax@89
    int v32; // ecx@91
    int v33; // eax@91
    //    unsigned int v34; // eax@97
    int v45; // eax@120
    int v46; // edx@120
    int j; // eax@121
    int val_list[800]; // [sp+Ch] [bp-C88h]@33
    int total_chance; // [sp+C8Ch] [bp-8h]@33
    signed int v56; // [sp+CA0h] [bp+Ch]@55
    int v57; // [sp+CA0h] [bp+Ch]@62

    if (!out_item)
        out_item = (ItemGen *)malloc(sizeof(ItemGen));
    memset(out_item, 0, sizeof(*out_item));


    treasureLevelMinus1 = treasure_level - 1;
    if (uTreasureType) //generate known treasure type
    {
        ITEM_EQUIP_TYPE   requested_equip;
        PLAYER_SKILL_TYPE requested_skill = PLAYER_SKILL_INVALID;
        switch (uTreasureType)
        {
        case 20: requested_equip = EQUIP_SINGLE_HANDED; break;
        case 21: requested_equip = EQUIP_ARMOUR; break;
        case 22: requested_skill = PLAYER_SKILL_MISC; break;
        case 23: requested_skill = PLAYER_SKILL_SWORD; break;
        case 24: requested_skill = PLAYER_SKILL_DAGGER; break;
        case 25: requested_skill = PLAYER_SKILL_AXE; break;
        case 26: requested_skill = PLAYER_SKILL_SPEAR; break;
        case 27: requested_skill = PLAYER_SKILL_BOW; break;
        case 28: requested_skill = PLAYER_SKILL_MACE; break;
        case 29: requested_skill = PLAYER_SKILL_CLUB; break;
        case 30: requested_skill = PLAYER_SKILL_STAFF; break;
        case 31: requested_skill = PLAYER_SKILL_LEATHER; break;
        case 32: requested_skill = PLAYER_SKILL_CHAIN; break;
        case 33: requested_skill = PLAYER_SKILL_PLATE; break;
        case 34: requested_equip = EQUIP_SHIELD; break;
        case 35: requested_equip = EQUIP_HELMET; break;
        case 36: requested_equip = EQUIP_BELT; break;
        case 37: requested_equip = EQUIP_CLOAK; break;
        case 38: requested_equip = EQUIP_GAUNTLETS; break;
        case 39: requested_equip = EQUIP_BOOTS; break;
        case 40: requested_equip = EQUIP_RING; break;
        case 41: requested_equip = EQUIP_AMULET; break;
        case 42: requested_equip = EQUIP_WAND; break;
        case 43: requested_equip = EQUIP_SPELL_SCROLL; break;
        case 44: requested_equip = EQUIP_POTION; break;
        case 45: requested_equip = EQUIP_REAGENT; break;
        case 46: requested_equip = EQUIP_GEM; break;
        default:
            __debugbreak(); // check this condition
            requested_equip = (ITEM_EQUIP_TYPE)(uTreasureType - 1);
            break;
        }
        memset(val_list, 0, sizeof(val_list));
        total_chance = 0;
        j = 0;
        //a2a = 1;
        if (requested_skill == PLAYER_SKILL_INVALID)  // no skill for this item needed
        {
            for (uint i = 1; i < 500; ++i)
            {
                if (pItems[i].uEquipType == requested_equip)
                {
                    val_list[j] = i;
                    ++j;
                    total_chance += pItems[i].uChanceByTreasureLvl[treasure_level - 1];
                }
            }
        }
        else  //have needed skill
        {
            for (uint i = 1; i < 500; ++i)
            {
                if (pItems[i].uSkillType == requested_skill)
                {
                    val_list[j] = i;
                    ++j;
                    total_chance += pItems[i].uChanceByTreasureLvl[treasure_level - 1];
                }
            }
        }

        current_chance = 0;
        if (total_chance)
        {
            current_chance = rand() % total_chance + 1;
            tmp_chance = 0;
            j = 0;
            while (tmp_chance < current_chance)
            {
                out_item->uItemID = val_list[j];
                tmp_chance += pItems[val_list[j]].uChanceByTreasureLvl[treasure_level - 1];
                ++j;
            }
        }
        else
        {
            out_item->uItemID = ITEM_LONGSWORD_1;
        }
    }
    else
    {
        //artifact
        if (treasureLevelMinus1 == 5)
        {
            v56 = 0;
            for (int i = 0; i < 29; ++i)
                v56 += pParty->pIsArtifactFound[i];
            v17 = rand() % 29;
            if ((rand() % 100 < 5) && !pParty->pIsArtifactFound[v17] && v56 < 13)
            {
                pParty->pIsArtifactFound[v17] = 1;
                out_item->uAttributes = 0;
                out_item->uItemID = v17 + ITEM_ARTIFACT_PUCK;
                SetSpecialBonus(out_item);
                return;
            }
        }

        v57 = 0;
        v18 = rand() % this->uChanceByTreasureLvlSumm[treasure_level - 1] + 1;
        while (v57 < v18)
        {
            ++out_item->uItemID;
            v57 += pItems[out_item->uItemID].uChanceByTreasureLvl[treasureLevelMinus1];
        }
    }
    if (out_item->GetItemEquipType() == EQUIP_POTION && out_item->uItemID != ITEM_POTION_BOTTLE)
    {// if it potion set potion spec
        out_item->uEnchantmentType = 0;
        for (int i = 0; i < 2; ++i)
            out_item->uEnchantmentType += rand() % 4 + 1;
        out_item->uEnchantmentType = out_item->uEnchantmentType * treasure_level;
    }

    if (out_item->uItemID == ITEM_SPELLBOOK_LIGHT_DIVINE_INTERVENTION
        && !(unsigned __int16)_449B57_test_bit(pParty->_quest_bits, 239))
        out_item->uItemID = ITEM_SPELLBOOK_LIGHT_SUN_BURST;
    if (pItemsTable->pItems[out_item->uItemID].uItemID_Rep_St)
        out_item->uAttributes = 0;
    else
        out_item->uAttributes = 1;

    if (out_item->GetItemEquipType() != EQUIP_POTION)
    {
        out_item->special_enchantment = ITEM_ENCHANTMENT_NULL;
        out_item->uEnchantmentType = 0;
    }
    //try get special enhansment
    switch (out_item->GetItemEquipType())
    {
    case EQUIP_SINGLE_HANDED:
    case EQUIP_TWO_HANDED:
    case EQUIP_BOW:
        if (!uBonusChanceWpSpecial[treasureLevelMinus1])
            return;
        if ((uint)(rand() % 100) >= uBonusChanceWpSpecial[treasureLevelMinus1])
            return;
        break;
    case      EQUIP_ARMOUR:
    case      EQUIP_SHIELD:
    case      EQUIP_HELMET:
    case      EQUIP_BELT:
    case      EQUIP_CLOAK:
    case      EQUIP_GAUNTLETS:
    case      EQUIP_BOOTS:
    case      EQUIP_RING:

        if (!uBonusChanceStandart[treasureLevelMinus1])
            return;
        special_chance = rand() % 100;
        if (special_chance < uBonusChanceStandart[treasureLevelMinus1])
        {
            v26 = rand() % pEnchantmentsSumm[out_item->GetItemEquipType() - 3] + 1;
            v27 = 0;
            while (v27 < v26)
            {
                ++out_item->uEnchantmentType;
                v27 += pEnchantments[out_item->uEnchantmentType].to_item[out_item->GetItemEquipType() - 3];
            }

            v33 = rand() % (bonus_ranges[treasureLevelMinus1].maxR - bonus_ranges[treasureLevelMinus1].minR + 1);
            out_item->m_enchantmentStrength = v33 + bonus_ranges[treasureLevelMinus1].minR;
            v32 = out_item->uEnchantmentType - 1;
            if (v32 == 21 || v32 == 22 || v32 == 23) //Armsmaster skill, Dodge skill, Unarmed skill 
                out_item->m_enchantmentStrength = out_item->m_enchantmentStrength / 2;
            if (out_item->m_enchantmentStrength <= 0)
                out_item->m_enchantmentStrength = 1;
            return;

        }
        else if (special_chance >= uBonusChanceStandart[treasureLevelMinus1] + uBonusChanceSpecial[treasureLevelMinus1])
            return;
        break;
    case EQUIP_WAND:
        out_item->uNumCharges = rand() % 6 + out_item->GetDamageMod() + 1;
        out_item->uMaxCharges = out_item->uNumCharges;
        return;
    default:
        return;
    }

    j = 0;
    int spc_sum = 0;
    int spc;
    memset(&val_list, 0, 3200);
    for (unsigned int i = 0; i < pSpecialEnchantments_count; ++i)
    {
        int tr_lv = pSpecialEnchantments[i].iTreasureLevel;
        if ((treasure_level - 1 == 2) && (tr_lv == 1 || tr_lv == 0) ||
            (treasure_level - 1 == 3) && (tr_lv == 2 || tr_lv == 1 || tr_lv == 0) ||
            (treasure_level - 1 == 4) && (tr_lv == 3 || tr_lv == 2 || tr_lv == 1) ||
            (treasure_level - 1 == 5) && (tr_lv == 3)
            )
        {
            spc = pSpecialEnchantments[i].to_item_apply[out_item->GetItemEquipType()];
            spc_sum += spc;
            if (spc)
            {
                val_list[j++] = i;
            }
        }
    }

    v46 = rand() % spc_sum + 1;//случайные значения от 1 до spc_sum
    j = 0;
    v45 = 0;
    while (v45 < v46)
    {
        ++j;
        out_item->special_enchantment = (ITEM_ENCHANTMENT)val_list[j];
        v45 += pSpecialEnchantments[val_list[j]].to_item_apply[out_item->GetItemEquipType()];
    }
}

//----- (004505CC) --------------------------------------------------------
bool ItemGen::GenerateArtifact()
{
  signed int uNumArtifactsNotFound; // esi@1
  int artifacts_list[32]; 

  memset(artifacts_list, 0,sizeof(artifacts_list));
  uNumArtifactsNotFound = 0;

  for (int i=500;i<529;++i)
     if ( !pParty->pIsArtifactFound[i-500] )
        artifacts_list[uNumArtifactsNotFound++] = i;

  Reset();
  if ( uNumArtifactsNotFound )
  {
    uItemID = artifacts_list[rand() % uNumArtifactsNotFound];
    pItemsTable->SetSpecialBonus(this);
    return true;
  }
  else
    return false;

}

std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* >ItemGen::regularBonusMap;
std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* >ItemGen::specialBonusMap;
std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* >ItemGen::artifactBonusMap;

#define NEWBONUSINTOSPECIALLIST(x,y) AddToMap(ItemGen::specialBonusMap, enchId, x, y);
#define NEWBONUSINTOSPECIALLIST2(x,y,z) AddToMap(ItemGen::specialBonusMap, enchId, x, y, z);

#define NEWBONUSINTOREGULARLIST(x) AddToMap(ItemGen::regularBonusMap, enchId, x);

#define NEWBONUSINTOARTIFACTLIST(x,y) AddToMap(ItemGen::artifactBonusMap, itemId, x, y);
#define NEWBONUSINTOARTIFACTLIST2(x,y,z) AddToMap(ItemGen::artifactBonusMap, itemId, x, y, z);

void ItemGen::AddToMap( std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* > &maptoadd, int enchId, CHARACTER_ATTRIBUTE_TYPE attrId, int bonusValue /*= 0*/, unsigned __int16 Player::* skillPtr /*= NULL*/ )
{
  auto key = maptoadd.find(enchId);
  std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currMap;
  if (key == maptoadd.end())
  {
    currMap = new std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>;
    maptoadd[enchId] = currMap;
  }
  else
  {
    currMap = key->second;
  }
  Assert(currMap->find(attrId) == currMap->end(), "Attribute %d already present for enchantment %d", attrId, enchId);
  (*currMap)[attrId] = new CEnchantment(bonusValue, skillPtr);
}

void ItemGen::PopulateSpecialBonusMap()
{
  int enchId = 1;// of Protection, +10 to all Resistances
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 10);
  NEWBONUSINTOSPECIALLIST( CHARACTER_ATTRIBUTE_RESIST_AIR, 10);
  NEWBONUSINTOSPECIALLIST( CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
  NEWBONUSINTOSPECIALLIST( CHARACTER_ATTRIBUTE_RESIST_EARTH, 10);
  NEWBONUSINTOSPECIALLIST( CHARACTER_ATTRIBUTE_RESIST_MIND, 10);
  NEWBONUSINTOSPECIALLIST( CHARACTER_ATTRIBUTE_RESIST_BODY, 10);

  enchId = 2;//of The Gods, +10 to all Seven Statistics
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_STRENGTH, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ACCURACY, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_SPEED, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_LUCK, 10);

  enchId = 26;//of Air Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_AIR,0, &Player::skillAir);

  enchId = 27;//of Body Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_BODY,0, &Player::skillBody);

  enchId = 28;//of Dark Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_DARK,0, &Player::skillDark);

  enchId = 29;//of Earth Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_EARTH,0, &Player::skillEarth);

  enchId = 30;//of Fire Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_FIRE,0, &Player::skillFire);

  enchId = 31;//of Light Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_LIGHT,0, &Player::skillLight);

  enchId = 32;//of Mind Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_MIND,0, &Player::skillMind);

  enchId = 33;//of Spirit Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_SPIRIT,0, &Player::skillSpirit);

  enchId = 34;//of Water Magic
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_WATER,0, &Player::skillWater);

  enchId = 42;//of Doom
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_STRENGTH, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ACCURACY, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_SPEED, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_LUCK, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_HEALTH, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_MANA, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_AC_BONUS, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_MIND, 1);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_BODY, 1);

  enchId = 43;//of Earth
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_AC_BONUS, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_HEALTH, 10);

  enchId = 44;//of Life
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_HEALTH, 10);

  enchId = 45;//Rogues
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_SPEED, 5);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ACCURACY, 5);

  enchId = 46;//of The Dragon
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_STRENGTH, 25);

  enchId = 47;//of The Eclipse
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_MANA, 10);

  enchId = 48;//of The Golem
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 15);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_AC_BONUS, 5);

  enchId = 49;//of The Moon
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_LUCK, 10);

  enchId = 50;//of The Phoenix
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 30);

  enchId = 51;//of The Sky
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_MANA, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_SPEED, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);

  enchId = 52;//of The Stars
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ACCURACY, 10);

  enchId = 53;//of The Sun
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_STRENGTH, 10);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 10);

  enchId = 54;//of The Troll
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 15);

  enchId = 55;//of The Unicorn
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_LUCK, 15);

  enchId = 56;//Warriors
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_STRENGTH, 5);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 5);

  enchId = 57;//Wizards
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 5);
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 5);

  enchId = 60;//Monks'
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_DODGE, 3, &Player::skillDodge);
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_UNARMED, 3, &Player::skillUnarmed);

  enchId = 61;//Thieves'
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 3, &Player::skillStealing);
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_STEALING, 3, &Player::skillDisarmTrap);

  enchId = 62;//of Identifying
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_ITEM_ID, 3, &Player::skillItemId);
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID, 3, &Player::skillMonsterId);

  enchId = 67;//Assassins'
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 2, &Player::skillDisarmTrap);

  enchId = 68;//Barbarians'
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_AC_BONUS, 5);

  enchId = 69;//of the Storm
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, 20);

  enchId = 70;//of the Ocean
  NEWBONUSINTOSPECIALLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
  NEWBONUSINTOSPECIALLIST2(CHARACTER_ATTRIBUTE_SKILL_ALCHEMY, 2, &Player::skillAlchemy);
}

void ItemGen::PopulateRegularBonusMap()
{
  int enchId = 1;//of Might
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_STRENGTH);

  enchId = 2;//of Thought
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE);

  enchId = 3;//of Charm
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_WILLPOWER);

  enchId = 4;//of Vigor 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_ENDURANCE);

  enchId = 5;//of Precision 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_ACCURACY);

  enchId = 6;//of Speed 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SPEED);

  enchId = 7;//of Luck 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_LUCK);

  enchId = 8;//of Health 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_HEALTH);

  enchId = 9;//of Magic 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_MANA);

  enchId = 10;//of Defense 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_AC_BONUS);

  enchId = 11;//of Fire Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE);

  enchId = 12;//of Air Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_AIR);

  enchId = 13;//of Water Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_WATER);

  enchId = 14;//of Earth Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH);

  enchId = 15;//of Mind Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_MIND);

  enchId = 16;//of Body Resistance 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_RESIST_BODY);

  enchId = 17;//of Alchemy 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_ALCHEMY);

  enchId = 18;//of Stealing 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_STEALING);

  enchId = 19;//of Disarming 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM);

  enchId = 20;//of Items 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_ITEM_ID);

  enchId = 21;//of Monsters 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID);

  enchId = 22;//of Arms 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER);

  enchId = 23;//of Dodging 
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_DODGE);

  enchId = 24;//of the Fist
  NEWBONUSINTOREGULARLIST(CHARACTER_ATTRIBUTE_SKILL_UNARMED);
}

void ItemGen::PopulateArtifactBonusMap()
{
  int itemId;
  itemId = ITEM_ARTIFACT_PUCK;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 40);

  itemId = ITEM_ARTIFACT_IRON_FEATHER;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 40);

  itemId = ITEM_ARTIFACT_WALLACE;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 40);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 10);

  itemId = ITEM_ARTIFACT_CORSAIR;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_LUCK, 40);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);

  itemId = ITEM_ARTIFACT_GOVERNORS_ARMOR;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ACCURACY, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_LUCK, 10);

  itemId = ITEM_ARTIFACT_YORUBA;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 25);

  itemId = ITEM_ARTIFACT_SPLITTER;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 50);

  itemId = ITEM_ARTEFACT_ULLYSES,
    NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ACCURACY, 50);

  itemId = ITEM_ARTEFACT_HANDS_OF_THE_MASTER,
    NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_DODGE, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_UNARMED, 10);

  itemId = ITEM_ARTIFACT_LEAGUE_BOOTS;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, 40);
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_WATER, 0, &Player::skillWater);

  itemId = ITEM_ARTIFACT_RULERS_RING;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_MIND, 0, &Player::skillMind);
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_DARK, 0, &Player::skillDark);

  itemId = ITEM_RELIC_MASH;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 150);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, -40);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, -40);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, -40);

  itemId = ITEM_RELIC_ETHRICS_STAFF;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_DARK, 0, &Player::skillDark);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_MEDITATION, 15);

  itemId = ITEM_RELIC_HARECS_LEATHER;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_LUCK, 50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

  itemId = ITEM_RELIC_OLD_NICK;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);

  itemId = ITEM_RELIC_AMUCK;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 100);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 100);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_AC_BONUS, -15);

  itemId = ITEM_RELIC_GLORY_SHIELD;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, &Player::skillSpirit);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_SHIELD, 5);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

  itemId = ITEM_RELIC_KELEBRIM;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH, -30);

  itemId = ITEM_RELIC_TALEDONS_HELM;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_LIGHT, 0, &Player::skillLight);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_LUCK, -40);

  itemId = ITEM_RELIC_SCHOLARS_CAP;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_LEARNING, +15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, -50);

  itemId = ITEM_RELIC_PHYNAXIAN_CROWN;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_FIRE, 0, &Player::skillFire);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, +50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 30);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_AC_BONUS, -20);

  itemId = ITEM_RILIC_TITANS_BELT;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 75);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, -40);

  itemId = ITEM_RELIC_TWILIGHT;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, 50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_LUCK, 50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, -15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, -15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, -15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH, -15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_MIND, -15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_BODY, -15);

  itemId = ITEM_RELIC_ANIA_SELVING;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ACCURACY, 150);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_BOW, 5);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_AC_BONUS, -25);

  itemId = ITEM_RELIC_JUSTICE;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_MIND, 0, &Player::skillMind);
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_BODY, 0, &Player::skillBody);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, -40);

  itemId = ITEM_RELIC_MEKORIGS_HAMMER;
  NEWBONUSINTOARTIFACTLIST2(CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, &Player::skillSpirit);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 75);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, -50);

  itemId = ITEM_ARTIFACT_HERMES_SANDALS;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, 100);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ACCURACY, 50);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, 50);

  itemId = ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, -20);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, -20);

  itemId = ITEM_ARTIFACT_MINDS_EYE;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_WILLPOWER, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_INTELLIGENCE, 15);

  itemId = ITEM_ELVEN_CHAINMAIL;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SPEED, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ACCURACY, 15);

  itemId = ITEM_FORGE_GAUNTLETS;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_ENDURANCE, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 30);

  itemId = ITEM_ARTIFACT_HEROS_BELT;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_STRENGTH, 15);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 5);

  itemId = ITEM_ARTIFACT_LADYS_ESCORT;
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_FIRE, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_AIR, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_EARTH, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_MIND, 10);
  NEWBONUSINTOARTIFACTLIST(CHARACTER_ATTRIBUTE_RESIST_BODY, 10);
}

void ItemGen::GetItemBonusSpecialEnchantment( Player* owner, CHARACTER_ATTRIBUTE_TYPE attrToGet, int* additiveBonus, int* halfSkillBonus )
{
  auto bonusList = ItemGen::specialBonusMap.find(this->special_enchantment);
  if (bonusList == ItemGen::specialBonusMap.end())
  {
    return;
  }
  std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
  if (currList->find(attrToGet) != currList->end())
  {
    CEnchantment* currBonus = (*currList)[attrToGet];
    if (currBonus->statPtr != NULL)
    {
      if (currBonus->statBonus == 0)
      {
        *halfSkillBonus = owner->*currBonus->statPtr / 2;
      }
      else
      {
        if (*additiveBonus < currBonus->statBonus)
        {
          *additiveBonus = currBonus->statBonus;
        }
      }
    }
    else
    {
      *additiveBonus += currBonus->statBonus;
    }
  }
}

void ItemGen::GetItemBonusArtifact( Player* owner, CHARACTER_ATTRIBUTE_TYPE attrToGet, int* bonusSum )
{
  auto bonusList = ItemGen::artifactBonusMap.find(this->uItemID);
  if (bonusList == ItemGen::artifactBonusMap.end())
  {
    return;
  }
  std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
  if (currList->find(attrToGet) != currList->end())
  {
    CEnchantment* currBonus = (*currList)[attrToGet];
    if (currBonus->statPtr != NULL)
    {
      *bonusSum = owner->*currBonus->statPtr / 2;
    }
    else
    {
      *bonusSum += currBonus->statBonus;
    }
  }
}

bool ItemGen::IsRegularEnchanmentForAttribute( CHARACTER_ATTRIBUTE_TYPE attrToGet )
{
  auto bonusList = ItemGen::specialBonusMap.find(this->uEnchantmentType);
  if (bonusList == ItemGen::specialBonusMap.end())
  {
    return false;
  }
  std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
  return (currList->find(attrToGet) != currList->end());
}

ITEM_EQUIP_TYPE ItemGen::GetItemEquipType()
{
  return pItemsTable->pItems[this->uItemID].uEquipType;
}

unsigned char ItemGen::GetPlayerSkillType()
{
  return pItemsTable->pItems[this->uItemID].uSkillType;
}

char* ItemGen::GetIconName()
{
  return pItemsTable->pItems[this->uItemID].pIconName;
}

unsigned __int8 ItemGen::GetDamageDice()
{
  return pItemsTable->pItems[this->uItemID].uDamageDice;
}

unsigned __int8 ItemGen::GetDamageRoll()
{
  return pItemsTable->pItems[this->uItemID].uDamageRoll;
}

unsigned __int8 ItemGen::GetDamageMod()
{
  return pItemsTable->pItems[this->uItemID].uDamageMod;
}
//----- (004B8E3D) --------------------------------------------------------
void GenerateStandartShopItems()
{
  signed int item_count; 
  signed int shop_index; 
  int treasure_lvl; 
  int item_class; 
  int mdf;

  shop_index = (signed int)window_SpeakInHouse->ptr_1C;
  if ( uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType] )
  {
    for (item_count = 0; item_count < uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType]; ++item_count )
    {
      if (shop_index <= 14) //weapon shop
      {
        treasure_lvl = shopWeap_variation_ord[shop_index].treasure_level;
        item_class = shopWeap_variation_ord[shop_index].item_class[rand() % 4];
      }
      else if (shop_index <= 28) //armor shop
      {
        mdf = 0;
        if (item_count > 3)
          ++mdf;// rechek offsets
        treasure_lvl = shopArmr_variation_ord[2*(shop_index - 15) + mdf].treasure_level;
        item_class = shopArmr_variation_ord[2*(shop_index - 15) + mdf].item_class[rand() % 4];
      }
      else if (shop_index <= 41)  //magic shop
      {
        treasure_lvl = shopMagic_treasure_lvl[shop_index - 28];
        item_class = 22;  //misc
      }
      else if (shop_index <= 53) //alchemist shop
      {
        if (item_count < 6)
        {
          pParty->StandartItemsInShops[shop_index][item_count].Reset();
          pParty->StandartItemsInShops[shop_index][item_count].uItemID = ITEM_POTION_BOTTLE;  //potion bottle
          continue;
        }
        else
        {
          treasure_lvl = shopAlch_treasure_lvl[shop_index - 41];
          item_class = 45;  //reagent
        }
      }
      pItemsTable->GenerateItem(treasure_lvl, item_class, &pParty->StandartItemsInShops[shop_index][item_count]);
      pParty->StandartItemsInShops[shop_index][item_count].SetIdentified();  //identified
    }
  }
  pParty->InTheShopFlags[shop_index] = 0;
}

//----- (004B8F94) --------------------------------------------------------
void  GenerateSpecialShopItems()
{
  signed int item_count; 
  signed int shop_index; 
  int treasure_lvl; 
  int item_class; 
  int mdf;

  shop_index = (signed int)window_SpeakInHouse->ptr_1C;
  if ( uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType] )
  {
    for ( item_count = 0; item_count < uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType]; ++item_count )
    {
      if (shop_index <= 14) //weapon shop
      {
        treasure_lvl = shopWeap_variation_spc[shop_index].treasure_level;
        item_class =  shopWeap_variation_spc[shop_index].item_class[rand() % 4];
      }
      else if (shop_index <= 28) //armor shop
      {
        mdf = 0;
        if (item_count > 3)
          ++mdf;
        treasure_lvl = shopArmr_variation_spc[2*(shop_index - 15) + mdf].treasure_level;
        item_class = shopArmr_variation_spc[2*(shop_index - 15) + mdf].item_class[rand() % 4];
      }
      else if (shop_index <= 41)  //magic shop
      {
        treasure_lvl = shopMagicSpc_treasure_lvl[shop_index - 28];
        item_class = 22;  //misc
      }
      else if (shop_index <= 53) //alchemist shop
      {
        if (item_count < 6)
        {
          pParty->SpecialItemsInShops[shop_index][item_count].Reset();
          pParty->SpecialItemsInShops[shop_index][item_count].uItemID = rand() % 32 + ITEM_RECIPE_REJUVENATION;  //mscrool
          continue;
        }
        else
        {
          treasure_lvl = shopAlchSpc_treasure_lvl[shop_index - 41];
          item_class = 44;  //potion
        }
      }
      pItemsTable->GenerateItem(treasure_lvl, item_class, &pParty->SpecialItemsInShops[shop_index][item_count]);
      pParty->SpecialItemsInShops[shop_index][item_count].SetIdentified();  //identified
    }
  }
  pParty->InTheShopFlags[shop_index] = 0;
}


//----- (00450218) --------------------------------------------------------
void GenerateItemsInChest()
{
    unsigned int mapType; // eax@1
    MapInfo *currMapInfo; // esi@1
    ItemGen *currItem; // ebx@2
    int additionaItemCount; // ebp@4
    int treasureLevelBot; // edi@4
    int treasureLevelTop; // esi@4
    signed int treasureLevelRange; // esi@4
    int resultTreasureLevel; // edx@4
    int goldAmount; // esi@8
    int v11; // ebp@25
    int v12; // esi@25
    signed int whatToGenerateProb; // [sp+10h] [bp-18h]@1

    mapType = pMapStats->GetMapInfo(pCurrentMapName);
    currMapInfo = &pMapStats->pInfos[mapType];
    for (int i = 1; i < 20; ++i)
    {
        for (int j = 0; j < 140; ++j)
        {

            currItem = &pChests[i].igChestItems[j];
            if (currItem->uItemID < 0)
            {
                additionaItemCount = rand() % 5; //additional items in chect
                treasureLevelBot = byte_4E8168[abs(currItem->uItemID) - 1][2 * currMapInfo->Treasure_prob];
                treasureLevelTop = byte_4E8168[abs(currItem->uItemID) - 1][2 * currMapInfo->Treasure_prob + 1];
                treasureLevelRange = treasureLevelTop - treasureLevelBot + 1;
                resultTreasureLevel = treasureLevelBot + rand() % treasureLevelRange;  //treasure level 
                if (resultTreasureLevel < 7)
                {
                    v11 = 0;
                    do
                    {
                        whatToGenerateProb = rand() % 100;
                        if (whatToGenerateProb < 20)
                        {
                            currItem->Reset();
                        }
                        else if (whatToGenerateProb < 60) //generate gold
                        {
                            goldAmount = 0;
                            currItem->Reset();
                            switch (resultTreasureLevel)
                            {
                            case 1:
                                goldAmount = rand() % 51 + 50;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case 2:
                                goldAmount = rand() % 101 + 100;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case 3:
                                goldAmount = rand() % 301 + 200;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case 4:
                                goldAmount = rand() % 501 + 500;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case 5:
                                goldAmount = rand() % 1001 + 1000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            case 6:
                                goldAmount = rand() % 3001 + 2000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            }
                            currItem->SetIdentified();
                            currItem->special_enchantment = (ITEM_ENCHANTMENT)goldAmount;
                        }
                        else
                        {
                            pItemsTable->GenerateItem(resultTreasureLevel, 0, currItem);
                        }
                        v12 = 0;
                        while (!(pChests[i].igChestItems[v12].uItemID == ITEM_NULL) && (v12 < 140))
                        {
                            ++v12;
                        }
                        if (v12 >= 140)
                            break;
                        currItem = &pChests[i].igChestItems[v12];
                        v11++;
                    } while (v11 < additionaItemCount + 1); // + 1 because it's the item at pChests[i].igChestItems[j] and the additional ones
                }
                else
                    currItem->GenerateArtifact();
            }
        }
    }

}


	

// 4505CC: using guessed type int var_A0[32];
	//----- (004B3703) --------------------------------------------------------
void FillAviableSkillsToTeach( int _this )
	{
	const char *v30; // ecx@65
	unsigned int v29; // edx@56
	int v15; // ecx@19
	int v33; // [sp-4h] [bp-2Ch]@23
	int v34; // [sp-4h] [bp-2Ch]@43
	int v21; // ecx@34
	int v35[5]; // [sp+Ch] [bp-1Ch]@8
	int v37=0; // [sp+24h] [bp-4h]@1*
	int i=0;

	dword_F8B1DC_currentShopOption = 0;

	switch (_this)
		{
	case 1:  //shop weapon
		for (int i=0; i<2; ++i)
			{
			for (int j=0; j<4; ++j)
				{
				if ( i )
					v21 = shopWeap_variation_spc[(unsigned int)window_SpeakInHouse->ptr_1C].item_class[j];
				else
					v21 = shopWeap_variation_ord[(unsigned int)window_SpeakInHouse->ptr_1C].item_class[j];

				switch (v21)
					{
				case 23:  v34 = 37;	break;
				case 24:  v34 = 38;	break;
				case 25:  v34 = 39;	break;
				case 26:  v34 = 40;	break;
				case 27:  v34 = 41;	break;
				case 28:  v34 = 42; break;
				case 30:  v34 = 36;	break;
				default:
					continue;
					}	
				v37 = sub_4BE571_AddItemToSet(v34, v35, v37, 5);
				}
			}
		break;
	case 2: //shop armor

		for (int i=0; i<2; ++i)
			{
			for (int j=0; j<2; ++j)
				{
				for (int k=0; k<4; ++k)
					{
					if ( i )
						v15 = shopArmr_variation_spc[(unsigned int)window_SpeakInHouse->ptr_1C-15+j].item_class[k];
					else
						v15 = shopArmr_variation_ord[(unsigned int)window_SpeakInHouse->ptr_1C-15+j].item_class[k];
					switch (v15)
						{
					case 31: v33 = 45; break;
					case 32: v33 = 46; break;
					case 33: v33 = 47; break;
					case 34: v33 = 44; break;
					default:
						continue;
						}
					v37 = sub_4BE571_AddItemToSet(v33, v35, v37, 5);
					}
				}
			}
		break;
	case 3:  //shop magic
		v37 = 2;
		v35[0] = 57;
		v35[1] = 59;
		break;
	case 4: //shop alchemist
		v37 = 2;
		v35[0] = 71;
		v35[1] = 68;
		break;
	case 21:  //tavern
		v37 = 3;
		v35[0] = 70;
		v35[1] = 65;
		v35[2] = 62;
		break;
	case 23:  //temple
		v37 = 3;
		v35[0] = 67;
		v35[1] = 66;
		v35[2] = 58;
		break;
	case 30:  ///trainig
		v37 = 2;
		v35[0] = 69;
		v35[1] = 60;
		break;
		}
	for(i=0;i<v37;++i) 
		{
		v29=v35[i];
		switch(v29)
			{
		case 40 :v30 = pSkillNames[4];	break;
		case 5 : v30 = pSkillNames[23];	break;
		case 36 :v30 = pSkillNames[0];	break;
		case 37 :v30 = pSkillNames[1];	break;
		case 38 :v30 = pSkillNames[2];	break;
		case 39 :v30 = pSkillNames[3];	break;
		case 41 :v30 = pSkillNames[5];	break;
		case 42 :v30 = pSkillNames[6];	break;
		case 44 :v30 = pSkillNames[8];	break;
		case 45 :v30 = pSkillNames[9];	break;
		case 46 :v30 = pSkillNames[10];	break;
		case 47 :v30 = pSkillNames[11];	break;
		case 66 :v30 = pSkillNames[30];	break;
		case 57 :v30 = pSkillNames[21];	break;
		case 58 :v30 = pSkillNames[22];	break;
		case 60 :v30 = pSkillNames[24];	break;
		case 62 :v30 = pSkillNames[26];	break;
		case 65 :v30 = pSkillNames[29];	break;
		case 67:v30 = pSkillNames[31];	break;
		case 68:v30 = pSkillNames[32];	break;
		case 69:v30 = pSkillNames[33];	break;
		case 70:v30 = pSkillNames[34];	break;
		case 71:v30 = pSkillNames[35]; break;
		default:
			v30 = pGlobalTXT_LocalizationStrings[127]; //"No Text!"
			}
		pShopOptions[dword_F8B1DC_currentShopOption] = const_cast<char *>(v30);
		++dword_F8B1DC_currentShopOption;
		CreateButtonInColumn(i+1, v29);
		}
	pDialogueWindow->_41D08F_set_keyboard_control_group(i, 1, 0, 2);
	dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
	}

	//----- (004BE571) --------------------------------------------------------
int sub_4BE571_AddItemToSet(int valueToAdd, int *outPutSet, int elemsAlreadyPresent, int elemsNeeded)
{
	int i; // esi@3

	if ( elemsAlreadyPresent < elemsNeeded )
	{
		for ( i = 0; i < elemsAlreadyPresent; ++i )
    {
      if ( valueToAdd == outPutSet[i] )
        return elemsAlreadyPresent;
    }
    outPutSet[elemsAlreadyPresent] = valueToAdd;
    return elemsAlreadyPresent + 1;
	}
  return  elemsNeeded;
}
//----- (0043C91D) --------------------------------------------------------
int GetItemTextureFilename(char *pOut, signed int item_id, int index, int shoulder)
{
  int result; // eax@2
  ITEM_EQUIP_TYPE pEquipType;

  result = 0; //BUG   fn is void
  pEquipType = pItemsTable->pItems[item_id].uEquipType;
  if ( item_id > 500 )
  {
    switch ( item_id )
    {
    case ITEM_RELIC_HARECS_LEATHER:
      if (byte_5111F6_OwnedArtifacts[2] != 0)
        item_id = 234;
      break;
    case ITEM_ARTIFACT_YORUBA:
      if (byte_5111F6_OwnedArtifacts[1] != 0)
        item_id = 236;
      break;
    case ITEM_ARTIFACT_GOVERNORS_ARMOR:
      if (byte_5111F6_OwnedArtifacts[0] != 0)
        item_id = 235;
      break;
    case ITEM_ELVEN_CHAINMAIL:
      if (byte_5111F6_OwnedArtifacts[16] != 0)
        item_id = 73;
      break;
    case ITEM_ARTIFACT_LEAGUE_BOOTS:
      if (byte_5111F6_OwnedArtifacts[3] != 0)
        item_id = 312;
      break;
    case ITEM_RELIC_TALEDONS_HELM:
      if (byte_5111F6_OwnedArtifacts[4] != 0)
        item_id = 239;
      break;
    case ITEM_RELIC_SCHOLARS_CAP:
      if (byte_5111F6_OwnedArtifacts[5] != 0)
        item_id = 240;
      break;
    case ITEM_RELIC_PHYNAXIAN_CROWN:
      if (byte_5111F6_OwnedArtifacts[6] != 0)
        item_id = 241;
      break;
    case ITEM_ARTIFACT_MINDS_EYE:
      if (byte_5111F6_OwnedArtifacts[7] != 0)
        item_id = 93;
      break;
    case ITEM_RARE_SHADOWS_MASK:
      if (byte_5111F6_OwnedArtifacts[8] != 0)
        item_id = 344;
      break;
    case ITEM_RILIC_TITANS_BELT:
      if (byte_5111F6_OwnedArtifacts[9] != 0)
        item_id = 324;
      break;
    case ITEM_ARTIFACT_HEROS_BELT:
      if (byte_5111F6_OwnedArtifacts[10] != 0)
        item_id = 104;
      break;
    case ITEM_RELIC_TWILIGHT:
      if (byte_5111F6_OwnedArtifacts[11] != 0)
        item_id = 325;
      break;
    case ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP:
      if (byte_5111F6_OwnedArtifacts[12] != 0)
        item_id = 330;
      break;
    case ITEM_RARE_SUN_CLOAK:
      if (byte_5111F6_OwnedArtifacts[13] != 0)
        item_id = 347;
      break;
    case ITEM_RARE_MOON_CLOAK:
      if (byte_5111F6_OwnedArtifacts[14] != 0)
        item_id = 348;
      break;
    case ITEM_RARE_VAMPIRES_CAPE:
      if (byte_5111F6_OwnedArtifacts[15] != 0)
        item_id = 350;
      break;
    default:
      return 0;
    }
  }

  switch (pEquipType)
  {
  case EQUIP_ARMOUR:
    if ( !shoulder )
      return sprintf(pOut, "item%3.3dv%d", item_id, index);
    else if ( shoulder == 1 )
      return sprintf(pOut, "item%3.3dv%da1", item_id, index);
    else if ( shoulder == 2 )
      return sprintf(pOut, "item%3.3dv%da2", item_id, index);
    break;
  case EQUIP_CLOAK:
    if ( !shoulder )
      return sprintf(pOut, "item%3.3dv%d", item_id, index);
    else
      return sprintf(pOut, "item%3.3dv%da1", item_id, index);
  default:
    return sprintf(pOut, "item%3.3dv%d", item_id, index);
  }

  result = item_id - 504;
  return result;
}


//----- (004BDAAF) --------------------------------------------------------
bool ItemGen::MerchandiseTest(int _2da_idx)
{
  bool test;

  if ( (p2DEvents[_2da_idx - 1].uType != 4 || (signed int)this->uItemID < 740 || (signed int)this->uItemID > 771)
    && ((signed int)this->uItemID >= 600 || (signed int)this->uItemID >= 529 && (signed int)this->uItemID <= 599) || this->IsStolen())
    return false;
  switch( p2DEvents[_2da_idx - 1].uType )
  {
  case BuildingType_WeaponShop:
    {
      test = this->GetItemEquipType() <= EQUIP_BOW;
      break;
    }
  case BuildingType_ArmorShop:
    {
      test = this->GetItemEquipType() >= EQUIP_ARMOUR && this->GetItemEquipType() <= EQUIP_BOOTS;
      break;
    }
  case BuildingType_MagicShop:
    {
      test = this->GetPlayerSkillType() == PLAYER_SKILL_MISC || this->GetItemEquipType() == EQIUP_ANY;
      break;
    }
  case BuildingType_AlchemistShop:
    {
      test = this->GetItemEquipType() == EQUIP_REAGENT || this->GetItemEquipType() == EQUIP_POTION 
        || (this->GetItemEquipType() > EQUIP_POTION && !(this->GetItemEquipType() != EQUIP_MESSAGE_SCROLL 
        || (signed int)this->uItemID < 740) && this->uItemID != 771);
      break;
    }
  default:
    {
      test = false;
      break;
    }
  }
  return test;
}

//----- (00493F79) --------------------------------------------------------
void init_summoned_item(stru351_summoned_item *_this, __int64 duration)
{
	signed __int64 v2; // ST2C_8@1
	signed __int64 v3; // qax@1
	//signed __int64 v4; // ST1C_8@1
	unsigned __int64 v5; // qax@1
	unsigned int v6; // ebx@1

	v2 = (signed __int64)((double)duration * 0.234375);
	v3 = v2 / 60 / 60;
	//v4 = v3;
	v5 = (unsigned int)v3 / 0x18;
	v6 = (unsigned int)(v5 / 7) >> 2;
	_this->field_0_expire_second = v2 % 60;
	_this->field_4_expire_minute = v2 / 60 % 60;
	_this->field_8_expire_hour = v3 % 24;
	_this->field_10_expire_week = v5 / 7 & 3;
	_this->field_C_expire_day = (unsigned int)v5 % 0x1C;
	_this->field_14_exprie_month = v6 % 0xC;
	_this->field_18_expire_year = v6 / 0xC + game_starting_year;
}
