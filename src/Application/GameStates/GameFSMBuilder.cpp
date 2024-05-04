#include "GameFSMBuilder.h"

#include <Engine/Engine.h>
#include <Library/Fsm/FSMBuilder.h>

#include <utility>
#include <memory>

#include "VideoState.h"
#include "LoadStep2State.h"

std::unique_ptr<FSM> GameFSMBuilder::buildFSM() {
    FSMBuilder fsmBuilder;
    _buildIntroVideoSequence(fsmBuilder);

    auto fsm = fsmBuilder.build();
    _setStartingState(*fsm);
    return std::move(fsm);
}

void GameFSMBuilder::_setStartingState(FSM &fsm) {
    GameConfig::Debug &debugConfig = engine->config->debug;
    std::string_view initialState = "LoadStep2";
    if (!debugConfig.NoVideo.value()) {
        if (!debugConfig.NoLogo.value()) {
            initialState = "3DOVideo";
        } else if (!debugConfig.NoIntro.value()) {
            initialState = "IntroVideo";
        }
    }
    fsm.jumpToState(initialState);
}

void GameFSMBuilder::_buildIntroVideoSequence(FSMBuilder &builder) {
    builder
    .state<VideoState>("3DOVideo", "3dologo")
        .on("videoEnd").jumpTo("NWCVideo")

    .state<VideoState>("NWCVideo", "new world logo")
        .on("videoEnd").jumpTo("JVCVideo")

    .state<VideoState>("JVCVideo", "jvc")
        .on("videoEnd")
            .jumpTo([]() { return !engine->config->debug.NoIntro.value(); }, "IntroVideo")
            .jumpTo(FSM::exitState)

    .state<VideoState>("IntroVideo", "Intro")
        .on("videoEnd").jumpTo("LoadStep2")

    .state<LoadStep2State>("LoadStep2")
        .on("done").jumpTo(FSM::exitState);
}
