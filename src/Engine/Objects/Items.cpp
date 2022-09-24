#include "Engine/Objects/Items.h"

#include <map>
#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/ItemEnchantment.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/PlayerEnums.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/StorylineTextTable.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIHouses.h"


std::array<std::array<char, 14>, 7> byte_4E8168 = {{  // byte_4E8178 -treasure levles
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},
    {{1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3}},
    {{2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4}},
    {{2, 2, 2, 2, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5}},
    {{2, 2, 2, 2, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6}},
    {{2, 2, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}}
}};

ItemGen* ptr_50C9A4_ItemToEnchant;

struct ItemsTable* pItemsTable;  // 005D29E0

extern std::vector<char*> Tokenize(char* input, const char separator);


static void AddToMap(
    std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>*>& maptoadd,
    int enchId, CHARACTER_ATTRIBUTE_TYPE attrId, int bonusValue = 0,
    uint16_t Player::* skillPtr = nullptr);
static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>*> regularBonusMap;
static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>*> specialBonusMap;
static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>*> artifactBonusMap;

//----- (00439DF3) --------------------------------------------------------
int ItemGen::_439DF3_get_additional_damage(DAMAGE_TYPE* damage_type,
                                           bool* draintargetHP) {
    *draintargetHP = false;
    *damage_type = DMGT_FIRE;
    if (uItemID == ITEM_NULL) return 0;

    UpdateTempBonus(pParty->GetPlayingTime());
    if (uItemID == ITEM_ARTIFACT_IRON_FEATHER) {
        *damage_type = DMGT_ELECTR;
        return rand() % 10 + 6;
    }
    if (uItemID == ITEM_ARTIFACT_GHOULSBANE) {
        *damage_type = DMGT_FIRE;
        return rand() % 16 + 3;
    }
    if (uItemID == ITEM_ARTIFACT_ULLYSES) {
        *damage_type = DMGT_COLD;
        return rand() % 4 + 9;
    }
    if (uItemID == ITEM_RELIC_OLD_NICK) {
        *damage_type = DMGT_BODY;
        return 8;
    }

    switch (special_enchantment) {
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
void ItemGen::Reset() {
    *this = ItemGen();
}

//----- (00458260) --------------------------------------------------------
void ItemGen::UpdateTempBonus(GameTime time) {
    if (this->uAttributes & ITEM_TEMP_BONUS) {
        if (time > this->uExpireTime) {
            this->uEnchantmentType = 0;
            this->special_enchantment = ITEM_ENCHANTMENT_NULL;
            this->uAttributes &= ~ITEM_TEMP_BONUS;
        }
    }
}

//----- (0045814E) --------------------------------------------------------
void ItemsTable::Release() {
    free(pMonstersTXT_Raw);
    free(pMonsterPlacementTXT_Raw);
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
    pStdItemsTXT_Raw = nullptr;
    pRndItemsTXT_Raw = nullptr;
    pItemsTXT_Raw = nullptr;
    pHostileTXT_Raw = nullptr;
    pHistoryTXT_Raw = nullptr;
    pPotionsTXT_Raw = nullptr;
    pPotionNotesTXT_Raw = nullptr;
}

//----- (00456D84) --------------------------------------------------------
void ItemsTable::Initialize() {
    std::map<std::string, ITEM_EQUIP_TYPE, ILess> equipStatMap;
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

    std::map<std::string, PLAYER_SKILL_TYPE, ILess> equipSkillMap;
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

    std::map<std::string, ITEM_MATERIAL, ILess> materialMap;
    materialMap["artifact"] = MATERIAL_ARTIFACT;
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

    pStdItemsTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("stditems.txt");
    strtok(pStdItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    // Standard Bonuses by Group
    for (int i = 0; i < 24; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        pEnchantments[i].pBonusStat = RemoveQuotes(tokens[0]);
        pEnchantments[i].pOfName = RemoveQuotes(tokens[1]);
        for (int j = 0; j < 9; j++) {
            pEnchantments[i].to_item[j] = atoi(tokens[j + 2]);
        }
    }

    memset(&pEnchantmentsSumm, 0, 36);
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 24; ++j)
            pEnchantmentsSumm[i] += pEnchantments[j].to_item[i];
    }

    // Bonus range for Standard by Level
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 6; ++i) {  // counted from 1
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() == 4, "Invalid number of tokens");
        bonus_ranges[i].minR = atoi(tokens[2]);
        bonus_ranges[i].maxR = atoi(tokens[3]);
    }

    pSpcItemsTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("spcitems.txt");
    strtok(pSpcItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 72; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() >= 17, "Invalid number of tokens");
        pSpecialEnchantments[i].pBonusStatement = RemoveQuotes(tokens[0]);
        pSpecialEnchantments[i].pNameAdd = RemoveQuotes(tokens[1]);
        for (int j = 0; j < 12; j++) {
            pSpecialEnchantments[i].to_item_apply[j] = atoi(tokens[j + 2]);
        }
        int res;
        res = atoi(tokens[14]);
        int mask = 0;
        if (!res) {
            ++tokens[14];
            while (*tokens[14] == ' ')  // fix X 2 case
                ++tokens[14];
            res = atoi(tokens[14]);
            mask = 4;  // bit encode for when we need to multuply value
        }
        pSpecialEnchantments[i].iValue = res;
        pSpecialEnchantments[i].iTreasureLevel = (tolower(tokens[15][0]) - 97) | mask;
    }

    pSpecialEnchantments_count = 72;
    memset(&pSpecialEnchantmentsSumm, 0, 96);
    for (int i = 0; i < 12; ++i) {
        for (unsigned int j = 0; j < pSpecialEnchantments_count; ++j)
            pSpecialEnchantmentsSumm[i] += pSpecialEnchantments[j].to_item_apply[i];
    }

    InitializeBuildingResidents();

    pItemsTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("items.txt");
    strtok(pItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    uAllItemsCount = 0;
    item_counter = 0;
    while (item_counter < 800) {
        test_string = strtok(NULL, "\r") + 1;

        extern std::vector<char*> Tokenize(char* input, const char separator);
        auto tokens = Tokenize(test_string, '\t');

        item_counter = atoi(tokens[0]);
        uAllItemsCount = item_counter;
        pItems[item_counter].pIconName = RemoveQuotes(tokens[1]);
        pItems[item_counter].pName = RemoveQuotes(tokens[2]);
        pItems[item_counter].uValue = atoi(tokens[3]);
        auto findResult = equipStatMap.find(tokens[4]);
        pItems[item_counter].uEquipType =
            findResult == equipStatMap.end() ? EQUIP_NONE : findResult->second;
        auto findResult2 = equipSkillMap.find(tokens[5]);
        pItems[item_counter].uSkillType = findResult2 == equipSkillMap.end()
                                              ? PLAYER_SKILL_MISC
                                              : findResult2->second;
        auto tokens2 = Tokenize(tokens[6], 'd');
        if (tokens2.size() == 2) {
            pItems[item_counter].uDamageDice = atoi(tokens2[0]);
            pItems[item_counter].uDamageRoll = atoi(tokens2[1]);
        } else if (tolower(tokens2[0][0]) != 's') {
            pItems[item_counter].uDamageDice = atoi(tokens2[0]);
            pItems[item_counter].uDamageRoll = 1;
        } else {
            pItems[item_counter].uDamageDice = 0;
            pItems[item_counter].uDamageRoll = 0;
        }
        pItems[item_counter].uDamageMod = atoi(tokens[7]);
        auto findResult3 = materialMap.find(tokens[8]);
        pItems[item_counter].uMaterial = findResult3 == materialMap.end()
                                             ? MATERIAL_COMMON
                                             : findResult3->second;
        pItems[item_counter].uItemID_Rep_St = atoi(tokens[9]);
        pItems[item_counter].pUnidentifiedName = RemoveQuotes(tokens[10]);
        pItems[item_counter].uSpriteID = atoi(tokens[11]);

        pItems[item_counter]._additional_value = 0;
        pItems[item_counter]._bonus_type = 0;
        if (pItems[item_counter].uMaterial == MATERIAL_SPECIAL) {
            for (int ii = 0; ii < 24; ++ii) {
                if (iequals(tokens[12], pEnchantments[ii].pOfName)) {
                    pItems[item_counter]._bonus_type = ii + 1;
                    break;
                }
            }
            if (!pItems[item_counter]._bonus_type) {
                for (int ii = 0; ii < 72; ++ii) {
                    if (iequals(tokens[12], pSpecialEnchantments[ii].pNameAdd)) {
                        pItems[item_counter]._additional_value = ii + 1;
                    }
                }
            }
        }

        if ((pItems[item_counter].uMaterial == MATERIAL_SPECIAL) &&
            (pItems[item_counter]._bonus_type)) {
            char b_s = atoi(tokens[13]);
            if (b_s)
                pItems[item_counter]._bonus_strength = b_s;
            else
                pItems[item_counter]._bonus_strength = 1;
        } else {
            pItems[item_counter]._bonus_strength = 0;
        }
        pItems[item_counter].uEquipX = atoi(tokens[14]);
        pItems[item_counter].uEquipY = atoi(tokens[15]);
        pItems[item_counter].pDescription = RemoveQuotes(tokens[16]);
        item_counter++;
    }

    uAllItemsCount = item_counter;
    pRndItemsTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("rnditems.txt");
    strtok(pRndItemsTXT_Raw, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (item_counter = 0; item_counter < 619; item_counter++) {
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

    // ChanceByTreasureLvl Summ - to calculate chance
    memset(&uChanceByTreasureLvlSumm, 0, 24);
    for (int i = 0; i < 6; ++i) {
        for (int j = 1; j < item_counter; ++j)
            uChanceByTreasureLvlSumm[i] += pItems[j].uChanceByTreasureLvl[i];
    }

    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (int i = 0; i < 3; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() > 7, "Invalid number of tokens");
        switch (i) {
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

    ItemGen::PopulateSpecialBonusMap();
    ItemGen::PopulateArtifactBonusMap();
    ItemGen::PopulateRegularBonusMap();
}

//----- (00456D17) --------------------------------------------------------
void ItemsTable::SetSpecialBonus(ItemGen* pItem) {
    if (pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL) {
        pItem->uEnchantmentType = pItems[pItem->uItemID]._bonus_type;
        pItem->special_enchantment =
            (ITEM_ENCHANTMENT)pItems[pItem->uItemID]._additional_value;
        pItem->m_enchantmentStrength = pItems[pItem->uItemID]._bonus_strength;
    }
}

//----- (00456D43) --------------------------------------------------------
bool ItemsTable::IsMaterialSpecial(ItemGen* pItem) {
    return this->pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL;
}

//----- (00456D5E) --------------------------------------------------------
bool ItemsTable::IsMaterialNonCommon(ItemGen* pItem) {
    return pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_RELIC ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_ARTIFACT;
}

//----- (00453B3C) --------------------------------------------------------
void ItemsTable::LoadPotions() {
    //    char Text[90];
    char* test_string;
    unsigned int uRow;
    unsigned int uColumn;
    unsigned __int8 potion_value;

    free(pPotionNotesTXT_Raw);
    std::vector<char *> tokens;
    char* pPotionsTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("potion.txt");
    test_string = strtok(pPotionsTXT_Raw, "\r") + 1;
    while (test_string) {
        tokens = Tokenize(test_string, '\t');
        if (!strcmp(tokens[0], "222")) break;
        test_string = strtok(NULL, "\r") + 1;
    }
    if (!test_string) {
        logger->Warning("Error Pre-Parsing Potion Table");
        return;
    }

    for (uRow = 0; uRow < 50; ++uRow) {
        if (tokens.size() < 50) {
            logger->Warning("Error Parsing Potion Table at Row: %d Column: %d",
                            uRow, tokens.size());
            return;
        }
        for (uColumn = 0; uColumn < 50; ++uColumn) {
            char* currValue = tokens[uColumn + 7];
            potion_value = atoi(currValue);
            if (!potion_value && tolower(currValue[0]) == 'e') {
                potion_value = atoi(currValue + 1);
            }
            this->potion_data[uRow][uColumn] = potion_value;
        }

        test_string = strtok(NULL, "\r") + 1;
        if (!test_string) {
            logger->Warning("Error Parsing Potion Table at Row: %d Column: %d",
                            uRow, 0);
            return;
        }
        tokens = Tokenize(test_string, '\t');
    }
}

//----- (00453CE5) --------------------------------------------------------
void ItemsTable::LoadPotionNotes() {
    //  char Text[90];
    char* test_string;
    unsigned int uRow;
    unsigned int uColumn;
    unsigned __int8 potion_note;

    free(pPotionNotesTXT_Raw);
    std::vector<char *> tokens;
    char* pPotionNotesTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("potnotes.txt");
    test_string = strtok(pPotionNotesTXT_Raw, "\r") + 1;
    while (test_string) {
        tokens = Tokenize(test_string, '\t');
        if (!strcmp(tokens[0], "222")) break;
        test_string = strtok(NULL, "\r") + 1;
    }
    if (!test_string) {
        logger->Warning("Error Pre-Parsing Potion Table");
        return;
    }

    for (uRow = 0; uRow < 50; ++uRow) {
        if (tokens.size() < 50) {
            logger->Warning("Error Parsing Potion Table at Row: %d Column: %d",
                            uRow, tokens.size());
            return;
        }
        for (uColumn = 0; uColumn < 50; ++uColumn) {
            char* currValue = tokens[uColumn + 7];
            potion_note = atoi(currValue);
            if (!potion_note && tolower(currValue[0]) == 'e') {
                potion_note = atoi(currValue + 1);
            }
            this->potion_note[uRow][uColumn] = potion_note;
        }

        test_string = strtok(NULL, "\r") + 1;
        if (!test_string) {
            logger->Warning("Error Parsing Potion Table at Row: %d Column: %d",
                            uRow, 0);
            return;
        }
        tokens = Tokenize(test_string, '\t');
    }
}

//----- (00456442) --------------------------------------------------------
unsigned int ItemGen::GetValue() {
    unsigned int uBaseValue;  // edi@1
    unsigned int mod, bonus;

    uBaseValue = pItemsTable->pItems[this->uItemID].uValue;
    if (this->uAttributes & ITEM_TEMP_BONUS ||
        pItemsTable->IsMaterialNonCommon(this))
        return uBaseValue;
    if (uEnchantmentType) return uBaseValue + 100 * m_enchantmentStrength;

    if (special_enchantment) {
        mod = (pItemsTable->pSpecialEnchantments[special_enchantment-1].iTreasureLevel & 4);
        bonus = pItemsTable->pSpecialEnchantments[special_enchantment-1].iValue;
        if (!mod)
            return uBaseValue + bonus;
        else
            return uBaseValue * bonus;
    }
    return uBaseValue;
}

//----- (00456499) --------------------------------------------------------
std::string ItemGen::GetDisplayName() {
    if (IsIdentified()) {
        return GetIdentifiedName();
    } else {
        return pItemsTable->pItems[uItemID].pUnidentifiedName;
    }
}

//----- (004564B3) --------------------------------------------------------
std::string ItemGen::GetIdentifiedName() {
    unsigned __int8 equip_type;

    equip_type = GetItemEquipType();
    if ((equip_type == EQUIP_REAGENT) || (equip_type == EQUIP_POTION) ||
        (equip_type == EQUIP_GOLD)) {
        return pItemsTable->pItems[uItemID].pName;
    }

    if (uItemID == ITEM_LICH_JAR_FULL) {  // Lich Jar
        if (uHolderPlayer > 0 && uHolderPlayer <= 4) {
            auto player_name = pPlayers[uHolderPlayer]->pName;
            if (player_name[strlen(player_name) - 1] == 's')
                return localization->FormatString(
                    LSTR_FMT_JAR,
                    pPlayers[uHolderPlayer]->pName);
            else
                return localization->FormatString(
                    LSTR_FMT_JAR_2,
                    pPlayers[uHolderPlayer]->pName);
        }
    }

    if (!pItemsTable->IsMaterialNonCommon(this)) {
        if (uEnchantmentType) {
            return std::string(pItemsTable->pItems[uItemID].pName) + " " +
                   pItemsTable->pEnchantments[uEnchantmentType - 1].pOfName;
        } else if (!special_enchantment) {
            return pItemsTable->pItems[uItemID].pName;
        } else {
            if (special_enchantment == ITEM_ENCHANTMENT_VAMPIRIC
                || special_enchantment == ITEM_ENCHANTMENT_DEMON_SLAYING
                || special_enchantment == ITEM_ENCHANTMENT_DRAGON_SLAYING
                || special_enchantment == ITEM_ENCHANTMENT_ROGUES
                || special_enchantment == ITEM_ENCHANTMENT_WARRIORS
                || special_enchantment == ITEM_ENCHANTMENT_WIZARDS
                || special_enchantment == ITEM_ENCHANTMENT_ANTIQUE
                || special_enchantment == ITEM_ENCHANTMENT_MONKS
                || special_enchantment == ITEM_ENCHANTMENT_THIEVES
                || special_enchantment == ITEM_ENCHANTMENT_SWIFT
                || special_enchantment == ITEM_ENCHANTMENT_ELF_SLAYING
                || special_enchantment == ITEM_ENCHANTMENT_UNDEAD_SLAYING
                || special_enchantment == ITEM_ENCHANTMENT_ASSASINS
                || special_enchantment == ITEM_ENCHANTMENT_BARBARIANS
            ) {            // enchantment and name positions inverted!
                return StringPrintf(
                    "%s %s",
                    pItemsTable->pSpecialEnchantments[special_enchantment - 1].pNameAdd,
                    pItemsTable->pItems[uItemID].pName
                );
            } else {
                return std::string(pItemsTable->pItems[uItemID].pName) + " " +
                       pItemsTable->pSpecialEnchantments[special_enchantment - 1].pNameAdd;
            }
        }
    }

    return pItemsTable->pItems[uItemID].pName;
}

//----- (00456620) --------------------------------------------------------
void ItemsTable::GenerateItem(int treasure_level, unsigned int uTreasureType,
                              ItemGen* out_item) {
    int treasureLevelMinus1;      // ebx@3
    int current_chance;           // ebx@43
    int tmp_chance;               // ecx@47
    int artifact_random_id;       // ebx@57
    int v18;                      // edx@62
    unsigned int special_chance;  // edx@86
    unsigned int v26;             // edx@89
    unsigned int v27;             // eax@89
    int v32;                      // ecx@91
    int v33;                      // eax@91
    //    unsigned int v34; // eax@97
    int v45;            // eax@120
    int v46;            // edx@120
    int j;              // eax@121
    int val_list[800];  // [sp+Ch] [bp-C88h]@33
    int total_chance;   // [sp+C8Ch] [bp-8h]@33
    int artifact_found = 0;       // [sp+CA0h] [bp+Ch]@55
    int v57;            // [sp+CA0h] [bp+Ch]@62

    if (!out_item) out_item = (ItemGen*)malloc(sizeof(ItemGen));
    memset(out_item, 0, sizeof(*out_item));

    treasureLevelMinus1 = treasure_level - 1;
    if (uTreasureType) {  // generate known treasure type
        ITEM_EQUIP_TYPE requested_equip;
        PLAYER_SKILL_TYPE requested_skill = PLAYER_SKILL_INVALID;
        switch (uTreasureType) {
            case 20:
                requested_equip = EQUIP_SINGLE_HANDED;
                break;
            case 21:
                requested_equip = EQUIP_ARMOUR;
                break;
            case 22:
                requested_skill = PLAYER_SKILL_MISC;
                break;
            case 23:
                requested_skill = PLAYER_SKILL_SWORD;
                break;
            case 24:
                requested_skill = PLAYER_SKILL_DAGGER;
                break;
            case 25:
                requested_skill = PLAYER_SKILL_AXE;
                break;
            case 26:
                requested_skill = PLAYER_SKILL_SPEAR;
                break;
            case 27:
                requested_skill = PLAYER_SKILL_BOW;
                break;
            case 28:
                requested_skill = PLAYER_SKILL_MACE;
                break;
            case 29:
                requested_skill = PLAYER_SKILL_CLUB;
                break;
            case 30:
                requested_skill = PLAYER_SKILL_STAFF;
                break;
            case 31:
                requested_skill = PLAYER_SKILL_LEATHER;
                break;
            case 32:
                requested_skill = PLAYER_SKILL_CHAIN;
                break;
            case 33:
                requested_skill = PLAYER_SKILL_PLATE;
                break;
            case 34:
                requested_equip = EQUIP_SHIELD;
                break;
            case 35:
                requested_equip = EQUIP_HELMET;
                break;
            case 36:
                requested_equip = EQUIP_BELT;
                break;
            case 37:
                requested_equip = EQUIP_CLOAK;
                break;
            case 38:
                requested_equip = EQUIP_GAUNTLETS;
                break;
            case 39:
                requested_equip = EQUIP_BOOTS;
                break;
            case 40:
                requested_equip = EQUIP_RING;
                break;
            case 41:
                requested_equip = EQUIP_AMULET;
                break;
            case 42:
                requested_equip = EQUIP_WAND;
                break;
            case 43:
                requested_equip = EQUIP_SPELL_SCROLL;
                break;
            case 44:
                requested_equip = EQUIP_POTION;
                break;
            case 45:
                requested_equip = EQUIP_REAGENT;
                break;
            case 46:
                requested_equip = EQUIP_GEM;
                break;
            default:
                __debugbreak();  // check this condition
                requested_equip = (ITEM_EQUIP_TYPE)(uTreasureType - 1);
                break;
        }
        memset(val_list, 0, sizeof(val_list));
        total_chance = 0;
        j = 0;
        // a2a = 1;
        if (requested_skill ==
            PLAYER_SKILL_INVALID) {  // no skill for this item needed
            for (uint i = 1; i < 500; ++i) {
                if (pItems[i].uEquipType == requested_equip) {
                    val_list[j] = i;
                    ++j;
                    total_chance +=
                        pItems[i].uChanceByTreasureLvl[treasure_level - 1];
                }
            }
        } else {  // have needed skill
            for (uint i = 1; i < 500; ++i) {
                if (pItems[i].uSkillType == requested_skill) {
                    val_list[j] = i;
                    ++j;
                    total_chance +=
                        pItems[i].uChanceByTreasureLvl[treasure_level - 1];
                }
            }
        }

        current_chance = 0;
        if (total_chance) {
            current_chance = rand() % total_chance + 1;
            tmp_chance = 0;
            j = 0;
            while (tmp_chance < current_chance) {
                out_item->uItemID = val_list[j];
                tmp_chance += pItems[val_list[j]].uChanceByTreasureLvl[treasure_level - 1];
                ++j;
            }
        } else {
            out_item->uItemID = ITEM_LONGSWORD_1;
        }
    } else {
        // artifact
        if (treasureLevelMinus1 == 5) {
            for (int i = 0; i < 29; ++i)
                artifact_found += pParty->pIsArtifactFound[i];
            artifact_random_id = rand() % 29;
            if ((rand() % 100 < 5) && !pParty->pIsArtifactFound[artifact_random_id] &&
                (engine->config->gameplay.ArtifactLimit.Get() == 0 || artifact_found < engine->config->gameplay.ArtifactLimit.Get())) {
                pParty->pIsArtifactFound[artifact_random_id] = 1;
                out_item->uAttributes = 0;
                out_item->uItemID = ITEM_ARTIFACT_PUCK + artifact_random_id;
                SetSpecialBonus(out_item);
                return;
            }
        }

        v57 = 0;
        v18 = rand() % this->uChanceByTreasureLvlSumm[treasure_level - 1] + 1;
        while (v57 < v18) {
            ++out_item->uItemID;
            v57 += pItems[out_item->uItemID]
                       .uChanceByTreasureLvl[treasureLevelMinus1];
        }
    }
    if (out_item->GetItemEquipType() == EQUIP_POTION &&
        out_item->uItemID !=
            ITEM_POTION_BOTTLE) {  // if it potion set potion spec
        out_item->uEnchantmentType = 0;
        for (int i = 0; i < 2; ++i)
            out_item->uEnchantmentType += rand() % 4 + 1;
        out_item->uEnchantmentType =
            out_item->uEnchantmentType * treasure_level;
    }

    if (out_item->uItemID == ITEM_SPELLBOOK_LIGHT_DIVINE_INTERVENTION &&
        !(unsigned __int16)_449B57_test_bit(pParty->_quest_bits, 239))
        out_item->uItemID = ITEM_SPELLBOOK_LIGHT_SUN_BURST;
    if (pItemsTable->pItems[out_item->uItemID].uItemID_Rep_St)
        out_item->uAttributes = 0;
    else
        out_item->uAttributes = 1;

    if (out_item->GetItemEquipType() != EQUIP_POTION) {
        out_item->special_enchantment = ITEM_ENCHANTMENT_NULL;
        out_item->uEnchantmentType = 0;
    }
    // try get special enhansment
    switch (out_item->GetItemEquipType()) {
        case EQUIP_SINGLE_HANDED:
        case EQUIP_TWO_HANDED:
        case EQUIP_BOW:
            if (!uBonusChanceWpSpecial[treasureLevelMinus1]) return;
            if ((uint)(rand() % 100) >=
                uBonusChanceWpSpecial[treasureLevelMinus1])
                return;
            break;
        case EQUIP_ARMOUR:
        case EQUIP_SHIELD:
        case EQUIP_HELMET:
        case EQUIP_BELT:
        case EQUIP_CLOAK:
        case EQUIP_GAUNTLETS:
        case EQUIP_BOOTS:
        case EQUIP_RING:

            if (!uBonusChanceStandart[treasureLevelMinus1]) return;
            special_chance = rand() % 100;
            if (special_chance < uBonusChanceStandart[treasureLevelMinus1]) {
                v26 = rand() %
                          pEnchantmentsSumm[out_item->GetItemEquipType() - 3] +
                      1;
                v27 = 0;
                while (v27 < v26) {
                    ++out_item->uEnchantmentType;
                    v27 += pEnchantments[out_item->uEnchantmentType]
                               .to_item[out_item->GetItemEquipType() - 3];
                }

                v33 = rand() % (bonus_ranges[treasureLevelMinus1].maxR -
                                bonus_ranges[treasureLevelMinus1].minR + 1);
                out_item->m_enchantmentStrength =
                    v33 + bonus_ranges[treasureLevelMinus1].minR;
                v32 = out_item->uEnchantmentType - 1;
                if (v32 == 21 || v32 == 22 ||
                    v32 == 23)  // Armsmaster skill, Dodge skill, Unarmed skill
                    out_item->m_enchantmentStrength =
                        out_item->m_enchantmentStrength / 2;
                if (out_item->m_enchantmentStrength <= 0)
                    out_item->m_enchantmentStrength = 1;
                return;

            } else if (special_chance >=
                uBonusChanceStandart[treasureLevelMinus1] +
                uBonusChanceSpecial[treasureLevelMinus1]) {
                return;
            }
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
    for (unsigned int i = 0; i < pSpecialEnchantments_count; ++i) {
        int tr_lv = (pSpecialEnchantments[i].iTreasureLevel) & 3;

        // tr_lv  0 = treasure level 3/4
        // tr_lv  1 = treasure level 3/4/5
        // tr_lv  2 = treasure level 4/5
        // tr_lv  3 = treasure level 5/6

        if ((treasure_level - 1 == 2) && (tr_lv == 1 || tr_lv == 0) ||
            (treasure_level - 1 == 3) &&
                (tr_lv == 2 || tr_lv == 1 || tr_lv == 0) ||
            (treasure_level - 1 == 4) &&
                (tr_lv == 3 || tr_lv == 2 || tr_lv == 1) ||
            (treasure_level - 1 == 5) && (tr_lv == 3)) {
            spc = pSpecialEnchantments[i]
                      .to_item_apply[out_item->GetItemEquipType()];
            spc_sum += spc;
            if (spc) {
                val_list[j++] = i;
            }
        }
    }

    v46 = rand() % spc_sum + 1;  //случайные значения от 1 до spc_sum
    j = 0;
    v45 = 0;
    while (v45 < v46) {
        ++j;

        // TODO(pskelton): investigate this
        // quick fix on limit
        if (j > 799) break;
        out_item->special_enchantment = (ITEM_ENCHANTMENT)val_list[j];
        v45 += pSpecialEnchantments[val_list[j]].to_item_apply[out_item->GetItemEquipType()];
    }
}

//----- (004505CC) --------------------------------------------------------
bool ItemGen::GenerateArtifact() {
    signed int uNumArtifactsNotFound;  // esi@1
    int artifacts_list[32];

    memset(artifacts_list, 0, sizeof(artifacts_list));
    uNumArtifactsNotFound = 0;

    for (int i = 500; i < 529; ++i)
        if (!pParty->pIsArtifactFound[i - 500])
            artifacts_list[uNumArtifactsNotFound++] = i;

    Reset();
    if (uNumArtifactsNotFound) {
        uItemID = artifacts_list[rand() % uNumArtifactsNotFound];
        pItemsTable->SetSpecialBonus(this);
        return true;
    } else {
        return false;
    }
}

static void AddToMap(
    std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>*>& maptoadd,
    int enchId, CHARACTER_ATTRIBUTE_TYPE attrId, int bonusValue /*= 0*/,
    unsigned __int16 Player::*skillPtr /*= NULL*/) {
    auto key = maptoadd.find(enchId);
    std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currMap;
    if (key == maptoadd.end()) {
        currMap = new std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>;
        maptoadd[enchId] = currMap;
    } else {
        currMap = key->second;
    }
    Assert(currMap->find(attrId) == currMap->end(), "Attribute %d already present for enchantment %d", attrId, enchId);
    (*currMap)[attrId] = new CEnchantment(bonusValue, skillPtr);
}

void ItemGen::PopulateSpecialBonusMap() {
    // of Protection, +10 to all Resistances (description in txt says all 4, need to verify!)
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_AIR, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_BODY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_EARTH, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_FIRE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_MIND, 10);
    //AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_SPIRIT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, CHARACTER_ATTRIBUTE_RESIST_WATER, 10);

    // of The Gods, +10 to all Seven Statistics
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_ACCURACY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_ENDURANCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_LUCK, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_SPEED, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_STRENGTH, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_WILLPOWER, 10);

    // of Air Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_AIR_MAGIC, CHARACTER_ATTRIBUTE_SKILL_AIR, 0, &Player::skillAir);

    // of Body Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_BODY_MAGIC, CHARACTER_ATTRIBUTE_SKILL_BODY, 0, &Player::skillBody);

    // of Dark Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DARK_MAGIC, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, &Player::skillDark);

    // of Earth Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH_MAGIC, CHARACTER_ATTRIBUTE_SKILL_EARTH, 0, &Player::skillEarth);

    // of Fire Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_FIRE_MAGIC, CHARACTER_ATTRIBUTE_SKILL_FIRE, 0, &Player::skillFire);

    // of Light Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_LIGHT_MAGIC, CHARACTER_ATTRIBUTE_SKILL_LIGHT, 0, &Player::skillLight);

    // of Mind Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MIND_MAGIC, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, &Player::skillMind);

    // of Spirit Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SPIRIT_MAGIC, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, &Player::skillSpirit);

    // of Water Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_WATER_MAGIC, CHARACTER_ATTRIBUTE_SKILL_WATER, 0, &Player::skillWater);

    // of Doom, +1 to Seven Stats, HP, SP, Armor, Resistances (in txt it says 4, need to check!)
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_ACCURACY, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_INTELLIGENCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_ENDURANCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_LUCK, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_SPEED, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_STRENGTH, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_WILLPOWER, 1);

    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_AIR, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_BODY, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_EARTH, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_FIRE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_MIND, 1);
    //AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_SPIRIT, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_RESIST_WATER, 1);

    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_AC_BONUS, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_HEALTH, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_MANA, 1);

    // of Earth
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, CHARACTER_ATTRIBUTE_AC_BONUS, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, CHARACTER_ATTRIBUTE_ENDURANCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, CHARACTER_ATTRIBUTE_HEALTH, 10);

    // of Life
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_LIFE, CHARACTER_ATTRIBUTE_HEALTH, 10);

    // Rogues
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ROGUES, CHARACTER_ATTRIBUTE_ACCURACY, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ROGUES, CHARACTER_ATTRIBUTE_SPEED, 5);

    // of The Dragon
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DRAGON, CHARACTER_ATTRIBUTE_STRENGTH, 25);

    // of The Eclipse
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_ECLIPSE, CHARACTER_ATTRIBUTE_MANA, 10);

    // of The Golem
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GOLEM, CHARACTER_ATTRIBUTE_AC_BONUS, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GOLEM, CHARACTER_ATTRIBUTE_ENDURANCE, 15);

    // of The Moon
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MOON, CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MOON, CHARACTER_ATTRIBUTE_LUCK, 10);

    // of The Phoenix
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PHOENIX, CHARACTER_ATTRIBUTE_RESIST_FIRE, 30);

    // of The Sky
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, CHARACTER_ATTRIBUTE_MANA, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, CHARACTER_ATTRIBUTE_SPEED, 10);

    // of The Stars
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STARS, CHARACTER_ATTRIBUTE_ACCURACY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STARS, CHARACTER_ATTRIBUTE_ENDURANCE, 10);

    // of The Sun
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, CHARACTER_ATTRIBUTE_STRENGTH, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, CHARACTER_ATTRIBUTE_WILLPOWER, 10);

    // of The Troll
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_TROLL, CHARACTER_ATTRIBUTE_ENDURANCE, 15);

    // of The Unicorn
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_UNICORN, CHARACTER_ATTRIBUTE_LUCK, 15);

    // Warriors
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, CHARACTER_ATTRIBUTE_ENDURANCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, CHARACTER_ATTRIBUTE_STRENGTH, 5);

    // Wizards
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, CHARACTER_ATTRIBUTE_INTELLIGENCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, CHARACTER_ATTRIBUTE_WILLPOWER, 5);

    // Monks
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, CHARACTER_ATTRIBUTE_SKILL_DODGE, 3, &Player::skillDodge);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, CHARACTER_ATTRIBUTE_SKILL_UNARMED, 3, &Player::skillUnarmed);

    // Thieves
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, CHARACTER_ATTRIBUTE_SKILL_STEALING, 3, &Player::skillStealing);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 3, &Player::skillDisarmTrap);

    // of Identifying
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, CHARACTER_ATTRIBUTE_SKILL_ITEM_ID, 3, &Player::skillItemId);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID, 3, &Player::skillMonsterId);

    // Assassins
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ASSASINS, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 2, &Player::skillDisarmTrap);

    // Barbarians
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_BARBARIANS, CHARACTER_ATTRIBUTE_AC_BONUS, 5);

    // of the Storm
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STORM, CHARACTER_ATTRIBUTE_RESIST_AIR, 20);

    // of the Ocean
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, CHARACTER_ATTRIBUTE_SKILL_ALCHEMY, 2, &Player::skillAlchemy);
}

