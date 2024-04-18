local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local function show_chars_hp()
    local count = mm.party.get_party_size()
    local message = ""
    for i = 1, count do
        local info = mm.party.get_character_info(i, { "name", "hp", "max_hp" })
        message = message..info.name..": "..info.hp.."/"..info.max_hp.."\n"
    end    
    return message, true
end

local function full_heal(char_index)
    if char_index == nil then
        char_index = mm.party.get_active_character()
    end

    local info = mm.party.get_character_info(char_index, { "name", "max_hp", "max_mana" })
    mm.party.clear_condition(char_index)
    mm.party.set_character_info(char_index, {
        hp = info.max_hp,
        mana = info.max_mana
    })
    return info.name.."'s hp and mana are restored.", true
end

local subcommands = {
    get = show_chars_hp,
    rem = utils.change_char_property("hp", utils.OP_TYPE.rem, false, tonumber),
    add = utils.change_char_property("hp", utils.OP_TYPE.add, false, tonumber),
    set = utils.change_char_property("hp", utils.OP_TYPE.set, false, tonumber),
    full_heal = full_heal,
    default = show_chars_hp
}

return {
    name = "hp",
    description = "Show and manipulate characters hp.",
    details = "",
    callback = subcommands
}
