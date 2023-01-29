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
    static void visit(T *mixin, Callable &&callable) {
        int count = 0;
        // Using += instead of one big expression to get deterministic call order.
        count += visitInternal<ProxyPlatform>(mixin, std::forward<Callable>(callable));
        count += visitInternal<ProxyEventLoop>(mixin, std::forward<Callable>(callable));
        count += visitInternal<ProxyWindow>(mixin, std::forward<Callable>(callable));
        count += visitInternal<ProxyOpenGLContext>(mixin, std::forward<Callable>(callable));
        count += visitInternal<PlatformEventFilter>(mixin, std::forward<Callable>(callable));
        count += visitInternal<PlatformApplicationAware>(mixin, std::forward<Callable>(callable));
        assert(count > 0);
        (void) count;
    }

 private:
    template<class Installable, class T, class Callable>
    static bool visitInternal(T *mixin, Callable &&callable) {
        if constexpr (std::is_base_of_v<Installable, T>) {
            callable(static_cast<Installable *>(mixin));
            return true;
        } else {
            return false;
        }
    }
};
