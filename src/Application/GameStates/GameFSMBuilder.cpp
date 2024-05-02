#include "GameFSMBuilder.h"

#include <Engine/Engine.h>

#include <utility>
#include <memory>

#include "VideoState.h"

std::unique_ptr<FSM> GameFSMBuilder::buildFSM() {
    // Sequence of video being played when the game starts
    auto fsm = std::make_unique<FSM>();
    _buildIntroVideoSequence(*fsm);
    _setStartingState(*fsm);
    return std::move(fsm);
}

void GameFSMBuilder::_setStartingState(FSM &fsm) {
    GameConfig::Debug &debugConfig = engine->config->debug;
    if (!debugConfig.NoVideo.value()) {
        if (!debugConfig.NoLogo.value()) {
            fsm.jumpToState("3DOVideo");
        } else if (!debugConfig.NoIntro.value()) {
            fsm.jumpToState("IntroVideo");
        }
    }
}

void GameFSMBuilder::_buildIntroVideoSequence(FSM &fsm) {
    fsm.addState("3DOVideo", std::make_unique<VideoState>("3dologo"), {
        { "videoEnd", {{ "NWCVideo" }} }
    });
    fsm.addState("NWCVideo", std::make_unique<VideoState>("new world logo"), {
        { "videoEnd", {{ "JVCVideo" }} }
    });
    fsm.addState("JVCVideo", std::make_unique<VideoState>("jvc"), {
        // In this case when we're triggering the videoEnd transition we can move to 2 different states one of them has a transition
        { "videoEnd", {
            { "IntroVideo", []() { return !engine->config->debug.NoIntro.value(); }},
            { "_Exit" }
        }}
    });

    fsm.addState("IntroVideo", std::make_unique<VideoState>("Intro"), {
        { "videoEnd", {{ "_Exit" }} }
    });
}
