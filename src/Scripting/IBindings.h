#pragma once

class IBindings {
 public:
    virtual ~IBindings() = default;
    virtual void init() = 0;
};