#pragma once

#include <string>

#include "ConfigFwd.h"

class AbstractConfigValue {
 public:
    AbstractConfigValue(ConfigSection *section, const std::string &name, const std::string &description); // Defined in Config.cpp
    virtual ~AbstractConfigValue() = default;

    virtual std::string toString() const = 0;
    virtual std::string defaultString() const = 0;
    virtual void setString(const std::string &value) = 0;
    virtual void reset() = 0;

    ConfigSection *section() const {
        return _section;
    }

    const std::string &name() const {
        return _name;
    }

    const std::string &description() const {
        return _description;
    }

 private:
    ConfigSection *_section = nullptr;
    std::string _name;
    std::string _description;
};
