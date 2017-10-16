#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Events2D.h"
#include "Engine/MapInfo.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"

#include "Engine/Objects/Items.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UiStatusBar.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIHouses.h"

#include "IO/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

Image *shop_ui_background = nullptr;

std::array<Image *, 12> shop_ui_items_in_store;

void ShopDialogMain(GUIWindow dialogwin) {

	if (HouseUI_CheckIfPlayerCanInteract()) {

		pShopOptions[0] = localization->GetString(134); //standard
		pShopOptions[1] = localization->GetString(152); //special
		pShopOptions[2] = localization->GetString(159); //display
		pShopOptions[3] = localization->GetString(160); //learn

		int all_text_height = 0;
		for (int i = 0; i < 4; ++i)
			all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialogwin, 0);

		int textspacings = (174 - all_text_height) / 4;
		int textoffset = 138 - (textspacings / 2);
		
		int pNumString = 0;
		GUIButton *pButton;
		int pColorText;

		for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
			
			pButton = pDialogueWindow->GetControl(i);
			pButton->uY = textspacings + textoffset;
			pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialogwin, 0);
			textoffset = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialogwin, 0) - 1;
			pButton->uW = textoffset;
			
			pColorText = Color16(0xE1u, 0xCDu, 0x23u);
			if (pDialogueWindow->pCurrentPosActiveItem != i)
				pColorText = Color16(0xFFu, 0xFFu, 0xFFu);

			dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
			++pNumString;
		}
	}

}

void ShopDialogDisplayEquip(GUIWindow dialogwin, BuildingType building = BuildingType_WeaponShop) {

	draw_leather();
	CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);

	pShopOptions[0] = localization->GetString(200); //sell
	pShopOptions[1] = localization->GetString(113); //identify
	pShopOptions[2] = localization->GetString(179); //repair - not for alchemy

	int options;
	if (building == BuildingType_AlchemistShop) {
		options = 2;
	}
	else {
		options = 3;
	}
	
	int all_text_height = 0;
	for (int i = 0; i < options; ++i)
		all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialogwin, 0);

	int textspacings = (174 - all_text_height) / options;
	int textoffset = 138 - (textspacings / 2);

	int pNumString = 0;
	GUIButton *pButton;
	int pColorText;

	for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {

		pButton = pDialogueWindow->GetControl(i);
		pButton->uY = textspacings + textoffset;
		pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialogwin, 0);
		textoffset = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialogwin, 0) - 1;
		pButton->uW = textoffset;

		pColorText = Color16(0xE1u, 0xCDu, 0x23u);
		if (pDialogueWindow->pCurrentPosActiveItem != i)
			pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
		dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
		++pNumString;
	}

}

void ShopDialogSellEquip(GUIWindow dialogwin, BuildingType building) {

	if (HouseUI_CheckIfPlayerCanInteract()) {

		draw_leather();
		CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
		GameUI_StatusBar_DrawImmediate(localization->GetString(199), 0);

		Point mouse = pMouse->GetCursorPos();

		int invindex = ((mouse.x - 14) / 32) + 14 * ((mouse.y - 17) / 32);
		if (mouse.x <= 13 || mouse.x >= 462)
			return;

		int pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&invindex);
		
		if (pItemID) {
			ItemGen *item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
			int phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, building, (int)window_SpeakInHouse->ptr_1C, 3);
			auto str = BuildDialogueString(	pMerchantsSellPhrases[phrases_id], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 3 	);
			dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
		}
	}

}

void ShopDialogIdentify(GUIWindow dialogwin, BuildingType building) {

	if (HouseUI_CheckIfPlayerCanInteract()) {

		draw_leather();
		CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
		GameUI_StatusBar_DrawImmediate(localization->GetString(197), 0);
		
		Point mouse = pMouse->GetCursorPos();

		int invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
		if (mouse.x <= 13 || mouse.x >= 462)
			return;

		int pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&invindex);
		
		if (pItemID) {
			ItemGen *item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];

			String str;
			if (!item->IsIdentified()) {
				int phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 4);
				str = BuildDialogueString(pMerchantsIdentifyPhrases[phrases_id], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4 );
			}
			else {
				str = BuildDialogueString("%24", uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4 );
			}

			dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);

		}
	}
}

void ShopDialogRepair(GUIWindow dialogwin, BuildingType building) {

	if (!HouseUI_CheckIfPlayerCanInteract()) {
		
		draw_leather();
		CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
		GameUI_StatusBar_DrawImmediate(localization->GetString(198), 0);
		
		Point mouse = pMouse->GetCursorPos();

		int invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
		if (mouse.x <= 13 || mouse.x >= 462)
			return;

		int pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&invindex);

		if (pItemID || (pPlayers[uActiveCharacter]->pOwnItems[pItemID - 1].uAttributes & 2) ) {
			ItemGen *item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
			int phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, building, (int)window_SpeakInHouse->ptr_1C, 5);
			String str = BuildDialogueString(pMerchantsRepairPhrases[phrases_id], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 5);
			dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
		}
	}
	
}

