#include "Item.h"

#include <map>
#include <string>
#include <unordered_map>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/ItemEnchantment.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"

#include "GUI/GUIButton.h"

#include "Utility/MapAccess.h"

Item *ptr_50C9A4_ItemToEnchant;

ItemTable *pItemTable;  // 005D29E0

static std::map<int, std::map<CharacterAttribute, CEnchantment>> regularBonusMap;
static std::map<ItemEnchantment, std::map<CharacterAttribute, CEnchantment>> specialBonusMap;
static std::map<ItemId, std::map<CharacterAttribute, CEnchantment>> artifactBonusMap;

static std::unordered_map<ItemId, ItemId> itemTextureIdByItemId = {
    { ITEM_RELIC_HARECKS_LEATHER,       ITEM_POTION_STONESKIN },
    { ITEM_ARTIFACT_YORUBA,             ITEM_POTION_HARDEN_ITEM },
    { ITEM_ARTIFACT_GOVERNORS_ARMOR,    ITEM_POTION_WATER_BREATHING },
    { ITEM_ARTIFACT_ELVEN_CHAINMAIL,    ITEM_FINE_CHAIN_MAIL },
    { ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, ITEM_SCROLL_FEATHER_FALL },
    { ITEM_RELIC_TALEDONS_HELM,         ITEM_POTION_CURE_INSANITY },
    { ITEM_RELIC_SCHOLARS_CAP,          ITEM_POTION_MIGHT_BOOST },
    { ITEM_RELIC_PHYNAXIAN_CROWN,       ITEM_POTION_INTELLECT_BOOST },
    { ITEM_ARTIFACT_MINDS_EYE,          ITEM_MOGRED_HELM },
    { ITEM_SPECIAL_SHADOWS_MASK,        ITEM_SCROLL_DETECT_LIFE },
    { ITEM_RELIC_TITANS_BELT,           ITEM_SCROLL_WATER_RESISTANCE },
    { ITEM_ARTIFACT_HEROS_BELT,         ITEM_GILDED_BELT },
    { ITEM_RELIC_TWILIGHT,              ITEM_SCROLL_ICE_BOLT },
    { ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ITEM_SCROLL_TOWN_PORTAL },
    { ITEM_SPECIAL_SUN_CLOAK,           ITEM_SCROLL_TURN_UNDEAD },
    { ITEM_SPECIAL_MOON_CLOAK,          ITEM_SCROLL_REMOVE_CURSE },
    { ITEM_SPECIAL_VAMPIRES_CAPE,       ITEM_SCROLL_HEROISM }
};

