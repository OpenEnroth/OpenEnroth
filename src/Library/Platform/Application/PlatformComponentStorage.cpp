#include "PlatformComponentStorage.h"

#include <ranges>

PlatformComponentStorage::PlatformComponentStorage() = default;

PlatformComponentStorage::~PlatformComponentStorage() {
    // First call the routines in reverse order.
    for (const auto &routine : _cleanupRoutines | std::views::reverse)
        routine();

    // Then destroy cleanup routines in reverse order.
    while (!_cleanupRoutines.empty())
        _cleanupRoutines.pop_back();

    // Then destroy every component that we own, also in reverse order.
    while (!_components.empty())
        _components.pop_back();
}
