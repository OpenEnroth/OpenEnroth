#include "AnyConfigEntry.h"

#include <cassert>
#include <utility>

#include "ConfigSection.h"
#include "AnySerializer.h"

AnyConfigEntry::AnyConfigEntry(ConfigSection *section, const std::string &name, std::any defaultValue, Validator validator,
                               AnySerializer *serializer, const std::string &description) {
    assert(section);
    assert(serializer);
    assert(!name.empty());
    assert(defaultValue.type() == serializer->type());

    _section = section;
    _name = name;
    _description = description;
    _validator = std::move(validator);
    _serializer = serializer;
    _value = _defaultValue = std::move(defaultValue);

    section->registerEntry(this);
}

void AnyConfigEntry::setValue(std::any value) {
    assert(value.type() == type());

    if (_validator) {
        _value = _validator(std::move(value));
    } else {
        _value = value;
    }
}

std::string AnyConfigEntry::defaultString() const {
    return _serializer->serialize(_defaultValue);
}

std::string AnyConfigEntry::string() const {
    return _serializer->serialize(_value);
}

void AnyConfigEntry::setString(const std::string &value) {
    setValue(_serializer->deserialize(value));
}
