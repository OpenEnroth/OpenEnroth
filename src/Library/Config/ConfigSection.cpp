#include "ConfigSection.h"

#include <cassert>

#include "Utility/MapAccess.h"

#include "Config.h"

ConfigSection::ConfigSection(Config *config, const std::string &name): _config(config), _name(name) {
    assert(config);
    assert(!name.empty());

    config->registerSection(this);
}

void ConfigSection::registerValue(AbstractConfigValue *value) {
    assert(value);
    assert(!_valueByName.contains(value->name()));

    _valueByName.emplace(value->name(), value);
}

AbstractConfigValue *ConfigSection::value(const std::string &name) const {
    return valueOr(_valueByName, name, nullptr);
}

std::vector<AbstractConfigValue *> ConfigSection::values() const {
    std::vector<AbstractConfigValue *> result;
    for(const auto &[_, value] : _valueByName)
        result.push_back(value);
    return result;
}
