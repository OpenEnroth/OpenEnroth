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
#include "Engine/Evt/EvtEnumFunctions.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Evt/EvtScripts.h"
#include "Engine/Party.h"

#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

struct MapTimer {
    EvtTimerSchedule schedule;
    Time alarmTime;
    EvtTimerKind kind = EVT_TIMER_KIND_REGULAR;
    int eventId = 0;
    int eventStep = 0;
    std::function<void()> callback; // Called instead of the event if set.
    int handle = 0;
    EvtTimerLifetime lifetime = EVT_TIMER_LIFETIME_MAP;
    bool isRemoved = false; // Erased once no loop over the timers is running.
};

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

static std::vector<MapTimer> mapTimers;

static std::vector<int> decorationsWithEvents;

// Was in original code and ensures that timers are checked not more often than 30 game seconds.
// Do not needed in practice but can be considered optimization to avoid checking timers too often.
static Time timerGuard;

static EvtScripts *scripts = nullptr;
static int lastTimerHandle = 0;
static int mapLeaveCount = 0;

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
 * @param start                         What the first alarm counts from.
 * @param now                           Current time.
 * @param levelLastVisit                Last visit to the current map.
 * @return                              When it fires first.
 */
static Time firstAlarm(const EvtTimerSchedule &schedule, EvtTimerStart start, Time now, Time levelLastVisit) {
    switch (schedule.type) {
    case EVT_TIMER_INTERVAL:
        return now + schedule.duration;
    case EVT_TIMER_DAILY: {
        if (start == EVT_TIMER_START_NOW) {
            Time result = Time::fromDays(now.toDays()) + schedule.duration;
            return result <= now ? result + Duration::fromDays(1) : result;
        }
        if (!levelLastVisit)
            return now;

        Time result = Time::fromDays(levelLastVisit.toDays()) + schedule.duration;
        return result < levelLastVisit ? result + Duration::fromDays(1) : result;
    }
    default:
        if (start == EVT_TIMER_START_NOW)
            return now + periodForTimerType(schedule.type);
        return levelLastVisit ? levelLastVisit + periodForTimerType(schedule.type) : now; // Without last visit all timers must fire immediately.
    }
}

static void registerTimers() {
    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelEnterTime = pParty->GetPlayingTime();
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    std::erase_if(mapTimers, [](const MapTimer &timer) { return timer.callback == nullptr; });

    std::vector<MapTimer> evtTimers;
    for (EvtOpcode triggerType : {EVENT_OnTimer, EVENT_OnLongTimer}) {
        for (const EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
            MapTimer timer;
            timer.schedule = timerSchedule(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep));
            timer.alarmTime = firstAlarm(timer.schedule, EVT_TIMER_START_LAST_VISIT, levelEnterTime, levelLastVisit);
            timer.kind = triggerType == EVENT_OnTimer ? EVT_TIMER_KIND_REGULAR : EVT_TIMER_KIND_REFILL;
            timer.eventId = trigger.eventId;
            timer.eventStep = trigger.eventStep;
            evtTimers.push_back(timer);
        }
    }
    mapTimers.insert(mapTimers.begin(), evtTimers.begin(), evtTimers.end()); // Before the scripts' timers, as evt events go first.
}

static void removeTimers(std::function<bool(const MapTimer &)> predicate) {
    for (MapTimer &timer : mapTimers)
        if (predicate(timer))
            timer.isRemoved = true;
}

static void eraseRemovedTimers() {
    std::erase_if(mapTimers, [](const MapTimer &timer) { return timer.isRemoved; });
}

EvtTransition runEvent(EvtContext context, int startStep) {
    if (context.scripted) {
        if (!scripts || !scripts->resumeEvent())
            return EVT_TRANSITION_NONE;
        onMapLeave();
        return EVT_TRANSITION_STARTED;
    }

    if (!context.eventId) {
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    bool isGlobal = context.source == EVT_SOURCE_GLOBAL;

    EvtInterpreter interpreter;
    MM_TRACE("Executing event starting from step {}", startStep);
    interpreter.prepare(context);

    bool isScripted = scripts && startStep == 0 && scripts->hasEvent(isGlobal, context.eventId);
    if (!interpreter.isValid() && !isScripted) {
        if (scripts && !isGlobal && scripts->eventHint(context.eventId))
            return EVT_TRANSITION_NONE; // The scripts gave the event a hint and nothing to do, like an evt event that is a hint and an exit.
        MM_WARNING("Face has invalid event ID");
        engine->_statusBar->nothingHere();
        return EVT_TRANSITION_NONE;
    }

    EvtTransition transition = interpreter.execute(startStep);
    if (isScripted && scripts->runEvent(isGlobal, context.eventId, context.objectPid, !context.duringMapLoad))
        transition = EVT_TRANSITION_STARTED;
    if (transition == EVT_TRANSITION_STARTED)
        onMapLeave();
    return transition;
}

bool canShowTopic(int eventId) {
    EvtInterpreter interpreter;
    interpreter.prepare(EvtContext(EVT_SOURCE_GLOBAL, eventId));
    bool result = interpreter.canShowTopic();
    if (scripts)
        result = scripts->canShowTopic(eventId).value_or(result);
    return result;
}

bool hasEventHint(int eventId) {
    if (scripts && scripts->hasEvent(false, eventId))
        return false; // The event does more than show a hint.
    if (engine->_localEventMap.hasEvent(eventId))
        return engine->_localEventMap.hasHint(eventId);
    return scripts && scripts->eventHint(eventId);
}

std::string getEventHintString(int eventId) {
    if (scripts)
        if (std::optional<std::string> hint = scripts->eventHint(eventId))
            return *hint;
    return engine->_localEventMap.hint(eventId);
}

static void registerEventTriggers() {
    onMapLoadTriggers.clear();
    onMapLoadTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapReload);
    onMapLeaveTriggers.clear();
    onMapLeaveTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapLeave);

    registerTimers();
}

