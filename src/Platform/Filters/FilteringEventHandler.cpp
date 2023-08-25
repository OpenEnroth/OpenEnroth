#include "FilteringEventHandler.h"

#include "Utility/ScopeGuard.h"
#include "PlatformEventFilter.h"
#include "Platform/PlatformEvents.h"

void FilteringEventHandler::event(const PlatformEvent *event) {
    _insideEvent = true;
    MM_AT_SCOPE_EXIT(_insideEvent = false);

    for (PlatformEventFilter *filter : _filters[event->type] | std::views::reverse)
        if (filter->event(event))
            return;
}

void FilteringEventHandler::installEventFilter(PlatformEventFilter *filter) {
    assert(!_insideEvent); // This can be properly supported, but right now it's not really needed.

    for (PlatformEventType eventType : filter->eventTypes())
        _filters[eventType].push_back(filter);
}

void FilteringEventHandler::removeEventFilter(PlatformEventFilter *filter) {
    assert(!_insideEvent); // This can be properly supported, but right now it's not really needed.

    for (PlatformEventType eventType : filter->eventTypes())
        std::erase(_filters[eventType], filter);
}