//----- (00439DF3) --------------------------------------------------------
int Item::_439DF3_get_additional_damage(DamageType *damage_type,
                                           bool *draintargetHP) {
    *draintargetHP = false;
    *damage_type = DAMAGE_FIRE;
    if (itemId == ITEM_NULL) return 0;

    UpdateTempBonus(pParty->GetPlayingTime());
    if (itemId == ITEM_ARTIFACT_IRON_FEATHER) {
        *damage_type = DAMAGE_AIR;
        return grng->random(10) + 6;
    }
    if (itemId == ITEM_ARTIFACT_GHOULSBANE) {
        *damage_type = DAMAGE_FIRE;
        return grng->random(16) + 3;
    }
    if (itemId == ITEM_ARTIFACT_ULLYSES) {
        *damage_type = DAMAGE_WATER;
        return grng->random(4) + 9;
    }
    if (itemId == ITEM_RELIC_OLD_NICK) {
        *damage_type = DAMAGE_BODY;
        return 8;
    }

    switch (specialEnchantment) {
        case ITEM_ENCHANTMENT_OF_COLD:
            *damage_type = DAMAGE_WATER;
            return grng->random(2) + 3;
            break;
        case ITEM_ENCHANTMENT_OF_FROST:
            *damage_type = DAMAGE_WATER;
            return grng->random(3) + 6;
            break;
        case ITEM_ENCHANTMENT_OF_ICE:
            *damage_type = DAMAGE_WATER;
            return grng->random(4) + 9;
            break;
        case ITEM_ENCHANTMENT_OF_SPARKS:
            *damage_type = DAMAGE_AIR;
            return grng->random(4) + 2;
            break;
        case ITEM_ENCHANTMENT_OF_LIGHTNING:
            *damage_type = DAMAGE_AIR;
            return grng->random(7) + 4;
            break;
        case ITEM_ENCHANTMENT_OF_THUNDERBOLTS:
            *damage_type = DAMAGE_AIR;
            return grng->random(10) + 6;
            break;
        case ITEM_ENCHANTMENT_OF_FIRE:
            *damage_type = DAMAGE_FIRE;
            return grng->randomDice(1, 6);
            break;
        case ITEM_ENCHANTMENT_OF_FLAME:
            *damage_type = DAMAGE_FIRE;
            return grng->randomDice(2, 6);
            break;
        case ITEM_ENCHANTMENT_OF_INFERNOS:
            *damage_type = DAMAGE_FIRE;
            return grng->randomDice(3, 6);
            break;
        case ITEM_ENCHANTMENT_OF_POISON:
            *damage_type = DAMAGE_BODY;
            return 5;
            break;
        case ITEM_ENCHANTMENT_OF_VENOM:
            *damage_type = DAMAGE_BODY;
            return 8;
            break;
        case ITEM_ENCHANTMENT_OF_ACID:
            *damage_type = DAMAGE_BODY;
            return 12;
            break;
        case ITEM_ENCHANTMENT_VAMPIRIC:
        case ITEM_ENCHANTMENT_OF_DARKNESS:
            *damage_type = DAMAGE_DARK;
            *draintargetHP = true;
            return 0;
            break;

        case ITEM_ENCHANTMENT_OF_DRAGON:
            *damage_type = DAMAGE_FIRE;
            return grng->random(11) + 10;
            break;
        default:
            *damage_type = DAMAGE_FIRE;
            return 0;
    }
}

//----- (00402F07) --------------------------------------------------------
void Item::Reset() {
    *this = Item();
}

//----- (00458260) --------------------------------------------------------
void Item::UpdateTempBonus(Time time) {
    if (this->flags & ITEM_TEMP_BONUS) {
        if (time > this->enchantmentExpirationTime) {
            this->standardEnchantment = {};
            this->specialEnchantment = ITEM_ENCHANTMENT_NULL;
            this->flags &= ~ITEM_TEMP_BONUS;
        }
    }
}

//----- (00456442) --------------------------------------------------------
int Item::GetValue() const {
    int uBaseValue = pItemTable->items[this->itemId].baseValue;
    if (flags & ITEM_TEMP_BONUS || rarity() != RARITY_COMMON)
        return uBaseValue;
    if (potionPower || standardEnchantment) // TODO(captainurist): can drop potionPower?
        return uBaseValue + 100 * standardEnchantmentStrength;

    if (specialEnchantment != ITEM_ENCHANTMENT_NULL) {
        int mod = (pItemTable->specialEnchantments[specialEnchantment].iTreasureLevel & 4);
        int bonus = pItemTable->specialEnchantments[specialEnchantment].additionalValue;
        if (!mod)
            return uBaseValue + bonus;
        else
            return uBaseValue * bonus;
    }
    return uBaseValue;
}

//----- (00456499) --------------------------------------------------------
std::string Item::GetDisplayName() const {
    if (IsIdentified()) {
        return GetIdentifiedName();
    } else {
        return pItemTable->items[itemId].unidentifiedName;
    }
}

