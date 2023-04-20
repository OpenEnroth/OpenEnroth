#include <utility>
#include <vector>

#include "Engine/Party.h"
#include "Engine/Events/EventMap.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/mm7_data.h"
#include "Utility/MapAccess.h"

void EventMap::add(int eventId, EventIR ir) {
    _eventsById[eventId].push_back(ir);
}

void EventMap::execute(int eventId, int startStep, bool canShowMessages) const {
    assert(_eventsById.contains(eventId));
    assert(startStep >= 0);

    int step = startStep;
    bool stepFound;
    PLAYER_CHOOSE_POLICY who = !pParty->hasActiveCharacter() ? CHOOSE_RANDOM : CHOOSE_ACTIVE;
    bool mapExitTriggered;

    do {
        stepFound = false;
        for (const EventIR &ir : valueOr(_eventsById, eventId, std::vector<EventIR>())) {
            if (ir.step == step) {
                step = ir.execute(canShowMessages, &who, &mapExitTriggered);
                stepFound = true;
                break;
            }
        }
    } while (stepFound && step != -1 && dword_5B65C4_cancelEventProcessing == 0);

    if (mapExitTriggered) {
        OnMapLeave();
    }
}

std::string EventMap::getHintString(int eventId) const {
    std::string result = "";
    bool mouseOverFound = false;

    for (const EventIR &ir : valueOr(_eventsById, eventId, std::vector<EventIR>())) {
        if (ir.type == EVENT_MouseOver) {
            mouseOverFound = true;
            result = &pLevelStr[pLevelStrOffsets[ir.data.text_id]];
        }
        if (mouseOverFound && ir.type == EVENT_SpeakInHouse) {
            result = p2DEvents[ir.data.house_id - 1].pName;
            break;
        }
    }

    return result;
}

void EventMap::dump(int eventId) const {
    if (_eventsById.contains(eventId)) {
        logger->verbose("Event: {}", eventId);
        for (const EventIR &ir : valueOr(_eventsById, eventId, std::vector<EventIR>())) {
            logger->verbose("{}", ir.toString());
        }
    } else {
        logger->verbose("Event {} not found", eventId);
    }
}

void EventMap::dumpAll() const {
    for (const auto &[id, _] : _eventsById) {
        dump(id);
    }
}

void eventProcessor(int eventId, int targetObj, int canShowMessages, int startStep) {
    EvtTargetObj = targetObj; // TODO: pass as local
    dword_5B65C4_cancelEventProcessing = 0; // TODO: rename and contain in this module or better remove it altogether

    logger->verbose("Executing event starting from step {}", startStep);
    if (activeLevelDecoration) {
        engine->_globalEventMap.dump(eventId);
        engine->_globalEventMap.execute(eventId, startStep, canShowMessages);
    } else {
        engine->_localEventMap.dump(eventId);
        engine->_localEventMap.execute(eventId, startStep, canShowMessages);
    }
}

std::string getEventHintString(unsigned int eventId) {
    return engine->_localEventMap.getHintString(eventId);
}
