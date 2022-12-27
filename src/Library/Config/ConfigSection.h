#pragma once

#include <string>

#include "ConfigFwd.h"

class ConfigSection {
 public:
    ConfigSection(Config *config, const std::string &sectionName): config(config), sectionName(sectionName) {}

    ConfigSection(const ConfigSection &other) = delete; // non-copyable
    ConfigSection(ConfigSection&& other) = delete; // non-movable

    Config *Parent() const {
        return config;
    }

    const std::string &Name() const {
        return sectionName;
    }

 private:
    Config *config = nullptr;
    std::string sectionName;
};
