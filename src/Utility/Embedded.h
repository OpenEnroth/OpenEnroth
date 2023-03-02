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
    explicit Embedded(Args &&...args): _data(std::forward<Args>(args)...) {}

    T &get() {
        return _data;
    }

    const T &get() const {
        return _data;
    }

 private:
    T _data;
};
