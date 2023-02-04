#pragma once

#include <string>
#include <memory>

// TODO(captainurist): this should be just a POD
class PlatformGamepad {
 public:
    virtual ~PlatformGamepad() = default;

    virtual std::string model() const = 0;
    virtual std::string serial() const = 0;

    virtual uint32_t id() const = 0;
};
