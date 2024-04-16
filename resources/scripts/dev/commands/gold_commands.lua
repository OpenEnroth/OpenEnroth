local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local gold_commands = {
    get = utils.show_property(mm.game.get_gold, "gold"),
    set = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.set, "gold", tonumber),
    add = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.add, "gold", tonumber),
    rem = utils.change_property(mm.game.get_gold, mm.game.set_gold, utils.OP_TYPE.rem, "gold", tonumber),
    default = utils.show_property(mm.game.get_gold, "gold")
}

return gold_commands