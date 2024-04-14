local commands = require "commands"

local config_commands
local gold_commands
local xp_commands
local sp_commands
local food_commands
local alignment_commands

function ui_init()
    --- Register all the commands used in the console ( all these commands could probably be moved to a game_commands.lua script to clean up things )
    commands.register("config", "Change any gameconfig value.", "Usage:\n\nconfig <action> [args]", config_commands)
    commands.register("gold", "Change the amount of gold.", "Usage:\n\ngold <action> [args]", gold_commands)
    commands.register("xp", "Change the amount of experience points for the party.", "Usage:\n\nxp <action> [args]", xp_commands)
    commands.register("sp", "Change the amount of skill points for the party.", "Usage:\n\nsp <action> [args]", sp_commands)
    commands.register("food", "Change the amount of food.", "Usage:\n\nfood <action> [args]", food_commands)
    commands.register("alignment", "Change the alignment of the party.", "Usage:\n\nalignment <action> [args]", alignment_commands)
end

-- COMMAND UTILITIES
---@enum op_type
local OP_TYPE = {
	set = 1,
	rem = 2,
	add = 3
}

--- Utility factory-function that generate a new function used as command to update one property/stat for a character
--- Example that generate a command used to add experience points to a character:
---     change_char_property("xp", ACTION_TYPE.add, true)
---
---@param key string            - field referring to the character_info table ( "xp", "age", "sp", etc... )
---@param op op_type            - the operation to execute on the variable
---@param play_award boolean    - play the award effect after the operation has been executed
---@return function
local change_char_property = function(key, op, play_award, conversion)
    return function(char_index, value)
        if conversion then
            value = conversion(value)
        end

        local get = game.get_character_info
        local set = game.set_character_info
        value = value ~= nil and value or 0
        local info = get(char_index)
        local newData = {}
        local message = ""
        if op == OP_TYPE.set then
            newData[key] = value
            set(char_index, newData)
            message = "Set "..value.." "..key.." for "..info.name
        elseif op == OP_TYPE.rem then
            newData[key] = info[key] - value
            set(char_index, newData)
            message = info.name.." lost "..value.." "..key..". Current "..key..": "..get(char_index)[key]
        elseif op == OP_TYPE.add then
            newData[key] = info[key] + value
            set(char_index, newData)
            message = info.name.." gained "..value.." "..key..". Current "..key..": "..get(char_index)[key]
        end

        if play_award then
            game.play_character_award_sound(char_index)
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show one property/stat for each member of the party
--- Example that generate a command that shows the skils points of each character:
---     show_chars_property("sp")
---
---@param key string field referring to the character_info table
---@return function
local show_chars_property = function(key)
    return function()
        local count = game.get_party_size()
        local message = "Party "..key.."\n"
        for i = 1, count do
            local info = game.get_character_info(i)
            message = message..info.name..": "..info[key].."\n"
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to update a value retrieved by callbacks
--- Example of a command that remove golds from the party:
---     change_property(game.get_gold, game.set_gold, ACTION_TYPE.rem, "gold")
---
---@param get function      - getter function used to retrieve the current value
---@param set function      - setter function used to update the current value
---@param op op_type        - the operation to execute on the variable
---@param prop_name string  - name of the property. Used only for prompting, it's not used to retrieve the value
---@return function
local change_property = function(get, set, op, prop_name, conversion)
    return function(value)
        if conversion then
            value = conversion(value)
        end

        local message = ""
        if op == OP_TYPE.set then
            set(value)
            message = "Set "..value.." "..prop_name
        elseif op == OP_TYPE.add then
            local total = get() + value
            set(total)
            message = "Added "..value.." "..prop_name..". Current: "..get()
        elseif op == OP_TYPE.rem then
            local total = get() - value
            set(total)
            message = "Removed "..value.." "..prop_name..". Current: "..get()
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show a value
--- Example of a function generating a command showing the current alignment:
---     show_property(game.get_alignment, "alignment")
---@param get function      getter function used to retrieve the current value
---@param prop_name string  name of the property. Used only for prompting, it's not used to retrieve the value
---@return function
local show_property = function(get, prop_name)
    return function()
        return "Current "..prop_name..": "..get(), true
    end
