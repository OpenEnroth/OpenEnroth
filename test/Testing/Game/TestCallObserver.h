#pragma once

#include <functional>
#include <typeinfo>
#include <memory>

#include "Engine/EngineCallObserver.h"

#include "Utility/IndexedArray.h"

#include "AccessibleVector.h"

class TestCallObserver : public EngineCallObserver {
 public:
    void reset() {
        _enabled = false;
        _handlers.fill({});
    }

    bool isEnabled() const {
        return _enabled;
    }

    void setEnabled(bool enabled) {
        _enabled = enabled;
    }

    template<class T>
    std::function<AccessibleVector<T>()> record(EngineCall call) {
        assert(!_handlers[call]); // Are you creating the same tape twice?

        std::shared_ptr<AccessibleVector<T>> sharedState = std::make_shared<AccessibleVector<T>>();

        // That's the push side - engine will be adding new values into the shared state using this callback.
        _handlers[call] = [sharedState] (const std::type_info &type, const void *data) {
            assert(type == typeid(T));
            sharedState->push_back(*static_cast<const T *>(data));
        };

        // And that's the pull side - this function will be called between frames to return what was accumulated in the
        // shared state.
        return [sharedState] {
            AccessibleVector<T> result;
            sharedState->swap(result);
            return result;
        };
    }

 protected:
    virtual void notify(EngineCall call, const std::type_info &type, const void *data) override {
        if (_enabled && _handlers[call])
            _handlers[call](type, data);
    }

 private:
    bool _enabled = false;
    IndexedArray<std::function<void(const std::type_info &, const void *)>, CALL_FIRST, CALL_LAST> _handlers;
};
