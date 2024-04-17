local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local function show_chars_mana()
    local count = mm.game.get_party_size()
    local message = ""
    for i = 1, count do
        local info = mm.game.get_character_info(i, { "name", "mana", "max_mana" })
        message = message..info.name..": "..info.mana.."/"..info.max_mana.."\n"
    end    
    return message, true
end

local subcommands = {
    get = show_chars_mana,
    rem = utils.change_char_property("mana", utils.OP_TYPE.rem, true, tonumber),
    add = utils.change_char_property("mana", utils.OP_TYPE.add, true, tonumber),
    set = utils.change_char_property("mana", utils.OP_TYPE.set, true, tonumber),
    default = show_chars_mana
}

return {
    name = "mana",
    description = "Show and manipulate characters mana.",
    details = "",
    callback = subcommands
}
