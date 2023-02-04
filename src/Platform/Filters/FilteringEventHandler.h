#pragma once

#include <vector>

#include "Platform/PlatformEventHandler.h"

#include "Utility/IndexedArray.h"

class PlatformEventFilter;

class FilteringEventHandler : public PlatformEventHandler {
 public:
    virtual void event(const PlatformEvent *event) override;

    void installEventFilter(PlatformEventFilter *filter);
    void removeEventFilter(PlatformEventFilter *filter);

 private:
    IndexedArray<std::vector<PlatformEventFilter *>, PlatformEvent::FirstEventType, PlatformEvent::LastEventType> _filters;
    bool _insideEvent = false;
};
