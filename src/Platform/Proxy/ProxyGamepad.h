#pragma once

#include <string>
#include "Platform/PlatformGamepad.h"
#include "ProxyBase.h"

class PlatformGamepad;

class ProxyGamepad: public ProxyBase<PlatformGamepad> {
 public:
    explicit ProxyGamepad(PlatformGamepad *base = nullptr);
    virtual ~ProxyGamepad() = default;

    virtual std::string model() const override;
    virtual std::string serial() const override;
};