//----- (004564B3) --------------------------------------------------------
std::string Item::GetIdentifiedName() const {
    ItemType equip_type = type();
    if ((equip_type == ITEM_TYPE_REAGENT) || (equip_type == ITEM_TYPE_POTION) ||
        (equip_type == ITEM_TYPE_GOLD)) {
        return pItemTable->items[itemId].name;
    }

    if (itemId == ITEM_QUEST_LICH_JAR_FULL) {  // Lich Jar
        if (lichJarCharacterIndex >= 0 && lichJarCharacterIndex < pParty->pCharacters.size()) {
            const std::string &player_name = pParty->pCharacters[lichJarCharacterIndex].name;
            if (player_name.back() == 's')
                return localization->FormatString(LSTR_S_JAR, player_name);
            else
                return localization->FormatString(LSTR_SS_JAR, player_name);
        }
    }

    if (rarity() == RARITY_COMMON) {
        if (standardEnchantment) {
            return std::string(pItemTable->items[itemId].name) + " " +
                   pItemTable->standardEnchantments[*standardEnchantment].itemSuffix;
        } else if (specialEnchantment == ITEM_ENCHANTMENT_NULL) {
            return pItemTable->items[itemId].name;
        } else {
            if (specialEnchantment == ITEM_ENCHANTMENT_VAMPIRIC
                || specialEnchantment == ITEM_ENCHANTMENT_DEMON_SLAYING
                || specialEnchantment == ITEM_ENCHANTMENT_DRAGON_SLAYING
                || specialEnchantment == ITEM_ENCHANTMENT_ROGUES
                || specialEnchantment == ITEM_ENCHANTMENT_WARRIORS
                || specialEnchantment == ITEM_ENCHANTMENT_WIZARDS
                || specialEnchantment == ITEM_ENCHANTMENT_ANTIQUE
                || specialEnchantment == ITEM_ENCHANTMENT_MONKS
                || specialEnchantment == ITEM_ENCHANTMENT_THIEVES
                || specialEnchantment == ITEM_ENCHANTMENT_SWIFT
                || specialEnchantment == ITEM_ENCHANTMENT_ELF_SLAYING
                || specialEnchantment == ITEM_ENCHANTMENT_UNDEAD_SLAYING
                || specialEnchantment == ITEM_ENCHANTMENT_ASSASINS
                || specialEnchantment == ITEM_ENCHANTMENT_BARBARIANS
            ) {            // enchantment and name positions inverted!
                return fmt::format(
                    "{} {}",
                    pItemTable->specialEnchantments[specialEnchantment].itemSuffixOrPrefix,
                    pItemTable->items[itemId].name);
            } else {
                return fmt::format(
                    "{} {}",
                    pItemTable->items[itemId].name,
                    pItemTable->specialEnchantments[specialEnchantment].itemSuffixOrPrefix);
            }
        }
    }

    return pItemTable->items[itemId].name;
}

//----- (004505CC) --------------------------------------------------------
bool Item::GenerateArtifact() {
    signed int uNumArtifactsNotFound;  // esi@1
    std::array<ItemId, 32> artifacts_list;

    artifacts_list.fill(ITEM_NULL);
    uNumArtifactsNotFound = 0;

    for (ItemId i : allSpawnableArtifacts())
        if (!pParty->pIsArtifactFound[i])
            artifacts_list[uNumArtifactsNotFound++] = i;

    Reset();
    if (uNumArtifactsNotFound) {
        itemId = artifacts_list[grng->random(uNumArtifactsNotFound)];
        postGenerate(ITEM_SOURCE_UNKNOWN);
        return true;
    } else {
        return false;
    }
}

void Item::generateGold(ItemTreasureLevel treasureLevel) {
    assert(isRandomTreasureLevel(treasureLevel));

    Reset();
    SetIdentified();

    switch (treasureLevel) {
    case ITEM_TREASURE_LEVEL_1:
        goldAmount = grng->random(51) + 50;
        itemId = ITEM_GOLD_SMALL;
        break;
    case ITEM_TREASURE_LEVEL_2:
        goldAmount = grng->random(101) + 100;
        itemId = ITEM_GOLD_SMALL;
        break;
    case ITEM_TREASURE_LEVEL_3:
        goldAmount = grng->random(301) + 200;
        itemId = ITEM_GOLD_MEDIUM;
        break;
    case ITEM_TREASURE_LEVEL_4:
        goldAmount = grng->random(501) + 500;
        itemId = ITEM_GOLD_MEDIUM;
        break;
    case ITEM_TREASURE_LEVEL_5:
        goldAmount = grng->random(1001) + 1000;
        itemId = ITEM_GOLD_LARGE;
        break;
    case ITEM_TREASURE_LEVEL_6:
        goldAmount = grng->random(3001) + 2000;
        itemId = ITEM_GOLD_LARGE;
        break;
    default:
        assert(false);
        break;
    }
}

template<class Key, class ActualKey>
static void AddToMap(std::map<Key, std::map<CharacterAttribute, CEnchantment>> &map,
                     ActualKey key, CharacterAttribute subkey, int bonusValue = 0, CharacterSkillType skill = CHARACTER_SKILL_INVALID) {
    auto &submap = map[key];

    assert(!submap.contains(subkey));

    submap[subkey] = CEnchantment(bonusValue, skill);
}

