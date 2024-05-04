#include "EnumSerializer.h"

#include <cassert>
#include <algorithm>
#include <string>

#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

bool detail::EnumSerializationTable::trySerialize(uint64_t src, std::string *dst) const {
    auto pos = _stringByEnum.find(src);
    if (pos == _stringByEnum.end())
        return false;
    *dst = std::string(pos->second);
    return true;
}

bool detail::EnumSerializationTable::tryDeserialize(std::string_view src, uint64_t *dst) const {
    auto DoTryDeserialize = [this] (std::string_view src, uint64_t *dst) {
        auto pos = _enumByString.find(src);
        if (pos == _enumByString.end())
            return false;
        *dst = pos->second;
        return true;
    };

    if (_caseSensitivity == CASE_INSENSITIVE) {
        return DoTryDeserialize(ascii::toLower(src), dst);
    } else {
        return DoTryDeserialize(src, dst);
    }
}

bool detail::EnumSerializationTable::isUsableWithFlags() const {
    auto pos = _enumByString.find("0");
    if (pos != _enumByString.end() && pos->second != 0)
        return false;

    for (const auto &[_, string] : _sortedEnumStrings)
        if (string.find_first_of("| ") != std::string::npos)
            return false;

    return true;
}

bool detail::EnumSerializationTable::trySerializeFlags(uint64_t src, std::string *dst) const {
    // First check if it's a single value.
    auto pos = _stringByEnum.find(src);
    if (pos != _stringByEnum.end()) {
        *dst = pos->second;
        return true;
    }

    // Also check for zero. Note that if enum already has a named zero value, then it would've been handled by the
    // check above.
    if (src == 0) {
        *dst = "0";
        return true;
    }

    // Then just go bit by bit.
    dst->clear();
    uint64_t accumulated = 0;
    for (const auto &[bit, string] : _sortedEnumStrings) {
        if ((src & bit) != bit || (accumulated & bit) == bit)
            continue; // Not in src, or already accumulated.

        accumulated |= bit;
        if (!dst->empty())
            *dst += '|';
        dst->append(string);

        if (accumulated == src)
            break; // Exit early if we can.
    }

    return accumulated == src;
}

bool detail::EnumSerializationTable::tryDeserializeFlags(std::string_view src, uint64_t *dst) const {
    uint64_t result = 0;

    size_t pos = 0;
    do {
        size_t end = src.find('|', pos);
        if (end == std::string_view::npos)
            end = src.size();

        std::string_view string = trim(src.substr(pos, end - pos));

        uint64_t bit;
        if (string == "0") {
            bit = 0;
        } else if (!tryDeserialize(string, &bit)) {
            return false;
        }

        result |= bit;
        pos = end + 1;
    } while (pos < src.size());

    *dst = result;
    return true;
}

void detail::EnumSerializationTable::insert(uint64_t value, std::string_view name) {
    if (!_stringByEnum.contains(value)) {
        _sortedEnumStrings.emplace_back(value, std::string(name));
        _stringByEnum.emplace(_sortedEnumStrings.back());
    }

    std::string storedName;
    if (_caseSensitivity == CASE_INSENSITIVE) {
        storedName = ascii::toLower(name);
    } else {
        storedName = name;
    }

    assert(!_enumByString.contains(storedName));
    _enumByString.emplace(storedName, value);
}

void detail::EnumSerializationTable::polish() {
    std::sort(_sortedEnumStrings.begin(), _sortedEnumStrings.end());
}
