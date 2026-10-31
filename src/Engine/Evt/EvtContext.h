#pragma once

#include "Engine/Pid.h"
#include "Engine/Evt/EvtEnums.h"

/**
 * Which event runs, and what it runs for.
 */
struct EvtContext {
    EvtContext() = default;
    EvtContext(EvtSource source, int eventId, Pid objectPid = Pid()) : source(source), eventId(eventId), objectPid(objectPid) {}

    EvtSource source = EVT_SOURCE_MAP;
    int eventId = 0;

    /** Face or decoration that triggered the event, or empty when none did. */
    Pid objectPid;

    /** The loading screen is still up, so status texts are dropped and an NPC dialogue waits for the first frame,
     * like Archibald's on entering MM7's Land of the Giants, out12.evt event 1. */
    bool duringMapLoad = false;
};

/**
 * An event paused on a dialogue.
 */
struct EvtContinuation {
    EvtContinuation(EvtContext context, int step) : context(context), step(step) {}

    EvtContext context;
    int step = 0; // Step the event resumes from.
};
