#include "GameFsmBuilder.h"

#include <Engine/Engine.h>
#include <Library/Fsm/FsmBuilder.h>

#include <utility>
#include <memory>

#include "VideoState.h"
#include "StartState.h"

std::unique_ptr<Fsm> GameFsmBuilder::buildFsm() {
    FsmBuilder fsmBuilder;
    _buildIntroVideoSequence(fsmBuilder);
    auto fsm = fsmBuilder.build("Start");
    return fsm;
}

void GameFsmBuilder::_buildIntroVideoSequence(FsmBuilder &builder) {
    builder
    .state<StartState>("Start")
        .on("proceed").jumpTo("3DOVideo")

    .state<VideoState>("3DOVideo", VideoState::VIDEO_LOGO, "3dologo")
        .on("videoEnd").jumpTo("NWCVideo")

    .state<VideoState>("NWCVideo", VideoState::VIDEO_LOGO, "new world logo")
        .on("videoEnd").jumpTo("JVCVideo")

    .state<VideoState>("JVCVideo", VideoState::VIDEO_LOGO, "jvc")
        .on("videoEnd").jumpTo("IntroVideo")

    .state<VideoState>("IntroVideo", VideoState::VIDEO_INTRO, "Intro")
        .on("videoEnd").exitFsm();
}
/*
#include "GameFSMBuilder.h"

#include <Engine/Engine.h>
#include <Library/Fsm/FSMBuilder.h>

#include <utility>
#include <memory>

#include "VideoState.h"
#include "LoadStep2State.h"
#include "StartState.h"

std::unique_ptr<FSM> GameFSMBuilder::buildFSM() {
    FSMBuilder fsmBuilder;
    _buildIntroVideoSequence(fsmBuilder);

    auto fsm = fsmBuilder.build();
    fsm->jumpToState("Start");
    return fsm;
}

void GameFSMBuilder::_buildIntroVideoSequence(FSMBuilder &builder) {
    builder
    .state<StartState>("Start")
        .on("skipVideo").jumpTo("LoadStep2")
        .on("skipLogo").jumpTo("IntroVideo")
        .on("noSkip").jumpTo("3DOVideo")

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
*/