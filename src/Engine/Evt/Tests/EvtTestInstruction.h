#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtInstruction.h"

inline EvtInstruction evtTestInstruction(std::string_view name, const std::vector<EvtFieldValue> &values) {
    const EvtCommandInfo *command = evtCommand(name);
    EvtInstruction result(command->opcode);
    for (size_t i = 0; i < values.size(); i++)
        if (command->fields[i].set)
            command->fields[i].set(&result, values[i]);
    return result;
}
