#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Texts.h"

#include "GUI/GUIWindow.h"

#include "GUIButton.h"
#include "GUIWindow.h"
#include "GUIFont.h"

#include "OSWindow.h"



struct GUIButton *pBtn_CloseBook;
struct GUIButton *pBtn_InstallRemoveSpell;
struct GUIButton *pBtn_Autonotes_Instructors;
struct GUIButton *pBtn_Autonotes_Misc;
struct GUIButton *pBtn_Book_6;
struct GUIButton *pBtn_Book_5;
struct GUIButton *pBtn_Book_4;
struct GUIButton *pBtn_Book_3;
struct GUIButton *pBtn_Book_2;
struct GUIButton *pBtn_Book_1;


struct GUIButton *pPlayerCreationUI_BtnReset;
struct GUIButton *pPlayerCreationUI_BtnOK;
struct GUIButton *pBtn_ExitCancel;
struct GUIButton *pBtn_YES;
struct GUIButton *pPlayerCreationUI_BtnPlus;
struct GUIButton *pPlayerCreationUI_BtnMinus;


struct GUIButton *pButton_RestUI_Main;
struct GUIButton *pButton_RestUI_Exit;
struct GUIButton *pButton_RestUI_Wait5Minutes;
struct GUIButton *pButton_RestUI_WaitUntilDawn;
struct GUIButton *pButton_RestUI_Wait1Hour;


struct GUIButton *pCharacterScreen_ExitBtn;
struct GUIButton *pCharacterScreen_AwardsBtn;
struct GUIButton *pCharacterScreen_InventoryBtn;
struct GUIButton *pCharacterScreen_SkillsBtn;
struct GUIButton *pCharacterScreen_StatsBtn;
struct GUIButton *pCharacterScreen_DollBtn;
struct GUIButton *pCharacterScreen_DetalizBtn;


struct GUIButton *pBtn_NPCRight;
struct GUIButton *pBtn_NPCLeft;
struct GUIButton *pBtn_GameSettings;
struct GUIButton *pBtn_QuickReference;
struct GUIButton *pBtn_CastSpell;
struct GUIButton *pBtn_Rest;
struct GUIButton *pBtn_History;
struct GUIButton *pBtn_Calendar;
struct GUIButton *pBtn_Maps;
struct GUIButton *pBtn_Autonotes;
struct GUIButton *pBtn_Quests;


struct GUIButton *pMMT_MainMenu_BtnMM6;
struct GUIButton *pMMT_MainMenu_BtnMM7;
struct GUIButton *pMMT_MainMenu_BtnMM8;
struct GUIButton *pMMT_MainMenu_BtnContinue;
struct GUIButton *pMMT_MainMenu_BtnExit;


struct GUIButton *pBtn_Up;
struct GUIButton *pBtn_Down;
struct GUIButton *ptr_507BA4;


struct GUIWindow *pPrimaryWindow;
struct GUIWindow *pChestWindow;
struct GUIWindow *pDialogueWindow;
struct GUIWindow *window_SpeakInHouse;
struct GUIWindow *pGUIWindow_ScrollWindow;
struct GUIWindow *ptr_507BC8;
struct GUIWindow *pGUIWindow_CurrentMenu;
struct GUIWindow *ptr_507BD0;
struct GUIWindow *pGUIWindow_CastTargetedSpell;
struct GUIWindow *pModalWindow;
struct GUIWindow *pGUIWindow_EscMessageWindow;
struct GUIWindow *pBooksButtonOverlay;
struct GUIWindow *pGUIWindow2;


struct GUIButton *pBtn_Resume;
struct GUIButton *pBtn_QuitGame;
struct GUIButton *pBtn_GameControls;
struct GUIButton *pBtn_LoadGame;
struct GUIButton *pBtn_SaveGame;
struct GUIButton *pBtn_NewGame;

struct GUIButton *pBtn_SliderRight;
struct GUIButton *pBtn_SliderLeft;


struct GUIButton *pBtnDownArrow;
struct GUIButton *pBtnArrowUp;
struct GUIButton *pBtnCancel;
struct GUIButton *pBtnLoadSlot;


std::array<GUIButton*, 4> pCreationUI_BtnPressRight2;
std::array<GUIButton*, 4> pCreationUI_BtnPressLeft2;
std::array<GUIButton*, 4> pCreationUI_BtnPressLeft;
std::array<GUIButton*, 4> pCreationUI_BtnPressRight;






