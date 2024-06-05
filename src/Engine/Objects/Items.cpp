#include "Engine/Objects/Items.h"

#include <map>
#include <string>
#include <unordered_map>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/ItemEnchantment.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"

#include "GUI/GUIButton.h"

#include "Utility/MapAccess.h"

ItemGen *ptr_50C9A4_ItemToEnchant;

ItemTable *pItemTable;  // 005D29E0

static std::map<int, std::map<CharacterAttributeType, CEnchantment>> regularBonusMap;
static std::map<ItemEnchantment, std::map<CharacterAttributeType, CEnchantment>> specialBonusMap;
static std::map<ItemId, std::map<CharacterAttributeType, CEnchantment>> artifactBonusMap;

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
int ItemGen::_439DF3_get_additional_damage(DamageType *damage_type,
                                           bool *draintargetHP) {
    *draintargetHP = false;
    *damage_type = DAMAGE_FIRE;
    if (uItemID == ITEM_NULL) return 0;

    UpdateTempBonus(pParty->GetPlayingTime());
    if (uItemID == ITEM_ARTIFACT_IRON_FEATHER) {
        *damage_type = DAMAGE_AIR;
        return grng->random(10) + 6;
    }
    if (uItemID == ITEM_ARTIFACT_GHOULSBANE) {
        *damage_type = DAMAGE_FIRE;
        return grng->random(16) + 3;
    }
    if (uItemID == ITEM_ARTIFACT_ULLYSES) {
        *damage_type = DAMAGE_WATER;
        return grng->random(4) + 9;
    }
    if (uItemID == ITEM_RELIC_OLD_NICK) {
        *damage_type = DAMAGE_BODY;
        return 8;
    }

    switch (special_enchantment) {
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
void ItemGen::Reset() {
    *this = ItemGen();
}

//----- (00458260) --------------------------------------------------------
void ItemGen::UpdateTempBonus(Time time) {
    if (this->uAttributes & ITEM_TEMP_BONUS) {
        if (time > this->uExpireTime) {
            this->attributeEnchantment = {};
            this->special_enchantment = ITEM_ENCHANTMENT_NULL;
            this->uAttributes &= ~ITEM_TEMP_BONUS;
        }
    }
}

//----- (00456442) --------------------------------------------------------
int ItemGen::GetValue() const {
    int uBaseValue = pItemTable->pItems[this->uItemID].uValue;
    if (uAttributes & ITEM_TEMP_BONUS || pItemTable->IsMaterialNonCommon(this))
        return uBaseValue;
    if (potionPower || attributeEnchantment) // TODO(captainurist): can drop potionPower?
        return uBaseValue + 100 * m_enchantmentStrength;

    if (special_enchantment != ITEM_ENCHANTMENT_NULL) {
        int mod = (pItemTable->pSpecialEnchantments[special_enchantment].iTreasureLevel & 4);
        int bonus = pItemTable->pSpecialEnchantments[special_enchantment].iValue;
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
        return pItemTable->pItems[uItemID].pUnidentifiedName;
    }
}

//----- (004564B3) --------------------------------------------------------
std::string ItemGen::GetIdentifiedName() {
    ItemType equip_type = GetItemEquipType();
    if ((equip_type == ITEM_TYPE_REAGENT) || (equip_type == ITEM_TYPE_POTION) ||
        (equip_type == ITEM_TYPE_GOLD)) {
        return pItemTable->pItems[uItemID].name;
    }

    if (uItemID == ITEM_QUEST_LICH_JAR_FULL) {  // Lich Jar
        if (uHolderPlayer >= 0 && uHolderPlayer < pParty->pCharacters.size()) {
            const std::string &player_name = pParty->pCharacters[uHolderPlayer].name;
            if (player_name.back() == 's')
                return localization->FormatString(LSTR_FMT_JAR_2, player_name);
            else
                return localization->FormatString(LSTR_FMT_JAR, player_name);
        }
    }

    if (!pItemTable->IsMaterialNonCommon(this)) {
        if (attributeEnchantment) {
            return std::string(pItemTable->pItems[uItemID].name) + " " +
                   pItemTable->standardEnchantments[*attributeEnchantment].pOfName;
        } else if (special_enchantment == ITEM_ENCHANTMENT_NULL) {
            return pItemTable->pItems[uItemID].name;
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
                return fmt::format(
                    "{} {}",
                    pItemTable->pSpecialEnchantments[special_enchantment].pNameAdd,
                    pItemTable->pItems[uItemID].name
                );
            } else {
                return std::string(pItemTable->pItems[uItemID].name) + " " +
                       pItemTable->pSpecialEnchantments[special_enchantment].pNameAdd;
            }
        }
    }

    return pItemTable->pItems[uItemID].name;
}

//----- (004505CC) --------------------------------------------------------
bool ItemGen::GenerateArtifact() {
    signed int uNumArtifactsNotFound;  // esi@1
    std::array<ItemId, 32> artifacts_list;

    artifacts_list.fill(ITEM_NULL);
    uNumArtifactsNotFound = 0;

    for (ItemId i : allSpawnableArtifacts())
        if (!pParty->pIsArtifactFound[i])
            artifacts_list[uNumArtifactsNotFound++] = i;

    Reset();
    if (uNumArtifactsNotFound) {
        uItemID = artifacts_list[grng->random(uNumArtifactsNotFound)];
        pItemTable->SetSpecialBonus(this);
        return true;
    } else {
        return false;
    }
}

void ItemGen::generateGold(ItemTreasureLevel treasureLevel) {
    assert(isRandomTreasureLevel(treasureLevel));

    Reset();
    SetIdentified();

    switch (treasureLevel) {
    case ITEM_TREASURE_LEVEL_1:
        goldAmount = grng->random(51) + 50;
        uItemID = ITEM_GOLD_SMALL;
        break;
    case ITEM_TREASURE_LEVEL_2:
        goldAmount = grng->random(101) + 100;
        uItemID = ITEM_GOLD_SMALL;
        break;
    case ITEM_TREASURE_LEVEL_3:
        goldAmount = grng->random(301) + 200;
        uItemID = ITEM_GOLD_MEDIUM;
        break;
    case ITEM_TREASURE_LEVEL_4:
        goldAmount = grng->random(501) + 500;
        uItemID = ITEM_GOLD_MEDIUM;
        break;
    case ITEM_TREASURE_LEVEL_5:
        goldAmount = grng->random(1001) + 1000;
        uItemID = ITEM_GOLD_LARGE;
        break;
    case ITEM_TREASURE_LEVEL_6:
        goldAmount = grng->random(3001) + 2000;
        uItemID = ITEM_GOLD_LARGE;
        break;
    default:
        assert(false);
        break;
    }
}

template<class Key, class ActualKey>
static void AddToMap(std::map<Key, std::map<CharacterAttributeType, CEnchantment>> &map,
                     ActualKey key, CharacterAttributeType subkey, int bonusValue = 0, CharacterSkillType skill = CHARACTER_SKILL_INVALID) {
    auto &submap = map[key];

    assert(!submap.contains(subkey));

    submap[subkey] = CEnchantment(bonusValue, skill);
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
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_MIGHT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_GODS, CHARACTER_ATTRIBUTE_PERSONALITY, 10);

    // of Air Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_AIR_MAGIC, CHARACTER_ATTRIBUTE_SKILL_AIR, 0, CHARACTER_SKILL_AIR);

    // of Body Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_BODY_MAGIC, CHARACTER_ATTRIBUTE_SKILL_BODY, 0, CHARACTER_SKILL_BODY);

    // of Dark Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DARK_MAGIC, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);

    // of Earth Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_EARTH_MAGIC, CHARACTER_ATTRIBUTE_SKILL_EARTH, 0, CHARACTER_SKILL_EARTH);

    // of Fire Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_FIRE_MAGIC, CHARACTER_ATTRIBUTE_SKILL_FIRE, 0, CHARACTER_SKILL_FIRE);

    // of Light Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_LIGHT_MAGIC, CHARACTER_ATTRIBUTE_SKILL_LIGHT, 0, CHARACTER_SKILL_LIGHT);

    // of Mind Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_MIND_MAGIC, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);

    // of Spirit Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SPIRIT_MAGIC, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);

    // of Water Magic
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_WATER_MAGIC, CHARACTER_ATTRIBUTE_SKILL_WATER, 0, CHARACTER_SKILL_WATER);

    // of Doom, +1 to Seven Stats, HP, SP, Armor, Resistances (in txt it says 4, need to check!)
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_ACCURACY, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_INTELLIGENCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_ENDURANCE, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_LUCK, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_SPEED, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_MIGHT, 1);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DOOM, CHARACTER_ATTRIBUTE_PERSONALITY, 1);

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
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_DRAGON, CHARACTER_ATTRIBUTE_MIGHT, 25);

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
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, CHARACTER_ATTRIBUTE_MIGHT, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_SUN, CHARACTER_ATTRIBUTE_PERSONALITY, 10);

    // of The Troll
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_TROLL, CHARACTER_ATTRIBUTE_ENDURANCE, 15);

    // of The Unicorn
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_UNICORN, CHARACTER_ATTRIBUTE_LUCK, 15);

    // Warriors
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, CHARACTER_ATTRIBUTE_ENDURANCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WARRIORS, CHARACTER_ATTRIBUTE_MIGHT, 5);

    // Wizards
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, CHARACTER_ATTRIBUTE_INTELLIGENCE, 5);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_WIZARDS, CHARACTER_ATTRIBUTE_PERSONALITY, 5);

    // Monks
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, CHARACTER_ATTRIBUTE_SKILL_DODGE, 3, CHARACTER_SKILL_DODGE);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_MONKS, CHARACTER_ATTRIBUTE_SKILL_UNARMED, 3, CHARACTER_SKILL_UNARMED);

    // Thieves
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, CHARACTER_ATTRIBUTE_SKILL_STEALING, 3, CHARACTER_SKILL_STEALING);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_THIEVES, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 3, CHARACTER_SKILL_TRAP_DISARM);

    // of Identifying
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, CHARACTER_ATTRIBUTE_SKILL_ITEM_ID, 3, CHARACTER_SKILL_ITEM_ID);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_IDENTIFYING, CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID, 3, CHARACTER_SKILL_MONSTER_ID);

    // Assassins
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_ASSASINS, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 2, CHARACTER_SKILL_TRAP_DISARM);

    // Barbarians
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_BARBARIANS, CHARACTER_ATTRIBUTE_AC_BONUS, 5);

    // of the Storm
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_STORM, CHARACTER_ATTRIBUTE_RESIST_AIR, 20);

    // of the Ocean
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(specialBonusMap, ITEM_ENCHANTMENT_OF_OCEAN, CHARACTER_ATTRIBUTE_SKILL_ALCHEMY, 2, CHARACTER_SKILL_ALCHEMY);
}

