#include "MainMenuState.h"

#include <Media/Audio/AudioPlayer.h>
#include <Engine/Engine.h>
#include <Engine/SaveLoad.h>
#include <GUI/GUIMessageQueue.h>
#include <GUI/GUIWindow.h>
#include <Library/Logger/Logger.h>
#include <Engine/Graphics/Renderer/Renderer.h>

#include <memory>

MainMenuState::MainMenuState() {
}

FsmAction MainMenuState::enter() {
    pAudioPlayer->stopSounds();
    pAudioPlayer->MusicPlayTrack(MUSIC_MAIN_MENU);

    current_screen_type = SCREEN_GAME;

    pGUIWindow_BranchlessDialogue = nullptr;

    _mainMenuUI = std::make_unique<GUIWindow_MainMenu>();

    // In the future we won't need a concept of "CurrentMenuID" if all the states are managed inside an Fsm
    SetCurrentMenuID(MENU_MAIN);

    return FsmAction::none();
}

FsmAction MainMenuState::update() {
    std::string_view transition;
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType messageType;
        engine->_messageQueue->popMessage(&messageType, nullptr, nullptr);

        _mainMenuUI->processMessage(messageType);

        switch (messageType) {
        case UIMSG_MainMenu_ShowPartyCreationWnd:
            SetCurrentMenuID(MENU_NEWGAME);
            transition = "newGame";
            break;
        case UIMSG_MainMenu_ShowLoadWindow:
            SetCurrentMenuID(MENU_SAVELOAD);
            transition = "loadGame";
            break;
        case UIMSG_ShowCredits:
            SetCurrentMenuID(MENU_CREDITS);
            transition = "credits";
            break;
        case UIMSG_ExitToWindows:
            SetCurrentMenuID(MENU_EXIT_GAME);
            transition = "exitGame";
            break;
        case UIMSG_QuickLoad: {
            int slot = GetQuickSaveSlot();
            if (slot != -1) {
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                pSavegameList->selectedSlot = slot;
                SetCurrentMenuID(MENU_LoadingProcInMainMenu);
                transition = "quickLoadGame";
            } else {
                logger->debug("UIMSG_QuickLoad - No quick save could be found!");
                pAudioPlayer->playUISound(SOUND_error);
            }
            break;
        }
        default:
            break;
        }
    }

    if (!transition.empty()) {
        // TODO(Gerark) Remove this GUI_UpdateWindows once we have a proper Retained Mode UI system.
        // Right now we're forced to call this to cause the proper removal of temporary "buttons"
        GUI_UpdateWindows();
        return FsmAction::transition(std::exchange(transition, ""));
    }

    return FsmAction::none();
}

void MainMenuState::exit() {
    _mainMenuUI->Release();
    _mainMenuUI = nullptr;
}
