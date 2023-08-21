#pragma once

#include <string>
#include <map>
#include <vector>

#include "ConfigFwd.h"

class AnyConfigEntry;
class Config;

class ConfigSection {
 public:
    ConfigSection(Config *config, const std::string &name); // Defined in Config.cpp

    ConfigSection(const ConfigSection &other) = delete; // non-copyable
    ConfigSection(ConfigSection &&other) = delete; // non-movable

    Config *config() const {
        return _config;
    }

    const std::string &name() const {
        return _name;
    }

    void registerEntry(AnyConfigEntry *entry);

    AnyConfigEntry *entry(const std::string &name) const;

    std::vector<AnyConfigEntry *> entries() const;

 private:
    Config *_config = nullptr;
    std::string _name;
    std::map<std::string, AnyConfigEntry *> _entryByName;
};
