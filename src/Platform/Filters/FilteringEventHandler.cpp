#include "FilteringEventHandler.h"

#include "Utility/Reversed.h"
#include "Utility/ScopeGuard.h"

#include "PlatformEventFilter.h"

void FilteringEventHandler::Event(PlatformWindow *window, const PlatformEvent *event) {
    _insideEvent = true;
    auto guard = ScopeGuard([&] { _insideEvent = false; });

    for (PlatformEventFilter *filter : Reversed(_filters[event->type]))
        if (filter->Event(window, event))
            return;
}

void FilteringEventHandler::installEventFilter(PlatformEventFilter *filter) {
    assert(!_insideEvent); // This can be properly supported, but right now it's not really needed.

    for (PlatformEvent::Type eventType : filter->eventTypes())
        _filters[eventType].push_back(filter);
}

void FilteringEventHandler::removeEventFilter(PlatformEventFilter *filter) {
    assert(!_insideEvent); // This can be properly supported, but right now it's not really needed.

    for (PlatformEvent::Type eventType : filter->eventTypes())
        std::erase(_filters[eventType], filter);
}
