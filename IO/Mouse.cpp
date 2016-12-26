#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Objects/Actor.h"

#include "IO/Mouse.h"

#include "GUI/GUIWindow.h"

#include "Media/Audio/AudioPlayer.h"


Mouse *pMouse = nullptr;





//----- (00469860) --------------------------------------------------------
void Mouse::GetClickPos(unsigned int *pX, unsigned int *pY)
{
    *pX = uMouseClickX;
    *pY = uMouseClickY;
}

//----- (004698A6) --------------------------------------------------------
void Mouse::RemoveHoldingItem()
{
    pParty->pPickedItem.Reset();
    if (this->cursor_name != "MICON2")
        SetCursorImage("MICON1");
}

//----- (004698D8) --------------------------------------------------------
void Mouse::SetCursorBitmapFromItemID(unsigned int uItemID)
{
  pMouse->SetCursorImage(pItemsTable->pItems[uItemID].pIconName);
}

//----- (004698F6) --------------------------------------------------------
void Mouse::SetCurrentCursorBitmap()
{
  SetCursorImage(this->cursor_name);
}

//----- (00469903) --------------------------------------------------------
void Mouse::SetCursorImage(const String &name)
{
    if (!this->bInitialized)
    {
        return;
    }

    if (this->cursor_name != name)
        this->cursor_name = name;

    if (name == "MICON2")
    {
        pEngine->uFlags2 |= GAME_FLAGS_2_TARGETING_MODE;
    }
    else
    {
        pEngine->uFlags2 &= ~GAME_FLAGS_2_TARGETING_MODE;
    }

    ClearCursor();
    if (name == "MICON1") // arrow
    {
        this->bActive = false;
        this->field_C = 1;
        window->SetCursor(name.c_str());
    }
    else // cursor is item or another bitmap
    {
        this->cursor_img = assets->GetImage_16BitColorKey(name, 0x7FF);
        this->AllocCursorSystemMem();
        this->field_C = 0;
        this->bRedraw = true;
        this->bActive = true;
    }
}

//----- (00469AE4) --------------------------------------------------------
void Mouse::_469AE4()
{
    this->field_8 = 1;

    Point pt = OS_GetMouseCursorPos();
    pt = window->TransformCursorPos(pt);

    auto v3 = pt.y;
    auto v2 = pt.x;

    this->uMouseClickX = v2;
    this->uMouseClickY = v3;

    if (true/*render->bWindowMode*/ && (v2 < 0 || v3 < 0 || v2 > window->GetWidth() - 1 || v3 > window->GetHeight() - 1))
    {
        this->bActive = false;
        this->field_8 = 0;
    }

    if (this->field_C)
    {
        this->bActive = false;
    }

    this->field_8 = 0;
}

//----- (00469BA3) --------------------------------------------------------
void Mouse::ClearCursor()
{
  this->bActive = false;
  free(this->pCursorBitmap_sysmem);
  this->pCursorBitmap_sysmem = nullptr;
  free(this->pCursorBitmap2_sysmem);
  this->pCursorBitmap2_sysmem = nullptr;
  free(this->ptr_90);
  this->ptr_90 = nullptr;
}

//----- (00469BE6) --------------------------------------------------------
void Mouse::AllocCursorSystemMem()
{
  bActive = false;
  if (!pCursorBitmap_sysmem)
    pCursorBitmap_sysmem = (unsigned __int16 *)DoAllocCursorMem();
  if (!pCursorBitmap2_sysmem)
    pCursorBitmap2_sysmem = (unsigned __int8 *)DoAllocCursorMem();
}

//----- (00469C0D) --------------------------------------------------------
void *Mouse::DoAllocCursorMem()
{
    return nullptr;
}

//----- (00469C39) --------------------------------------------------------
Point Mouse::GetCursorPos()
{
    return Point(this->uMouseClickX, this->uMouseClickY);
}

