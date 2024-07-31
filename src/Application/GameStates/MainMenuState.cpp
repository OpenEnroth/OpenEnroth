#include "MainMenuState.h"

#include <Media/Audio/AudioPlayer.h>
#include <Engine/Engine.h>
#include <Engine/SaveLoad.h>
#include <GUI/GUIMessageQueue.h>
#include <GUI/GUIWindow.h>
#include <GUI/UI/UIMainMenu.h>
#include <Library/Logger/Logger.h>

#include <memory>

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
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType messageType;
        engine->_messageQueue->popMessage(&messageType, nullptr, nullptr);

        _mainMenuUI->processMessage(messageType);

        switch (messageType) {
        case UIMSG_MainMenu_ShowPartyCreationWnd:
            SetCurrentMenuID(MENU_NEWGAME);
            return FsmAction::transition("newGame");
        case UIMSG_MainMenu_ShowLoadWindow:
            SetCurrentMenuID(MENU_SAVELOAD);
            return FsmAction::transition("loadGame");
        case UIMSG_ShowCredits:
            SetCurrentMenuID(MENU_CREDITS);
            return FsmAction::transition("credits");
        case UIMSG_ExitToWindows:
            SetCurrentMenuID(MENU_EXIT_GAME);
            return FsmAction::transition("exit");
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        case UIMSG_QuickLoad: {
            int slot = GetQuickSaveSlot();
            if (slot != -1) {
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                pSavegameList->selectedSlot = slot;
                SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            } else {
                logger->error("QuickLoadGame:: No quick save could be found!");
                pAudioPlayer->playUISound(SOUND_error);
            }
            break;
        }
        default:
            break;
        }
    }

    return FsmAction::none();
}

void MainMenuState::exit() {
    _mainMenuUI->Release();
    _mainMenuUI = nullptr;
}
