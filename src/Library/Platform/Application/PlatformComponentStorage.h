#pragma once

#include <cassert>
#include <utility>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <vector>
#include <memory>

#include "Utility/MapAccess.h"

class PlatformApplicationAware;

/**
 * Type-erased storage for components.
 *
 * Note that the storage takes ownership of the inserted components, and they are removed & destroyed only when the
 * storage itself is destroyed.
 *
 * There used to be a non-owning interface here, but in the end it just made very little sense. Non-owning interface
 * means that we'll also need `erase` / `remove` methods. And looking at it a user might then conclude that since
 * we're providing both `insert` and `remove`, then it should be OK to dynamically insert & remove components, which
 * is totally NOT OK. Storage itself doesn't care about insertion order, but the enclosing `PlatformApplication`
 * maintains proxy and event filter chains, for which insertion order matters. Letting the user insert & remove
 * components on the fly will result in hard to debug problems with components ending up in the wrong place.
 */
class PlatformComponentStorage {
 public:
    PlatformComponentStorage();
    ~PlatformComponentStorage();

    template<class T>
    void insert(std::unique_ptr<T> component, std::function<void()> cleanupRoutine) {
        assert(component);

        std::type_index index = typeid(T);
        assert(!_componentByType.contains(index));

        _componentByType.emplace(index, component.get());
        _components.emplace_back(std::move(component));
        _cleanupRoutines.emplace_back(std::move(cleanupRoutine));
    }

    template<class T>
    bool contains() const {
        return _componentByType.contains(typeid(T));
    }

    template<class T>
    T *require() const {
        T *result = static_cast<T *>(valueOr(_componentByType, typeid(T), nullptr));
        assert(result);
        return static_cast<T *>(result);
    }

    void clear();

 private:
    std::unordered_map<std::type_index, void *> _componentByType;
    std::vector<std::shared_ptr<void>> _components; // shared_ptr<void> to type-erase the destructor.
    std::vector<std::function<void()>> _cleanupRoutines;
};
