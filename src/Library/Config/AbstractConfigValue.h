#pragma once

#include <string>

#include "ConfigFwd.h"

class AbstractConfigValue {
 public:
    AbstractConfigValue(ConfigSection *section, const std::string &name); // Defined in Config.cpp
    virtual ~AbstractConfigValue() = default;

    virtual std::string GetString() const = 0;
    virtual std::string DefaultString() const = 0;
    virtual void SetString(const std::string &value) = 0;
    virtual void Reset() = 0;

    ConfigSection *Section() const {
        return section;
    }

    const std::string &Name() const {
        return name;
    }

 private:
    ConfigSection *section = nullptr;
    std::string name;
};