void onGameLoad() {
    if (!scripts)
        return;

    engine->_globalEventMap = EvtProgram::load(engine->resources()->eventsData("global.evt"));
    removeTimers([](const MapTimer &timer) { return timer.callback != nullptr; }); // They belong to the game that is left.
    eraseRemovedTimers();
    scripts->loadGlobalScripts();
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

    std::ranges::for_each(uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->faces : pOutdoor->faces, update);
}

void onMapLoad() {
    removeTimers([](const MapTimer &timer) { return timer.lifetime == EVT_TIMER_LIFETIME_MAP; });
    eraseRemovedTimers();

    if (scripts) {
        std::string mapName = pMapTable->pInfos[engine->_currentLoadedMapId].fileName;
        scripts->loadMapScripts(mapName.substr(0, mapName.rfind('.'))); // Before the triggers, scripts can remove events.
        updateFaceHints(); // The level loader set the flags from the evt file alone.
    }

    // Register all triggers when map done loading
    registerEventTriggers();

    timerGuard = pParty->GetPlayingTime();

    for (EventTrigger trigger : std::vector(onMapLoadTriggers)) // A script can remove triggers while they run.
        runEvent(EvtContext(EVT_SOURCE_MAP, trigger.eventId, Pid(), /* duringMapLoad = */ true), trigger.eventStep + 1);

    if (scripts && scripts->onMapLoad())
        onMapLeave();
}

void onMapLeave() {
    mapLeaveCount++;

    for (EventTrigger trigger : std::vector(onMapLeaveTriggers)) // A script can remove triggers while they run.
        runEvent(EvtContext(EVT_SOURCE_MAP, trigger.eventId), trigger.eventStep + 1);

    if (scripts)
        scripts->onMapLeave();

    // Cleanup timers to avoid firing while map transition is in process
    removeTimers([](const MapTimer &timer) { return timer.lifetime == EVT_TIMER_LIFETIME_MAP; });
}

/**
 * Fires the timers that are due, the regular ones first, until one of them leaves the map.
 */
static void checkTimers() {
    int leaveCount = mapLeaveCount;
    for (EvtTimerKind kind : {EVT_TIMER_KIND_REGULAR, EVT_TIMER_KIND_REFILL}) {
        for (size_t i = 0; i < mapTimers.size(); i++) {
            MapTimer &timer = mapTimers[i];
            if (timer.kind != kind || timer.isRemoved || pParty->GetPlayingTime() < timer.alarmTime)
                continue;

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

            if (std::function<void()> callback = timer.callback) {
                callback(); // Can add timers, which invalidates `timer`.
            } else {
                runEvent(EvtContext(EVT_SOURCE_MAP, timer.eventId), timer.eventStep + 1);
            }
            if (mapLeaveCount != leaveCount)
                return; // The rest wait for the next pass, on the new map.
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
    eraseRemovedTimers();
}

void setEvtScripts(EvtScripts *value) {
    scripts = value;
    std::erase_if(mapTimers, [](const MapTimer &timer) { return timer.callback != nullptr; }); // The callbacks belong to the scripts that are going away.
}

EvtScripts *evtScripts() {
    return scripts;
}

int addTimer(const EvtTimerSchedule &schedule, EvtTimerStart start, EvtTimerKind kind, EvtTimerLifetime lifetime, std::function<void()> callback) {
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    MapTimer result;
    result.schedule = schedule;
    result.alarmTime = firstAlarm(schedule, start, pParty->GetPlayingTime(), levelLastVisit);
    result.kind = kind;
    result.callback = std::move(callback);
    result.handle = ++lastTimerHandle;
    result.lifetime = lifetime;
    mapTimers.push_back(std::move(result));
    return lastTimerHandle;
}

void removeTimer(int handle) {
    assert(handle > 0); // Evt timers have no handle.
    removeTimers([handle](const MapTimer &timer) { return timer.handle == handle; });
}

void removeEventTriggers(std::optional<int> eventId) {
    auto isRemoved = [eventId](int id) { return !eventId || id == *eventId; };
    removeTimers([&](const MapTimer &timer) { return timer.callback == nullptr && isRemoved(timer.eventId); });
    std::erase_if(onMapLoadTriggers, [&](const EventTrigger &trigger) { return isRemoved(trigger.eventId); });
    std::erase_if(onMapLeaveTriggers, [&](const EventTrigger &trigger) { return isRemoved(trigger.eventId); });
}
