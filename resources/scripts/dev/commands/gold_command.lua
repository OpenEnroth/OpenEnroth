local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.getGold, "gold"),
    set = Utilities.changeProperty(MM.party.getGold, MM.party.setGold, Utilities.opType.set, "gold", tonumber),
    add = Utilities.changeProperty(MM.party.getGold, MM.party.setGold, Utilities.opType.add, "gold", tonumber),
    rem = Utilities.changeProperty(MM.party.getGold, MM.party.setGold, Utilities.opType.rem, "gold", tonumber),
    default = Utilities.showProperty(MM.party.getGold, "gold")
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    callback = subCommands
}
