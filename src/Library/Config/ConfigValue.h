#pragma once

#include <string>

#include "ConfigFwd.h"
#include "AbstractConfigValue.h"
#include "ConfigSerialization.h"

template <class T>
class ConfigValue : public AbstractConfigValue {
 public:
    using validator_type = T (*)(T);

    ConfigValue(const ConfigValue &other) = delete; // non-copyable
    ConfigValue(ConfigValue&& other) = delete; // non-movable

    ConfigValue(ConfigSection *section, const std::string &name, T defaultValue, validator_type v = nullptr) :
        AbstractConfigValue(section, name), defaultValue_(defaultValue), value_(defaultValue), validator_(v) {}

    const T &Default() const {
        return defaultValue_;
    }

    const T &Get() const {
        return value_;
    }

    T Set(const T &val) {
        if (validator_)
            value_ = validator_(val);
        else
            value_ = val;

        return value_;
    }

    virtual std::string GetString() const override {
        std::string result;
        SerializeConfigValue(value_, &result);
        return result;
    }

    virtual std::string DefaultString() const override {
        std::string result;
        SerializeConfigValue(defaultValue_, &result);
        return result;
    }

    virtual void SetString(const std::string &value) override {
        DeserializeConfigValue(value, &value_);
    }

    virtual void Reset() override {
        value_ = defaultValue_;
    }

    T Toggle() requires std::is_same_v<T, bool> {
        value_ = !value_;

        return value_;
    }

    T Increment() requires std::is_same_v<T, int> {
        if (validator_)
            value_ = validator_(value_ + 1);
        else
            value_++;

        return value_;
    }

    void Decrement() requires std::is_same_v<T, int> {
        if (validator_)
            value_ = validator_(value_ - 1);
        else
            value_--;

        return value_;
    }

 private:
    T value_;
    T defaultValue_;
    validator_type validator_;
};
