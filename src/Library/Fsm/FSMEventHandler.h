#pragma once

#include <Library/Platform/Filters/PlatformEventFilter.h>

class FsmEventHandler : public PlatformEventFilter {
 public:
    FsmEventHandler() : PlatformEventFilter(EVENTS_ALL) {}
};
