#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Shop : public GUIWindow_House {
 public:
    explicit GUIWindow_Shop(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Shop() {}

    virtual void houseDialogueOptionSelected(DialogueId option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;
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
    virtual std::vector<DialogueId> listShopLearnableSkills() = 0;
};

class GUIWindow_WeaponShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_WeaponShop(HouseId houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_WeaponShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DialogueId> listShopLearnableSkills() override;
};

class GUIWindow_ArmorShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_ArmorShop(HouseId houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_ArmorShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DialogueId> listShopLearnableSkills() override;
};

class GUIWindow_MagicAlchemyShop : public GUIWindow_Shop {
 public:
    explicit GUIWindow_MagicAlchemyShop(HouseId houseId) : GUIWindow_Shop(houseId) {}
    virtual ~GUIWindow_MagicAlchemyShop() {}

 private:
    virtual void shopWaresDialogue(bool isSpecial) override;
};

class GUIWindow_MagicShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_MagicShop(HouseId houseId) : GUIWindow_MagicAlchemyShop(houseId) {}
    virtual ~GUIWindow_MagicShop() {}

 private:
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DialogueId> listShopLearnableSkills() override;
};

class GUIWindow_AlchemyShop : public GUIWindow_MagicAlchemyShop {
 public:
    explicit GUIWindow_AlchemyShop(HouseId houseId) : GUIWindow_MagicAlchemyShop(houseId) {}
    virtual ~GUIWindow_AlchemyShop() {}

    virtual std::vector<DialogueId> listDialogueOptions() override;
    virtual void playHouseGoodbyeSpeech() override;

 private:
    virtual void generateShopItems(bool isSpecial) override;
    virtual std::vector<DialogueId> listShopLearnableSkills() override;
};

void UIShop_Buy_Identify_Repair();

extern GraphicsImage *shop_ui_background;

extern std::array<GraphicsImage *, 12> shop_ui_items_in_store;
extern std::array<int, 6> weaponYPos;
