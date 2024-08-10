#pragma once

#include <memory>
#include <functional>
#include <utility>

#include "Library/Platform/Interface/PlatformOpenGLOptions.h"

#include "PlatformIntrospection.h"
#include "PlatformComponentStorage.h"

class FilteringEventHandler;
class ApplicationProxy;

/**
 * This class ties together everything in platform for a particular use case of an application with a single window.
 *
 * The proper way to use it is by creating components that implement the pieces of functionality that you need,
 * and then installing them into the `PlatformApplication`.
 *
 * Supported base classes for components are:
 * - All kinds of platform proxies;
 * - `PlatformEventFilter`;
 * - `PlatformApplicationAware`.
 *
 * Components can have multiple bases, this is handled automatically. If you want to use private bases, befriend
 * `PlatformIntrospection`.
 *
 * The components you install are inserted into the proxy and event filter chains exactly in the order of installation.
 * The ordering is usually important, so it makes sense to install everything in a single place. If this model doesn't
 * work for you (e.g. you have two components that implement several proxies that need to be ordered differently w.r.t.
 * each other), then you'll have to split your component into several parts.
 */
class PlatformApplication {
 public:
    explicit PlatformApplication(Platform *platform);
    ~PlatformApplication();

    void initializeOpenGLContext(const PlatformOpenGLOptions &options);
    void initializeOpenGLContext(std::unique_ptr<PlatformOpenGLContext> context);

    Platform *platform();
    PlatformEventLoop *eventLoop();
    PlatformWindow *window();
    PlatformOpenGLContext *openGLContext();
    PlatformEventHandler *eventHandler();

    template<class T>
    T *installComponent(std::unique_ptr<T> component) {
        PlatformIntrospection::visit(component.get(), [this] (auto *specificComponent) {
            installComponentInternal(specificComponent);
        });

        std::function<void()> cleanup = [this, component = component.get()] {
            PlatformIntrospection::visit(component, [this] (auto *specificComponent) {
                removeComponentInternal(specificComponent);
            });
        };

        return _components.insert(std::move(component), std::move(cleanup));
    }

    template<class T>
    std::unique_ptr<T> removeComponent() {
        return _components.remove<T>();
    }

    const PlatformComponentStorage *components() const {
        return &_components;
    }

    template<class T>
    T *component() const {
        return _components.require<T>();
    }

    void processMessages(int count = -1);
    void waitForMessages();

 private:
    friend class PlatformApplicationAware;

    void installComponentInternal(ProxyPlatform *platform);
    void removeComponentInternal(ProxyPlatform *platform);
    void installComponentInternal(ProxyEventLoop *eventLoop);
    void removeComponentInternal(ProxyEventLoop *eventLoop);
    void installComponentInternal(ProxyWindow *window);
    void removeComponentInternal(ProxyWindow *window);
    void installComponentInternal(ProxyOpenGLContext *openGLContext);
    void removeComponentInternal(ProxyOpenGLContext *openGLContext);
    void installComponentInternal(PlatformEventFilter *eventFilter);
    void removeComponentInternal(PlatformEventFilter *eventFilter);
    void installComponentInternal(PlatformApplicationAware *aware);
    void removeComponentInternal(PlatformApplicationAware *aware);

 private:
    Platform *_platform;
    std::unique_ptr<PlatformEventLoop> _eventLoop;
    std::unique_ptr<PlatformWindow> _window;
    std::unique_ptr<PlatformOpenGLContext> _openGLContext;
    std::unique_ptr<FilteringEventHandler> _eventHandler;
    std::unique_ptr<ApplicationProxy> _rootProxy;
    PlatformComponentStorage _components;
};
