#include "Processor.h"

#include <vector>
#include <string>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Party.h"

#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

/**
 * When a timer fires.
 */
struct EvtTimerSchedule {
    Duration interval; // Counted from the last time it fired. Zero for a timer that follows the calendar.
    Duration period; // Of a timer that follows the calendar: a day, a week, 28 days or a year.
    Duration timeOfDay; // Of a daily timer.
};

struct MapTimer {
    EvtTimerSchedule schedule;
    Time alarmTime;
    int eventId = 0;
    int eventStep = 0;
};

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

static std::vector<MapTimer> onLongTimerTriggers;
static std::vector<MapTimer> onTimerTriggers;

static std::vector<int> decorationsWithEvents;

// Was in original code and ensures that timers are checked not more often than 30 game seconds.
// Do not needed in practice but can be considered optimization to avoid checking timers too often.
static Time timerGuard;

void initDecorationEvents() {
    DecorationId id = pDecorationTable->decorationId("Event Trigger");

    decorationsWithEvents.clear();
    for (int i = 0; i < pLevelDecorations.size(); ++i) {
        if (pLevelDecorations[i].uDecorationDescID == id) {
            decorationsWithEvents.push_back(i);
        }
    }
}

void checkDecorationEvents() {
    for (int decorationId : decorationsWithEvents) {
        const LevelDecoration &decoration = pLevelDecorations[decorationId];

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_TOUCH)
            if ((decoration.vPosition - pParty->pos).length() < decoration.uTriggerRange)
                runEvent(EvtContext(EVT_SOURCE_MAP, decoration.uEventID, Pid(OBJECT_Decoration, decorationId)));

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_MONSTER)
            for (const Actor &actor : pActors)
                if ((decoration.vPosition - actor.pos).length() < decoration.uTriggerRange)
                    runEvent(EvtContext(EVT_SOURCE_MAP, decoration.uEventID));

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_OBJECT)
            for (const SpriteObject &object : pSpriteObjects)
                if ((decoration.vPosition - object.vPosition).length() < decoration.uTriggerRange)
                    runEvent(EvtContext(EVT_SOURCE_MAP, decoration.uEventID));
    }
}

static EvtTimerSchedule timerSchedule(const EvtInstruction &ir) {
    EvtTimerSchedule result;
    if (ir.data.timer_descr.alt_halfmin_interval) {
        result.interval = Duration::fromSeconds(ir.data.timer_descr.alt_halfmin_interval * 30);
    } else if (ir.data.timer_descr.is_yearly) {
        result.period = Duration::fromYears(1);
    } else if (ir.data.timer_descr.is_monthly) {
        result.period = Duration::fromDays(28);
    } else if (ir.data.timer_descr.is_weekly) {
        result.period = Duration::fromDays(7);
    } else {
        result.period = Duration::fromDays(1);
        result.timeOfDay = Duration::fromHours(ir.data.timer_descr.daily_start_hour);
        result.timeOfDay += Duration::fromMinutes(ir.data.timer_descr.daily_start_minute);
        result.timeOfDay += Duration::fromSeconds(ir.data.timer_descr.daily_start_second);
    }
    return result;
}

/**
 * @param schedule                      When the timer fires.
 * @param levelLastVisit                Last visit to the current map.
 * @return                              When it fires first.
 */
static Time firstAlarm(const EvtTimerSchedule &schedule, Time levelLastVisit) {
    if (schedule.interval)
        return pParty->GetPlayingTime() + schedule.interval;

    assert(schedule.period > 0_ticks);
    if (schedule.period == Duration::fromDays(1)) {
        if (!levelLastVisit)
            return Time(); // Set alarm time to zero because it must always fire.

        // Calculate alarm time inside last visit day
        Time result = Time::fromDays(levelLastVisit.toDays()) + schedule.timeOfDay;
        return result < levelLastVisit ? result + Duration::fromDays(1) : result; // Last visit time already passed alarm time inside that day so move alarm to next day.
    }

    return levelLastVisit ? levelLastVisit + schedule.period : pParty->GetPlayingTime(); // Without last visit all timers must fire immediately.
}