void Item::PopulateSpecialBonusMap() {
    // of Protection, +10 to all Resistances (description in txt says all 4, need to verify!)
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_AIR, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_BODY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_EARTH, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_FIRE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_MIND, 10);
    //AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_SPIRIT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PROTECTION, ATTRIBUTE_RESIST_WATER, 10);

    // of The Gods, +10 to all Seven Statistics
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_ACCURACY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_ENDURANCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_LUCK, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_SPEED, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_MIGHT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, ATTRIBUTE_PERSONALITY, 10);

    // of Air Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_AIR_MAGIC, ATTRIBUTE_SKILL_AIR, 0, CHARACTER_SKILL_AIR);

    // of Body Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_BODY_MAGIC, ATTRIBUTE_SKILL_BODY, 0, CHARACTER_SKILL_BODY);

    // of Dark Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DARK_MAGIC, ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);

    // of Earth Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH_MAGIC, ATTRIBUTE_SKILL_EARTH, 0, CHARACTER_SKILL_EARTH);

    // of Fire Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_FIRE_MAGIC, ATTRIBUTE_SKILL_FIRE, 0, CHARACTER_SKILL_FIRE);

    // of Light Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_LIGHT_MAGIC, ATTRIBUTE_SKILL_LIGHT, 0, CHARACTER_SKILL_LIGHT);

    // of Mind Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MIND_MAGIC, ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);

    // of Spirit Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SPIRIT_MAGIC, ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);

    // of Water Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_WATER_MAGIC, ATTRIBUTE_SKILL_WATER, 0, CHARACTER_SKILL_WATER);

    // of Doom, +1 to Seven Stats, HP, SP, Armor, Resistances (in txt it says 4, need to check!)
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_ACCURACY, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_INTELLIGENCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_ENDURANCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_LUCK, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_SPEED, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_MIGHT, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_PERSONALITY, 1);

    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_AIR, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_BODY, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_EARTH, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_FIRE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_MIND, 1);
    //AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_SPIRIT, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_RESIST_WATER, 1);

    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_AC_BONUS, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_HEALTH, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, ATTRIBUTE_MANA, 1);

    // of Earth
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, ATTRIBUTE_AC_BONUS, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, ATTRIBUTE_ENDURANCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH, ATTRIBUTE_HEALTH, 10);

    // of Life
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_LIFE, ATTRIBUTE_HEALTH, 10);

    // Rogues
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ROGUES, ATTRIBUTE_ACCURACY, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ROGUES, ATTRIBUTE_SPEED, 5);

    // of The Dragon
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DRAGON, ATTRIBUTE_MIGHT, 25);

    // of The Eclipse
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_ECLIPSE, ATTRIBUTE_MANA, 10);

    // of The Golem
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GOLEM, ATTRIBUTE_AC_BONUS, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GOLEM, ATTRIBUTE_ENDURANCE, 15);

    // of The Moon
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MOON, ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MOON, ATTRIBUTE_LUCK, 10);

    // of The Phoenix
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_PHOENIX, ATTRIBUTE_RESIST_FIRE, 30);

    // of The Sky
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, ATTRIBUTE_MANA, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SKY, ATTRIBUTE_SPEED, 10);

    // of The Stars
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STARS, ATTRIBUTE_ACCURACY, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STARS, ATTRIBUTE_ENDURANCE, 10);

    // of The Sun
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, ATTRIBUTE_MIGHT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, ATTRIBUTE_PERSONALITY, 10);

    // of The Troll
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_TROLL, ATTRIBUTE_ENDURANCE, 15);

    // of The Unicorn
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_UNICORN, ATTRIBUTE_LUCK, 15);

    // Warriors
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, ATTRIBUTE_ENDURANCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, ATTRIBUTE_MIGHT, 5);

    // Wizards
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, ATTRIBUTE_INTELLIGENCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, ATTRIBUTE_PERSONALITY, 5);

    // Monks
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, ATTRIBUTE_SKILL_DODGE, 3, CHARACTER_SKILL_DODGE);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, ATTRIBUTE_SKILL_UNARMED, 3, CHARACTER_SKILL_UNARMED);

    // Thieves
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, ATTRIBUTE_SKILL_STEALING, 3, CHARACTER_SKILL_STEALING);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, ATTRIBUTE_SKILL_TRAP_DISARM, 3, CHARACTER_SKILL_TRAP_DISARM);

    // of Identifying
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, ATTRIBUTE_SKILL_ITEM_ID, 3, CHARACTER_SKILL_ITEM_ID);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, ATTRIBUTE_SKILL_MONSTER_ID, 3, CHARACTER_SKILL_MONSTER_ID);

    // Assassins
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ASSASINS, ATTRIBUTE_SKILL_TRAP_DISARM, 2, CHARACTER_SKILL_TRAP_DISARM);

    // Barbarians
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_BARBARIANS, ATTRIBUTE_AC_BONUS, 5);

    // of the Storm
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STORM, ATTRIBUTE_RESIST_AIR, 20);

    // of the Ocean
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, ATTRIBUTE_SKILL_ALCHEMY, 2, CHARACTER_SKILL_ALCHEMY);
}

