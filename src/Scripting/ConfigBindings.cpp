#include "ConfigBindings.h"

#include <Library/Logger/Logger.h>
#include <Engine/Engine.h>

#include <functional>
#include <string>

ConfigBindings::ConfigBindings(const sol::state_view &solState) : _solState(solState) {
}

void ConfigBindings::init() {
    _solState.set_function("initConfig", [this]() {
        sol::table mainTable = _solState.create_table_with(
            "setConfig", sol::overload(
                [this](std::string_view section, std::string_view config, std::string_view value) { return setConfigValue(section, config, value); },
                [this](std::string_view config, std::string_view value){ return setConfigValue(config, value); }
            ),
            "getConfig", sol::overload(
                [this](std::string_view section, std::string_view config) { return getConfigValue(section, config); },
                [this](std::string_view config) { return getConfigValue(config); }
            )
        );
        return mainTable;
    });
}

bool ConfigBindings::setConfigValue(std::string_view sectionName, std::string_view configName, std::string_view value) const {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section) {
        logger->warning("Can't find section: {}", sectionName);
        return false;
    }

    AnyConfigEntry *configEntry = section->entry(configName);
    if (!configEntry) {
        logger->warning("Can't find config: {}", configName);
        return false;
    }

    configEntry->setString(value);
    return true;
}

bool ConfigBindings::setConfigValue(std::string_view configName, std::string_view value) const {
    for (auto &&section : engine->config->sections()) {
        AnyConfigEntry *configEntry = section->entry(configName);
        if (configEntry != nullptr) {
            configEntry->setString(value);
            return true;
        }
    }

    logger->warning("Can't find config: {}", configName);
    return false;
}

std::optional<std::string_view> ConfigBindings::getConfigValue(std::string_view sectionName, std::string_view configName) const {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section) {
        logger->warning("Can't find section: {}", sectionName);
        return std::nullopt;
    }

    AnyConfigEntry *configEntry = section->entry(configName);
    if (!configEntry) {
        logger->warning("Can't find config: {}", configName);
        return std::nullopt;
    }

    return configEntry->string();
}

std::optional<std::string_view> ConfigBindings::getConfigValue(std::string_view configName) const {
    for (auto &&section : engine->config->sections()) {
        AnyConfigEntry *configEntry = section->entry(configName);
        if (configEntry != nullptr) {
            return configEntry->string();
        }
    }

    logger->warning("Can't find config: {}", configName);
    return std::nullopt;
}
