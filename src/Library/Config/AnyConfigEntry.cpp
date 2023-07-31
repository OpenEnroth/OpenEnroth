#include "AnyConfigEntry.h"

#include <cassert>
#include <utility>

#include "ConfigSection.h"
#include "AnyHandler.h"

AnyConfigEntry::AnyConfigEntry(ConfigSection *section, const std::string &name, const std::string &description, AnyHandler *handler,
                               std::any defaultValue, Validator validator) {
    assert(section);
    assert(handler);
    assert(!name.empty());
    assert(defaultValue.type() == handler->type());

    _section = section;
    _name = name;
    _description = description;
    _handler = handler;
    _value = _defaultValue = std::move(defaultValue);
    _validator = std::move(validator);

    section->registerEntry(this);
}

void AnyConfigEntry::setValue(std::any value) {
    assert(value.type() == type());

    if (_validator)
        value = _validator(std::move(value));

    if (_handler->equals(value, _value))
        return;

    _value = std::move(value);
}

std::string AnyConfigEntry::defaultString() const {
    return _handler->serialize(_defaultValue);
}

std::string AnyConfigEntry::string() const {
    return _handler->serialize(_value);
}

void AnyConfigEntry::setString(const std::string &value) {
    setValue(_handler->deserialize(value));
}
