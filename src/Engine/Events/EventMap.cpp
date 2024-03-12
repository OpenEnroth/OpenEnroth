#include "EventMap.h"

#include <ranges>
#include <tuple>
#include <vector>
#include <utility>

#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Library/Logger/Logger.h"

#include "Utility/Memory/Blob.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

#include "RawEvent.h"

EventMap EventMap::load(const Blob &rawData) {
    EventMap result;

    const char *pos = reinterpret_cast<const char *>(rawData.data());
    const char *end = pos + rawData.size();
    while (pos < end) {
        const RawEvent *evt = reinterpret_cast<const RawEvent *>(pos);

        size_t size = evt->_e_size + 1; // +1 because we also count the size byte.
        if (size < 5)
            throw Exception("Invalid evt record size: expected at least {}, got {}", 5, size);
        if (pos + size > end)
            throw Exception("Encountered corrupted evt binary data");

        int eventId = EVT_WORD(&evt->v1);
        result.add(eventId, EventIR::parse(evt, size));
        pos += size;
    }

    return result;
}

void EventMap::add(int eventId, EventIR ir) {
    _eventsById[eventId].push_back(std::move(ir));
}

void EventMap::clear() {
    _eventsById.clear();
}

const EventIR &EventMap::event(int eventId, int step) const {
    for (const EventIR &ir : events(eventId))
        if (ir.step == step)
            return ir;
    throw Exception("Event {}:{} not found", eventId, step);
}

const std::vector<EventIR>& EventMap::events(int eventId) const {
    const auto *result = valuePtr(_eventsById, eventId);
    if (!result)
        throw Exception("Event {} not found", eventId);
    return *result;
}

std::vector<EventTrigger> EventMap::enumerateTriggers(EventType triggerType) {
    std::vector<EventTrigger> result;

    for (const auto &[id, events] : _eventsById) {
        for (const EventIR &event : events) {
            // As retarded as it might look, there are scripts that have THREE EVENT_OnLongTimer instructions.
            // Thus, we might have several event triggers for the same event id.
            if (event.type == triggerType) {
                EventTrigger trigger;
                trigger.eventId = id;
                trigger.eventStep = event.step;
                result.push_back(trigger);
            }
        }
    }

    // Need to sort the result so that the order doesn't depend on how the events were laid out in the hash map.
    std::ranges::sort(result, std::less(), [] (const EventTrigger &value) { return std::tie(value.eventId, value.eventStep); }); // NOLINT
    return result;
}

bool EventMap::hasHint(int eventId) const {
    const auto* events = valuePtr(_eventsById, eventId);
    if (!events || events->size() < 2)
        return false;

    return (*events)[0].type == EVENT_MouseOver && (*events)[1].type == EVENT_Exit;
}

std::string EventMap::hint(int eventId) const {
    std::string result;
    bool mouseOverFound = false;

    const auto* events = valuePtr(_eventsById, eventId);
    if (!events) { // no entry in .evt file
        return result;
    }

    for (const EventIR &ir : *events) {
        if (ir.type == EVENT_MouseOver) {
            mouseOverFound = true;
            if (ir.data.text_id < engine->_levelStrings.size()) {
                result = engine->_levelStrings[ir.data.text_id];
            }
        }
        if (mouseOverFound && ir.type == EVENT_SpeakInHouse) {
            if (buildingTable.indices().contains(ir.data.house_id)) {
                result = buildingTable[ir.data.house_id].name;
            }
            break;
        }
    }

    return result;
}

void EventMap::dump(int eventId) const {
    const auto *events = valuePtr(_eventsById, eventId);
    if (events) {
        logger->trace("Event: {}", eventId);
        for (const EventIR &ir : *events) {
            logger->trace("{}", ir.toString());
        }
    } else {
        logger->trace("Event {} not found", eventId);
    }
}

void EventMap::dumpAll() const {
    for (const auto &[id, _] : _eventsById) {
        dump(id);
    }
}
