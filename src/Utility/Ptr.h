#pragma once

#include <concepts>
#include <cstddef>
#include <memory>

#include "Utility/Attributes.h"

/**
 * Non-owning pointer that converts implicitly from a raw pointer or a smart pointer, so that the caller doesn't have
 * to spell `.get()`.
 *
 * It doesn't own anything, doesn't extend the lifetime of what it points to, and doesn't touch the reference count.
 * It is meant for function parameters only, and should not be stored.
 */
template<class T>
class Ptr {
 public:
    constexpr Ptr() noexcept = default;
    constexpr Ptr(std::nullptr_t) noexcept {} // NOLINT: implicit conversion is intended.
    constexpr Ptr(T *ptr) noexcept : _ptr(ptr) {} // NOLINT: implicit conversion is intended.

    template<class U> requires std::convertible_to<U *, T *>
    Ptr(const std::shared_ptr<U> &ptr MM_LIFETIMEBOUND) noexcept : _ptr(ptr.get()) {} // NOLINT: implicit conversion is intended.

    template<class U, class D> requires std::convertible_to<U *, T *>
    Ptr(const std::unique_ptr<U, D> &ptr MM_LIFETIMEBOUND) noexcept : _ptr(ptr.get()) {} // NOLINT: implicit conversion is intended.

    constexpr T *get() const noexcept {
        return _ptr;
    }

    constexpr T *operator->() const noexcept {
        return _ptr;
    }

    constexpr T &operator*() const noexcept {
        return *_ptr;
    }

    constexpr operator T *() const noexcept { // NOLINT: implicit conversion is intended.
        return _ptr;
    }

 private:
    T *_ptr = nullptr;
};