//----- (0041D0D8) --------------------------------------------------------
void GUIButton::Release()
{
  if ( this )
  {
    if ( this == this->pParent->pControlsHead )
    {
      if ( this->pNext )
      {
        this->pParent->pControlsHead = this->pNext;
        this->pNext->pPrev = 0;
      }
      else
      {
        this->pParent->pControlsHead = 0;
        this->pParent->pControlsTail = 0;
      }
    }
    else
    {
      if ( this->pNext )
      {
        this->pPrev->pNext = this->pNext;
        this->pNext->pPrev = this->pPrev;
      }
      else
      {
        this->pPrev->pNext = 0;
        this->pParent->pControlsTail = this->pPrev;
      }
    }
    --this->pParent->uNumControls;
  }
}

//----- (00415180) --------------------------------------------------------
void GUIButton::DrawLabel( const char *label_text, struct GUIFont *pFont, int a5, int uFontShadowColor )
{
  //strlen(edx0);
  return pParent->DrawText(pFont,
           this->uX + (signed int)(this->uWidth - pFont->GetLineWidth(label_text)) / 2,
           this->uY + (signed int)(this->uHeight - pFont->uFontHeight) / 2,
           a5, label_text, 0, 0, uFontShadowColor);
}
//----- (004B36CC) --------------------------------------------------------
void CreateButtonInColumn( int column_pos, unsigned int control_id )
{
  pDialogueWindow->CreateButton( 480, 30 * column_pos + 146, 140, 30,  1,  0, UIMSG_SelectShopDialogueOption,  control_id,  0,   "",   0);
}
//----- (00419379) --------------------------------------------------------
void ReleaseAwardsScrollBar()
{
  GUIButton *pButton; // esi@2

  if ( awards_scroll_bar_created )
  {
    awards_scroll_bar_created = false;
	ptr_507BA4->Release();
    pBtn_Up->Release();
    pBtn_Down->Release();
    pBtn_Down = 0;
    pBtn_Up = 0;
    for ( pButton = pGUIWindow_CurrentMenu->pControlsHead; pButton; pButton = pButton->pNext )
    {
      if ( pButton->msg == UIMSG_InventoryLeftClick )
      {
        pButton->uX = dword_50698C_uX;
        pButton->uY = dword_506988_uY;
        pButton->uZ = dword_506984_uZ;
        pButton->uW = dword_506980_uW;
        pGUIWindow_CurrentMenu->_41D08F_set_keyboard_control_group(1, 0, 0, 0);
      }
    }
  }
}
//----- (00419220) --------------------------------------------------------
void CreateAwardsScrollBar()
{
  GUIButton *pButton; // eax@2

  if ( !awards_scroll_bar_created )
  {
    awards_scroll_bar_created = 1;
    for ( pButton = pGUIWindow_CurrentMenu->pControlsHead; pButton; pButton = pButton->pNext )
    {
      if ( pButton->msg == UIMSG_InventoryLeftClick )
      {
        dword_50698C_uX = pButton->uX;
        dword_506988_uY = pButton->uY;
        dword_506984_uZ = pButton->uZ;
        dword_506980_uW = pButton->uW;
        pButton->uW = 0;
        pButton->uZ = 0;
        pButton->uY = 0;
        pButton->uX = 0;
      }
    }
    pBtn_Up = pGUIWindow_CurrentMenu->CreateButton(438, 46,
                   ui_ar_up_up->GetWidth(),
                   ui_ar_up_up->GetHeight(),
                   1, 0, UIMSG_ClickAwardsUpBtn, 0, 0, "",
                   ui_ar_up_up,
                   ui_ar_up_dn, 0);
    pBtn_Down = pGUIWindow_CurrentMenu->CreateButton(438, 292,
                   ui_ar_dn_up->GetWidth(),
                   ui_ar_dn_up->GetHeight(),
                   1, 0, UIMSG_ClickAwardsDownBtn, 0, 0, "",
                   ui_ar_dn_up,
                   ui_ar_dn_dn, 0);
    ptr_507BA4 = pGUIWindow_CurrentMenu->CreateButton(440, 62, 16, 232, 1, 0, UIMSG_ClickAwardScrollBar, 0, 0, "", 0);
  }
}
//----- (004BCA33) --------------------------------------------------------
void UI_CreateEndConversationButton()
{
  pDialogueWindow->Release();
  pDialogueWindow = new GUIWindow(0, 0, window->GetWidth(), 345, 0, 0);
  pBtn_ExitCancel = pDialogueWindow->CreateButton( 471, 445,  169, 35, 1, 0, UIMSG_Escape,  0,  0,
                 pGlobalTXT_LocalizationStrings[74],  //"End Conversation"
      ui_exit_cancel_button_background, 0);
  pDialogueWindow->CreateButton(8, 8, 450, 320, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, 0, "", 0);
}