local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local food_commands = {
    get = utils.show_property(mm.game.get_food, "food"),
    set = utils.change_property(mm.game.get_food, mm.game.set_food, utils.OP_TYPE.set, "food", tonumber),
    add = utils.change_property(mm.game.get_food, mm.game.set_food, utils.OP_TYPE.add, "food", tonumber),
    rem = utils.change_property(mm.game.get_food, mm.game.set_food, utils.OP_TYPE.rem, "food", tonumber),
    default = utils.show_property(mm.game.get_food, "food")
}

return food_commands