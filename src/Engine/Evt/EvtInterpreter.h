#pragma once

#include <vector>

#include "Engine/Evt/EvtContext.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtProgram.h"

#include "Library/Geometry/Vec.h"

/**
 * What an instruction tells the interpreter to do next.
 */
enum class EvtFlowType {
    EVT_FLOW_NEXT,  // Go on with the next step.
    EVT_FLOW_JUMP,  // Go on with the step in `EvtFlow::target`.
    EVT_FLOW_STOP,  // End the event.
    EVT_FLOW_YIELD, // The event pauses here, and the dialogue it opened resumes it at the next step if the player goes on.
};
using enum EvtFlowType;

/**
 * Where an event goes after one of its instructions ran.
 */
struct EvtFlow {
    EvtFlowType type = EVT_FLOW_NEXT;
    int target = 0; // Step to go on with, for `EVT_FLOW_JUMP`.
};

// EvtInterpreter
class EvtInterpreter {
 public:
     bool executeRegular(int startStep);
     bool executeNpcDialogue(int startStep);

     /**
      * @param eventMap                 Program to take the event from, the one `context.source` names.
      * @param context                  Event to run, and what it runs for.
      */
     void prepare(const EvtProgram &eventMap, const EvtContext &context);
     bool isValid();

 protected:
     int executeOneEvent(int step, bool isNpc);

     /**
      * @param ir                       Instruction to run, outside of NPC mode.
      * @return                         Where the event goes after this instruction.
      */
     EvtFlow executeInstruction(EvtInstruction ir);

 private:
     /**
      * Logs an error naming the event and step when the command's value is out of range for its variable.
      *
      * @param ir                       A compare, set, add or subtract command.
      * @return                         Whether the value is in range.
      */
     [[nodiscard]] bool validateVariableValue(const EvtInstruction &ir) const;

 private:
     EvtContext _context;
     std::vector<EvtInstruction> _events;
     bool _canShowOption = true;
     bool _readyToExit = false;
     bool _mapExitTriggered = false;
     EvtTargetCharacter _who = CHOOSE_PARTY;
};

void spawnMonsters(int16_t typeindex, int16_t level, int count,
    Vec3f pos, int group, int uUniqueName);
