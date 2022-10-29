#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>

template<class Map, class Key, class Value = typename Map::mapped_type>
Value ValueOr(const Map &map, const Key &key, const std::type_identity_t<Value> &def = Value()) {
    auto pos = map.find(key);
    return pos == map.end() ? def : pos->second;
}

template<class Map, class Key, class Value = typename Map::mapped_type>
Value ExistingValue(const Map &map, const Key &key) {
    auto pos = map.find(key);
    assert(pos != map.end());
    return pos->second;
}

template<class Key, class Value>
std::unordered_map<Value, Key> Inverted(const std::unordered_map<Key, Value> &map) {
    std::unordered_map<Value, Key> result;

    for (const auto &pair : map) {
        assert(!result.contains(pair.second));
        result[pair.second] = pair.first;
    }

    return result;
}
