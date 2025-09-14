#pragma once

#include <cassert>
#include <string>
#include <initializer_list>
#include <utility>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Spells/SpellEnums.h"

#include "Utility/Segment.h"

#include "CharacterEnumFunctions.h"
#include "ItemEnums.h"

class Localization;

//
// DamageType
//

std::string displayNameForDamageType(DamageType damageType, Localization *localization);


//
// ItemTreasureLevel
//

inline bool isRandomTreasureLevel(ItemTreasureLevel level) {
    return level >= ITEM_TREASURE_LEVEL_FIRST_RANDOM && level <= ITEM_TREASURE_LEVEL_LAST_RANDOM;
}


//
// ItemId
//

/**
 * Checks if item is a regular item - a weapon or equipment that's not a quest item or an artifact.
 *
 * Regular items can be enchanted, unlike other types of items.
 *
 * @param type                          Item type to check.
 * @return                              Whether the provided item is a regular item.
 */
inline bool isRegular(ItemId type) {
    return type >= ITEM_FIRST_REGULAR && type <= ITEM_LAST_REGULAR;
}

inline bool isRecipe(ItemId type) {
    return type >= ITEM_FIRST_RECIPE && type <= ITEM_LAST_RECIPE;
}

inline bool isWand(ItemId type) {
    return type >= ITEM_FIRST_WAND && type <= ITEM_LAST_WAND;
}

inline bool isPotion(ItemId type) {
    return type >= ITEM_FIRST_POTION && type <= ITEM_LAST_POTION;
}

inline bool isReagent(ItemId type) {
    return type >= ITEM_FIRST_REAGENT && type <= ITEM_LAST_REAGENT;
}

inline bool isEnchantingPotion(ItemId type) {
    return type >= ITEM_POTION_FLAMING && type <= ITEM_POTION_SWIFT || type == ITEM_POTION_SLAYING;
}

inline bool isMessageScroll(ItemId type) {
    return type >= ITEM_FIRST_MESSAGE_SCROLL && type <= ITEM_LAST_MESSAGE_SCROLL;
}

inline bool isArtifact(ItemId type) {
    return type >= ITEM_FIRST_ARTIFACT && type <= ITEM_LAST_ARTIFACT;
}

inline bool isSpawnableArtifact(ItemId type) {
    return type >= ITEM_FIRST_SPAWNABLE_ARTIFACT && type <= ITEM_LAST_SPAWNABLE_ARTIFACT;
}

inline bool isGold(ItemId type) {
    return type >= ITEM_FIRST_GOLD && type <= ITEM_LAST_GOLD;
}

inline bool isRandomItem(ItemId type) {
    return type >= ITEM_FIRST_RANDOM && type <= ITEM_LAST_RANDOM;
}

inline bool isSpellbook(ItemId item) {
    return item >= ITEM_FIRST_SPELLBOOK && item <= ITEM_LAST_SPELLBOOK;
}

inline bool isQuestItem(ItemId item) {
    return item >= ITEM_FIRST_QUEST && item <= ITEM_LAST_QUEST;
}

inline bool isAncientWeapon(ItemId item) {
    return item == ITEM_BLASTER || item == ITEM_BLASTER_RIFLE;
}

inline ItemTreasureLevel randomItemTreasureLevel(ItemId type) {
    assert(isRandomItem(type));
    return static_cast<ItemTreasureLevel>(-std::to_underlying(type));
}

inline Segment<ItemId> spellbooksForSchool(MagicSchool school, Mastery maxMastery = MASTERY_GRANDMASTER) {
    int spellSchoolSequential = std::to_underlying(school);
    int firstSpell = std::to_underlying(ITEM_FIRST_SPELLBOOK);
    int numSpells = spellCountForMastery(maxMastery);
    int firstSpellInSchool = firstSpell + 11 * spellSchoolSequential;
    int lastSpellInSchool = firstSpellInSchool + numSpells - 1;
    return {static_cast<ItemId>(firstSpellInSchool), static_cast<ItemId>(lastSpellInSchool)};
}

