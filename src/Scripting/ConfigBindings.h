#pragma once

#include "IBindings.h"

#include <string>

class ConfigBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) override;

 private:
    [[nodiscard]] static bool setConfigValue1(std::string_view sectionName, std::string_view configName, std::string_view value);
    [[nodiscard]] static bool setConfigValue2(std::string_view configName, std::string_view value);
    [[nodiscard]] static std::optional<std::string> getConfigValue1(std::string_view sectionName, std::string_view configName);
    [[nodiscard]] static std::optional<std::string> getConfigValue2(std::string_view configName);
};