// TODO: where is it used?
void ItemGen::PopulateRegularBonusMap() {
    // of Might
    AddToMap(regularBonusMap, 1, CHARACTER_ATTRIBUTE_STRENGTH);

    // of Thought
    AddToMap(regularBonusMap, 2, CHARACTER_ATTRIBUTE_INTELLIGENCE);

    // of Charm
    AddToMap(regularBonusMap, 3, CHARACTER_ATTRIBUTE_WILLPOWER);

    // of Vigor
    AddToMap(regularBonusMap, 4, CHARACTER_ATTRIBUTE_ENDURANCE);

    // of Precision
    AddToMap(regularBonusMap, 5, CHARACTER_ATTRIBUTE_ACCURACY);

    // of Speed
    AddToMap(regularBonusMap, 6, CHARACTER_ATTRIBUTE_SPEED);

    // of Luck
    AddToMap(regularBonusMap, 7, CHARACTER_ATTRIBUTE_LUCK);

    // of Health
    AddToMap(regularBonusMap, 8, CHARACTER_ATTRIBUTE_HEALTH);

    // of Magic
    AddToMap(regularBonusMap, 9, CHARACTER_ATTRIBUTE_MANA);

    // of Defense
    AddToMap(regularBonusMap, 10, CHARACTER_ATTRIBUTE_AC_BONUS);

    // of Fire Resistance
    AddToMap(regularBonusMap, 11, CHARACTER_ATTRIBUTE_RESIST_FIRE);

    // of Air Resistance
    AddToMap(regularBonusMap, 12, CHARACTER_ATTRIBUTE_RESIST_AIR);

    // of Water Resistance
    AddToMap(regularBonusMap, 13, CHARACTER_ATTRIBUTE_RESIST_WATER);

    // of Earth Resistance
    AddToMap(regularBonusMap, 14, CHARACTER_ATTRIBUTE_RESIST_EARTH);

    // of Mind Resistance
    AddToMap(regularBonusMap, 15, CHARACTER_ATTRIBUTE_RESIST_MIND);

    // of Body Resistance
    AddToMap(regularBonusMap, 16, CHARACTER_ATTRIBUTE_RESIST_BODY);

    // of Alchemy
    AddToMap(regularBonusMap, 17, CHARACTER_ATTRIBUTE_SKILL_ALCHEMY);

    // of Stealing
    AddToMap(regularBonusMap, 18, CHARACTER_ATTRIBUTE_SKILL_STEALING);

    // of Disarming
    AddToMap(regularBonusMap, 19, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM);

    // of Items
    AddToMap(regularBonusMap, 20, CHARACTER_ATTRIBUTE_SKILL_ITEM_ID);

    // of Monsters
    AddToMap(regularBonusMap, 21, CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID);

    // of Arms
    AddToMap(regularBonusMap, 22, CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER);

    // of Dodging
    AddToMap(regularBonusMap, 23, CHARACTER_ATTRIBUTE_SKILL_DODGE);

    // of the Fist
    AddToMap(regularBonusMap, 24, CHARACTER_ATTRIBUTE_SKILL_UNARMED);
}

