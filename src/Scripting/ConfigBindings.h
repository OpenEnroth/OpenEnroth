#pragma once

#include "IBindings.h"

#include <string>

class ConfigBindings : public IBindings {
 public:
    explicit ConfigBindings(const sol::state_view &solState);
    virtual sol::table getBindingTable() override;

 private:
    [[nodiscard]] bool setConfigValue(std::string_view sectionName, std::string_view configName, std::string_view value) const;
    [[nodiscard]] bool setConfigValue(std::string_view configName, std::string_view value) const;
    [[nodiscard]] std::optional<std::string> getConfigValue(std::string_view sectionName, std::string_view configName) const;
    [[nodiscard]] std::optional<std::string> getConfigValue(std::string_view configName) const;

    sol::state_view _solState;
    std::optional<sol::table> _bindingTable;
};
