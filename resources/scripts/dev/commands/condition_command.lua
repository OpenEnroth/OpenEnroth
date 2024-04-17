local mm = require "mmbindings"
require "utils"

local function condition_enum_to_string(value)
    for k, v in pairs(CharacterCondition) do
        if v == value then
            return k
        end
    end

    return ""
end

local function show_party_conditions()
    local count = mm.game.get_party_size()
    local message = "Party Conditions\n"
    for i = 1, count do
        local info = mm.game.get_character_info(i, { "condition", "name" })
        local condition_str = ""
        for key, value in pairs(info.condition) do
            condition_str = condition_str..condition_enum_to_string(key).." "
        end

        if isempty(condition_str) then
            condition_str = "None"
        end

        message = message..info.name..": "..condition_str.."\n"
    end
    return message, true
end

local function set_condition(condition, character_index)
    print(character_index)
    if character_index == nil then
        character_index = mm.game.get_active_character()
    end

    mm.game.set_character_info(character_index, { condition = CharacterCondition[condition] })
    local info = mm.game.get_character_info(character_index, { "name" })

    local message = "Set condition "..condition.." to "..info.name
    return message, true
end

local subcommands = {
    get = show_party_conditions,
    set = set_condition,
    default = show_party_conditions
}

return {
    name = "condition",
    description = "Change the conditions of the party members.",
    details = "",
    callback = subcommands
}
