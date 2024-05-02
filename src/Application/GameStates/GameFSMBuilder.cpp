#include "GameFSMBuilder.h"

#include "VideoState.h"

std::unique_ptr<FSM> GameFSMBuilder::buildFSM() {
    // Sequence of video being played when the game starts
    auto fsm = std::make_unique<FSM>();
    _buildIntroVideoSequence(*fsm);

    fsm->setInitialState("3DOVideo");
    return std::move(fsm);
}

void GameFSMBuilder::_buildIntroVideoSequence(FSM &fsm) {
    fsm.addState("3DOVideo", std::make_unique<VideoState>("3dologo"), {
        { "videoEnd", "NWCVideo" }
    });
    fsm.addState("NWCVideo", std::make_unique<VideoState>("new world logo"), {
        { "videoEnd", "JVCVideo" }
    });
    fsm.addState("JVCVideo", std::make_unique<VideoState>("jvc"), {
        { "videoEnd", "IntroVideo" }
    });
    fsm.addState("IntroVideo", std::make_unique<VideoState>("Intro"), {
        { "videoEnd", "ExitFSM" }
    });
}