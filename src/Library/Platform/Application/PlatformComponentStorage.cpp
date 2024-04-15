#include "PlatformComponentStorage.h"

#include <cassert>
#include <algorithm>
#include <ranges>
#include <vector>
#include <utility>

PlatformComponentStorage::PlatformComponentStorage() = default;

PlatformComponentStorage::~PlatformComponentStorage() {
    clear();
}

void PlatformComponentStorage::clear() {
    // We need to clean up everything in reverse order to how it was added.
    std::vector<Data *> order;
    for (auto &[_, data] : _dataByType)
        order.push_back(&data);
    std::ranges::sort(order, std::ranges::greater(), &Data::index);

    // First call the cleanup routines in reverse order. These will call into PlatformApplicationAware::removeNotify.
    for (Data *data : order)
        data->cleanupRoutine();

    // At this point components shouldn't be able to call into storage, so clear the externally observable state.
    auto tmp = std::move(_dataByType);
    assert(_dataByType.empty());

    // Then destroy every component that we own, also in reverse order.
    for (Data *data : order)
        data->component.reset();
}
