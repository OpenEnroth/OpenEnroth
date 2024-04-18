local mm = require "mmbindings"
local command_utils = require "dev.commands.command_utils"
local utils = require "utils"

local function show_party_conditions()
    local count = mm.party.get_party_size()
    local message = "Party Conditions\n"
    for i = 1, count do
        local info = mm.party.get_character_info(i, { "condition", "name" })
        local condition_str = ""
        for key, value in pairs(info.condition) do
            condition_str = condition_str..utils.enum_to_string(mm.CharacterCondition, key).." "
        end

        if utils.isempty(condition_str) then
            condition_str = "None"
        end

        message = message..info.name..": "..condition_str.."\n"
    end
    return message, true
end

local function set_condition(condition_name, character_index)
    character_index = command_utils.character_or_current(character_index)

    mm.party.set_character_info(character_index, { condition = mm.CharacterCondition[condition_name] })
    local info = mm.party.get_character_info(character_index, { "name" })

    local message = "Set condition "..condition_name.." to "..info.name
    return message, true
end

local function clear_condition(condition_name, character_index)
    if condition_name == nil then
        local count = mm.party.get_party_size()
        for i = 1, count do
            mm.party.clear_condition(i)
        end
        return "All conditions cleared", true
    else
        character_index = command_utils.character_or_current(character_index)
        mm.party.clear_condition(character_index, mm.CharacterCondition[condition_name])
        return "Condition "..condition_name.." cleared", true
    end
    
end

local subcommands = {
    get = show_party_conditions,
    set = set_condition,
    clear = clear_condition,
    default = show_party_conditions
}

return {
    name = "condition",
    description = "Change the conditions of the party members.",
    details = "",
    callback = subcommands
}
