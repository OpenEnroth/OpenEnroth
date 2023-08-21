#pragma once

#include <vector>
#include <array>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Engine/Tables/BuildingTable.h"

enum DIALOGUE_TYPE : int32_t;

class GUIWindow_Shop : public GUIWindow_House {
 public:
    explicit GUIWindow_Shop(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Shop() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions() override;
    virtual void updateDialogueOnEscape() override;
    virtual void houseScreenClick() override;
    virtual void playHouseGoodbyeSpeech() override;

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

    virtual void shopWaresDialogue(bool isSpecial) = 0;
    virtual void generateShopItems(bool isSpecial) = 0;
    virtual std::vector<DIALOGUE_TYPE> listShopLearnableSkills() = 0;
};

class GUIWindow_WeaponShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_WeaponShop(HOUSE_ID houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_WeaponShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DIALOGUE_TYPE> listShopLearnableSkills() override;
};

class GUIWindow_ArmorShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_ArmorShop(HOUSE_ID houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_ArmorShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DIALOGUE_TYPE> listShopLearnableSkills() override;
};

class GUIWindow_MagicAlchemyShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_MagicAlchemyShop(HOUSE_ID houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_MagicAlchemyShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
};

class GUIWindow_MagicShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_MagicShop(HOUSE_ID houseId) : GUIWindow_MagicAlchemyShop(houseId) {}
    virtual ~GUIWindow_MagicShop() {}

 private:
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DIALOGUE_TYPE> listShopLearnableSkills() override;
};

class GUIWindow_AlchemyShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_AlchemyShop(HOUSE_ID houseId) : GUIWindow_MagicAlchemyShop(houseId) {}
    virtual ~GUIWindow_AlchemyShop() {}

    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions() override;
    virtual void playHouseGoodbyeSpeech() override;

 private:
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DIALOGUE_TYPE> listShopLearnableSkills() override;
};

void UIShop_Buy_Identify_Repair();

extern class GraphicsImage *shop_ui_background;

extern std::array<class GraphicsImage *, 12> shop_ui_items_in_store;
extern std::array<int, 6> weaponYPos;