//----- (00469C65) --------------------------------------------------------
void Mouse::Initialize(OSWindow *window)
{
  this->window = window;
  this->bActive = false;
  this->bInitialized = true;

  //this->field_8 = 0;//Ritor1: result incorrect uMouseClickX, this->uMouseClickY in _469AE4()
  this->uCursorBitmapPitch = 0;//Ritor1: it's include
  for ( uint i = 0; i < 13; i++ )
    this->field_5C[i] = 0;

  this->pCursorBitmapPos.x = 0;
  this->pCursorBitmapPos.y = 0;
  this->uMouseClickX = 0;
  this->uMouseClickY = 0;
  this->pCursorBitmap_sysmem = nullptr;
  this->field_34 = 0;
  this->pCursorBitmap2_sysmem = nullptr;

  SetCursorImage("MICON3");
  SetCursorImage("MICON2");
  SetCursorImage("MICON1");
}

// inlined
//----- (0045FE00) mm6 chinese --------------------------------------------
void Mouse::SetActive(bool active)
{
  bActive = active;
}

//----- (00469CC2) --------------------------------------------------------
void Mouse::Deactivate()
{
  if (bInitialized)
    SetActive(false);
}

//----- (00469CCD) --------------------------------------------------------
void Mouse::DrawCursor()
{
    unsigned int v9; // eax@31

    if (this->bInitialized)
    {
        if (!this->field_8 && this->bActive && !this->field_C) //Uninitialized memory access(this->field_8)
            pMouse->_469AE4();//Ritor1: странная, непонятная функция
        this->field_F4 = 1;
        if (this->field_C)
        {
            this->field_F4 = 0;
            return;
        }

        //if ( render->bWindowMode )
        {
            if (this->uMouseClickX < 0 || this->uMouseClickY < 0 || this->uMouseClickX > window->GetWidth() - 1 || this->uMouseClickY > window->GetHeight() - 1)
            {
                this->field_F4 = 0;
                return;
            }
        }

        this->pCursorBitmapRect.x = this->uMouseClickX;
        this->pCursorBitmapRect.w = this->uMouseClickY + this->field_5C[0]; //Ritor1: Maybe this->field_5C[0] - cursor width
        this->pCursorBitmapRect.y = this->uMouseClickY;
        this->pCursorBitmapRect.z = this->uMouseClickX + this->uCursorBitmapPitch; //Ritor1: Maybe this->uCursorBitmapPitch - cursor height
        if (this->uMouseClickX < 0)
            this->pCursorBitmapRect.x = 0;
        if (this->uMouseClickY < 0)
            this->pCursorBitmapRect.y = 0;
        if (this->pCursorBitmapRect.z > window->GetWidth())
            this->pCursorBitmapRect.z = window->GetWidth();
        if (this->pCursorBitmapRect.w > window->GetHeight())
            this->pCursorBitmapRect.w = window->GetHeight();
        this->bActive = false;
        this->uCursorBitmapWidth = this->pCursorBitmapRect.z - this->pCursorBitmapRect.x;
        this->uCursorBitmapHeight = this->pCursorBitmapRect.w - this->pCursorBitmapRect.y;
        if (this->bRedraw)
        {
            if (pMouse->ptr_90)
                v9 = 2 * pMouse->uCursorBitmapPitch;
            else
                v9 = 0;
            render->_4A6DF5(
                this->pCursorBitmap_sysmem, v9, &this->pCursorBitmapPos, render->pTargetSurface, render->uTargetSurfacePitch,
                &this->pCursorBitmapRect
            );//срабатывает когда берём курсором вещь в инвенторе
            this->bRedraw = false;
        }
    }
}

//----- (00469E1C) --------------------------------------------------------
void Mouse::Activate()
{
  bActive = true;
}

//----- (00469E24) --------------------------------------------------------
void Mouse::_469E24()
{
  free(pCursorBitmap3_sysmembits_16bit);
  pCursorBitmap3_sysmembits_16bit = nullptr;
}

//----- (00469E3B) --------------------------------------------------------
void Mouse::DrawCursorToTarget()//??? DrawCursorWithItem
{
  if (!pCursorBitmap3_sysmembits_16bit)
    return;
  //пишем на экран курсор с вещью
  ushort* pSrc = pCursorBitmap3_sysmembits_16bit;
  for (int y = uCursorWithItemY; y < uCursorWithItemZ; ++y)
    for (int x = uCursorWithItemX; x < uCursorWithItemW; ++x)
      //render->pTargetSurface[y * render->uTargetSurfacePitch + x] = *pSrc++;
        render->WritePixel16(x, y, *pSrc++);
}

