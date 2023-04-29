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

EventIR EventMap::get(int eventId, int step) const {
    for (const EventIR &ir : _eventsById.at(eventId)) {
        if (ir.step == step) {
            return ir;
        }
    }
    assert(false);

    return EventIR();
}

const std::vector<EventIR>& EventMap::getEvents(int eventId) const {
    return _eventsById.at(eventId);
}

std::vector<EventTrigger> EventMap::enumerateTriggers(EventType triggerType) {
    std::vector<EventTrigger> triggers;

    for (const auto &mapElem : _eventsById) {
        for (const EventIR &ir : mapElem.second) {
            if (ir.type == triggerType) {
                EventTrigger trigger;
                trigger.eventId = mapElem.first;
                trigger.eventStep = ir.step;

                triggers.push_back(trigger);
            }
        }
    }

    return triggers;
}

std::string EventMap::getHintString(int eventId) const {
    std::string result = "";
    bool mouseOverFound = false;

    if (!_eventsById.contains(eventId)) { // no entry in .evt file
        return result;
    }

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
        logger->verbose("Event: {}", eventId);
        for (const EventIR &ir : _eventsById.at(eventId)) {
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