// TODO: where is it used?
void Item::PopulateRegularBonusMap() {
    // of Might
    AddToMap(regularBonusMap, 1, ATTRIBUTE_MIGHT);

    // of Thought
    AddToMap(regularBonusMap, 2, ATTRIBUTE_INTELLIGENCE);

    // of Charm
    AddToMap(regularBonusMap, 3, ATTRIBUTE_PERSONALITY);

    // of Vigor
    AddToMap(regularBonusMap, 4, ATTRIBUTE_ENDURANCE);

    // of Precision
    AddToMap(regularBonusMap, 5, ATTRIBUTE_ACCURACY);

    // of Speed
    AddToMap(regularBonusMap, 6, ATTRIBUTE_SPEED);

    // of Luck
    AddToMap(regularBonusMap, 7, ATTRIBUTE_LUCK);

    // of Health
    AddToMap(regularBonusMap, 8, ATTRIBUTE_HEALTH);

    // of Magic
    AddToMap(regularBonusMap, 9, ATTRIBUTE_MANA);

    // of Defense
    AddToMap(regularBonusMap, 10, ATTRIBUTE_AC_BONUS);

    // of Fire Resistance
    AddToMap(regularBonusMap, 11, ATTRIBUTE_RESIST_FIRE);

    // of Air Resistance
    AddToMap(regularBonusMap, 12, ATTRIBUTE_RESIST_AIR);

    // of Water Resistance
    AddToMap(regularBonusMap, 13, ATTRIBUTE_RESIST_WATER);

    // of Earth Resistance
    AddToMap(regularBonusMap, 14, ATTRIBUTE_RESIST_EARTH);

    // of Mind Resistance
    AddToMap(regularBonusMap, 15, ATTRIBUTE_RESIST_MIND);

    // of Body Resistance
    AddToMap(regularBonusMap, 16, ATTRIBUTE_RESIST_BODY);

    // of Alchemy
    AddToMap(regularBonusMap, 17, ATTRIBUTE_SKILL_ALCHEMY);

    // of Stealing
    AddToMap(regularBonusMap, 18, ATTRIBUTE_SKILL_STEALING);

    // of Disarming
    AddToMap(regularBonusMap, 19, ATTRIBUTE_SKILL_TRAP_DISARM);

    // of Items
    AddToMap(regularBonusMap, 20, ATTRIBUTE_SKILL_ITEM_ID);

    // of Monsters
    AddToMap(regularBonusMap, 21, ATTRIBUTE_SKILL_MONSTER_ID);

    // of Arms
    AddToMap(regularBonusMap, 22, ATTRIBUTE_SKILL_ARMSMASTER);

    // of Dodging
    AddToMap(regularBonusMap, 23, ATTRIBUTE_SKILL_DODGE);

    // of the Fist
    AddToMap(regularBonusMap, 24, ATTRIBUTE_SKILL_UNARMED);
}