//----- (00469EA4) --------------------------------------------------------
void Mouse::ReadCursorWithItem()
{
    unsigned int pTextureID; // eax@2
    Image *pTexture; // edi@2
  //  int v8; // ecx@25
  //  int v9; // ebx@26
  //  unsigned int v10; // eax@26
  //  int v11; // edx@27
    int pTextureHeight; // [sp+20h] [bp-8h]@15
  //  unsigned __int16 *v20; // [sp+20h] [bp-8h]@28
    int pTextureWidth; // [sp+24h] [bp-4h]@12
    unsigned __int16 *v22; // [sp+24h] [bp-4h]@25

    if (pParty->pPickedItem.uItemID)
    {
        pTexture = assets->GetImage_16BitAlpha(pParty->pPickedItem.GetIconName());

        if ((signed int)pMouse->uMouseClickX <= window->GetWidth() - 1 && (signed int)pMouse->uMouseClickY <= window->GetHeight() - 1)
        {
            if ((signed int)(pTexture->GetWidth() + pMouse->uMouseClickX) <= window->GetWidth())
                pTextureWidth = pTexture->GetWidth();
            else
                pTextureWidth = window->GetWidth() - pMouse->uMouseClickX;
            if ((signed int)(pTexture->GetHeight() + pMouse->uMouseClickY) <= window->GetHeight())
                pTextureHeight = pTexture->GetHeight();
            else
                pTextureHeight = window->GetHeight() - pMouse->uMouseClickY;
            if (!this->pCursorBitmap3_sysmembits_16bit
                || pMouse->uMouseClickX != this->uCursorWithItemX
                || pMouse->uMouseClickY != this->uCursorWithItemY
                || pMouse->uMouseClickX + pTextureWidth != this->uCursorWithItemW
                || pMouse->uMouseClickY + pTextureHeight != this->uCursorWithItemZ)
            {
                free(this->pCursorBitmap3_sysmembits_16bit);
                this->pCursorBitmap3_sysmembits_16bit = (unsigned __int16 *)malloc(2 * pTexture->GetHeight() * pTexture->GetWidth());
                this->uCursorWithItemX = pMouse->uMouseClickX;
                this->uCursorWithItemW = pMouse->uMouseClickX + pTextureWidth;
                this->uCursorWithItemY = pMouse->uMouseClickY;
                this->uCursorWithItemZ = pMouse->uMouseClickY + pTextureHeight;
            }
            v22 = this->pCursorBitmap3_sysmembits_16bit;

            for (int y = this->uCursorWithItemY; y < this->uCursorWithItemZ; ++y)
            {
                for (int x = this->uCursorWithItemX; x < this->uCursorWithItemW; ++x)
                {
                    *v22++ = render->ReadPixel16(x, y);
                }
            }

            if (pParty->pPickedItem.IsBroken())
                render->DrawTransparentRedShade(pMouse->uMouseClickX / 640.0f, pMouse->uMouseClickY / 480.0f, pTexture);
            else if (!pParty->pPickedItem.IsIdentified())
                render->DrawTransparentGreenShade(pMouse->uMouseClickX / 640.0f, pMouse->uMouseClickY / 480.0f, pTexture);
            else
                render->DrawTextureAlphaNew(pMouse->uMouseClickX / 640.0f, pMouse->uMouseClickY / 480.0f, pTexture);
        }
    }
    else
    {
        free(this->pCursorBitmap3_sysmembits_16bit);
        this->pCursorBitmap3_sysmembits_16bit = nullptr;
    }
}

//----- (0046A080) --------------------------------------------------------
void Mouse::ChangeActivation(int a1)
{
  this->bActive = a1;
}

