#include "ConfigBindings.h"

#include <functional>
#include <string>

#include "Library/Logger/Logger.h"
#include "Engine/Engine.h"

sol::table ConfigBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "setConfig", sol::overload(setConfigValue1, setConfigValue2),
        "getConfig", sol::overload(getConfigValue1, getConfigValue2)
    );
}

bool ConfigBindings::setConfigValue1(std::string_view sectionName, std::string_view configName, std::string_view value) {
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

bool ConfigBindings::setConfigValue2(std::string_view configName, std::string_view value) {
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

std::optional<std::string> ConfigBindings::getConfigValue1(std::string_view sectionName, std::string_view configName) {
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

std::optional<std::string> ConfigBindings::getConfigValue2(std::string_view configName) {
    for (auto &&section : engine->config->sections()) {
        AnyConfigEntry *configEntry = section->entry(configName);
        if (configEntry != nullptr) {
            return configEntry->string();
        }
    }

    logger->warning("Can't find config: {}", configName);
    return std::nullopt;
}
