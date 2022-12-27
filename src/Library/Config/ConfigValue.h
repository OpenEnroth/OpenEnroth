#pragma once

#include <string>

#include "ConfigFwd.h"

template <class T>
class ConfigValue {
 public:
    using validator_type = T (*)(T);

    ConfigValue(const ConfigValue &other) = delete; // non-copyable
    ConfigValue(ConfigValue&& other) = delete; // non-movable

    ConfigValue(ConfigSection *section, const std::string &name, T defaultValue, validator_type v = nullptr) :
        section(section), name(name), defaultValue(defaultValue), value(defaultValue), validator(v) {
        RegisterConfigValue(this);
    }

    ConfigSection *Parent() const {
        return section;
    }

    const std::string &Name() const {
        return name;
    }

    const T &Default() const {
        return defaultValue;
    }

    const T &Get() const {
        return value;
    }

    T Set(const T &val) {
        if (validator)
            value = validator(val);
        else
            value = val;

        return value;
    }

    void Reset() {
        value = defaultValue;
    }

    T Toggle() requires std::is_same_v<T, bool> {
        value = !value;

        return value;
    }

    T Increment() requires std::is_same_v<T, int> {
        if (validator)
            value = validator(value + 1);
        else
            value++;

        return value;
    }

    void Decrement() requires std::is_same_v<T, int> {
        if (validator)
            value = validator(value - 1);
        else
            value--;

        return value;
    }

 private:
    ConfigSection *section = nullptr;
    std::string name;
    T value;
    T defaultValue;
    validator_type validator;
};
