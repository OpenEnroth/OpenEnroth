#pragma once

#include <vector>

#include "Engine/Pid.h"
#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventMap.h"
#include "Engine/Events/EventEnums.h"

class EventMap;

class EventInterpreter {
 public:
     bool executeRegular(int startStep);
     bool executeNpcDialogue(int startStep);

     void prepare(const EventMap &eventMap, int eventId, Pid objectPid, bool canShowMessages);

 protected:
     int executeOneEvent(int step, bool isNpc);

 private:
     int _eventId = 0;
     std::vector<EventIR> _events;
     Pid _objectPid = Pid();
     bool _canShowMessages = false;
     bool _canShowOption = true;
     bool _readyToExit = false;
     bool _mapExitTriggered = false;
     CharacterChoosePolicy _who = CHOOSE_PARTY;
};
