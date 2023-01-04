#pragma once

#include <string>

#include "ConfigFwd.h"

class AbstractConfigValue {
 public:
    AbstractConfigValue(ConfigSection *section, const std::string &name, const std::string &description); // Defined in Config.cpp
    virtual ~AbstractConfigValue() = default;

    virtual std::string GetString() const = 0;
    virtual std::string DefaultString() const = 0;
    virtual void SetString(const std::string &value) = 0;
    virtual void Reset() = 0;

    ConfigSection *Section() const {
        return section_;
    }

    const std::string &Name() const {
        return name_;
    }

    const std::string &Description() const {
        return description_;
    }

 private:
    ConfigSection *section_ = nullptr;
    std::string name_;
    std::string description_;
};
