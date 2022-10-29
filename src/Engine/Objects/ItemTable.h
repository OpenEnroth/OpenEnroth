#pragma once

#include <string>

#include "Engine/Objects/ItemEnchantment.h"
#include "Engine/Objects/Items.h"
#include "Utility/IndexedArray.h"

#pragma pack(push, 1)
struct BonusRange {
    unsigned int minR;
    unsigned int maxR;
};
#pragma pack(pop)

/*  176 */
#pragma pack(push, 1)
struct ItemTable {
    void Initialize();
    void LoadPotions();
    void LoadPotionNotes();
    void GenerateItem(int treasure_level, unsigned int uTreasureType,
        ItemGen* pItem);
    void SetSpecialBonus(ItemGen* pItem);
    bool IsMaterialSpecial(ItemGen* pItem);
    bool IsMaterialNonCommon(ItemGen* pItem);
    void Release();
    void PrintItemTypesEnum();

    int uAllItemsCount; // TODO(captainurist): DROP!
    IndexedArray<ItemDesc, ITEM_FIRST_VALID, ITEM_LAST_VALID> pItems;                   // 4-9604h
    ItemEnchantment pEnchantments[24];                // 9604h
    IndexedArray<ItemSpecialEnchantment, ITEM_ENCHANTMENT_FIRST_VALID, ITEM_ENCHANTMENT_LAST_VALID> pSpecialEnchantments;  // 97E4h -9FC4h
    char field_9FC4[5000];
    char field_B348[5000];
    char field_C6D0[5000];
    char field_DA58[5000];
    char field_EDE0[384];
    uint16_t potion_data[50][50];  // 77B2h*2=EF64h  -102ECh
    uint16_t potion_note[50][50];  // 8176h*2=102ECh -11674h
    std::string pItemsTXT_Raw;              // 11674h
    std::string pRndItemsTXT_Raw;
    std::string pStdItemsTXT_Raw;           // 1167Ch
    std::string pSpcItemsTXT_Raw;           // 11680h
    unsigned int uChanceByTreasureLvlSumm[6];   // 11684
    unsigned int uBonusChanceStandart[6];       // 1169c
    unsigned int uBonusChanceSpecial[6];        // 116B4
    unsigned int uBonusChanceWpSpecial[6];      // 116cc -116e4
    IndexedArray<unsigned int, EQUIP_FIRST_NORMAL_ENCHANTABLE, EQUIP_LAST_NORMAL_ENCHANTABLE> pEnchantmentsSumm; // 116E4h -11708h
    BonusRange bonus_ranges[6];                 // 45C2h*4 =11708h
    //IndexedArray<int, EQUIP_LAST_SPECIAL_ENCHANTABLE>
    IndexedArray<unsigned int, EQUIP_FIRST_SPECIAL_ENCHANTABLE, EQUIP_LAST_SPECIAL_ENCHANTABLE> pSpecialEnchantmentsSumm; // 11738h  // TODO(captainurist): Can be dropped?
    unsigned int pSpecialEnchantments_count;    // 11798h
    char field_1179C;
    char field_1179D;
    char field_1179E;
    char field_1179F;
};
#pragma pack(pop)

extern struct ItemTable* pItemTable;
