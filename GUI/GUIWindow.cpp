#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/OurMath.h"
#include "Engine/Time.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/Autonotes.h"
#include "Engine/Awards.h"
#include "Engine/Events.h"

#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Tables/IconFrameTable.h"

#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIBooks.h"
#include "GUI/UI/UIArena.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UiStatusBar.h"

#include "Media/Audio/AudioPlayer.h"


typedef struct _RGBColor
    {
    unsigned char R;
    unsigned char B;
    unsigned char G;
    }RGBColor;


std::array<RGBColor, 20> spell_tooltip_colors={{ 
    {0x96, 0xD4, 0xFF},
    {0xFF, 0x80, 0x00},
    {0xFF, 0xFF, 0x9B},
    {0xE1, 0xE1, 0xE1},
    {0x80, 0x80, 0x80},
    {0x96, 0xD4, 0xFF},
    {0xFF, 0x55, 0x00},
    {0x96, 0xD4, 0xFF},
    {0xFF, 0x55, 0x00},
    {0xE1, 0xE1, 0xE1},
    {0xFF, 0x55, 0x00},
    {0x96, 0xD4, 0xFF},
    {0xEB, 0x0F, 0xFF},
    {0xFF, 0x80, 0x00},
    {0x96, 0xD4, 0xFF},
    {0x80, 0x80, 0x80},
    {0xFF, 0x55, 0x00},
    {0x00, 0x80, 0xFF},
    {0x00, 0x80, 0xFF},
    {0x96, 0xD4, 0xFF}}};


enum WindowType current_character_screen_window;
struct GUIWindow *pWindow_MMT_MainMenu;
struct GUIWindow *pWindow_MainMenu;
std::array<struct GUIWindow *, 50> pWindowList;

struct GUIMessageQueue *pMessageQueue_50CBD0 = new GUIMessageQueue;
struct GUIMessageQueue *pMessageQueue_50C9E8 = new GUIMessageQueue;


Image *ui_exit_cancel_button_background = nullptr;
Image *game_ui_right_panel_frame = nullptr;
Image *dialogue_ui_x_ok_u = nullptr;
Image *dialogue_ui_x_x_u = nullptr;

Image *ui_buttdesc2 = nullptr;
Image *ui_buttyes2 = nullptr;

Image *ui_btn_npc_right = nullptr;
Image *ui_btn_npc_left = nullptr;

Image *ui_ar_dn_dn = nullptr;
Image *ui_ar_dn_up = nullptr;
Image *ui_ar_up_dn = nullptr;
Image *ui_ar_up_up = nullptr;



Image *ui_leather_mm6 = nullptr;
Image *ui_leather_mm7 = nullptr;


GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
    GUIWindow(x, y, width, height, button, hint)
{
    pMouse->SetCursorImage("MICON2");
    pBtn_ExitCancel = CreateButton(392, 318, 75, 33, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(34), // Cancel    Отмена
        ui_buttdesc2, nullptr);
    GameUI_StatusBar_OnEvent(localization->GetString(39), 2); // Choose target / Выбрать цель
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    current_screen_type = SCREEN_CASTING;
}

GUIWindow_House::GUIWindow_House(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
    GUIWindow(x, y, width, height, button, hint)
{
    current_screen_type = SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(80), // Quit building / Выйти из здания
        ui_exit_cancel_button_background, 0);
    for (int v26 = 0; v26 < uNumDialogueNPCPortraits; ++v26)
    {
        const char *v29, *v30;
        if (v26 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic)
        {
            v30 = pMapStats->pInfos[uHouse_ExitPic].pName;
            v29 = localization->GetString(411); // Enter %s
        }
        else
        {
            if (v26 || !dword_591080)
                v30 = HouseNPCData[v26 + 1 - ((dword_591080 != 0) ? 1 : 0)]->pName;
            else
                v30 = p2DEvents[button - 1].pProprieterName;
            v29 = localization->GetString(435);
        }
        sprintf(byte_591180[v26].data(), v29, v30);
        HouseNPCPortraitsButtonsList[v26] = CreateButton(pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v26],
            pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v26],
            63, 73, 1, 0, UIMSG_ClickHouseNPCPortrait, v26, 0, byte_591180[v26].data(), 0, 0, 0);
    }
    if (uNumDialogueNPCPortraits == 1)
    {
        window_SpeakInHouse = this;
        _4B4224_UpdateNPCTopics(0);
    }
}

GUIWindow_Dialogue::GUIWindow_Dialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
    GUIWindow(x, y, width, height, button, hint)
{
    prev_screen_type = current_screen_type;
    current_screen_type = SCREEN_NPC_DIALOGUE;
    pBtn_ExitCancel = CreateButton(0x1D7u, 0x1BDu, 0xA9u, 0x23u, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(79), //"Exit"
        ui_exit_cancel_button_background, 0);
    if (par1C != 1)
    {
        int num_menu_buttons = 0;
        int v11 = pFontArrus->GetFontHeight() - 3;
        NPCData *speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
        if (GetGreetType(sDialogue_SpeakingActorNPC_ID) == 1)//QuestsNPC_greet
        {
            if (speakingNPC->joins)
            {
                CreateButton(480, 130, 140, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0xDu, 0, "", 0);
                num_menu_buttons = 1;
            }
            if (speakingNPC->evt_A)
            {
                if (num_menu_buttons < 4)
                {
                    int v14 = NPC_EventProcessor(speakingNPC->evt_A);
                    if (v14 == 1 || v14 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x13u, 0, "", 0);
                }
            }
            if (speakingNPC->evt_B)
            {
                if (num_menu_buttons < 4)
                {
                    int v16 = NPC_EventProcessor(speakingNPC->evt_B);
                    if (v16 == 1 || v16 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x14u, 0, "", 0);
                }
            }
            if (speakingNPC->evt_C)
            {
                if (num_menu_buttons < 4)
                {
                    int v18 = NPC_EventProcessor(speakingNPC->evt_C);
                    if (v18 == 1 || v18 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x15u, 0, "", 0);
                }
            }
            if (speakingNPC->evt_D)
            {
                if (num_menu_buttons < 4)
                {
                    int v20 = NPC_EventProcessor(speakingNPC->evt_D);
                    if (v20 == 1 || v20 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x16u, 0, "", 0);
                }
            }
            if (speakingNPC->evt_E)
            {
                if (num_menu_buttons < 4)
                {
                    int v22 = NPC_EventProcessor(speakingNPC->evt_E);
                    if (v22 == 1 || v22 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x17u, 0, "", 0);
                }
            }
            if (speakingNPC->evt_F)
            {
                if (num_menu_buttons < 4)
                {
                    int v24 = NPC_EventProcessor(speakingNPC->evt_F);
                    if (v24 == 1 || v24 == 2)
                        CreateButton(0x1E0u, num_menu_buttons++ * v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x18u, 0, "", 0);
                }
            }
        }
        else
        {
            if (speakingNPC->joins)
            {
                CreateButton(0x1E0u, 0x82u, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x4Du, 0, localization->GetString(407), 0); // Подробнее
                if (speakingNPC->Hired())
                {
                    CreateButton(
                        0x1E0u, v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x4Cu, 0,
                        localization->FormatString(408, speakingNPC->pName), // Release %s    Отпустить %s
                        0
                    );
                }
                else
                    CreateButton(0x1E0u, v11 + 130, 0x8Cu, v11, 1, 0, UIMSG_SelectNPCDialogueOption, 0x4Cu, 0, localization->GetString(406), 0); // Hire    Нанять
                num_menu_buttons = 2;
            }
        }
        _41D08F_set_keyboard_control_group(num_menu_buttons, 1, 0, 1);
    }
}


GUIWindow_GenericDialogue::GUIWindow_GenericDialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
    GUIWindow(x, y, width, height, button, hint)
{
    prev_screen_type = current_screen_type;
    pKeyActionMap->EnterText(0, 15, this);
    current_screen_type = SCREEN_BRANCHLESS_NPC_DIALOG;
}

OnCastTargetedSpell::OnCastTargetedSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int button, const char *hint) :
    GUIWindow(x, y, width, height, button, hint)
{
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pMouse->SetCursorImage("MICON2");
    GameUI_StatusBar_OnEvent(localization->GetString(39)); // Choose target / Выберите цель
}


// inlined
//----- (mm6c::00420520) --------------------------------------------------
void GUIMessageQueue::Flush()
{
  if (uNumMessages)
    uNumMessages = pMessages[0].field_8 != 0;
}

//----- (004356B9) --------------------------------------------------------
void GUIMessageQueue::PopMessage(enum UIMessageType *pType, int *pParam, int *a4)
{
  if ( this->uNumMessages )
  {
    *pType = this->pMessages[0].eType;
    *pParam = this->pMessages[0].param;
    *a4 = this->pMessages[0].field_8;
    if ( (signed int)(this->uNumMessages - 1) > 0 )
    {
      for ( uint i = 0; i < (signed int)(this->uNumMessages - 1); ++i )
      {
        this->pMessages[i].eType = this->pMessages[i + 1].eType;
        this->pMessages[i].param = this->pMessages[i + 1].param;
        this->pMessages[i].field_8 = this->pMessages[i + 1].field_8;
      }
    }
    --this->uNumMessages;
  }
}

//----- (0041B4E1) --------------------------------------------------------
void GUI_ReplaceHotkey(unsigned __int8 uOldHotkey, unsigned __int8 uNewHotkey, char bFirstCall)
{
  int i; // edx@2
  GUIButton *j; // ecx@3
  int k; // edx@7
  GUIButton *l; // ecx@8
  unsigned __int8 v9; // [sp+4h] [bp-8h]@1
  char old_hot_key; // [sp+8h] [bp-4h]@1

  //v3 = uNewHotkey;
  old_hot_key = toupper(uOldHotkey);
  v9 = toupper(uNewHotkey);
  if ( bFirstCall )
  {
    for ( i = uNumVisibleWindows; i >= 0; --i )
    {
      for ( j = pWindowList[pVisibleWindowsIdxs[i] - 1]->pControlsHead; j; j = j->pNext )
        j->field_28 = 0;
    }
  }
  for ( k = uNumVisibleWindows; k >= 0; --k )
  {
    for ( l = pWindowList[pVisibleWindowsIdxs[k] - 1]->pControlsHead; l; l = l->pNext )
    {
      if ( l->uHotkey == old_hot_key )
      {
        if ( !l->field_28 )
        {
          l->field_28 = 1;
          l->uHotkey = v9;
        }
      }
    }
  }
}

//----- (0041B438) --------------------------------------------------------
GUIButton *GUI_HandleHotkey(unsigned __int8 uHotkey)
{
  char Hot_key_num; // al@1
  GUIWindow *current_window; // ecx@2
  GUIButton *result; // eax@2

  Hot_key_num = toupper(uHotkey);
  for( int i = uNumVisibleWindows; i >= 0 && pVisibleWindowsIdxs[i] > 0; i-- )
  {
	current_window = pWindowList[pVisibleWindowsIdxs[i] - 1];
	for ( result = current_window->pControlsHead; result; result = result->pNext )
	{
	  if ( result->uHotkey == Hot_key_num )
	  {
		pMessageQueue_50CBD0->AddGUIMessage(result->msg, result->msg_param, 0);
		return result;
	  }
	}
	if ( !current_window->uFrameX && !current_window->uFrameY
		&& (current_window->uFrameWidth == window->GetWidth() && current_window->uFrameHeight == window->GetWidth()) )
	  break;
  }
  return 0;
}



//----- (0041D73D) --------------------------------------------------------
void GUIWindow::_41D73D_draw_buff_tooltip()
{
    unsigned short text_color;
    int Y_pos; // esi@11
    int string_count; // [sp+20h] [bp-4h]@7

    string_count = 0;
    for (int i = 0; i < 20; ++i)
    {
        if (pParty->pPartyBuffs[i].Active())
            ++string_count;
    }

    uFrameHeight = pFontArrus->uFontHeight + 72;
    uFrameHeight += (string_count - 1) * pFontArrus->uFontHeight;
    uFrameZ = uFrameWidth + uFrameX - 1;
    uFrameW = uFrameY + uFrameHeight - 1;
    DrawMessageBox(0);
    DrawTitleText(pFontArrus, 0, 12, 0, localization->GetString(451), 3);
    if (!string_count)
        DrawTitleText(pFontComic, 0, 40, 0, localization->GetString(153), 3);

    string_count = 0;
    for (int i = 0; i < 20; ++i)
    {
        if (pParty->pPartyBuffs[i].Active())
        {
            auto remaing_time = pParty->pPartyBuffs[i].expire_time - pParty->GetPlayingTime();
            Y_pos = string_count * pFontComic->uFontHeight + 40;
            text_color = Color16(spell_tooltip_colors[i].R, spell_tooltip_colors[i].G, spell_tooltip_colors[i].B);
            DrawText(pFontComic, 52, Y_pos, text_color, localization->GetSpellName(i), 0, 0, 0);
            DrawBuff_remaining_time_string(Y_pos, this, remaing_time, pFontComic);
            ++string_count;
        }
    }
}


//----- (0041D08F) --------------------------------------------------------
void GUIWindow::_41D08F_set_keyboard_control_group(int num_buttons, int a3, int a4, int a5)
{
  if (num_buttons)
  {
    this->pNumPresenceButton = num_buttons;
    this->field_30 = a3;
    this->field_34 = a4;
    this->pCurrentPosActiveItem = a5;
    this->pStartingPosActiveItem = a5;
    this->receives_keyboard_input = true;
  }
  else
  {
    this->pNumPresenceButton = 0;
    this->field_30 = a3;
    this->field_34 = a4;
    this->pCurrentPosActiveItem = 0;
    this->pStartingPosActiveItem = 0;
    this->receives_keyboard_input = false;
  }
}




void GUIWindow_Dialogue::Release()
{
// -----------------------------------------
// 0041C26A void GUIWindow::Release --- part
    uNumDialogueNPCPortraits = 0;

    if (game_ui_dialogue_background)
    {
        game_ui_dialogue_background->Release();  
       game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}

void GUIWindow_GenericDialogue::Release()
{
// -----------------------------------------
// 0041C26A void GUIWindow::Release --- part
    current_screen_type = prev_screen_type;
    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);

    GUIWindow::Release();
}