void ItemGen::PopulateArtifactBonusMap() {
    // Puck
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_PUCK, CHARACTER_ATTRIBUTE_SPEED, 40);

    // Iron Feather
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_IRON_FEATHER, CHARACTER_ATTRIBUTE_STRENGTH, 40);

    // Wallace
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, CHARACTER_ATTRIBUTE_WILLPOWER, 40);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_LUCK, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_STRENGTH, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_WILLPOWER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_ENDURANCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_ACCURACY, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_SPEED, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_LUCK, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_YORUBA, CHARACTER_ATTRIBUTE_ENDURANCE, 25);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SPLITTER, CHARACTER_ATTRIBUTE_RESIST_FIRE, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ULLYSES, CHARACTER_ATTRIBUTE_ACCURACY, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, CHARACTER_ATTRIBUTE_SKILL_DODGE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, CHARACTER_ATTRIBUTE_SKILL_UNARMED, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LEAGUE_BOOTS, CHARACTER_ATTRIBUTE_SPEED, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LEAGUE_BOOTS, CHARACTER_ATTRIBUTE_SKILL_WATER, 0, &Player::skillWater);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, &Player::skillMind);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, &Player::skillDark);

    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_STRENGTH, 150);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_INTELLIGENCE, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_WILLPOWER, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, &Player::skillDark);
    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, CHARACTER_ATTRIBUTE_SKILL_MEDITATION, 15);

    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_LUCK, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_FIRE, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_WATER, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_AIR, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_EARTH, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_OLD_NICK, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);

    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_STRENGTH, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_ENDURANCE, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_AC_BONUS, -15);

    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, &Player::skillSpirit);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_SKILL_SHIELD, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, CHARACTER_ATTRIBUTE_ENDURANCE, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, CHARACTER_ATTRIBUTE_RESIST_EARTH, -30);

    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_SKILL_LIGHT, 0, &Player::skillLight);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_WILLPOWER, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_STRENGTH, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_LUCK, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, CHARACTER_ATTRIBUTE_SKILL_LEARNING, +15);
    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, CHARACTER_ATTRIBUTE_ENDURANCE, -50);

    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_SKILL_FIRE, 0, &Player::skillFire);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_RESIST_WATER, +50);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_WILLPOWER, 30);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_AC_BONUS, -20);

    AddToMap(artifactBonusMap, ITEM_RELIC_TITANS_BELT, CHARACTER_ATTRIBUTE_STRENGTH, 75);
    AddToMap(artifactBonusMap, ITEM_RELIC_TITANS_BELT, CHARACTER_ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_SPEED, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_LUCK, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_FIRE, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_WATER, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_AIR, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_EARTH, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_MIND, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, CHARACTER_ATTRIBUTE_RESIST_BODY, -15);

    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, CHARACTER_ATTRIBUTE_ACCURACY, 150);
    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, CHARACTER_ATTRIBUTE_SKILL_BOW, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, CHARACTER_ATTRIBUTE_AC_BONUS, -25);

    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, &Player::skillMind);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SKILL_BODY, 0, &Player::skillBody);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, &Player::skillSpirit);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_STRENGTH, 75);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_RESIST_AIR, -50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_SPEED, 100);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_ACCURACY, 50);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_RESIST_AIR, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, CHARACTER_ATTRIBUTE_WILLPOWER, -20);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, CHARACTER_ATTRIBUTE_INTELLIGENCE, -20);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, CHARACTER_ATTRIBUTE_WILLPOWER, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, CHARACTER_ATTRIBUTE_INTELLIGENCE, 15);

    AddToMap(artifactBonusMap, ITEM_ELVEN_CHAINMAIL, CHARACTER_ATTRIBUTE_SPEED, 15);
    AddToMap(artifactBonusMap, ITEM_ELVEN_CHAINMAIL, CHARACTER_ATTRIBUTE_ACCURACY, 15);

    AddToMap(artifactBonusMap, ITEM_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_STRENGTH, 15);
    AddToMap(artifactBonusMap, ITEM_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_ENDURANCE, 15);
    AddToMap(artifactBonusMap, ITEM_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_RESIST_FIRE, 30);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, CHARACTER_ATTRIBUTE_STRENGTH, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_FIRE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_AIR, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_EARTH, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_MIND, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_BODY, 10);
}

