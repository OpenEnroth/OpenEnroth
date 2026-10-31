#pragma once

#include "Engine/Pid.h"
#include "Engine/Evt/EvtEnums.h"

/**
 * Which event runs, and what it runs for.
 */
struct EvtContext {
    EvtSource source = EVT_SOURCE_MAP;
    int eventId = 0;
    Pid objectPid; // Object that triggered the event. For a global event, an interactive decoration, or empty for an NPC topic.
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
