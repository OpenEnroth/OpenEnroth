#pragma once

#include <cstdlib>

// TODO(captainurist): move to Memory?

/**
 * Deleter class to be used with `std::unique_ptr` for managing memory allocated with `malloc`.
 *
 * Example usage:
 * ```
 * std::unique_ptr<void, FreeDeleter> ptr(malloc(1000));
 * ```
 */
struct FreeDeleter {
    template <typename T>
    void operator()(const T *p) const {
        std::free(const_cast<T *>(p));
    }
};