// TODO: where is it used?
void ItemGen::PopulateRegularBonusMap() {
    // of Might
    AddToMap(regularBonusMap, 1, CHARACTER_ATTRIBUTE_MIGHT);

    // of Thought
    AddToMap(regularBonusMap, 2, CHARACTER_ATTRIBUTE_INTELLIGENCE);

    // of Charm
    AddToMap(regularBonusMap, 3, CHARACTER_ATTRIBUTE_PERSONALITY);

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
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_IRON_FEATHER, CHARACTER_ATTRIBUTE_MIGHT, 40);

    // Wallace
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_WALLACE, CHARACTER_ATTRIBUTE_PERSONALITY, 40);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_LUCK, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CORSAIR, CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_MIGHT, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_INTELLIGENCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_PERSONALITY, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_ENDURANCE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_ACCURACY, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_SPEED, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_GOVERNORS_ARMOR, CHARACTER_ATTRIBUTE_LUCK, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_YORUBA, CHARACTER_ATTRIBUTE_ENDURANCE, 25);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SPLITTER, CHARACTER_ATTRIBUTE_RESIST_FIRE, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ULLYSES, CHARACTER_ATTRIBUTE_ACCURACY, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, CHARACTER_ATTRIBUTE_SKILL_DODGE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HANDS_OF_THE_MASTER, CHARACTER_ATTRIBUTE_SKILL_UNARMED, 10);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, CHARACTER_ATTRIBUTE_SPEED, 40);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, CHARACTER_ATTRIBUTE_SKILL_WATER, 0, CHARACTER_SKILL_WATER);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_RULERS_RING, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);

    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_MIGHT, 150);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_INTELLIGENCE, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_PERSONALITY, -40);
    AddToMap(artifactBonusMap, ITEM_RELIC_MASH, CHARACTER_ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, CHARACTER_ATTRIBUTE_SKILL_DARK, 0, CHARACTER_SKILL_DARK);
    AddToMap(artifactBonusMap, ITEM_RELIC_ETHRICS_STAFF, CHARACTER_ATTRIBUTE_SKILL_MEDITATION, 15);

    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_SKILL_STEALING, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_LUCK, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_FIRE, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_WATER, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_AIR, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_EARTH, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_HARECKS_LEATHER, CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_OLD_NICK, CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM, 5);

    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_MIGHT, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_ENDURANCE, 100);
    AddToMap(artifactBonusMap, ITEM_RELIC_AMUCK, CHARACTER_ATTRIBUTE_AC_BONUS, -15);

    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_SKILL_SHIELD, 5);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_RESIST_MIND, -10);
    AddToMap(artifactBonusMap, ITEM_RELIC_GLORY_SHIELD, CHARACTER_ATTRIBUTE_RESIST_BODY, -10);

    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, CHARACTER_ATTRIBUTE_ENDURANCE, 50);
    AddToMap(artifactBonusMap, ITEM_RELIC_KELEBRIM, CHARACTER_ATTRIBUTE_RESIST_EARTH, -30);

    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_SKILL_LIGHT, 0, CHARACTER_SKILL_LIGHT);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_PERSONALITY, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_RELIC_TALEDONS_HELM, CHARACTER_ATTRIBUTE_LUCK, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, CHARACTER_ATTRIBUTE_SKILL_LEARNING, +15);
    AddToMap(artifactBonusMap, ITEM_RELIC_SCHOLARS_CAP, CHARACTER_ATTRIBUTE_ENDURANCE, -50);

    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_SKILL_FIRE, 0, CHARACTER_SKILL_FIRE);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_RESIST_WATER, +50);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_PERSONALITY, 30);
    AddToMap(artifactBonusMap, ITEM_RELIC_PHYNAXIAN_CROWN, CHARACTER_ATTRIBUTE_AC_BONUS, -20);

    AddToMap(artifactBonusMap, ITEM_RELIC_TITANS_BELT, CHARACTER_ATTRIBUTE_MIGHT, 75);
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

    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SKILL_MIND, 0, CHARACTER_SKILL_MIND);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SKILL_BODY, 0, CHARACTER_SKILL_BODY);
    AddToMap(artifactBonusMap, ITEM_RELIC_JUSTICE, CHARACTER_ATTRIBUTE_SPEED, -40);

    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_SKILL_SPIRIT, 0, CHARACTER_SKILL_SPIRIT);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_MIGHT, 75);
    AddToMap(artifactBonusMap, ITEM_RELIC_MEKORIGS_HAMMER, CHARACTER_ATTRIBUTE_RESIST_AIR, -50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_SPEED, 100);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_ACCURACY, 50);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HERMES_SANDALS, CHARACTER_ATTRIBUTE_RESIST_AIR, 50);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, CHARACTER_ATTRIBUTE_PERSONALITY, -20);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, CHARACTER_ATTRIBUTE_INTELLIGENCE, -20);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, CHARACTER_ATTRIBUTE_PERSONALITY, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_MINDS_EYE, CHARACTER_ATTRIBUTE_INTELLIGENCE, 15);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ELVEN_CHAINMAIL, CHARACTER_ATTRIBUTE_SPEED, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_ELVEN_CHAINMAIL, CHARACTER_ATTRIBUTE_ACCURACY, 15);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_ENDURANCE, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_FORGE_GAUNTLETS, CHARACTER_ATTRIBUTE_RESIST_FIRE, 30);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, CHARACTER_ATTRIBUTE_MIGHT, 15);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_HEROS_BELT, CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER, 5);

    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_FIRE, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_AIR, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_WATER, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_EARTH, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_MIND, 10);
    AddToMap(artifactBonusMap, ITEM_ARTIFACT_LADYS_ESCORT, CHARACTER_ATTRIBUTE_RESIST_BODY, 10);
}

