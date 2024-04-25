local Utilities = require "dev.commands.command_utils"
local game = require "core.game".bindings

local subCommands = {
    get = Utilities.showProperty(game.party.getFood, "food"),
    set = Utilities.changeProperty(game.party.getFood, game.party.setFood, Utilities.opType.set, "food", tonumber),
    add = Utilities.changeProperty(game.party.getFood, game.party.setFood, Utilities.opType.add, "food", tonumber),
    rem = Utilities.changeProperty(game.party.getFood, game.party.setFood, Utilities.opType.rem, "food", tonumber),
    default = Utilities.showProperty(game.party.getFood, "food")
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    callback = subCommands
}
