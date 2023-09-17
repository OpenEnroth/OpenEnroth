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
    static EventMap load(const Blob &rawData);

    void add(int eventId, EventIR ir);
    void clear();

    bool hasEvent(int eventId) const {
        return _eventsById.contains(eventId);
    }

    /**
     * @param eventId                   Event id.
     * @param step                      Step in the script to get event for.
     * @return                          Reference to an event for the given `eventId` and `step`.
     * @throws Exception                If the event doesn't exist for the provided `eventId` and `step`.
     */
    const EventIR &event(int eventId, int step) const;

    /**
     * @param eventId                   Event id.
     * @return                          Reference to a list of events for the provided `eventId`.
     * @throws Exception                If there are no events for the provided `eventId`.
     */
    const std::vector<EventIR>& events(int eventId) const;

    /**
     * @param triggerType               Event type to look for.
     * @return                          List of all event positions that have the given event type.
     */
    std::vector<EventTrigger> enumerateTriggers(EventType triggerType);

    /**
     *
     * @param eventId                   Event id to check.
     * @return                          Whether a script exists for the provided `eventId` that shows a hint.
     */
    bool hasHint(int eventId) const;

    /**
     * @param eventId                   Event id to check.
     * @return                          Hint to show, if any. Note that unlike `events()`, this function returns
     *                                  an empty string for non-existent events.
     */
    std::string hint(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;

 private:
    std::unordered_map<int, std::vector<EventIR>> _eventsById;
};
