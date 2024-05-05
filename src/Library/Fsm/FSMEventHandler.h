#pragma once

#include <Library/Platform/Filters/PlatformEventFilter.h>

class FSMEventHandler : public PlatformEventFilter {
 public:
    FSMEventHandler() : PlatformEventFilter(EVENTS_ALL) {}
};
