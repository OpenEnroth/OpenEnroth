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
    Duration interval;
    Duration timeInsideDay;
    Duration altInterval;
    Time alarmTime;
    int eventId = 0;
    int eventStep = 0;
    std::function<void()> callback; // Called instead of the event if set.
    int handle = 0;
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

static EvtScripts *scripts = nullptr;
static int lastTimerHandle = 0;
static int mapLeaveCount = 0;

static std::function<void()> eventContinuation;
LevelDecoration *savedDecoration;

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

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_TOUCH) {
            if ((decoration.vPosition - pParty->pos).length() < decoration.uTriggerRange) {
                eventProcessor(decoration.uEventID, Pid(OBJECT_Decoration, decorationId), 1);
            }
        }

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_MONSTER) {
            for (const Actor &actor : pActors) {
                if ((decoration.vPosition - actor.pos).length() < decoration.uTriggerRange) {
                    eventProcessor(decoration.uEventID, Pid(), 1);
                }
            }
        }

        if (decoration.uFlags & LEVEL_DECORATION_TRIGGERED_BY_OBJECT) {
            for (int i = 0; i < pSpriteObjects.size(); i++) {
                if ((decoration.vPosition - pSpriteObjects[i].vPosition).length() < decoration.uTriggerRange) {
                    eventProcessor(decoration.uEventID, Pid(), 1);
                }
            }
        }
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

static MapTimer makeTimer(const EvtTimerSchedule &schedule) {
    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    Time now = pParty->GetPlayingTime();
    MapTimer timer;
    if (schedule.interval) {
        timer.altInterval = schedule.interval;
        timer.alarmTime = now + timer.altInterval;
    } else if (schedule.start == EVT_TIMER_START_NOW) {
        timer.interval = schedule.period;
        timer.timeInsideDay = schedule.timeOfDay;
        if (timer.interval == Duration::fromDays(1)) {
            timer.alarmTime = Time::fromDays(now.toDays()) + timer.timeInsideDay;
            if (timer.alarmTime <= now)
                timer.alarmTime += Duration::fromDays(1);
        } else {
            timer.alarmTime = now + timer.interval;
        }
    } else {
        timer.interval = schedule.period;
        if (timer.interval == Duration::fromDays(1)) {
            timer.timeInsideDay = schedule.timeOfDay;
            if (levelLastVisit) {
                // Calculate alarm time inside last visit day
                timer.alarmTime = Time::fromDays(levelLastVisit.toDays()) + timer.timeInsideDay;
                if (timer.alarmTime < levelLastVisit) {
                    // Last visit time already passed alarm time inside that day so move alarm to next day
                    timer.alarmTime = timer.alarmTime + Duration::fromDays(1);
                }
            } else {
                // Set alarm time to zero because it must always fire
                timer.alarmTime = Time();
            }
        } else {
            if (levelLastVisit) {
                timer.alarmTime = levelLastVisit + timer.interval;
            } else {
                // Without last visit all timers must fire immediately
                timer.alarmTime = now;
            }
        }

        assert(timer.interval > 0_ticks);
    }
    return timer;
}

static void registerTimerTriggers(EvtOpcode triggerType, std::vector<MapTimer> *triggers) {
    std::erase_if(*triggers, [](const MapTimer &timer) { return timer.callback == nullptr; });

    std::vector<MapTimer> evtTimers;
    for (EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
        MapTimer timer = makeTimer(timerSchedule(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep)));
        timer.eventId = trigger.eventId;
        timer.eventStep = trigger.eventStep;
        evtTimers.push_back(std::move(timer));
    }
    triggers->insert(triggers->begin(), evtTimers.begin(), evtTimers.end()); // Before the scripts' timers, as evt events go first.
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

void eventProcessor(int eventId, Pid targetObj, bool canShowMessages, int startStep) {
    if (!eventId) {
        engine->_statusBar->nothingHere();
        return;
    }

    bool isGlobal = activeLevelDecoration != nullptr;

    EvtInterpreter interpreter;
    MM_TRACE("Executing regular event starting from step {}", startStep);
    if (isGlobal) {
        engine->_globalEventMap.dump(eventId);
        interpreter.prepare(engine->_globalEventMap, eventId, targetObj, canShowMessages);
    } else {
        engine->_localEventMap.dump(eventId);
        interpreter.prepare(engine->_localEventMap, eventId, targetObj, canShowMessages);
    }

    bool isScripted = scripts && startStep == 0 && scripts->hasEvent(isGlobal, eventId);
    if (!interpreter.isValid() && !isScripted) {
        if (scripts && !isGlobal && scripts->eventHint(eventId))
            return; // The scripts gave the event a hint and nothing to do, like an evt event that is a hint and an exit.
        MM_WARNING("Face has invalid event ID");
        engine->_statusBar->nothingHere();
        return;
    }

    bool mapExitTriggered = interpreter.executeRegular(startStep);
    if (isScripted)
        mapExitTriggered |= scripts->runEvent(isGlobal, eventId, targetObj, canShowMessages);
    if (mapExitTriggered)
        onMapLeave();
}

