#include "ItemTable.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

#include "Library/Serialization/Serialization.h"

#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Party.h"
#include "Engine/Resources/ResourceManager.h"

#include "GUI/UI/UIHouses.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

void ItemTable::LoadStandardEnchantments(std::string_view stditems) {
    // stditems.txt has two sections.
    std::vector<std::string_view> lines = split(stditems).by("\r\n").drop(4).skip("");

    // #1 Standard Bonuses by Group: attribute name (localized) | suffix (localized) | chance by item type....
    standardEnchantmentChanceSumByItemType.fill(0);
    for (auto [line, i] : view(lines).zip(allEnchantableAttributes())) {
        std::array<std::string_view, 11> tokens = split(line).by('\t');
        standardEnchantments[i].attributeName = unquote(tokens[0]);
        standardEnchantments[i].itemSuffix = unquote(tokens[1]);

        int k = 2;
        for (ItemType equipType : standardEnchantments[i].chanceByItemType.indices()) {
            standardEnchantments[i].chanceByItemType[equipType] = fromString<int>(tokens[k++]);
            standardEnchantmentChanceSumByItemType[equipType] += standardEnchantments[i].chanceByItemType[equipType];
        }
    }

    // #2 Bonus range for Standard by Level: (empty) | treasure level | min | max.
    for (auto [line, i] : view(lines).drop(allEnchantableAttributes().size() + 3).zip(standardEnchantmentRangeByTreasureLevel.indices())) {
        std::array<std::string_view, 4> rangeTokens = split(line).by('\t');
        standardEnchantmentRangeByTreasureLevel[i] = Segment(fromString<int>(rangeTokens[2]), fromString<int>(rangeTokens[3]));
    }
}

void ItemTable::LoadSpecialEnchantments(std::string_view spcitems) {
    // spcitems.txt table structure: description (localized) | suffix/prefix (localized) | chance by item type... | gold value | enchantment level.
    for (auto [line, i] : split(spcitems).by("\r\n").drop(4).skip("").zip(specialEnchantments.indices())) {
        std::array<std::string_view, 17> tokens = split(line).by('\t');
        specialEnchantments[i].description = unquote(tokens[0]);
        specialEnchantments[i].itemSuffixOrPrefix = unquote(tokens[1]);

        int k = 2;
        for (ItemType j : specialEnchantments[i].chanceByItemType.indices())
            specialEnchantments[i].chanceByItemType[j] = fromString<int>(tokens[k++]);

        std::string_view token14 = tokens[14];
        int res;
        bool isMul = !token14.empty() && (token14[0] == 'x' || token14[0] == 'X');
        if (isMul) {
            // "X 2" or "x2" — strip prefix, parse number.
            token14.remove_prefix(1);
            while (!token14.empty() && token14[0] == ' ')
                token14.remove_prefix(1);
            res = fromString<int>(token14);
        } else {
            res = fromString<int>(token14);
        }
        if (isMul)
            specialEnchantments[i].valueMul = res;
        else
            specialEnchantments[i].valueAdd = res;
        specialEnchantments[i].enchantmentLevel = !tokens[15].empty() ? (tolower(tokens[15][0]) - 'a') : 0;
    }
}

