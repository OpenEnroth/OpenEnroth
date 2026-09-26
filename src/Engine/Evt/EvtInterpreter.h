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
     bool _cancelled = false; // Set when a script asks for more than the party has, e.g. gold, and aborts it.
     EvtTargetCharacter _who = CHOOSE_PARTY;
};

void spawnMonsters(int16_t typeindex, int16_t level, int count,
    Vec3f pos, int group, int uUniqueName);
