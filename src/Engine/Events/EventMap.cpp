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

EventIR EventMap::get(int eventId, int step) {
    for (const EventIR &ir : _eventsById.at(eventId)) {
        if (ir.step == step) {
            return ir;
        }
    }
    assert(false);

    return EventIR();
}

std::vector<EventTrigger> EventMap::enumerateTriggers(EventType triggerType) {
    std::vector<EventTrigger> triggers;

    for (const auto &mapElem : _eventsById) {
        for (const EventIR &ir : mapElem.second) {
            if (ir.type == triggerType) {
                triggers.push_back(EventTrigger(mapElem.first, ir.step));
            }
        }
    }

    return triggers;
}

bool EventMap::execute(int eventId, int startStep, bool canShowMessages) const {
    assert(_eventsById.contains(eventId));
    assert(startStep >= 0);

    int step = startStep;
    bool stepFound;
    PLAYER_CHOOSE_POLICY who = !pParty->hasActiveCharacter() ? CHOOSE_RANDOM : CHOOSE_ACTIVE;
    bool mapExitTriggered = false;

    do {
        stepFound = false;
        for (const EventIR &ir : _eventsById.at(eventId)) {
            if (ir.step == step) {
                step = ir.execute(eventId, canShowMessages, &who, &mapExitTriggered);
                stepFound = true;
                break;
            }
        }
    } while (stepFound && step != -1 && dword_5B65C4_cancelEventProcessing == 0);

    return mapExitTriggered;
}

std::string EventMap::getHintString(int eventId) const {
    std::string result = "";
    bool mouseOverFound = false;

    for (const EventIR &ir : _eventsById.at(eventId)) {
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
        logger->warning("Event: {}", eventId);
        for (const EventIR &ir : _eventsById.at(eventId)) {
            logger->warning("{}", ir.toString());
        }
    } else {
        logger->warning("Event {} not found", eventId);
    }
}

void EventMap::dumpAll() const {
    for (const auto &[id, _] : _eventsById) {
        dump(id);
    }
}
