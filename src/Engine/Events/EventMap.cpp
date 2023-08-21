#include <assert.h>
#include <utility>
#include <vector>

#include "Engine/Events/EventMap.h"
#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Library/Logger/Logger.h"
#include "Engine/Events/EventEnums.h"
#include "Engine/Tables/BuildingTable.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Utility/IndexedArray.h"
#include "Utility/Segment.h"

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

bool EventMap::hasHint(int eventId) const {
    if (!_eventsById.contains(eventId)) {
        return false;
    }

    const std::vector<EventIR>& eventInsn = _eventsById.at(eventId);

    if (eventInsn.size() < 2) {
        return false;
    }

    return eventInsn[0].type == EVENT_MouseOver && eventInsn[1].type == EVENT_Exit;
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
            if (ir.data.text_id < engine->_levelStrings.size()) {
                result = engine->_levelStrings[ir.data.text_id];
            }
        }
        if (mouseOverFound && ir.type == EVENT_SpeakInHouse) {
            if (allHouses().contains(ir.data.house_id)) {
                result = buildingTable[ir.data.house_id].pName;
            }
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
