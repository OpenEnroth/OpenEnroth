#pragma once

#include "Engine/Tables/BuildingTable.h"

void WeaponShopWares(GUIWindow dialogwin, bool special = 0);
void ArmorShopWares(GUIWindow dialogwin, bool special = 0);
void AlchemyMagicShopWares(GUIWindow dialogwin, BuildingType building, bool special = 0);


void UIShop_Buy_Identify_Repair();
void sub_4B1523_showSpellbookInfo(ITEM_TYPE spellItemId);
void ShowPopupShopSkills();
void ShowPopupShopItem();
void GetHouseGoodbyeSpeech();
void sub_4B1447_party_fine(int shopId, int stealingResult, int fineToAdd);

extern class Image *shop_ui_background;

extern std::array<class Image *, 12> shop_ui_items_in_store;
