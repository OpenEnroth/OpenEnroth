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

    auto fsm = fsmBuilder.build();
    fsm->jumpToState("Start");
    return fsm;
}

void GameFsmBuilder::_buildIntroVideoSequence(FsmBuilder &builder) {
    builder
    .state<StartState>("Start")
        .on("proceed").jumpTo("3DOVideo")

    .state<VideoState>("3DOVideo", "3dologo", VideoState::LOGO_GROUP)
        .on("videoEnd").jumpTo("NWCVideo")

    .state<VideoState>("NWCVideo", "new world logo", VideoState::LOGO_GROUP)
        .on("videoEnd").jumpTo("JVCVideo")

    .state<VideoState>("JVCVideo", "jvc", VideoState::LOGO_GROUP)
        .on("videoEnd")
            .jumpTo([]() { return !engine->config->debug.NoIntro.value(); }, "IntroVideo")
            .jumpTo(Fsm::exitState)

    .state<VideoState>("IntroVideo", "Intro", VideoState::INTRO_GROUP)
        .on("videoEnd").jumpTo(Fsm::exitState);
}
