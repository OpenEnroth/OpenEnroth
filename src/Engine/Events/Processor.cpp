#include <vector>

#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Events/EventMap.h"
#include "Engine/Events/EventInterpreter.h"
#include "Engine/Events/Processor.h"
#include "Engine/Events.h"

static std::vector<EventTrigger> onMapLoadTriggers;
static std::vector<EventTrigger> onMapLeaveTriggers;

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

void registerEventTriggers() {
    onMapLoadTriggers.clear();
    onMapLoadTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapReload);
    onMapLeaveTriggers.clear();
    onMapLeaveTriggers = engine->_localEventMap.enumerateTriggers(EVENT_OnMapLeave);
}

void onMapLoad() {
    // TODO(Nik-RE-dev): linked to old function for now
    OnMapLoad();
    return;

    for (EventTrigger &triggers : onMapLoadTriggers) {
        eventProcessor(triggers.eventId, 0, false, triggers.eventStep + 1);
    }
}

void onMapLeave() {
    for (EventTrigger &triggers : onMapLeaveTriggers) {
        eventProcessor(triggers.eventId, 0, true, triggers.eventStep + 1);
    }
}
