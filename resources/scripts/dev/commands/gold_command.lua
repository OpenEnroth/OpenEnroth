local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local subCommands = {
    get = Utilities.showProperty(Game.party.getGold, "gold"),
    set = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.set, "gold", tonumber),
    add = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.add, "gold", tonumber),
    rem = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.rem, "gold", tonumber),
    default = Utilities.showProperty(Game.party.getGold, "gold")
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    callback = subCommands
}
