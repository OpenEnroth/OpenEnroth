#pragma once

#include <functional>
#include <string>

#include "Engine/Pid.h"

#include "Core/Time/Duration.h"

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
 * When a timer fires.
 */
struct EvtTimerSchedule {
    Duration interval; // Counted from when the timer is added. Zero for a timer that follows the calendar.
    Duration period; // Of a timer that follows the calendar: a day, a week, 28 days or a year.
    Duration timeOfDay; // Of a daily timer that follows the calendar.
};

/**
 * The engine checks all the regular timers before any refill timer, as vanilla does with `EVENT_OnTimer` and
 * `EVENT_OnLongTimer`.
 */
enum class EvtTimerKind {
    EVT_TIMER_REGULAR,
    EVT_TIMER_REFILL,
};
using enum EvtTimerKind;

enum class EvtTimerLifetime {
    EVT_TIMER_MAP, // Dropped when the party leaves the map, like the evt timers.
    EVT_TIMER_GAME, // Kept until a game is loaded or started.
};
using enum EvtTimerLifetime;

/**
 * @param schedule                      When the timer fires.
 * @param kind                          Which timers it's checked with.
 * @param lifetime                      How long it's kept.
 * @param callback                      What to call when it fires.
 * @return                              Handle of the timer, for `removeTimer`.
 */
int addTimer(const EvtTimerSchedule &schedule, EvtTimerKind kind, EvtTimerLifetime lifetime, std::function<void()> callback);

/**
 * @param handle                        Timer to remove, as `addTimer` returned it. The timer can be the one that is
 *                                      firing.
 */
void removeTimer(int handle);

extern int savedEventID;
extern int savedEventStep;
extern LevelDecoration *savedDecoration;
