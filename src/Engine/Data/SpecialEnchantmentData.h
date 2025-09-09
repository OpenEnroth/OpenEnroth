#pragma once

#include <string>

#include "Engine/Objects/ItemEnums.h" // TODO(captainurist): Data -> Objects dependency, we don't want that.

#include "Utility/IndexedArray.h"

struct SpecialEnchantmentData {
    std::string description; // Enchantment description, e.g. "Explosive impact!".
    std::string itemSuffixOrPrefix; // Suffix or prefix for the enchanted item, e.g. "of Carnage". Whether it's a prefix
    // or a suffix is hardcoded in item name generation function.
    IndexedArray<char, ITEM_TYPE_FIRST_SPECIAL_ENCHANTABLE, ITEM_TYPE_LAST_SPECIAL_ENCHANTABLE> chanceByItemType;
    int additionalValue; // Value in gold added to enchanted item's base value.
    int iTreasureLevel;
};