void GUIWindow_House::Release()
{
// -----------------------------------------
// 0041C26A void GUIWindow::Release --- part
    for (int i = 0; i < uNumDialogueNPCPortraits; ++i)
    {
        if (pDialogueNPCPortraits[i])
        {
            pDialogueNPCPortraits[i]->Release();
            pDialogueNPCPortraits[i] = nullptr;
        }
    }
    uNumDialogueNPCPortraits = 0;

    if (game_ui_dialogue_background)
    {
        game_ui_dialogue_background->Release(); 
       game_ui_dialogue_background = nullptr;
    }

    dword_5C35D4 = 0;
    if (bFlipOnExit)
    {
        pParty->sRotationY = (stru_5C6E00->uIntegerDoublePi - 1) & (stru_5C6E00->uIntegerPi + pParty->sRotationY);
        pIndoorCameraD3D->sRotationY = pParty->sRotationY;
    }
    pParty->uFlags |= 2u;

    GUIWindow::Release();
}

//----- (0041C26A) --------------------------------------------------------
void GUIWindow::Release()
{
  //GUIWindow *v1; // esi@1
//  int i; // edi@20
  //GUIButton *v8; // eax@26
  GUIButton *pNextBtn; // edi@27
  //int v10; // esi@28
  //int v11; // ecx@28
  int v12; // edx@29

  //v1 = this;
  if ( !this )
    return;

  //v8 = this->pControlsHead;
  if ( this->pControlsHead )
  {
    do
    {
      pNextBtn = this->pControlsHead->pNext;
      free(this->pControlsHead);
      this->pControlsHead = pNextBtn;
    }
    while ( pNextBtn );
  }
  this->pControlsHead = 0;
  this->pControlsTail = 0;
  this->uNumControls = 0;
  this->eWindowType = WINDOW_null;
  while ( this->numVisibleWindows < uNumVisibleWindows )
  {
    v12 = pVisibleWindowsIdxs[this->numVisibleWindows + 1];
    pVisibleWindowsIdxs[this->numVisibleWindows] = v12;
    --pWindowList[v12 - 1]->numVisibleWindows;
    ++this->numVisibleWindows;
  }
  pVisibleWindowsIdxs[uNumVisibleWindows] = 0;
  uNumVisibleWindows = uNumVisibleWindows - 1;
  //should pwindowlist[x] = nullptr;??
}

//----- (0041CD3B) --------------------------------------------------------
GUIButton *GUIWindow::GetControl(unsigned int uID)
{
  GUIButton *result; // eax@1

  result = this->pControlsHead;
  for ( uID; uID; --uID )
    result = result->pNext;
  return result;
}


//----- (00415551) --------------------------------------------------------
void GUIWindow::DrawMessageBox(bool inside_game_viewport)
{
  unsigned int v16; // esi@19
  GUIWindow current_window; // [sp+Ch] [bp-60h]@18
//  unsigned int v22; // [sp+74h] [bp+8h]@2

  int x = 0;
  int y = 0;
  int z, w;
  if (inside_game_viewport)
  {
    x = pViewport->uViewportTL_X;
    z = pViewport->uViewportBR_X;
    y = pViewport->uViewportTL_Y;
    w = pViewport->uViewportBR_Y;
  }
  else
  {
    z = window->GetWidth();
    w = window->GetHeight();
  }

  Point cursor = pMouse->GetCursorPos();
  if ( (signed int)this->uFrameX >= x )
  {
    if ( (signed int)(this->uFrameWidth + this->uFrameX) > z )
    {
      this->uFrameX = z - this->uFrameWidth;
      this->uFrameY = cursor.y + 30;
    }
  }
  else
  {
    this->uFrameX = x;
    this->uFrameY = cursor.y + 30;
  }

  if ( (signed int)this->uFrameY >= y )
  {
    if ( (signed int)(this->uFrameY + this->uFrameHeight) > w)
      this->uFrameY = cursor.y - this->uFrameHeight - 30;
  }
  else
    this->uFrameY = cursor.y + 30;
  if ( (signed int)this->uFrameY < y )
    this->uFrameY = y;
  if ( (signed int)this->uFrameX < x )
    this->uFrameX = x;
  this->uFrameZ = this->uFrameWidth + this->uFrameX - 1;
  this->uFrameW = this->uFrameHeight + this->uFrameY - 1;
  memcpy(&current_window, this, sizeof(current_window));
  current_window.uFrameX += 12;
  current_window.uFrameWidth -= 24;
  current_window.uFrameY += 12;
  current_window.uFrameHeight -= 12;
  current_window.uFrameZ = current_window.uFrameWidth + current_window.uFrameX - 1;
  current_window.uFrameW = current_window.uFrameHeight + current_window.uFrameY - 1;
  if ( this->Hint )
    v16 = pFontLucida->CalcTextHeight(this->Hint, &current_window, 0) + 24;
  else
    v16 = this->uFrameHeight;
  if ( (signed int)v16 < 64 )
    v16 = 64;
  if ( (signed int)(v16 + this->uFrameY) > 479 )
    v16 = 479 - this->uFrameY;
  DrawPopupWindow(this->uFrameX, this->uFrameY, this->uFrameWidth, v16);
  if ( this->Hint )
    current_window.DrawTitleText(pFontLucida, 0, (signed int)(v16 - pFontLucida->CalcTextHeight(this->Hint, &current_window, 0)) / 2 - 14, 0, this->Hint, 3);
}




//----- (004B3157) --------------------------------------------------------
void GUIWindow::HouseDialogManager()
{
    unsigned __int16 pWhiteColor; // di@2
    const char *pHouseName; // edx@4
    signed int v3; // edx@5
    char *v4; // edi@9
    int pTextHeight; // eax@45
    int v6; // edi@45
    int v8; // edi@46
    int v9; // eax@50
    unsigned int v10; // [sp-10h] [bp-C8h]@53
    char *pTitleText; // [sp-8h] [bp-C0h]@50
    GUIWindow pDialogWindow; // [sp+Ch] [bp-ACh]@4
    GUIWindow pWindow; // [sp+60h] [bp-58h]@2
    int pColor2; // [sp+B4h] [bp-4h]@2

    if (!window_SpeakInHouse)
        return;
    memcpy(&pWindow, this, sizeof(pWindow));
    pWindow.uFrameWidth -= 18;
    pWindow.uFrameZ -= 18;
    pWhiteColor = Color16(0xFFu, 0xFFu, 0xFFu);
    pColor2 = Color16(0x15u, 0x99u, 0xE9u);
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    if (pDialogueNPCCount != uNumDialogueNPCPortraits || !uHouse_ExitPic)
    {
        pDialogWindow.uFrameWidth = 130;
        pDialogWindow.uFrameHeight = 2 * pFontCreate->GetFontHeight();
        pHouseName = p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].pName;
        if (pHouseName)
        {
            v3 = 2 * pFontCreate->GetFontHeight() - 6 - pFontCreate->CalcTextHeight(pHouseName, &pDialogWindow, 0);
            if (v3 < 0)
                v3 = 0;
            pWindow.DrawTitleText(pFontCreate, 0x1EAu, v3 / 2 + 4, pWhiteColor,
                //(const char *)p2DEvents_minus1_::04[13 * (unsigned int)ptr_507BC0->ptr_1C],
                p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].pName, 3);
        }
    }
    pWindow.uFrameWidth += 8;
    pWindow.uFrameZ += 8;
    if (!pDialogueNPCCount)
    {
        if (in_current_building_type == BuildingType_Jail)
        {
            JailDialog();
            if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic)
            {
                render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
                render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
            }
            else
                render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
            return;
        }
        if (!current_npc_text.empty())
        {
            pDialogWindow.uFrameWidth = 458;
            pDialogWindow.uFrameZ = 457;
            pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, &pDialogWindow, 13);
            v6 = pTextHeight + 7;
            render->DrawTextureCustomHeight(
                8 / 640.0f,
                (352 - (pTextHeight + 7)) / 480.0f,
                ui_leather_mm7,
                pTextHeight + 7);
            render->DrawTextureAlphaNew(8 / 640.0f, (347 - v6) / 480.0f, _591428_endcap);
            window_SpeakInHouse->DrawText(pFontArrus, 13, 354 - v6, 0, FitTextInAWindow(current_npc_text, pFontArrus, &pDialogWindow, 13), 0, 0, 0);
        }
        if (uNumDialogueNPCPortraits <= 0)
        {
            if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic)
            {
                render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
                render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
            }
            else
                render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
            return;
        }
        for (v8 = 0; v8 < uNumDialogueNPCPortraits; ++v8)
        {
            render->DrawTextureAlphaNew((pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] - 4) / 640.0f,
                (pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] - 4) / 480.0f, game_ui_evtnpc);
            render->DrawTextureAlphaNew(pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] / 640.0f,
                pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] / 480.0f, pDialogueNPCPortraits[v8]);
            if (uNumDialogueNPCPortraits < 4)
            {
                if (v8 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic)
                {
                    pTitleText = pMapStats->pInfos[uHouse_ExitPic].pName;
                    v9 = 94 * v8 + 113;
                }
                else
                {
                    if (!v8 && dword_591080)
                    {
                        pTitleText = (char *)p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].pProprieterTitle;
                        pWindow.DrawTitleText(pFontCreate, 0x1E3u, 113, pColor2, pTitleText, 3);
                        continue;
                    }
                    pTitleText = HouseNPCData[v8 + 1 - (dword_591080 != 0)]->pName;
                    v9 = pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] + pDialogueNPCPortraits[v8]->GetHeight() + 2;
                }
                v10 = v9;
                pWindow.DrawTitleText(pFontCreate, 483, v10, pColor2, pTitleText, 3);
            }
        }
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic)
        {
            render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
            render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
        }
        else
            render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
        return;
    }
    v4 = (char *)pDialogueNPCCount - 1;
    render->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureAlphaNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, pDialogueNPCPortraits[(signed int)v4]);
    if (current_screen_type == SCREEN_E)
    {
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic)
        {
            render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
            render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
        }
        else
            render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
        return;
    }
    if (v4 || !dword_591080)// emerald isle ship before quest's done   /   на изумрудном острове заходит на корабле пока не выполнены квесты
        SimpleHouseDialog();
    else
    {
        pWindow.DrawTitleText(
            pFontCreate, 0x1E3u, 0x71u, pColor2,
            localization->FormatString(
                429,
                p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].pProprieterName,
                p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].pProprieterTitle
                ),
            3
            );
        switch (in_current_building_type)
        {
        case BuildingType_WeaponShop:
            WeaponShopDialog();
            break;
        case BuildingType_ArmorShop:
            ArmorShopDialog();
            break;
        case BuildingType_MagicShop:
            MagicShopDialog();
            break;
        case BuildingType_AlchemistShop:
            AlchemistDialog();
            break;
        case BuildingType_FireGuild:
        case BuildingType_AirGuild:
        case BuildingType_WaterGuild:
        case BuildingType_EarthGuild:
        case BuildingType_SpiritGuild:
        case BuildingType_MindGuild:
        case BuildingType_BodyGuild:
        case BuildingType_LightGuild:
        case BuildingType_DarkGuild:
            GuildDialog();
            break;
        case BuildingType_18:
            __debugbreak(); //What over the dialog?
            sub_4B6478();
            break;
        case BuildingType_TownHall:
            TownHallDialog();
            break;
        case BuildingType_Tavern:
            TavernDialog();
            break;
        case BuildingType_Bank:
            BankDialog();
            break;
        case BuildingType_Temple:
            TempleDialog();
            break;
        case BuildingType_Stables:
        case BuildingType_Boats:
            TravelByTransport();
            break;
        case BuildingType_Training:
           // __debugbreak(); // param was passed via pTmpBuf, investiage ??
			// no idea why this could pass an argument - its always reset
            TrainingDialog("");
            break;
        case BuildingType_Jail:
            JailDialog();
            break;
        default:
            //__debugbreak();//New BuildingType (if enter Boat)
            break;
        }
    }
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic)
    {
        render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
        render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    }
    else
        render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
}


String MakeDateTimeString(GameTime time)
{
    int seconds = time.GetSecondsFraction();
    int minutes = time.GetMinutesFraction();
    int hours = time.GetHoursOfDay();
    int days = time.GetDays();

    String str = "";
    if (days)
    {
        auto day_str = localization->GetString(57); // Days
        if (days <= 1)
            day_str = localization->GetString(56); // Day

        str += StringPrintf("%d %s ", days, day_str);
    }

    if (hours)
    {
        auto hour_str = localization->GetString(110);// Hours
        if (hours <= 1)
            hour_str = localization->GetString(109);// Hour

        str += StringPrintf("%d %s ", hours, hour_str);
    }

    if (minutes && !days)
    {
        auto minute_str = localization->GetString(436); // Minutes
        if (minutes <= 1)
            minute_str = localization->GetString(437);// Minute

        str += StringPrintf("%d %s ", minutes, minute_str);
    }

    if (seconds && !hours)
    {
        auto seconds_str = localization->GetString(438);// Seconds
        if (seconds <= 1)
            seconds_str = localization->GetString(439);// Second

        str += StringPrintf("%d %s ", seconds, seconds_str);
    }

    return str;
}

//----- (004B1854) --------------------------------------------------------
void GUIWindow::DrawShops_next_generation_time_string(GameTime time)
{
    auto str = MakeDateTimeString(time);
    this->DrawTitleText(
        pFontArrus,
        0,
        (212 - pFontArrus->CalcTextHeight(str, this, 0)) / 2 + 101,
        Color16(0xFFu, 0xFFu, 0x9Bu),
        localization->GetString(532) + str,
        3
    );
}


void GUIWindow::DrawTitleText(GUIFont *font, int horizontal_margin, int vertical_margin, unsigned __int16 uDefaultColor, const String &str, int line_spacing)
{
    this->DrawTitleText(font, horizontal_margin, vertical_margin, uDefaultColor, str.c_str(), line_spacing);
}

