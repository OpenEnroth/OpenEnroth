#pragma once

#include <vector>

#include "Engine/Pid.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtProgram.h"

#include "Library/Geometry/Vec.h"

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
      * @param ir                       Instruction to run.
      * @return                         Step to continue from, which is `ir.target_step` if the instruction jumped,
      *                                 the picked step of a `RandomGoTo` and `ir.step + 1` otherwise, or -1 if the
      *                                 event has to stop here.
      */
     int executeInstruction(EvtInstruction ir);

     /**
      * @param who                      Characters that the instructions that follow apply to, as `ForPartyMember` sets.
      */
     void setTargetCharacter(EvtTargetCharacter who) {
         _who = who;
     }

     /**
      * @return                         Whether an instruction asked for more than the party has, e.g. gold, which
      *                                 stops the event.
      */
     bool isCancelled() const {
         return _cancelled;
     }

     bool isMapExitTriggered() const {
         return _mapExitTriggered;
     }

 protected:
     int executeOneEvent(int step, bool isNpc);

 private:
     int _eventId = 0;
     std::vector<EvtInstruction> _events;
     Pid _objectPid = Pid();
     bool _canShowMessages = false;
     bool _canShowOption = true;
     bool _readyToExit = false;
     bool _mapExitTriggered = false;
     bool _cancelled = false; // Set when a script asks for more than the party has, e.g. gold, and aborts it.
     EvtTargetCharacter _who = CHOOSE_PARTY;
};

void spawnMonsters(int16_t typeindex, int16_t level, int count,
    Vec3f pos, int group, int uUniqueName);
