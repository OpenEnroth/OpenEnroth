local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local subcommands = {
    get = utils.show_property(mm.party.get_food, "food"),
    set = utils.change_property(mm.party.get_food, mm.party.set_food, utils.OP_TYPE.set, "food", tonumber),
    add = utils.change_property(mm.party.get_food, mm.party.set_food, utils.OP_TYPE.add, "food", tonumber),
    rem = utils.change_property(mm.party.get_food, mm.party.set_food, utils.OP_TYPE.rem, "food", tonumber),
    default = utils.show_property(mm.party.get_food, "food")
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    callback = subcommands
}