//----- (0044D406) --------------------------------------------------------
void GUIWindow::DrawTitleText(GUIFont *a2, int uHorizontalMargin, int uVerticalMargin, unsigned __int16 uDefaultColor,
    const char *pInString, int uLineSpacing)
{
    //GUIWindow *pWindow; // esi@1
    unsigned int v8; // ebx@1
    char *v9; // eax@1
    unsigned int v11; // edi@1
    signed int v12; // esi@1
    int v13; // eax@2
    GUIFont *pFont; // [sp+Ch] [bp-4h]@1
    const char *Stra; // [sp+24h] [bp+14h]@5

    //pWindow = this;
    pFont = a2;
    v8 = this->uFrameWidth - uHorizontalMargin;
    ui_current_text_color = uDefaultColor;
    v9 = FitTextInAWindow(pInString, a2, this, uHorizontalMargin);
    Stra = strtok(v9, "\n");
    v11 = uHorizontalMargin + this->uFrameX;
    v12 = uVerticalMargin + this->uFrameY;
    while (1)
    {
        if (!Stra)
            break;
        v13 = (signed int)(v8 - pFont->GetLineWidth(Stra)) >> 1;
        if (v13 < 0)
            v13 = 0;
        pFont->DrawTextLine(uDefaultColor, v11 + v13, v12, Stra, window->GetWidth());
        v12 += pFont->uFontHeight - uLineSpacing;
        Stra = strtok(0, "\n");
    }
}


void GUIWindow::DrawText(GUIFont *font, int x, int y, unsigned short uFontColor, const String &str, bool present_time_transparency, int max_text_height, int uFontShadowColor)
{
    this->DrawText(font, x, y, uFontColor, str.c_str(), present_time_transparency, max_text_height, uFontShadowColor);
}

//----- (0044CE08) --------------------------------------------------------
void GUIWindow::DrawText(GUIFont *font, int uX, int uY, unsigned short uFontColor, const char *Str, bool present_time_transparency, int max_text_height, int uFontShadowColor)
{
  int v14; // edx@9
  char Dest[6]; // [sp+Ch] [bp-2Ch]@32
  size_t v30; // [sp+2Ch] [bp-Ch]@4

  int left_margin = 0;
  if ( !Str )
  {
    logger->Warning(L"Invalid string passed!");
    return;
  }
  if (!strcmp(Str, "null"))
    return;

  v30 = strlen(Str);
  if ( !uX )
    uX = 12;

  const char *string_begin = Str;
  if ( max_text_height == 0 )
    string_begin = FitTextInAWindow(Str, font, this, uX);
  auto string_end = string_begin;
  auto string_base = string_begin;

  int out_x = uX + uFrameX;
  int out_y = uY + uFrameY;
  v14 = 0;

  if (max_text_height != 0 && out_y + font->GetFontHeight() > max_text_height)
    return;

  if ( (signed int)v30 > 0 )
  {
    do
    {
      unsigned char c = string_base[v14];
      if ( c >= font->cFirstChar && c <= font->cLastChar
        || c == '\f'
        || c == '\r'
        || c == '\t'
        || c == '\n' )
      {
        switch ( c )
        {
          case '\t':
            strncpy(Dest, &string_base[v14 + 1], 3);
            Dest[3] = 0;
            v14 += 3;
            left_margin = atoi(Dest);
            out_x = uX + uFrameX + left_margin;
            break;
          case '\n':
            uY = uY + font->GetFontHeight() - 3;
            out_y = uY + uFrameY;
            out_x = uX + uFrameX + left_margin;
            if ( max_text_height != 0 )
            {
              if (font->GetFontHeight() + out_y - 3 > max_text_height )
                return;
            }
            break;
          case '\f':
            strncpy(Dest, &string_base[v14 + 1], 5);
            Dest[5] = 0;
            uFontColor = atoi(Dest);
            v14 += 5;
            break;
          case '\r':
            strncpy(Dest, &string_base[v14 + 1], 3);
            Dest[3] = 0;
            v14 += 3;
            left_margin = atoi(Dest);
            out_x = uFrameZ - font->GetLineWidth(&string_base[v14]) - left_margin;
            out_y = uY + uFrameY;
            if ( max_text_height != 0 )
            {
              if (font->GetFontHeight() + out_y - 3 > max_text_height )
                return;
              break;
            }
            break;

          default:
            if (c == '\"' && string_base[v14 + 1] == '\"')
              ++v14;
                
            c = (unsigned __int8)string_base[v14];
            if ( v14 > 0 )
              out_x += font->pMetrics[c].uLeftSpacing;

            unsigned char *letter_pixels = &font->pFontData[font->font_pixels_offset[c]];
            if ( uFontColor )
              render->DrawText(out_x, out_y, letter_pixels, font->pMetrics[c].uWidth, font->GetFontHeight(),
                  font->pFontPalettes[0], uFontColor, uFontShadowColor);
            else
              render->DrawTextAlpha(out_x, out_y, letter_pixels, font->pMetrics[c].uWidth, font->GetFontHeight(),
                  font->pFontPalettes[0], present_time_transparency);

            out_x += font->pMetrics[c].uWidth;
            if ( (signed int)v14 < (signed int)v30 )
              out_x += font->pMetrics[c].uRightSpacing;
            break;
          }
        }
      }
      while ( (signed int)++v14 < (signed int)v30 );
    }
}


int GUIWindow::DrawTextInRect(GUIFont *font, unsigned int x, unsigned int y, unsigned int color, const char *text, int rect_width, int reverse_text)
{
    return DrawTextInRect(font, x, y, color, String(text), rect_width, reverse_text);
}

//----- (0044CB4F) --------------------------------------------------------
int GUIWindow::DrawTextInRect(GUIFont *pFont, unsigned int uX, unsigned int uY, unsigned int uColor, String &str, int rect_width, int reverse_text)
{
  int pLineWidth; // ebx@1
  int text_width; // esi@3
  unsigned __int8 v12; // cl@7
  signed int v13; // esi@19
  signed int v14; // ebx@19
  unsigned __int8 v15; // cl@21
//  int v16; // eax@22
//  int v17; // ecx@22
//  int v18; // ecx@23
//  int v19; // ecx@24
  unsigned int v20; // ecx@26
  unsigned char* v21; // eax@28
//  int v22; // ebx@34
  int v23; // eax@34
  int v24; // ebx@36
  char Str[6]; // [sp+Ch] [bp-20h]@34
//  char v26; // [sp+Fh] [bp-1Dh]@34
//  char v27; // [sp+11h] [bp-1Bh]@35
  int v28; // [sp+20h] [bp-Ch]@17
  GUIWindow *pWindow; // [sp+24h] [bp-8h]@1
  size_t pNumLen; // [sp+28h] [bp-4h]@1
  size_t Str1a; // [sp+40h] [bp+14h]@5
//  size_t Str1b; // [sp+40h] [bp+14h]@19
//  const char *Sourcea; // [sp+44h] [bp+18h]@20
//  int v34; // [sp+48h] [bp+1Ch]@26
  int i;

  char text[4096];
  Assert(str.length() < sizeof(text));
  strcpy(text, str.c_str());

  pWindow = this;
  pNumLen = strlen(text);
  pLineWidth = pFont->GetLineWidth(text);
  if ( pLineWidth < rect_width )
  {
    pWindow->DrawText(pFont, uX, uY, uColor, text, 0, 0, 0);
    return pLineWidth;
  }

  text_width = 0;
  if ( reverse_text )
    _strrev(text);
  Str1a = 0;
  for ( i = 0; i < pNumLen; ++i )
    {
      if ( text_width >= rect_width )
        break;
      v12 = text[i];
      if ( pFont->IsCharValid(v12) )
      {
      switch (v12)
          {
      case '\t':// Horizontal tab 09
      case '\n': //Line Feed 0A 10
      case '\r': //Form Feed, page eject  0C 12
          break;
      case '\f': //Carriage Return 0D 13
          i += 5;	  
          break;
      default:
          if ( i > 0 )
            text_width += pFont->pMetrics[v12].uLeftSpacing;
          text_width += pFont->pMetrics[v12].uWidth;
          if ( i < pNumLen )
              text_width += pFont->pMetrics[v12].uRightSpacing;
          }
      }
    }
  text[i - 1] = 0;


  pNumLen = strlen(text);
  v28 = pFont->GetLineWidth(text);
  if ( reverse_text )
    _strrev(text);

  v13 = uX + pWindow->uFrameX;
  v14 = uY + pWindow->uFrameY;
  for (i=0; i<pNumLen; ++i)
  {
      v15 = text[i];
      if ( pFont->IsCharValid(v15) )
      {
      switch (v12)
          {
      case '\t':// Horizontal tab 09
          {
          strncpy(Str,  &text[i+1], 3);
          Str[3] = 0;
       //   atoi(Str);
          i += 3;
          break;
          }
      case '\n': //Line Feed 0A 10
          {
          v24 = pFont->uFontHeight;
          v13 = uX;
          uY = uY + pFont->uFontHeight - 3;
          v14 = uY+pFont->uFontHeight - 3;
          break;
          }
      case '\r': //Form Feed, page eject  0C 12
          {
          strncpy(Str, &text[i+1], 5);
          Str[5] = 0;
          i += 5;
          uColor = atoi(Str);
          break;
          }
      case '\f': //Carriage Return 0D 13
          {
          strncpy(Str, &text[i+1], 3);
          Str[3] = 0;
          i += 3;
          v23 = pFont->GetLineWidth(&text[i]);
          v13 = pWindow->uFrameZ - v23 - atoi(Str);
          v14 = uY;
          break;
          }
      default:
          v20 = pFont->pMetrics[v15].uWidth;
          if ( i > 0 )
              v13 += pFont->pMetrics[v15].uLeftSpacing;
          v21 = &pFont->pFontData[pFont->font_pixels_offset[v15]];
          if ( uColor )
              render->DrawText(v13, v14,  v21, v20, pFont->uFontHeight, pFont->pFontPalettes[0], uColor, 0);
          else
              render->DrawTextAlpha(v13, v14, v21, v20, pFont->uFontHeight, pFont->pFontPalettes[0], false);
          v13 += v20;
          if ( i < (signed int)pNumLen )
              v13 += pFont->pMetrics[v15].uRightSpacing;
          }
      }
  }
  return v28;
}


GUIButton *GUIWindow::CreateButton(
    int x, int y, int width, int height, int a6, int a7, UIMessageType msg,
    unsigned int msg_param, unsigned __int8 hotkey, const String &label, Image *textures, ...
)
{
    GUIButton *res = nullptr;

    va_list textures_vararg;
    va_start(textures_vararg, textures);
    {
        res = this->CreateButtonInternal(x, y, width, height, a6, a7, msg, msg_param, hotkey, label, textures_vararg - sizeof(textures));
    }
    va_end(textures_vararg);

    return res;
}

GUIButton *GUIWindow::CreateButton(
    int x, int y, int width, int height, int a6, int a7, UIMessageType msg,
    unsigned int msg_param, unsigned __int8 hotkey, const char *label, Image *textures, ...
)
{
    GUIButton *res = nullptr;

    va_list textures_vararg;
    va_start(textures_vararg, label);
    {
        res = this->CreateButtonInternal(x, y, width, height, a6, a7, msg, msg_param, hotkey, String(label), textures_vararg);
    }
    va_end(textures_vararg);

    return res;
}

//----- (0041D12F) --------------------------------------------------------
GUIButton *GUIWindow::CreateButtonInternal(
    int uX, int uY, int uWidth, int uHeight, int a6, int a7, UIMessageType msg,
    unsigned int msg_param, unsigned __int8 uHotkey, const String &label, va_list textures
)
{
  auto pButton = new GUIButton();

  for (unsigned int i = 0; i < 5; ++i)
      pButton->pTextures[i] = nullptr;

  pButton->pParent = this;
  pButton->uWidth = uWidth;
  pButton->uHeight = uHeight;
  
  if ( a6 == 2 && !uHeight )
    pButton->uHeight = uWidth;

  pButton->uButtonType = a6;
  pButton->uX = uX + this->uFrameX;
  pButton->uY = uY + this->uFrameY;
  pButton->uZ = pButton->uX + uWidth - 1;
  pButton->uW = pButton->uY + uHeight - 1;
  pButton->field_2C_is_pushed = 0;
  pButton->field_1C = a7;
  pButton->msg = msg;
  pButton->msg_param = msg_param;
  pButton->uHotkey = uHotkey;

  pButton->button_name = label;
  strcpy(pButton->pButtonName, pButton->button_name.c_str());

  pButton->uNumTextures = 0;

  Image *img = nullptr;
  do
  {
      img = va_arg(textures, Image *);
      pButton->pTextures[pButton->uNumTextures++] = img;
  } while (img);

  if ( this->pControlsTail )
    this->pControlsTail->pNext = pButton;
  else
    this->pControlsHead = pButton;
  pButton->pPrev = this->pControlsTail;
  this->pControlsTail = pButton;
  pButton->pNext = 0;
  ++this->uNumControls;
  return pButton;
}


void GUIWindow::InitializeGUI()
{
    SetUserInterface(PartyAlignment_Neutral, false);

    for (uint i = 0; i < 20; ++i) //should this be 50??
        pWindowList[i] = nullptr;
    uNumVisibleWindows = -1;
    memset(pVisibleWindowsIdxs.data(), 0, sizeof(pVisibleWindowsIdxs));

    MainMenuUI_LoadFontsAndSomeStuff();
}

//----- (00459C2B) --------------------------------------------------------
void GUIWindow::DrawFlashingInputCursor( signed int uX, int uY, struct GUIFont *a2 )
{
  if ( OS_GetTime() % 1000 > 500 )
    DrawText(a2, uX, uY, 0, "_", 0, 0, 0);
}


GUIWindow::GUIWindow() :
    uNumControls(0),
    pControlsHead(nullptr),
    pControlsTail(nullptr),
    eWindowType(WINDOW_null)
{}