void Item::PopulateArtifactBonusMap() {
    // Puck
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_PUCK, ATTRIBUTE_SPEED, 40);

    // Iron Feather
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_IRON_FEATHER, ATTRIBUTE_MIGHT, 40);

    // Wallace
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, ATTRIBUTE_SKILL_ARMSMASTER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, ATTRIBUTE_PERSONALITY, 40);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, ATTRIBUTE_LUCK, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, ATTRIBUTE_SKILL_STEALING, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_MIGHT, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_PERSONALITY, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_ENDURANCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_ACCURACY, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_SPEED, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, ATTRIBUTE_LUCK, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_YORUBA, ATTRIBUTE_ENDURANCE, 25);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SPLITTER, ATTRIBUTE_RESIST_FIRE, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ULLYSES, ATTRIBUTE_ACCURACY, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, ATTRIBUTE_SKILL_DODGE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, ATTRIBUTE_SKILL_UNARMED, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, ATTRIBUTE_SPEED, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, ATTRIBUTE_SKILL_WATER, 0, CHARACTER_SKILL_WATER);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);

    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, ATTRIBUTE_MIGHT, 150);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, ATTRIBUTE_INTELLIGENCE, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, ATTRIBUTE_PERSONALITY, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);
    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, ATTRIBUTE_SKILL_MEDITATION, 15);

    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_SKILL_STEALING, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_LUCK, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_FIRE, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_WATER, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_AIR, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_EARTH, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_OLD_NICK, ATTRIBUTE_SKILL_TRAP_DISARM, 5);

    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, ATTRIBUTE_MIGHT, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, ATTRIBUTE_ENDURANCE, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, ATTRIBUTE_AC_BONUS, -15);

    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, ATTRIBUTE_SKILL_SHIELD, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, ATTRIBUTE_ENDURANCE, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, ATTRIBUTE_RESIST_EARTH, -30);

    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, ATTRIBUTE_SKILL_LIGHT, 0, CHARACTER_SKILL_LIGHT);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, ATTRIBUTE_PERSONALITY, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, ATTRIBUTE_LUCK, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, ATTRIBUTE_SKILL_LEARNING, +15);
    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, ATTRIBUTE_ENDURANCE, -50);

    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, ATTRIBUTE_SKILL_FIRE, 0, CHARACTER_SKILL_FIRE);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, ATTRIBUTE_RESIST_WATER, +50);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, ATTRIBUTE_PERSONALITY, 30);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, ATTRIBUTE_AC_BONUS, -20);

    AddToMap(artifactBonusMap, ITEM_RELIC_TITANS_BELT, ATTRIBUTE_MIGHT, 75);
    AddToMap(artifactBonusMap, ITEM_RELIC_TITANS_BELT, ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_SPEED, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_LUCK, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_FIRE, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_WATER, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_AIR, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_EARTH, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_MIND, -15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TWILIGHT, ATTRIBUTE_RESIST_BODY, -15);

    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, ATTRIBUTE_ACCURACY, 150);
    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, ATTRIBUTE_SKILL_BOW, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_ANIA_SELVING, ATTRIBUTE_AC_BONUS, -25);

    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, ATTRIBUTE_SKILL_BODY, 0, CHARACTER_SKILL_BODY);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, ATTRIBUTE_MIGHT, 75);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, ATTRIBUTE_RESIST_AIR, -50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, ATTRIBUTE_SPEED, 100);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, ATTRIBUTE_ACCURACY, 50);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, ATTRIBUTE_RESIST_AIR, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ATTRIBUTE_PERSONALITY, -20);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, ATTRIBUTE_INTELLIGENCE, -20);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, ATTRIBUTE_PERSONALITY, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, ATTRIBUTE_INTELLIGENCE, 15);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ELVEN_CHAINMAIL, ATTRIBUTE_SPEED, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ELVEN_CHAINMAIL, ATTRIBUTE_ACCURACY, 15);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, ATTRIBUTE_ENDURANCE, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, ATTRIBUTE_RESIST_FIRE, 30);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, ATTRIBUTE_SKILL_ARMSMASTER, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_FIRE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_AIR, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_EARTH, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_MIND, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, ATTRIBUTE_RESIST_BODY, 10);
}

