#pragma once

#include <cassert>
#include <utility>
#include <type_traits>

#include "Platform/Proxy/ProxyWindow.h"
#include "Platform/Proxy/ProxyPlatform.h"
#include "Platform/Proxy/ProxyEventLoop.h"
#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Filters/PlatformEventFilter.h"

#include "PlatformApplicationAware.h"

class PlatformIntrospection {
 public:
    template<class T, class Callable>
    static void visit(T *plugin, Callable &&callable) {
        int count = 0;
        // Using += instead of one big expression to get deterministic call order.
        count += visitInternal<ProxyPlatform>(plugin, std::forward<Callable>(callable));
        count += visitInternal<ProxyEventLoop>(plugin, std::forward<Callable>(callable));
        count += visitInternal<ProxyWindow>(plugin, std::forward<Callable>(callable));
        count += visitInternal<ProxyOpenGLContext>(plugin, std::forward<Callable>(callable));
        count += visitInternal<PlatformEventFilter>(plugin, std::forward<Callable>(callable));
        count += visitInternal<PlatformApplicationAware>(plugin, std::forward<Callable>(callable));
        assert(count > 0);
        (void) count;
    }

 private:
    template<class Installable, class T, class Callable>
    static bool visitInternal(T *plugin, Callable &&callable) {
        if constexpr (std::is_base_of_v<Installable, T>) {
            callable(static_cast<Installable *>(plugin));
            return true;
        } else {
            return false;
        }
    }
};
