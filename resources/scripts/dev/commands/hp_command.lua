local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local function show_chars_hp()
    local count = mm.game.get_party_size()
    local message = ""
    for i = 1, count do
        local info = mm.game.get_character_info(i, { "name", "hp", "max_hp" })
        message = message..info.name..": "..info.hp.."/"..info.max_hp.."\n"
    end    
    return message, true
end

local subcommands = {
    get = show_chars_hp,
    rem = utils.change_char_property("hp", utils.OP_TYPE.rem, true, tonumber),
    add = utils.change_char_property("hp", utils.OP_TYPE.add, true, tonumber),
    set = utils.change_char_property("hp", utils.OP_TYPE.set, true, tonumber),
    default = show_chars_hp
}

return {
    name = "hp",
    description = "Show and manipulate characters hp.",
    details = "",
    callback = subcommands
}