void Item::GetItemBonusSpecialEnchantment(const Character *owner,
                                             CharacterAttribute attrToGet,
                                             int *additiveBonus,
                                             int *halfSkillBonus) const {
    auto pos = specialBonusMap.find(this->specialEnchantment);
    if (pos == specialBonusMap.end())
        return;

    auto subpos = pos->second.find(attrToGet);
    if (subpos == pos->second.end())
        return;

    const CEnchantment &currBonus = subpos->second;
    if (currBonus.skillType != CHARACTER_SKILL_INVALID) {
        if (currBonus.statBonus == 0) {
            *halfSkillBonus = owner->pActiveSkills[currBonus.skillType].level() / 2;
        } else {
            if (*additiveBonus < currBonus.statBonus) {
                *additiveBonus = currBonus.statBonus;
            }
        }
    } else {
        *additiveBonus += currBonus.statBonus;
    }
}

void Item::GetItemBonusArtifact(const Character *owner,
                                   CharacterAttribute attrToGet,
                                   int *bonusSum) const {
    auto pos = artifactBonusMap.find(this->itemId);
    if (pos == artifactBonusMap.end())
        return;

    auto subpos = pos->second.find(attrToGet);
    if (subpos == pos->second.end())
        return;

    const CEnchantment &currBonus = subpos->second;
    if (currBonus.skillType != CHARACTER_SKILL_INVALID) {
        *bonusSum = owner->pActiveSkills[currBonus.skillType].level() / 2;
    } else {
        *bonusSum += currBonus.statBonus;
    }
}

bool Item::IsRegularEnchanmentForAttribute(CharacterAttribute attrToGet) {
    //auto pos = specialBonusMap.find(this->standardEnchantment);
    //if (pos == specialBonusMap.end())
    //    return false;

    //return pos->second.find(attrToGet) != pos->second.end();
    // TODO(captainurist): what is this code about? ^
    return false;
}

CharacterSkillType Item::GetPlayerSkillType() const {
    CharacterSkillType skl = pItemTable->items[this->itemId].skill;
    if (skl == CHARACTER_SKILL_CLUB && engine->config->gameplay.TreatClubAsMace.value()) {
        // club skill not used but some items load it
        skl = CHARACTER_SKILL_MACE;
    }
    return skl;
}

const std::string& Item::GetIconName() const {
    return pItemTable->items[this->itemId].iconName;
}

uint8_t Item::GetDamageDice() const {
    return pItemTable->items[this->itemId].damageDice;
}

uint8_t Item::GetDamageRoll() const {
    return pItemTable->items[this->itemId].damageRoll;
}

uint8_t Item::GetDamageMod() const {
    return pItemTable->items[this->itemId].damageMod;
}

//----- (0043C91D) --------------------------------------------------------
std::string GetItemTextureFilename(ItemId item_id, int index, int shoulder) {
    // For some reason artifact textures are stored using different ids,
    // and textures under original ids simply don't exist.
    int texture_id = std::to_underlying(valueOr(itemTextureIdByItemId, item_id, item_id));

    switch (pItemTable->items[item_id].type) {
        case ITEM_TYPE_ARMOUR:
            if (shoulder == 0)
                return fmt::format("item{:03}v{}", texture_id, index);
            else if (shoulder == 1)
                return fmt::format("item{:03}v{}a1", texture_id, index);
            else // shoulder == 2
                return fmt::format("item{:03}v{}a2", texture_id, index);
        case ITEM_TYPE_CLOAK:
            if (shoulder == 0)
                return fmt::format("item{:03}v{}", texture_id, index);
            else // shoulder == 1
                return fmt::format("item{:03}v{}a1", texture_id, index);
        default:
            return fmt::format("item{:03}v{}", texture_id, index);
    }
}

//----- (004BDAAF) --------------------------------------------------------
bool Item::canSellRepairIdentifyAt(HouseId houseId) {
    if (this->IsStolen())
        return false;

    if (isQuestItem(itemId))
        return false; // Can't sell quest items.

    if (isArtifact(itemId) && !isSpawnableArtifact(itemId))
        return false; // Can't sell quest artifacts, e.g. Hermes Sandals.

    if (::isMessageScroll(itemId) && !isRecipe(itemId))
        return false; // Can't sell message scrolls. Recipes are sellable at alchemy shops.

    switch (houseTable[houseId].uType) {
        case HOUSE_TYPE_WEAPON_SHOP:
            return this->isWeapon();
        case HOUSE_TYPE_ARMOR_SHOP:
            return this->isArmor();
        case HOUSE_TYPE_MAGIC_SHOP:
            return (this->GetPlayerSkillType() == CHARACTER_SKILL_MISC && !isRecipe(this->itemId)) || this->isBook();
        case HOUSE_TYPE_ALCHEMY_SHOP:
            return this->isReagent() ||
                   this->isPotion() ||
                   (this->isMessageScroll() && isRecipe(this->itemId));
        default:
            return false;
    }
}

