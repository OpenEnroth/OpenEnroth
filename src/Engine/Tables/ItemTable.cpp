#include "ItemTable.h"

#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <utility>

#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/Party.h"
#include "Engine/GameResourceManager.h"

#include "GUI/UI/UIHouses.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

static void strtokSkipLines(int n) {
    for (int i = 0; i < n; ++i) {
        (void)strtok(NULL, "\r");
    }
}

//----- (00456D84) --------------------------------------------------------
void ItemTable::Initialize(GameResourceManager *resourceManager) {
    std::map<std::string, ItemType, ascii::NoCaseLess> equipStatMap; // TODO(captainurist): #enum use enum serialization
    equipStatMap["weapon"] = ITEM_TYPE_SINGLE_HANDED;
    equipStatMap["weapon2"] = ITEM_TYPE_TWO_HANDED;
    equipStatMap["weapon1or2"] = ITEM_TYPE_SINGLE_HANDED;
    equipStatMap["missile"] = ITEM_TYPE_BOW;
    equipStatMap["bow"] = ITEM_TYPE_BOW;
    equipStatMap["armor"] = ITEM_TYPE_ARMOUR;
    equipStatMap["shield"] = ITEM_TYPE_SHIELD;
    equipStatMap["helm"] = ITEM_TYPE_HELMET;
    equipStatMap["belt"] = ITEM_TYPE_BELT;
    equipStatMap["cloak"] = ITEM_TYPE_CLOAK;
    equipStatMap["gauntlets"] = ITEM_TYPE_GAUNTLETS;
    equipStatMap["boots"] = ITEM_TYPE_BOOTS;
    equipStatMap["ring"] = ITEM_TYPE_RING;
    equipStatMap["amulet"] = ITEM_TYPE_AMULET;
    equipStatMap["weaponw"] = ITEM_TYPE_WAND;
    equipStatMap["herb"] = ITEM_TYPE_REAGENT;
    equipStatMap["reagent"] = ITEM_TYPE_REAGENT;
    equipStatMap["bottle"] = ITEM_TYPE_POTION;
    equipStatMap["sscroll"] = ITEM_TYPE_SPELL_SCROLL;
    equipStatMap["book"] = ITEM_TYPE_BOOK;
    equipStatMap["mscroll"] = ITEM_TYPE_MESSAGE_SCROLL;
    equipStatMap["gold"] = ITEM_TYPE_GOLD;
    equipStatMap["gem"] = ITEM_TYPE_GEM;

    std::map<std::string, CharacterSkillType, ascii::NoCaseLess> equipSkillMap;
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

    std::map<std::string, ItemRarity, ascii::NoCaseLess> materialMap;
    materialMap["artifact"] = RARITY_ARTIFACT;
    materialMap["relic"] = RARITY_RELIC;
    materialMap["special"] = RARITY_SPECIAL;

    char *lineContent;

    LoadPotions(resourceManager->getEventsFile("potion.txt"));
    LoadPotionNotes(resourceManager->getEventsFile("potnotes.txt"));

    std::string txtRaw;

    txtRaw = resourceManager->getEventsFile("stditems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    // Standard Bonuses by Group
    standardEnchantmentChanceSumByItemType.fill(0);
    for (CharacterAttribute i : allEnchantableAttributes()) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        standardEnchantments[i].attributeName = removeQuotes(tokens[0]);
        standardEnchantments[i].itemSuffix = removeQuotes(tokens[1]);

        int k = 2;
        for (ItemType equipType : standardEnchantments[i].chanceByItemType.indices()) {
            standardEnchantments[i].chanceByItemType[equipType] = atoi(tokens[k++]);
            standardEnchantmentChanceSumByItemType[equipType] += standardEnchantments[i].chanceByItemType[equipType];
        }
    }

    // Bonus range for Standard by Level
    strtokSkipLines(5);
    for (ItemTreasureLevel i : standardEnchantmentRangeByTreasureLevel.indices()) {  // counted from 1
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() == 4 && "Invalid number of tokens");
        standardEnchantmentRangeByTreasureLevel[i] = Segment(atoi(tokens[2]), atoi(tokens[3]));
    }

    txtRaw = resourceManager->getEventsFile("spcitems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    for (ItemEnchantment i : specialEnchantments.indices()) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() >= 17 && "Invalid number of tokens");
        specialEnchantments[i].description = removeQuotes(tokens[0]);
        specialEnchantments[i].itemSuffixOrPrefix = removeQuotes(tokens[1]);

        int k = 2;
        for (ItemType j : specialEnchantments[i].chanceByItemType.indices())
            specialEnchantments[i].chanceByItemType[j] = atoi(tokens[k++]);

        int res = atoi(tokens[14]);
        int mask = 0;
        if (!res) {
            ++tokens[14];
            while (*tokens[14] == ' ')  // fix X 2 case
                ++tokens[14];
            res = atoi(tokens[14]);
            mask = 4;  // bit encode for when we need to multiply value
        }
        specialEnchantments[i].additionalValue = res;
        specialEnchantments[i].iTreasureLevel = (tolower(tokens[15][0]) - 'a') | mask;
    }

    txtRaw = resourceManager->getEventsFile("items.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(1);
    for (size_t line = 0; line < 799; line++) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');

        ItemId item_counter = ItemId(atoi(tokens[0]));
        items[item_counter].iconName = removeQuotes(tokens[1]);
        items[item_counter].name = removeQuotes(tokens[2]);
        items[item_counter].baseValue = atoi(tokens[3]);
        items[item_counter].type = valueOr(equipStatMap, tokens[4], ITEM_TYPE_NONE);
        items[item_counter].skill = valueOr(equipSkillMap, tokens[5], CHARACTER_SKILL_MISC);
        auto diceRollTokens = tokenize(tokens[6], 'd');
        if (diceRollTokens.size() == 2) {
            items[item_counter].damageDice = atoi(diceRollTokens[0]);
            items[item_counter].damageRoll = atoi(diceRollTokens[1]);
        } else if (tolower(diceRollTokens[0][0]) != 's') {
            items[item_counter].damageDice = atoi(diceRollTokens[0]);
            items[item_counter].damageRoll = 1;
        } else {
            items[item_counter].damageDice = 0;
            items[item_counter].damageRoll = 0;
        }
        items[item_counter].damageMod = atoi(tokens[7]);
        items[item_counter].rarity = valueOr(materialMap, tokens[8], RARITY_COMMON);
        items[item_counter].identifyDifficulty = atoi(tokens[9]);
        items[item_counter].unidentifiedName = removeQuotes(tokens[10]);
        items[item_counter].spriteId = static_cast<SpriteId>(atoi(tokens[11]));

        items[item_counter].specialEnchantment = ITEM_ENCHANTMENT_NULL;
        items[item_counter].standardEnchantment = {};
        if (items[item_counter].rarity == RARITY_SPECIAL) {
            for (CharacterAttribute ii : allEnchantableAttributes()) {
                if (ascii::noCaseEquals(tokens[12], standardEnchantments[ii].itemSuffix)) { // TODO(captainurist): #unicode this is not ascii
                    items[item_counter].standardEnchantment = ii;
                    break;
                }
            }
            if (!items[item_counter].standardEnchantment) {
                for (ItemEnchantment ii : specialEnchantments.indices()) {
                    if (ascii::noCaseEquals(tokens[12], specialEnchantments[ii].itemSuffixOrPrefix)) { // TODO(captainurist): #unicode this is not ascii
                        items[item_counter].specialEnchantment = ii;
                    }
                }
            }
        }

        if ((items[item_counter].rarity == RARITY_SPECIAL) &&
            (items[item_counter].standardEnchantment)) {
            char b_s = atoi(tokens[13]);
            if (b_s)
                items[item_counter].standardEnchantmentStrength = b_s;
            else
                items[item_counter].standardEnchantmentStrength = 1;
        } else {
            items[item_counter].standardEnchantmentStrength = 0;
        }
        items[item_counter].paperdollAnchorOffset.x = atoi(tokens[14]);
        items[item_counter].paperdollAnchorOffset.y = atoi(tokens[15]);
        items[item_counter].description = removeQuotes(tokens[16]);
    }

    txtRaw = resourceManager->getEventsFile("rnditems.txt").string_view();
    strtok(txtRaw.data(), "\r");
    strtokSkipLines(3);
    for(size_t line = 0; line < 618; line++) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() > 7 && "Invalid number of tokens");

        ItemId item_counter = ItemId(atoi(tokens[0]));
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
    }

    // ChanceByTreasureLvl Summ - to calculate chance
    itemChanceSumByTreasureLevel.fill(0);
    for (ItemTreasureLevel i : itemChanceSumByTreasureLevel.indices())
        for (ItemId j : items.indices())
            itemChanceSumByTreasureLevel[i] += items[j].uChanceByTreasureLvl[i];

    strtokSkipLines(5);
    for (int i = 0; i < 3; ++i) {
        lineContent = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(lineContent, '\t');
        assert(tokens.size() > 7 && "Invalid number of tokens");
        switch (i) {
            case 0:
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                standardEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
            case 1:
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                specialEnchantmentChanceForEquipment[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
            case 2:
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_1] = atoi(tokens[2]);
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_2] = atoi(tokens[3]);
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_3] = atoi(tokens[4]);
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_4] = atoi(tokens[5]);
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_5] = atoi(tokens[6]);
                specialEnchantmentChanceForWeapons[ITEM_TREASURE_LEVEL_6] = atoi(tokens[7]);
                break;
        }
    }

    Item::PopulateSpecialBonusMap();
    Item::PopulateArtifactBonusMap();
    Item::PopulateRegularBonusMap();
    LoadItemSizes();
}