void ItemTable::LoadItems(std::string_view itemsBlob) {
    // items.txt table structure: index | icon | name (localized) | value | type | skill | damage | mod | material | ...

    static const std::map<std::string, ItemType, ascii::NoCaseLess> equipStatMap = { // TODO(captainurist): #enum use enum serialization
        {"weapon", ITEM_TYPE_SINGLE_HANDED},
        {"weapon2", ITEM_TYPE_TWO_HANDED},
        {"weapon1or2", ITEM_TYPE_SINGLE_HANDED},
        {"missile", ITEM_TYPE_BOW},
        {"bow", ITEM_TYPE_BOW},
        {"armor", ITEM_TYPE_ARMOUR},
        {"shield", ITEM_TYPE_SHIELD},
        {"helm", ITEM_TYPE_HELMET},
        {"belt", ITEM_TYPE_BELT},
        {"cloak", ITEM_TYPE_CLOAK},
        {"gauntlets", ITEM_TYPE_GAUNTLETS},
        {"boots", ITEM_TYPE_BOOTS},
        {"ring", ITEM_TYPE_RING},
        {"amulet", ITEM_TYPE_AMULET},
        {"weaponw", ITEM_TYPE_WAND},
        {"herb", ITEM_TYPE_REAGENT},
        {"reagent", ITEM_TYPE_REAGENT},
        {"bottle", ITEM_TYPE_POTION},
        {"sscroll", ITEM_TYPE_SPELL_SCROLL},
        {"book", ITEM_TYPE_BOOK},
        {"mscroll", ITEM_TYPE_MESSAGE_SCROLL},
        {"gold", ITEM_TYPE_GOLD},
        {"gem", ITEM_TYPE_GEM},
    };

    static const std::map<std::string, Skill, ascii::NoCaseLess> equipSkillMap = {
        {"staff", SKILL_STAFF},
        {"sword", SKILL_SWORD},
        {"dagger", SKILL_DAGGER},
        {"axe", SKILL_AXE},
        {"spear", SKILL_SPEAR},
        {"bow", SKILL_BOW},
        {"mace", SKILL_MACE},
        {"blaster", SKILL_BLASTER},
        {"shield", SKILL_SHIELD},
        {"leather", SKILL_LEATHER},
        {"chain", SKILL_CHAIN},
        {"plate", SKILL_PLATE},
        {"club", SKILL_CLUB},
    };

    static const std::map<std::string, ItemRarity, ascii::NoCaseLess> materialMap = {
        {"artifact", RARITY_ARTIFACT},
        {"relic", RARITY_RELIC},
        {"special", RARITY_SPECIAL},
    };

    for (std::string_view line : split(itemsBlob).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 17> tokens = split(line).by('\t');
        ItemId item_counter = ItemId(fromString<int>(tokens[0]));
        items[item_counter].iconName = unquote(tokens[1]);
        items[item_counter].name = unquote(tokens[2]);
        items[item_counter].baseValue = fromString<int>(tokens[3]);
        items[item_counter].type = valueOr(equipStatMap, tokens[4], ITEM_TYPE_NONE);
        items[item_counter].skill = valueOr(equipSkillMap, tokens[5], SKILL_MISC);
        std::array<std::string_view, 2> diceRollTokens = split(tokens[6]).by('d');
        char damagePrefix = tolower(diceRollTokens[0][0]);
        if (!diceRollTokens[1].empty()) {
            items[item_counter].damageDice = fromString<int>(diceRollTokens[0]);
            items[item_counter].damageRoll = fromString<int>(diceRollTokens[1]);
        } else if (damagePrefix != 's' && damagePrefix != 'm') {
            items[item_counter].damageDice = fromString<int>(diceRollTokens[0]);
            items[item_counter].damageRoll = 1;
        } else {
            items[item_counter].damageDice = 0;
            items[item_counter].damageRoll = 0;
        }
        items[item_counter].damageMod = fromString<int>(tokens[7]);
        items[item_counter].rarity = valueOr(materialMap, tokens[8], RARITY_COMMON);
        items[item_counter].identifyAndRepairDifficulty = fromString<int>(tokens[9]);
        items[item_counter].unidentifiedName = unquote(tokens[10]);
        items[item_counter].spriteId = static_cast<SpriteId>(fromString<int>(tokens[11]));

        if (items[item_counter].type == ITEM_TYPE_REAGENT) {
            items[item_counter].reagentPower = items[item_counter].damageDice;
            items[item_counter].damageDice = items[item_counter].damageRoll = items[item_counter].damageMod = 0;
        }

        items[item_counter].specialEnchantment = ITEM_ENCHANTMENT_NULL;
        items[item_counter].standardEnchantment = {};
        if (items[item_counter].rarity == RARITY_SPECIAL) {
            for (Attribute ii : allEnchantableAttributes()) {
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
            char b_s = fromString<int>(tokens[13]);
            if (b_s)
                items[item_counter].standardEnchantmentStrength = b_s;
            else
                items[item_counter].standardEnchantmentStrength = 1;
        } else {
            items[item_counter].standardEnchantmentStrength = 0;
        }
        items[item_counter].paperdollAnchorOffset.x = fromString<int>(tokens[14]);
        items[item_counter].paperdollAnchorOffset.y = fromString<int>(tokens[15]);
        items[item_counter].description = unquote(tokens[16]);
    }
}

void ItemTable::LoadRandomItems(std::string_view rnditems) {
    // rnditems.txt has two sections.
    std::vector<std::string_view> lines = split(rnditems).by("\r\n").drop(4).skip("");
    constexpr size_t section1Size = 618;

    // #1 Per-item chances: item index | id (e.g. "ring1", not used) | chance by treasure level 1-6.
    for (std::string_view line : view(lines).drop(0).resize(section1Size, "")) {
        std::array<std::string_view, 8> tokens = split(line).by('\t');
        ItemId item_counter = ItemId(fromString<int>(tokens[0]));
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_1] = fromString<int>(tokens[2]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_2] = fromString<int>(tokens[3]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_3] = fromString<int>(tokens[4]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_4] = fromString<int>(tokens[5]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_5] = fromString<int>(tokens[6]);
        items[item_counter].uChanceByTreasureLvl[ITEM_TREASURE_LEVEL_6] = fromString<int>(tokens[7]);
    }
    itemChanceSumByTreasureLevel.fill(0);
    for (ItemTreasureLevel i : itemChanceSumByTreasureLevel.indices())
        for (ItemId j : items.indices())
            itemChanceSumByTreasureLevel[i] += items[j].uChanceByTreasureLvl[i];

    // #2: Enchantment chances by level: line type (not localized, not used) | enchantment type (not localized, not used) | chance by treasure level 1-6.
    auto chancesArray = std::array{&standardEnchantmentChanceForEquipment, &specialEnchantmentChanceForEquipment, &specialEnchantmentChanceForWeapons};
    for (auto [line, chances] : view(lines).drop(section1Size + 1).zip(chancesArray)) {
        std::array<std::string_view, 8> tokens = split(line).by('\t');
        for (ItemTreasureLevel i : chances->indices())
            (*chances)[i] = fromString<int>(tokens[2 + std::to_underlying(i) - std::to_underlying(ITEM_TREASURE_LEVEL_FIRST_RANDOM)]);
    }
}

