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
#include "Engine/Evt/EvtEnumFunctions.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Party.h"

#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

/**
 * When a timer fires.
 */
struct EvtTimerSchedule {
    EvtTimerType type = EVT_TIMER_INTERVAL;
    Duration duration;
};

struct MapTimer {
    EvtTimerSchedule schedule;
    Time alarmTime;
    int eventId = 0;
    int eventStep = 0;
};

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

static std::vector<MapTimer> mapTimers;

static std::vector<int> decorationsWithEvents;

// Was in original code and ensures that timers are checked not more often than 30 game seconds.
// Do not needed in practice but can be considered optimization to avoid checking timers too often.
static Time timerGuard;

void initDecorationEvents() {
    DecorationId id = pDecorationTable->decorationId("Event Trigger");

    decorationsWithEvents.clear();
    for (int i = 0; i < pLevelDecorations.size(); ++i)
        if (pLevelDecorations[i].uDecorationDescID == id)
            decorationsWithEvents.push_back(i);
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
    assert(ir.opcode == EVENT_OnTimer || ir.opcode == EVENT_OnLongTimer);

    EvtTimerSchedule result;
    if (ir.data.timer_descr.alt_halfmin_interval) {
        result.type = EVT_TIMER_INTERVAL;
        result.duration = Duration::fromSeconds(ir.data.timer_descr.alt_halfmin_interval * 30);
    } else if (ir.data.timer_descr.is_yearly) {
        result.type = EVT_TIMER_YEARLY;
    } else if (ir.data.timer_descr.is_monthly) {
        result.type = EVT_TIMER_MONTHLY;
    } else if (ir.data.timer_descr.is_weekly) {
        result.type = EVT_TIMER_WEEKLY;
    } else {
        result.type = EVT_TIMER_DAILY;
        result.duration = Duration::fromHours(ir.data.timer_descr.daily_start_hour) + Duration::fromMinutes(ir.data.timer_descr.daily_start_minute) +
                          Duration::fromSeconds(ir.data.timer_descr.daily_start_second);
    }
    return result;
}

/**
 * @param schedule                      When the timer fires.
 * @param levelEnterTime                When the party entered the current map.
 * @param levelLastVisit                Last visit to the current map.
 * @return                              When it fires first.
 */
static Time firstAlarm(const EvtTimerSchedule &schedule, Time levelEnterTime, Time levelLastVisit) {
    switch (schedule.type) {
    case EVT_TIMER_INTERVAL:
        return levelEnterTime + schedule.duration;
    case EVT_TIMER_DAILY: {
        if (!levelLastVisit)
            return levelEnterTime;

        Time result = Time::fromDays(levelLastVisit.toDays()) + schedule.duration;
        return result < levelLastVisit ? result + Duration::fromDays(1) : result;
    }
    default:
        return levelLastVisit ? levelLastVisit + periodForTimerType(schedule.type) : levelEnterTime; // Without last visit all timers must fire immediately.
    }
}

static void registerTimers() {
    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelEnterTime = pParty->GetPlayingTime();
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    mapTimers.clear();
    for (EvtOpcode triggerType : {EVENT_OnTimer, EVENT_OnLongTimer}) { // The `OnTimer` ones are checked first.
        for (const EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
            MapTimer timer;
            timer.schedule = timerSchedule(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep));
            timer.alarmTime = firstAlarm(timer.schedule, levelEnterTime, levelLastVisit);
            timer.eventId = trigger.eventId;
            timer.eventStep = trigger.eventStep;
            mapTimers.push_back(timer);
        }
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

    registerTimers();
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
    mapTimers.clear();
}

/**
 * Fires the timers that are due, until one of them starts a map transition.
 */
static void checkTimers() {
    for (MapTimer &timer : mapTimers) {
        if (pParty->GetPlayingTime() < timer.alarmTime)
            continue;

        if (runEvent(EvtContext(EVT_SOURCE_MAP, timer.eventId), timer.eventStep + 1) == EVT_TRANSITION_STARTED)
            return; // Leaving the map cleared the timers.

        Time now = pParty->GetPlayingTime();
        if (timer.schedule.type == EVT_TIMER_INTERVAL) {
            timer.alarmTime = now + timer.schedule.duration;
        } else if (timer.schedule.type == EVT_TIMER_DAILY) {
            timer.alarmTime = Time::fromDays(now.toDays()) + timer.schedule.duration;
            if (timer.alarmTime <= now)
                timer.alarmTime += Duration::fromDays(1);
        } else {
            while (now >= timer.alarmTime)
                timer.alarmTime += periodForTimerType(timer.schedule.type);
        }
    }
}

void onTimer() {
    if (gameTimer->isPaused())
        return;

    if ((pParty->GetPlayingTime() - timerGuard) < Duration::fromSeconds(30)) // 30 game seconds = 1 realtime second.
        return;

    timerGuard = pParty->GetPlayingTime();

    checkTimers();
}
