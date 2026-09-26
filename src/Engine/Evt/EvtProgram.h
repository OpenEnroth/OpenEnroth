#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include <string>

#include "Engine/Data/HouseEnums.h"
#include "Engine/Evt/EvtInstruction.h"

class Blob;

struct EventTrigger {
    int eventId = 0;
    int eventStep = 0;
};

/**
 * Where the hint of an event comes from.
 */
struct EvtHintSource {
    int textId = 0; // Index into the map's strings. Can be past their end, and then the hint is empty.
    HouseId houseId = HOUSE_INVALID; // House whose name the hint is instead, if any.
};

class EvtProgram {
 public:
    static EvtProgram load(const Blob &rawData);

    void add(int eventId, EvtInstruction ir);
    void remove(int eventId);
    void clear();

    size_t eventCount() const {
        return _eventsById.size();
    }

    bool hasEvent(int eventId) const {
        return _eventsById.contains(eventId);
    }

    /**
     * @return                          Ids of the events, sorted.
     */
    std::vector<int> eventIds() const;

    /**
     * @param eventId                   Event id.
     * @param step                      Step in the script to get event for.
     * @return                          Reference to an instruction for the given `eventId` and `step`.
     * @throws Exception                If the instruction doesn't exist for the provided `eventId` and `step`.
     */
    const EvtInstruction &instruction(int eventId, int step) const;

    /**
     * @param eventId                   Event id.
     * @return                          Reference to a list of events for the provided `eventId`.
     * @throws Exception                If there are no events for the provided `eventId`.
     */
    const std::vector<EvtInstruction>& function(int eventId) const;

    /**
     * @param triggerType               Event type to look for.
     * @return                          List of all event positions that have the given event type.
     */
    std::vector<EventTrigger> enumerateTriggers(EvtOpcode triggerType);

    /**
     *
     * @param eventId                   Event id to check.
     * @return                          Whether a script exists for the provided `eventId` that shows a hint.
     */
    bool hasHint(int eventId) const;

    /**
     * @param instructions              Instructions of an event.
     * @return                          Whether the event only shows a hint, which it does if it opens with a
     *                                  `MouseOver` and an `Exit`.
     */
    static bool isHintOnly(const std::vector<EvtInstruction> &instructions);

    /**
     * @param instructions              Instructions of an event.
     * @return                          Where the event's hint comes from, or `std::nullopt` if it has no hint.
     */
    static std::optional<EvtHintSource> hintSource(const std::vector<EvtInstruction> &instructions);

    /**
     * @param eventId                   Event id to check.
     * @return                          Hint to show, if any. Note that unlike `events()`, this function returns
     *                                  an empty string for non-existent events.
     */
    std::string hint(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;

 private:
    std::unordered_map<int, std::vector<EvtInstruction>> _eventsById;
};
