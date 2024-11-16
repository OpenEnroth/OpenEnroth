#pragma once

#include <cassert>
#include <climits>
#include <string>
#include <utility>
#include <concepts>

#include "Library/Serialization/Serialization.h"

#include "ConfigFwd.h"
#include "AnyConfigEntry.h"
#include "AnyHandler.h"

template <class T>
class ConfigEntry : public AnyConfigEntry {
    using base_type = AnyConfigEntry;
 public: // NOLINT: why are you complaining?
    ConfigEntry(const ConfigEntry &other) = delete; // non-copyable
    ConfigEntry(ConfigEntry &&other) = delete; // non-movable

    template<class TypedValidator>
    ConfigEntry(ConfigSection *section, std::string_view name, T defaultValue, TypedValidator validator, std::string_view description) :
        AnyConfigEntry(section, name, description, AnyHandler::forType<T>(), defaultValue, wrapValidator(std::move(validator))) {}

    ConfigEntry(ConfigSection *section, std::string_view name, T defaultValue, std::string_view description) :
        AnyConfigEntry(section, name, description, AnyHandler::forType<T>(), defaultValue, nullptr) {}

    const T &defaultValue() const {
        return std::any_cast<const T &>(AnyConfigEntry::defaultValue());
    }

    const T &value() const {
        return std::any_cast<const T &>(AnyConfigEntry::value());
    }

    void setValue(T value) {
        AnyConfigEntry::setValue(std::move(value));
    }

    void toggle() requires std::is_same_v<T, bool> {
        setValue(!value());
    }

    void increment() requires std::is_same_v<T, int> {
        setValue(value() + 1);
    }

    void decrement() requires std::is_same_v<T, int> {
        setValue(value() - 1);
    }

    void cycleIncrement() requires std::is_same_v<T, int> {
        assert(validator()); // We rely on validator with std::clamp-like behaviour.

        int oldValue = value();
        increment();
        if (value() == oldValue)
            setValue(INT_MIN);
    }

    void cycleDecrement() requires std::is_same_v<T, int> {
        assert(validator()); // We rely on validator with std::clamp-like behaviour.

        int oldValue = value();
        decrement();
        if (value() == oldValue)
            setValue(INT_MAX);
    }

    using base_type::addListener;

    template<std::invocable<T> Listener>
    void addListener(void *ctx, Listener listener) {
        addListener(ctx, [this, listener = std::move(listener)] {
            listener(value());
        });
    }

 private:
    template<class TypedValidator>
    static Validator wrapValidator(TypedValidator validator) {
        return [validator = std::move(validator)] (std::any value) {
            return std::any(std::in_place_type<T>, validator(std::any_cast<T &&>(std::move(value))));
        };
    }
};
