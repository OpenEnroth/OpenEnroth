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
    std::function<void()> callback; // Called instead of the event if set.
};

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

static std::vector<MapTimer> mapTimers;

static std::vector<int> decorationsWithEvents;

// Was in original code and ensures that timers are checked not more often than 30 game seconds.
// Do not needed in practice but can be considered optimization to avoid checking timers too often.
static Time timerGuard;

static EvtScripts *scripts = nullptr;

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

    bool isGlobal = context.source == EVT_SOURCE_GLOBAL;

    bool mapExitTriggered = false;
    if (scripts && startStep > 0 && scripts->resumeEvent(context.eventId, &mapExitTriggered)) {
        if (!mapExitTriggered)
            return EVT_TRANSITION_NONE;
        onMapLeave();
        return EVT_TRANSITION_STARTED;
    }

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
    if (scripts && scripts->eventHint(eventId))
        return true;
    return engine->_localEventMap.hasHint(eventId);
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
    scripts->loadGlobalScripts();
}

/**
 * `FACE_EVENT_IS_HINT` is set while the level loads, from the evt file alone. The map's scripts load later and can change
 * which events only show a hint.
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
    if (scripts) {
        std::string mapName = pMapTable->pInfos[engine->_currentLoadedMapId].fileName;
        scripts->loadMapScripts(mapName.substr(0, mapName.rfind('.'))); // Before the triggers, scripts can remove events.
        updateFaceHints();
    }

    // Register all triggers when map done loading
    registerEventTriggers();

    timerGuard = pParty->GetPlayingTime();

    for (EventTrigger &triggers : onMapLoadTriggers)
        runEvent(EvtContext(EVT_SOURCE_MAP, triggers.eventId, Pid(), /* duringMapLoad = */ true), triggers.eventStep + 1);

    if (scripts)
        scripts->onMapLoad();
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers)
        runEvent(EvtContext(EVT_SOURCE_MAP, triggers.eventId), triggers.eventStep + 1);

    if (scripts)
        scripts->onMapLeave();

    // Cleanup timers to avoid firing while map transition is in process
    mapTimers.clear();
}

/**
 * Fires the timers that are due, until one of them starts a map transition.
 */
static void checkTimers() {
    for (size_t i = 0; i < mapTimers.size(); i++) {
        if (pParty->GetPlayingTime() < mapTimers[i].alarmTime)
            continue;

        if (std::function<void()> callback = mapTimers[i].callback) {
            callback();
            if (i >= mapTimers.size())
                return; // The callback sent the party to another map, and leaving a map drops its timers.
        } else if (runEvent(EvtContext(EVT_SOURCE_MAP, mapTimers[i].eventId), mapTimers[i].eventStep + 1) == EVT_TRANSITION_STARTED) {
            return; // Leaving the map cleared the timers.
        }

        MapTimer &timer = mapTimers[i];
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

void setEvtScripts(EvtScripts *value) {
    scripts = value;
    std::erase_if(mapTimers, [](const MapTimer &timer) { return timer.callback != nullptr; }); // The callbacks belong to the scripts that are going away.
}

EvtScripts *evtScripts() {
    return scripts;
}

void addTimer(const EvtInstruction &timer, std::function<void()> callback) {
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    MapTimer result;
    result.schedule = timerSchedule(timer);
    result.alarmTime = firstAlarm(result.schedule, pParty->GetPlayingTime(), levelLastVisit);
    result.callback = std::move(callback);
    mapTimers.push_back(std::move(result));
}
