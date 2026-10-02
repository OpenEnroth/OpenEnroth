#pragma once

#include <string>

#include "Engine/Pid.h"
#include "Engine/Evt/EvtContext.h"

/**
 * An event paused on a dialogue.
 */
struct EvtContinuation {
    EvtContext context;
    int step = 0; // Step the event resumes from.
};

/**
 * @offset 0x4613C4
 */
void initDecorationEvents();

/**
 * @offset 0x46CC4B
 */
void checkDecorationEvents();

void eventProcessor(int eventId, Pid targetObj, bool canShowMessages, int startStep = 0);

/**
 * Runs an event from global.evt.
 *
 * @param eventId                       Event to run.
 * @param targetObj                     Interactive decoration the event runs for, or an empty pid for an NPC topic.
 */
void globalEventProcessor(int eventId, Pid targetObj);

/**
 * @param continuation                  Event to resume when the dialogue it paused on closes.
 */
void setEventContinuation(const EvtContinuation &continuation);

/**
 * Resumes the stored continuation, if there is one, and clears it.
 */
void runEventContinuation();

/**
 * Clears the stored continuation without resuming it.
 */
void dropEventContinuation();

bool npcDialogueEventProcessor(int eventId, int startStep = 0);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();
