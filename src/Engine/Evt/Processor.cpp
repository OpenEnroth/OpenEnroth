#include "Processor.h"

#include <algorithm>
#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Evt/EvtScripts.h"
#include "Engine/Party.h"

#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

struct MapTimer {
    EvtTimerSchedule schedule;
    std::optional<Time> alarmTime; // Not set until the level that `schedule` counts from has loaded.
    int eventId = 0;
    int eventStep = 0;
    int handle = 0; // The scripts' handle, 0 for an evt timer.
    EvtTimerLifetime lifetime = EVT_TIMER_LIFETIME_MAP;
    bool isRemoved = false; // Erased once no loop over the timers is running.
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
 * @return                              When it fires first.
 */
static Time firstAlarm(const EvtTimerSchedule &schedule) {
    Time now = pParty->GetPlayingTime();
    if (schedule.interval)
        return now + schedule.interval;

    assert(schedule.period > 0_ticks);
    bool isDaily = schedule.period == Duration::fromDays(1);
    if (schedule.start == EVT_TIMER_START_NOW) {
        if (!isDaily)
            return now + schedule.period;
        Time result = Time::fromDays(now.toDays()) + schedule.timeOfDay;
        return result <= now ? result + Duration::fromDays(1) : result;
    }

    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;
    if (!levelLastVisit)
        return isDaily ? Time() : now; // Without last visit all timers must fire immediately.
    if (!isDaily)
        return levelLastVisit + schedule.period;

    // Calculate alarm time inside last visit day
    Time result = Time::fromDays(levelLastVisit.toDays()) + schedule.timeOfDay;
    return result < levelLastVisit ? result + Duration::fromDays(1) : result; // Last visit time already passed alarm time inside that day so move alarm to next day.
}

static void registerTimerTriggers(EvtOpcode triggerType, std::vector<MapTimer> *triggers) {
    std::vector<MapTimer> evtTimers;
    for (EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
        MapTimer timer;
        timer.schedule = timerSchedule(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep));
        timer.alarmTime = firstAlarm(timer.schedule);
        timer.eventId = trigger.eventId;
        timer.eventStep = trigger.eventStep;
        evtTimers.push_back(std::move(timer));
    }
    triggers->insert(triggers->begin(), evtTimers.begin(), evtTimers.end()); // Before the scripts' timers, as evt events go first.

    for (MapTimer &timer : *triggers)
        if (!timer.alarmTime)
            timer.alarmTime = firstAlarm(timer.schedule);
}

static void removeTimers(std::function<bool(const MapTimer &)> predicate) {
    for (std::vector<MapTimer> *timers : {&onTimerTriggers, &onLongTimerTriggers})
        for (MapTimer &timer : *timers)
            if (predicate(timer))
                timer.isRemoved = true;
}

static void eraseRemovedTimers() {
    auto isRemoved = [](const MapTimer &timer) { return timer.isRemoved; };
    std::erase_if(onTimerTriggers, isRemoved);
    std::erase_if(onLongTimerTriggers, isRemoved);
}

/**
 * @param transition                    What an event reported.
 * @return                              The same. The map's leave events have run if a transition started.
 */
static EvtTransition leaveMapOn(EvtTransition transition) {
    if (transition == EVT_TRANSITION_STARTED)
        onMapLeave();
    return transition;
}