//----- (0041C432) --------------------------------------------------------
GUIWindow::GUIWindow(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, int pButton, const char* hint) :
    uNumControls(0),
    pControlsHead(nullptr),
    pControlsTail(nullptr),
    eWindowType(WINDOW_MainMenu)
{
    unsigned int uNextFreeWindowID; // ebp@1
    //int *v8; // eax@1
    //GUIWindow *pWindow; // esi@4
    //int v10; // eax@4
//    unsigned int v11; // ebx@15
//    NPCData *speakingNPC; // ebp@15
//    int v14; // eax@20
//    int v16; // eax@25
//    int v18; // eax@30
//    int v20; // eax@35
//    int v22; // eax@40
//    int v24; // eax@45
    //  int v25; // eax@65
//    unsigned int v26; // ebx@65
//    char *v27; // eax@71
//    const char *v29; // [sp-8h] [bp-18h]@68
//    char *v30; // [sp-4h] [bp-14h]@68
    //  int uWidtha; // [sp+14h] [bp+4h]@66
//    int num_menu_buttons; // [sp+20h] [bp+10h]@15

    for (uNextFreeWindowID = 0; uNextFreeWindowID < 20; ++uNextFreeWindowID) // should this limit be 50  as pwindowlist is size 50??
    {
        if (pWindowList[uNextFreeWindowID] == nullptr || (pWindowList[uNextFreeWindowID]->eWindowType== WINDOW_null)) // ??testy test
            break;
    }

	logger->Info(L"New window");

	//pwindowlist not freeing/resetting properly?? above is work around
	Assert(pWindowList[uNextFreeWindowID] == nullptr || (pWindowList[uNextFreeWindowID]->eWindowType == WINDOW_null), "Window out of range!");

    //GUIWindow* pWindow = &pWindowList[uNextFreeWindowID];
    pWindowList[uNextFreeWindowID] = this;//sometimes uNextFreeWindowID == 20. it's result crash
    this->uFrameWidth = uWidth;
    this->uFrameHeight = uHeight;

    this->uFrameX = uX;
    this->uFrameY = uY;
    this->uFrameZ = uX + uWidth - 1;
    this->uFrameW = uY + uHeight - 1;

    this->ptr_1C = (void *)pButton;
    this->Hint = hint;

    //this->eWindowType = eWindowType;
    this->receives_keyboard_input = false;
    ++uNumVisibleWindows;
    this->numVisibleWindows = uNumVisibleWindows;
    pVisibleWindowsIdxs[uNumVisibleWindows] = uNextFreeWindowID + 1;
}


//----- (004B3EF0) --------------------------------------------------------
void DrawJoinGuildWindow( int pEventCode )
{
  uDialogueType = 81;//enum JoinGuildDialog
  current_npc_text = (char *)pNPCTopics[pEventCode + 99].pText;
  ContractSelectText(pEventCode);
  pDialogueWindow->Release();
  pDialogueWindow = new GUIWindow(0, 0, window->GetWidth(), 350, pEventCode, 0);
  pBtn_ExitCancel = pDialogueWindow->CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape,                    0, 0, localization->GetString(34), ui_exit_cancel_button_background, 0); // Cancel
                    pDialogueWindow->CreateButton(  0,   0,   0,  0, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, 0, "", 0);
                    pDialogueWindow->CreateButton(480, 160, 140, 30, 1, 0, UIMSG_ClickNPCTopic,             82, 0, localization->GetString(122), 0);
  pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
  dialog_menu_id = HOUSE_DIALOGUE_OTHER;
}
//----- (0044603D) --------------------------------------------------------
void DialogueEnding()
{
  sDialogue_SpeakingActorNPC_ID = 0;
  if (pDialogueWindow)
		pDialogueWindow->Release();
  pDialogueWindow = 0;
  pMiscTimer->Resume();
  pEventTimer->Resume();
}


void GUIWindow_BooksButtonOverlay::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameY/640.0f, uFrameX/480.0f, pButton->pTextures[0]);
    viewparams->bRedrawGameUI = true;
}

void GUIWindow_Dialogue::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    GameUI_DrawDialogue();
}

void GUIWindow_GenericDialogue::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    GameUI_DrawBranchlessDialogue();
}

void GUIWindow_House::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    HouseDialogManager();
    if (!window_SpeakInHouse)
        return;
    if (window_SpeakInHouse->par1C >= 53)
        return;
    if (pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->par1C] <= pParty->GetPlayingTime())
    {
        if (window_SpeakInHouse->par1C < 53)
            pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->par1C] = 0;
        return;
    }
	//dialog_menu_id = HOUSE_DIALOGUE_MAIN; 
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0); // banned from shop so leaving
}

void GUIWindow_Scroll::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    CreateScrollWindow();
}

void GUIWindow_Inventory::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    DrawMessageBox(0);
    DrawText(pFontLucida, 10, 20, 0, "Making item number", 0, 0, 0);
    DrawText(pFontLucida, 10, 40, 0, pKeyActionMap->pPressedKeysBuffer, 0, 0, 0);
    if (!pKeyActionMap->field_204)
    {
        ItemGen ItemGen2;
        ItemGen2.Reset();
        Release();
        pEventTimer->Resume();
        current_screen_type = SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
        int v39 = atoi(pKeyActionMap->pPressedKeysBuffer);
        if (v39 > 0 && v39 < 800)
            SpawnActor(v39);
    }
}

void GUIWindow_Inventory_CastSpell::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    render->ClearZBuffer(0, 479);
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
    CharacterUI_DrawPaperdoll(pPlayers[uActiveCharacter]);
    render->DrawTextureAlphaNew(pBtn_ExitCancel->uX/640.0f, pBtn_ExitCancel->uY/480.0f, dialogue_ui_x_x_u);
}

void OnButtonClick::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    GUIButton *pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();
}

void OnButtonClick2::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    GUIButton *pButton = (GUIButton *)ptr_1C;
    if (pButton->uX >= 0 && pButton->uX <= window->GetWidth())
    {
        if (pButton->uY >= 0 && pButton->uY <= window->GetHeight())
        {
            render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[0]);
            viewparams->bRedrawGameUI = true;
            if (Hint && Hint != (char *)1)
                pButton->DrawLabel(Hint, pFontCreate, 0, 0);
            Release();
            return;
        }
    }
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();
}

void OnButtonClick3::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[1]);
    viewparams->bRedrawGameUI = 1;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();
}

void OnButtonClick4::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[1]);
    viewparams->bRedrawGameUI = true;

    Release();
}

void OnSaveLoad::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();

    if (current_screen_type == SCREEN_SAVEGAME)
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    else
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
}

void OnCancel::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    auto pGUIButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pGUIButton->pTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pGUIButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel2::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[1]);
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel3::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    if (Hint != (char *)1)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);

    auto pButton = (GUIButton *)ptr_1C;
    render->DrawTextureAlphaNew(uFrameX/640.0f, uFrameY/480.0f, pButton->pTextures[0]);
    viewparams->bRedrawGameUI = true;
    if (Hint && Hint != (char *)1)
        pButton->DrawLabel(Hint, pFontCreate, 0, 0);
    Release();

    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
}

//----- (004156F0) --------------------------------------------------------
void GUI_UpdateWindows()
{
    GUIWindow *pWindow; // esi@4
//    const char *pHint; // edx@66
//    int v26; // eax@98
//    unsigned int v27; // ebp@106
//    GUIWindow *pGUIWindow2; // ecx@109
//    int v31; // eax@115
//    GUIButton *pButton; // ebp@118
//    int v39; // eax@129
//    GUIButton *pGUIButton; // ebp@146
    GUIButton GUIButton2; // [sp+28h] [bp-E0h]@133
    ItemGen ItemGen2; // [sp+E4h] [bp-24h]@129

    if (GetCurrentMenuID() != MENU_CREATEPARTY)
        Mouse::UI_OnKeyDown(VK_NEXT);

    for (unsigned int i = 1; i <= uNumVisibleWindows; ++i)
    {
		pWindow = pWindowList[pVisibleWindowsIdxs[i] - 1]; //pVisibleWindowsIdxs[i] = 21 then problems //enchanting window problems visiblewindows -1??
        pWindow->Update();
    }

    if (GetCurrentMenuID() == -1)
        GameUI_DrawFoodAndGold();
    if (sub_4637E0_is_there_popup_onscreen())
        UI_OnMouseRightClick(0);
}


//----- (00467FB6) --------------------------------------------------------
void CreateScrollWindow()
{
  unsigned int v0; // eax@1
  char *v1; // ST18_4@3
  GUIWindow a1; // [sp+Ch] [bp-54h]@1

  memcpy(&a1, pGUIWindow_ScrollWindow, sizeof(a1));
  a1.Hint = 0;
  a1.uFrameX = 1;
  a1.uFrameY = 1;
  a1.uFrameWidth = 468;
  v0 = pFontSmallnum->CalcTextHeight(pScrolls[pGUIWindow_ScrollWindow->par1C], &a1, 0) + 2 * (unsigned char)pFontCreate->uFontHeight + 24;
  a1.uFrameHeight = v0;
  if ( (signed int)(v0 + a1.uFrameY) > 479 )
  {
    v0 = 479 - a1.uFrameY;
    a1.uFrameHeight = 479 - a1.uFrameY;
  }
  a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
  a1.uFrameW = v0 + a1.uFrameY - 1;
  a1.DrawMessageBox(0);
  a1.uFrameX += 12;
  a1.uFrameWidth -= 24;
  a1.uFrameY += 12;
  a1.uFrameHeight -= 12;
  a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
  a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;
  v1 = pItemsTable->pItems[(unsigned int)pGUIWindow_ScrollWindow->ptr_1C + 700].pName;

  a1.DrawTitleText(pFontCreate, 0, 0, 0, StringPrintf(format_4E2D80, Color16(0xFFu, 0xFFu, 0x9Bu), v1), 3);
  a1.DrawText(pFontSmallnum, 1, (unsigned char)pFontCreate->uFontHeight - 3, 0,
              pScrolls[(unsigned int)pGUIWindow_ScrollWindow->ptr_1C], 0, 0, 0);
}
//----- (00467F48) --------------------------------------------------------
void CreateMsgScrollWindow(signed int mscroll_id)
{
    if (!pGUIWindow_ScrollWindow && mscroll_id >= 700)
    {
        if (mscroll_id <= 782)
        {
            pGUIWindow_ScrollWindow = new GUIWindow_Scroll(0, 0, window->GetWidth(), window->GetHeight(), mscroll_id - 700, 0);
        }
    }
}

