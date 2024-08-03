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

FsmAction LoadSlotState::enter() {
    current_screen_type = SCREEN_LOADGAME;
    bool isInGame = false;
    _uiLoadSaveSlot = std::make_unique<GUIWindow_Load>(isInGame);
    // Unfortunately there's a need to set this global pointer if we don't want to refactor the entire SaveLoad UI ( not worth it right now )
    pGUIWindow_CurrentMenu = _uiLoadSaveSlot.get();
    return FsmAction::none();
}

FsmAction LoadSlotState::update() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType message;
        int param1, param2;
        engine->_messageQueue->popMessage(&message, &param1, &param2);
        switch (message) {
        case UIMSG_LoadGame: {
            if (!pSavegameList->pSavegameUsedSlots[pSavegameList->selectedSlot]) {
                break;
            }
            SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            return FsmAction::transition("slotConfirmed");
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
            SetCurrentMenuID(MENU_MAIN);
            current_screen_type = SCREEN_GAME;
            return FsmAction::transition("back");
        }
        case UIMSG_SaveLoadScroll: {
            _uiLoadSaveSlot->scroll(param1);
            break;
        }
        default:
            break;
        }
    }

    return FsmAction::none();
}

void LoadSlotState::exit() {
    pGUIWindow_CurrentMenu = nullptr;
    _uiLoadSaveSlot->Release();
    _uiLoadSaveSlot.reset();
}