void ShopDialogLearn(GUIWindow dialogwin) {

	if (HouseUI_CheckIfPlayerCanInteract()) {

		uint item_num = 0;
		int all_text_height = 0;

		int baseprice = (signed __int64)(p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
		int pPrice = baseprice * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
		if (pPrice < baseprice / 3)
			pPrice = baseprice / 3;

		for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {

			if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pDialogueWindow->GetControl(i)->msg_param - 36]
				&& !pPlayers[uActiveCharacter]->pActiveSkills[pDialogueWindow->GetControl(i)->msg_param - 36]) {

				all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(pDialogueWindow->GetControl(i)->msg_param - 36), &dialogwin, 0);
				item_num++;
			}
		}

		if (item_num) {

			auto str = localization->FormatString(401, pPrice); // Skill price: %lu   /   Стоимость навыка: %lu
			dialogwin.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);
			
			int textspacings = (149 - all_text_height) / item_num;
			if (textspacings > 32)
				textspacings = 32;

			int textoffset = 162 - textspacings / 2;

			GUIButton *pButton;
			int pColorText;
			
			for (uint i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {

				pButton = pDialogueWindow->GetControl(i);
				
				if (!byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pButton->msg_param - 36] || pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param - 36]) {
					pButton->uW = 0;
					pButton->uHeight = 0;
					pButton->uY = 0;
				}
				else {
					pButton->uY = textspacings + textoffset;
					pButton->uHeight = pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialogwin, 0);
					textoffset = pButton->uY + pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialogwin, 0) - 1;
					pButton->uW = textoffset;
					pColorText = Color16(0xE1u, 0xCDu, 0x23u);
					if (pDialogueWindow->pCurrentPosActiveItem != i)
						pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
					dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, localization->GetSkillName(pButton->msg_param - 36), 3);
				}
			}
			return;
			
		}

		//seek knowledge elsewhere
		auto str = localization->FormatString( 544, pPlayers[uActiveCharacter]->pName, localization->GetClassName(pPlayers[uActiveCharacter]->classType) ) + "\n \n" + localization->GetString(528);//Больше ничего не могу предложить.
		dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0, 0)) / 2 + 138, Color16(0xE1u, 0xCDu, 0x23u), str, 3);
		
	}
	
}