//----- (00467F9F) --------------------------------------------------------
void free_book_subwindow()
{
  if ( pGUIWindow_ScrollWindow )
  {
    pGUIWindow_ScrollWindow->Release();
    pGUIWindow_ScrollWindow = 0;
  }
}
//----- (004226EF) --------------------------------------------------------
void SetUserInterface(PartyAlignment align, bool bReplace)
{
    extern void set_default_ui_skin();
    set_default_ui_skin();


    if (!parchment)
    {
        parchment = assets->GetImage_16BitColorKey("parchment", 0x7FF);
    }

    if (align == PartyAlignment_Evil)
    {
        if (bReplace)
        {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe = assets->GetImage_PCXFromIconsLOD("ib-b-C.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar = assets->GetImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->GetImage_16BitAlpha("ib-mb-C");

            game_ui_minimap_frame = assets->GetImage_16BitAlpha("ib-autmask-c");
            game_ui_minimap_compass = assets->GetImage_16BitColorKey("IB-COMP-C", 0x7FF);

            game_ui_player_alert_green = assets->GetImage_16BitAlpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->GetImage_16BitAlpha("IB-InitY-c");
            game_ui_player_alert_red = assets->GetImage_16BitAlpha("IB-InitR-c");

            ui_btn_npc_left = assets->GetImage_16BitColorKey("IB-NPCLD-C", 0x7FF);
            ui_btn_npc_right = assets->GetImage_16BitColorKey("IB-NPCRD-C", 0x7FF);
            game_ui_btn_zoomin = assets->GetImage_16BitAlpha("ib-autout-C");
            game_ui_btn_zoomout = assets->GetImage_16BitAlpha("ib-autin-C");
            game_ui_player_selection_frame = assets->GetImage_16BitColorKey("IB-selec-C", 0x7FF);
            game_ui_btn_cast = assets->GetImage_16BitAlpha("ib-m1d-c");
            game_ui_btn_rest = assets->GetImage_16BitAlpha("ib-m2d-c");
            game_ui_btn_quickref = assets->GetImage_16BitAlpha("ib-m3d-c");
            game_ui_btn_settings = assets->GetImage_16BitAlpha("ib-m4d-c");

            game_ui_playerbuff_bless = assets->GetImage_16BitColorKey("isg-01-c", 0x7FF);
            game_ui_playerbuff_preservation = assets->GetImage_16BitColorKey("isg-02-c", 0x7FF);
            game_ui_playerbuff_hammerhands = assets->GetImage_16BitColorKey("isg-03-c", 0x7FF);
            game_ui_playerbuff_pain_reflection = assets->GetImage_16BitColorKey("isg-04-c", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->GetImage_16BitColorKey("ib-bcu-c", 0x7FF);

            game_ui_evtnpc = assets->GetImage_16BitColorKey("evtnpc-c", 0x7FF);
            ui_character_inventory_background = assets->GetImage_16BitColorKey("fr_inven-c", 0x7FF);
            messagebox_corner_y = assets->GetImage_16BitAlpha("cornr_ll-c");
            messagebox_corner_w = assets->GetImage_16BitAlpha("cornr_lr-c");
            messagebox_corner_x = assets->GetImage_16BitAlpha("cornr_ul-c");
            messagebox_corner_z = assets->GetImage_16BitAlpha("cornr_ur-c");
            messagebox_border_bottom = assets->GetImage_16BitAlpha("edge_btm-c");
            messagebox_border_left = assets->GetImage_16BitAlpha("edge_lf-c");
            messagebox_border_right = assets->GetImage_16BitAlpha("edge_rt-c");
            messagebox_border_top = assets->GetImage_16BitAlpha("edge_top-c");
            _591428_endcap = assets->GetImage_16BitColorKey("endcap-c", 0x7FF);
        }
        else
        {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe = assets->GetImage_PCXFromIconsLOD("ib-b-c.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar = assets->GetImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->GetImage_16BitAlpha("ib-mb-C");
            game_ui_minimap_frame = assets->GetImage_16BitAlpha("ib-autmask-c");
            game_ui_minimap_compass = assets->GetImage_16BitColorKey("IB-COMP-C", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_16BitAlpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->GetImage_16BitAlpha("IB-InitY-c");
            game_ui_player_alert_red = assets->GetImage_16BitAlpha("IB-InitR-c");

            ui_btn_npc_left = assets->GetImage_16BitColorKey("IB-NPCLD-C", 0x7FF);
            ui_btn_npc_right = assets->GetImage_16BitColorKey("IB-NPCRD-C", 0x7FF);
            game_ui_btn_zoomin = assets->GetImage_16BitColorKey("ib-autout-C", 0x7FF);
            game_ui_btn_zoomout = assets->GetImage_16BitColorKey("ib-autin-C", 0x7FF);
            game_ui_player_selection_frame = assets->GetImage_16BitColorKey("IB-selec-C", 0x7FF);
            game_ui_btn_cast = assets->GetImage_16BitAlpha("ib-m1d-c");
            game_ui_btn_rest = assets->GetImage_16BitAlpha("ib-m2d-c");
            game_ui_btn_quickref = assets->GetImage_16BitAlpha("ib-m3d-c");
            game_ui_btn_settings = assets->GetImage_16BitAlpha("ib-m4d-c");
            ui_exit_cancel_button_background = assets->GetImage_16BitColorKey("ib-bcu-c", 0x7FF);

            game_ui_playerbuff_bless = assets->GetImage_16BitColorKey("isg-01-c", 0x7FF);
            game_ui_playerbuff_preservation = assets->GetImage_16BitColorKey("isg-02-c", 0x7FF);
            game_ui_playerbuff_hammerhands = assets->GetImage_16BitColorKey("isg-03-c", 0x7FF);
            game_ui_playerbuff_pain_reflection = assets->GetImage_16BitColorKey("isg-04-c", 0x7FF);

            game_ui_evtnpc = assets->GetImage_16BitColorKey("evtnpc-c", 0x7FF);
            ui_character_inventory_background = assets->GetImage_16BitColorKey("fr_inven", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);
        }
        uGameUIFontMain = Color16(0xC8u, 0, 0);
        uGameUIFontShadow = Color16(10, 0, 0);
    }
    else if (align == PartyAlignment_Neutral)
    {
        if (bReplace)
        {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-a.pcx");
            game_ui_bottomframe = assets->GetImage_PCXFromIconsLOD("ib-b-a.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-a.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-a.pcx");
            game_ui_statusbar = assets->GetImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->GetImage_16BitAlpha("ib-mb-a");
            game_ui_minimap_frame = assets->GetImage_16BitAlpha("ib-autmask-a");
            game_ui_minimap_compass = assets->GetImage_16BitColorKey("IB-COMP-a", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_16BitAlpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->GetImage_16BitAlpha("IB-InitY-a");
            game_ui_player_alert_red = assets->GetImage_16BitAlpha("IB-InitR-a");

            ui_btn_npc_left = assets->GetImage_16BitColorKey("IB-NPCLD-a", 0x7FF);
            ui_btn_npc_right = assets->GetImage_16BitColorKey("IB-NPCRD-a", 0x7FF);
            game_ui_btn_zoomin = assets->GetImage_16BitColorKey("ib-autout-a", 0x7FF);
            game_ui_btn_zoomout = assets->GetImage_16BitColorKey("ib-autin-a", 0x7FF);
            game_ui_player_selection_frame = assets->GetImage_16BitColorKey("IB-selec-a", 0x7FF);
            game_ui_btn_cast = assets->GetImage_16BitAlpha("ib-m1d-a");
            game_ui_btn_rest = assets->GetImage_16BitAlpha("ib-m2d-a");
            game_ui_btn_quickref = assets->GetImage_16BitAlpha("ib-m3d-a");
            game_ui_btn_settings = assets->GetImage_16BitAlpha("ib-m4d-a");

            game_ui_playerbuff_bless = assets->GetImage_16BitColorKey("isg-01-a", 0x7FF);
            game_ui_playerbuff_preservation = assets->GetImage_16BitColorKey("isg-02-a", 0x7FF);
            game_ui_playerbuff_hammerhands = assets->GetImage_16BitColorKey("isg-03-a", 0x7FF);
            game_ui_playerbuff_pain_reflection = assets->GetImage_16BitColorKey("isg-04-a", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->GetImage_16BitColorKey("ib-bcu-a", 0x7FF);

            game_ui_evtnpc = assets->GetImage_16BitColorKey("evtnpc", 0x7FF);
            ui_character_inventory_background = assets->GetImage_16BitColorKey("fr_inven", 0x7FF);
            messagebox_corner_y = assets->GetImage_16BitAlpha("cornr_ll");
            messagebox_corner_w = assets->GetImage_16BitAlpha("cornr_lr");
            messagebox_corner_x = assets->GetImage_16BitAlpha("cornr_ul");
            messagebox_corner_z = assets->GetImage_16BitAlpha("cornr_ur");
            messagebox_border_bottom = assets->GetImage_16BitAlpha("edge_btm");
            messagebox_border_left = assets->GetImage_16BitAlpha("edge_lf");
            messagebox_border_right = assets->GetImage_16BitAlpha("edge_rt");
            messagebox_border_top = assets->GetImage_16BitAlpha("edge_top");
            _591428_endcap = assets->GetImage_16BitColorKey("endcap", 0x7FF);
        }
        else
        {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-A.pcx");
            game_ui_bottomframe = assets->GetImage_PCXFromIconsLOD("ib-b-A.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-A.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-A.pcx");
            game_ui_statusbar = assets->GetImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->GetImage_16BitAlpha("ib-mb-A");
            game_ui_minimap_frame = assets->GetImage_16BitAlpha("ib-autmask-a");
            game_ui_minimap_compass = assets->GetImage_16BitColorKey("IB-COMP-A", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_16BitAlpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->GetImage_16BitAlpha("IB-InitY-a");
            game_ui_player_alert_red = assets->GetImage_16BitAlpha("IB-InitR-a");

            ui_btn_npc_left = assets->GetImage_16BitColorKey("IB-NPCLD-A", 0x7FF);
            ui_btn_npc_right = assets->GetImage_16BitColorKey("IB-NPCRD-A", 0x7FF);
            game_ui_player_selection_frame = assets->GetImage_16BitColorKey("IB-selec-A", 0x7FF);
            game_ui_btn_cast = assets->GetImage_16BitAlpha("ib-m1d-a");
            game_ui_btn_rest = assets->GetImage_16BitAlpha("ib-m2d-a");
            game_ui_btn_quickref = assets->GetImage_16BitAlpha("ib-m3d-a");
            game_ui_btn_settings = assets->GetImage_16BitAlpha("ib-m4d-a");
            game_ui_btn_zoomin = assets->GetImage_16BitColorKey("ib-autout-a", 0x7FF);
            game_ui_btn_zoomout = assets->GetImage_16BitColorKey("ib-autin-a", 0x7FF);
            ui_exit_cancel_button_background = assets->GetImage_16BitColorKey("ib-bcu-a", 0x7FF);

            game_ui_playerbuff_bless = assets->GetImage_16BitColorKey("isg-01-a", 0x7FF);
            game_ui_playerbuff_preservation = assets->GetImage_16BitColorKey("isg-02-a", 0x7FF);
            game_ui_playerbuff_hammerhands = assets->GetImage_16BitColorKey("isg-03-a", 0x7FF);
            game_ui_playerbuff_pain_reflection = assets->GetImage_16BitColorKey("isg-04-a", 0x7FF);

            game_ui_evtnpc = assets->GetImage_16BitColorKey("evtnpc", 0x7FF);
            ui_character_inventory_background = assets->GetImage_16BitColorKey("fr_inven", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            messagebox_corner_y = assets->GetImage_16BitAlpha("cornr_ll");
            messagebox_corner_w = assets->GetImage_16BitAlpha("cornr_lr");
            messagebox_corner_x = assets->GetImage_16BitAlpha("cornr_ul");
            messagebox_corner_z = assets->GetImage_16BitAlpha("cornr_ur");
            messagebox_border_bottom = assets->GetImage_16BitAlpha("edge_btm");
            messagebox_border_left = assets->GetImage_16BitAlpha("edge_lf");
            messagebox_border_right = assets->GetImage_16BitAlpha("edge_rt");
            messagebox_border_top = assets->GetImage_16BitAlpha("edge_top");
            _591428_endcap = assets->GetImage_16BitColorKey("endcap", 0x7FF);
        }
        uGameUIFontMain = Color16(0xAu, 0, 0);
        uGameUIFontShadow = Color16(230, 214, 193);
    }
    else if (align == PartyAlignment_Good)
    {
        if (bReplace)
        {
            game_ui_rightframe = assets->GetImage_PCXFromIconsLOD("ib-r-B.pcx");
            game_ui_bottomframe = assets->GetImage_PCXFromIconsLOD("ib-b-B.pcx");
            game_ui_topframe = assets->GetImage_PCXFromIconsLOD("ib-t-B.pcx");
            game_ui_leftframe = assets->GetImage_PCXFromIconsLOD("ib-l-B.pcx");
            game_ui_statusbar = assets->GetImage_PCXFromIconsLOD("IB-Foot-b.pcx");

            game_ui_right_panel_frame = assets->GetImage_16BitAlpha("ib-mb-B");
            game_ui_minimap_frame = assets->GetImage_16BitAlpha("ib-autmask-b");
            game_ui_minimap_compass = assets->GetImage_16BitColorKey("IB-COMP-B", 0x7FF);
            game_ui_player_alert_green = assets->GetImage_16BitAlpha("IB-InitG-b");
            game_ui_player_alert_yellow = assets->GetImage_16BitAlpha("IB-InitY-b");
            game_ui_player_alert_red = assets->GetImage_16BitAlpha("IB-InitR-b");

            ui_btn_npc_left = assets->GetImage_16BitColorKey("IB-NPCLD-B", 0x7FF);
            ui_btn_npc_right = assets->GetImage_16BitColorKey("IB-NPCRD-B", 0x7FF);
            game_ui_btn_zoomin = assets->GetImage_16BitColorKey("ib-autout-B", 0x7FF);
            game_ui_btn_zoomout = assets->GetImage_16BitColorKey("ib-autin-B", 0x7FF);
            game_ui_player_selection_frame = assets->GetImage_16BitColorKey("IB-selec-B", 0x7FF);
            game_ui_btn_cast = assets->GetImage_16BitAlpha("ib-m1d-b");
            game_ui_btn_rest = assets->GetImage_16BitAlpha("ib-m2d-b");
            game_ui_btn_quickref = assets->GetImage_16BitAlpha("ib-m3d-b");
            game_ui_btn_settings = assets->GetImage_16BitAlpha("ib-m4d-b");

            game_ui_playerbuff_bless = assets->GetImage_16BitColorKey("isg-01-b", 0x7FF);
            game_ui_playerbuff_preservation = assets->GetImage_16BitColorKey("isg-02-b", 0x7FF);
            game_ui_playerbuff_hammerhands = assets->GetImage_16BitColorKey("isg-03-b", 0x7FF);
            game_ui_playerbuff_pain_reflection = assets->GetImage_16BitColorKey("isg-04-b", 0x7FF);

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeB");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchB");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->GetImage_16BitColorKey("ib-bcu-b", 0x7FF);
            game_ui_evtnpc = assets->GetImage_16BitColorKey("evtnpc-b", 0x7FF);
            ui_character_inventory_background = assets->GetImage_16BitColorKey("fr_inven-b", 0x7FF);
            messagebox_corner_y = assets->GetImage_16BitAlpha("cornr_ll-b");
            messagebox_corner_w = assets->GetImage_16BitAlpha("cornr_lr-b");
            messagebox_corner_x = assets->GetImage_16BitAlpha("cornr_ul-b");
            messagebox_corner_z = assets->GetImage_16BitAlpha("cornr_ur-b");
            messagebox_border_bottom = assets->GetImage_16BitAlpha("edge_btm-b");
            messagebox_border_left = assets->GetImage_16BitAlpha("edge_lf-b");
            messagebox_border_right = assets->GetImage_16BitAlpha("edge_rt-b");
            messagebox_border_top = assets->GetImage_16BitAlpha("edge_top-b");
            _591428_endcap = assets->GetImage_16BitColorKey("endcap-b", 0x7FF);
        }
        uGameUIFontMain = Color16(0, 0, 0xC8u);
        uGameUIFontShadow = Color16(255, 255, 255);
    }
    else Error("Invalid alignment type: %u", align);
}

//----- (0041D20D) --------------------------------------------------------
void DrawBuff_remaining_time_string(int uY, struct GUIWindow *window, GameTime remaining_time, struct GUIFont *Font)
{
    window->DrawText(Font, 32, uY, 0, "\r020" + MakeDateTimeString(remaining_time), 0, 0, 0);
}


//----- (0042EB8D) --------------------------------------------------------
void GUIMessageQueue::AddMessageImpl(UIMessageType msg, int param, unsigned int a4, const char *file, int line)
{
    //logger->Warning(L"%s @ (%S %u)", UIMessage2String(msg), file, line);
    if (uNumMessages < 40)
    {
        files[uNumMessages] = file;
        lines[uNumMessages] = line;

        pMessages[uNumMessages].eType = msg;
        pMessages[uNumMessages].param = param;
        pMessages[uNumMessages++].field_8 = a4;
    }
}

//----- (004637E0) --------------------------------------------------------
char sub_4637E0_is_there_popup_onscreen()
{
    return dword_507BF0_is_there_popup_onscreen == 1;
}

//----- (00417AD4) --------------------------------------------------------
unsigned int GetSkillColor(unsigned int uPlayerClass, PLAYER_SKILL_TYPE uPlayerSkillType, signed int skill_level)
{
    switch (uPlayerClass % 4)
    {
        case 0:
        {
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass][uPlayerSkillType] >= skill_level)
                return ui_character_skillinfo_can_learn;
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass + 1][uPlayerSkillType] < skill_level &&
                byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass + 2][uPlayerSkillType] < skill_level)
            {
                if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass + 3][uPlayerSkillType] < skill_level)
                    return ui_character_skillinfo_cant_learn;
            }
            return ui_character_skillinfo_can_learn_gm;
        }
        break;

        case 1:
        {
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass][uPlayerSkillType] >= skill_level)
                return ui_character_skillinfo_can_learn;
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass + 1][uPlayerSkillType] < skill_level)
            {
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass + 2][uPlayerSkillType] < skill_level)
                    return ui_character_skillinfo_cant_learn;
            }
            return ui_character_skillinfo_can_learn_gm;
        }
        break;

        case 2:
        case 3:
        {
            if (byte_4ED970_skill_learn_ability_by_class_table[uPlayerClass][uPlayerSkillType] < skill_level)
                return ui_character_skillinfo_cant_learn;
            return ui_character_skillinfo_can_learn;
        }
        break;
    }
    Error("Invalid player class: %u", uPlayerClass);
}