inline Segment<ItemId> allRecipeScrolls() {
    return {ITEM_FIRST_RECIPE, ITEM_LAST_RECIPE};
}

/**
 * @return                              Range of all items that can be randomly generated as loot. Note that not all
 *                                      of the entries might actually be valid.
 */
inline Segment<ItemId> allSpawnableItems() {
    return {ITEM_FIRST_SPAWNABLE, ITEM_LAST_SPAWNABLE};
}

/**
 * @return                              Range of all artifacts and relics that can be randomly generated as loot
 *                                      (e.g. picked up from a dragon corpse). Note that not all artifacts can be
 *                                      generated this way, e.g. Hermes' Sandals can only be picked up from a
 *                                      Mega Dragon's corpse.
 */
inline Segment<ItemId> allSpawnableArtifacts() {
    return {ITEM_FIRST_SPAWNABLE_ARTIFACT, ITEM_LAST_SPAWNABLE_ARTIFACT};
}

/**
 * @return                              List of lowest level (power=1) alchemical reagents.
 */
inline std::initializer_list<ItemId> allLevel1Reagents() {
    static constexpr std::initializer_list<ItemId> result = {
        ITEM_REAGENT_WIDOWSWEEP_BERRIES,
        ITEM_REAGENT_PHIRNA_ROOT,
        ITEM_REAGENT_POPPYSNAPS
    };
    return result;
}

inline ItemEnchantment potionEnchantment(ItemId enchantingPotion) {
    assert(isEnchantingPotion(enchantingPotion));

    switch (enchantingPotion) {
    case ITEM_POTION_SLAYING:   return ITEM_ENCHANTMENT_DRAGON_SLAYING;
    case ITEM_POTION_FLAMING:   return ITEM_ENCHANTMENT_OF_FLAME;
    case ITEM_POTION_FREEZING:  return ITEM_ENCHANTMENT_OF_FROST;
    case ITEM_POTION_NOXIOUS:   return ITEM_ENCHANTMENT_OF_POISON;
    case ITEM_POTION_SHOCKING:  return ITEM_ENCHANTMENT_OF_SPARKS;
    case ITEM_POTION_SWIFT:     return ITEM_ENCHANTMENT_SWIFT;
    default:
        assert(false);
        return ITEM_ENCHANTMENT_NULL;
    }
}

SpellId spellForSpellbook(ItemId spellbook);

SpellId spellForScroll(ItemId scroll);

SpellId spellForWand(ItemId wand);

inline Attribute statForPureStatPotion(ItemId pureStatPotion) {
    switch (pureStatPotion) {
    default: assert(false); [[fallthrough]];
    case ITEM_POTION_PURE_LUCK:         return ATTRIBUTE_LUCK;
    case ITEM_POTION_PURE_SPEED:        return ATTRIBUTE_SPEED;
    case ITEM_POTION_PURE_INTELLECT:    return ATTRIBUTE_INTELLIGENCE;
    case ITEM_POTION_PURE_ENDURANCE:    return ATTRIBUTE_ENDURANCE;
    case ITEM_POTION_PURE_PERSONALITY:  return ATTRIBUTE_PERSONALITY;
    case ITEM_POTION_PURE_ACCURACY:     return ATTRIBUTE_ACCURACY;
    case ITEM_POTION_PURE_MIGHT:        return ATTRIBUTE_MIGHT;
    }
}


//
// ItemSlot
//

inline Segment<ItemSlot> allRingSlots() {
    return {ITEM_SLOT_RING1, ITEM_SLOT_RING6};
}

inline ItemSlot ringSlot(int index) {
    assert(index >= 0 && index <= 5);
    return static_cast<ItemSlot>(std::to_underlying(ITEM_SLOT_RING1) + index);
}

