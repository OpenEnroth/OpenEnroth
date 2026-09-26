#pragma once

#include <functional>
#include <optional>
#include <string>

#include "Engine/Evt/EvtContext.h"

#include "Core/Time/Duration.h"

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
    EvtTimerType type = EVT_TIMER_INTERVAL;
    Duration duration;
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
 * @param start                         What the first alarm counts from.
 * @param kind                          Which timers it's checked with.
 * @param lifetime                      How long it's kept.
 * @param callback                      What to call when it fires.
 * @return                              Handle of the timer, for `removeTimer`.
 */
int addTimer(const EvtTimerSchedule &schedule, EvtTimerStart start, EvtTimerKind kind, EvtTimerLifetime lifetime, std::function<void()> callback);

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