//----- (00456D84) --------------------------------------------------------
void ItemTable::Initialize(ResourceManager *resourceManager) {
    LoadPotions(resourceManager->eventsText("potion.txt"));
    LoadPotionNotes(resourceManager->eventsText("potnotes.txt"));
    LoadStandardEnchantments(resourceManager->eventsText("stditems.txt"));
    LoadSpecialEnchantments(resourceManager->eventsText("spcitems.txt"));
    LoadItems(resourceManager->eventsText("items.txt"));
    LoadRandomItems(resourceManager->eventsText("rnditems.txt"));

    Item::PopulateSpecialBonusMap();
    Item::PopulateArtifactBonusMap();
    LoadItemSizes();

    // Patch up the data - we want wetsuits to be armor.
    items[ITEM_QUEST_WETSUIT].type = ITEM_TYPE_ARMOUR;
}

//----- (00453B3C) --------------------------------------------------------
void ItemTable::LoadPotions(std::string_view potions) {
    // potion.txt table structure: item index | name (localized) | unidentified name (localized) | effect (not localized) | mixing matrix...
    // First rows are reagents, then real potions follow. Reagents don't have the mixing matrix values.
    // Matrix values: item ID of result, "no" for self-mixing, "E{n}" for damage level n on invalid mix.
    for (std::string_view line : split(potions).by("\r\n").skip("").drop(1)) {
        std::array<std::string_view, 57> tokens = split(line).by('\t'); // 7 header cells + 50 mixing-matrix cells.
        if (tokens[0].empty())
            continue; // Skip tab-only lines.

        ItemId row = static_cast<ItemId>(fromString<int>(tokens[0]));
        if (row < ITEM_FIRST_REAL_POTION || row > ITEM_LAST_REAL_POTION)
            continue; // Skip reagents, catalyst, etc.

        for (ItemId column : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
            int flatPotionId = std::to_underlying(column) - std::to_underlying(ITEM_FIRST_REAL_POTION);
            std::string_view cell = tokens[flatPotionId + 7];
            if (cell == "no")
                potionCombination[row][column] = ITEM_NULL;
            else if (cell[0] == 'E')
                potionCombination[row][column] = static_cast<ItemId>(fromString<int>(cell.substr(1))); // Damage level.
            else
                potionCombination[row][column] = static_cast<ItemId>(fromString<int>(cell));
        }
    }
}

