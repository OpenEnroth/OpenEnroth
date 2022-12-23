#pragma once

#include <utility>

/**
 * Scope guard to be used to roll back operations in an exception-safe way.
 *
 * Example usage:
 * \code
 * value = 10;
 * auto guard = ScopeGuard([&] { value = 1; });
 * \endcode
 */
template<class T>
class ScopeGuard {
 public:
    explicit ScopeGuard(T &&callable): callable_(std::move(callable)) {}

    ~ScopeGuard() {
        callable_();
    }

 private:
    T callable_;
};
