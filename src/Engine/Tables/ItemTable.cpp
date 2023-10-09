#include "ItemTable.h"

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "Engine/Spells/Spells.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/GameResourceManager.h"

#include "GUI/UI/UIHouses.h"

#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"

#include "Utility/String.h"
#include "Utility/MapAccess.h"

static void strtokSkipLines(int n) {
    for (int i = 0; i < n; ++i) {
        (void)strtok(NULL, "\r");
    }
}

//----- (00456D84) --------------------------------------------------------
void ItemTable::Initialize(GameResourceManager *resourceManager) {
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

    std::map<std::string, CharacterSkillType, ILess> equipSkillMap;
    equipSkillMap["staff"] = CHARACTER_SKILL_STAFF;
    equipSkillMap["sword"] = CHARACTER_SKILL_SWORD;
    equipSkillMap["dagger"] = CHARACTER_SKILL_DAGGER;
    equipSkillMap["axe"] = CHARACTER_SKILL_AXE;
    equipSkillMap["spear"] = CHARACTER_SKILL_SPEAR;
    equipSkillMap["bow"] = CHARACTER_SKILL_BOW;
    equipSkillMap["mace"] = CHARACTER_SKILL_MACE;
    equipSkillMap["blaster"] = CHARACTER_SKILL_BLASTER;
    equipSkillMap["shield"] = CHARACTER_SKILL_SHIELD;
    equipSkillMap["leather"] = CHARACTER_SKILL_LEATHER;
    equipSkillMap["chain"] = CHARACTER_SKILL_CHAIN;
    equipSkillMap["plate"] = CHARACTER_SKILL_PLATE;
    equipSkillMap["club"] = CHARACTER_SKILL_CLUB;

    std::map<std::string, ITEM_MATERIAL, ILess> materialMap;
    materialMap["artifact"] = MATERIAL_ARTIFACT;
    materialMap["relic"] = MATERIAL_RELIC;
    materialMap["special"] = MATERIAL_SPECIAL;

    char *lineContent;

    LoadPotions(resourceManager->getEventsFile("potion.txt"));
    LoadPotionNotes(resourceManager->getEventsFile("potnotes.txt"));

    std::string txtRaw;

    txtRaw = resourceManager->getEventsFile("stditems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    // Standard Bonuses by Group
    chanceByItemTypeSums.fill(0);
    for (CharacterAttributeType i : allEnchantableAttributes()) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        standardEnchantments[i].pBonusStat = removeQuotes(tokens[0]);
        standardEnchantments[i].pOfName = removeQuotes(tokens[1]);

        int k = 2;
        for (ITEM_EQUIP_TYPE equipType : standardEnchantments[i].chancesByItemType.indices()) {
            standardEnchantments[i].chancesByItemType[equipType] = atoi(tokens[k++]);
            chanceByItemTypeSums[equipType] += standardEnchantments[i].chancesByItemType[equipType];
        }
    }

    // Bonus range for Standard by Level
    strtokSkipLines(5);
    for (ItemTreasureLevel i : bonusRanges.indices()) {  // counted from 1
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() == 4 && "Invalid number of tokens");
        bonusRanges[i].minR = atoi(tokens[2]);
        bonusRanges[i].maxR = atoi(tokens[3]);
    }

    txtRaw = resourceManager->getEventsFile("spcitems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    for (ITEM_ENCHANTMENT i : pSpecialEnchantments.indices()) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() >= 17 && "Invalid number of tokens");
        pSpecialEnchantments[i].pBonusStatement = removeQuotes(tokens[0]);
        pSpecialEnchantments[i].pNameAdd = removeQuotes(tokens[1]);

        int k = 2;
        for (ITEM_EQUIP_TYPE j : pSpecialEnchantments[i].to_item_apply.indices())
            pSpecialEnchantments[i].to_item_apply[j] = atoi(tokens[k++]);

        int res = atoi(tokens[14]);
        int mask = 0;
        if (!res) {
            ++tokens[14];
            while (*tokens[14] == ' ')  // fix X 2 case
                ++tokens[14];
            res = atoi(tokens[14]);
            mask = 4;  // bit encode for when we need to multiply value
        }
        pSpecialEnchantments[i].iValue = res;
        pSpecialEnchantments[i].iTreasureLevel = (tolower(tokens[15][0]) - 'a') | mask;
    }

    pSpecialEnchantments_count = 72;

    txtRaw = resourceManager->getEventsFile("items.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(1);
    for (size_t line = 0; line < 799; line++) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');

        ItemId item_counter = ItemId(atoi(tokens[0]));
        pItems[item_counter].iconName = removeQuotes(tokens[1]);
        pItems[item_counter].name = removeQuotes(tokens[2]);
        pItems[item_counter].uValue = atoi(tokens[3]);
        pItems[item_counter].uEquipType = valueOr(equipStatMap, tokens[4], EQUIP_NONE);
        pItems[item_counter].uSkillType = valueOr(equipSkillMap, tokens[5], CHARACTER_SKILL_MISC);
        auto diceRollTokens = tokenize(tokens[6], 'd');
        if (diceRollTokens.size() == 2) {
            pItems[item_counter].uDamageDice = atoi(diceRollTokens[0]);
            pItems[item_counter].uDamageRoll = atoi(diceRollTokens[1]);
        } else if (tolower(diceRollTokens[0][0]) != 's') {
            pItems[item_counter].uDamageDice = atoi(diceRollTokens[0]);
            pItems[item_counter].uDamageRoll = 1;
        } else {
            pItems[item_counter].uDamageDice = 0;
            pItems[item_counter].uDamageRoll = 0;
        }
        pItems[item_counter].uDamageMod = atoi(tokens[7]);
        pItems[item_counter].uMaterial = valueOr(materialMap, tokens[8], MATERIAL_COMMON);
        pItems[item_counter].uItemID_Rep_St = atoi(tokens[9]);
        pItems[item_counter].pUnidentifiedName = removeQuotes(tokens[10]);
        pItems[item_counter].uSpriteID = static_cast<SPRITE_OBJECT_TYPE>(atoi(tokens[11]));

        pItems[item_counter]._additional_value = ITEM_ENCHANTMENT_NULL;
        pItems[item_counter]._bonus_type = {};
        if (pItems[item_counter].uMaterial == MATERIAL_SPECIAL) {
            for (CharacterAttributeType ii : allEnchantableAttributes()) {
                if (iequals(tokens[12], standardEnchantments[ii].pOfName)) {
                    pItems[item_counter]._bonus_type = ii;
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
        pItems[item_counter].pDescription = removeQuotes(tokens[16]);
    }

    txtRaw = resourceManager->getEventsFile("rnditems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    for(size_t line = 0; line < 618; line++) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() > 7 && "Invalid number of tokens");

        ItemId item_counter = ItemId(atoi(tokens[0]));
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
        pItems[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
    }

    // ChanceByTreasureLvl Summ - to calculate chance
    memset(&chanceByTreasureLevelSums, 0, 24);
    for (ItemTreasureLevel i : chanceByTreasureLevelSums.indices())
        for (ItemId j : pItems.indices())
            chanceByTreasureLevelSums[i] += pItems[j].uChanceByTreasureLvl[i];

    strtokSkipLines(5);
    for (int i = 0; i < 3; ++i) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() > 7 && "Invalid number of tokens");
        switch (i) {
            case 0:
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                uBonusChanceStandart[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
            case 1:
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                uBonusChanceSpecial[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
            case 2:
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                uBonusChanceWpSpecial[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
        }
    }

    ItemGen::PopulateSpecialBonusMap();
    ItemGen::PopulateArtifactBonusMap();
    ItemGen::PopulateRegularBonusMap();
}

//----- (00456D17) --------------------------------------------------------
void ItemTable::SetSpecialBonus(ItemGen *pItem) {
    if (pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL) {
        pItem->attributeEnchantment = pItems[pItem->uItemID]._bonus_type;
        pItem->special_enchantment =
                (ITEM_ENCHANTMENT)pItems[pItem->uItemID]._additional_value;
        pItem->m_enchantmentStrength = pItems[pItem->uItemID]._bonus_strength;
    }
}

//----- (00456D43) --------------------------------------------------------
bool ItemTable::IsMaterialSpecial(const ItemGen *pItem) {
    return this->pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL;
}

//----- (00456D5E) --------------------------------------------------------
bool ItemTable::IsMaterialNonCommon(const ItemGen *pItem) {
    return pItems[pItem->uItemID].uMaterial == MATERIAL_SPECIAL ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_RELIC ||
           pItems[pItem->uItemID].uMaterial == MATERIAL_ARTIFACT;
}

//----- (00453B3C) --------------------------------------------------------
void ItemTable::LoadPotions(const Blob &potions) {
    //    char Text[90];
    char *test_string;
    uint8_t potion_value;

    std::vector<char *> tokens;
    std::string txtRaw(potions.string_view());
    test_string = strtok(txtRaw.data(), "\r") + 1;
    while (test_string) {
        tokens = tokenize(test_string, '\t');
        if (!strcmp(tokens[0], "222")) break;
        test_string = strtok(NULL, "\r") + 1;
    }
    if (!test_string) {
        logger->error("Error Pre-Parsing Potion Table");
        return;
    }

    for (ItemId row : Segment<ItemId>(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
        if (tokens.size() < 50) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row) - std::to_underlying(ITEM_FIRST_REAL_POTION), tokens.size());
            return;
        }
        for (ItemId column : Segment<ItemId>(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
            int flatPotionId = std::to_underlying(column) - std::to_underlying(ITEM_FIRST_REAL_POTION);
            char *currValue = tokens[flatPotionId + 7];
            potion_value = atoi(currValue);
            if (!potion_value && currValue[0] == 'E') {
                // values like "E{x}" represent damage level {x} when using invalid potion combination
                potion_value = atoi(currValue + 1);
            }
            this->potionCombination[row][column] = (ItemId)potion_value;
        }

        test_string = strtok(NULL, "\r") + 1;
        if (!test_string) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row), 0);
            return;
        }
        tokens = tokenize(test_string, '\t');
    }
}

