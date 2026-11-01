#pragma once

#include "Engine/Evt/EvtContext.h"

/**
 * An event paused on a dialogue.
 */
struct EvtContinuation {
    EvtContinuation(EvtContext context, int step) : context(context), step(step) {}

    EvtContext context;
    int step; // Step the event resumes from.
};
