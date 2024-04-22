#pragma once

#include <unordered_map>
#include <functional>
#include <string_view>
#include <string>
#include <vector>
#include <sol/sol.hpp>

typedef std::vector<std::string_view> QueryTable;

// A helper class used to fill a lua table with all the requested information
template<typename ItemType>
class LuaItemQueryTable {
 public:
    explicit LuaItemQueryTable(sol::state_view &luaState) : _luaState(luaState) {}

    template<typename T>
    void add(const std::string_view &key, T &&query) {
        _mapping.insert(std::make_pair(
            key, [this, query](auto &item) { return sol::make_object(_luaState, query(item)); }
        ));
    }

    /** Generate a Lua table by extracting information from the provided item.
        Treat the query table as a vector, with each item being
        a string representing the property to extract. */
    sol::object createTable(const ItemType &item, const QueryTable &queryTable) {
        sol::table table = _luaState.create_table();
        if (queryTable.empty()) {
            for (auto &&[key, query] : _mapping) {
                table[key] = query(item);
            }
        } else {
            for (auto &&key : queryTable) {
                if (auto itr = _mapping.find(key.data()); itr != _mapping.end()) {
                    table[key] = itr->second(item);
                }
            }
        }
        return table;
    }

 private:
    typedef std::unordered_map<std::string, std::function<sol::object(const ItemType &)>> MapFunctions;

    MapFunctions _mapping;
    sol::state_view _luaState;
};
