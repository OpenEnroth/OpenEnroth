#include "Mouse.h"

#include <cstdlib>
#include <list>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/AssetsManager.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIBranchlessDialogue.h"

#include "Media/Audio/AudioPlayer.h"

std::shared_ptr<Io::Mouse> mouse = nullptr;

void Io::Mouse::GetClickPos(int *pX, int *pY) {
    *pX = uMouseX;
    *pY = uMouseY;
}

void Io::Mouse::RemoveHoldingItem() {
    pParty->pPickedItem.Reset();
    if (this->cursor_name != "MICON2") {
        SetCursorImage("MICON1");
    }
}

void Io::Mouse::SetCursorBitmapFromItemID(ItemId uItemID) {
    SetCursorImage(pItemTable->pItems[uItemID].iconName);
}

void Io::Mouse::SetCurrentCursorBitmap() { SetCursorImage(this->cursor_name); }

void Io::Mouse::SetCursorImage(const std::string &name) {
    if (!this->bInitialized) {
        return;
    }

    if (this->cursor_name != name)
        this->cursor_name = name;

    engine->SetTargetingMode(name == "MICON2");

    ClearCursor();
    if (name == "MICON1") {  // arrow
        this->bActive = false;
        this->field_C = 1;
        platform->setCursorShown(true);
        this->cursor_img = nullptr;
    } else {  // cursor is item or another bitmap
        this->cursor_img = assets->getImage_ColorKey(name, colorTable.Black /*colorTable.TealMask*/);
        this->AllocCursorSystemMem();
        this->field_C = 0;
        this->bActive = true;
    }
}

void Io::Mouse::_469AE4() {
    this->field_8 = 1;

    Pointi pt = GetCursorPos();

    auto v3 = pt.y;
    auto v2 = pt.x;

    this->uMouseX = v2;
    this->uMouseY = v3;

    Sizei renDims = render->GetPresentDimensions();
    if (true /*render->bWindowMode*/ &&
        (v2 < 0 || v3 < 0 || v2 > renDims.w - 1 ||
         v3 > renDims.h - 1)) {
        this->bActive = false;
        this->field_8 = 0;
    }

    if (this->field_C) {
        this->bActive = false;
    }

    this->field_8 = 0;
}

void Io::Mouse::ClearCursor() {
    this->bActive = false;
    free(this->pCursorBitmap_sysmem);
    this->pCursorBitmap_sysmem = nullptr;
    free(this->pCursorBitmap2_sysmem);
    this->pCursorBitmap2_sysmem = nullptr;
    free(this->ptr_90);
    this->ptr_90 = nullptr;
}

void Io::Mouse::AllocCursorSystemMem() {
    bActive = false;
    if (!pCursorBitmap_sysmem)
        pCursorBitmap_sysmem = (uint16_t *)DoAllocCursorMem();
    if (!pCursorBitmap2_sysmem)
        pCursorBitmap2_sysmem = (uint8_t *)DoAllocCursorMem();
}

void *Io::Mouse::DoAllocCursorMem() { return nullptr; }

Pointi Io::Mouse::GetCursorPos() {
    return Pointi(this->uMouseX, this->uMouseY);
}

void Io::Mouse::Initialize() {
    this->bActive = false;
    this->bInitialized = true;

    // this->field_8 = 0;//Ritor1: result incorrect uMouseX,
    // this->uMouseY in _469AE4()
    this->uCursorBitmapPitch = 0;  // Ritor1: it's include
    for (uint i = 0; i < 13; i++) this->field_5C[i] = 0;

    this->pCursorBitmapPos.x = 0;
    this->pCursorBitmapPos.y = 0;
    this->uMouseX = 0;
    this->uMouseY = 0;
    this->pCursorBitmap_sysmem = nullptr;
    this->field_34 = 0;
    this->pCursorBitmap2_sysmem = nullptr;

    SetCursorImage("MICON3");
    SetCursorImage("MICON2");
    SetCursorImage("MICON1");
}

void Io::Mouse::SetActive(bool active) { bActive = active; }

void Io::Mouse::Deactivate() {
    if (bInitialized) {
        SetActive(false);
    }
}

