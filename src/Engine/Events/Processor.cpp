#include <vector>
#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Events/EventMap.h"
#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventInterpreter.h"
#include "Engine/Events/Processor.h"
#include "Engine/Party.h"

#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

struct MapTimer {
    GameTime interval = GameTime(0);
    GameTime timeInsideDay = GameTime(0);
    GameTime altInterval = GameTime(0);
    GameTime alarmTime = GameTime(0);
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
static GameTime timerGuard = GameTime(0);

int savedEventID;
int savedEventStep;
struct LevelDecoration *savedDecoration;

void initDecorationEvents() {
    int id = pDecorationList->GetDecorIdByName("Event Trigger");

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
            for (int i = 0; i < pActors.size(); i++) {
                if ((decoration.vPosition - pActors[i].pos).length() < decoration.uTriggerRange) {
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

static void registerTimerTriggers(EventType triggerType, std::vector<MapTimer> *triggers) {
    std::vector<EventTrigger> timerTriggers = engine->_localEventMap.enumerateTriggers(triggerType);

    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    GameTime levelLastVisit = currentLocationTime().last_visit;

    triggers->clear();
    for (EventTrigger &trigger : timerTriggers) {
        MapTimer timer;
        EventIR ir = engine->_localEventMap.get(trigger.eventId, trigger.eventStep);

        if (ir.data.timer_descr.alt_halfmin_interval) {
            // Alternative interval is defined in terms of half-minutes
            timer.altInterval = GameTime::FromSeconds(ir.data.timer_descr.alt_halfmin_interval * 30);
            timer.alarmTime = pParty->GetPlayingTime() + timer.altInterval;
        } else {
            if (ir.data.timer_descr.is_yearly) {
                timer.interval = GameTime::FromYears(1);
            } else if (ir.data.timer_descr.is_monthly) {
                timer.interval = GameTime::FromDays(28);
            } else if (ir.data.timer_descr.is_weekly) {
                timer.interval = GameTime::FromDays(7);
            } else {
                // Interval is daily with exact time of day
                timer.interval = GameTime::FromDays(1);
                timer.timeInsideDay = GameTime::FromHours(ir.data.timer_descr.daily_start_hour);
                timer.timeInsideDay = timer.timeInsideDay.AddMinutes(ir.data.timer_descr.daily_start_minute);
                timer.timeInsideDay = timer.timeInsideDay.AddSeconds(ir.data.timer_descr.daily_start_second);
            }

            if (timer.interval == GameTime::FromDays(1)) {
                if (levelLastVisit) {
                    // Calculate alarm time inside last visit day
                    timer.alarmTime = GameTime::FromDays(levelLastVisit.GetDays()) + timer.timeInsideDay;
                    if (timer.alarmTime < levelLastVisit) {
                        // Last visit time already passed alarm time inside that day so move alarm to next day
                        timer.alarmTime = timer.alarmTime + GameTime::FromDays(1);
                    }
                } else {
                    // Set alarm time to zero because it must always fire
                    timer.alarmTime = GameTime(0);
                }
            } else {
                if (levelLastVisit) {
                    timer.alarmTime = levelLastVisit + timer.interval;
                } else {
                    // Without last visit all timers must fire immediately
                    timer.alarmTime = pParty->GetPlayingTime();
                }
            }
            assert(timer.interval.Valid());
        }
        timer.eventId = trigger.eventId;
        timer.eventStep = trigger.eventStep;

        triggers->push_back(timer);
    }
}

void eventProcessor(int eventId, Pid targetObj, bool canShowMessages, int startStep) {
    if (!eventId) {
        return;
    }

    dword_5B65C4_cancelEventProcessing = 0; // TODO: rename and contain in this module or better remove it altogether

    if (!eventId) { // TODO(captainurist): never executed because it's checked above.
        engine->_statusBar->nothingHere();
        return;
    }

    EventInterpreter interpreter;
    bool mapExitTriggered = false;
    logger->verbose("Executing regular event starting from step {}", startStep);
    if (activeLevelDecoration) {
        engine->_globalEventMap.dump(eventId);
        interpreter.prepare(engine->_globalEventMap, eventId, targetObj, canShowMessages);
    } else {
        engine->_localEventMap.dump(eventId);
        interpreter.prepare(engine->_localEventMap, eventId, targetObj, canShowMessages);
    }

    if (interpreter.executeRegular(startStep)) {
        onMapLeave();
    }
}

bool npcDialogueEventProcessor(int eventId, int startStep) {
    if (!eventId) {
        return false;
    }

    EventInterpreter interpreter;

    logger->verbose("Executing NPC dialogue event starting from step {}", startStep);
    LevelDecoration *oldDecoration = activeLevelDecoration;
    activeLevelDecoration = (LevelDecoration *)1; // Required for correct printing of messages
    engine->_globalEventMap.dump(eventId);
    activeLevelDecoration = oldDecoration;
    interpreter.prepare(engine->_globalEventMap, eventId, Pid(), false);
    return interpreter.executeNpcDialogue(startStep);
}

bool hasEventHint(int eventId) {
    return engine->_localEventMap.hasHint(eventId);
}

std::string getEventHintString(int eventId) {
    return engine->_localEventMap.getHintString(eventId);
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

    for (EventTrigger &triggers : onMapLoadTriggers) {
        eventProcessor(triggers.eventId, Pid(), false, triggers.eventStep + 1);
    }
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers) {
        eventProcessor(triggers.eventId, Pid(), true, triggers.eventStep + 1);
    }

    // Cleanup timers to avoid firing while map transition is in process
    onLongTimerTriggers.clear();
    onTimerTriggers.clear();
}

static void checkTimer(MapTimer &timer) {
    if (pParty->GetPlayingTime() >= timer.alarmTime) {
        eventProcessor(timer.eventId, Pid(), true, timer.eventStep + 1);
        if (timer.altInterval) {
            timer.alarmTime = pParty->GetPlayingTime() + timer.altInterval;
        } else {
            if (timer.alarmTime == GameTime(0) && timer.interval == GameTime::FromDays(1)) {
                // Initial firing of daily timers, next alarm must be configured to fire on exact time of day
                timer.alarmTime = timer.timeInsideDay;
            }
            while (pParty->GetPlayingTime() >= timer.alarmTime) {
                timer.alarmTime += timer.interval;
            }
        }
    }
}

void onTimer() {
    if (pEventTimer->bPaused) {
        return;
    }

    if ((pParty->GetPlayingTime() - timerGuard) < GameTime::FromSeconds(TIME_SECONDS_PER_QUANT)) {
        return;
    }

    timerGuard = pParty->GetPlayingTime();

    for (MapTimer &timer : onTimerTriggers) {
        checkTimer(timer);
    }

    for (MapTimer &timer : onLongTimerTriggers) {
        checkTimer(timer);
    }
}
