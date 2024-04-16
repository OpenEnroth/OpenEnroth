local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local subcommands = {
    get = utils.show_property(mm.game.get_gold, "gold"),
    set = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.set, "gold", tonumber),
    add = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.add, "gold", tonumber),
    rem = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.rem, "gold", tonumber),
    default = utils.show_property(mm.game.get_gold, "gold")
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    callback = subcommands
}