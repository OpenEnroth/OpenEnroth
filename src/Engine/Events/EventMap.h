#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Engine/Events/EventIR.h"

enum class EventType : uint8_t;

struct EventTrigger {
    int eventId;
    int eventStep;
};

class EventMap {
 public:
    void add(int eventId, EventIR ir);
    bool isHaveEvents(int eventId) const { return _eventsById.contains(eventId); }
    EventIR get(int eventId, int step) const;
    const std::vector<EventIR>& getEvents(int eventId) const;
    void clear() { _eventsById.clear(); }

    std::vector<EventTrigger> enumerateTriggers(EventType triggerType);

    bool hasHint(int eventId) const;
    std::string getHintString(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;
 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};
