#pragma once

#include <cassert>
#include <utility>
#include <type_traits>

#include "Library/Platform/Proxy/ProxyWindow.h"
#include "Library/Platform/Proxy/ProxyPlatform.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"
#include "Library/Platform/Proxy/ProxyOpenGLContext.h"
#include "Library/Platform/Filters/PlatformEventFilter.h"

#include "PlatformApplicationAware.h"

class PlatformIntrospection {
 public:
    template<class T, class Callable>
    static void visit(T *component, Callable &&callable) {
        int count = 0;
        // Using += instead of one big expression to get deterministic call order.
        count += visitInternal<ProxyPlatform>(component, std::forward<Callable>(callable));
        count += visitInternal<ProxyEventLoop>(component, std::forward<Callable>(callable));
        count += visitInternal<ProxyWindow>(component, std::forward<Callable>(callable));
        count += visitInternal<ProxyOpenGLContext>(component, std::forward<Callable>(callable));
        count += visitInternal<PlatformEventFilter>(component, std::forward<Callable>(callable));
        count += visitInternal<PlatformApplicationAware>(component, std::forward<Callable>(callable));
        assert(count > 0); // Must derive from at least one of the supported types.
        (void) count;
    }

 private:
    template<class Installable, class T, class Callable>
    static bool visitInternal(T *component, Callable &&callable) {
        if constexpr (std::is_base_of_v<Installable, T>) {
            callable(static_cast<Installable *>(component));
            return true;
        } else {
            return false;
        }
    }
};
