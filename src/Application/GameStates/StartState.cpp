#include "StartState.h"

#include <Engine/Engine.h>

StartState::StartState() {
}

FsmAction StartState::enter() {
    auto &debug = engine->config->debug;
    if(debug.NoVideo.value() || (debug.NoLogo.value() && debug.NoIntro.value()))
        return FsmActionTransition("skipVideo");
    else if (debug.NoLogo.value())
        return FsmActionTransition("skipLogo");

    return FsmActionTransition("noSkip");
}

FsmAction StartState::update() {
    return FsmActionNone();
}

void StartState::exit() {
}
