#pragma once

#include "IBindings.h"

#include <string>

class ConfigBindings : public IBindings {
 public:
    explicit ConfigBindings(const sol::state_view &solState);
    virtual sol::table getBindingTable() override;

 private:
    [[nodiscard]] static bool setConfigValue1(std::string_view sectionName, std::string_view configName, std::string_view value);
    [[nodiscard]] static bool setConfigValue2(std::string_view configName, std::string_view value);
    [[nodiscard]] static std::optional<std::string> getConfigValue1(std::string_view sectionName, std::string_view configName);
    [[nodiscard]] static std::optional<std::string> getConfigValue2(std::string_view configName);

    sol::state_view _solState;
    std::optional<sol::table> _bindingTable;
};
