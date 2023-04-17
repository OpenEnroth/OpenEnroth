#pragma once

#include "Engine/Events/EventParser.h"

void dumpEvent(int eventId, bool isGlobal);
void dumpAllEvents(bool isGlobal);

inline void dumpEventGlobal(int eventId) {
    dumpEvent(eventId, true);
}
inline void dumpEventLocal(int eventId) {
    dumpEvent(eventId, false);
}
inline void dumpAllEventsGlobal(int eventId) {
    dumpAllEvents(true);
}
inline void dumpAllEventsLocal(int eventId) {
    dumpAllEvents(false);
}
