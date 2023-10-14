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

struct ItemEnchantmentTable {  // Bonus|Sta|Of
                          // Name|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul
    std::string pBonusStat;
    std::string pOfName;
    IndexedArray<unsigned char, EQUIP_FIRST_NORMAL_ENCHANTABLE, EQUIP_LAST_NORMAL_ENCHANTABLE> chancesByItemType;
};

struct ItemSpecialEnchantmentTable {  // 1Ch
                                 // Bonus Stat|Name
    // Add|W1|W2|Miss|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul|Value|Lvl|Description
    // fo special Bonuses and values

    std::string pBonusStatement;   // 0
    std::string pNameAdd;          // 4
    IndexedArray<char, EQUIP_FIRST_SPECIAL_ENCHANTABLE, EQUIP_LAST_SPECIAL_ENCHANTABLE> to_item_apply;  // 8
    int iValue;              // 14
    int iTreasureLevel;      // 18
};
