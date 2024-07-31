#include "CreditsState.h"

#include <Media/Audio/AudioPlayer.h>
#include <Engine/Engine.h>
#include <GUI/GUIMessageQueue.h>
#include <GUI/UI/UICredits.h>

#include <memory>

CreditsState::CreditsState() {
}

FsmAction CreditsState::enter() {
    pAudioPlayer->MusicStop();
    engine->_messageQueue->clear();

    pAudioPlayer->MusicPlayTrack(MUSIC_CREDITS);

    _uiCredits = std::make_unique<GUICredits>([this]() {
        _scheduleBackTransition = true;
    });
    current_screen_type = SCREEN_CREATORS;
    SetCurrentMenuID(MENU_CREDITSPROC);
    return FsmAction::none();
}

FsmAction CreditsState::update() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType messageType;
        engine->_messageQueue->popMessage(&messageType, nullptr, nullptr);

        if (messageType == UIMSG_Escape) {
            _scheduleBackTransition = true;
        }
    }

    if (_scheduleBackTransition) {
        _scheduleBackTransition = false;
        return FsmAction::transition("back");
    }

    return FsmAction::none();
}

void CreditsState::exit() {
    _uiCredits->Release();
    _uiCredits.reset();

    pAudioPlayer->MusicStop();
    pAudioPlayer->stopSounds();
    _scheduleBackTransition = false;
}
