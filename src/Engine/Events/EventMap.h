#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Engine/Events/EventIR.h"

class EventMap {
 public:
    void add(int eventId, EventIR ir);
    void clear() { _eventsById.clear(); }

    void execute(int eventId, int startStep, bool canShowMessages) const;
    std::string getHintString(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;
 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};

// TODO(Nik-RE-dev): move these to separate location
void eventProcessor(int eventId, int targetObj, int canShowMessages, int startStep);
std::string getEventHintString(int eventId);