//----- (00453CE5) --------------------------------------------------------
void ItemTable::LoadPotionNotes(const Blob &potionNotes) {
    //  char Text[90];
    char *test_string;
    uint8_t potion_note;

    std::vector<char *> tokens;
    std::string txtRaw(potionNotes.string_view());
    test_string = strtok(txtRaw.data(), "\r") + 1;
    while (test_string) {
        tokens = tokenize(test_string, '\t');
        if (!strcmp(tokens[0], "222")) break;
        test_string = strtok(NULL, "\r") + 1;
    }
    if (!test_string) {
        logger->error("Error Pre-Parsing Potion Table");
        return;
    }

    for (ItemId row : Segment<ItemId>(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
        if (tokens.size() < 50) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row), tokens.size());
            return;
        }
        for (ItemId column : Segment<ItemId>(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
            int flatPotionId = std::to_underlying(column) - std::to_underlying(ITEM_FIRST_REAL_POTION);
            char *currValue = tokens[flatPotionId + 7];
            this->potionNotes[row][column] = atoi(currValue);
        }

        test_string = strtok(NULL, "\r") + 1;
        if (!test_string) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row) - std::to_underlying(ITEM_FIRST_REAL_POTION), 0);
            return;
        }
        tokens = tokenize(test_string, '\t');
    }
}

