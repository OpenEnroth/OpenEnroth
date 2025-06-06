#include "EvtProgram.h"

#include <ranges>
#include <tuple>
#include <vector>
#include <utility>
#include <string>

#include "Engine/Tables/HouseTable.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Library/Logger/Logger.h"

#include "Utility/Memory/Blob.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"
#include "Utility/SequentialBlobReader.h"

EvtProgram EvtProgram::load(const Blob &rawData) {
    EvtProgram result;

    const uint8_t *pos = reinterpret_cast<const uint8_t *>(rawData.data());
    const uint8_t *const end = pos + rawData.size();
    while (pos < end) {
        size_t size = *pos + 1; // +1 because we also count the size byte.
        if (size < 5)
            throw Exception("Invalid evt record size: expected at least {}, got {}", 5, size);
        if (pos + size > end)
            throw Exception("Encountered corrupted evt binary data");
        SequentialBlobReader sbr(pos + 1, size - 1); // offset is 1 because we are skipping the `size` byte - it was already read
        int eventId = sbr.read<uint16_t>();
        result.add(eventId, EvtInstruction::parse(sbr, size));
        pos += size;
    }

    return result;
}

void EvtProgram::add(int eventId, EvtInstruction ir) {
    _eventsById[eventId].push_back(std::move(ir));
}

void EvtProgram::clear() {
    _eventsById.clear();
}

const EvtInstruction &EvtProgram::instruction(int eventId, int step) const {
    for (const EvtInstruction &ir : function(eventId))
        if (ir.step == step)
            return ir;
    throw Exception("Event {}:{} not found", eventId, step);
}

const std::vector<EvtInstruction>& EvtProgram::function(int eventId) const {
    const auto *result = valuePtr(_eventsById, eventId);
    if (!result)
        throw Exception("Event {} not found", eventId);
    return *result;
}

std::vector<EventTrigger> EvtProgram::enumerateTriggers(EvtOpcode triggerType) {
    std::vector<EventTrigger> result;

    for (const auto &[id, events] : _eventsById) {
        for (const EvtInstruction &event : events) {
            // As retarded as it might look, there are scripts that have THREE EVENT_OnLongTimer instructions.
            // Thus, we might have several event triggers for the same event id.
            if (event.opcode == triggerType) {
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

bool EvtProgram::hasHint(int eventId) const {
    const auto* events = valuePtr(_eventsById, eventId);
    if (!events || events->size() < 2)
        return false;

    return (*events)[0].opcode == EVENT_MouseOver && (*events)[1].opcode == EVENT_Exit;
}

std::string EvtProgram::hint(int eventId) const {
    std::string result;
    bool mouseOverFound = false;

    const auto* events = valuePtr(_eventsById, eventId);
    if (!events) { // no entry in .evt file
        return result;
    }

    for (const EvtInstruction &ir : *events) {
        if (ir.opcode == EVENT_MouseOver) {
            mouseOverFound = true;
            if (ir.data.text_id < engine->_levelStrings.size()) {
                result = engine->_levelStrings[ir.data.text_id];
            }
        }
        if (mouseOverFound && ir.opcode == EVENT_SpeakInHouse) {
            if (houseTable.indices().contains(ir.data.house_id)) {
                result = houseTable[ir.data.house_id].name;
            }
            break;
        }
    }

    return result;
}

void EvtProgram::dump(int eventId) const {
    const auto *events = valuePtr(_eventsById, eventId);
    if (events) {
        logger->trace("Event: {}", eventId);
        for (const EvtInstruction &ir : *events) {
            logger->trace("{}", ir.toString());
        }
    } else {
        logger->trace("Event {} not found", eventId);
    }
}

void EvtProgram::dumpAll() const {
    for (const auto &[id, _] : _eventsById) {
        dump(id);
    }
}