//----- (004BC49B) --------------------------------------------------------
void OnSelectNPCDialogueOption(DIALOGUE_TYPE newDialogueType)
{
	NPCData *speakingNPC; // ebp@1
	int npc_event_id; // ecx@10
	char *v13; // [sp-8h] [bp-18h]@60

	speakingNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
	uDialogueType = newDialogueType;
	if (!speakingNPC->uFlags)
		speakingNPC->uFlags = 1;
	if (newDialogueType == DIALOGUE_PROFESSION_DETAILS)
		dialogue_show_profession_details = ~dialogue_show_profession_details;
	else if (newDialogueType == DIALOGUE_76)
	{
		if (speakingNPC->Hired())
		{
			if ((signed int)pNPCStats->uNumNewNPCs > 0)
			{
				for (uint i = 0; i < (unsigned int)pNPCStats->uNumNewNPCs; ++i)
				{
					if (pNPCStats->pNewNPCData[i].uFlags & 0x80 && !strcmp(speakingNPC->pName, pNPCStats->pNewNPCData[i].pName))
						pNPCStats->pNewNPCData[i].uFlags &= 0x7Fu;
				}
			}
			if (pParty->pHirelings[0].pName && !_stricmp(pParty->pHirelings[0].pName, speakingNPC->pName))
				memset(&pParty->pHirelings[0], 0, sizeof(NPCData));
			else if (pParty->pHirelings[1].pName && !_stricmp(pParty->pHirelings[1].pName, speakingNPC->pName))
				memset(&pParty->pHirelings[1], 0, sizeof(NPCData));
			pParty->hirelingScrollPosition = 0;
			pParty->CountHirelings();
			pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
			dword_7241C8 = 0;
			return;
		}
		if (pParty->pHirelings[0].pName && pParty->pHirelings[1].pName)
			GameUI_StatusBar_OnEvent(localization->GetString(533));// "I cannot join you, you're party is full"
		else
		{
			if (speakingNPC->uProfession != 51) //burglars have no hiring price
			{
				if (pParty->uNumGold < pNPCStats->pProfessions[speakingNPC->uProfession].uHirePrice)
				{
					GameUI_StatusBar_OnEvent(localization->GetString(155));// "You don't have enough gold"
					dialogue_show_profession_details = false;
					uDialogueType = 13;
					if (uActiveCharacter)
						pPlayers[uActiveCharacter]->PlaySound(SPEECH_NotEnoughGold, 0);
					if (!dword_7241C8)
						pEngine->Draw();
					dword_7241C8 = 0;
					return;
				}
				Party::TakeGold(pNPCStats->pProfessions[speakingNPC->uProfession].uHirePrice);
			}
			speakingNPC->uFlags |= 0x80u;
			if (pParty->pHirelings[0].pName)
			{
				memcpy(&pParty->pHirelings[1], speakingNPC, sizeof(pParty->pHirelings[1]));
				v13 = pParty->pHireling2Name;
			}
			else
			{
				memcpy(&pParty->pHirelings[0], speakingNPC, sizeof(pParty->pHirelings[0]));
				v13 = pParty->pHireling1Name;
			}
			strcpy(v13, speakingNPC->pName);
			pParty->hirelingScrollPosition = 0;
			pParty->CountHirelings();
			pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
			if (sDialogue_SpeakingActorNPC_ID >= 0)
				pDialogue_SpeakingActor->uAIState = Removed;
			if (uActiveCharacter)
				pPlayers[uActiveCharacter]->PlaySound(SPEECH_61, 0);
		}
	}
	else if ((signed int)newDialogueType > DIALOGUE_84 && (signed int)newDialogueType <= DIALOGUE_ARENA_SELECT_CHAMPION) //выбор уровня сложности боя
	{
		ArenaFight();
		return;
	}
	else if (newDialogueType == DIALOGUE_USE_NPC_ABILITY)
	{
		if (UseNPCSkill((NPCProf)speakingNPC->uProfession) == 0)
		{
			if (speakingNPC->uProfession != GateMaster)
				speakingNPC->bHasUsedTheAbility = 1;
			pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
		}
		else
			GameUI_StatusBar_OnEvent(localization->GetString(140)); //"Your packs are already full!"
	}
	else if (newDialogueType == DIALOGUE_13)
	{
		if (!speakingNPC->Hired())
		{
			sub_4B3E1E();
			dialogue_show_profession_details = false;
		}
		else
		{
			for (uint i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i)
			{
				if (pNPCStats->pNewNPCData[i].uFlags & 0x80 && !strcmp(speakingNPC->pName, pNPCStats->pNewNPCData[i].pName))
					pNPCStats->pNewNPCData[i].uFlags &= 0x7Fu;
			}
			if (pParty->pHirelings[0].pName && !_stricmp(pParty->pHirelings[0].pName, speakingNPC->pName))
				memset(&pParty->pHirelings[0], 0, sizeof(NPCData));
			else if (pParty->pHirelings[1].pName && !_stricmp(pParty->pHirelings[1].pName, speakingNPC->pName))
				memset(&pParty->pHirelings[1], 0, sizeof(NPCData));
			pParty->hirelingScrollPosition = 0;
			pParty->CountHirelings();
			pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
			dword_7241C8 = 0;
			return;
		}
	}
	else if (newDialogueType >= DIALOGUE_EVT_A && newDialogueType <= DIALOGUE_EVT_F)
	{
		switch (newDialogueType)
		{
		case DIALOGUE_EVT_A:  npc_event_id = speakingNPC->evt_A; break;
		case DIALOGUE_EVT_B:  npc_event_id = speakingNPC->evt_B; break;
		case DIALOGUE_EVT_C:  npc_event_id = speakingNPC->evt_C; break;
		case DIALOGUE_EVT_D:  npc_event_id = speakingNPC->evt_D; break;
		case DIALOGUE_EVT_E:  npc_event_id = speakingNPC->evt_E; break;
		case DIALOGUE_EVT_F:  npc_event_id = speakingNPC->evt_F; break;
		}
		if ((npc_event_id >= 200) && (npc_event_id <= 310))
			_4B3FE5_training_dialogue(npc_event_id); //200-310
		else if ((npc_event_id >= 400) && (npc_event_id <= 410))
		{ //400-410
			dword_F8B1D8 = newDialogueType;
			DrawJoinGuildWindow(npc_event_id - 400);
		}
		else
		{
			switch (npc_event_id)
			{
			case 139:
				OracleDialogue();
				break;
			case 311:
				CheckBountyRespawnAndAward();
				break;
			case 399:
				Arena_SelectionFightLevel();
				break;
			default:
				activeLevelDecoration = (LevelDecoration*)1;
				current_npc_text.clear();
				EventProcessor(npc_event_id, 0, 1);
				activeLevelDecoration = nullptr;
				break;
			}
		}
	}
	if (!dword_7241C8)
		pEngine->Draw();
	dword_7241C8 = 0;
}

//----- (004B3E1E) --------------------------------------------------------
void sub_4B3E1E()
{
	NPCData *v0; // ST40_4@1
	signed int v1; // edi@1
	//GUIWindow *v2; // ecx@1

	__debugbreak();
	v0 = GetNPCData(sDialogue_SpeakingActorNPC_ID);
	v1 = 0;
	pDialogueWindow->eWindowType = WINDOW_MainMenu;
	pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow_Dialogue(0, 0, window->GetWidth(), window->GetHeight(), 1, 0);
	if (pNPCStats->pProfessions[v0->uProfession].pBenefits)//*(&pNPCStats->field_13A5C + 5 * v0->uProfession) )
	{
		pDialogueWindow->CreateButton(480, 160, 140, 28, 1, 0, UIMSG_SelectNPCDialogueOption, 77, 0, localization->GetString(407), 0); // Details / Подробнее
		v1 = 1;
	}
	pDialogueWindow->CreateButton(480, 30 * v1 + 160, 140, 30, 1, 0, UIMSG_SelectNPCDialogueOption, 76, 0, localization->GetString(406), 0); // Hire    Нанять
	pDialogueWindow->_41D08F_set_keyboard_control_group(v1 + 1, 1, 0, 1);
}

//----- (004B2001) --------------------------------------------------------
void ClickNPCTopic(signed int uMessageParam)
{
	NPCData *pCurrentNPCInfo; // ebp@1
	int pEventNumber; // ecx@8
	Player *v4; // esi@20
	char *v12; // eax@53
	char *v13; // eax@56
	char *v14; // eax@57
	char *v15; // eax@58
//	char *v17; // ecx@63
	signed int pPrice; // ecx@70
	char *v22; // [sp-Ch] [bp-18h]@73
	char *v24; // [sp-8h] [bp-14h]@73

	uDialogueType = uMessageParam + 1;
	pCurrentNPCInfo = HouseNPCData[pDialogueNPCCount - ((dword_591080 != 0) ? 1 : 0)];//- 1
	if (uMessageParam <= 24)
	{
		switch (uMessageParam)
		{
		case 13:
			current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->uProfession].pJoinText, uActiveCharacter - 1, 0, 0, 0);
			NPCHireableDialogPrepare();
			dialogue_show_profession_details = false;
			BackToHouseMenu();
			return;		
		case 19:
			pEventNumber = pCurrentNPCInfo->evt_A;
			break;
		case 20:
			pEventNumber = pCurrentNPCInfo->evt_B;
			break;
		case 21:
			pEventNumber = pCurrentNPCInfo->evt_C;
			break;
		case 22:
			pEventNumber = pCurrentNPCInfo->evt_D;
			break;
		case 23:
			pEventNumber = pCurrentNPCInfo->evt_E;
			break;
		case 24:
			pEventNumber = pCurrentNPCInfo->evt_F;
			break;
		default:
			BackToHouseMenu();
			return;
		}
		/*switch ( pEventNumber )
		{
		case 139:
		OracleDialogue();
		goto _return;
		case 311:
		CheckBountyRespawnAndAward();
		goto _return;
		}*/
		if (pEventNumber < 200 || pEventNumber > 310)
		{
			if (pEventNumber < 400 || pEventNumber > 410)
			{
				if (pEventNumber == 139)
				{
					OracleDialogue();
				}
				else
				{
					if (pEventNumber == 311)
					{
						CheckBountyRespawnAndAward();
					}
					else
					{
						current_npc_text.clear();
						activeLevelDecoration = (LevelDecoration*)1;
						EventProcessor(pEventNumber, 0, 1);
						activeLevelDecoration = nullptr;
					}
				}
			}
			else
			{
				dword_F8B1D8 = uMessageParam;
				DrawJoinGuildWindow(pEventNumber - 400);
			}
		}
		else
		{
			_4B3FE5_training_dialogue(pEventNumber);
		}
		BackToHouseMenu();
		return;
	}
	if (uMessageParam != 76)
	{
		if (uMessageParam == 77)
		{
			//v16 = pCurrentNPCInfo->uProfession;
			__debugbreak();  // probably hirelings found in buildings, not present in MM7, changed "pCurrentNPCInfo->uProfession - 1" to "pCurrentNPCInfo->uProfession", have to check in other versions whether it's ok
			if (dialogue_show_profession_details)
            {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->uProfession].pJoinText, uActiveCharacter - 1, 0, 0, 0
                );
            }
			else
            {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->uProfession].pBenefits, uActiveCharacter - 1, 0, 0, 0
                );
            }
            dialogue_show_profession_details = ~dialogue_show_profession_details;
		}
		else
		{
			if (uMessageParam == 79)
			{
				if (contract_approved)
				{
					Party::TakeGold(gold_transaction_amount);
					if (uActiveCharacter)
					{
						v12 = (char *)&pPlayers[uActiveCharacter]->pActiveSkills[dword_F8B1AC_award_bit_number];
						*(short *)v12 &= 0x3Fu;
						switch (dword_F8B1B0_MasteryBeingTaught)
						{
						case 2:
							v15 = (char *)&pPlayers[uActiveCharacter]->pActiveSkills[dword_F8B1AC_award_bit_number];
							*v15 |= 0x40u;
							break;
						case 3:
							v14 = (char *)&pPlayers[uActiveCharacter]->pActiveSkills[dword_F8B1AC_award_bit_number];
							*v14 |= 0x80u;
							break;
						case 4:
							v13 = (char *)&pPlayers[uActiveCharacter]->pActiveSkills[dword_F8B1AC_award_bit_number];
							v13[1] |= 1u;
							break;
						}
						pPlayers[uActiveCharacter]->PlaySound(SPEECH_85, 0);
					}
					pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
					/*if ( (signed int)pMessageQueue_50CBD0->uNumMessages < 40 )
					{
					pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType = UIMSG_Escape;
					pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 1;
					*(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
					++pMessageQueue_50CBD0->uNumMessages;
					}*/
				}
			}
			else
			{
				if (uMessageParam == 82 && contract_approved) //join guild
				{
					Party::TakeGold(gold_transaction_amount);
					v4 = pParty->pPlayers.data();
					do
					{
						v4->SetVariable(VAR_Award, dword_F8B1AC_award_bit_number);
						++v4;
					} while ((signed int)v4 < (signed int)pParty->pHirelings.data());
					switch (dword_F8B1D8)
					{
					case 19:
						pEventNumber = pCurrentNPCInfo->evt_A;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_A = 0;
						break;
					case 20:
						pEventNumber = pCurrentNPCInfo->evt_B;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_B = 0;
						break;
					case 21:
						pEventNumber = pCurrentNPCInfo->evt_C;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_C = 0;
						break;
					case 22:
						pEventNumber = pCurrentNPCInfo->evt_D;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_D = 0;
						break;
					case 23:
						pEventNumber = pCurrentNPCInfo->evt_E;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_E = 0;
						break;
					case 24:
						pEventNumber = pCurrentNPCInfo->evt_F;
						if (pEventNumber >= 400 && pEventNumber <= 416)
							pCurrentNPCInfo->evt_F = 0;
						break;
					}
					pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
					/*if ( (signed int)pMessageQueue_50CBD0->uNumMessages < 40 )
					{
					pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType = UIMSG_Escape;
					pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 1;
					*(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
					++pMessageQueue_50CBD0->uNumMessages;
					}*/
					//v11 = uActiveCharacter;
					if (uActiveCharacter)
					{
						pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_86, 0);
						BackToHouseMenu();
						return;
					}
				}
			}
		}
		BackToHouseMenu();
		return;
	}
	if (pParty->pHirelings[0].pName && pParty->pHirelings[1].pName)
	{
		GameUI_StatusBar_OnEvent(localization->GetString(533));// ""I cannot join you, you're party is full""
		BackToHouseMenu();
		return;
	}
	
	
	if (pCurrentNPCInfo->uProfession != 51) //burglars have no hiring price
	{
		__debugbreak();  // probably hirelings found in buildings, not present in MM7, changed "pCurrentNPCInfo->uProfession - 1" to "pCurrentNPCInfo->uProfession", have to check in other versions whether it's ok
		pPrice = pNPCStats->pProfessions[pCurrentNPCInfo->uProfession].uHirePrice;
		if (pParty->uNumGold < (unsigned int)pPrice)
		{
			GameUI_StatusBar_OnEvent(localization->GetString(155));
			dialogue_show_profession_details = false;
			uDialogueType = 13;
			current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->uProfession].pJoinText, uActiveCharacter - 1, 0, 0, 0);
			if (uActiveCharacter)
				pPlayers[uActiveCharacter]->PlaySound(SPEECH_NotEnoughGold, 0);
			GameUI_StatusBar_OnEvent(localization->GetString(155));
			BackToHouseMenu();
			return;
		}
		else
			Party::TakeGold(pPrice);
	}

	pCurrentNPCInfo->uFlags |= 128;
	pParty->hirelingScrollPosition = 0;
	pParty->CountHirelings();
	if (pParty->pHirelings[0].pName)
	{
		memcpy(&pParty->pHirelings[1], pCurrentNPCInfo, sizeof(pParty->pHirelings[1]));
		v24 = pCurrentNPCInfo->pName;
		v22 = pParty->pHireling2Name;
	}
	else
	{
		memcpy(pParty->pHirelings.data(), pCurrentNPCInfo, 0x4Cu);
		v24 = pCurrentNPCInfo->pName;
		v22 = pParty->pHireling1Name;
	}
	strcpy(v22, v24);
	pParty->hirelingScrollPosition = 0;
	pParty->CountHirelings();
	PrepareHouse((HOUSE_ID)(int)window_SpeakInHouse->ptr_1C);
	dialog_menu_id = HOUSE_DIALOGUE_MAIN;

	pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
	/*if ( (signed int)pMessageQueue_50CBD0->uNumMessages < 40 )
	{
	pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType = UIMSG_Escape;
	pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 1;
	*(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
	++pMessageQueue_50CBD0->uNumMessages;
	}*/
	if (uActiveCharacter)
		pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)61, 0);

