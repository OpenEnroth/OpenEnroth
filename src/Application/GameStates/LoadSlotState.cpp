#include "LoadSlotState.h"

#include <Engine/Engine.h>
#include <Engine/Graphics/Renderer/Renderer.h>
#include <Engine/SaveLoad.h>
#include <GUI/GUIMessageQueue.h>
#include <GUI/GUIWindow.h>
#include <GUI/UI/UISaveLoad.h>

#include <memory>

LoadSlotState::LoadSlotState() {
}

void LoadSlotState::enter() {
    current_screen_type = SCREEN_LOADGAME;
    bool isInGame = false;
    _uiLoadSaveSlot = std::make_unique<GUIWindow_Load>(isInGame);
    // Unfortunately there's a need to set this global pointer if we don't want to refactor the entire SaveLoad UI ( not worth it right now )
    pGUIWindow_CurrentMenu = _uiLoadSaveSlot.get();
}

void LoadSlotState::update() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType message;
        int param1, param2;
        engine->_messageQueue->popMessage(&message, &param1, &param2);
        switch (message) {
        case UIMSG_LoadGame: {
            if (!pSavegameList->pSavegameUsedSlots[pSavegameList->selectedSlot]) {
                break;
            }
            executeTransition("slotConfirmed");
            SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            break;
        }
        case UIMSG_SelectLoadSlot: {
            _uiLoadSaveSlot->slotSelected(param1);
            break;
        }
        case UIMSG_SaveLoadBtn: {
            _uiLoadSaveSlot->loadButtonPressed();
            break;
        }
        case UIMSG_DownArrow: {
            _uiLoadSaveSlot->downArrowPressed(param1);
            break;
        }
        case UIMSG_ArrowUp: {
            _uiLoadSaveSlot->upArrowPressed();
            break;
        }
        case UIMSG_Cancel: {
            _uiLoadSaveSlot->cancelButtonPressed();
            break;
        }
        case UIMSG_Escape: {
            pEventTimer->setPaused(false);
            _goBack();
            break;
        }
        case UIMSG_SaveLoadScroll: {
            _uiLoadSaveSlot->scroll(param1);
            break;
        }
        default:
            break;
        }
    }
}

void LoadSlotState::exit() {
    pGUIWindow_CurrentMenu = nullptr;
    _uiLoadSaveSlot->Release();
    _uiLoadSaveSlot.reset();
}

void LoadSlotState::_goBack() {
    // One day we'll be able to get rid of this. We shouldn't know from a state where we're going.
    // That's why we trigger a "back" transition few lines below
    SetCurrentMenuID(MENU_MAIN);
    current_screen_type = SCREEN_GAME;

    executeTransition("back");
}
