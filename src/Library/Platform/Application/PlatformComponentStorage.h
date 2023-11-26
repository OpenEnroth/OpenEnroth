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

 private:
    std::unordered_map<std::type_index, void *> _componentByType;
    std::vector<std::shared_ptr<void>> _components; // shared_ptr<void> to type-erase the destructor.
    std::vector<std::function<void()>> _cleanupRoutines;
};
