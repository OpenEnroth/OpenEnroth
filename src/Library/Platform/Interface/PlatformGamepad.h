#pragma once

#include <string>
#include <memory>

class PlatformGamepad {
 public:
    virtual ~PlatformGamepad() = default;

    // TODO(captainurist): add rumble methods here!

    /**
     * @return                          Model of this gamepad.
     */
    virtual std::string model() const = 0;

    /**
     * @return                          Serial number of this gamepad.
     */
    virtual std::string serial() const = 0;
};
