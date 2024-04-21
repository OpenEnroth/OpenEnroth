local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.get_gold, "gold"),
    set = Utilities.changeProperty(MM.party.get_gold, MM.party.set_gold, Utilities.opType.set, "gold", tonumber),
    add = Utilities.changeProperty(MM.party.get_gold, MM.party.set_gold, Utilities.opType.add, "gold", tonumber),
    rem = Utilities.changeProperty(MM.party.get_gold, MM.party.set_gold, Utilities.opType.rem, "gold", tonumber),
    default = Utilities.showProperty(MM.party.get_gold, "gold")
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    callback = subCommands
}
