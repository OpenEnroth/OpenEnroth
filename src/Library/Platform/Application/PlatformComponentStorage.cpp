#include "PlatformComponentStorage.h"

#include <ranges>

PlatformComponentStorage::PlatformComponentStorage() = default;

PlatformComponentStorage::~PlatformComponentStorage() {
    clear();
}

void PlatformComponentStorage::clear() {
    // First call the routines in reverse order.
    for (const auto &routine : _cleanupRoutines | std::views::reverse)
        routine();

    // At this point components shouldn't be able to call into storage, so clear the externally observable state.
    _componentByType.clear();

    // Then destroy cleanup routines in reverse order.
    while (!_cleanupRoutines.empty())
        _cleanupRoutines.pop_back();

    // Then destroy every component that we own, also in reverse order.
    while (!_components.empty())
        _components.pop_back();
}
