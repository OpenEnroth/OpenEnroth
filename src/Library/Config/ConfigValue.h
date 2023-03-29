#pragma once

#include <climits>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "ConfigFwd.h"
#include "AbstractConfigValue.h"

template <class T>
class ConfigValue : public AbstractConfigValue {
 public:
    using validator_type = T (*)(T);

    ConfigValue(const ConfigValue &other) = delete; // non-copyable
    ConfigValue(ConfigValue&& other) = delete; // non-movable

    ConfigValue(ConfigSection *section, const std::string &name, T defaultValue, validator_type validator, const std::string &description) :
        AbstractConfigValue(section, name, description), _defaultValue(defaultValue), _value(defaultValue), _validator(validator) {}

    ConfigValue(ConfigSection *section, const std::string &name, T defaultValue, const std::string &description) :
        ConfigValue(section, name, defaultValue, nullptr, description) {}

    const T &defaultValue() const {
        return _defaultValue;
    }

    const T &value() const {
        return _value;
    }

    T setValue(const T &val) {
        if (_validator)
            _value = _validator(val);
        else
            _value = val;

        return _value;
    }

    virtual std::string toString() const override {
        return ::toString(_value);
    }

    virtual std::string defaultString() const override {
        return ::toString(_defaultValue);
    }

    virtual void setString(const std::string &value) override {
        _value = fromString<T>(value);
    }

    virtual void reset() override {
        _value = _defaultValue;
    }

    T toggle() requires std::is_same_v<T, bool> {
        _value = !_value;

        return _value;
    }

    T increment() requires std::is_same_v<T, int> {
        if (_validator)
            _value = _validator(_value + 1);
        else
            _value++;

        return _value;
    }

    T decrement() requires std::is_same_v<T, int> {
        if (_validator)
            _value = _validator(_value - 1);
        else
            _value--;

        return _value;
    }

    T cycleIncrement() requires std::is_same_v<T, int> {
        // we rely on validator with std::clamp-like behaviour.
        assert(_validator);

        int old = _value;
        _value = _validator(_value + 1);
        if (_value == old)
            _value = _validator(INT_MIN);

        return _value;
    }

    T cycleDecrement() requires std::is_same_v<T, int> {
        // we rely on validator with std::clamp-like behaviour.
        assert(_validator);

        int old = _value;
        _value = _validator(_value - 1);
        if (_value == old)
            _value = _validator(INT_MAX);

        return _value;
    }

 private:
    T _value;
    T _defaultValue;
    validator_type _validator;
};