void setEventContinuation(std::function<void()> continuation) {
    eventContinuation = std::move(continuation);
}

bool hasEventContinuation() {
    return eventContinuation != nullptr;
}

void continueSavedEvent() {
    if (std::function<void()> continuation = std::exchange(eventContinuation, nullptr))
        continuation();
}

void cancelSavedEvent() {
    eventContinuation = nullptr;
}

bool npcDialogueEventProcessor(int eventId, int startStep) {
    if (!eventId) {
        return false;
    }

    EvtInterpreter interpreter;

    MM_TRACE("Executing NPC dialogue event starting from step {}", startStep);
    LevelDecoration *oldDecoration = activeLevelDecoration;
    activeLevelDecoration = (LevelDecoration *)1; // Required for correct printing of messages
    engine->_globalEventMap.dump(eventId);
    activeLevelDecoration = oldDecoration;
    interpreter.prepare(engine->_globalEventMap, eventId, Pid(), false);
    bool result = interpreter.executeNpcDialogue(startStep);
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

    registerTimerTriggers(EVENT_OnLongTimer, &onLongTimerTriggers);
    registerTimerTriggers(EVENT_OnTimer, &onTimerTriggers);
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

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        std::ranges::for_each(pIndoor->faces, update);
    } else {
        for (BSPModel &model : pOutdoor->pBModels)
            std::ranges::for_each(model.faces, update);
    }
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
        eventProcessor(trigger.eventId, Pid(), false, trigger.eventStep + 1);

    if (scripts && scripts->onMapLoad())
        onMapLeave();
}

void onMapLeave() {
    mapLeaveCount++;

    for (EventTrigger trigger : std::vector(onMapLeaveTriggers)) // A script can remove triggers while they run.
        eventProcessor(trigger.eventId, Pid(), true, trigger.eventStep + 1);

    if (scripts)
        scripts->onMapLeave();

    // Cleanup timers to avoid firing while map transition is in process
    removeTimers([](const MapTimer &timer) { return timer.lifetime == EVT_TIMER_LIFETIME_MAP; });
}

/**
 * Fires the timers of a list that are due, until one of them leaves the map.
 *
 * @param timers                        Timers to check.
 * @return                              Whether a timer left the map.
 */
static bool checkTimers(std::vector<MapTimer> *timers) {
    int leaveCount = mapLeaveCount;
    for (size_t i = 0; i < timers->size(); i++) {
        MapTimer &timer = (*timers)[i];
        if (timer.isRemoved || pParty->GetPlayingTime() < timer.alarmTime)
            continue;

        if (timer.altInterval) {
            timer.alarmTime = pParty->GetPlayingTime() + timer.altInterval;
        } else {
            if (!timer.alarmTime.isValid() && timer.interval == Duration::fromDays(1)) {
                // Initial firing of daily timers, next alarm must be configured to fire on exact time of day
                timer.alarmTime = Time() + timer.timeInsideDay;
            }
            while (pParty->GetPlayingTime() >= timer.alarmTime) {
                timer.alarmTime += timer.interval;
            }
        }

        if (std::function<void()> callback = timer.callback) {
            callback(); // Can add timers, which invalidates `timer`.
        } else {
            eventProcessor(timer.eventId, Pid(), true, timer.eventStep + 1);
        }
        if (mapLeaveCount != leaveCount)
            return true; // The rest wait for the next pass, on the new map.
    }
    return false;
}

void onTimer() {
    if (gameTimer->isPaused()) {
        return;
    }

    if ((pParty->GetPlayingTime() - timerGuard) < Duration::fromSeconds(30)) { // 30 game seconds = 1 realtime second.
        return;
    }

    timerGuard = pParty->GetPlayingTime();

    if (!checkTimers(&onTimerTriggers))
        checkTimers(&onLongTimerTriggers);
    eraseRemovedTimers();
}

void setEvtScripts(EvtScripts *value) {
    scripts = value;
    auto isScripted = [](const MapTimer &timer) { return timer.callback != nullptr; };
    std::erase_if(onTimerTriggers, isScripted); // The callbacks belong to the scripts that are going away.
    std::erase_if(onLongTimerTriggers, isScripted);
}

EvtScripts *evtScripts() {
    return scripts;
}

int addTimer(const EvtTimerSchedule &schedule, EvtTimerKind kind, EvtTimerLifetime lifetime, std::function<void()> callback) {
    MapTimer result = makeTimer(schedule);
    result.callback = std::move(callback);
    result.handle = ++lastTimerHandle;
    result.lifetime = lifetime;
    (kind == EVT_TIMER_KIND_REGULAR ? onTimerTriggers : onLongTimerTriggers).push_back(std::move(result));
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
