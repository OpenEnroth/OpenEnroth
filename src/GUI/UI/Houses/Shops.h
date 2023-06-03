#pragma once

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Engine/Tables/BuildingTable.h"

class GUIWindow_Shop : public GUIWindow_House {
 public:
    explicit GUIWindow_Shop(HOUSE_ID houseId, BuildingType type) : GUIWindow_House(houseId), _buildingType(type) {}
    virtual ~GUIWindow_Shop() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;

    /**
     * @offset 0x4B1447
     */
    void processStealingResult(int stealingResult, int fineToAdd);

 private:
    void mainDialogue();
    void displayEquipmentDialogue();
    void sellDialogue();
    void identifyDialogue();
    void repairDialogue();
    void learnSkillsDialogue();

    virtual void shopWaresDialogue(bool isSpecial) = 0;

 protected:
    BuildingType _buildingType;
};

class GUIWindow_WeaponShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_WeaponShop(HOUSE_ID houseId) : GUIWindow_Shop(houseId, BuildingType_WeaponShop) {}
    virtual ~GUIWindow_WeaponShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
};

class GUIWindow_ArmorShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_ArmorShop(HOUSE_ID houseId) : GUIWindow_Shop(houseId, BuildingType_ArmorShop) {}
    virtual ~GUIWindow_ArmorShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
};

class GUIWindow_MagicAlchemyShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_MagicAlchemyShop(HOUSE_ID houseId, BuildingType type) : GUIWindow_Shop(houseId, type) {}
    virtual ~GUIWindow_MagicAlchemyShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
};

class GUIWindow_MagicShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_MagicShop(HOUSE_ID houseId) : GUIWindow_MagicAlchemyShop(houseId, BuildingType_MagicShop) {}
    virtual ~GUIWindow_MagicShop() {}
};

class GUIWindow_AlchemyShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_AlchemyShop(HOUSE_ID houseId) : GUIWindow_MagicAlchemyShop(houseId, BuildingType_AlchemistShop) {}
    virtual ~GUIWindow_AlchemyShop() {}
};

void UIShop_Buy_Identify_Repair();

extern class GraphicsImage *shop_ui_background;

extern std::array<class GraphicsImage *, 12> shop_ui_items_in_store;
extern std::array<int, 6> weaponYPos;

// TODO(Nik-RE-dev): these declarations required to enumerate available skills in weapon/armor shops
//                   need to virtualize enumeration through GUIWindow_Shop class and remove these
struct ITEM_VARIATION {
    ITEM_TREASURE_LEVEL treasure_level;
    uint16_t item_class[4];
};

extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationStandart;
extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationSpecial;
extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationStandart;
extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationStandart;
extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationSpecial;
extern const IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationSpecial;