//----- (00453B3C) --------------------------------------------------------
void ItemTable::LoadPotions(const Blob &potions) {
    //    char Text[90];
    char *test_string;
    ItemId potion_value;

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

    for (ItemId row : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
        if (tokens.size() < 50) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row) - std::to_underlying(ITEM_FIRST_REAL_POTION), tokens.size());
            return;
        }
        for (ItemId column : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
            int flatPotionId = std::to_underlying(column) - std::to_underlying(ITEM_FIRST_REAL_POTION);
            char *currValue = tokens[flatPotionId + 7];
            potion_value = static_cast<ItemId>(atoi(currValue));
            if (potion_value == ITEM_NULL && currValue[0] == 'E') {
                // values like "E{x}" represent damage level {x} when using invalid potion combination
                potion_value = static_cast<ItemId>(atoi(currValue + 1));
            }
            this->potionCombination[row][column] = potion_value;
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

    for (ItemId row : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
        if (tokens.size() < 50) {
            logger->error("Error Parsing Potion Table at Row: {} Column: {}", std::to_underlying(row), tokens.size());
            return;
        }
        for (ItemId column : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
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

void ItemTable::LoadItemSizes() {
    // Item sizes are loaded at startup directly from LOD image headers. This would have been an overkill back in 1999
    // (think about all these random reads from your HDD) but is totally fine today. Another option would've been to
    // precalculate these and place in a json file, but why precalculate what's cheap to recalculate?
    LodReader reader(dfs->read("data/icons.lod"));

    for (ItemId itemId : items.indices()) {
        std::string iconName = items[itemId].iconName;

        Sizei iconSize(1, 1); // Actual icon name that will be used in this case is "pending", see LodTextureCache.
        if (reader.exists(iconName))
            iconSize = lod::decodeImageSize(reader.read(iconName));

        itemSizes[itemId] = Sizei(GetSizeInInventorySlots(iconSize.w), GetSizeInInventorySlots(iconSize.h));
    }
}

void ItemTable::generateItem(ItemTreasureLevel treasureLevel, RandomItemType uTreasureType, Item *outItem) {
    assert(isRandomTreasureLevel(treasureLevel));

    std::vector<ItemId> possibleItems;
    std::vector<ItemEnchantment> possibleEnchantments;
    std::vector<int> cumulativeWeights;
    int weightSum = 0;

    assert(outItem != NULL);
    *outItem = Item();

    if (uTreasureType != RANDOM_ITEM_ANY) {  // generate known treasure type
        ItemType requestedEquip;
        CharacterSkillType requestedSkill = CHARACTER_SKILL_INVALID;
        switch (uTreasureType) {
            case RANDOM_ITEM_WEAPON:
                requestedEquip = ITEM_TYPE_SINGLE_HANDED;
                break;
            case RANDOM_ITEM_ARMOR:
                requestedEquip = ITEM_TYPE_ARMOUR;
                break;
            case RANDOM_ITEM_MICS:
                requestedSkill = CHARACTER_SKILL_MISC;
                break;
            case RANDOM_ITEM_SWORD:
                requestedSkill = CHARACTER_SKILL_SWORD;
                break;
            case RANDOM_ITEM_DAGGER:
                requestedSkill = CHARACTER_SKILL_DAGGER;
                break;
            case RANDOM_ITEM_AXE:
                requestedSkill = CHARACTER_SKILL_AXE;
                break;
            case RANDOM_ITEM_SPEAR:
                requestedSkill = CHARACTER_SKILL_SPEAR;
                break;
            case RANDOM_ITEM_BOW:
                requestedSkill = CHARACTER_SKILL_BOW;
                break;
            case RANDOM_ITEM_MACE:
                requestedSkill = CHARACTER_SKILL_MACE;
                break;
            case RANDOM_ITEM_CLUB:
                requestedSkill = CHARACTER_SKILL_CLUB;
                break;
            case RANDOM_ITEM_STAFF:
                requestedSkill = CHARACTER_SKILL_STAFF;
                break;
            case RANDOM_ITEM_LEATHER_ARMOR:
                requestedSkill = CHARACTER_SKILL_LEATHER;
                break;
            case RANDOM_ITEM_CHAIN_ARMOR:
                requestedSkill = CHARACTER_SKILL_CHAIN;
                break;
            case RANDOM_ITEM_PLATE_ARMOR:
                requestedSkill = CHARACTER_SKILL_PLATE;
                break;
            case RANDOM_ITEM_SHIELD:
                requestedEquip = ITEM_TYPE_SHIELD;
                break;
            case RANDOM_ITEM_HELMET:
                requestedEquip = ITEM_TYPE_HELMET;
                break;
            case RANDOM_ITEM_BELT:
                requestedEquip = ITEM_TYPE_BELT;
                break;
            case RANDOM_ITEM_CLOAK:
                requestedEquip = ITEM_TYPE_CLOAK;
                break;
            case RANDOM_ITEM_GAUNTLETS:
                requestedEquip = ITEM_TYPE_GAUNTLETS;
                break;
            case RANDOM_ITEM_BOOTS:
                requestedEquip = ITEM_TYPE_BOOTS;
                break;
            case RANDOM_ITEM_RING:
                requestedEquip = ITEM_TYPE_RING;
                break;
            case RANDOM_ITEM_AMULET:
                requestedEquip = ITEM_TYPE_AMULET;
                break;
            case RANDOM_ITEM_WAND:
                requestedEquip = ITEM_TYPE_WAND;
                break;
            case RANDOM_ITEM_SPELL_SCROLL:
                requestedEquip = ITEM_TYPE_SPELL_SCROLL;
                break;
            case RANDOM_ITEM_POTION:
                requestedEquip = ITEM_TYPE_POTION;
                break;
            case RANDOM_ITEM_REAGENT:
                requestedEquip = ITEM_TYPE_REAGENT;
                break;
            case RANDOM_ITEM_GEM:
                requestedEquip = ITEM_TYPE_GEM;
                break;
            default:
                assert(false);  // check this condition
                // TODO(captainurist): explore
                requestedEquip = static_cast<ItemType>(std::to_underlying(uTreasureType) - 1);
                break;
        }

        if (requestedSkill == CHARACTER_SKILL_INVALID) {  // no skill for this item needed
            for (ItemId itemId : allSpawnableItems()) {
                if (items[itemId].type == requestedEquip) {
                    if (items[itemId].uChanceByTreasureLvl[treasureLevel]) {
                        weightSum += items[itemId].uChanceByTreasureLvl[treasureLevel];
                        possibleItems.push_back(itemId);
                        cumulativeWeights.push_back(weightSum);
                    }
                }
            }
        } else {  // have needed skill
            for (ItemId itemId : allSpawnableItems()) {
                if (items[itemId].skill == requestedSkill) {
                    if (items[itemId].uChanceByTreasureLvl[treasureLevel]) {
                        weightSum += items[itemId].uChanceByTreasureLvl[treasureLevel];
                        possibleItems.push_back(itemId);
                        cumulativeWeights.push_back(weightSum);
                    }
                }
            }
        }

        if (weightSum) {
            int pickedWeight = grng->random(weightSum) + 1;
            auto foundWeight = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), pickedWeight);

            assert(foundWeight != cumulativeWeights.end());

            outItem->itemId = possibleItems[std::distance(cumulativeWeights.begin(), foundWeight)];
        } else {
            outItem->itemId = ITEM_CRUDE_LONGSWORD;
        }
    } else {
        // Trying to generate artifact
        if (treasureLevel == ITEM_TREASURE_LEVEL_6) {
            int artifactsFound = 0;
            ItemId artifactRandomId = grng->randomSample(allSpawnableArtifacts());
            for (ItemId i : allSpawnableArtifacts())
                artifactsFound += pParty->pIsArtifactFound[i];
            bool artifactLimitReached = (engine->config->gameplay.ArtifactLimit.value() != 0 && artifactsFound >= engine->config->gameplay.ArtifactLimit.value());
            if ((grng->random(100) < 5) && !pParty->pIsArtifactFound[artifactRandomId] && !artifactLimitReached) {
                pParty->pIsArtifactFound[artifactRandomId] = true;
                outItem->flags = 0;
                outItem->itemId = artifactRandomId;
                outItem->postGenerate(ITEM_SOURCE_UNKNOWN);
                return;
            }
        }

        // Otherwise try to spawn any random item
        int randomWeight = grng->random(this->itemChanceSumByTreasureLevel[treasureLevel]) + 1;
        for (ItemId itemId : allSpawnableItems()) {
            weightSum += items[itemId].uChanceByTreasureLvl[treasureLevel];
            if (weightSum >= randomWeight) {
                outItem->itemId = itemId;
                break;
            }
        }
    }
    if (outItem->isPotion() && outItem->itemId != ITEM_POTION_BOTTLE) {  // if it potion set potion spec
        outItem->potionPower = grng->randomDice(2, 4) * std::to_underlying(treasureLevel);
    }

    if (outItem->itemId == ITEM_SPELLBOOK_DIVINE_INTERVENTION && !pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED])
        outItem->itemId = ITEM_SPELLBOOK_SUNRAY;
    if (pItemTable->items[outItem->itemId].identifyDifficulty)
        outItem->flags = 0;
    else
        outItem->flags = ITEM_IDENTIFIED;

    if (!outItem->isPotion()) {
        outItem->specialEnchantment = ITEM_ENCHANTMENT_NULL;
        outItem->standardEnchantment = {};
    }
    // try get special enchantment
    switch (outItem->type()) {
        case ITEM_TYPE_SINGLE_HANDED:
        case ITEM_TYPE_TWO_HANDED:
        case ITEM_TYPE_BOW:
            if (!specialEnchantmentChanceForWeapons[treasureLevel] || grng->random(100) >= specialEnchantmentChanceForWeapons[treasureLevel])
                return;
            break;
        case ITEM_TYPE_ARMOUR:
        case ITEM_TYPE_SHIELD:
        case ITEM_TYPE_HELMET:
        case ITEM_TYPE_BELT:
        case ITEM_TYPE_CLOAK:
        case ITEM_TYPE_GAUNTLETS:
        case ITEM_TYPE_BOOTS:
        case ITEM_TYPE_RING:
        case ITEM_TYPE_AMULET: {
            if (!standardEnchantmentChanceForEquipment[treasureLevel])
                return;
            int bonusChanceRoll = grng->random(100);
            if (bonusChanceRoll < standardEnchantmentChanceForEquipment[treasureLevel]) {
                int enchantmentChanceSumRoll = grng->random(standardEnchantmentChanceSumByItemType[outItem->type()]) + 1;
                int currentEnchantmentChancesSum = 0;
                for (CharacterAttribute attr : allEnchantableAttributes()) {
                    if (currentEnchantmentChancesSum >= enchantmentChanceSumRoll)
                        break;

                    currentEnchantmentChancesSum += standardEnchantments[attr].chanceByItemType[outItem->type()];
                    outItem->standardEnchantment = attr;
                }
                assert(outItem->standardEnchantment);

                outItem->standardEnchantmentStrength = grng->randomSample(standardEnchantmentRangeByTreasureLevel[treasureLevel]);
                CharacterAttribute standardEnchantmentAttributeSkill = *outItem->standardEnchantment;
                if (standardEnchantmentAttributeSkill == ATTRIBUTE_SKILL_ARMSMASTER ||
                    standardEnchantmentAttributeSkill == ATTRIBUTE_SKILL_DODGE ||
                    standardEnchantmentAttributeSkill == ATTRIBUTE_SKILL_UNARMED) {
                    outItem->standardEnchantmentStrength /= 2;
                }
                // if enchantment generated, it needs to actually have an effect
                if (outItem->standardEnchantmentStrength <= 0) {
                    outItem->standardEnchantmentStrength = 1;
                }
                return;
            } else if (bonusChanceRoll >= standardEnchantmentChanceForEquipment[treasureLevel] + specialEnchantmentChanceForEquipment[treasureLevel]) {
                return;
            }
        }
            break;
        case ITEM_TYPE_WAND:
            outItem->numCharges = grng->random(6) + outItem->GetDamageMod() + 1;
            outItem->maxCharges = outItem->numCharges;
            return;
        default:
            return;
    }

    cumulativeWeights.clear();
    weightSum = 0;
    for (ItemEnchantment ench : specialEnchantments.indices()) {
        int tr_lv = (specialEnchantments[ench].iTreasureLevel) & 3;

        // tr_lv  0 = treasure level 3/4
        // tr_lv  1 = treasure level 3/4/5
        // tr_lv  2 = treasure level 4/5
        // tr_lv  3 = treasure level 5/6

        if ((treasureLevel == ITEM_TREASURE_LEVEL_3) && (tr_lv == 1 || tr_lv == 0) ||
            (treasureLevel == ITEM_TREASURE_LEVEL_4) && (tr_lv == 2 || tr_lv == 1 || tr_lv == 0) ||
            (treasureLevel == ITEM_TREASURE_LEVEL_5) && (tr_lv == 3 || tr_lv == 2 || tr_lv == 1) ||
            (treasureLevel == ITEM_TREASURE_LEVEL_6) && (tr_lv == 3)) {
            int spc = specialEnchantments[ench].chanceByItemType[outItem->type()];
            if (spc) {
                weightSum += spc;
                possibleEnchantments.push_back(ench);
                cumulativeWeights.push_back(weightSum);
            }
        }
    }

    int pickedWeight = grng->random(weightSum) + 1;
    auto foundWeight = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), pickedWeight);
    assert(foundWeight != cumulativeWeights.end());
    outItem->specialEnchantment = possibleEnchantments[std::distance(cumulativeWeights.begin(), foundWeight)];
}