void Io::Mouse::DrawCursor() {
    // get mouse pos
    Pointi pos;
    this->GetClickPos(&pos.x, &pos.y);

    // for party held item
    if (pParty->pPickedItem.uItemID != ITEM_NULL) {
        DrawPickedItem();
    } else {
        ClearPickedItem();

        // for other cursor img ie target mouse
        if (this->cursor_img) {
            platform->setCursorShown(false);
            // draw image - needs centering
            pos.x -= (this->cursor_img->width()) / 2;
            pos.y -= (this->cursor_img->height()) / 2;

            render->DrawTextureNew(pos.x / 640., pos.y / 480., this->cursor_img);
        } else {
            platform->setCursorShown(true);
        }
    }

    /*
      if (this->bInitialized) {
        if (!this->field_8 && this->bActive && !this->field_C) //Uninitialized
    memory access(this->field_8) pMouse->_469AE4();  // Ritor1: странная,
    непонятная функция this->field_F4 = 1; if (this->field_C) { this->field_F4 =
    0; return;
        }

        if (this->uMouseX < 0 || this->uMouseY < 0 ||
    this->uMouseX > window->GetWidth() - 1 || this->uMouseY >
    window->GetHeight() - 1) { this->field_F4 = 0; return;
        }

        this->pCursorBitmapRect_x = this->uMouseX;
        this->pCursorBitmapRect_w = this->uMouseY + this->field_5C[0];
    //Ritor1: Maybe this->field_5C[0] - cursor width this->pCursorBitmapRect_y =
    this->uMouseY; this->pCursorBitmapRect_z = this->uMouseX +
    this->uCursorBitmapPitch; //Ritor1: Maybe this->uCursorBitmapPitch - cursor
    height if (this->uMouseX < 0) this->pCursorBitmapRect_x = 0; if
    (this->uMouseY < 0) this->pCursorBitmapRect_y = 0; if
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

          Vec2i point;
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

void Io::Mouse::Activate() { bActive = true; }

void Io::Mouse::ClearPickedItem() { pPickedItem = nullptr; }

void Io::Mouse::DrawCursorToTarget() {  //??? DrawCursorWithItem
    return;

    if (pPickedItem == nullptr) {
        return;
    }
    //пишем на экран курсор с вещью
    render->DrawTextureNew(uCursorWithItemX / 640.0f,
                                uCursorWithItemY / 480.0f, pPickedItem);
}

void Io::Mouse::DrawPickedItem() {
    if (pParty->pPickedItem.uItemID == ITEM_NULL)
        return;

    GraphicsImage *pTexture = assets->getImage_Alpha(pParty->pPickedItem.GetIconName());
    if (!pTexture) return;

    if (pParty->pPickedItem.IsBroken()) {
        render->DrawTransparentRedShade(uMouseX / 640.0f, uMouseY / 480.0f, pTexture);
    } else if (!pParty->pPickedItem.IsIdentified()) {
        render->DrawTransparentGreenShade(uMouseX / 640.0f, uMouseY / 480.0f, pTexture);
    } else {
        render->DrawTextureNew(uMouseX / 640.0f, uMouseY / 480.0f, pTexture);
    }
}

void Io::Mouse::ChangeActivation(int a1) { this->bActive = a1; }

void Io::Mouse::SetMouseClick(int x, int y) {
    uMouseX = x;
    uMouseY = y;
}

void Io::Mouse::UI_OnMouseLeftClick() {
    if (current_screen_type == SCREEN_VIDEO || isHoldingMouseRightButton())
        return;

    if (pGUIWindow_BranchlessDialogue && pGUIWindow_BranchlessDialogue->wData.val == (int)EVENT_PressAnyKey) {
        releaseBranchlessDialogue();
        return;
    }

    int x = 0;
    int y = 0;
    GetClickPos(&x, &y);

    if (GetCurrentMenuID() != MENU_NONE || current_screen_type != SCREEN_GAME ||
        !keyboardInputHandler->IsStealingToggled() || !pViewport->Contains(x, y)) {
        std::list<GUIWindow*> targetedSpellUI = {pGUIWindow_CastTargetedSpell};
        std::list<GUIWindow*> *checkWindowList = &lWindowList;
        if (pGUIWindow_CastTargetedSpell) {
            // Block regular UI if targeted spell casting is active
            checkWindowList = &targetedSpellUI;
        }
        for (GUIWindow *win : *checkWindowList) {
            if (win->Contains(x, y)) {
                for (GUIButton *control : win->vButtons) {
                    if (control->uButtonType == 1) {
                        if (control->Contains(x, y)) {
                            control->field_2C_is_pushed = true;
                            engine->_messageQueue->clear();
                            engine->_messageQueue->addMessageCurrentFrame(control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                    if (control->uButtonType == 2) {  // adventurers portraits click
                        if (std::sqrt(
                                (double)((x - control->uX) * (x - control->uX) +
                                         (y - control->uY) * (y - control->uY))) < (double)control->uWidth) {
                            control->field_2C_is_pushed = true;
                            engine->_messageQueue->clear();
                            engine->_messageQueue->addMessageCurrentFrame(control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                    if (control->uButtonType == 3) {  // clicking skills
                        if (control->Contains(x, y)) {
                            control->field_2C_is_pushed = true;
                            engine->_messageQueue->clear();
                            engine->_messageQueue->addMessageCurrentFrame(control->msg, control->msg_param, 0);
                            return;
                        }
                        continue;
                    }
                }
                // absorb event if its within this windows boundary
                // prevent hidden buttons being pressed
                break;
            }
        }
        return;
    }

    Vis_PIDAndDepth picked_object = EngineIocContainer::ResolveVis()->get_picked_object_zbuf_val();

    ObjectType type = picked_object.object_pid.type();
    if (type == OBJECT_Actor && pParty->hasActiveCharacter() && picked_object.depth < 0x200 &&
        pParty->activeCharacter().CanAct() &&
        pParty->activeCharacter().CanSteal()) {
        engine->_messageQueue->addMessageCurrentFrame(
            UIMSG_STEALFROMACTOR,
            picked_object.object_pid.id(),
            0
        );

        if (pParty->bTurnBasedModeOn) {
            if (pTurnEngine->turn_stage == TE_MOVEMENT) {
                pTurnEngine->flags |= TE_FLAG_8_finished;
            }
        }
    }
}

bool UI_OnKeyDown(PlatformKey key) {
    for (GUIWindow *win : lWindowList) {
        if (!win->receives_keyboard_input) {
            continue;
        }

        if (keyboardActionMapping->IsKeyMatchAction(Io::InputAction::DialogLeft, key)) {
            int v12 = win->field_34;
            if (win->pCurrentPosActiveItem - win->pStartingPosActiveItem - v12 >= 0) {
                win->pCurrentPosActiveItem -= v12;
                if (current_screen_type == SCREEN_PARTY_CREATION) {
                    pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
                }
            }
            if (win->field_30 != 0) {
                break;
            }
            GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
            engine->_messageQueue->addMessageCurrentFrame(pButton->msg, pButton->msg_param, 0);
            break;
        } else if (keyboardActionMapping->IsKeyMatchAction(Io::InputAction::DialogRight, key)) {
            int v7 = win->pCurrentPosActiveItem + win->field_34;
            if (v7 < win->pNumPresenceButton + win->pStartingPosActiveItem) {
                win->pCurrentPosActiveItem = v7;
                if (current_screen_type == SCREEN_PARTY_CREATION) {
                    pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
                }
            }
            if (win->field_30 != 0) {
                break;
            }
            GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
            engine->_messageQueue->addMessageCurrentFrame(pButton->msg, pButton->msg_param, 0);
            break;
        } else if (keyboardActionMapping->IsKeyMatchAction(Io::InputAction::DialogDown, key)) {
            int v17 = win->pStartingPosActiveItem;
            int v18 = win->pCurrentPosActiveItem;
            if (v18 >= win->pNumPresenceButton + v17 - 1)
                win->pCurrentPosActiveItem = v17;
            else
                win->pCurrentPosActiveItem = v18 + 1;
            if (win->field_30 != 0) return true;
            GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
            engine->_messageQueue->addMessageCurrentFrame(pButton->msg, pButton->msg_param, 0);
            return true;
        } else if (key == PlatformKey::KEY_SELECT) {
            int uClickX;
            int uClickY;
            EngineIocContainer::ResolveMouse()->GetClickPos(&uClickX, &uClickY);
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
                        // v2 = pCurrentFrameMessageQueue->uNumMessages;
                        // --i;
                        // if ( i < 0 )
                        return false;
                        // continue;
                    }
                }
                win->pCurrentPosActiveItem = v4;
                return true;
            }
            break;
        } else if (keyboardActionMapping->IsKeyMatchAction(Io::InputAction::DialogUp, key)) {
            int v22 = win->pCurrentPosActiveItem;
            int v23 = win->pStartingPosActiveItem;
            if (v22 <= v23)
                win->pCurrentPosActiveItem =
                    win->pNumPresenceButton + v23 - 1;
            else
                win->pCurrentPosActiveItem = v22 - 1;
            if (win->field_30 != 0) return true;
            GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
            engine->_messageQueue->addMessageCurrentFrame(pButton->msg, pButton->msg_param, 0);
            return true;
        } else if (keyboardActionMapping->IsKeyMatchAction(Io::InputAction::DialogSelect, key)) {
            GUIButton *pButton = win->GetControl(win->pCurrentPosActiveItem);
            engine->_messageQueue->addMessageCurrentFrame(pButton->msg, pButton->msg_param, 0);
        } else if (key == PlatformKey::KEY_PAGEDOWN) { // not button event from user, but a call from GUI_UpdateWindows to track mouse
            if (win->field_30 != 0) {
                int uClickX;
                int uClickY;
                EngineIocContainer::ResolveMouse()->GetClickPos(&uClickX, &uClickY);
                int v29 = win->pStartingPosActiveItem + win->pNumPresenceButton;
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
    }

    return 0;
}