end

-- GOLD COMMANDS
gold_commands = {
    get = show_property(game.get_gold, "gold"),
    set = change_property(game.get_gold, game.set_gold, OP_TYPE.set, "gold", tonumber),
    add = change_property(game.get_gold, game.set_gold, OP_TYPE.add, "gold", tonumber),
    rem = change_property(game.get_gold, game.set_gold, OP_TYPE.rem, "gold", tonumber),
    default = show_property(game.get_gold, "gold")
}

-- XP COMMANDS

---Function that gives to the party a "xp" amount of Experience points and play the award animation on all characters
---@param xp integer - number of experience points to give to the party
---@return string    - the message sent back to the console
---@return boolean   - flag that tells if the command has been successful
local give_party_xp = function(xp)
    game.give_party_xp(xp)
    game.play_all_characters_award_sound()
    return "Party gained "..xp.." xp", true
end

xp_commands = {
    get = show_chars_property("xp"),
    rem = change_char_property("xp", OP_TYPE.rem, true, tonumber),
    add = change_char_property("xp", OP_TYPE.add, true, tonumber),
    set = change_char_property("xp", OP_TYPE.set, true, tonumber),
    party = give_party_xp,
    default = show_chars_property("xp")
}

-- SKILL POINTS COMMANDS

sp_commands = {
    get = show_chars_property("sp"),
    rem = change_char_property("sp", OP_TYPE.rem, true, tonumber),
    add = change_char_property("sp", OP_TYPE.add, true, tonumber),
    set = change_char_property("sp", OP_TYPE.set, true, tonumber),
    default = show_chars_property("sp")
}

-- FOOD COMMANDS

food_commands = {
    get = show_property(game.get_food, "food"),
    set = change_property(game.get_food, game.set_food, OP_TYPE.set, "food", tonumber),
    add = change_property(game.get_food, game.set_food, OP_TYPE.add, "food", tonumber),
    rem = change_property(game.get_food, game.set_food, OP_TYPE.rem, "food", tonumber),
    default = show_property(game.get_food, "food")
}

-- ALIGNMENT COMMANDS

alignment_commands = {
    get = show_property(game.get_alignment, "alignment"),
    set = change_property(game.get_alignment, game.set_alignment, OP_TYPE.set, "alignment"),
    default = show_property(game.get_alignment, "alignment")
}

-- CONFIG COMMANDS

---Get the value of a configEntry
---@param config_name string    - config entry name
---@param section_name? string  - section of the config entry 
---@return string               - the message sent back to the console
---@return boolean              - flag that tells if the command has been successful
local function get_config(config_name, section_name)
    local value
    local message
    if section_name ~= nil then
        value = dev.config_get(section_name, config_name)
        message = "["..section_name.."] - "..config_name..": "
    else
        value = dev.config_get(config_name)
        message = config_name..": "
    end
    return message..tostring(value), true
end

---Change the value of the configEntry
---@param config_name string - name of the configEntry
---@param param2 string      - section of the config entry. If param3 is nil param2 represents the value instead 
---@param param3? string     - new value to apply
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function set_config(config_name, param2, param3)
    local value
    local message
    local section_name
    if param3 ~= nil then
        section_name = param2
        value = param3
    else
        value = param2
    end

    if section_name ~= nil then
        dev.config_set(section_name, config_name, value)
        value = dev.config_get(section_name, config_name)
        message = "["..section_name.."] - "..config_name..": "
    else
        dev.config_set(config_name, value)
        value = dev.config_get(config_name)
        message = config_name..": "
    end
    return message..tostring(value), true
end

config_commands = {
    get = get_config,
    set = set_config,
}
