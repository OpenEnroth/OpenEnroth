#include "PlatformApplication.h"

#include <cassert>

#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyEventLoop.h"
#include "Platform/Proxy/ProxyWindow.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Filters/FilteringEventHandler.h"
#include "Platform/PlatformLogger.h"

class ApplicationProxy : public ProxyPlatform, public ProxyEventLoop, public ProxyWindow, public ProxyOpenGLContext {
 public:
    ApplicationProxy() {}

    virtual std::unique_ptr<PlatformWindow> CreateWindow() override {
        assert(false && "use PlatformApplication::window");
        return nullptr;
    }

    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override {
        assert(false && "use PlatformApplication::eventLoop");
        return nullptr;
    }

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &) override {
        assert(false && "use PlatformApplication::initializeOpenGLContext");
        return nullptr;
    }
};

template<class T>
static void installTypedProxy(ProxyBase<T> *root, ProxyBase<T> *proxy) {
    T *tmp = root->Base();
    root->SetBase(proxy);
    proxy->SetBase(tmp);
}

template<class T>
static void removeTypedProxy(ProxyBase<T> *root, ProxyBase<T> *proxy, T *leaf) {
    while (true) {
        assert(root); // Can only trigger this if passing invalid leaf

        if (root->Base() == leaf)
            return;

        ProxyBase<T> *next = static_cast<ProxyBase<T> *>(root->Base());
        if (proxy == next) {
            root->SetBase(proxy->Base());
            proxy->SetBase(nullptr);
            return;
        }

        root = next;
    }
}

template<class T>
static void initProxyLeaf(ProxyBase<T> *root, T *leaf) {
    while (root->Base() != nullptr)
        root = static_cast<ProxyBase<T> *>(root->Base());
    root->SetBase(leaf);
}

PlatformApplication::PlatformApplication(PlatformLogger *logger) : _logger(logger) {
    assert(logger);

    _platform = Platform::CreateStandardPlatform(logger);
    _eventLoop = _platform->CreateEventLoop();
    _window = _platform->CreateWindow();
    _eventHandler = std::make_unique<FilteringEventHandler>();

    _rootProxy = std::make_unique<ApplicationProxy>();
    initProxyLeaf<Platform>(_rootProxy.get(), _platform.get());
    initProxyLeaf<PlatformEventLoop>(_rootProxy.get(), _eventLoop.get());
    initProxyLeaf<PlatformWindow>(_rootProxy.get(), _window.get());
}

PlatformApplication::~PlatformApplication() {}

void PlatformApplication::initializeOpenGLContext(const PlatformOpenGLOptions &options) {
    assert(!_openGLContext);

    _openGLContext = _window->CreateOpenGLContext(options);
    initProxyLeaf<PlatformOpenGLContext>(_rootProxy.get(), _openGLContext.get());
}

PlatformLogger *PlatformApplication::logger() {
    return _logger;
}

Platform *PlatformApplication::platform() {
    return _rootProxy.get();
}

PlatformEventLoop *PlatformApplication::eventLoop() {
    return _rootProxy.get();
}

PlatformWindow *PlatformApplication::window() {
    return _rootProxy.get();
}

PlatformOpenGLContext *PlatformApplication::openGLContext() {
    return _rootProxy.get();
}

FilteringEventHandler *PlatformApplication::eventHandler() {
    return _eventHandler.get();
}

void PlatformApplication::installProxy(ProxyPlatform *platform) {
    installTypedProxy<Platform>(_rootProxy.get(), platform);
}

void PlatformApplication::removeProxy(ProxyPlatform *platform) {
    removeTypedProxy<Platform>(_rootProxy.get(), platform, _platform.get());
}

void PlatformApplication::installProxy(ProxyEventLoop *eventLoop) {
    installTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop);
}

void PlatformApplication::removeProxy(ProxyEventLoop *eventLoop) {
    removeTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop, _eventLoop.get());
}

void PlatformApplication::installProxy(ProxyWindow *window) {
    installTypedProxy<PlatformWindow>(_rootProxy.get(), window);
}

void PlatformApplication::removeProxy(ProxyWindow *window) {
    removeTypedProxy<PlatformWindow>(_rootProxy.get(), window, _window.get());
}

void PlatformApplication::installProxy(ProxyOpenGLContext *openGLContext) {
    installTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext);
}

void PlatformApplication::removeProxy(ProxyOpenGLContext *openGLContext) {
    removeTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext, _openGLContext.get());
}

void PlatformApplication::processMessages(int count) {
    eventLoop()->ProcessMessages(eventHandler(), count);
}

void PlatformApplication::waitForMessages() {
    eventLoop()->WaitForMessages();
}
