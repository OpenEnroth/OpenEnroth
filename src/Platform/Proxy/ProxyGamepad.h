#pragma once

#include <string>

#include "Platform/PlatformGamepad.h"

#include "ProxyBase.h"

class ProxyGamepad: public ProxyBase<PlatformGamepad> {
 public:
    explicit ProxyGamepad(PlatformGamepad *base = nullptr);
    virtual ~ProxyGamepad() = default;

    virtual std::string Model() const override;
    virtual std::string Serial() const override;
    virtual uint32_t Id() const override;
};
