#pragma once

#include <map>
#include <string>
#include <vector>

#include "Utility/String/TransparentFunctors.h"

#include "ConfigFwd.h"
#include "ConfigSection.h"
#include "ConfigEntry.h"

class OutputStream;
class InputStream;

class Config {
 public:
    Config() = default;
    Config(const Config &other) = delete; // non-copyable
    Config(Config &&other) = delete; // non-movable

    void load(std::string_view path);
    void save(std::string_view path) const;
    void load(InputStream *stream);
    void save(OutputStream *stream) const;

    void reset();

    void registerSection(ConfigSection *section);

    ConfigSection *section(std::string_view name) const;

    std::vector<ConfigSection *> sections() const;

 private:
    std::map<std::string, ConfigSection *, TransparentStringLess> _sectionByName;
};
