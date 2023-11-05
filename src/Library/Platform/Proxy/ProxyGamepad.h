#pragma once

#include <string>

#include "Library/Platform/Interface/PlatformGamepad.h"

#include "ProxyBase.h"

class ProxyGamepad: public ProxyBase<PlatformGamepad> {
 public:
    explicit ProxyGamepad(PlatformGamepad *base = nullptr);
    virtual ~ProxyGamepad() = default;

    virtual std::string model() const override;
    virtual std::string serial() const override;
};
