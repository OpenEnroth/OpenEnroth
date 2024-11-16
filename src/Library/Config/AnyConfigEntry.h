#pragma once

#include <string>
#include <functional>
#include <any>
#include <vector>
#include <utility>

#include "ConfigFwd.h"

class AnyConfigEntry {
 public:
    using Validator = std::function<std::any(std::any)>;

    AnyConfigEntry(ConfigSection *section, std::string_view name, std::string_view description, AnyHandler *handler,
                   std::any defaultValue, Validator validator);
    virtual ~AnyConfigEntry() = default;

    const std::type_info &type() const {
        return _defaultValue.type();
    }

    const std::any &defaultValue() const {
        return _defaultValue;
    }

    const std::any &value() const {
        return _value;
    }

    void setValue(std::any value);

    void reset() {
        _value = _defaultValue;
    }

    std::string defaultString() const;

    std::string string() const;

    void setString(std::string_view value);

    ConfigSection *section() const {
        return _section;
    }

    const std::string &name() const {
        return _name;
    }

    const std::string &description() const {
        return _description;
    }

    void addListener(void *ctx, std::function<void()> listener) {
        _listeners.emplace_back(ctx, std::move(listener));
    }

    void removeListeners(void *ctx) {
        std::erase_if(_listeners, [ctx] (const auto &pair) { return pair.first == ctx;});
    }

 protected:
    Validator validator() const {
        return _validator;
    }

 private:
    ConfigSection *_section = nullptr;
    std::string _name;
    std::string _description;
    AnyHandler *_handler = nullptr;
    std::any _defaultValue;
    std::any _value;
    Validator _validator = nullptr;
    std::vector<std::pair<void *, std::function<void()>>> _listeners;
};
