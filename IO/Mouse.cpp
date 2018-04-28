#include "IO/Mouse.h"

#include <crtdbg.h>
#include <stdlib.h>

#include "Engine/Engine.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;

void Mouse::GetClickPos(unsigned int *pX, unsigned int *pY) {
    *pX = uMouseClickX;
    *pY = uMouseClickY;
}

void Mouse::RemoveHoldingItem() {
    pParty->pPickedItem.Reset();
    if (this->cursor_name != "MICON2") {
        SetCursorImage("MICON1");
    }
}

void Mouse::SetCursorBitmapFromItemID(unsigned int uItemID) {
    SetCursorImage(pItemsTable->pItems[uItemID].pIconName);
}

void Mouse::SetCurrentCursorBitmap() { SetCursorImage(this->cursor_name); }

void Mouse::SetCursorImage(const String &name) {
    if (!this->bInitialized) {
        return;
    }

    if (this->cursor_name != name)
        this->cursor_name = name;

    engine_config->SetTargetingMode(name == "MICON2");

    ClearCursor();
    if (name == "MICON1") {  // arrow
        this->bActive = false;
        this->field_C = 1;
        window->SetCursor(name.c_str());
    } else {  // cursor is item or another bitmap
        this->cursor_img = assets->GetImage_ColorKey(name, 0x7FF);
        this->AllocCursorSystemMem();
        this->field_C = 0;
        this->bRedraw = true;
        this->bActive = true;
    }
}

void Mouse::_469AE4() {
    this->field_8 = 1;

    Point pt = OS_GetMouseCursorPos();
    pt = window->TransformCursorPos(pt);

    auto v3 = pt.y;
    auto v2 = pt.x;

    this->uMouseClickX = v2;
    this->uMouseClickY = v3;

    if (true /*render->bWindowMode*/ &&
        (v2 < 0 || v3 < 0 || v2 > window->GetWidth() - 1 ||
         v3 > window->GetHeight() - 1)) {
        this->bActive = false;
        this->field_8 = 0;
    }

    if (this->field_C) {
        this->bActive = false;
    }

    this->field_8 = 0;
}

void Mouse::ClearCursor() {
    this->bActive = false;
    free(this->pCursorBitmap_sysmem);
    this->pCursorBitmap_sysmem = nullptr;
    free(this->pCursorBitmap2_sysmem);
    this->pCursorBitmap2_sysmem = nullptr;
    free(this->ptr_90);
    this->ptr_90 = nullptr;
}

void Mouse::AllocCursorSystemMem() {
    bActive = false;
    if (!pCursorBitmap_sysmem)
        pCursorBitmap_sysmem = (uint16_t *)DoAllocCursorMem();
    if (!pCursorBitmap2_sysmem)
        pCursorBitmap2_sysmem = (uint8_t *)DoAllocCursorMem();
}

void *Mouse::DoAllocCursorMem() { return nullptr; }

Point Mouse::GetCursorPos() {
    return Point(this->uMouseClickX, this->uMouseClickY);
}

