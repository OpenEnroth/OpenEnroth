#pragma once

#include <string>
#include <map>
#include <vector>

#include "ConfigFwd.h"

class ConfigSection {
 public:
    ConfigSection(::Config *config, const std::string &name); // Defined in Config.cpp

    ConfigSection(const ConfigSection &other) = delete; // non-copyable
    ConfigSection(ConfigSection&& other) = delete; // non-movable

    ::Config *Config() const {
        return config_;
    }

    const std::string &Name() const {
        return name_;
    }

    void RegisterValue(AbstractConfigValue *value);

    AbstractConfigValue *Value(const std::string &name) const;

    std::vector<AbstractConfigValue *> Values() const;

 private:
    ::Config *config_ = nullptr;
    std::string name_;
    std::map<std::string, AbstractConfigValue *> valueByName_;
};
