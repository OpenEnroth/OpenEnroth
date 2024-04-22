local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.getFood, "food"),
    set = Utilities.changeProperty(MM.party.getFood, MM.party.setFood, Utilities.opType.set, "food", tonumber),
    add = Utilities.changeProperty(MM.party.getFood, MM.party.setFood, Utilities.opType.add, "food", tonumber),
    rem = Utilities.changeProperty(MM.party.getFood, MM.party.setFood, Utilities.opType.rem, "food", tonumber),
    default = Utilities.showProperty(MM.party.getFood, "food")
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    callback = subCommands
}
