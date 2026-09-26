#pragma once

#include <vector>

#include "Engine/Pid.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtProgram.h"

#include "Library/Geometry/Vec.h"

/**
 * What an instruction tells the interpreter to do next.
 */
enum class EvtOutcome {
    EVT_OUTCOME_NEXT, // Go on with the next step.
    EVT_OUTCOME_JUMP, // Go on with the step in `EvtResult::target`.
    EVT_OUTCOME_STOP, // The event ends here.
    EVT_OUTCOME_WAIT, // The event ends here, and a dialogue it opened goes on with it once it closes.
};
using enum EvtOutcome;

/**
 * What `EvtInterpreter::executeInstruction` returns.
 */
struct EvtResult {
    EvtOutcome outcome = EVT_OUTCOME_NEXT;
    int target = 0; // Step to go on with, for `EVT_OUTCOME_JUMP`.
};

// EvtInterpreter
class EvtInterpreter {
 public:
     bool executeRegular(int startStep);
     bool executeNpcDialogue(int startStep);

     void prepare(const EvtProgram &eventMap, int eventId, Pid objectPid, bool canShowMessages);
     bool isValid();

 protected:
     int executeOneEvent(int step, bool isNpc);

     /**
      * @param ir                       Instruction to run, outside of NPC mode.
      * @return                         What the event does next. A condition that holds jumps, and so does a
      *                                 `RandomGoTo`.
      */
     EvtResult executeInstruction(EvtInstruction ir);

 private:
     int _eventId = 0;
     std::vector<EvtInstruction> _events;
     Pid _objectPid = Pid();
     bool _canShowMessages = false;
     bool _canShowOption = true;
     bool _readyToExit = false;
     bool _mapExitTriggered = false;
     EvtTargetCharacter _who = CHOOSE_PARTY;
};

void spawnMonsters(int16_t typeindex, int16_t level, int count,
    Vec3f pos, int group, int uUniqueName);