void WeaponShopWares(GUIWindow dialogwin, bool special = 0) {

	render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
	int item_X = 0;

	for (uint i = 0; i < 6; ++i) {

		if ( (special == 0 && pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID) || 
				( special == 1 && pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID )) {
						render->DrawTextureAlphaNew(((60 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) + item_X) / 640.0f, (weapons_Ypos[i] + 30) / 480.0f, shop_ui_items_in_store[i]);
		}

		item_X += 70;
	}

	if (HouseUI_CheckIfPlayerCanInteract()) {

		uint item_num = 0;
		for (uint i = 0; i < 6; ++i) {
			if ( (special == 0 && pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID) ||
					( special == 1 && pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID )) {
						++item_num;
			}
		}

		if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
			GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0); // Steal item  /  Украсть предмет
		else
			GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0); // Buy item  /  Выберите предмет для покупки

		if (item_num) { // this shoudl go into func??

			Point mouse = pMouse->GetCursorPos();
			ItemGen *item;

			int testx = (mouse.x - 30) / 70;
			
			if (testx >= 0 && testx < 6) { // testx limits check

				if (special == 0) {
					item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}
				else {
					item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}

				if (item->uItemID) { // item picking

					int testpos = ((60 - ((signed int)shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 70);

					if (mouse.x >= testpos && mouse.x < (testpos + (signed int)shop_ui_items_in_store[testx]->GetWidth())) {
						if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {

							String str;
							if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal()) {
								str = BuildDialogueString(
									pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 2)],
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							else {
								str = BuildDialogueString(
									localization->GetString(181),
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
				}
			}
		}
		else { //shop empty
			dialogwin.DrawShops_next_generation_time_string(
				pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
				);//Приходите через 7 дней
		}
	}

}

//----- (004B910F) --------------------------------------------------------
void WeaponShopDialog() {

	GUIWindow dialog_window; // [sp+7Ch] [bp-7Ch]@1
   
    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

	switch (dialog_menu_id) {

		case HOUSE_DIALOGUE_MAIN: {
			ShopDialogMain(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_STANDARD: {
			WeaponShopWares(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: {
			WeaponShopWares(dialog_window, 1);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_SELL: {
			ShopDialogSellEquip(dialog_window, BuildingType_WeaponShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_IDENTIFY: {
			ShopDialogIdentify(dialog_window, BuildingType_WeaponShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_REPAIR: {
			ShopDialogRepair(dialog_window, BuildingType_WeaponShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
			ShopDialogDisplayEquip(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_LEARN_SKILLS: {
			ShopDialogLearn(dialog_window);
			break;
		}

		default: {
			__debugbreak();
			break;
		}
	}
}

void ArmorShopWares(GUIWindow dialogwin, bool special = 0) {

	render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
	int item_x = 0;

	for (int i = 0; i < 8; ++i) {
		
		if ( (special ==  0 && pParty->StandartItemsInShops[window_SpeakInHouse->par1C][i].uItemID) ||
				( special == 1 && pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][i].uItemID )) {
			
			if (i >= 4) {  //low row
				render->DrawTextureAlphaNew( ((90 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x - 420) / 640.0f, 126 / 480.0f, shop_ui_items_in_store[i]);
			}
			else {
				render->DrawTextureAlphaNew( ((86 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x) / 640.0f, (98 - shop_ui_items_in_store[i]->GetHeight()) / 480.0f,
					shop_ui_items_in_store[i]);
			}
		}
		item_x += 105;
	}

	if (HouseUI_CheckIfPlayerCanInteract()) {

		int pItemCount = 0;
		for (int i = 0; i < 8; ++i) {
			if ( (special==0 && pParty->StandartItemsInShops[window_SpeakInHouse->par1C][i].uItemID) || (special==1 && pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][i].uItemID) )
				++pItemCount;
		}

		if (OS_IfCtrlPressed() == 0 || pPlayers[uActiveCharacter]->CanSteal() == 0)
			GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0); // Select the Item to Buy
		else
			GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0); // Steal item

		if (pItemCount) { // this should go into func??

			Point mouse = pMouse->GetCursorPos();

			int testx = (mouse.x - 40) / 105;
			// testx limits check
			if (testx >= 0 && testx < 4) {
				if (mouse.y >= 126) {
					testx += 4;
				}

				ItemGen *item;
				if (special == 0) {
					item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}
				else {
					item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}

				if (item->uItemID) {

					int testpos;
					if (testx >= 4) {
						testpos = ((90 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + (testx * 105) - 420); //low row
					}
					else {
						testpos = ((86 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 105 );
					}

					if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
						if ((mouse.y >= 126 && mouse.y < (126 + shop_ui_items_in_store[testx]->GetHeight())) ||
								(mouse.y <= 98 && mouse.y >= (98 - shop_ui_items_in_store[testx]->GetHeight()))) {
							// y is 126 to 126 + height low or 98-height to 98

							String str;
							if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal()) {
								str = BuildDialogueString(pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_ArmorShop, window_SpeakInHouse->par1C, 2)],
										uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
							}
							else {
								str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2); //"Steal %24"
							}
							dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
				}
			}
		} 
		else {
			//empty shop
			dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->par1C] - pParty->GetPlayingTime());
		}
	}
}

//----- (004BA928) --------------------------------------------------------
void ArmorShopDialog() {
 
    GUIWindow dialog_window; // [sp+8Ch] [bp-78h]@1

    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {

        case HOUSE_DIALOGUE_MAIN: {
			ShopDialogMain(dialog_window);
            break;
        }

        case HOUSE_DIALOGUE_SHOP_BUY_STANDARD: {
			ArmorShopWares(dialog_window);
			break;            
        }

		case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: {
			ArmorShopWares(dialog_window, 1);
			break;
		}
      
  		case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
			ShopDialogDisplayEquip(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_SELL: {
			ShopDialogSellEquip(dialog_window, BuildingType_ArmorShop);
			break;
		}
    
		case HOUSE_DIALOGUE_SHOP_IDENTIFY: {
			ShopDialogIdentify(dialog_window, BuildingType_ArmorShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_REPAIR: {
			ShopDialogRepair(dialog_window, BuildingType_ArmorShop);
			break;
		}

		case HOUSE_DIALOGUE_LEARN_SKILLS: {
			ShopDialogLearn(dialog_window);
			break;
		}
        
		default: {
			__debugbreak();
			break;
		}
    }
}

void AlchemyMagicShopWares(GUIWindow dialogwin, BuildingType building, bool special = 0) {

	render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);

	int itemx;
	int itemy;

	for (uint i = 0; i < 12; ++i) {

		if ((special == 0 && pParty->StandartItemsInShops[window_SpeakInHouse->par1C][i].uItemID) ||
			(special == 1 && pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][i].uItemID)) {

			if (i >= 6) {  //low row
				itemy = 308 - shop_ui_items_in_store[i]->GetHeight();
				if (itemy < 0)
					itemy = 0;

				itemx = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40 - 450;

			}
			else {
				itemy = 152 - shop_ui_items_in_store[i]->GetHeight();
				if (itemy < 0)
					itemy = 0;

				itemx = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
			}

			if (i == 0 || i == 6) {
				if (itemx < 18)
					itemx = 18;
			}

			if (i == 5 || i == 11) {
				if (itemx > 457 - shop_ui_items_in_store[i]->GetWidth())
					itemx = 457 - shop_ui_items_in_store[i]->GetWidth();
			}

			render->DrawTextureAlphaNew(itemx / 640.0f, itemy / 480.0f, shop_ui_items_in_store[i]);
		}
	}
		

	if (HouseUI_CheckIfPlayerCanInteract()) {

		int item_num = 0;

		for (uint i = 0; i < 12; ++i) {
			if (special == 0 && pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID ||
				special == 1 && pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
					++item_num;
		}

		if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
			GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);
		else
			GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0);

		if (item_num) { 

			Point mouse = pMouse->GetCursorPos();

			int testx = (mouse.x) / 75;
			// testx limits check
			if (testx >= 0 && testx < 6) {
				if (mouse.y > 152) {
					testx += 6;
				}

				ItemGen *item;
				if (special == 0) {
					item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}
				else {
					item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
				}

				if (item->uItemID) {// item picking

					int testpos;
					if (mouse.y > 152) {
						testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40 - 450;
					}
					else {
						testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40;
					}

					if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
						if ((mouse.y <= 308 && mouse.y >= (308 - shop_ui_items_in_store[testx]->GetHeight())) ||
							(mouse.y <= 152 && mouse.y >= (152 - shop_ui_items_in_store[testx]->GetHeight()))) {
							// y is 152-h to 152 or 308-height to 308

							String str;
							if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal()) {
								str = BuildDialogueString(pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, building, window_SpeakInHouse->par1C, 2)],
											uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
							}
							else {
								str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2); //"Steal %24"
							}
							dialogwin.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialogwin, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
				}
			}
		}
		else {
			//shop empty
			dialogwin.DrawShops_next_generation_time_string( pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime() );
		}
	}
}
	
//----- (004B9CC6) --------------------------------------------------------
void  AlchemistDialog() {

    GUIWindow dialog_window; // [sp+5Ch] [bp-7Ch]@1

	memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {

		case HOUSE_DIALOGUE_MAIN: {
			ShopDialogMain(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_STANDARD: {
			AlchemyMagicShopWares(dialog_window,BuildingType_AlchemistShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: {
			AlchemyMagicShopWares(dialog_window, BuildingType_AlchemistShop, 1);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_SELL: {
			ShopDialogSellEquip(dialog_window, BuildingType_AlchemistShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_IDENTIFY: {
			ShopDialogIdentify(dialog_window, BuildingType_AlchemistShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
			ShopDialogDisplayEquip(dialog_window, BuildingType_AlchemistShop);
			break;
		}

		case HOUSE_DIALOGUE_LEARN_SKILLS: {
			ShopDialogLearn(dialog_window);
			break;
		}
    
		default: {
			__debugbreak();
			break;
		}

	}
}

//----- (004B4FCF) --------------------------------------------------------
void MagicShopDialog() {
 
    GUIWindow dialog_window; // [sp+7Ch] [bp-7Ch]@1
 
    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

	switch (dialog_menu_id) {

		case HOUSE_DIALOGUE_MAIN: {
			ShopDialogMain(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_STANDARD: {
			AlchemyMagicShopWares(dialog_window, BuildingType_MagicShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: {
			AlchemyMagicShopWares(dialog_window, BuildingType_MagicShop, 1);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
			ShopDialogDisplayEquip(dialog_window);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_SELL: {
			ShopDialogSellEquip(dialog_window, BuildingType_MagicShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_IDENTIFY: {
			ShopDialogIdentify(dialog_window, BuildingType_MagicShop);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_REPAIR: {
			ShopDialogRepair(dialog_window, BuildingType_MagicShop);
			break;
		}

		case HOUSE_DIALOGUE_LEARN_SKILLS: {
			ShopDialogLearn(dialog_window);
			break;
		}

		default: {
			__debugbreak();
			break;
		}

	}
}

//----- (004BDB56) --------------------------------------------------------
void  UIShop_Buy_Identify_Repair() {

	unsigned int pItemID; // esi@20
    ItemGen *item; // esi@21
  
    int v18; // ecx@37
    float pPriceMultiplier; // ST1C_4@38
    int taken_item; // eax@40
    ItemGen *bought_item; // esi@51
    int party_reputation; // eax@55
    int v39; // eax@63
    int v42; // esi@74
    signed int v43; // ebx@74
    unsigned __int16 *pSkill; // esi@77
    int v55; // [sp+0h] [bp-B4h]@26
    int a6; // [sp+98h] [bp-1Ch]@57
    int a3; // [sp+9Ch] [bp-18h]@53
    unsigned int uNumSeconds; // [sp+A4h] [bp-10h]@53
    unsigned int invindex; // [sp+A8h] [bp-Ch]@9
    int uPriceItemService; // [sp+ACh] [bp-8h]@12

    if (current_screen_type == SCREEN_E) {
        pPlayers[uActiveCharacter]->OnInventoryLeftClick();
        return;
    }

    if (!HouseUI_CheckIfPlayerCanInteract()) {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
        return;
    }

    Point mouse = pMouse->GetCursorPos();

    switch (dialog_menu_id) {

        case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pPlayers[uActiveCharacter]->OnInventoryLeftClick();
            break;
        }

        case HOUSE_DIALOGUE_GUILD_BUY_BOOKS: {

			int testx = (mouse.x - 32) / 70;
			if (testx >= 0 && testx < 6) {
				if (mouse.y >= 250) {
					testx += 6;
				}

				bought_item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][testx];

				if (bought_item->uItemID) {

					int testpos;
					if (mouse.y >= 250) {
						testpos = 32 + 70 * testx - 420;
					}
					else {
						testpos = 32 + 70 * testx;
					}

					if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
						if ((mouse.y >= 90 && mouse.y <= (90 + shop_ui_items_in_store[testx]->GetHeight())) ||
							(mouse.y >= 250 && mouse.y <= (250 + shop_ui_items_in_store[testx]->GetHeight()))) {

							pPriceMultiplier = p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;
							uPriceItemService = pPlayers[uActiveCharacter]->GetBuyingPrice(bought_item->GetValue(), pPriceMultiplier);

							if (pParty->uNumGold < uPriceItemService) { // not enought gold
								PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
								GameUI_StatusBar_OnEvent(localization->GetString(155));
								return;
							}

							taken_item = pPlayers[uActiveCharacter]->AddItem(-1, bought_item->uItemID);
							if (taken_item) {
								bought_item->SetIdentified();
								memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[taken_item - 1], bought_item, 0x24u);
								dword_F8B1E4 = 1;
								Party::TakeGold(uPriceItemService);
								viewparams->bRedrawGameUI = 1;
								bought_item->Reset();
								render->ClearZBuffer(0, 479);
								pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_75, 0);
								return;
							}

							pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
							GameUI_StatusBar_OnEvent(localization->GetString(563));  // "Pack is Full!"
							break;
						}
					}
				}
			}
			
			return; //no item
			break;
        }

		case HOUSE_DIALOGUE_SHOP_SELL: {
			
			invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
			if (mouse.x <= 13 || mouse.x >= 462 || (pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&invindex), !pItemID) )
				return;

			if (pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1].MerchandiseTest((int)window_SpeakInHouse->ptr_1C)) {
				dword_F8B1E4 = 1;
				pPlayers[uActiveCharacter]->SalesProcess(invindex, pItemID - 1, (int)window_SpeakInHouse->ptr_1C);
				viewparams->bRedrawGameUI = 1;
				render->ClearZBuffer(0, 479);
				pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)77, 0);
				return;
			}

			pPlayers[uActiveCharacter]->PlaySound(SPEECH_79, 0);
			pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
			break;
		
		}

		case HOUSE_DIALOGUE_SHOP_IDENTIFY: {

			invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
			if (mouse.x <= 13 || mouse.x >= 462 || (pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&invindex), !pItemID))
				return;
       
			uPriceItemService = pPlayers[uActiveCharacter]->GetPriceIdentification(p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier);
			item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
            
			if (!(item->uAttributes & ITEM_IDENTIFIED)) {
				if (item->MerchandiseTest((int)window_SpeakInHouse->ptr_1C)) {
					
					if (pParty->uNumGold >= uPriceItemService) {
						dword_F8B1E4 = 1;
						Party::TakeGold(uPriceItemService);
						item->uAttributes |= ITEM_IDENTIFIED;
						pPlayers[uActiveCharacter]->PlaySound(SPEECH_73, 0);
						GameUI_StatusBar_OnEvent(localization->GetString(569));
						return;
					}
                    
					PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
                    return;
				}

				pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
                pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)79, 0);
                return;
            }

			pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)76, 0);
			break;
		}

		case HOUSE_DIALOGUE_SHOP_REPAIR: {

			invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
			if (mouse.x <= 13 || mouse.x >= 462 || (pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&invindex), !pItemID))
				return;

			item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
			pPriceMultiplier = p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;
			uPriceItemService = pPlayers[uActiveCharacter]->GetPriceRepair(item->GetValue(), pPriceMultiplier);

			if (item->uAttributes & ITEM_BROKEN) {
				if (item->MerchandiseTest((int)window_SpeakInHouse->ptr_1C)) {
					if (pParty->uNumGold >= uPriceItemService) {
						dword_F8B1E4 = 1;
						Party::TakeGold(uPriceItemService);
						item->uAttributes = (item->uAttributes & 0xFFFFFFFD) | 1;
						pPlayers[uActiveCharacter]->PlaySound(SPEECH_74, 0);
						GameUI_StatusBar_OnEvent(localization->GetString(570));
						return;
					}
				
					PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
					return;
				}

				pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
				pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)79, 0);
				return;
			}

			pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)76, 0);
			break;
		}

    case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
    case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: {

		int testx;
		int testpos;

		switch (in_current_building_type){

			case BuildingType_WeaponShop:
				
				testx = (mouse.x - 30) / 70;
				if (testx >= 0 && testx < 6) {

					if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
						bought_item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					else
						bought_item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

					if (bought_item->uItemID) {
						testpos = ((60 - ((signed int)shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 70);
						if (mouse.x >= testpos && mouse.x < (testpos + (signed int)shop_ui_items_in_store[testx]->GetWidth())) {
							if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
								break; //good
							}
						}
						else {
							bought_item = nullptr;
							return;
						}
					}
					else
						return;

				}
				break;

			case BuildingType_ArmorShop:

				testx = (mouse.x - 40) / 105;
				if (testx >= 0 && testx < 4) {
					if (mouse.y >= 126) {
						testx += 4;
					}

					if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
						bought_item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					else
						bought_item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

					if (bought_item->uItemID) {

						if (testx >= 4) {
							testpos = ((90 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + (testx * 105) - 420); //low row
						}
						else {
							testpos = ((86 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 105);
						}

						if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
							if ((mouse.y >= 126 && mouse.y < (126 + shop_ui_items_in_store[testx]->GetHeight())) ||
								(mouse.y <= 98 && mouse.y >= (98 - shop_ui_items_in_store[testx]->GetHeight()))) {
									break;//good
							}
						}
						else {
							bought_item = nullptr;
							return;
						}
					}
					else
						return;

				}
				break;

			case BuildingType_AlchemistShop:
			case BuildingType_MagicShop:

				testx = (mouse.x) / 75;
				if (testx >= 0 && testx < 6) {
					if (mouse.y > 152) {
						testx += 6;
					}
					
					if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
						bought_item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					else
						bought_item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

					if (bought_item->uItemID) {

						if (mouse.y > 152) {
							testpos =  75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40 - 450;
						}
						else {
							testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40;
						}

						if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
							if ((mouse.y <= 308 && mouse.y >= (308 - shop_ui_items_in_store[testx]->GetHeight())) ||
								(mouse.y <= 152 && mouse.y >= (152 - shop_ui_items_in_store[testx]->GetHeight()))) {
									// y is 152-h to 152 or 308-height to 308
									break; //good
							}
						}
						else {
							bought_item = nullptr;
							return;
						}
					}
					else
						return;
				}
				break;
				

			default:
				return;
		}
		
        
        

        uPriceItemService = pPlayers[uActiveCharacter]->GetBuyingPrice(bought_item->GetValue(), p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier);
        uNumSeconds = 0;
        a3 = 0;
        if (pMapStats->GetMapInfo(pCurrentMapName))
            a3 = pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]._steal_perm;
        party_reputation = pParty->GetPartyReputation();
        if (pPlayers[uActiveCharacter]->CanSteal() && OS_IfCtrlPressed())
        {
            uNumSeconds = pPlayers[uActiveCharacter]->StealFromShop(bought_item, a3, party_reputation, 0, &a6);
            if (!uNumSeconds)
            {
                sub_4B1447_party_fine((int)window_SpeakInHouse->ptr_1C, 0, a6);
                return;
            }
        }
        else if (pParty->uNumGold < uPriceItemService)
        {
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
            GameUI_StatusBar_OnEvent(localization->GetString(155));// "You don't have enough gold"
            return;
        }
        v39 = pPlayers[uActiveCharacter]->AddItem(-1, bought_item->uItemID);
        if (v39)
        {
            bought_item->SetIdentified();
            memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[v39 - 1], bought_item, sizeof(ItemGen));
            if (uNumSeconds != 0)
            {
                pPlayers[uActiveCharacter]->pInventoryItemList[v39 - 1].SetStolen();
                sub_4B1447_party_fine((int)window_SpeakInHouse->ptr_1C, uNumSeconds, a6);
            }
            else
            {
                dword_F8B1E4 = 1;
                Party::TakeGold(uPriceItemService);
            }
            viewparams->bRedrawGameUI = 1;
            bought_item->Reset();
            render->ClearZBuffer(0, 479);
            pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_75, 0);
            return;
        }
        else
        {
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
            GameUI_StatusBar_OnEvent(localization->GetString(563)); // "Pack is Full!"
            return;
        }
        break;
    }
    default:// if click video screen in shop
    {
       
        if (dialog_menu_id >= 36 && dialog_menu_id <= 72)
        {
			 __debugbreak(); // please do record these dialogue ids to the HOUSE_DIALOGUE_MENU  enum
			
			v42 = dialog_menu_id - 36;
            //v43 = (signed __int64)(*(float *)&p2DEvents_minus1__24[13 * (unsigned int)ptr_507BC0->ptr_1C] * 500.0);
            v43 = (signed __int64)(p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
            uPriceItemService = v43 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
            if (uPriceItemService < v43 / 3)
                uPriceItemService = v43 / 3;
            if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][v42])
            {
                pSkill = &pPlayers[uActiveCharacter]->pActiveSkills[v42];
                if (!*pSkill)
                {
                    if (pParty->uNumGold < uPriceItemService)
                    {
                        GameUI_StatusBar_OnEvent(localization->GetString(155));// "You don't have enough gold"
                        if (in_current_building_type == BuildingType_Training)
                            v55 = 4;
                        else
                            v55 = 2;
                        PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)v55);
                        return;
                    }
                    Party::TakeGold(uPriceItemService);
                    dword_F8B1E4 = 1;
                    *pSkill = 1;
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)78, 0);
                    return;
                }
            }
        }
        break;
    }
    }
}


