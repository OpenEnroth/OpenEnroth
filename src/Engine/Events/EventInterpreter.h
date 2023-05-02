#pragma once

#include <vector>

#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventMap.h"

class EventInterpreter {
 public:
     bool executeRegular(int startStep);
     bool executeNpcDialogue(int startStep);

     void prepare(const EventMap &eventMap, int eventId, bool canShowMessages);

 protected:
     int executeOneEvent(int step, bool isNpc);

 private:
     int _eventId = 0;
     std::vector<EventIR> _events;
     bool _canShowMessages = false;
     bool _canShowOption = true;
     bool _readyToExit = false;
     bool _mapExitTriggered = false;
     PLAYER_CHOOSE_POLICY _who = CHOOSE_PARTY;
};