//----- (00453CE5) --------------------------------------------------------
void ItemTable::LoadPotionNotes(std::string_view notes) {
    // potnotes.txt has the same layout as potion.txt, but the mixing matrix contains autonote bit indices
    // (for recipe discovery) instead of resulting item IDs.
    for (std::string_view line : split(notes).by("\r\n").skip("").drop(1)) {
        std::array<std::string_view, 57> tokens = split(line).by('\t'); // 7 header cells + 50 mixing-matrix cells.
        if (tokens[0].empty())
            continue; // Skip tab-only lines.

        ItemId row = static_cast<ItemId>(fromString<int>(tokens[0]));
        if (row < ITEM_FIRST_REAL_POTION || row > ITEM_LAST_REAL_POTION)
            continue; // Skip reagents, catalyst, etc.

        for (ItemId column : Segment(ITEM_FIRST_REAL_POTION, ITEM_LAST_REAL_POTION)) {
            int flatPotionId = std::to_underlying(column) - std::to_underlying(ITEM_FIRST_REAL_POTION);
            std::string_view cell = tokens[flatPotionId + 7];
            potionNotes[row][column] = cell == "no" ? 0 : fromString<int>(cell);
        }
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
        auto [requestedType, requestedSkill] = itemTypeOrSkillForRandomItemType(uTreasureType);
        for (ItemId itemId : allSpawnableItems()) {
            if ((requestedType == ITEM_TYPE_INVALID || items[itemId].type == requestedType) && (requestedSkill == SKILL_INVALID || items[itemId].skill == requestedSkill)) {
                if (items[itemId].uChanceByTreasureLvl[treasureLevel]) {
                    weightSum += items[itemId].uChanceByTreasureLvl[treasureLevel];
                    possibleItems.push_back(itemId);
                    cumulativeWeights.push_back(weightSum);
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
        // Try to generate an artifact.
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
                return;
            }
        }

        // Otherwise try to spawn any random item.
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
    if (pItemTable->items[outItem->itemId].identifyAndRepairDifficulty)
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
                for (Attribute attr : allEnchantableAttributes()) {
                    if (currentEnchantmentChancesSum >= enchantmentChanceSumRoll)
                        break;

                    currentEnchantmentChancesSum += standardEnchantments[attr].chanceByItemType[outItem->type()];
                    outItem->standardEnchantment = attr;
                }
                assert(outItem->standardEnchantment);

                outItem->standardEnchantmentStrength = grng->randomSample(standardEnchantmentRangeByTreasureLevel[treasureLevel]);
                Attribute standardEnchantmentAttributeSkill = *outItem->standardEnchantment;
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
        int tr_lv = specialEnchantments[ench].enchantmentLevel;

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