void ItemGen::GetItemBonusSpecialEnchantment(const Character *owner,
                                             CharacterAttributeType attrToGet,
                                             int *additiveBonus,
                                             int *halfSkillBonus) const {
    auto pos = specialBonusMap.find(this->special_enchantment);
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

void ItemGen::GetItemBonusArtifact(const Character *owner,
                                   CharacterAttributeType attrToGet,
                                   int *bonusSum) const {
    auto pos = artifactBonusMap.find(this->uItemID);
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

bool ItemGen::IsRegularEnchanmentForAttribute(CharacterAttributeType attrToGet) {
    //auto pos = specialBonusMap.find(this->attributeEnchantment);
    //if (pos == specialBonusMap.end())
    //    return false;

    //return pos->second.find(attrToGet) != pos->second.end();
    // TODO(captainurist): what is this code about? ^
    return false;
}

ItemType ItemGen::GetItemEquipType() const {
    // to avoid nzi - is this safe??
    if (this->uItemID == ITEM_NULL)
        return ITEM_TYPE_NONE;
    else
        return pItemTable->pItems[this->uItemID].uEquipType;
}

CharacterSkillType ItemGen::GetPlayerSkillType() const {
    CharacterSkillType skl = pItemTable->pItems[this->uItemID].uSkillType;
    if (skl == CHARACTER_SKILL_CLUB && engine->config->gameplay.TreatClubAsMace.value()) {
        // club skill not used but some items load it
        skl = CHARACTER_SKILL_MACE;
    }
    return skl;
}

const std::string& ItemGen::GetIconName() const {
    return pItemTable->pItems[this->uItemID].iconName;
}

uint8_t ItemGen::GetDamageDice() const {
    return pItemTable->pItems[this->uItemID].uDamageDice;
}

uint8_t ItemGen::GetDamageRoll() const {
    return pItemTable->pItems[this->uItemID].uDamageRoll;
}

uint8_t ItemGen::GetDamageMod() const {
    return pItemTable->pItems[this->uItemID].uDamageMod;
}

//----- (0043C91D) --------------------------------------------------------
std::string GetItemTextureFilename(ItemId item_id, int index, int shoulder) {
    // For some reason artifact textures are stored using different ids,
    // and textures under original ids simply don't exist.
    int texture_id = std::to_underlying(valueOr(itemTextureIdByItemId, item_id, item_id));

    switch (pItemTable->pItems[item_id].uEquipType) {
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
bool ItemGen::canSellRepairIdentifyAt(HouseId houseId) {
    if (this->IsStolen())
        return false;

    if (isQuestItem(uItemID))
        return false; // Can't sell quest items.

    if (isArtifact(uItemID) && !isSpawnableArtifact(uItemID))
        return false; // Can't sell quest artifacts, e.g. Hermes Sandals.

    if (::isMessageScroll(uItemID) && !isRecipe(uItemID))
        return false; // Can't sell message scrolls. Recipes are sellable at alchemy shops.

    switch (buildingTable[houseId].uType) {
        case BUILDING_WEAPON_SHOP:
            return this->isWeapon();
        case BUILDING_ARMOR_SHOP:
            return this->isArmor();
        case BUILDING_MAGIC_SHOP:
            return this->GetPlayerSkillType() == CHARACTER_SKILL_MISC || this->isBook();
        case BUILDING_ALCHEMY_SHOP:
            return this->isReagent() ||
                   this->isPotion() ||
                   (this->isMessageScroll() && isRecipe(this->uItemID));
        default:
            return false;
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
