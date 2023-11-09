#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>

/**
 * Convenient map accessor that returns either the value stored in a map, or default value if the key doesn't exist.
 *
 * @param map                           Map to get value from.
 * @param key                           Key to look up value for.
 * @param def                           Default value to return if the key is not in the map.
 * @return                              Value for the provided key, or `def` if the key was not found in the map.
 */
template<class Map, class Key, class Value = typename Map::mapped_type>
Value valueOr(const Map &map, const Key &key, const std::type_identity_t<Value> &def = Value()) {
    auto pos = map.find(key);
    return pos == map.end() ? def : pos->second;
}

/**
 * Convenient map accessor that returns either the pointer to the value stored in a map, or `nullptr` if the key
 * doesn't exist.
 *
 * @param map                           Map to get value from.
 * @param key                           Key to look up value for.
 * @return                              Pointer to the value for the provided key, or `nullptr` if the key was not
 *                                      found in the map.
 */
template<class Map, class Key>
auto *valuePtr(Map &&map, const Key &key) {
    auto pos = map.find(key);
    return pos == map.end() ? nullptr : &pos->second;
}

/**
 * Inverts an unordered map, creating a map from values into keys.
 *
 * The values must be unique, otherwise the function will assert.
 *
 * @param map                           Map to invert.
 * @return                              Inverted map.
 */
template<class Key, class Value>
std::unordered_map<Value, Key> inverted(const std::unordered_map<Key, Value> &map) {
    std::unordered_map<Value, Key> result;

    for (const auto &pair : map) {
        assert(!result.contains(pair.second));
        result[pair.second] = pair.first;
    }

    return result;
}
