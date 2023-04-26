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
     int executeOneEvent(int step);

 private:
     int _eventId;
     std::vector<EventIR> _events;
     bool _canShowMessages;
     bool _canShowOption;
     bool _readyToExit;
     bool _mapExitTriggered;
     PLAYER_CHOOSE_POLICY _who;
};
