#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Engine/Events/EventIR.h"

class Blob;

struct EventTrigger {
    int eventId;
    int eventStep;
};

class EventMap {
 public:
    void add(int eventId, EventIR ir);
    void clear();

    bool hasEvent(int eventId) const {
        return _eventsById.contains(eventId);
    }

    EventIR event(int eventId, int step) const;
    const std::vector<EventIR>& events(int eventId) const;

    std::vector<EventTrigger> enumerateTriggers(EventType triggerType);

    bool hasHint(int eventId) const;
    std::string hint(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;

    static EventMap load(const Blob &rawData);

 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};
