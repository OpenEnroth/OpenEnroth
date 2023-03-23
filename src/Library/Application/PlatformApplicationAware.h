#pragma once

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

    void setApplication(PlatformApplication *application) {
        _application = application;
    }

 private:
    PlatformApplication *_application = nullptr;
};
