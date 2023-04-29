#include <vector>
#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Events/EventMap.h"
#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventInterpreter.h"
#include "Engine/Events/Processor.h"
#include "Engine/Events.h"

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

static void registerTimerTriggers(EventType triggerType, std::vector<MapTimer> *triggers) {
    std::vector<EventTrigger> timerTriggers = engine->_localEventMap.enumerateTriggers(triggerType);
    GameTime levelLastVisit{};

    // TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it.
    //                   To support fair timers they need to be saved directly.
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        levelLastVisit = pIndoor->stru1.last_visit;
    } else {
        levelLastVisit = pOutdoor->loc_time.last_visit;
    }

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

            if (timer.timeInsideDay) {
                if (levelLastVisit) {
                    // Calculate alarm time inside last visit day
                    int last_seconds = levelLastVisit.GetSecondsFraction();
                    int last_minutes = levelLastVisit.GetMinutesFraction();
                    int last_hours = levelLastVisit.GetHoursOfDay();
                    timer.alarmTime = levelLastVisit - GameTime(last_seconds, last_minutes, last_hours) + timer.timeInsideDay;
                    if (timer.alarmTime < levelLastVisit) {
                        // Last visit time already passed alarm time inside that day so move alarm to next day
                        timer.alarmTime = timer.alarmTime + GameTime::FromDays(1);
                    }
                } else {
                    // Set alarm time on the time of the previous day because it must fire
                    int seconds = pParty->GetPlayingTime().GetSecondsFraction();
                    int minutes = pParty->GetPlayingTime().GetMinutesFraction();
                    int hours = pParty->GetPlayingTime().GetHoursOfDay();
                    timer.alarmTime = pParty->GetPlayingTime() - GameTime(seconds, minutes, hours) + timer.timeInsideDay - GameTime::FromDays(1);
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

void eventProcessor(int eventId, int targetObj, bool canShowMessages, int startStep) {
    if (!eventId) {
        return;
    }

    EvtTargetObj = targetObj; // TODO: pass as local
    dword_5B65C4_cancelEventProcessing = 0; // TODO: rename and contain in this module or better remove it altogether

    // TODO(Nik-RE-dev): linked to old processor for now
    EventProcessor(eventId, targetObj, canShowMessages, startStep);
    return;

    EventInterpreter interpreter;
    bool mapExitTriggered = false;
    logger->verbose("Executing regular event starting from step {}", startStep);
    if (activeLevelDecoration) {
        engine->_globalEventMap.dump(eventId);
        interpreter.prepare(engine->_globalEventMap, eventId, canShowMessages);
    } else {
        engine->_localEventMap.dump(eventId);
        interpreter.prepare(engine->_localEventMap, eventId, canShowMessages);
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
    engine->_globalEventMap.dump(eventId);
    interpreter.prepare(engine->_globalEventMap, eventId, false);
    return interpreter.executeNpcDialogue(startStep);
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
    // Register triggers all triggers when map done loading
    registerEventTriggers();

    for (EventTrigger &triggers : onMapLoadTriggers) {
        eventProcessor(triggers.eventId, 0, false, triggers.eventStep + 1);
    }
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers) {
        eventProcessor(triggers.eventId, 0, true, triggers.eventStep + 1);
    }

    // Cleanup timers to avoid firing when on transitions
    onLongTimerTriggers.clear();
    onTimerTriggers.clear();
}

static void checkTimer(MapTimer &timer) {
    if (pParty->GetPlayingTime() >= timer.alarmTime) {
        eventProcessor(timer.eventId, 0, true, timer.eventStep + 1);
        if (timer.altInterval) {
            timer.alarmTime = pParty->GetPlayingTime() + timer.altInterval;
        } else {
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

    for (MapTimer &timer : onTimerTriggers) {
        checkTimer(timer);
    }

    for (MapTimer &timer : onLongTimerTriggers) {
        checkTimer(timer);
    }
}
