local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local function add_sp_party(skill_points)
    local add_sp_to_char = utils.change_char_property("sp", utils.OP_TYPE.add, true, tonumber)
    local count = mm.party.get_party_size()
    local message = ""
    local all_call_succeded = true
    for i = 1, count do
        local msg, is_success = add_sp_to_char(skill_points, i)
        message = message..msg.."\n"
        all_call_succeded = all_call_succeded and is_success
    end
    return message, all_call_succeded
end

local subcommands = {
    get = utils.show_chars_property("sp"),
    rem = utils.change_char_property("sp", utils.OP_TYPE.rem, true, tonumber),
    add = utils.change_char_property("sp", utils.OP_TYPE.add, true, tonumber),
    set = utils.change_char_property("sp", utils.OP_TYPE.set, true, tonumber),
    add_party = add_sp_party,
    default = utils.show_chars_property("sp")
}

return {
    name = "sp",
    description = "Change the amount of skill points for the party.",
    details = "",
    callback = subcommands
}
