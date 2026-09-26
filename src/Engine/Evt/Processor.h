#pragma once

#include <functional>
#include <optional>
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
 * @param continuation                  What goes on with the event that stops for the dialogue it opened, once the
 *                                      dialogue closes and lets it. Empty if nothing waits for the dialogue.
 */
void setEventContinuation(std::function<void()> continuation);

bool hasEventContinuation();

/**
 * Called when a dialogue that an event stopped for closes and lets the event go on.
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
 * What a timer counts its first alarm from.
 */
enum class EvtTimerStart {
    EVT_TIMER_START_LAST_VISIT, // The last visit to the map, as evt timers do when their map loads. Fires at once on a first visit.
    EVT_TIMER_START_NOW,
};
using enum EvtTimerStart;

/**
 * When a timer fires.
 */
struct EvtTimerSchedule {
    Duration interval; // Counted from when the timer is added. Zero for a timer that follows the calendar.
    Duration period; // Of a timer that follows the calendar: a day, a week, 28 days or a year.
    Duration timeOfDay; // Of a daily timer that follows the calendar.
    EvtTimerStart start = EVT_TIMER_START_LAST_VISIT; // Of a timer that follows the calendar.
};

/**
 * The engine checks all the regular timers before any refill timer, as vanilla does with `EVENT_OnTimer` and
 * `EVENT_OnLongTimer`.
 */
enum class EvtTimerKind {
    EVT_TIMER_KIND_REGULAR,
    EVT_TIMER_KIND_REFILL,
};
using enum EvtTimerKind;

enum class EvtTimerLifetime {
    EVT_TIMER_LIFETIME_MAP, // Dropped when the party leaves the map, like the evt timers.
    EVT_TIMER_LIFETIME_GAME, // Kept until a game is loaded or started.
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

/**
 * Drops the timers and the map load and leave triggers that the current map's evt file registered for an event, once
 * a script has removed the event.
 *
 * @param eventId                       Event, or `std::nullopt` for all of them.
 */
void removeEventTriggers(std::optional<int> eventId);

extern LevelDecoration *savedDecoration;
