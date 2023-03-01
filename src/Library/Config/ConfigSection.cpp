#include "ConfigSection.h"

#include <cassert>

#include "Utility/MapAccess.h"

#include "Config.h"

ConfigSection::ConfigSection(::Config *config, const std::string &name): config_(config), name_(name) {
    assert(config);
    assert(!name.empty());

    config->RegisterSection(this);
}

void ConfigSection::RegisterValue(AbstractConfigValue *value) {
    assert(value);
    assert(!valueByName_.contains(value->Name()));

    valueByName_.emplace(value->Name(), value);
}

AbstractConfigValue *ConfigSection::Value(const std::string &name) const {
    return valueOr(valueByName_, name, nullptr);
}

std::vector<AbstractConfigValue *> ConfigSection::Values() const {
    std::vector<AbstractConfigValue *> result;
    for(const auto &[_, value] : valueByName_)
        result.push_back(value);
    return result;
}
