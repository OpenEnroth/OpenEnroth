#pragma once

#include "IBindings.h"

#include <string>

class AnyConfigEntry;

class ConfigBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    [[nodiscard]] static void setConfigValue1(std::string_view sectionName, std::string_view entryName, std::string_view value);
    [[nodiscard]] static void setConfigValue2(std::string_view entryName, std::string_view value);
    [[nodiscard]] static std::string configValue1(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static std::string configValue2(std::string_view entryName);

    [[nodiscard]] static AnyConfigEntry *locateEntry1(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static AnyConfigEntry *locateEntry2(std::string_view entryName);
};
