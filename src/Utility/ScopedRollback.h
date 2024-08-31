#pragma once

#include <utility>

template<class T>
class ScopedRollback {
 public:
    ScopedRollback(T *target, T value) : _target(target), _value(std::move(*target)) {
        *_target = std::move(value);
    }

    ~ScopedRollback() {
        *_target = std::move(_value);
    }

 private:
    T *_target = nullptr;
    T _value;
};
