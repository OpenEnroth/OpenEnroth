#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <type_traits>

#include "Utility/Flags.h"
#include "Utility/String/TransparentFunctors.h"

enum class CaseSensitivity {
    CASE_SENSITIVE,
    CASE_INSENSITIVE
};
using enum CaseSensitivity;

namespace detail {
class EnumSerializationTable {
 public:
    template<class T>
    EnumSerializationTable(CaseSensitivity caseSensitivity, std::initializer_list<std::pair<T, const char *>> pairs) : _caseSensitivity(caseSensitivity) {
        for (const auto &[value, name] : pairs)
            insert(static_cast<uint64_t>(value), name);
        polish();
    }

    bool trySerialize(uint64_t src, std::string *dst) const;
    bool tryDeserialize(std::string_view src, uint64_t *dst) const;

    bool isUsableWithFlags() const;
    bool trySerializeFlags(uint64_t src, std::string *dst) const;
    bool tryDeserializeFlags(std::string_view src, uint64_t *dst) const;

 private:
    void insert(uint64_t value, std::string_view name);
    void polish();

 private:
    CaseSensitivity _caseSensitivity;
    std::unordered_map<uint64_t, std::string> _stringByEnum;
    std::unordered_map<std::string, uint64_t, TransparentStringHash, TransparentStringEquals> _enumByString;
    std::vector<std::pair<uint64_t, std::string>> _sortedEnumStrings;
};

template<class T>
class EnumSerializer {
 public:
    EnumSerializer() {}

    EnumSerializer(CaseSensitivity caseSensitivity, std::initializer_list<std::pair<T, const char *>> pairs) : _table(caseSensitivity, pairs) {}

    bool trySerialize(T src, std::string *dst) const {
        return _table.trySerialize(static_cast<uint64_t>(src), dst);
    }

    bool tryDeserialize(std::string_view src, T *dst) const {
        uint64_t tmp;
        if (!_table.tryDeserialize(src, &tmp))
            return false;
        *dst = static_cast<T>(tmp);
        return true;
    }

    bool trySerialize(Flags<T> src, std::string *dst) const requires std::is_enum_v<T> {
        return _table.trySerializeFlags(static_cast<uint64_t>(std::to_underlying(src)), dst);
    }

    bool tryDeserialize(std::string_view src, Flags<T> *dst) const requires std::is_enum_v<T> {
        uint64_t tmp;
        if (!_table.tryDeserializeFlags(src, &tmp))
            return false;
        *dst = static_cast<Flags<T>>(tmp);
        return true;
    }

    bool isUsableWithFlags() const {
        return _table.isUsableWithFlags();
    }

 private:
    EnumSerializationTable _table;
};
} // namespace detail
