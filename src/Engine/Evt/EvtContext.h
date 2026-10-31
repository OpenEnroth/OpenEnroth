#pragma once

#include "Engine/Pid.h"
#include "Engine/Evt/EvtEnums.h"

/**
 * Which event runs, and what it runs for.
 */
struct EvtContext {
    EvtSource source = EVT_SOURCE_MAP;
    int eventId = 0;
    Pid objectPid; // Face or decoration that triggered the event, or empty when none did.
    bool duringMapLoad = false; // The loading screen is still up, so status texts are dropped and an NPC dialogue waits for the first frame.
};

/**
 * An event paused on a dialogue.
 */
struct EvtContinuation {
    EvtContinuation(EvtContext context, int step) : context(context), step(step) {}

    EvtContext context;
    int step = 0; // Step the event resumes from.
};
