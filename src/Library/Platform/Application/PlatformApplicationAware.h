#pragma once

#include <cassert>

#include "PlatformComponentStorage.h"

class PlatformApplication;

/**
 * Derive your component from `PlatformApplicationAware` to get access to an application instance that it was
 * installed into.
 */
class PlatformApplicationAware {
 protected:
    ~PlatformApplicationAware() = default; // Don't destroy instances through this interface.

    PlatformApplication *application() const {
        return _application;
    }

    template<class T>
    T *component() const {
        assert(_application); // Should be installed.
        return _componentStorage->require<T>();
    }

    /**
     * Called after installation into a `PlatformApplication`. `application()` will return non-null inside this function.
     */
    virtual void installNotify() {}

    /**
     * Called before removal from a `PlatformApplication`. `application()` will return non-null inside this function.
     */
    virtual void removeNotify() {}

 private:
    friend class PlatformApplication;

    void initialize(PlatformApplication *application, const PlatformComponentStorage *componentStorage) {
        _application = application;
        _componentStorage = componentStorage;
    }

    void deinitialize() {
        _application = nullptr;
        _componentStorage = nullptr;
    }

 private:
    PlatformApplication *_application = nullptr;
    const PlatformComponentStorage *_componentStorage = nullptr;
};
