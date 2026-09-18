#pragma once

#include <functional>
#include <string>

#include "Engine/Evt/EvtContext.h"

class EvtInstruction;
class EvtScripts;

/**
 * @offset 0x4613C4
 */
void initDecorationEvents();

/**
 * @offset 0x46CC4B
 */
void checkDecorationEvents();

/**
 * @param context                       Event to run, and what it runs for.
 * @param startStep                     Step to start from.
 * @return                              Whether the event started a map transition. If it did, the map's leave
 *                                      events have already run.
 */
EvtTransition runEvent(EvtContext context, int startStep = 0);

/**
 * @param eventId                       Global event behind an NPC's scripted topic.
 * @return                              Whether the topic is listed in the NPC's dialogue.
 */
bool canShowTopic(int eventId);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

/**
 * Called when a game is started or loaded. Brings the global events back to what global.evt holds and lets the
 * scripts change them again.
 */
void onGameLoad();

void onMapLoad();
void onMapLeave();
void onTimer();

/**
 * @param scripts                       Scripted event handlers to run next to the evt ones, or `nullptr` for none.
 */
void setEvtScripts(EvtScripts *scripts);

EvtScripts *evtScripts();

/**
 * Registers a timer for the current map. It's dropped when the party leaves the map, like the evt timers.
 *
 * @param timer                         `EVENT_OnTimer` or `EVENT_OnLongTimer` instruction that says when to fire.
 * @param callback                      What to call when it fires.
 */
void addTimer(const EvtInstruction &timer, std::function<void()> callback);
