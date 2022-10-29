#include "ItemTable.h"

#include <map>
#include <vector>
#include <string>
#include <unordered_map>

#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/LOD.h"
#include "Engine/MapInfo.h"
#include "Engine/mm7_data.h"

#include "GUI/UI/UIHouses.h"

#include "Utility/String.h"
#include "Utility/Random.h"

//----- (0045814E) --------------------------------------------------------
void ItemTable::Release() {
    pMonstersTXT_Raw.clear();
    pMonsterPlacementTXT_Raw.clear();
    pSpcItemsTXT_Raw.clear();
    pStdItemsTXT_Raw.clear();
    pRndItemsTXT_Raw.clear();
    pItemsTXT_Raw.clear();
    pHostileTXT_Raw.clear();
    pHistoryTXT_Raw.clear();
    free(pPotionsTXT_Raw);
    free(pPotionNotesTXT_Raw);
    pPotionsTXT_Raw = nullptr;
    pPotionNotesTXT_Raw = nullptr;
}

//----- (00456D84) --------------------------------------------------------
void ItemTable::Initialize() {
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

    pStdItemsTXT_Raw = pEvents_LOD->LoadCompressedTexture("stditems.txt").string_view();
    strtok(pStdItemsTXT_Raw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    // Standard Bonuses by Group
    for (int i = 0; i < 24; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        pEnchantments[i].pBonusStat = RemoveQuotes(tokens[0]);
        pEnchantments[i].pOfName = RemoveQuotes(tokens[1]);

        int k = 2;
        for (ITEM_EQUIP_TYPE j : pEnchantments[i].to_item.indices())
            pEnchantments[i].to_item[j] = atoi(tokens[k++]);
    }

    pEnchantmentsSumm.fill(0);
    for (int j = 0; j < 24; ++j)
        for (ITEM_EQUIP_TYPE i : pEnchantments[j].to_item.indices())
            pEnchantmentsSumm[i] += pEnchantments[j].to_item[i];

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

    pSpcItemsTXT_Raw = pEvents_LOD->LoadCompressedTexture("spcitems.txt").string_view();
    strtok(pSpcItemsTXT_Raw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    for (ITEM_ENCHANTMENT i : pSpecialEnchantments.indices()) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() >= 17, "Invalid number of tokens");
        pSpecialEnchantments[i].pBonusStatement = RemoveQuotes(tokens[0]);
        pSpecialEnchantments[i].pNameAdd = RemoveQuotes(tokens[1]);

        int k = 2;
        for (ITEM_EQUIP_TYPE j : pSpecialEnchantments[i].to_item_apply.indices())
            pSpecialEnchantments[i].to_item_apply[j] = atoi(tokens[k++]);

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

    // TODO(captainurist): can be dropped?
    pSpecialEnchantmentsSumm.fill(0);
    for (ITEM_ENCHANTMENT j : pSpecialEnchantments.indices())
        for (ITEM_EQUIP_TYPE i : pSpecialEnchantments[j].to_item_apply.indices())
            pSpecialEnchantmentsSumm[i] += pSpecialEnchantments[j].to_item_apply[i];

    InitializeBuildingResidents();

    pItemsTXT_Raw = pEvents_LOD->LoadCompressedTexture("items.txt").string_view();
    strtok(pItemsTXT_Raw.data(), "\r");
    strtok(NULL, "\r");
    uAllItemsCount = 0;
    while (true) {
        test_string = strtok(NULL, "\r");
        if (!test_string)
            break;
        ++test_string;

        auto tokens = Tokenize(test_string, '\t');

        ITEM_TYPE item_counter = ITEM_TYPE(atoi(tokens[0]) + std::to_underlying(ITEM_FIRST_VALID));
        //uAllItemsCount = item_counter;
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
                for (ITEM_ENCHANTMENT ii : pSpecialEnchantments.indices()) {
                    if (iequals(tokens[12], pSpecialEnchantments[ii].pNameAdd)) {
                        pItems[item_counter]._additional_value = ii;
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
    }

    pRndItemsTXT_Raw = pEvents_LOD->LoadCompressedTexture("rnditems.txt").string_view();
    strtok(pRndItemsTXT_Raw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    while(true) {
        test_string = strtok(NULL, "\r");
        if (!test_string)
            break;
        test_string++;

        auto tokens = Tokenize(test_string, '\t');
        Assert(tokens.size() > 7, "Invalid number of tokens");
        ITEM_TYPE item_counter = ITEM_TYPE(atoi(tokens[0]));
        pItems[item_counter].uChanceByTreasureLvl1 = atoi(tokens[2]);
        pItems[item_counter].uChanceByTreasureLvl2 = atoi(tokens[3]);
        pItems[item_counter].uChanceByTreasureLvl3 = atoi(tokens[4]);
        pItems[item_counter].uChanceByTreasureLvl4 = atoi(tokens[5]);
        pItems[item_counter].uChanceByTreasureLvl5 = atoi(tokens[6]);
        pItems[item_counter].uChanceByTreasureLvl6 = atoi(tokens[7]);
    }

    // ChanceByTreasureLvl Summ - to calculate chance
    memset(&uChanceByTreasureLvlSumm, 0, 24);
    for (int i = 0; i < 6; ++i)
        for (ITEM_TYPE j : pItems.indices())
            uChanceByTreasureLvlSumm[i] += pItems[j].uChanceByTreasureLvl[i];

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
    pRndItemsTXT_Raw.clear();

    ItemGen::PopulateSpecialBonusMap();
    ItemGen::PopulateArtifactBonusMap();
    ItemGen::PopulateRegularBonusMap();
}

//----- (00456D17) --------------------------------------------------------
void ItemTable::SetSpecialBonus(ItemGen* pItem) {
    if (pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL) {
        pItem->uEnchantmentType = pItems[pItem->uItemID]._bonus_type;
        pItem->special_enchantment =
                (ITEM_ENCHANTMENT)pItems[pItem->uItemID]._additional_value;
        pItem->m_enchantmentStrength = pItems[pItem->uItemID]._bonus_strength;
    }
}

//----- (00456D43) --------------------------------------------------------
bool ItemTable::IsMaterialSpecial(ItemGen* pItem) {
    return this->pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL;
}

//----- (00456D5E) --------------------------------------------------------
bool ItemTable::IsMaterialNonCommon(ItemGen* pItem) {
    return pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_RELIC ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_ARTIFACT;
}

//----- (00453B3C) --------------------------------------------------------
void ItemTable::LoadPotions() {
    //    char Text[90];
    char* test_string;
    unsigned int uRow;
    unsigned int uColumn;
    uint8_t potion_value;

    free(pPotionNotesTXT_Raw);
    std::vector<char *> tokens;
    std::string pPotionsTXT_Raw = std::string(pEvents_LOD->LoadCompressedTexture("potion.txt").string_view());
    test_string = strtok(pPotionsTXT_Raw.data(), "\r") + 1;
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
void ItemTable::LoadPotionNotes() {
    //  char Text[90];
    char* test_string;
    unsigned int uRow;
    unsigned int uColumn;
    uint8_t potion_note;

    free(pPotionNotesTXT_Raw);
    std::vector<char *> tokens;
    std::string pPotionNotesTXT_Raw = std::string(pEvents_LOD->LoadCompressedTexture("potnotes.txt").string_view());
    test_string = strtok(pPotionNotesTXT_Raw.data(), "\r") + 1;
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

//----- (00456620) --------------------------------------------------------
void ItemTable::GenerateItem(int treasure_level, unsigned int uTreasureType,
                             ItemGen* out_item) {
    int treasureLevelMinus1;      // ebx@3
    int current_chance;           // ebx@43
    int tmp_chance;               // ecx@47
    ITEM_TYPE artifact_random_id;       // ebx@57
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
    std::array<ITEM_TYPE, 800> val_list;  // [sp+Ch] [bp-C88h]@33
    std::array<ITEM_ENCHANTMENT, 800> val_list2;
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
                // TODO(captainurist): explore
                requested_equip = (ITEM_EQUIP_TYPE)(uTreasureType - 1);
                break;
        }
        val_list.fill(ITEM_NULL);
        total_chance = 0;
        j = 0;
        // a2a = 1;
        if (requested_skill ==
            PLAYER_SKILL_INVALID) {  // no skill for this item needed
            for (ITEM_TYPE i : RandomSpawnableItems()) {
                if (pItems[i].uEquipType == requested_equip) {
                    val_list[j] = i;
                    ++j;
                    total_chance +=
                            pItems[i].uChanceByTreasureLvl[treasure_level - 1];
                }
            }
        } else {  // have needed skill
            for (ITEM_TYPE i : RandomSpawnableItems()) {
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
            out_item->uItemID = ITEM_CRUDE_LONGSWORD;
        }
    } else {
        // artifact
        if (treasureLevelMinus1 == 5) {
            for (ITEM_TYPE i : Artifacts())
                artifact_found += pParty->pIsArtifactFound[i];
            artifact_random_id = Sample(Artifacts());
            if ((rand() % 100 < 5) && !pParty->pIsArtifactFound[artifact_random_id] &&
                (engine->config->gameplay.ArtifactLimit.Get() == 0 || artifact_found < engine->config->gameplay.ArtifactLimit.Get())) {
                pParty->pIsArtifactFound[artifact_random_id] = 1;
                out_item->uAttributes = 0;
                out_item->uItemID = artifact_random_id;
                SetSpecialBonus(out_item);
                return;
            }
        }

        v57 = 0;
        v18 = rand() % this->uChanceByTreasureLvlSumm[treasure_level - 1] + 1;
        while (v57 < v18) {
            // TODO(captainurist): what's going on here? Get rid of casts.
            out_item->uItemID = ITEM_TYPE(std::to_underlying(out_item->uItemID) + 1);
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

    if (out_item->uItemID == ITEM_SPELLBOOK_DIVINE_INTERVENTION &&
        !(uint16_t)_449B57_test_bit(pParty->_quest_bits, 239))
        out_item->uItemID = ITEM_SPELLBOOK_SUNRAY;
    if (pItemTable->pItems[out_item->uItemID].uItemID_Rep_St)
        out_item->uAttributes = 0;
    else
        out_item->uAttributes = ITEM_IDENTIFIED;

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
                      pEnchantmentsSumm[out_item->GetItemEquipType()] + 1;
                v27 = 0;
                while (v27 < v26) {
                    ++out_item->uEnchantmentType;
                    v27 += pEnchantments[out_item->uEnchantmentType]
                            .to_item[out_item->GetItemEquipType()];
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
    val_list2.fill(ITEM_ENCHANTMENT_NULL);
    for (ITEM_ENCHANTMENT i : pSpecialEnchantments.indices()) {
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
            spc = pSpecialEnchantments[i].to_item_apply[out_item->GetItemEquipType()];
            spc_sum += spc;
            if (spc) {
                val_list2[j++] = i;
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
        out_item->special_enchantment = val_list2[j];
        v45 += pSpecialEnchantments[val_list2[j]].to_item_apply[out_item->GetItemEquipType()];
    }
}

void ItemTable::PrintItemTypesEnum() {
    std::unordered_map<std::string, int> countByName;
    std::unordered_map<std::string, int> indexByName;
    std::vector<std::pair<std::string, std::string>> items;

    items.emplace_back("NULL", "");

    for(ITEM_TYPE i : pItems.indices()) {
        const ItemDesc &desc = pItems[i];
        std::string icon = desc.pIconName;
        std::string name = desc.pName;
        std::string description = desc.pDescription;

        if (icon.empty() || icon == "null") {
            items.emplace_back("", "Unused.");
            continue;
        }

        if (name == "Empty Message Scroll" || name == "Name of Message") {
            items.emplace_back("", "Empty scroll placeholder, unused.");
            continue;
        }

        if (name == "Newname Key") {
            items.emplace_back("", "Key placeholder, unused.");
            continue;
        }

        if (name.contains("Placeholder") || name.contains("Sealed Letter")) {
            items.emplace_back("", name + ", unused.");
            continue;
        }

        std::string enumName;
        for (char c : name) {
            if (isalnum(c)) {
                enumName += static_cast<char>(toupper(c));
            } else if (isspace(c) || c == '/' || c == '-') {
                if (!enumName.ends_with('_'))
                    enumName += '_';
            }
        }

        if (desc.uEquipType == EQUIP_REAGENT) {
            enumName = "REAGENT_" + enumName;
        } else if (desc.uEquipType == EQUIP_POTION) {
            if (name != "Potion Bottle")
                enumName = "POTION_" + enumName;
            if (name.ends_with("_POTION"))
                name = name.substr(0, name.size() - 7);
        } else if (desc.uEquipType == EQUIP_SPELL_SCROLL) {
            enumName = "SCROLL_" + enumName;
        } else if (desc.uEquipType == EQUIP_BOOK) {
            enumName = "SPELLBOOK_" + enumName;
        } else if (desc.uEquipType == EQUIP_MESSAGE_SCROLL) {
            enumName = "MESSAGE_" + enumName;
        } else if (desc.uEquipType == EQUIP_GOLD) {
            if (description == "A small pile of gold coins.") {
                enumName = "GOLD_SMALL";
            } else if (description == "A pile of gold coins.") {
                enumName = "GOLD_MEDIUM";
            } else if (description == "A large pile of gold coins.") {
                enumName = "GOLD_LARGE";
            } else {
                Assert(false);
            }
        } else if (desc.uEquipType == EQUIP_GEM) {
            enumName = "GEM_" + enumName;
        }

        if (desc.uMaterial == MATERIAL_ARTIFACT) {
            enumName = "ARTIFACT_" + enumName;
        } else if (desc.uMaterial == MATERIAL_RELIC) {
            enumName = "RELIC_" + enumName;
        } else if (desc.uMaterial == MATERIAL_SPECIAL) {
            enumName = "RARE_" + enumName;
        } else if (description.starts_with("Quest")) {
            enumName = "QUEST_" + enumName;
        }

        if (name == "Lich Jar") {
            if (description.contains("Empty")) {
                enumName += "_EMPTY";
            } else {
                enumName += "_FULL";
            }
        }

        if (name == "The Perfect Bow")
            if (!description.contains("off-balance"))
                enumName += "_FIXED";

        if (indexByName.contains(enumName)) {
            int count = ++countByName[enumName];
            if (count == 2)
                items[indexByName[enumName]].first = enumName + "_1";

            enumName = enumName + "_" + std::to_string(count);
        } else {
            indexByName[enumName] = items.size();
            countByName[enumName] = 1;
        }

        items.emplace_back(enumName, "");
    }

    printf("enum ITEM_TYPE {\n");
    for (size_t i = 0; i < items.size(); i++) {
        if (!items[i].first.empty()) {
            printf("    ITEM_%s = %d,\n", items[i].first.c_str(), static_cast<int>(i));
        } else {
            printf("    ITEM_%d = %d, // %s\n", static_cast<int>(i), static_cast<int>(i), items[i].second.c_str());
        }
    }
    printf("};\n");
}
