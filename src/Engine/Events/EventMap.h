#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Engine/Events/EventIR.h"

struct EventTrigger {
    int eventId;
    int eventStep;
};

class EventMap {
 public:
    void add(int eventId, EventIR ir);
    EventIR get(int eventId, int step);
    void clear() { _eventsById.clear(); }

    std::vector<EventTrigger> enumerateTriggers(EventType triggerType);

    bool execute(int eventId, int startStep, bool canShowMessages) const;
    std::string getHintString(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;
 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};
