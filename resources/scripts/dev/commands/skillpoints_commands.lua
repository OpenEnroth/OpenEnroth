local utils = require "dev.commands.command_utils"

local sp_commands = {
    get = utils.show_chars_property("sp"),
    rem = utils.change_char_property("sp", utils.OP_TYPE.rem, true, tonumber),
    add = utils.change_char_property("sp", utils.OP_TYPE.add, true, tonumber),
    set = utils.change_char_property("sp", utils.OP_TYPE.set, true, tonumber),
    default = utils.show_chars_property("sp")
}

return sp_commands