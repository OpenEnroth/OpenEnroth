#include "StartState.h"

#include <Engine/Engine.h>

StartState::StartState() {
}

FSMAction StartState::enter() {
    auto &debug = engine->config->debug;
    if(debug.NoVideo.value() || (debug.NoLogo.value() && debug.NoIntro.value()))
        return FSMActionTransition("skipVideo");
    else if (debug.NoLogo.value())
        return FSMActionTransition("skipLogo");

    return FSMActionTransition("noSkip");
}

FSMAction StartState::update() {
    return FSMActionNone();
}

void StartState::exit() {
}