inline Segment<ItemSlot> allItemSlots() {
    return {ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID};
}


//
// ItemType
//

/**
 * @param type                          Type to check.
 * @return                              Whether the provided type represents a `passive` equipment, e.g. shields,
 *                                      armor, rings, amulets, etc.
 */
inline bool isPassiveEquipment(ItemType type) {
    return type >= ITEM_TYPE_ARMOUR && type <= ITEM_TYPE_AMULET;
}

/**
 * @param type                          Type to check.
 * @return                              Whether the provided type represents wearable armor, e.g. any passive
 *                                      equipment except amulets and rings.
 */
inline bool isArmor(ItemType type) {
    return type >= ITEM_TYPE_ARMOUR && type <= ITEM_TYPE_BOOTS;
}

/**
 * @param type                          Type to check.
 * @return                              Whether the provided type represents a weapon. Note that wands are not
 *                                      considered weapons.
 */
inline bool isWeapon(ItemType type) {
    return type >= ITEM_TYPE_SINGLE_HANDED && type <= ITEM_TYPE_BOW;
}

/**
 * @param type                          Type to check.
 * @return                              Item slots where an item of given type might be (but not necessarily can be)
 *                                      equipped.
 */
inline Segment<ItemSlot> itemSlotsForItemType(ItemType type) {
    switch (type) {
    case ITEM_TYPE_SINGLE_HANDED:   return {ITEM_SLOT_OFF_HAND, ITEM_SLOT_MAIN_HAND};
    case ITEM_TYPE_TWO_HANDED:      return {ITEM_SLOT_MAIN_HAND, ITEM_SLOT_MAIN_HAND};
    case ITEM_TYPE_BOW:             return {ITEM_SLOT_BOW, ITEM_SLOT_BOW};
    case ITEM_TYPE_ARMOUR:          return {ITEM_SLOT_ARMOUR, ITEM_SLOT_ARMOUR};
    case ITEM_TYPE_SHIELD:          return {ITEM_SLOT_OFF_HAND, ITEM_SLOT_OFF_HAND};
    case ITEM_TYPE_HELMET:          return {ITEM_SLOT_HELMET, ITEM_SLOT_HELMET};
    case ITEM_TYPE_BELT:            return {ITEM_SLOT_BELT, ITEM_SLOT_BELT};
    case ITEM_TYPE_CLOAK:           return {ITEM_SLOT_CLOAK, ITEM_SLOT_CLOAK};
    case ITEM_TYPE_GAUNTLETS:       return {ITEM_SLOT_GAUNTLETS, ITEM_SLOT_GAUNTLETS};
    case ITEM_TYPE_BOOTS:           return {ITEM_SLOT_BOOTS, ITEM_SLOT_BOOTS};
    case ITEM_TYPE_RING:            return allRingSlots();
    case ITEM_TYPE_AMULET:          return {ITEM_SLOT_AMULET, ITEM_SLOT_AMULET};
    case ITEM_TYPE_WAND:            return {ITEM_SLOT_MAIN_HAND, ITEM_SLOT_MAIN_HAND};
    default:
        assert(false);
        [[fallthrough]];
    case ITEM_TYPE_REAGENT:
    case ITEM_TYPE_POTION:
    case ITEM_TYPE_SPELL_SCROLL:
    case ITEM_TYPE_BOOK:
    case ITEM_TYPE_MESSAGE_SCROLL:
    case ITEM_TYPE_GOLD:
    case ITEM_TYPE_GEM:
    case ITEM_TYPE_NONE:
        return {};
    }
}


//
// RandomItemType
//

inline Segment<RandomItemType> allSpawnableRandomItemTypes() {
    return {RANDOM_ITEM_FIRST_SPAWNABLE, RANDOM_ITEM_LAST_SPAWNABLE};
}

