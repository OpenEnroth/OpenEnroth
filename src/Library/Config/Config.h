#pragma once

#include <map>
#include <string>
#include <vector>

#include "ConfigFwd.h"
#include "ConfigSection.h"
#include "ConfigValue.h"

class Config {
 public:
    Config() = default;
    Config(const Config &other) = delete; // non-copyable
    Config(Config&& other) = delete; // non-movable

    void Load(const std::string &path);
    void Save(const std::string &path) const;
    void Reset();

    void RegisterSection(ConfigSection *section);

    ConfigSection *Section(const std::string &name) const;

    std::vector<ConfigSection *> Sections() const;

 private:
    std::map<std::string, ConfigSection *> sectionByName_;
};
