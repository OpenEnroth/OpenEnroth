#pragma once

#include <memory>

#include "Platform/PlatformOpenGLOptions.h"

#include "PlatformProxyIntrospection.h"

class FilteringEventHandler;
class ApplicationProxy;

/**
 * This class ties together everything in platform for a particular use case of an application with a single window.
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
    FilteringEventHandler *eventHandler();

    void installProxy(ProxyPlatform *platform);
    void removeProxy(ProxyPlatform *platform);
    void installProxy(ProxyEventLoop *eventLoop);
    void removeProxy(ProxyEventLoop *eventLoop);
    void installProxy(ProxyWindow *window);
    void removeProxy(ProxyWindow *window);
    void installProxy(ProxyOpenGLContext *openGLContext);
    void removeProxy(ProxyOpenGLContext *openGLContext);

    template<class T>
    void installProxy(T *proxy) {
        PlatformProxyIntrospection(proxy).visit([this] (auto *specificProxy) {
            installProxy(specificProxy);
        });
    }

    template<class T>
    void removeProxy(T *proxy) {
        PlatformProxyIntrospection(proxy).visit([this] (auto *specificProxy) {
            removeProxy(specificProxy);
        });
    }

    void processMessages(int count = -1);
    void waitForMessages();

 private:
    PlatformLogger *_logger;
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<PlatformEventLoop> _eventLoop;
    std::unique_ptr<PlatformWindow> _window;
    std::unique_ptr<PlatformOpenGLContext> _openGLContext;
    std::unique_ptr<FilteringEventHandler> _eventHandler;
    std::unique_ptr<ApplicationProxy> _rootProxy;
};