//----- (0046A08A) --------------------------------------------------------
void Mouse::SetMouseClick(int x, int y)
{
  uMouseClickX = x;
  uMouseClickY = y;
}
//----- (004175C0) --------------------------------------------------------
void Mouse::UI_OnMouseLeftClick(int *pXY)
{
  signed int y; // eax@7
  signed int x; // ecx@7
  signed int v5; // eax@17
  GUIButton *control; // esi@37
  signed int v10; // eax@50
//  int v11; // ecx@52
  unsigned int pX; // [sp+14h] [bp-8h]@7
  unsigned int pY; // [sp+18h] [bp-4h]@7

  if ( current_screen_type == SCREEN_VIDEO || sub_4637E0_is_there_popup_onscreen() )
    return;
  if ( pGUIWindow2 && pGUIWindow2->ptr_1C == (void *)33 )
  {
    sub_4452BB();
    return;
  }
  if ( pXY )
  {
    x = *pXY;
    y = pXY[1];
    pX = *pXY;
    pY = y;
  }
  else
  {
    pMouse->GetClickPos(&pX, &pY);
    y = pY;
    x = pX;
  }

  extern bool _507B98_ctrl_pressed;
  x = pX;
  if ( GetCurrentMenuID() != -1 || current_screen_type != SCREEN_GAME || !_507B98_ctrl_pressed // stealing cursor
      || (signed int)pX < (signed int)pViewport->uViewportTL_X || (signed int)pX > (signed int)pViewport->uViewportBR_X
      || (signed int)pY < (signed int)pViewport->uViewportTL_Y || (signed int)pY > (signed int)pViewport->uViewportBR_Y)
  {
    y = pY;
    for ( int i = uNumVisibleWindows; i >= 0; --i )
    {
      if ( x >= (signed int)pWindowList[pVisibleWindowsIdxs[i] - 1]->uFrameX && x <= (signed int)pWindowList[pVisibleWindowsIdxs[i] - 1]->uFrameZ
        && y >= (signed int)pWindowList[pVisibleWindowsIdxs[i] - 1]->uFrameY && y <= (signed int)pWindowList[pVisibleWindowsIdxs[i] - 1]->uFrameW )
      {
        for ( control = pWindowList[pVisibleWindowsIdxs[i] - 1]->pControlsHead; control; control = control->pNext )
        {
          if ( control->uButtonType == 1 )
          {
            if ( x >= (signed int)control->uX && x <= (signed int)control->uZ && y >= (signed int)control->uY && y <= (signed int)control->uW )
            {
              control->field_2C_is_pushed = 1;
              v10 = pMessageQueue_50CBD0->uNumMessages;
              if ( pMessageQueue_50CBD0->uNumMessages )
              {
                v10 = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
                pMessageQueue_50CBD0->uNumMessages = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
              }
              pMessageQueue_50CBD0->AddGUIMessage(control->msg, control->msg_param, 0);
              return;
            }
            continue;
          }
          if ( control->uButtonType == 2 )//когда нажимаешь на партреты персов
          {
            if ( (signed int)(signed __int64)sqrt((double)((x - control->uX) * (x - control->uX) + (y - control->uY) * (y - control->uY))) < (signed int)control->uWidth )
            {
              control->field_2C_is_pushed = 1;
              v10 = pMessageQueue_50CBD0->uNumMessages;
              if ( pMessageQueue_50CBD0->uNumMessages )
              {
                v10 = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
                pMessageQueue_50CBD0->uNumMessages = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
              }
              pMessageQueue_50CBD0->AddGUIMessage(control->msg, control->msg_param, 0);
              return;
            }
            continue;
          }
          if ( control->uButtonType == 3 )//когда нажимаешь на скиллы
          {
            if ( x >= (signed int)control->uX && x <= (signed int)control->uZ && y >= (signed int)control->uY && y <= (signed int)control->uW )
            {
              control->field_2C_is_pushed = 1;
              v10 = pMessageQueue_50CBD0->uNumMessages;
              if ( pMessageQueue_50CBD0->uNumMessages )
              {
                v10 = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
                pMessageQueue_50CBD0->uNumMessages = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
              }
              pMessageQueue_50CBD0->AddGUIMessage(control->msg, control->msg_param, 0);
              return;
            }
            continue;
          }
          y = pY;
          x = pX;
        }
      }
    }
    return;
  }
  y = pY;
  //if ( render->pRenderD3D )
    v5 = pEngine->pVisInstance->get_picked_object_zbuf_val();
  /*else
    v5 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/

  uint type = PID_TYPE((unsigned __int16)v5);
  if (type == OBJECT_Actor && uActiveCharacter && v5 < 0x2000000
    && pPlayers[uActiveCharacter]->CanAct() && pPlayers[uActiveCharacter]->CanSteal() )
  {
    /*if ( (signed int)pMessageQueue_50CBD0->uNumMessages < 40 )
    {
      pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType = UIMSG_1B;
      pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = v6 >> 3;
      *(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
      ++pMessageQueue_50CBD0->uNumMessages;
    }*/
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_STEALFROMACTOR, PID_ID((unsigned __int16)v5), 0);

    if ( pParty->bTurnBasedModeOn == 1 )
    {
      if ( pTurnEngine->turn_stage == TE_MOVEMENT )
        pTurnEngine->field_18 |= TE_FLAG_8;
    }
  }
}


