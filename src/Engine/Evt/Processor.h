#pragma once

#include <functional>
#include <string>

#include "Engine/Pid.h"

class EvtInstruction;
class EvtScripts;
struct LevelDecoration;

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
 * Goes on with the event that stopped for a dialogue, from `savedEventID` and `savedEventStep`, once the dialogue
 * closes and lets it.
 */
void continueSavedEvent();

/**
 * Called when a dialogue that an event stopped for closes without letting the event go on.
 */
void cancelSavedEvent();

bool npcDialogueEventProcessor(int eventId, int startStep = 0);
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
 * @param value                         Scripted event handlers to run next to the evt ones, or `nullptr` for none.
 */
void setEvtScripts(EvtScripts *value);

EvtScripts *evtScripts();

/**
 * Registers a timer for the current map. It's dropped when the party leaves the map, like the evt timers.
 *
 * @param timer                         `EVENT_OnTimer` or `EVENT_OnLongTimer` instruction that says when to fire.
 * @param callback                      What to call when it fires.
 */
void addTimer(const EvtInstruction &timer, std::function<void()> callback);

extern int savedEventID;
extern int savedEventStep;
extern LevelDecoration *savedDecoration;