void ItemGen::GetItemBonusSpecialEnchantment(Player* owner,
                                             CHARACTER_ATTRIBUTE_TYPE attrToGet,
                                             int* additiveBonus,
                                             int* halfSkillBonus) {
    auto bonusList = specialBonusMap.find(this->special_enchantment);
    if (bonusList == specialBonusMap.end()) {
        return;
    }
    std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
    if (currList->find(attrToGet) != currList->end()) {
        CEnchantment* currBonus = (*currList)[attrToGet];
        if (currBonus->statPtr != NULL) {
            if (currBonus->statBonus == 0) {
                *halfSkillBonus = owner->*currBonus->statPtr / 2;
            } else {
                if (*additiveBonus < currBonus->statBonus) {
                    *additiveBonus = currBonus->statBonus;
                }
            }
        } else {
            *additiveBonus += currBonus->statBonus;
        }
    }
}

void ItemGen::GetItemBonusArtifact(Player* owner,
                                   CHARACTER_ATTRIBUTE_TYPE attrToGet,
                                   int* bonusSum) {
    auto bonusList = artifactBonusMap.find(this->uItemID);
    if (bonusList == artifactBonusMap.end()) {
        return;
    }
    std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
    if (currList->find(attrToGet) != currList->end()) {
        CEnchantment* currBonus = (*currList)[attrToGet];
        if (currBonus->statPtr != NULL) {
            *bonusSum = owner->*currBonus->statPtr / 2;
        } else {
            *bonusSum += currBonus->statBonus;
        }
    }
}

