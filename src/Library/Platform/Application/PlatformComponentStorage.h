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
 * While this class offers `remove` method, its usage is discouraged. The reason is that even though the storage itself
 * doesn't care about insertion order, the enclosing `PlatformApplication` maintains proxy and event filter chains,
 * for which insertion order matters. So inserting & removing components on the fly will likely result in hard to debug
 * problems with components ending up in the wrong place.
 */
class PlatformComponentStorage {
 public:
    PlatformComponentStorage();
    ~PlatformComponentStorage();

    template<class T>
    T *insert(std::unique_ptr<T> component, std::function<void()> cleanupRoutine) {
        assert(component);

        std::type_index index = typeid(T);
        assert(!_dataByType.contains(index));

        T *result = component.get();

        Data data;
        data.component = std::unique_ptr<void, void(*)(void *)>(component.release(), &PlatformComponentStorage::destructor<T>);
        data.cleanupRoutine = std::move(cleanupRoutine);
        data.index = _nextIndex++;
        _dataByType.emplace(index, std::move(data));

        return result;
    }

    template<class T>
    std::unique_ptr<T> remove() {
        auto pos = _dataByType.find(typeid(T));
        assert(pos != _dataByType.end());

        pos->second.cleanupRoutine();
        std::unique_ptr<T> result(static_cast<T *>(pos->second.component.release()));
        _dataByType.erase(pos);
        return result;
    }

    template<class T>
    bool contains() const {
        return _dataByType.contains(typeid(T));
    }

    template<class T>
    T *require() const {
        const Data *data = valuePtr(_dataByType, typeid(T));
        assert(data);
        return static_cast<T *>(data->component.get());
    }

    void clear();

 private:
    template<class T>
    static void destructor(void *p) {
        delete static_cast<T *>(p);
    }

    struct Data {
        std::unique_ptr<void, void(*)(void *)> component = {nullptr, nullptr};
        std::function<void()> cleanupRoutine;
        int index = 0;
    };

 private:
    std::unordered_map<std::type_index, Data> _dataByType;
    int _nextIndex = 0;
};
