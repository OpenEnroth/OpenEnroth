#pragma once

#include "IBindings.h"

#include <string_view>
#include <sol/sol.hpp>

class ConfigBindings : public IBindings {
 public:
    explicit ConfigBindings(const sol::state_view &solState);

    void init() override;

 private:
    [[nodiscard]] bool setConfigValue(std::string_view sectionName, std::string_view configName, std::string_view value) const;
    [[nodiscard]] bool setConfigValue(std::string_view configName, std::string_view value) const;
    [[nodiscard]] std::optional<std::string_view> getConfigValue(std::string_view sectionName, std::string_view configName) const;
    [[nodiscard]] std::optional<std::string_view> getConfigValue(std::string_view configName) const;

    sol::state_view _solState;
};
