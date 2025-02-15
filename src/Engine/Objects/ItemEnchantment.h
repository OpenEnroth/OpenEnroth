#pragma once

#include <string>

#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/ItemEnums.h"

#include "Utility/IndexedArray.h"

class Character;

struct CEnchantment {
    CEnchantment() {}
    explicit CEnchantment(int bonus, CharacterSkillType skill = CHARACTER_SKILL_INVALID)
        : skillType(skill), statBonus(bonus) {}

    CharacterSkillType skillType = CHARACTER_SKILL_INVALID;
    int statBonus = 0;
};

// TODO(captainurist): belongs to Engine/Data.

struct ItemEnchantmentTable {
    std::string attributeName; // Name of the attribute this applies to, e.g. "Might" or "Armsmaster skill".
    std::string itemSuffix; // Suffix for the enchanted item, e.g. "of Might" or "of Arms".
    IndexedArray<unsigned char, ITEM_TYPE_FIRST_NORMAL_ENCHANTABLE, ITEM_TYPE_LAST_NORMAL_ENCHANTABLE> chanceByItemType;
};

struct ItemSpecialEnchantmentTable {
    std::string description; // Enchantment description, e.g. "Explosive impact!".
    std::string itemSuffixOrPrefix; // Suffix or prefix for the enchanted item, e.g. "of Carnage". Whether it's a prefix
                                    // or a suffix is hardcoded in item name generation function.
    IndexedArray<char, ITEM_TYPE_FIRST_SPECIAL_ENCHANTABLE, ITEM_TYPE_LAST_SPECIAL_ENCHANTABLE> chanceByItemType;
    int additionalValue; // Value in gold added to enchanted item's base value.
    int iTreasureLevel;
};
