#include "MainMenuState.h"

#include <Media/Audio/AudioPlayer.h>
#include <Engine/Engine.h>
#include <GUI/GUIMessageQueue.h>
#include <GUI/GUIWindow.h>
#include <GUI/UI/UIMainMenu.h>

#include <thread>

MainMenuState::MainMenuState() {
}

void MainMenuState::enter() {
    pAudioPlayer->stopSounds();
    pAudioPlayer->MusicPlayTrack(MUSIC_MAIN_MENU);

    current_screen_type = SCREEN_GAME;

    pGUIWindow_BranchlessDialogue = nullptr;

    _mainMenuUI = std::make_unique<GUIWindow_MainMenu>();

    // In the future we won't need a concept of "CurrentMenuID" if all the states are managed inside an FSM
    SetCurrentMenuID(MENU_MAIN);
}

void MainMenuState::update() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType messageType;
        engine->_messageQueue->popMessage(&messageType, nullptr, nullptr);
        
        _mainMenuUI->processMessage(messageType);

        switch (messageType) {
        case UIMSG_MainMenu_ShowPartyCreationWnd:
            executeTransition("newGame");
            SetCurrentMenuID(MENU_NEWGAME);
            break;
        case UIMSG_MainMenu_ShowLoadWindow:
            executeTransition("loadGame");
            SetCurrentMenuID(MENU_SAVELOAD);
            break;
        case UIMSG_ShowCredits:
            executeTransition("credits");
            SetCurrentMenuID(MENU_CREDITS);
            break;
        case UIMSG_ExitToWindows:
            executeTransition("exit");
            SetCurrentMenuID(MENU_EXIT_GAME);
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        default:
            break;
        }
    }
    _mainMenuUI->Update();
}

void MainMenuState::exit() {
    _mainMenuUI->Release();
    _mainMenuUI = nullptr;
}
