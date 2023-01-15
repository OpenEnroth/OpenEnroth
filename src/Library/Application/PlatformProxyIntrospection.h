#pragma once

#include <cassert>
#include <type_traits>

#include "Platform/Proxy/ProxyWindow.h"
#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyEventLoop.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"

class PlatformProxyIntrospection {
 public:
    template<class T>
    explicit PlatformProxyIntrospection(T *proxy) {
        init(&_platform, proxy);
        init(&_eventLoop, proxy);
        init(&_window, proxy);
        init(&_openGlContext, proxy);
        assert(_platform || _eventLoop || _window || _openGlContext);
    }

    template<class Callable>
    void visit(Callable &&callable) const {
        if (_platform)
            callable(_platform);
        if (_eventLoop)
            callable(_eventLoop);
        if (_window)
            callable(_window);
        if (_openGlContext)
            callable(_openGlContext);
    }

 private:
    template<class T, class Y>
    void init(T **field, Y *proxy) {
        if constexpr (std::is_base_of_v<T, Y>)
            *field = proxy;
    }

 private:
    ProxyPlatform *_platform = nullptr;
    ProxyEventLoop *_eventLoop = nullptr;
    ProxyWindow *_window = nullptr;
    ProxyOpenGLContext *_openGlContext = nullptr;
};
