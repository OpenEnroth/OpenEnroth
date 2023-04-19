#pragma once

#include <unordered_map>
#include <vector>

#include "Engine/Events/EventIR.h"

class EventMap {
 public:
    void add(int id, EventIR ir);
    void clear() { _eventsById.clear(); }

    void dumpAll() const;
    void dump(int id) const;
 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};
