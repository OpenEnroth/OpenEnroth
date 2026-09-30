#pragma once

#include <vector>

#include "Engine/Pid.h"
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

     void prepare(const EvtProgram &eventMap, int eventId, Pid objectPid, bool canShowMessages);
     bool isValid();

     /**
      * Prepares to run instructions that don't come from an evt file, one `executeInstruction` call at a time.
      *
      * @param eventId                  Id of the event the instructions run as.
      * @param objectPid                Object that triggered the event.
      * @param canShowMessages          Whether the event can show status texts and open dialogues.
      */
     void prepare(int eventId, Pid objectPid, bool canShowMessages);

     /**
      * @param ir                       Instruction to run, outside of NPC mode.
      * @return                         Where the event goes after this instruction.
      */
     EvtFlow executeInstruction(EvtInstruction ir);

     /**
      * @param who                      Characters that the instructions that follow apply to, as `ForPartyMember` sets.
      */
     void setTargetCharacter(EvtTargetCharacter who) {
         _who = who;
     }

     bool isMapExitTriggered() const {
         return _mapExitTriggered;
     }

 protected:
     int executeOneEvent(int step, bool isNpc);

 private:
     /**
      * Logs an error naming the event and step when the command's value is out of range for its variable.
      *
      * @param ir                       A compare, set, add or subtract command.
      * @return                         Whether the value is in range.
      */
     [[nodiscard]] bool validateVariableValue(const EvtInstruction &ir) const;

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
