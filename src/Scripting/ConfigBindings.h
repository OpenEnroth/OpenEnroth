#pragma once

#include "IBindings.h"

#include <string>
#include <vector>

class AnyConfigEntry;

class ConfigBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    [[nodiscard]] static AnyConfigEntry *entry1(std::string_view entryName);
    [[nodiscard]] static AnyConfigEntry *entry2(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static std::vector<AnyConfigEntry *> list(std::string_view sectionName, std::string_view filter);

    [[nodiscard]] static std::string path(AnyConfigEntry *entry);
    [[nodiscard]] static std::string sectionName(AnyConfigEntry *entry);
    static void toggle(AnyConfigEntry *entry);
};