void ItemTable::generateItem(ItemTreasureLevel treasure_level, RandomItemType uTreasureType, ItemGen *outItem) {
    assert(isRandomTreasureLevel(treasure_level));

    int current_chance;           // ebx@43
    int tmp_chance;               // ecx@47
    ItemId artifactRandomId;       // ebx@57
    int v18;                      // edx@62
    //    unsigned int v34; // eax@97
    int j;              // eax@121
    std::array<ItemId, 800> spawnableRequestedItems;  // [sp+Ch] [bp-C88h]@33
    std::array<ITEM_ENCHANTMENT, 800> possibleSpecialIds;
    int total_chance;   // [sp+C8Ch] [bp-8h]@33
    int v57;            // [sp+CA0h] [bp+Ch]@62

    if (!outItem) outItem = (ItemGen*)malloc(sizeof(ItemGen));
    memset(outItem, 0, sizeof(*outItem));

    if (uTreasureType != RANDOM_ITEM_ANY) {  // generate known treasure type
        ITEM_EQUIP_TYPE requested_equip;
        CharacterSkillType requested_skill = CHARACTER_SKILL_INVALID;
        switch (uTreasureType) {
            case RANDOM_ITEM_WEAPON:
                requested_equip = EQUIP_SINGLE_HANDED;
                break;
            case RANDOM_ITEM_ARMOR:
                requested_equip = EQUIP_ARMOUR;
                break;
            case RANDOM_ITEM_MICS:
                requested_skill = CHARACTER_SKILL_MISC;
                break;
            case RANDOM_ITEM_SWORD:
                requested_skill = CHARACTER_SKILL_SWORD;
                break;
            case RANDOM_ITEM_DAGGER:
                requested_skill = CHARACTER_SKILL_DAGGER;
                break;
            case RANDOM_ITEM_AXE:
                requested_skill = CHARACTER_SKILL_AXE;
                break;
            case RANDOM_ITEM_SPEAR:
                requested_skill = CHARACTER_SKILL_SPEAR;
                break;
            case RANDOM_ITEM_BOW:
                requested_skill = CHARACTER_SKILL_BOW;
                break;
            case RANDOM_ITEM_MACE:
                requested_skill = CHARACTER_SKILL_MACE;
                break;
            case RANDOM_ITEM_CLUB:
                requested_skill = CHARACTER_SKILL_CLUB;
                break;
            case RANDOM_ITEM_STAFF:
                requested_skill = CHARACTER_SKILL_STAFF;
                break;
            case RANDOM_ITEM_LEATHER_ARMOR:
                requested_skill = CHARACTER_SKILL_LEATHER;
                break;
            case RANDOM_ITEM_CHAIN_ARMOR:
                requested_skill = CHARACTER_SKILL_CHAIN;
                break;
            case RANDOM_ITEM_PLATE_ARMOR:
                requested_skill = CHARACTER_SKILL_PLATE;
                break;
            case RANDOM_ITEM_SHIELD:
                requested_equip = EQUIP_SHIELD;
                break;
            case RANDOM_ITEM_HELMET:
                requested_equip = EQUIP_HELMET;
                break;
            case RANDOM_ITEM_BELT:
                requested_equip = EQUIP_BELT;
                break;
            case RANDOM_ITEM_CLOAK:
                requested_equip = EQUIP_CLOAK;
                break;
            case RANDOM_ITEM_GAUNTLETS:
                requested_equip = EQUIP_GAUNTLETS;
                break;
            case RANDOM_ITEM_BOOTS:
                requested_equip = EQUIP_BOOTS;
                break;
            case RANDOM_ITEM_RING:
                requested_equip = EQUIP_RING;
                break;
            case RANDOM_ITEM_AMULET:
                requested_equip = EQUIP_AMULET;
                break;
            case RANDOM_ITEM_WAND:
                requested_equip = EQUIP_WAND;
                break;
            case RANDOM_ITEM_SPELL_SCROLL:
                requested_equip = EQUIP_SPELL_SCROLL;
                break;
            case RANDOM_ITEM_POTION:
                requested_equip = EQUIP_POTION;
                break;
            case RANDOM_ITEM_REAGENT:
                requested_equip = EQUIP_REAGENT;
                break;
            case RANDOM_ITEM_GEM:
                requested_equip = EQUIP_GEM;
                break;
            default:
                __debugbreak();  // check this condition
                // TODO(captainurist): explore
                requested_equip = static_cast<ITEM_EQUIP_TYPE>(std::to_underlying(uTreasureType) - 1);
                break;
        }
        spawnableRequestedItems.fill(ITEM_NULL);
        total_chance = 0;
        j = 0;
        // a2a = 1;
        if (requested_skill == CHARACTER_SKILL_INVALID) {  // no skill for this item needed
            for (ItemId i : allSpawnableItems()) {
                if (pItems[i].uEquipType == requested_equip) {
                    spawnableRequestedItems[j] = i;
                    ++j;
                    total_chance +=
                            pItems[i].uChanceByTreasureLvl[treasure_level];
                }
            }
        } else {  // have needed skill
            for (ItemId itemId : allSpawnableItems()) {
                if (pItems[itemId].uSkillType == requested_skill) {
                    spawnableRequestedItems[j] = itemId;
                    ++j;
                    total_chance +=
                            pItems[itemId].uChanceByTreasureLvl[treasure_level];
                }
            }
        }

        current_chance = 0;
        if (total_chance) {
            current_chance = grng->random(total_chance) + 1;
            tmp_chance = 0;
            j = 0;
            while (tmp_chance < current_chance) {
                outItem->uItemID = spawnableRequestedItems[j];
                tmp_chance += pItems[spawnableRequestedItems[j]].uChanceByTreasureLvl[treasure_level];
                ++j;
            }
        } else {
            outItem->uItemID = ITEM_CRUDE_LONGSWORD;
        }
    } else {
        // artifact
        if (treasure_level == ITEM_TREASURE_LEVEL_6) {
            int artifactsFound = 0;  // [sp+CA0h] [bp+Ch]@55
            for (ItemId i : allSpawnableArtifacts())
                artifactsFound += pParty->pIsArtifactFound[i];
            artifactRandomId = grng->randomSample(allSpawnableArtifacts());
            if ((grng->random(100) < 5) && !pParty->pIsArtifactFound[artifactRandomId] &&
                (engine->config->gameplay.ArtifactLimit.value() == 0 || artifactsFound < engine->config->gameplay.ArtifactLimit.value())) {
                pParty->pIsArtifactFound[artifactRandomId] = 1;
                outItem->uAttributes = 0;
                outItem->uItemID = artifactRandomId;
                SetSpecialBonus(outItem);
                return;
            }
        }

        v57 = 0;
        v18 = grng->random(this->chanceByTreasureLevelSums[treasure_level]) + 1;
        while (v57 < v18) {
            // TODO(captainurist): what's going on here? Get rid of casts.
            outItem->uItemID = ItemId(std::to_underlying(outItem->uItemID) + 1);
            v57 += pItems[outItem->uItemID].uChanceByTreasureLvl[treasure_level];
        }
    }
    if (outItem->isPotion() && outItem->uItemID != ITEM_POTION_BOTTLE) {  // if it potion set potion spec
        outItem->potionPower = 0;
        for (int i = 0; i < 2; ++i) {
            outItem->potionPower += grng->random(4) + 1;
        }
        outItem->potionPower = outItem->potionPower * std::to_underlying(treasure_level);
    }

    if (outItem->uItemID == ITEM_SPELLBOOK_DIVINE_INTERVENTION && !pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED])
        outItem->uItemID = ITEM_SPELLBOOK_SUNRAY;
    if (pItemTable->pItems[outItem->uItemID].uItemID_Rep_St)
        outItem->uAttributes = 0;
    else
        outItem->uAttributes = ITEM_IDENTIFIED;

    if (!outItem->isPotion()) {
        outItem->special_enchantment = ITEM_ENCHANTMENT_NULL;
        outItem->attributeEnchantment = {};
    }
    // try get special enhansment
    switch (outItem->GetItemEquipType()) {
        case EQUIP_SINGLE_HANDED:
        case EQUIP_TWO_HANDED:
        case EQUIP_BOW:
            if (!uBonusChanceWpSpecial[treasure_level]) return;
            if (grng->random(100) >= uBonusChanceWpSpecial[treasure_level])
                return;
            break;
        case EQUIP_ARMOUR:
        case EQUIP_SHIELD:
        case EQUIP_HELMET:
        case EQUIP_BELT:
        case EQUIP_CLOAK:
        case EQUIP_GAUNTLETS:
        case EQUIP_BOOTS:
        case EQUIP_RING: {
            if (!uBonusChanceStandart[treasure_level]) return;
            int bonusChanceRoll = grng->random(100);  // edx@86
            if (bonusChanceRoll < uBonusChanceStandart[treasure_level]) {
                int enchantmentChanceSumRoll = grng->random(chanceByItemTypeSums[outItem->GetItemEquipType()]) + 1;
                int currentEnchantmentChancesSum = 0;
                for (CharacterAttributeType attr : allEnchantableAttributes()) {
                    if (currentEnchantmentChancesSum >= enchantmentChanceSumRoll)
                        break;

                    currentEnchantmentChancesSum += standardEnchantments[attr].chancesByItemType[outItem->GetItemEquipType()];
                    outItem->attributeEnchantment = attr;
                }
                assert(outItem->attributeEnchantment);

                outItem->m_enchantmentStrength = bonusRanges[treasure_level].minR +
                                                 grng->random(bonusRanges[treasure_level].maxR - bonusRanges[treasure_level].minR + 1);
                CharacterAttributeType standardEnchantmentAttributeSkill = *outItem->attributeEnchantment;
                if (standardEnchantmentAttributeSkill == CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER ||
                    standardEnchantmentAttributeSkill == CHARACTER_ATTRIBUTE_SKILL_DODGE ||
                    standardEnchantmentAttributeSkill == CHARACTER_ATTRIBUTE_SKILL_UNARMED) {
                    outItem->m_enchantmentStrength /= 2;
                }
                // if enchantment generated, it needs to actually have an effect
                if (outItem->m_enchantmentStrength <= 0) {
                    outItem->m_enchantmentStrength = 1;
                }
                return;

            } else if (bonusChanceRoll >= uBonusChanceStandart[treasure_level] + uBonusChanceSpecial[treasure_level]) {
                return;
            }
        }
            break;
        case EQUIP_WAND:
            outItem->uNumCharges = grng->random(6) + outItem->GetDamageMod() + 1;
            outItem->uMaxCharges = outItem->uNumCharges;
            return;
        default:
            return;
    }

    j = 0;
    int spc_sum = 0;
    int spc;
    possibleSpecialIds.fill(ITEM_ENCHANTMENT_NULL);
    for (ITEM_ENCHANTMENT i : pSpecialEnchantments.indices()) {
        int tr_lv = (pSpecialEnchantments[i].iTreasureLevel) & 3;

        // tr_lv  0 = treasure level 3/4
        // tr_lv  1 = treasure level 3/4/5
        // tr_lv  2 = treasure level 4/5
        // tr_lv  3 = treasure level 5/6

        if ((treasure_level == ITEM_TREASURE_LEVEL_3) && (tr_lv == 1 || tr_lv == 0) ||
            (treasure_level == ITEM_TREASURE_LEVEL_4) && (tr_lv == 2 || tr_lv == 1 || tr_lv == 0) ||
            (treasure_level == ITEM_TREASURE_LEVEL_5) && (tr_lv == 3 || tr_lv == 2 || tr_lv == 1) ||
            (treasure_level == ITEM_TREASURE_LEVEL_6) && (tr_lv == 3)) {
            spc = pSpecialEnchantments[i].to_item_apply[outItem->GetItemEquipType()];
            spc_sum += spc;
            if (spc) {
                possibleSpecialIds[j++] = i;
            }
        }
    }

    int target = grng->random(spc_sum);
    for (int currentSum = 0, k = 0; k < j; k++) {
        currentSum += pSpecialEnchantments[possibleSpecialIds[k]].to_item_apply[outItem->GetItemEquipType()];
        if (currentSum > target) {
            outItem->special_enchantment = possibleSpecialIds[k];
            return;
        }
    }
    assert(false); // Should never get here.
}