inline std::pair<ItemType, Skill> itemTypeOrSkillForRandomItemType(RandomItemType type) {
    switch (type) {
    case RANDOM_ITEM_ANY:           return {ITEM_TYPE_INVALID,              SKILL_INVALID};
    case RANDOM_ITEM_WEAPON:        return {ITEM_TYPE_SINGLE_HANDED,        SKILL_INVALID};
    case RANDOM_ITEM_ARMOR:         return {ITEM_TYPE_ARMOUR,               SKILL_INVALID};
    case RANDOM_ITEM_MICS:          return {ITEM_TYPE_INVALID,              SKILL_MISC};
    case RANDOM_ITEM_SWORD:         return {ITEM_TYPE_INVALID,              SKILL_SWORD};
    case RANDOM_ITEM_DAGGER:        return {ITEM_TYPE_INVALID,              SKILL_DAGGER};
    case RANDOM_ITEM_AXE:           return {ITEM_TYPE_INVALID,              SKILL_AXE};
    case RANDOM_ITEM_SPEAR:         return {ITEM_TYPE_INVALID,              SKILL_SPEAR};
    case RANDOM_ITEM_BOW:           return {ITEM_TYPE_INVALID,              SKILL_BOW};
    case RANDOM_ITEM_MACE:          return {ITEM_TYPE_INVALID,              SKILL_MACE};
    case RANDOM_ITEM_CLUB:          return {ITEM_TYPE_INVALID,              SKILL_CLUB};
    case RANDOM_ITEM_STAFF:         return {ITEM_TYPE_INVALID,              SKILL_STAFF};
    case RANDOM_ITEM_LEATHER_ARMOR: return {ITEM_TYPE_INVALID,              SKILL_LEATHER};
    case RANDOM_ITEM_CHAIN_ARMOR:   return {ITEM_TYPE_INVALID,              SKILL_CHAIN};
    case RANDOM_ITEM_PLATE_ARMOR:   return {ITEM_TYPE_INVALID,              SKILL_PLATE};
    case RANDOM_ITEM_SHIELD:        return {ITEM_TYPE_SHIELD,               SKILL_INVALID};
    case RANDOM_ITEM_HELMET:        return {ITEM_TYPE_HELMET,               SKILL_INVALID};
    case RANDOM_ITEM_BELT:          return {ITEM_TYPE_BELT,                 SKILL_INVALID};
    case RANDOM_ITEM_CLOAK:         return {ITEM_TYPE_CLOAK,                SKILL_INVALID};
    case RANDOM_ITEM_GAUNTLETS:     return {ITEM_TYPE_GAUNTLETS,            SKILL_INVALID};
    case RANDOM_ITEM_BOOTS:         return {ITEM_TYPE_BOOTS,                SKILL_INVALID};
    case RANDOM_ITEM_RING:          return {ITEM_TYPE_RING,                 SKILL_INVALID};
    case RANDOM_ITEM_AMULET:        return {ITEM_TYPE_AMULET,               SKILL_INVALID};
    case RANDOM_ITEM_WAND:          return {ITEM_TYPE_WAND,                 SKILL_INVALID};
    case RANDOM_ITEM_SPELL_SCROLL:  return {ITEM_TYPE_SPELL_SCROLL,         SKILL_INVALID};
    case RANDOM_ITEM_POTION:        return {ITEM_TYPE_POTION,               SKILL_INVALID};
    case RANDOM_ITEM_REAGENT:       return {ITEM_TYPE_REAGENT,              SKILL_INVALID};
    case RANDOM_ITEM_GEM:           return {ITEM_TYPE_GEM,                  SKILL_INVALID};
    default:
        // TODO(captainurist): Values 1-19 are supposedly mapped to ItemType, but are not used in MM7.
        //                     Figure out if this is an MM6 remnant, if not then just drop.
        assert(false);
        return {static_cast<ItemType>(std::to_underlying(type) - 1), SKILL_INVALID};
    }
}
