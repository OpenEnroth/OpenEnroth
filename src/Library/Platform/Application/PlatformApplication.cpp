#include "PlatformApplication.h"

#include <cassert>
#include <ranges>
#include <memory>
#include <utility>
#include <vector>

#include "Library/Platform/Proxy/ProxyPlatform.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"
#include "Library/Platform/Proxy/ProxyWindow.h"
#include "Library/Platform/Proxy/ProxyOpenGLContext.h"
#include "Library/Platform/Filters/FilteringEventHandler.h"

class ApplicationProxy : public ProxyPlatform, public ProxyEventLoop, public ProxyWindow, public ProxyOpenGLContext {
 public:
    ApplicationProxy() {}

    virtual std::unique_ptr<PlatformWindow> createWindow() override {
        assert(false && "use PlatformApplication::window");
        return nullptr;
    }

    virtual std::unique_ptr<PlatformEventLoop> createEventLoop() override {
        assert(false && "use PlatformApplication::eventLoop");
        return nullptr;
    }

    virtual std::unique_ptr<PlatformOpenGLContext> createOpenGLContext(const PlatformOpenGLOptions &) override {
        assert(false && "use PlatformApplication::initializeOpenGLContext");
        return nullptr;
    }

    virtual void exec(PlatformEventHandler *eventHandler) override {
        pendingDeletions.clear();
        ProxyEventLoop::exec(eventHandler);
    }

    virtual void processMessages(PlatformEventHandler *eventHandler, int count) override {
        pendingDeletions.clear();
        ProxyEventLoop::processMessages(eventHandler, count);
    }

    std::vector<std::shared_ptr<void>> pendingDeletions;
};

template<class T>
static void installTypedProxy(ProxyBase<T> *root, ProxyBase<T> *proxy) {
    T *tmp = root->base();
    root->setBase(proxy);
    proxy->setBase(tmp);
}

template<class T>
static void removeTypedProxy(ProxyBase<T> *root, ProxyBase<T> *proxy, T *leaf) {
    while (true) {
        assert(root); // Can only trigger this if passing invalid leaf

        if (root->base() == leaf)
            return;

        ProxyBase<T> *next = static_cast<ProxyBase<T> *>(root->base());
        if (proxy == next) {
            root->setBase(proxy->base());
            proxy->setBase(nullptr);
            return;
        }

        root = next;
    }
}

template<class T>
static void initProxyLeaf(ProxyBase<T> *root, T *leaf) {
    while (root->base() != nullptr)
        root = static_cast<ProxyBase<T> *>(root->base());
    root->setBase(leaf);
}

PlatformApplication::PlatformApplication(Platform *platform) : _platform(platform) {
    assert(platform);

    _eventLoop = _platform->createEventLoop();
    _window = _platform->createWindow();
    _eventHandler = std::make_unique<FilteringEventHandler>();

    _rootProxy = std::make_unique<ApplicationProxy>();
    initProxyLeaf<Platform>(_rootProxy.get(), _platform);
    initProxyLeaf<PlatformEventLoop>(_rootProxy.get(), _eventLoop.get());
    initProxyLeaf<PlatformWindow>(_rootProxy.get(), _window.get());
}

PlatformApplication::~PlatformApplication() {
    _components.clear();
}

void PlatformApplication::initializeOpenGLContext(const PlatformOpenGLOptions &options) {
    initializeOpenGLContext(_window->createOpenGLContext(options));
}

void PlatformApplication::initializeOpenGLContext(std::unique_ptr<PlatformOpenGLContext> context) {
    assert(!_openGLContext);
    assert(context);

    _openGLContext = std::move(context);
    initProxyLeaf<PlatformOpenGLContext>(_rootProxy.get(), _openGLContext.get());
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

PlatformEventHandler *PlatformApplication::eventHandler() {
    return _eventHandler.get();
}

void PlatformApplication::processMessages(int count) {
    eventLoop()->processMessages(eventHandler(), count);
}

void PlatformApplication::waitForMessages() {
    eventLoop()->waitForMessages();
}

void PlatformApplication::installComponentInternal(ProxyPlatform *platform) {
    installTypedProxy<Platform>(_rootProxy.get(), platform);
}

void PlatformApplication::removeComponentInternal(ProxyPlatform *platform) {
    removeTypedProxy<Platform>(_rootProxy.get(), platform, _platform);
}

void PlatformApplication::installComponentInternal(ProxyEventLoop *eventLoop) {
    installTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop);
}

void PlatformApplication::removeComponentInternal(ProxyEventLoop *eventLoop) {
    removeTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop, _eventLoop.get());
}

void PlatformApplication::installComponentInternal(ProxyWindow *window) {
    installTypedProxy<PlatformWindow>(_rootProxy.get(), window);
}

void PlatformApplication::removeComponentInternal(ProxyWindow *window) {
    removeTypedProxy<PlatformWindow>(_rootProxy.get(), window, _window.get());
}

void PlatformApplication::installComponentInternal(ProxyOpenGLContext *openGLContext) {
    installTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext);
}

void PlatformApplication::removeComponentInternal(ProxyOpenGLContext *openGLContext) {
    removeTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext, _openGLContext.get());
}

void PlatformApplication::installComponentInternal(PlatformEventFilter *eventFilter) {
    _eventHandler->installEventFilter(eventFilter);
}

void PlatformApplication::removeComponentInternal(PlatformEventFilter *eventFilter) {
    _eventHandler->removeEventFilter(eventFilter);
}

void PlatformApplication::installComponentInternal(PlatformApplicationAware *aware) {
    assert(aware->application() == nullptr);
    aware->initialize(this, components());
    aware->installNotify();
}

void PlatformApplication::removeComponentInternal(PlatformApplicationAware *aware) {
    if (aware->application() == nullptr)
        return; // All remove methods allow double-removal.

    assert(aware->application() == this);
    aware->removeNotify();
    aware->deinitialize();
}

void PlatformApplication::deleteLater(std::shared_ptr<void> component) {
    _rootProxy->pendingDeletions.emplace_back(std::move(component));
}
