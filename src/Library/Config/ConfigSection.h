#pragma once

#include <string>
#include <map>
#include <vector>

#include "ConfigFwd.h"

class ConfigSection {
 public:
    ConfigSection(Config *config, const std::string &name); // Defined in Config.cpp

    ConfigSection(const ConfigSection &other) = delete; // non-copyable
    ConfigSection(ConfigSection&& other) = delete; // non-movable

    Config *config() const {
        return _config;
    }

    const std::string &name() const {
        return _name;
    }

    void registerValue(AbstractConfigValue *value);

    // TODO(captainurist): AbstractConfigEntry / entry() / entries()
    AbstractConfigValue *value(const std::string &name) const;

    std::vector<AbstractConfigValue *> values() const;

 private:
    Config *_config = nullptr;
    std::string _name;
    std::map<std::string, AbstractConfigValue *> _valueByName;
};
