#pragma once

#include "Engine/Objects/Items.h"

#include "Utility/IndexedArray.h"

struct Player;

struct CEnchantment {
    CEnchantment() {}
    explicit CEnchantment(int bonus, uint16_t Player::* skillPtr = nullptr)
        : statPtr(skillPtr), statBonus(bonus) {}

    uint16_t Player::* statPtr = nullptr;
    int statBonus = 0;
};

struct ItemEnchantment {  // Bonus|Sta|Of
                          // Name|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul
    char *pBonusStat;
    char *pOfName;
    /*	union{
                    struct {
                            unsigned char to_arm;
                            unsigned char to_shld;
                            unsigned char to_helm;
                            unsigned char to_belt;
                            unsigned char to_cape;
                            unsigned char to_gaunt;
                            unsigned char to_boot;
                            unsigned char to_ring;
                            unsigned char to_amul;
                            }; */
    IndexedArray<unsigned char, EQUIP_FIRST_NORMAL_ENCHANTABLE, EQUIP_LAST_NORMAL_ENCHANTABLE> chancesByItemType;
    // };
};

struct ItemSpecialEnchantment {  // 1Ch
                                 // Bonus Stat|Name
    // Add|W1|W2|Miss|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul|Value|Lvl|Description
    // fo special Bonuses and values

    char *pBonusStatement;   // 0
    char *pNameAdd;          // 4
    IndexedArray<char, EQUIP_FIRST_SPECIAL_ENCHANTABLE, EQUIP_LAST_SPECIAL_ENCHANTABLE> to_item_apply;  // 8
    int iValue;              // 14
    int iTreasureLevel;      // 18
};