//----- (0041CD4F) --------------------------------------------------------
bool Mouse::UI_OnKeyDown(unsigned int vkKey)
{
  //unsigned int v1; // edi@1
  //unsigned int v2; // eax@2
  int v3; // esi@3
  int v4; // ecx@10
  GUIButton *pButton; // eax@11
  int v6; // edx@12
  int v7; // ecx@20
  char v8; // zf@21
  //GUIButton *v9; // ecx@24
  int v10; // esi@24
  //int v11; // edx@26
  int v12; // edx@28
  int v13; // esi@32
  //GUIButton *v14; // eax@37
  int v15; // edx@38
  int v17; // ecx@50
  int v18; // edx@50
  //GUIButton *v19; // ecx@54
  int v20; // esi@54
  //int v21; // edx@56
  int v22; // ecx@59
  int v23; // edx@59
  int v24; // ecx@60
  int v25; // esi@63
  //unsigned int v26; // [sp+Ch] [bp-14h]@1
  //int v27; // [sp+10h] [bp-10h]@1
  int v28; // [sp+14h] [bp-Ch]@10
  int v29; // [sp+14h] [bp-Ch]@36
  unsigned int uClickX; // [sp+18h] [bp-8h]@10
  unsigned int uClickY; // [sp+1Ch] [bp-4h]@10

  //v1 = 0;
  //v27 = uNumVisibleWindows;
  if ( uNumVisibleWindows < 0 )
    return false;
  //v2 = pMessageQueue_50CBD0->uNumMessages;
  for (int i = uNumVisibleWindows; i >= 0; --i)
  //while ( 1 )
  {
    v3 = pVisibleWindowsIdxs[i] - 1;
    if (!pWindowList[v3]->receives_keyboard_input)
      continue;

    switch (vkKey)
    {
      case VK_LEFT:
      {
        v12 = pWindowList[v3]->field_34;
        if ( pWindowList[v3]->pCurrentPosActiveItem - pWindowList[v3]->pStartingPosActiveItem - v12 >= 0 )
        {
          v8 = current_screen_type == SCREEN_PARTY_CREATION;
          pWindowList[v3]->pCurrentPosActiveItem -= v12;
          if ( v8 )
          {
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
            //v2 = pMessageQueue_50CBD0->uNumMessages;
          }
        }
        if ( pWindowList[v3]->field_30 != 0 )
        {
          break;
        }
        pButton = pWindowList[v3]->pControlsHead;
        v13 = pWindowList[v3]->pCurrentPosActiveItem;
        if ( v13 > 0)
        {
          do
          {
            pButton = pButton->pNext;
            --v13;
          }
          while ( v13 );
        }
        pMessageQueue_50CBD0->AddGUIMessage(pButton->msg, pButton->msg_param, 0);
        break;
      }
      case VK_RIGHT:
      {
        v7 = pWindowList[v3]->pCurrentPosActiveItem + pWindowList[v3]->field_34;
        if ( v7 < pWindowList[v3]->pNumPresenceButton + pWindowList[v3]->pStartingPosActiveItem )
        {
          v8 = current_screen_type == SCREEN_PARTY_CREATION;
          pWindowList[v3]->pCurrentPosActiveItem = v7;
          if ( v8 )
          {
            pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
            //v2 = pMessageQueue_50CBD0->uNumMessages;
          }
        }
        if ( pWindowList[v3]->field_30 != 0 )
        {
          break;
        }
        pButton = pWindowList[v3]->pControlsHead;
        v10 = pWindowList[v3]->pCurrentPosActiveItem;
        if ( v10 > 0)
        {
          do
          {
            pButton = pButton->pNext;
            --v10;
          }
          while ( v10 );
        }
        pMessageQueue_50CBD0->AddGUIMessage(pButton->msg, pButton->msg_param, 0);
        break;
      }
      case VK_DOWN:
      {
        v17 = pWindowList[v3]->pStartingPosActiveItem;
        v18 = pWindowList[v3]->pCurrentPosActiveItem;
        if ( v18 >= pWindowList[v3]->pNumPresenceButton + v17 - 1 )
          pWindowList[v3]->pCurrentPosActiveItem = v17;
        else
          pWindowList[v3]->pCurrentPosActiveItem = v18 + 1;
        if ( pWindowList[v3]->field_30 != 0 )
          return true;
        pButton = pWindowList[v3]->pControlsHead;
        v20 = pWindowList[v3]->pCurrentPosActiveItem;
        if ( v20 > 0)
        {
          do
          {
            pButton = pButton->pNext;
            --v20;
          }
          while ( v20 );
        }
        pMessageQueue_50CBD0->AddGUIMessage(pButton->msg, pButton->msg_param, 0);
        return true;
      }
      case VK_SELECT:
      {
        pMouse->GetClickPos(&uClickX, &uClickY);
        v4 = pWindowList[v3]->pStartingPosActiveItem;
        v28 = v4 + pWindowList[v3]->pNumPresenceButton;
        if ( v4 < v4 + pWindowList[v3]->pNumPresenceButton )
        {
          while ( 1 )
          {
            pButton = pWindowList[v3]->pControlsHead;
            if ( v4 > 0 )
            {
              v6 = v4;
              do
              {
                pButton = pButton->pNext;
                --v6;
              }
              while ( v6 );
            }
            if ( (signed int)uClickX >= (signed int)pButton->uX//test for StatsTab in PlayerCreation Window
               && (signed int)uClickX <= (signed int)pButton->uZ
               && (signed int)uClickY >= (signed int)pButton->uY
               && (signed int)uClickY <= (signed int)pButton->uW )
              break;
            ++v4;
            if ( v4 >= v28 )
            {
              //v1 = 0;
              //v2 = pMessageQueue_50CBD0->uNumMessages;
              //--i;
              //if ( i < 0 )
                return false;
              //continue;
            }
          }
          pWindowList[v3]->pCurrentPosActiveItem = v4;
          return true;
        }
        //v2 = pMessageQueue_50CBD0->uNumMessages;
        break;
      }
      case VK_UP:
      {
        v22 = pWindowList[v3]->pCurrentPosActiveItem;
        v23 = pWindowList[v3]->pStartingPosActiveItem;
        if ( v22 <= v23 )
          v24 = pWindowList[v3]->pNumPresenceButton + v23 - 1;
        else
          v24 = v22 - 1;
        v8 = pWindowList[v3]->field_30 == 0;
        pWindowList[v3]->pCurrentPosActiveItem = v24;
        if ( !v8 )
          return true;
        pButton = pWindowList[v3]->pControlsHead;
        v25 = pWindowList[v3]->pCurrentPosActiveItem;
        if ( v25 > 0)
        {
          do
          {
            pButton = pButton->pNext;
            --v25;
          }
          while ( v25 );
        }
        pMessageQueue_50CBD0->AddGUIMessage(pButton->msg, pButton->msg_param, 0);
        return true;
      }
      case VK_NEXT:
      {  
        if ( pWindowList[v3]->field_30 != 0 )   //crashed at skill draw
        {
          pMouse->GetClickPos(&uClickX, &uClickY);
          v29 = pWindowList[v3]->pStartingPosActiveItem + pWindowList[v3]->pNumPresenceButton; //num buttons more than buttons 
          for ( v4 = pWindowList[v3]->pStartingPosActiveItem; v4 < v29; ++v4 )
          {
            pButton = pWindowList[v3]->pControlsHead;
            if (!pButton)
                continue;
            if ( v4 > 0 )
            {
              for ( v15 = v4; v15; --v15 )
                pButton = pButton->pNext;
            }
            if ( (signed int)uClickX >= (signed int)pButton->uX && (signed int)uClickX <= (signed int)pButton->uZ
              && (signed int)uClickY >= (signed int)pButton->uY && (signed int)uClickY <= (signed int)pButton->uW )
            {
              pWindowList[v3]->pCurrentPosActiveItem = v4;
              return true;
            }
          }
        }
        break;
      }
      default:
        break;
    }
  }
}
