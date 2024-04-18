#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <sol/sol.hpp>

struct lua_State;

class Character;

class GameLuaBindings {
 public:
    GameLuaBindings();
    ~GameLuaBindings();
    void init(lua_State *lua);

 private:
    void _registerAudioBindings(sol::state_view &luaState, sol::table &table);
    void _registerRenderBindings(sol::state_view &luaState, sol::table &table);
    void _registerGameBindings(sol::state_view &luaState, sol::table &table);
    void _registerPartyBindings(sol::state_view &luaState, sol::table &table);
    void _registerItemBindings(sol::state_view &luaState, sol::table &table);
    void _registerSerializationBindings(sol::state_view &luaState, sol::table &table);
    void _registerEnums(sol::state_view &luaState, sol::table &table);

    std::unique_ptr<sol::state_view> _luaState;

    // A helper class used to fill a lua table with all the requested information
    template<typename ItemType>
    class ItemQueryTable {
     public:
        explicit ItemQueryTable(sol::state_view &luaState) : _luaState(luaState) {}

        template<typename T>
        void add(const std::string_view &key, T&& query) {
            _mapping.insert(std::make_pair(
                key, [this, query](auto &item) { return sol::make_object(_luaState, query(item)); }
            ));
        }

        /** Create a lua table by extracting the information from the provided item. */
        sol::object createTable(const ItemType &item, const sol::object &queryTable) {
            sol::table table = _luaState.create_table();
            if (!queryTable.is<sol::table>()) {
                for (auto &&pair : _mapping) {
                    table[pair.first] = pair.second(item);
                }
            } else {
                for (auto &&pair : queryTable.as<sol::table>()) {
                    auto key = pair.second.as<std::string_view>();
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
        sol::state_view &_luaState;
    };

    std::unique_ptr<ItemQueryTable<Character>> _characterInfoQueryTable;
};