//----- (004B1A2D) --------------------------------------------------------
void  ShowPopupShopItem() {

    ItemGen *item; // ecx@13
	int invindex;
	int testpos;

	if (in_current_building_type <= 0)
        return;
    if (dialog_menu_id < HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
        return;

	Point mouse = pMouse->GetCursorPos();
	int testx;

    if (in_current_building_type <= BuildingType_AlchemistShop) {
        if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD || dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL) {
            
			switch (in_current_building_type) {

				case BuildingType_WeaponShop: {

					testx = (mouse.x - 30) / 70;
					if (testx >= 0 && testx < 6) {
						if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
							item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
						else
							item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

						if (item->uItemID) {

							testpos = ((60 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 70);
							if (mouse.x >= testpos && mouse.x < (testpos + shop_ui_items_in_store[testx]->GetWidth())) {
								if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
									GameUI_DrawItemInfo(item);
								}
							}
							else {
								return;
							}
						}
					}
					else
						return;

					break;
				}

				case BuildingType_ArmorShop:

					testx = (mouse.x - 40) / 105;
					if (testx >= 0 && testx < 4) {
						if (mouse.y >= 126) {
							testx += 4;
						}

						if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
							item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
						else
							item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

						if (item->uItemID) {

							
							if (testx >= 4) {
								testpos = ((90 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + (testx * 105) - 420); //low row
							}
							else {
								testpos = ((86 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) + testx * 105);
							}

							if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
								if ((mouse.y >= 126 && mouse.y < (126 + shop_ui_items_in_store[testx]->GetHeight())) ||
									(mouse.y <= 98 && mouse.y >= (98 - shop_ui_items_in_store[testx]->GetHeight()))) {
									GameUI_DrawItemInfo(item);
								}
								else {
									return;
								}
							}
						}
						else
							return;

					}
					break;

				case BuildingType_AlchemistShop:
				case BuildingType_MagicShop:

					testx = (mouse.x) / 75;
					// testx limits check
					if (testx >= 0 && testx < 6) {
						if (mouse.y >= 152) {
							testx += 6;
						}

						if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
							item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
						else
							item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

						if (item->uItemID) {

							if (mouse.y > 152) {
								testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40 - 450;
							}
							else {
								testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40;
							}

							if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
								if ((mouse.y <= 308 && mouse.y >= (308 - shop_ui_items_in_store[testx]->GetHeight())) ||
									(mouse.y <= 152 && mouse.y >= (152 - shop_ui_items_in_store[testx]->GetHeight()))) {
									GameUI_DrawItemInfo(item);
								}
								else {
									return;
								}
							}
						}
						else
							return;

					}
					break;

			default:
				//v3 = render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF;
				//if (!v3)
			//		return;
			//	v7 = &pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][v3 - 1];
			//	if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL)
			//		v7 = &pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][v3 - 1];
			//	GameUI_DrawItemInfo(v7);
				return;
				break;
			}


			
        }

        if (dialog_menu_id >= HOUSE_DIALOGUE_SHOP_SELL && dialog_menu_id <= HOUSE_DIALOGUE_SHOP_REPAIR
            || dialog_menu_id == HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT) {

            invindex = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
            if (mouse.x <= 13 || mouse.x >= 462
                || !pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&invindex))
                return;

            GameUI_DrawItemInfo(&pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&invindex) - 1]);
            return;
        }
    }

    if (in_current_building_type <= BuildingType_16 && dialog_menu_id == HOUSE_DIALOGUE_GUILD_BUY_BOOKS) {

		int testx = (mouse.x - 32) / 70;
		if (testx >= 0 && testx < 6) {
			if (mouse.y >= 250) {
				testx += 6;
			}

			item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][testx];

			if (item->uItemID) {

				int testpos;
				if (mouse.y >= 250) {
					testpos = 32 + 70 * testx - 420;
				}
				else {
					testpos = 32 + 70 * testx;
				}

				if (mouse.x >= testpos && mouse.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
					if ((mouse.y >= 90 && mouse.y <= (90 + shop_ui_items_in_store[testx]->GetHeight())) ||
						(mouse.y >= 250 && mouse.y <= (250 + shop_ui_items_in_store[testx]->GetHeight()))) {

							unsigned int guildId = (unsigned int)window_SpeakInHouse->ptr_1C - 139;
							sub_4B1523_showSpellbookInfo(pParty->SpellBooksInGuilds[guildId][testx].uItemID);

					}
				}
			}
        }
    }
}