ItemType Item::type() const {
    return itemId == ITEM_NULL ? ITEM_TYPE_NONE : pItemTable->items[itemId].type;
}

ItemRarity Item::rarity() const {
    return itemId == ITEM_NULL ? RARITY_COMMON : pItemTable->items[itemId].rarity;
}

Sizei Item::inventorySize() const {
    return itemId == ITEM_NULL ? Sizei() : pItemTable->itemSizes[itemId];
}

void Item::postGenerate(ItemSource source) {
    if (itemId == ITEM_NULL)
        return;

    if (rarity() == RARITY_SPECIAL) {
        standardEnchantment = pItemTable->items[itemId].standardEnchantment;
        specialEnchantment = pItemTable->items[itemId].specialEnchantment;
        standardEnchantmentStrength = pItemTable->items[itemId].standardEnchantmentStrength;
    }

    if (type() == ITEM_TYPE_POTION && itemId != ITEM_POTION_BOTTLE && potionPower == 0) {
        if (source == ITEM_SOURCE_MAP || source == ITEM_SOURCE_CHEST) {
            potionPower = grng->random(15) + 5;
        } else if (source == ITEM_SOURCE_MONSTER) {
            potionPower = 2 * grng->random(4) + 2; // TODO(captainurist): change to 2d4+2.
        }

        assert(potionPower > 0);
    }

    if (type() == ITEM_TYPE_WAND && maxCharges == 0) {
        if (source == ITEM_SOURCE_MONSTER || source == ITEM_SOURCE_SCRIPT || source == ITEM_SOURCE_MAP) {
            numCharges = maxCharges = grng->random(6) + GetDamageMod() + 1;
        } else if (source == ITEM_SOURCE_CHEST) {
            numCharges = maxCharges = grng->random(21) + 10;
        }

        assert(maxCharges > 0);
    }
}

Segment<ItemTreasureLevel> RemapTreasureLevel(ItemTreasureLevel itemTreasureLevel, MapTreasureLevel mapTreasureLevel) {
    // Mapping [item_level][map_level] -> [actual_level_min, actual_level_max];
    // Rows are item treasure levels, columns are map treasure levels. Not using IndexedArray to keep things terse.
    // Original offset was 0x004E8168.
    static constexpr std::array<std::array<Segment<int>, 7>, 7> mapping = {{
        {{{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}}},
        {{{1, 1}, {1, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}}},
        {{{1, 2}, {2, 2}, {2, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}}},
        {{{2, 2}, {2, 2}, {3, 3}, {3, 4}, {4, 4}, {4, 4}, {4, 4}}},
        {{{2, 2}, {2, 2}, {3, 4}, {4, 4}, {4, 5}, {5, 5}, {5, 5}}},
        {{{2, 2}, {2, 2}, {4, 4}, {4, 5}, {5, 5}, {5, 6}, {6, 6}}},
        {{{2, 2}, {2, 2}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}}}
    }};
    static_assert(std::to_underlying(ITEM_TREASURE_LEVEL_1) == 1);
    static_assert(std::to_underlying(ITEM_TREASURE_LEVEL_7) == 7); // Otherwise static_casts at the end of this function won't work.

    assert(itemTreasureLevel >= ITEM_TREASURE_LEVEL_1 && ITEM_TREASURE_LEVEL_1 <= ITEM_TREASURE_LEVEL_7);
    assert(mapTreasureLevel >= MAP_TREASURE_LEVEL_1 && mapTreasureLevel <= MAP_TREASURE_LEVEL_7);

    int itemIdx = std::to_underlying(itemTreasureLevel) - std::to_underlying(ITEM_TREASURE_LEVEL_1);
    int mapIdx = std::to_underlying(mapTreasureLevel) - std::to_underlying(MAP_TREASURE_LEVEL_1);
    Segment<int> result = mapping[itemIdx][mapIdx];
    return {static_cast<ItemTreasureLevel>(result.front()), static_cast<ItemTreasureLevel>(result.back())};
}