bool ItemGen::IsRegularEnchanmentForAttribute(
    CHARACTER_ATTRIBUTE_TYPE attrToGet) {
    auto bonusList = specialBonusMap.find(this->uEnchantmentType);
    if (bonusList == specialBonusMap.end()) {
        return false;
    }
    std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment*>* currList = bonusList->second;
    return currList->find(attrToGet) != currList->end();
}

ITEM_EQUIP_TYPE ItemGen::GetItemEquipType() {
    // to avoid nzi - is this safe??
    if (this->uItemID == 0)
        return EQUIP_NONE;
    else
        return pItemsTable->pItems[this->uItemID].uEquipType;
}

unsigned char ItemGen::GetPlayerSkillType() {
    unsigned char skl = pItemsTable->pItems[this->uItemID].uSkillType;
    if (skl == PLAYER_SKILL_CLUB) {
        // club skill not used but some items load it
        skl = PLAYER_SKILL_MACE;
    }
    return skl;
}

char* ItemGen::GetIconName() {
    return pItemsTable->pItems[this->uItemID].pIconName;
}

unsigned __int8 ItemGen::GetDamageDice() {
    return pItemsTable->pItems[this->uItemID].uDamageDice;
}

unsigned __int8 ItemGen::GetDamageRoll() {
    return pItemsTable->pItems[this->uItemID].uDamageRoll;
}

