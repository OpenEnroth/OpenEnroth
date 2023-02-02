#pragma once

#include <utility>

#include "Preprocessor.h"

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

/**
 * Runs provided statements at scope exit by creating a temporary `ScopeGuard` object.
 */
#define MM_AT_SCOPE_EXIT(...)                                                                                              \
    auto MM_PP_CAT(guard, __LINE__) = ScopeGuard([&] { __VA_ARGS__; })

