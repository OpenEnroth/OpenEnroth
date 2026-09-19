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

int savedEventID;
int savedEventStep;
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

static MapTimer makeTimer(const EvtInstruction &ir) {
    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    Time levelLastVisit = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->lastVisitTime : pOutdoor->lastVisitTime;

    MapTimer timer;
    if (ir.data.timer_descr.alt_halfmin_interval) {
        // Alternative interval is defined in terms of half-minutes
        timer.altInterval = Duration::fromSeconds(ir.data.timer_descr.alt_halfmin_interval * 30);
        timer.alarmTime = pParty->GetPlayingTime() + timer.altInterval;
    } else {
        if (ir.data.timer_descr.is_yearly) {
            timer.interval = Duration::fromYears(1);
        } else if (ir.data.timer_descr.is_monthly) {
            timer.interval = Duration::fromDays(28);
        } else if (ir.data.timer_descr.is_weekly) {
            timer.interval = Duration::fromDays(7);
        } else {
            // Interval is daily with exact time of day
            timer.interval = Duration::fromDays(1);
            timer.timeInsideDay = Duration::fromHours(ir.data.timer_descr.daily_start_hour);
            timer.timeInsideDay += Duration::fromMinutes(ir.data.timer_descr.daily_start_minute);
            timer.timeInsideDay += Duration::fromSeconds(ir.data.timer_descr.daily_start_second);
        }

        if (timer.interval == Duration::fromDays(1)) {
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
                timer.alarmTime = pParty->GetPlayingTime();
            }
        }

        assert(timer.interval > 0_ticks);
    }
    return timer;
}

static void registerTimerTriggers(EvtOpcode triggerType, std::vector<MapTimer> *triggers) {
    triggers->clear();
    for (EventTrigger &trigger : engine->_localEventMap.enumerateTriggers(triggerType)) {
        MapTimer timer = makeTimer(engine->_localEventMap.instruction(trigger.eventId, trigger.eventStep));
        timer.eventId = trigger.eventId;
        timer.eventStep = trigger.eventStep;
        triggers->push_back(std::move(timer));
    }
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

void continueSavedEvent() {
    bool mapExitTriggered = false;
    if (scripts && scripts->resumeEvent(savedEventID, &mapExitTriggered)) {
        if (mapExitTriggered)
            onMapLeave();
        return;
    }
    eventProcessor(savedEventID, Pid(), true, savedEventStep);
}

void cancelSavedEvent() {
    if (scripts)
        scripts->cancelEvent();
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
    if (scripts) {
        std::string mapName = pMapTable->pInfos[engine->_currentLoadedMapId].fileName;
        scripts->loadMapScripts(mapName.substr(0, mapName.rfind('.'))); // Before the triggers, scripts can remove events.
        updateFaceHints(); // The level loader set the flags from the evt file alone.
    }

    // Register all triggers when map done loading
    registerEventTriggers();

    timerGuard = pParty->GetPlayingTime();

    for (EventTrigger &triggers : onMapLoadTriggers) {
        eventProcessor(triggers.eventId, Pid(), false, triggers.eventStep + 1);
    }

    if (scripts && scripts->onMapLoad())
        onMapLeave();
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers) {
        eventProcessor(triggers.eventId, Pid(), true, triggers.eventStep + 1);
    }

    if (scripts)
        scripts->onMapLeave();

    // Cleanup timers to avoid firing while map transition is in process
    onLongTimerTriggers.clear();
    onTimerTriggers.clear();
}

static void checkTimers(std::vector<MapTimer> *timers) {
    for (size_t i = 0; i < timers->size(); i++) {
        if (pParty->GetPlayingTime() < (*timers)[i].alarmTime)
            continue;

        if (std::function<void()> callback = (*timers)[i].callback) {
            callback();
        } else {
            eventProcessor((*timers)[i].eventId, Pid(), true, (*timers)[i].eventStep + 1);
        }
        if (i >= timers->size())
            return; // The event sent the party to another map, and leaving a map drops its timers.

        MapTimer &timer = (*timers)[i];
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
    }
}

void onTimer() {
    if (gameTimer->isPaused()) {
        return;
    }

    if ((pParty->GetPlayingTime() - timerGuard) < Duration::fromSeconds(30)) { // 30 game seconds = 1 realtime second.
        return;
    }

    timerGuard = pParty->GetPlayingTime();

    checkTimers(&onTimerTriggers);
    checkTimers(&onLongTimerTriggers);
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

void addTimer(const EvtInstruction &timer, std::function<void()> callback) {
    assert(timer.opcode == EVENT_OnTimer || timer.opcode == EVENT_OnLongTimer);

    MapTimer result = makeTimer(timer);
    result.callback = std::move(callback);
    (timer.opcode == EVENT_OnTimer ? onTimerTriggers : onLongTimerTriggers).push_back(std::move(result));
}
