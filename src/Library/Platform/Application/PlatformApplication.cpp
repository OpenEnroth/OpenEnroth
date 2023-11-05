#include "PlatformApplication.h"

#include <cassert>
#include <ranges> // NOLINT

#include "Library/Platform/Proxy/ProxyPlatform.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"
#include "Library/Platform/Proxy/ProxyWindow.h"
#include "Library/Platform/Proxy/ProxyOpenGLContext.h"
#include "Library/Platform/Filters/FilteringEventHandler.h"

#include "Utility/MapAccess.h"

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
    // First call the routines in reverse order - this should uninstall everything.
    for (const auto &routine : _cleanupRoutines | std::views::reverse)
        routine();

    // User should uninstall all components that platform application doesn't own before destroying the platform application.
    assert(_componentByType.empty());

    // Then destroy every component that we own.
    while (!_cleanupRoutines.empty())
        _cleanupRoutines.pop_back();
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

void PlatformApplication::installInternal(ProxyPlatform *platform) {
    installTypedProxy<Platform>(_rootProxy.get(), platform);
}

void PlatformApplication::removeInternal(ProxyPlatform *platform) {
    removeTypedProxy<Platform>(_rootProxy.get(), platform, _platform);
}

void PlatformApplication::installInternal(ProxyEventLoop *eventLoop) {
    installTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop);
}

void PlatformApplication::removeInternal(ProxyEventLoop *eventLoop) {
    removeTypedProxy<PlatformEventLoop>(_rootProxy.get(), eventLoop, _eventLoop.get());
}

void PlatformApplication::installInternal(ProxyWindow *window) {
    installTypedProxy<PlatformWindow>(_rootProxy.get(), window);
}

void PlatformApplication::removeInternal(ProxyWindow *window) {
    removeTypedProxy<PlatformWindow>(_rootProxy.get(), window, _window.get());
}

void PlatformApplication::installInternal(ProxyOpenGLContext *openGLContext) {
    installTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext);
}

void PlatformApplication::removeInternal(ProxyOpenGLContext *openGLContext) {
    removeTypedProxy<PlatformOpenGLContext>(_rootProxy.get(), openGLContext, _openGLContext.get());
}

void PlatformApplication::installInternal(PlatformEventFilter *eventFilter) {
    _eventHandler->installEventFilter(eventFilter);
}

void PlatformApplication::removeInternal(PlatformEventFilter *eventFilter) {
    _eventHandler->removeEventFilter(eventFilter);
}

void PlatformApplication::installInternal(PlatformApplicationAware *aware) {
    assert(aware->application() == nullptr);
    aware->setApplication(this);
    aware->installNotify();
}

void PlatformApplication::removeInternal(PlatformApplicationAware *aware) {
    if (aware->application() == nullptr)
        return; // All remove methods allow double-removal.

    assert(aware->application() == this);
    aware->removeNotify();
    aware->setApplication(nullptr);
}

void PlatformApplication::installInternal(std::type_index componentType, void *component) {
    assert(component);
    assert(!_componentByType.contains(componentType));
    _componentByType.emplace(componentType, component);
}

void PlatformApplication::removeInternal(std::type_index componentType, void *component) {
    assert(component);
    assert(!_componentByType.contains(componentType) || valueOr(_componentByType, componentType, nullptr) == component);
    _componentByType.erase(componentType);
}

bool PlatformApplication::hasInternal(std::type_index componentType) const {
    return _componentByType.contains(componentType);
}

void *PlatformApplication::getInternal(std::type_index componentType) const {
    return valueOr(_componentByType, componentType, nullptr);
}

