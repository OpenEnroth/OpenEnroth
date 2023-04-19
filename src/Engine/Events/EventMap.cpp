#include <utility>

#include "Engine/Events/EventMap.h"
#include "Engine/Engine.h"

void EventMap::add(int id, EventIR ir) {
    _eventsById[id].push_back(ir);
}

void EventMap::dump(int eventId) {
    if (_eventsById.contains(eventId)) {
        logger->verbose("Event: {}", eventId);
        for (EventIR &ir : _eventsById[eventId]) {
            logger->verbose("{}", ir.toString());
        }
    } else {
        logger->verbose("Event {} not found", eventId);
    }
}

void EventMap::dumpAll() {
    for (std::pair<int, std::vector<EventIR>> pair : _eventsById) {
        dump(pair.first);
    }
}
