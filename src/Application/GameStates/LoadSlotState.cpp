#include "LoadSlotState.h"

#include <memory>

#include "Engine/Engine.h"
#include "Engine/SaveLoad.h"
#include "Engine/Time/Timer.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UISaveLoad.h"
#include "Io/Mouse.h"

LoadSlotState::LoadSlotState() = default;
LoadSlotState::~LoadSlotState() = default;

FsmAction LoadSlotState::enter() {
    current_screen_type = SCREEN_LOADGAME;
    bool isInGame = false;
    // Unfortunately there's a need to set this global pointer if we don't want to refactor the entire SaveLoad UI ( not worth it right now )
    pGUIWindow_CurrentMenu = std::make_unique<GUIWindow_Load>(isInGame);
    _uiLoadSaveSlot = static_cast<GUIWindow_Load*>(pGUIWindow_CurrentMenu.get());
    return FsmAction::none();
}

FsmAction LoadSlotState::update() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType message;
        int param1, param2;
        engine->_messageQueue->popMessage(&message, &param1, &param2);
        switch (message) {
        case UIMSG_LoadGame: {
            if (!_uiLoadSaveSlot->hasSelectedSlot())
                break;
            engine->_pendingLoadFileName = _uiLoadSaveSlot->selectedSlot().fileName;
            SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            return FsmAction::transition("slotConfirmed");
        }
        case UIMSG_SelectLoadSlot: {
            _uiLoadSaveSlot->slotClicked(param1, param2);
            break;
        }
        case UIMSG_SaveLoadBtn: {
            _uiLoadSaveSlot->loadButtonPressed();
            break;
        }
        case UIMSG_DownArrow: {
            _uiLoadSaveSlot->downArrowPressed();
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
            gameTimer->setPaused(false);
            SetCurrentMenuID(MENU_MAIN);
            current_screen_type = SCREEN_GAME;
            return FsmAction::transition("back");
        }
        case UIMSG_SaveLoadScroll: {
            _uiLoadSaveSlot->scrollWithMouse(mouse->position());
            break;
        }
        case UIMSG_QuickLoad: {
            _uiLoadSaveSlot->quickLoad();
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
    _uiLoadSaveSlot = nullptr;
}
