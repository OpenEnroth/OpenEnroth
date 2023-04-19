#include <utility>
#include <vector>

#include "Engine/Events/EventMap.h"
#include "Engine/Engine.h"
#include "Utility/MapAccess.h"

void EventMap::add(int id, EventIR ir) {
    _eventsById[id].push_back(ir);
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
