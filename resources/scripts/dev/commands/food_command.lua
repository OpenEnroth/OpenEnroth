local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local subCommands = {
    get = Utilities.showProperty(Game.party.getFood, "food"),
    set = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.set, "food", tonumber),
    add = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.add, "food", tonumber),
    rem = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.rem, "food", tonumber),
    default = Utilities.showProperty(Game.party.getFood, "food")
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    callback = subCommands
}