_return:
	BackToHouseMenu();
}

//----- (004B3FE5) --------------------------------------------------------
//Originally called _4B254D_SkillMasteryTeacher to have contract_approved assigned, to be able to set some button name. 
//But it the name gets immediately overwritten
void _4B3FE5_training_dialogue(int a4)
{
//	const char *v2; // edi@1

	//__debugbreak();
	uDialogueType = DIALOGUE_SKILL_TRAINER;
	current_npc_text = String(pNPCTopics[a4 + 168].pText);
	_4B254D_SkillMasteryTeacher(a4);  //might be needed because of contract_approved ?
	pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(0, 0, window->GetWidth(), 350, a4, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, 0,
        localization->GetString(34), ui_exit_cancel_button_background, 0);
	pDialogueWindow->CreateButton(0, 0, 0, 0, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, 0, "", 0);
    pDialogueWindow->CreateButton(480, 160, 0x8Cu, 0x1Eu, 1, 0, UIMSG_ClickNPCTopic, 0x4Fu, 0, contract_approved ? localization->GetString(535) : "", 0);
	pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
	dialog_menu_id = HOUSE_DIALOGUE_OTHER;
}

//----- (004B1ECE) --------------------------------------------------------
void OracleDialogue()
{
	__int16 *v0; // edi@1
	signed int v4; // eax@9
	int v5; // ebx@11
	signed int v8; // edi@14
	ItemGen *v9; // [sp+Ch] [bp-Ch]@11
	signed int v10; // [sp+10h] [bp-8h]@13
	int v11; // [sp+14h] [bp-4h]@1

	contract_approved = 0;
	v11 = 0;
	uDialogueType = 84;
	current_npc_text = (char *)pNPCTopics[667].pText;
	v0 = _4F0882_evt_VAR_PlayerItemInHands_vals.data();

	for (uint i = 0; i <= 53; i++)
	{
		if ((unsigned __int16)_449B57_test_bit(pParty->_quest_bits, *v0))
		{
			for (uint pl = 0; pl < 4; pl++)
			{
				if (pParty->pPlayers[pl].CompareVariable(VAR_PlayerItemInHands, *(v0 + 1)))
					break;
			}
		}
		++v11;
	}
	if (v0 <= &_4F0882_evt_VAR_PlayerItemInHands_vals[53])
	{
		current_npc_text = (char *)pNPCTopics[666].pText; // Here's %s that you lost. Be careful
		v4 = _4F0882_evt_VAR_PlayerItemInHands_vals[2 * v11];
		contract_approved = _4F0882_evt_VAR_PlayerItemInHands_vals[2 * v11];
		pParty->pPlayers[0].AddVariable(VAR_PlayerItemInHands, v4);
	}
	if (contract_approved == 601)
	{
		v5 = 0;
		//v12 = pParty->pPlayers.data();//[0].uClass;
		v9 = 0;
		//while ( 1 )
		for (uint i = 0; i < 4; i++)
		{
			if (pParty->pPlayers[i].classType == PLAYER_CLASS_LICH)
			{
				v10 = 0;
				//v6 = pParty->pPlayers.data();//[0].pInventoryItems[0].field_1A;
				for (uint pl = 0; pl < 4; pl++)
				{
					for (v8 = 0; v8 < 126; v8++)//138
					{
						if (pParty->pPlayers[pl].pInventoryItemList[v8].uItemID == ITEM_LICH_JAR_FULL)
						{
							if (!pParty->pPlayers[pl].pInventoryItemList[v8].uHolderPlayer)
								v9 = &pParty->pPlayers[pl].pInventoryItemList[v8];
							if (pParty->pPlayers[pl].pInventoryItemList[v8].uHolderPlayer == v5)
								v10 = 1;
						}
					}
				}
				if (!v10)
					break;
			}
			//      ++v12;
			++v5;
			//  if ( v12 > &pParty->pPlayers[3] )
			//  return;
		}
		if (v9)
			v9->uHolderPlayer = v5;
	}
}

//----- (004BBA85) --------------------------------------------------------
void CheckBountyRespawnAndAward()
{
	int i; // eax@2
	int rand_monster_id; // edx@3

	uDialogueType = 83;
	pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(0, 0, window->GetWidth(), 350, 0, 0);
	pBtn_ExitCancel = pDialogueWindow->CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(34),// "Cancel"
        ui_exit_cancel_button_background, 0);
	pDialogueWindow->CreateButton(0, 0, 0, 0, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, 0, "", 0);
	pDialogueWindow->CreateButton(480, 160, 140, 30, 1, 0, UIMSG_0, 83, 0, "", 0);
	pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
	dialog_menu_id = HOUSE_DIALOGUE_OTHER;
	//get new monster for hunting
	if (pParty->PartyTimes.bountyHunting_next_generation_time[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] < pParty->GetPlayingTime())
	{
		pParty->monster_for_hunting_killed[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] = false;
		pParty->PartyTimes.bountyHunting_next_generation_time[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] = (signed __int64)((double)(0x12750000 * (pParty->uCurrentMonth + 12i64 * pParty->uCurrentYear - 14015)) * 0.033333335);
		for (i = rand();; i = rand())
		{
			rand_monster_id = i % 258 + 1;
			pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] = rand_monster_id;
			if ((unsigned __int16)rand_monster_id < 0x73u || (unsigned __int16)rand_monster_id > 0x84u)
			{
				if (((unsigned __int16)rand_monster_id < 0xEBu || (unsigned __int16)rand_monster_id > 0xFCu)
					&& ((unsigned __int16)rand_monster_id < 0x85u || (unsigned __int16)rand_monster_id > 0x96u)
					&& ((unsigned __int16)rand_monster_id < 0x97u || (unsigned __int16)rand_monster_id > 0xBAu)
					&& ((unsigned __int16)rand_monster_id < 0xC4u || (unsigned __int16)rand_monster_id > 0xC6u))
					break;
			}
		}
	}
	bountyHunting_monster_id_for_hunting = pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)];
	if (!pParty->monster_for_hunting_killed[(int)((char *)window_SpeakInHouse->ptr_1C - 102)])
	{
		bountyHunting_text = pNPCTopics[351].pText;
		if (!pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)])
			bountyHunting_text = pNPCTopics[353].pText;
	}
	else//get prize
	{
		if (pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)])
		{
			pParty->PartyFindsGold(100 * pMonsterStats->pInfos[(unsigned __int16)pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)]].uLevel, 0);
			for (uint i = 0; i < 4; ++i)
				pParty->pPlayers[i].SetVariable(VAR_Award, 86);
			pParty->uNumBountiesCollected += 100 * pMonsterStats->pInfos[pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)]].uLevel;
			pParty->monster_id_for_hunting[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] = 0;
			pParty->monster_for_hunting_killed[(int)((char *)window_SpeakInHouse->ptr_1C - 102)] = false;
		}
		bountyHunting_text = pNPCTopics[352].pText;
	}
}