unsigned __int8 ItemGen::GetDamageMod() {
    return pItemsTable->pItems[this->uItemID].uDamageMod;
}

//----- (0043C91D) --------------------------------------------------------
int GetItemTextureFilename(char* pOut, signed int item_id, int index,
                           int shoulder) {
    int result;  // eax@2
    ITEM_EQUIP_TYPE pEquipType;

    result = 0;  // BUG   fn is void
    pEquipType = pItemsTable->pItems[item_id].uEquipType;
    if (item_id > 500) {
        switch (item_id) {
            case ITEM_RELIC_HARECS_LEATHER:
                if (byte_5111F6_OwnedArtifacts[2] != 0) item_id = 234;
                break;
            case ITEM_ARTIFACT_YORUBA:
                if (byte_5111F6_OwnedArtifacts[1] != 0) item_id = 236;
                break;
            case ITEM_ARTIFACT_GOVERNORS_ARMOR:
                if (byte_5111F6_OwnedArtifacts[0] != 0) item_id = 235;
                break;
            case ITEM_ELVEN_CHAINMAIL:
                if (byte_5111F6_OwnedArtifacts[16] != 0) item_id = 73;
                break;
            case ITEM_ARTIFACT_LEAGUE_BOOTS:
                if (byte_5111F6_OwnedArtifacts[3] != 0) item_id = 312;
                break;
            case ITEM_RELIC_TALEDONS_HELM:
                if (byte_5111F6_OwnedArtifacts[4] != 0) item_id = 239;
                break;
            case ITEM_RELIC_SCHOLARS_CAP:
                if (byte_5111F6_OwnedArtifacts[5] != 0) item_id = 240;
                break;
            case ITEM_RELIC_PHYNAXIAN_CROWN:
                if (byte_5111F6_OwnedArtifacts[6] != 0) item_id = 241;
                break;
            case ITEM_ARTIFACT_MINDS_EYE:
                if (byte_5111F6_OwnedArtifacts[7] != 0) item_id = 93;
                break;
            case ITEM_RARE_SHADOWS_MASK:
                if (byte_5111F6_OwnedArtifacts[8] != 0) item_id = 344;
                break;
            case ITEM_RELIC_TITANS_BELT:
                if (byte_5111F6_OwnedArtifacts[9] != 0) item_id = 324;
                break;
            case ITEM_ARTIFACT_HEROS_BELT:
                if (byte_5111F6_OwnedArtifacts[10] != 0) item_id = 104;
                break;
            case ITEM_RELIC_TWILIGHT:
                if (byte_5111F6_OwnedArtifacts[11] != 0) item_id = 325;
                break;
            case ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP:
                if (byte_5111F6_OwnedArtifacts[12] != 0) item_id = 330;
                break;
            case ITEM_RARE_SUN_CLOAK:
                if (byte_5111F6_OwnedArtifacts[13] != 0) item_id = 347;
                break;
            case ITEM_RARE_MOON_CLOAK:
                if (byte_5111F6_OwnedArtifacts[14] != 0) item_id = 348;
                break;
            case ITEM_RARE_VAMPIRES_CAPE:
                if (byte_5111F6_OwnedArtifacts[15] != 0) item_id = 350;
                break;
            default:
                return 0;
        }
    }

    switch (pEquipType) {
        case EQUIP_ARMOUR:
            if (!shoulder)
                return sprintf(pOut, "item%3.3dv%d", item_id, index);
            else if (shoulder == 1)
                return sprintf(pOut, "item%3.3dv%da1", item_id, index);
            else if (shoulder == 2)
                return sprintf(pOut, "item%3.3dv%da2", item_id, index);
            break;
        case EQUIP_CLOAK:
            if (!shoulder)
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
bool ItemGen::MerchandiseTest(int _2da_idx) {
    bool test;

    if ((p2DEvents[_2da_idx - 1].uType != BuildingType_AlchemistShop ||
         (signed int)this->uItemID < 740 || (signed int)this->uItemID > 771) &&
            ((signed int)this->uItemID >= 600 ||
             (signed int)this->uItemID >= 529 &&
                 (signed int)this->uItemID <= 599) ||
        this->IsStolen())
        return false;
    switch (p2DEvents[_2da_idx - 1].uType) {
        case BuildingType_WeaponShop: {
            test = this->GetItemEquipType() <= EQUIP_BOW;
            break;
        }
        case BuildingType_ArmorShop: {
            test = this->GetItemEquipType() >= EQUIP_ARMOUR &&
                   this->GetItemEquipType() <= EQUIP_BOOTS;
            break;
        }
        case BuildingType_MagicShop: {
            test = this->GetPlayerSkillType() == PLAYER_SKILL_MISC ||
                   this->GetItemEquipType() == EQIUP_ANY;
            break;
        }
        case BuildingType_AlchemistShop: {
            test = this->GetItemEquipType() == EQUIP_REAGENT ||
                   this->GetItemEquipType() == EQUIP_POTION ||
                   (this->GetItemEquipType() > EQUIP_POTION &&
                    !(this->GetItemEquipType() != EQUIP_MESSAGE_SCROLL ||
                      (signed int)this->uItemID < 740) &&
                    this->uItemID != 771);
            break;
        }
        default: {
            test = false;
            break;
        }
    }
    return test;
}
