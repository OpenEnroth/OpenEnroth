local Utilities = require "dev.commands.command_utils"
local game = requireGame()

local subCommands = {
    get = Utilities.showProperty(game.party.getGold, "gold"),
    set = Utilities.changeProperty(game.party.getGold, game.party.setGold, Utilities.opType.set, "gold", tonumber),
    add = Utilities.changeProperty(game.party.getGold, game.party.setGold, Utilities.opType.add, "gold", tonumber),
    rem = Utilities.changeProperty(game.party.getGold, game.party.setGold, Utilities.opType.rem, "gold", tonumber),
    default = Utilities.showProperty(game.party.getGold, "gold")
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    callback = subCommands
}
