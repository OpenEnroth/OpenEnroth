#include <vector>

#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Events/EventMap.h"

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

void eventProcessor(int eventId, int targetObj, bool canShowMessages, int startStep) {
    EvtTargetObj = targetObj; // TODO: pass as local
    dword_5B65C4_cancelEventProcessing = 0; // TODO: rename and contain in this module or better remove it altogether

    logger->warning("Executing event starting from step {}", startStep);
    if (activeLevelDecoration) {
        engine->_globalEventMap.dump(eventId);
        engine->_globalEventMap.execute(eventId, startStep, canShowMessages);
    } else {
        engine->_localEventMap.dump(eventId);
        engine->_localEventMap.execute(eventId, startStep, canShowMessages);
    }
}

std::string getEventHintString(int eventId) {
    return engine->_localEventMap.getHintString(eventId);
}

void registerEventTriggers() {
    onMapLoadTriggers.clear();
    onMapLoadTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapReload);
    onMapLeaveTriggers.clear();
    onMapLeaveTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapLeave);
}

void onMapLoad() {
    for (EventTrigger &triggers : onMapLoadTriggers) {
        eventProcessor(triggers.eventId, 0, true, triggers.eventStep + 1);
    }
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers) {
        eventProcessor(triggers.eventId, 0, true, triggers.eventStep + 1);
    }
}