//----- (004B254D) --------------------------------------------------------
String _4B254D_SkillMasteryTeacher(int trainerInfo)
{
	int teacherLevel; // edx@1
	int skillBeingTaught; // ecx@1
	int pClassType; // eax@7
	int currClassMaxMastery; // eax@7
	int pointsInSkillWOutMastery; // ebx@7
	int classBaseId; // eax@8
	unsigned int skillMastery; // eax@29
	unsigned __int16 pointsInSkill; // [sp+1Ch] [bp-10h]@7
	int masteryLevelBeingTaught; // [sp+24h] [bp-8h]@7

	contract_approved = 0;
	teacherLevel = (trainerInfo - 200) % 3;
	skillBeingTaught = (trainerInfo - 200) / 3;
	Player* activePlayer = pPlayers[uActiveCharacter];
	pClassType = activePlayer->classType;
	currClassMaxMastery = byte_4ED970_skill_learn_ability_by_class_table[pClassType][skillBeingTaught];
	masteryLevelBeingTaught = teacherLevel + 2;
	dword_F8B1B0_MasteryBeingTaught = masteryLevelBeingTaught;
	if (currClassMaxMastery < masteryLevelBeingTaught)
	{
		classBaseId = pClassType - pClassType % 4;

        if (byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 1][skillBeingTaught] >= masteryLevelBeingTaught)
        {
            return localization->FormatString(633, localization->GetClassName(classBaseId + 1)); // You have to be promoted to %s to learn this skill level.   /   Вы должны достичь звания %s для обучения этому уровню навыка.
        }
        else if (byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 2][skillBeingTaught] >= masteryLevelBeingTaught
            && byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 3][skillBeingTaught] >= masteryLevelBeingTaught
            )
        {
            return localization->FormatString(634, localization->GetClassName(classBaseId + 2), localization->GetClassName(classBaseId + 3)); // You have to be promoted to %s or %s to learn this skill level.   /   Вы должны достичь звания %s или %s для обучения этому уровню навыка.
        }
        else if (byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 2][skillBeingTaught] >= masteryLevelBeingTaught)
        {
            return localization->FormatString(633, localization->GetClassName(classBaseId + 2)); // You have to be promoted to %s to learn this skill level.   /   Вы должны достичь звания %s для обучения этому уровню навыка.
        }
        else if (byte_4ED970_skill_learn_ability_by_class_table[classBaseId + 3][skillBeingTaught] >= masteryLevelBeingTaught)
        {
            return localization->FormatString(633, localization->GetClassName(classBaseId + 3)); // You have to be promoted to %s to learn this skill level.   /   Вы должны достичь звания %s для обучения этому уровню навыка.
        }
        else
        {
            return localization->FormatString(632, localization->GetClassName(pClassType)); // This skill level can not be learned by the %s class.   /   Этот уровень навыка не может быть постигнут классом %s.
        }
	}

	if (!activePlayer->CanAct())
		return String(pNPCTopics[122].pText); // Not in your condition!

	pointsInSkill = activePlayer->pActiveSkills[skillBeingTaught];
	pointsInSkillWOutMastery = pointsInSkill & 0x3F;
	if (!pointsInSkillWOutMastery)
		return String(pNPCTopics[131].pText); // You must know the skill before you can become an expert in it!

	skillMastery = SkillToMastery(pointsInSkill);
	if (skillMastery > teacherLevel + 1)
		return String(pNPCTopics[teacherLevel + 128].pText);    // You are already an SKILLLEVEL in this skill.	

	dword_F8B1AC_award_bit_number = skillBeingTaught;
	if (masteryLevelBeingTaught == 2 && pointsInSkillWOutMastery < 4
		|| masteryLevelBeingTaught == 3 && pointsInSkillWOutMastery < 7
		|| masteryLevelBeingTaught == 4 && pointsInSkillWOutMastery < 10
		)
		return String(pNPCTopics[127].pText);  // You don't meet the requirements, and cannot be taught until you do.

	switch (dword_F8B1AC_award_bit_number)
	{
	case PLAYER_SKILL_STAFF:
	case PLAYER_SKILL_SWORD:
	case PLAYER_SKILL_DAGGER:
	case PLAYER_SKILL_AXE:
	case PLAYER_SKILL_SPEAR:
	case PLAYER_SKILL_BOW:
	case PLAYER_SKILL_MACE:
	case PLAYER_SKILL_ARMSMASTER:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			gold_transaction_amount = 5000;
			break;
		case 4:
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_BLASTER:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 0;
			break;
		case 3:
			gold_transaction_amount = 0;
			break;
		case 4:
			gold_transaction_amount = 0;
			break;
		}
		break;
	case PLAYER_SKILL_SHIELD:
	case PLAYER_SKILL_LEATHER:
	case PLAYER_SKILL_CHAIN:
	case PLAYER_SKILL_PLATE:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 1000;
			break;
		case 3:
			gold_transaction_amount = 3000;
			break;
		case 4:
			gold_transaction_amount = 7000;
			break;
		}
		break;
	case PLAYER_SKILL_FIRE:
	case PLAYER_SKILL_AIR:
	case PLAYER_SKILL_WATER:
	case PLAYER_SKILL_EARTH:
	case PLAYER_SKILL_SPIRIT:
	case PLAYER_SKILL_MIND:
	case PLAYER_SKILL_BODY:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 1000;
			break;
		case 3:
			gold_transaction_amount = 4000;
			break;
		case 4:
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_LIGHT:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			if (!(unsigned __int16)_449B57_test_bit(pParty->_quest_bits, 114))
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 5000;
			break;
		case 4:
			if (!activePlayer->ProfessionOrGuildFlagsCorrect(0x22u, 1) ||
				!activePlayer->ProfessionOrGuildFlagsCorrect(0x1Au, 1))
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_DARK:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			if (!(unsigned __int16)_449B57_test_bit(pParty->_quest_bits, 110))
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 5000;
			break;
		case 4:
			if (!activePlayer->ProfessionOrGuildFlagsCorrect(0x23u, 1)
				|| !activePlayer->ProfessionOrGuildFlagsCorrect(0x1Bu, 1))
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_ITEM_ID:
	case PLAYER_SKILL_REPAIR:
	case PLAYER_SKILL_MEDITATION:
	case PLAYER_SKILL_PERCEPTION:
	case PLAYER_SKILL_TRAP_DISARM:
	case PLAYER_SKILL_MONSTER_ID:
	case PLAYER_SKILL_STEALING:
	case PLAYER_SKILL_ALCHEMY:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 500;
			break;
		case 3:
			gold_transaction_amount = 2500;
			break;
		case 4:
			gold_transaction_amount = 6000;
			break;
		}
		break;
	case PLAYER_SKILL_MERCHANT:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			if (activePlayer->GetBaseWillpower() < 50)
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 5000;
			break;
		case 4:
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_BODYBUILDING:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 500;
			break;
		case 3:
			if (activePlayer->GetBaseEndurance() < 50)
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 2500;
			break;
		case 4:
			gold_transaction_amount = 6000;
			break;
		}
		break;
	case PLAYER_SKILL_DIPLOMACY:
		Error("Diplomacy not used");
		break;
	case PLAYER_SKILL_TIEVERY:
		Error("Thievery not used");
		break;
	case PLAYER_SKILL_DODGE:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			gold_transaction_amount = 5000;
			break;
		case 4:
			if ((activePlayer->pActiveSkills[PLAYER_SKILL_UNARMED] & 63) < 0xA)
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 8000;
			break;
		}
		break;
	case PLAYER_SKILL_UNARMED:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			gold_transaction_amount = 5000;
			break;
		case 4:
			if ((activePlayer->pActiveSkills[PLAYER_SKILL_DODGE] & 63) < 0xA)
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 8000;
			break;
		}
		break;

	case PLAYER_SKILL_LEARNING:
		switch (masteryLevelBeingTaught)
		{
		case 2:
			gold_transaction_amount = 2000;
			break;
		case 3:
			if (activePlayer->GetBaseIntelligence() < 50)
				return String(pNPCTopics[127].pText);
			gold_transaction_amount = 5000;
			break;
		case 4:
			gold_transaction_amount = 8000;
			break;
		}
		break;
	default:
		Error("Unknown skill");
	}

	if (gold_transaction_amount > pParty->uNumGold)
		return String(pNPCTopics[124].pText);  // You don't have enough gold!

	contract_approved = 1;
	if (masteryLevelBeingTaught == 2)
	{
        return localization->FormatString(
            534, // Получить степень ^Pr[%s] в навыке ^Pr[%s] за ^I[%lu] золот^L[ой;ых;ых]
			localization->GetString(433), // Expert
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount
        );
	}
	else if (masteryLevelBeingTaught == 3)
	{
        return localization->FormatString(
            534,
			localization->GetString(432), // Master
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount
        );
	}
    else if (masteryLevelBeingTaught == 4)
    {
        return localization->FormatString(
            534,
            localization->GetString(225), // Grandmaster
            localization->GetSkillName(dword_F8B1AC_award_bit_number),
            gold_transaction_amount
        );
    }

    return String("");
}

String BuildDialogueString(const char *lpsz, unsigned __int8 uPlayerID, ItemGen *a3, char *a4, int a5, GameTime *a6)
{
    return BuildDialogueString(String(lpsz), uPlayerID, a3, a4, a5, a6);
}

//----- (00495461) --------------------------------------------------------
String BuildDialogueString(String &str, unsigned __int8 uPlayerID, ItemGen *a3, char *a4, int shop_screen, GameTime *a6)
{
    char v1[256];
	Player *pPlayer; // ebx@3
	const char *pText; // esi@7
	int v17; // eax@10
	signed __int64 v18; // qax@18
	unsigned __int8 *v20; // ebx@32
	int v21; // ecx@34
	int v29; // eax@68
	__int16 v55[56]; // [sp+10h] [bp-128h]@34
    SummonedItem v56; // [sp+80h] [bp-B8h]@107
	int v63; // [sp+12Ch] [bp-Ch]@32

	pPlayer = &pParty->pPlayers[uPlayerID];

	NPCData *npc = nullptr;
	if (dword_5C35D4)
		npc = HouseNPCData[(unsigned int)((char *)pDialogueNPCCount + -(dword_591080 != 0))]; //- 1
	else
		npc = GetNPCData(sDialogue_SpeakingActorNPC_ID);

    String result;

	//pText = a4;
	uint len = str.length();
	for (int i = 0, dst = 0; i < len; ++i)
	{
		char c = str[i]; //skip through string till we find insertion point
		if (c != '%')
			result += c; // add char to result string
		else
		{
			v17 = 10 * (int)(str[i + 1] - '0') + str[i + 2] - '0'; //v17 tells what the gap needs filling with
            i += 2;

			switch (v17)
			{
			case 1:
                result += npc->pName;
				break;
			case 2:
                result += pPlayer->pName;
				i += 2;
				break;
			case 3:
			case 4:
                result += v1;
				break;
			case 5:
				v18 = pParty->GetPlayingTime().GetHoursOfDay();
				pText = localization->GetString(397);// "evening"
				if (HEXRAYS_SHIDWORD(v18) <= 0 && HEXRAYS_SHIDWORD(v18) >= 0 && (unsigned int)v18 >= 5 && HEXRAYS_SHIDWORD(v18) <= 0)
				{
					if (HEXRAYS_SHIDWORD(v18) >= 0 && (unsigned int)v18 >= 11)
					{
						if (v18 < 20)
							pText = localization->GetString(396);// "day"
					}
					else
					{
						pText = localization->GetString(395);// "morning"
					}
				}
                result += pText;
				break;
			case 6:
				if (pPlayer->uSex)
                    result += localization->GetString(387);// "lady"
				else
                    result += localization->GetString(385);// "sir"
				break;
			case 7:
				if (pPlayer->uSex)
                    result += localization->GetString(389);// "Lady"
				else
                    result += localization->GetString(386);// "Sir"
				break;
			case 8:
				v63 = 0;
				v20 = (unsigned __int8 *)pPlayer->_achieved_awards_bits;
				for (uint _i = 0; _i < 28; ++_i)
				{
					if ((unsigned __int16)_449B57_test_bit(v20, word_4EE150[i]))
					{
						v21 = v63;
						++v63;
						v55[v63] = word_4EE150[i];
					}
				}
				if (v63)
				{
					if (dword_A74CDC == -1)
						dword_A74CDC = rand() % v63;
					pText = pAwards[v55[dword_A74CDC]].pText;//(char *)dword_723E80_award_related[2 * v55[v24]];
				}
				else
					pText = pNPCTopics[55].pText;
                result += pText;
				break;
			case 9:
				if (npc->uSex)
                    result += localization->GetString(384);// "her"
				else
                    result += localization->GetString(383);// "his"
				break;
			case 10:
				if (pPlayer->uSex)
                    result += localization->GetString(389);// "Lady"
				else
                    result += localization->GetString(388);// "Lord"
				break;
			case 11:
                result += GetReputationString(pParty->GetPartyReputation());
				break;
			case 12:
                result += GetReputationString(npc->rep);
				break;
			case 13:
                result += pNPCStats->sub_495366_MispronounceName(pPlayer->pName[0], pPlayer->uSex);
				break;
			case 14:
				if (npc->uSex)
                    result += localization->GetString(391);// "sister"
				else
                    result += localization->GetString(390);// "brother"
				break;
			case 15:
                result += localization->GetString(393);// "daughter"
				break;
			case 16:
				if (npc->uSex)
                    result += localization->GetString(391);// "sister"
				else
                    result += localization->GetString(390);// "brother"
				break;
			case 17: // hired npc text   текст наёмного НПС
			{
						uint pay_percentage = pNPCStats->pProfessions[npc->uProfession].uHirePrice / 100;
						if (!pay_percentage)
							pay_percentage = 1;
						sprintf(v1, "%lu", pay_percentage);
                        result += v1;
						break;
			}
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 26:
				strncpy(v1, str.c_str() + i + 1, 2);
				sprintf(v1, "%lu", atoi(v1));
                result += v1;
				break;
			case 23:
				if (pMapStats->GetMapInfo(pCurrentMapName))
                    result += pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName;
				else
                    result += localization->GetString(394);// "Unknown"
				break;

			case 24://item name
				sprintf(v1, format_4E2D80, Color16(255, 255, 155), a3->GetDisplayName().c_str());
                result += v1;
				break;

			case 25: // base prices
				v29 = pPlayer->GetBaseBuyingPrice(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
				switch (shop_screen)
				{
				case 3:
					v29 = pPlayer->GetBaseSellingPrice(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					break;
				case 4:
					v29 = pPlayer->GetBaseIdentifyPrice(p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					break;
				case 5:
					v29 = pPlayer->GetBaseRepairPrice(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					break;
				case 6:
					v29 = pPlayer->GetBaseSellingPrice(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier) / 2;
					break;
				}
				sprintf(v1, "%lu", v29);
                result += v1;
				break;

			case 27://actual price
				v29 = pPlayer->GetBuyingPrice(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
				if (shop_screen == 3)
				{
					//v29 = pPlayer->GetPriceSell(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					v29 = pPlayer->GetPriceSell(*a3, p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					//if (a3->IsBroken())
						//v29 = 1;
					sprintf(v1, "%lu", v29);
                    result += v1;
					break;
				}
				if (shop_screen != 4)
				{
					if (shop_screen == 5)
						v29 = pPlayer->GetPriceRepair(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier);
					else
					{
						if (shop_screen == 6)
						{
							//v29 = pPlayer->GetPriceSell(a3->GetValue(), p2DEvents[(signed int)a4 - 1].fPriceMultiplier) / 2;
							v29 = pPlayer->GetPriceSell(*a3, p2DEvents[(signed int)a4 - 1].fPriceMultiplier) / 2;
							//if (a3->IsBroken())
							//	v29 = 1;
							if (!v29) // cannot be 0
								v29 = 1;
							sprintf(v1, "%lu", v29);
                            result += v1;
							break;
						}
					}
					sprintf(v1, "%lu", v29);
                    result += v1;
					break;
				}
				sprintf(v1, "%lu", pPlayer->GetPriceIdentification(p2DEvents[(signed int)a4 - 1].fPriceMultiplier));
                result += v1;
				break;

			case 28: //shop type - blacksmith ect..
                result += p2DEvents[(signed int)a4 - 1].pProprieterTitle;
				break;

			case 29: // identify cost
				sprintf(v1, "%lu", pPlayer->GetPriceIdentification(p2DEvents[(signed int)a4 - 1].fPriceMultiplier));
                result += v1;
				break;
			case 30:
				if (!a6)
				{
                    result += a4;
					break;
				}
                v56.Initialize(*a6);
                result += localization->FormatString(378, localization->GetMonthName(v56.field_14_exprie_month), v56.field_C_expire_day + 1, v56.field_18_expire_year);
				break;
			case 31:
			case 32:
			case 33:
			case 34:
                result += pParty->pPlayers[v17 - 31].pName;
				break;
			default:
				if (v17 <= 50 || v17 > 70)
				{
					strncpy(v1, str.c_str() + i + 1, 2);
					sprintf(v1, "%lu", atoi(v1));
                    result += v1;
					break;
				}
				if (v17 - 51 >= 20)
				{
                    result += a4;
					break;
				}

				v56.Initialize(pParty->PartyTimes._s_times[v17 - 51]);
                result += localization->FormatString(378, localization->GetMonthName(v56.field_14_exprie_month), v56.field_C_expire_day + 1, v56.field_18_expire_year);
				break;
			}
		}
	}

	return result;
}

//----- (0044C28B) --------------------------------------------------------
int const_2()
{
	return 2;
}