void Mouse::Initialize(OSWindow *window) {
    this->window = window;
    this->bActive = false;
    this->bInitialized = true;

    // this->field_8 = 0;//Ritor1: result incorrect uMouseClickX,
    // this->uMouseClickY in _469AE4()
    this->uCursorBitmapPitch = 0;  // Ritor1: it's include
    for (uint i = 0; i < 13; i++) this->field_5C[i] = 0;

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

void Mouse::SetActive(bool active) { bActive = active; }

void Mouse::Deactivate() {
    if (bInitialized) {
        SetActive(false);
    }
}

void Mouse::DrawCursor() {
    /*
      if (this->bInitialized) {
        if (!this->field_8 && this->bActive && !this->field_C) //Uninitialized
    memory access(this->field_8) pMouse->_469AE4();  // Ritor1: странная,
    непонятная функция this->field_F4 = 1; if (this->field_C) { this->field_F4 =
    0; return;
        }

        if (this->uMouseClickX < 0 || this->uMouseClickY < 0 ||
    this->uMouseClickX > window->GetWidth() - 1 || this->uMouseClickY >
    window->GetHeight() - 1) { this->field_F4 = 0; return;
        }

        this->pCursorBitmapRect_x = this->uMouseClickX;
        this->pCursorBitmapRect_w = this->uMouseClickY + this->field_5C[0];
    //Ritor1: Maybe this->field_5C[0] - cursor width this->pCursorBitmapRect_y =
    this->uMouseClickY; this->pCursorBitmapRect_z = this->uMouseClickX +
    this->uCursorBitmapPitch; //Ritor1: Maybe this->uCursorBitmapPitch - cursor
    height if (this->uMouseClickX < 0) this->pCursorBitmapRect_x = 0; if
    (this->uMouseClickY < 0) this->pCursorBitmapRect_y = 0; if
    (this->pCursorBitmapRect_z > window->GetWidth()) this->pCursorBitmapRect_z =
    window->GetWidth(); if (this->pCursorBitmapRect_w > window->GetHeight())
          this->pCursorBitmapRect_w = window->GetHeight();
        this->bActive = false;
        this->uCursorBitmapWidth = this->pCursorBitmapRect_z -
    this->pCursorBitmapRect_x; this->uCursorBitmapHeight =
    this->pCursorBitmapRect_w - this->pCursorBitmapRect_y; if (this->bRedraw) {
          unsigned int v9 = 0;
          if (pMouse->ptr_90) {
            v9 = 2 * pMouse->uCursorBitmapPitch;
          }

          Vec2_int_ point;
          point.x = pCursorBitmapPos.x;
          point.y = pCursorBitmapPos.y;

          Vec4_int_ rect;
          rect.x = pCursorBitmapRect_x;
          rect.y = pCursorBitmapRect_y;
          rect.w = pCursorBitmapRect_w;
          rect.z = pCursorBitmapRect_z;

    //      render->_4A6DF5(pCursorBitmap_sysmem, v9, &point, &rect);  //
    срабатывает когда берём курсором вещь в инвенторе this->bRedraw = false;
        }
      }
    */
}

void Mouse::Activate() { bActive = true; }

void Mouse::ClearPickedItem() { pPickedItem = nullptr; }

void Mouse::DrawCursorToTarget() {  //??? DrawCursorWithItem
    if (pPickedItem == nullptr) {
        return;
    }
    //пишем на экран курсор с вещью
    render->DrawTextureAlphaNew(uCursorWithItemX / 640.0f,
                                uCursorWithItemY / 480.0f, pPickedItem);
}

void Mouse::ReadCursorWithItem() {
    if (pParty->pPickedItem.uItemID) {
        Image *pTexture =
            assets->GetImage_Alpha(pParty->pPickedItem.GetIconName());
        pTexture->GetWidth();

        if ((int)uMouseClickX <= window->GetWidth() - 1 &&
            (int)uMouseClickY <= window->GetHeight() - 1) {
            int pTextureHeight;
            int pTextureWidth;
            if ((int)(pTexture->GetWidth() + uMouseClickX) <=
                window->GetWidth())
                pTextureWidth = pTexture->GetWidth();
            else
                pTextureWidth = window->GetWidth() - uMouseClickX;
            if ((int)(pTexture->GetHeight() + uMouseClickY) <=
                window->GetHeight())
                pTextureHeight = pTexture->GetHeight();
            else
                pTextureHeight = window->GetHeight() - uMouseClickY;

            pPickedItem = pTexture;
            this->uCursorWithItemX = uMouseClickX;
            this->uCursorWithItemY = uMouseClickY;

            if (pParty->pPickedItem.IsBroken())
                render->DrawTransparentRedShade(uMouseClickX / 640.0f,
                                                uMouseClickY / 480.0f,
                                                pTexture);
            else if (!pParty->pPickedItem.IsIdentified())
                render->DrawTransparentGreenShade(uMouseClickX / 640.0f,
                                                  uMouseClickY / 480.0f,
                                                  pTexture);
            else
                render->DrawTextureAlphaNew(uMouseClickX / 640.0f,
                                            uMouseClickY / 480.0f,
                                            pTexture);
        }
    } else {
        ClearPickedItem();
    }
}

void Mouse::ChangeActivation(int a1) { this->bActive = a1; }

void Mouse::SetMouseClick(int x, int y) {
    uMouseClickX = x;
    uMouseClickY = y;
}

void Mouse::UI_OnMouseLeftClick() {
    if (current_screen_type == SCREEN_VIDEO ||
        sub_4637E0_is_there_popup_onscreen())
        return;

    if (pGUIWindow2 && pGUIWindow2->ptr_1C == (void *)33) {
        sub_4452BB();
        return;
    }

    unsigned int x = 0;
    unsigned int y = 0;
    GetClickPos(&x, &y);

    extern bool _507B98_ctrl_pressed;
    if (GetCurrentMenuID() != -1 || current_screen_type != SCREEN_GAME ||
        !OS_IfCtrlPressed() || !pViewport->Contains(x, y)) {
        for (GUIWindow *win : lWindowList) {
            if (win->Contains(x, y)) {
                for (GUIButton *control : win->vButtons) {
                    if (control->uButtonType == 1) {
                        if (control->Contains(x, y)) {
                            control->field_2C_is_pushed = true;
                            pMessageQueue_50CBD0->Flush();
                            pMessageQueue_50CBD0->AddGUIMessage(
                                control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                    if (control->uButtonType ==
                        2) {  // когда нажимаешь на партреты персов
                        if ((int)sqrt(
                                (double)((x - control->uX) * (x - control->uX) +
                                         (y - control->uY) *
                                             (y - control->uY))) <
                            (int)control->uWidth) {
                            control->field_2C_is_pushed = true;
                            pMessageQueue_50CBD0->Flush();
                            pMessageQueue_50CBD0->AddGUIMessage(
                                control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                    if (control->uButtonType ==
                        3) {  // когда нажимаешь на скиллы
                        if (control->Contains(x, y)) {
                            control->field_2C_is_pushed = true;
                            pMessageQueue_50CBD0->Flush();
                            pMessageQueue_50CBD0->AddGUIMessage(
                                control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                }
            }
        }
        return;
    }

    int picked_object = EngineIoc::ResolveVis()->get_picked_object_zbuf_val();

    ObjectType type = PID_TYPE(picked_object);
    if (type == OBJECT_Actor && uActiveCharacter && picked_object < 0x2000000 &&
        pPlayers[uActiveCharacter]->CanAct() &&
        pPlayers[uActiveCharacter]->CanSteal()) {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_STEALFROMACTOR,
                                            PID_ID(picked_object), 0);

        if (pParty->bTurnBasedModeOn == 1) {
            if (pTurnEngine->turn_stage == TE_MOVEMENT)
                pTurnEngine->field_18 |= TE_FLAG_8;
        }
    }
}

bool UI_OnVkKeyDown(unsigned int vkKey) {
    for (GUIWindow *win : lWindowList) {
        if (!win->receives_keyboard_input) {
            continue;
        }

        switch (vkKey) {
            case VK_LEFT: {
                int v12 = win->field_34;
                if (win->pCurrentPosActiveItem - win->pStartingPosActiveItem - v12 >= 0) {
                    win->pCurrentPosActiveItem -= v12;
                    if (current_screen_type == SCREEN_PARTY_CREATION) {
                        pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0);
                        // v2 = pMessageQueue_50CBD0->uNumMessages;
                    }
                }
                if (win->field_30 != 0) {
                    break;
                }
                GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
                pMessageQueue_50CBD0->AddGUIMessage(pButton->msg, pButton->msg_param, 0);
                break;
            }
            case VK_RIGHT: {
                int v7 = win->pCurrentPosActiveItem + win->field_34;
                if (v7 <
                    win->pNumPresenceButton + win->pStartingPosActiveItem) {
                    win->pCurrentPosActiveItem = v7;
                    if (current_screen_type == SCREEN_PARTY_CREATION) {
                        pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0,
                                                0, -1, 0, 0);
                        // v2 = pMessageQueue_50CBD0->uNumMessages;
                    }
                }
                if (win->field_30 != 0) {
                    break;
                }
                GUIButton *pButton =
                    win->GetControl(win->pCurrentPosActiveItem);
                pMessageQueue_50CBD0->AddGUIMessage(pButton->msg,
                                                    pButton->msg_param, 0);
                break;
            }
            case VK_DOWN: {
                int v17 = win->pStartingPosActiveItem;
                int v18 = win->pCurrentPosActiveItem;
                if (v18 >= win->pNumPresenceButton + v17 - 1)
                    win->pCurrentPosActiveItem = v17;
                else
                    win->pCurrentPosActiveItem = v18 + 1;
                if (win->field_30 != 0) return true;
                GUIButton *pButton =
                    win->GetControl(win->pCurrentPosActiveItem);
                pMessageQueue_50CBD0->AddGUIMessage(pButton->msg,
                                                    pButton->msg_param, 0);
                return true;
            }
            case VK_SELECT: {
                unsigned int uClickX;
                unsigned int uClickY;
                EngineIoc::ResolveMouse()->GetClickPos(&uClickX, &uClickY);
                int v4 = win->pStartingPosActiveItem;
                int v28 = v4 + win->pNumPresenceButton;
                if (v4 < v4 + win->pNumPresenceButton) {
                    while (true) {
                        GUIButton *pButton = win->GetControl(v4);
                        if (uClickX >= pButton->uX  // test for StatsTab in
                                                    // PlayerCreation Window
                            && uClickX <= pButton->uZ &&
                            uClickY >= pButton->uY && uClickY <= pButton->uW)
                            break;
                        ++v4;
                        if (v4 >= v28) {
                            // v1 = 0;
                            // v2 = pMessageQueue_50CBD0->uNumMessages;
                            // --i;
                            // if ( i < 0 )
                            return false;
                            // continue;
                        }
                    }
                    win->pCurrentPosActiveItem = v4;
                    return true;
                }
                // v2 = pMessageQueue_50CBD0->uNumMessages;
                break;
            }
            case VK_UP: {
                int v22 = win->pCurrentPosActiveItem;
                int v23 = win->pStartingPosActiveItem;
                if (v22 <= v23)
                    win->pCurrentPosActiveItem =
                        win->pNumPresenceButton + v23 - 1;
                else
                    win->pCurrentPosActiveItem = v22 - 1;
                if (win->field_30 != 0) return true;
                GUIButton *pButton =
                    win->GetControl(win->pCurrentPosActiveItem);
                pMessageQueue_50CBD0->AddGUIMessage(pButton->msg,
                                                    pButton->msg_param, 0);
                return true;
            }
            case VK_NEXT: {
                if (win->field_30 != 0) {  // crashed at skill draw
                    unsigned int uClickX;
                    unsigned int uClickY;
                    EngineIoc::ResolveMouse()->GetClickPos(&uClickX, &uClickY);
                    int v29 = win->pStartingPosActiveItem +
                              win->pNumPresenceButton;  // num buttons more than
                                                        // buttons
                    for (int v4 = win->pStartingPosActiveItem; v4 < v29; ++v4) {
                        GUIButton *pButton = win->GetControl(v4);
                        if (!pButton) continue;
                        if (uClickX >= pButton->uX && uClickX <= pButton->uZ &&
                            uClickY >= pButton->uY && uClickY <= pButton->uW) {
                            win->pCurrentPosActiveItem = v4;
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

    return 0;
}
