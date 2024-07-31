#pragma once

#include <string>
#include <map>
#include <vector>

#include "Utility/String/TransparentFunctors.h"

#include "ConfigFwd.h"

class ConfigSection {
 public:
    ConfigSection(Config *config, std::string_view name); // Defined in Config.cpp

    ConfigSection(const ConfigSection &other) = delete; // non-copyable
    ConfigSection(ConfigSection &&other) = delete; // non-movable

    Config *config() const {
        return _config;
    }

    const std::string &name() const {
        return _name;
    }

    void registerEntry(AnyConfigEntry *entry);

    AnyConfigEntry *entry(std::string_view name) const;

    std::vector<AnyConfigEntry *> entries() const;

 private:
    Config *_config = nullptr;
    std::string _name;
    std::map<std::string, AnyConfigEntry *, TransparentStringLess> _entryByName;
};
