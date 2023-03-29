#include "Config.h"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <cassert>
#include <filesystem>
#include <string>

#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

void Config::load(const std::string &path) {
    if (!std::filesystem::exists(path))
        throw Exception("config file '{}' doesn't exist", path);

    mINI::INIFile file(path);
    mINI::INIStructure ini;
    if (!file.read(ini))
        throw Exception("Couldn't read config file '{}'", path);

    for (const auto &[sectionName, iniSection] : ini)
        if (ConfigSection *section = this->section(sectionName))
            for (const auto &[valueName, valueString] : iniSection)
                if (AbstractConfigValue *value = section->value(valueName))
                    value->setString(valueString);
}

void Config::save(const std::string &path) const {
    mINI::INIFile file(path);
    mINI::INIStructure ini;

    for (ConfigSection *section : sections())
        for (AbstractConfigValue *value : section->values())
            ini[section->name()][value->name()] = value->toString();

    if (!file.write(ini, true))
        throw Exception("Couldn't save config file '{}'", path);
}

void Config::reset() {
    for (ConfigSection *section : sections())
        for (AbstractConfigValue *value : section->values())
            value->reset();
}

void Config::registerSection(ConfigSection *section) {
    assert(section);
    assert(!_sectionByName.contains(section->name()));

    _sectionByName.emplace(section->name(), section);
}

ConfigSection *Config::section(const std::string &name) const {
    return valueOr(_sectionByName, name, nullptr);
}

std::vector<ConfigSection *> Config::sections() const {
    std::vector<ConfigSection *> result;
    for (const auto &[_, section] : _sectionByName)
        result.push_back(section);
    return result;
}

AbstractConfigValue::AbstractConfigValue(ConfigSection *section, const std::string &name, const std::string &description):
    _section(section),
    _name(name),
    _description(description) {
    assert(section);
    assert(!name.empty());

    section->registerValue(this);
}