//----- (004B1523) --------------------------------------------------------
void sub_4B1523_showSpellbookInfo(int spellItemId)
{
    int spellId; // esi@1
    int spellLevel; // edx@1
    unsigned int v3; // eax@2
    int v4; // eax@4
    long v5; // ecx@4
    int v6; // eax@10
    char *v7; // ST44_4@12
    unsigned __int16 v8; // ax@12
    GUIWindow a1; // [sp+Ch] [bp-68h]@4
    unsigned int spellSchool; // [sp+60h] [bp-14h]@1
    int v13; // [sp+6Ch] [bp-8h]@4
    int v14; // [sp+70h] [bp-4h]@4

    spellId = spellItemId - 399;
    spellLevel = (spellItemId - 400) % 11 + 1;
    spellSchool = 4 * (spellItemId - 400) / 11;

    // sprintf(tmp_str.data(), "%s%03d", spellbook_texture_filename_suffices[v11 / 4], v2); not used

    Point a2 = pMouse->GetCursorPos();
    if (a2.y <= 320)
        v3 = a2.y + 30;
    else
        v3 = 30;
    a1.Hint = nullptr;
    a1.uFrameY = v3;
    a1.uFrameWidth = 328;
    a1.uFrameHeight = 68;
    a1.uFrameX = 90;
    a1.uFrameZ = 417;
    a1.uFrameW = v3 + 67;
    a2.y = pFontSmallnum->GetLineWidth(localization->GetString(431));
    v14 = pFontSmallnum->GetLineWidth(localization->GetString(433));
    v13 = pFontSmallnum->GetLineWidth(localization->GetString(432));
    v4 = pFontSmallnum->GetLineWidth(localization->GetString(96));
    v5 = a2.y;
    if (v14 > a2.y)
        v5 = v14;
    if (v13 > v5)
        v5 = v13;
    if (v4 > v5)
        v5 = v4;

    auto str = StringPrintf(
        "%s\n\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s",
        pSpellStats->pInfos[spellId].pDescription, localization->GetString(431),        // "Normal"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pBasicSkillDesc, localization->GetString(433),        // "Expert"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pExpertSkillDesc, localization->GetString(432),        // "Master"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pMasterSkillDesc, localization->GetString(96),         // "Grand"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pGrandmasterSkillDesc);
    v6 = pFontSmallnum->CalcTextHeight(str, &a1, 0);
    a1.uFrameHeight += v6;
    if ((signed int)a1.uFrameHeight < 150)
        a1.uFrameHeight = 150;
    a1.uFrameWidth = game_viewport_width;
    a1.DrawMessageBox(0);
    a1.uFrameWidth -= 12;
    a1.uFrameHeight -= 12;
    v7 = pSpellStats->pInfos[spellId].pName;
    a1.uFrameZ = a1.uFrameX + a1.uFrameWidth - 1;
    a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;
    v8 = Color16(0xFFu, 0xFFu, 0x9Bu);
    a1.DrawTitleText(pFontArrus, 0x78u, 0xCu, v8, v7, 3u);
    a1.DrawText(pFontSmallnum, 120, 44, 0, str, 0, 0, 0);
    a1.uFrameZ = a1.uFrameX + 107;
    a1.uFrameWidth = 108;
    a1.DrawTitleText(pFontComic, 0xCu, 0x4Bu, 0, localization->GetSkillName(spellSchool / 4 + 12), 3u);

    str = StringPrintf("%s\n%d", localization->GetString(522), *(&pSpellDatas[0].uNormalLevelMana + 10 * spellId));
    a1.DrawTitleText(pFontComic, 0xCu, a1.uFrameHeight - pFontComic->GetFontHeight() - 16, 0, str, 3);
}

//----- (004B1D27) --------------------------------------------------------
void  GetHouseGoodbyeSpeech()
{
    signed int v2; // edi@10
    signed int v5; // edi@20
    int v7[4]; // [sp+Ch] [bp-10h]@12

    if (in_current_building_type > 0)
    {
        if (in_current_building_type > BuildingType_MagicShop)
        {
            if (in_current_building_type == BuildingType_Bank)
            {
                if (!dword_F8B1E4)
                    return;
            }
            else
            {
                if (in_current_building_type != BuildingType_Temple)
                    return;
            }
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, HouseSound_Greeting_2);
            return;
        }
        if ((signed __int64)pParty->PartyTimes._shop_ban_times[(unsigned int)window_SpeakInHouse->ptr_1C] <= pParty->GetPlayingTime())
        {
            if (pParty->uNumGold <= 10000)
            {
                if (!dword_F8B1E4)
                    return;
                PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, HouseSound_Goodbye);
                return;
            }
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)(dword_F8B1E4 + 3));
            if (!dword_F8B1E4 && !_A750D8_player_speech_timer)
            {
                v5 = 0;
                for (uint i = 1; i <= 4; ++i)
                {
                    if (pPlayers[i]->CanAct())
                        v7[v5++] = i;
                }
                if (v5)
                {
                    _A750D8_player_speech_timer = 256i64;
                    PlayerSpeechID = SPEECH_80;
                    uSpeakingCharacter = v7[rand() % v5];
                    return;
                }
            }
        }
        else
        {
            if (!_A750D8_player_speech_timer)
            {
                v2 = 0;
                for (uint i = 1; i <= 4; ++i)
                {
                    if (pPlayers[i]->CanAct())
                        v7[v2++] = i;
                }
                if (v2)
                {
                    _A750D8_player_speech_timer = 256i64;
                    PlayerSpeechID = SPEECH_80;
                    uSpeakingCharacter = v7[rand() % v2];
                    return;
                }
            }
        }
    }
}