static void registerTimerTriggers(EvtOpcode triggerType, std::vector<MapTimer> *triggers) {
    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    triggers->clear();
    for (const EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
        MapTimer timer;
        timer.schedule = timerSchedule(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep));
        timer.alarmTime = firstAlarm(timer.schedule, levelLastVisit);
        timer.eventId = trigger.eventId;
        timer.eventStep = trigger.eventStep;
        triggers->push_back(timer);
    }
}

EvtTransition runEvent(EvtContext context, int startStep) {
    if (!context.eventId) {
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    EvtInterpreter interpreter;
    MM_TRACE("Executing event starting from step {}", startStep);
    interpreter.prepare(context);

    if (!interpreter.isValid()) {
        MM_WARNING("Face has invalid event ID");
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    EvtTransition transition = interpreter.execute(startStep);
    if (transition == EVT_TRANSITION_STARTED)
        onMapLeave();
    return transition;
}

bool canShowTopic(int eventId) {
    EvtInterpreter interpreter;
    interpreter.prepare(EvtContext(EVT_SOURCE_GLOBAL, eventId));
    return interpreter.canShowTopic();
}

bool hasEventHint(int eventId) {
    return engine->_localEventMap.hasHint(eventId);
}

std::string getEventHintString(int eventId) {
    return engine->_localEventMap.hint(eventId);
}

static void registerEventTriggers() {
    onMapLoadTriggers.clear();
    onMapLoadTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapReload);
    onMapLeaveTriggers.clear();
    onMapLeaveTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapLeave);

    registerTimerTriggers(EVENT_OnLongTimer, &onLongTimerTriggers);
    registerTimerTriggers(EVENT_OnTimer, &onTimerTriggers);
}

void onMapLoad() {
    // Register all triggers when map done loading
    registerEventTriggers();

    timerGuard = pParty->GetPlayingTime();

    for (EventTrigger &triggers : onMapLoadTriggers)
        runEvent(EvtContext(EVT_SOURCE_MAP, triggers.eventId, Pid(), /* duringMapLoad = */ true), triggers.eventStep + 1);
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers)
        runEvent(EvtContext(EVT_SOURCE_MAP, triggers.eventId), triggers.eventStep + 1);

    // Cleanup timers to avoid firing while map transition is in process
    onLongTimerTriggers.clear();
    onTimerTriggers.clear();
}

/**
 * Fires the timers of a list that are due, until one of them starts a map transition.
 *
 * @param timers                        Timers to check.
 * @return                              Whether a timer started a map transition.
 */
static EvtTransition checkTimers(std::vector<MapTimer> *timers) {
    for (MapTimer &timer : *timers) {
        if (pParty->GetPlayingTime() < timer.alarmTime)
            continue;

        if (runEvent(EvtContext(EVT_SOURCE_MAP, timer.eventId), timer.eventStep + 1) == EVT_TRANSITION_STARTED)
            return EVT_TRANSITION_STARTED; // Leaving the map cleared the timers.

        if (timer.schedule.interval) {
            timer.alarmTime = pParty->GetPlayingTime() + timer.schedule.interval;
        } else {
            if (!timer.alarmTime.isValid() && timer.schedule.period == Duration::fromDays(1)) {
                // Initial firing of daily timers, next alarm must be configured to fire on exact time of day
                timer.alarmTime = Time() + timer.schedule.timeOfDay;
            }
            while (pParty->GetPlayingTime() >= timer.alarmTime) {
                timer.alarmTime += timer.schedule.period;
            }
        }
    }
    return EVT_TRANSITION_NONE;
}

void onTimer() {
    if (gameTimer->isPaused()) {
        return;
    }

    if ((pParty->GetPlayingTime() - timerGuard) < Duration::fromSeconds(30)) { // 30 game seconds = 1 realtime second.
        return;
    }

    timerGuard = pParty->GetPlayingTime();

    if (checkTimers(&onTimerTriggers) == EVT_TRANSITION_NONE)
        checkTimers(&onLongTimerTriggers);
}
