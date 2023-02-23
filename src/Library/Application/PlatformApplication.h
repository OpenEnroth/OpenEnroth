#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <vector>
#include <unordered_map>

#include "Platform/PlatformOpenGLOptions.h"

#include "PlatformIntrospection.h"

class FilteringEventHandler;
class ApplicationProxy;

/**
 * This class ties together everything in platform for a particular use case of an application with a single window.
 *
 * The proper way to use it is by creating plugins that implement the pieces of functionality that you need, and then
 * installing them into the `PlatformApplication`.
 *
 * Supported base classes for plugins are:
 * - All kinds of platform proxies;
 * - `PlatformEventFilter`;
 * - `PlatformApplicationAware`.
 *
 * Plugins can have multiple bases, this is handled automatically. If you want to use private bases, befriend
 * `PlatformIntrospection`.
 *
 * The plugins you install are inserted into the proxy and event filter chains exactly in the order of installation.
 * The ordering is usually important, so it makes sense to install everything in a single place. If this model doesn't
 * work for you (e.g. you have two plugins that implement several proxies that need to be ordered differently w.r.t.
 * each other), then you'll have to split your plugin into several parts.
 */
class PlatformApplication {
 public:
    explicit PlatformApplication(PlatformLogger *logger);
    ~PlatformApplication();

    void initializeOpenGLContext(const PlatformOpenGLOptions &options);

    PlatformLogger *logger();
    Platform *platform();
    PlatformEventLoop *eventLoop();
    PlatformWindow *window();
    PlatformOpenGLContext *openGLContext();
    PlatformEventHandler *eventHandler();

    // TODO(captainurist): rename plugin->component? the functionality here pretty much matches the ecs
    template<class T>
    void install(T *plugin) {
        installInternal(typeid(T), plugin);
        PlatformIntrospection::visit(plugin, [this] (auto *specificPlugin) {
            installInternal(specificPlugin);
        });
    }

    template<class T>
    void install(std::unique_ptr<T> plugin) {
        install(plugin.get());

        // TODO(captainurist): drop shared_ptr here once we have std::move_only_function
        _cleanupRoutines.push_back([this, plugin = std::shared_ptr<T>(plugin.release())] {
            remove(plugin.get());
        });
    }

    template<class T>
    void remove(T *plugin) {
        PlatformIntrospection::visit(plugin, [this] (auto *specificPlugin) {
            removeInternal(specificPlugin);
        });
        removeInternal(typeid(plugin), plugin);
    }

    template<class T>
    bool has() const {
        return hasInternal(typeid(T));
    }

    template<class T>
    T *get() const {
        return static_cast<T *>(getInternal(typeid(T)));
    }

    void processMessages(int count = -1);
    void waitForMessages();

 private:
    void installInternal(ProxyPlatform *platform);
    void removeInternal(ProxyPlatform *platform);
    void installInternal(ProxyEventLoop *eventLoop);
    void removeInternal(ProxyEventLoop *eventLoop);
    void installInternal(ProxyWindow *window);
    void removeInternal(ProxyWindow *window);
    void installInternal(ProxyOpenGLContext *openGLContext);
    void removeInternal(ProxyOpenGLContext *openGLContext);
    void installInternal(PlatformEventFilter *eventFilter);
    void removeInternal(PlatformEventFilter *eventFilter);
    void installInternal(PlatformApplicationAware *aware);
    void removeInternal(PlatformApplicationAware *aware);
    void installInternal(std::type_index pluginType, void *plugin);
    void removeInternal(std::type_index pluginType, void *plugin);

    bool hasInternal(std::type_index pluginType) const;
    void *getInternal(std::type_index pluginType) const;

 private:
    PlatformLogger *_logger;
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<PlatformEventLoop> _eventLoop;
    std::unique_ptr<PlatformWindow> _window;
    std::unique_ptr<PlatformOpenGLContext> _openGLContext;
    std::unique_ptr<FilteringEventHandler> _eventHandler;
    std::unique_ptr<ApplicationProxy> _rootProxy;

    std::unordered_map<std::type_index, void *> _pluginByType;
    std::vector<std::function<void()>> _cleanupRoutines;
};