//----- (004B1447) --------------------------------------------------------
void sub_4B1447_party_fine(int shopId, int stealingResult, int fineToAdd)
{
	signed int v3; // esi@1
	DDM_DLV_Header *v7; // eax@14

	if (stealingResult == 0 || stealingResult == 1)
	{
		if (pParty->uFine < 4000000)
		{
			if (fineToAdd + pParty->uFine < 0)
				pParty->uFine = 0;
			else if (fineToAdd + pParty->uFine > 4000000)
				pParty->uFine = 4000000;
			else
				pParty->uFine += fineToAdd;
		}
		if (pParty->uFine)
		{
			for (uint i = 1; i <= 4; ++i)
			{
				if (!_449B57_test_bit(pPlayers[i]->_achieved_awards_bits, 1))
					_449B7E_toggle_bit(pPlayers[i]->_achieved_awards_bits, 1, 1);
			}
		}
		if (stealingResult == 1)
			v3 = 2;
		else
			v3 = 1;
	}
	else
		v3 = 2;
	pParty->PartyTimes._shop_ban_times[shopId] = pParty->GetPlayingTime() + GameTime::FromDays(1);
	pParty->InTheShopFlags[shopId] = 1;
	v7 = &pOutdoor->ddm;
	if (uCurrentlyLoadedLevelType != LEVEL_Outdoor)
		v7 = &pIndoor->dlv;
	v7->uReputation += v3;
	if (v7->uReputation > 10000)
		v7->uReputation = 10000;
}