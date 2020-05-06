#pragma once

#include "Engine/Objects/Items.h"

struct Player;

typedef struct CEnchantment {
    CEnchantment(int bonus, unsigned __int16 Player::* skillPtr = nullptr)
        : statPtr(skillPtr), statBonus(bonus) {}

    uint16_t Player::* statPtr;
    int statBonus;
} CEnchantment;

/*  177 */
#pragma pack(push, 1)
struct ItemEnchantment {  // Bonus|Sta|Of
                          // Name|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul
    char* pBonusStat;
    char* pOfName;
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
    unsigned char to_item[12];
    // };
};
#pragma pack(pop)

/*  178 */
#pragma pack(push, 1)
struct ItemSpecialEnchantment {  // 1Ch
                                 // Bonus Stat|Name
    // Add|W1|W2|Miss|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul|Value|Lvl|Description
    // fo special Bonuses and values

    char* pBonusStatement;   // 0
    char* pNameAdd;          // 4
    char to_item_apply[12];  // 8
    int iValue;              // 14
    int iTreasureLevel;      // 18
};
#pragma pack(pop)