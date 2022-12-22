#pragma once

#include <utility>

/**
 * Base class implementing the base-from-member idiom.
 *
 * @tparam T                            Stored type.
 */
template<class T>
class Embedded {
 public:
    template<class... Args>
    explicit Embedded(Args &&...args): data_(std::forward<Args>(args)...) {}

    T &get() {
        return data_;
    }

    const T &get() const {
        return data_;
    }

 private:
    T data_;
};
