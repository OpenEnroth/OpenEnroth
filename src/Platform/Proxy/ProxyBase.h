#pragma once

#include <cassert>

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
