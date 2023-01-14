#pragma once

#include <cassert>

/**
 * A convenient base class for other platform proxies. It provides common accessors for the base class,
 * and exposes a `NonNullBase` method for derived classes to simplify implementations.
 *
 * Note that it's perfectly OK for a proxy to point to a null base, and in this case `Base` will return `nullptr`,
 * but `NonNullBase` will assert, and thus you cannot really call any methods on such a proxy.
 *
 * @tparam T                            Platform class to proxy, e.g. a `PlatformWindow`.
 */
template<class T>
class ProxyBase : public T {
 public:
    explicit ProxyBase(T *base) : base_(base) {}

    T *Base() const {
        return base_;
    }

    void SetBase(T *base) {
        base_ = base;
    }

 protected:
    T *NonNullBase() const {
        assert(base_);
        return base_;
    }

 private:
    T *base_;
};