EvtTransition runEvent(EvtContext context, int startStep) {
    EvtScripts *scripts = engine->_evtScripts;

    if (context.scriptFrame)
        return leaveMapOn(scripts->resumeEvent(context.scriptFrame));

    if (!context.eventId) {
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    EvtInterpreter interpreter;
    MM_TRACE("Executing event starting from step {}", startStep);
    interpreter.prepare(context);

    bool isScripted = scripts && startStep == 0 && scripts->hasEvent(context.source, context.eventId);
    if (!interpreter.isValid() && !isScripted) {
        if (scripts && context.source == EVT_SOURCE_MAP && scripts->eventHint(context.eventId))
            return EVT_TRANSITION_NONE; // The scripts gave the event a hint and nothing to do, like an evt event that is a hint and an exit.
        MM_WARNING("Face has invalid event ID");
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    EvtTransition transition = interpreter.execute(startStep);
    if (isScripted && scripts->runEvent(context) == EVT_TRANSITION_STARTED)
        transition = EVT_TRANSITION_STARTED;
    return leaveMapOn(transition);
}

bool canShowTopic(int eventId) {
    EvtInterpreter interpreter;
    interpreter.prepare(EvtContext(EVT_SOURCE_GLOBAL, eventId));
    bool result = interpreter.canShowTopic();
    if (EvtScripts *scripts = engine->_evtScripts)
        result = scripts->canShowTopic(eventId).value_or(result);
    return result;
}

bool hasEventHint(int eventId) {
    EvtScripts *scripts = engine->_evtScripts;
    if (scripts && scripts->hasEvent(EVT_SOURCE_MAP, eventId))
        return false; // The event does more than show a hint.
    if (engine->_localEventMap.hasEvent(eventId))
        return engine->_localEventMap.hasHint(eventId);
    return scripts && scripts->eventHint(eventId);
}

std::string getEventHintString(int eventId) {
    if (EvtScripts *scripts = engine->_evtScripts)
        if (std::optional<std::string> hint = scripts->eventHint(eventId))
            return *hint;
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

void onGameLoad() {
    if (!engine->_evtScripts)
        return;

    engine->_globalEventMap = EvtProgram::load(engine->resources()->eventsData("global.evt"));
    removeTimers([](const MapTimer &timer) { return timer.handle != 0; }); // They belong to the game that is left.
    eraseRemovedTimers();
    engine->_evtScripts->loadGlobalScripts();

    for (std::vector<MapTimer> *timers : {&onTimerTriggers, &onLongTimerTriggers})
        for (MapTimer &timer : *timers)
            timer.alarmTime.reset(); // Counted when the first map of the game loads.
}

/**
 * Sets `FACE_EVENT_IS_HINT` on the faces of the current level whose event only shows a hint, and clears it on the
 * others.
 */
static void updateFaceHints() {
    auto update = [](BLVFace &face) {
        if (!face.eventId)
            return;
        if (hasEventHint(face.eventId)) {
            face.attributes |= FACE_EVENT_IS_HINT;
        } else {
            face.attributes &= ~FACE_EVENT_IS_HINT;
        }
    };

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        std::ranges::for_each(pIndoor->faces, update);
    } else {
        std::ranges::for_each(pOutdoor->faces, update);
    }
}

void onMapLoad() {
    removeTimers([](const MapTimer &timer) { return timer.lifetime == EVT_TIMER_LIFETIME_MAP; });
    eraseRemovedTimers();

    EvtScripts *scripts = engine->_evtScripts;
    if (scripts) {
        scripts->loadMapScripts(mapFileStem(engine->_currentLoadedMapId)); // Before the triggers, scripts can remove events.
        updateFaceHints(); // The level loader set the flags from the evt file alone.
    }

    // Register all triggers when map done loading
    registerEventTriggers();

    timerGuard = pParty->GetPlayingTime();

    for (EventTrigger trigger : std::vector(onMapLoadTriggers)) // A script can remove triggers while they run.
        runEvent(EvtContext(EVT_SOURCE_MAP, trigger.eventId, Pid(), /* duringMapLoad = */ true), trigger.eventStep + 1);

    if (scripts)
        leaveMapOn(scripts->onMapLoad());
}

void onMapLeave() {
    for (EventTrigger trigger : std::vector(onMapLeaveTriggers)) // A script can remove triggers while they run.
        runEvent(EvtContext(EVT_SOURCE_MAP, trigger.eventId), trigger.eventStep + 1);

    if (engine->_evtScripts)
        engine->_evtScripts->onMapLeave();

    // Cleanup timers to avoid firing while map transition is in process
    removeTimers([](const MapTimer &timer) { return timer.lifetime == EVT_TIMER_LIFETIME_MAP; });
}

/**
 * Fires the timers of a list that are due, until one of them starts a map transition.
 *
 * @param timers                        Timers to check.
 * @return                              Whether a timer started a map transition.
 */
static EvtTransition checkTimers(std::vector<MapTimer> *timers) {
    for (size_t i = 0; i < timers->size(); i++) {
        MapTimer &timer = (*timers)[i];
        if (timer.isRemoved || !timer.alarmTime || pParty->GetPlayingTime() < *timer.alarmTime)
            continue;

        if (timer.schedule.interval) {
            timer.alarmTime = pParty->GetPlayingTime() + timer.schedule.interval;
        } else {
            if (!timer.alarmTime->isValid() && timer.schedule.period == Duration::fromDays(1)) {
                // Initial firing of daily timers, next alarm must be configured to fire on exact time of day
                timer.alarmTime = Time() + timer.schedule.timeOfDay;
            }
            while (pParty->GetPlayingTime() >= *timer.alarmTime) {
                *timer.alarmTime += timer.schedule.period;
            }
        }

        // Either can add timers, which invalidates `timer`.
        EvtTransition transition = timer.handle ? leaveMapOn(engine->_evtScripts->fireTimer(timer.handle))
                                                : runEvent(EvtContext(EVT_SOURCE_MAP, timer.eventId), timer.eventStep + 1);
        if (transition == EVT_TRANSITION_STARTED)
            return transition; // The rest wait for the next pass, on the new map.
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
    eraseRemovedTimers();
}

void addTimer(int handle, const EvtTimerSchedule &schedule, EvtTimerKind kind, EvtTimerLifetime lifetime) {
    assert(handle > 0); // Evt timers have no handle.
    MapTimer result;
    result.schedule = schedule;
    if (schedule.start == EVT_TIMER_START_NOW)
        result.alarmTime = firstAlarm(schedule);
    result.handle = handle;
    result.lifetime = lifetime;
    (kind == EVT_TIMER_KIND_REGULAR ? onTimerTriggers : onLongTimerTriggers).push_back(std::move(result));
}

void removeTimer(int handle) {
    assert(handle > 0); // Evt timers have no handle.
    removeTimers([handle](const MapTimer &timer) { return timer.handle == handle; });
}

void removeEventTriggers(std::optional<int> eventId) {
    auto isRemoved = [eventId](int id) { return !eventId || id == *eventId; };
    removeTimers([&](const MapTimer &timer) { return timer.handle == 0 && isRemoved(timer.eventId); });
    std::erase_if(onMapLoadTriggers, [&](const EventTrigger &trigger) { return isRemoved(trigger.eventId); });
    std::erase_if(onMapLeaveTriggers, [&](const EventTrigger &trigger) { return isRemoved(trigger.eventId); });
}
