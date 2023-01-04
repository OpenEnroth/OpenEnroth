#pragma once

#include <string>
#include <memory>

class PlatformGamepad {
 public:
    virtual ~PlatformGamepad() = default;

    virtual std::string Model() const = 0;
    virtual std::string Serial() const = 0;

    virtual uint32_t Id() const = 0;
};
