#pragma once

#include <string>

#include "Utility/IndexedArray.h"

#include "Engine/Objects/ItemEnums.h" // TODO(captainurist): Data -> Objects dependency, we don't want that.

struct StandardEnchantmentData {
    std::string attributeName; // Name of the attribute this applies to, e.g. "Might" or "Armsmaster skill".
    std::string itemSuffix; // Suffix for the enchanted item, e.g. "of Might" or "of Arms".
    IndexedArray<unsigned char, ITEM_TYPE_FIRST_NORMAL_ENCHANTABLE, ITEM_TYPE_LAST_NORMAL_ENCHANTABLE> chanceByItemType;
};
