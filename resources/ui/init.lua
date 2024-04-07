local commands = require "commands"
local logger = require "logger"

local config_commands
local gold_commands
local xp_commands
local sp_commands
local food_commands
local alignment_commands

function ui_init()
    commands.register("config", "Change any gameconfig value.", "Usage:\n\nconfig <action> [args]", config_commands)
    commands.register("gold", "Change the amount of gold.", "Usage:\n\ngold <action> [args]", gold_commands)
    commands.register("xp", "Change the amount of experience points for the party.", "Usage:\n\nxp <action> [args]", xp_commands)
    commands.register("sp", "Change the amount of skill points for the party.", "Usage:\n\nsp <action> [args]", sp_commands)
    commands.register("food", "Change the amount of food.", "Usage:\n\nfood <action> [args]", food_commands)
    commands.register("alignment", "Change the alignment of the party.", "Usage:\n\nalignment <action> [args]", alignment_commands)
end

-- UTILITIES

local change_char_property = function(key, action, play_award)
    return function(char_index, value)
        local get = game.get_character_info
        local set = game.set_character_info
        value = value ~= nil and value or 0
        local info = get(char_index)
        local newData = {}
        local message = ""
        if action == "set" then
            newData[key] = value
            set(char_index, newData)
            message = "Set "..value.." "..key.." for "..info.name
        elseif action == "rem" then
            newData[key] = info[key] - value
            set(char_index, newData)
            message = info.name.." lost "..value.." "..key..". Current "..key..": "..get(char_index)[key]
        elseif action == "add" then
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

local change_property = function(get, set, action, prop_name)
    return function(value)
        local message = ""
        if action == "set" then
            set(value)
            message = "Set "..value.." "..prop_name
        elseif action == "add" then
            local total = get() + value
            set(total)
            message = "Added "..value.." "..prop_name..". Current: "..get()
        elseif action == "rem" then
            local total = get() - value
            set(total)
            message = "Removed "..value.." "..prop_name..". Current: "..get()
        end
        return message, true
    end
end

local show_property = function(get, prop_name)
    return function()
        return "Current "..prop_name..": "..get(), true
    end
end

-- GOLD COMMANDS

gold_commands = {
    get = show_property(game.get_gold, "gold"),
    set = change_property(game.get_gold, game.set_gold, "set", "gold"),
    add = change_property(game.get_gold, game.set_gold, "add", "gold"),
    rem = change_property(game.get_gold, game.set_gold, "rem", "gold"),
    default = show_property(game.get_gold, "gold")
}

-- XP COMMANDS

local give_party_xp = function(xp)
    game.give_party_xp(xp)
    game.play_all_characters_award_sound()
    return "Party gained "..xp.." xp", true
end

xp_commands = {
    get = show_chars_property("xp"),
    rem = change_char_property("xp", "rem", true),
    add = change_char_property("xp", "add", true),
    set = change_char_property("xp", "set", true),
    party = give_party_xp,
    default = show_chars_property("xp")
}

-- SKILL POINTS COMMANDS

sp_commands = {
    get = show_chars_property("sp"),
    rem = change_char_property("sp", "rem", true),
    add = change_char_property("sp", "add", true),
    set = change_char_property("sp", "set", true),
    default = show_chars_property("sp")
}

-- FOOD COMMANDS

food_commands = {
    get = show_property(game.get_food, "food"),
    set = change_property(game.get_food, game.set_food, "set", "food"),
    add = change_property(game.get_food, game.set_food, "add", "food"),
    rem = change_property(game.get_food, game.set_food, "rem", "food"),
    default = show_property(game.get_food, "food")
}

-- ALIGNMENT COMMANDS

alignment_commands = {
    get = show_property(game.get_alignment, "alignment"),
    set = change_property(game.get_alignment, game.set_alignment, "set", "alignment"),
    default = show_property(game.get_alignment, "alignment")
}

-- CONFIG COMMANDS

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
