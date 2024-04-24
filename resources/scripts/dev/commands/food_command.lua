local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.get_food, "food"),
    set = Utilities.changeProperty(MM.party.get_food, MM.party.set_food, Utilities.opType.set, "food", tonumber),
    add = Utilities.changeProperty(MM.party.get_food, MM.party.set_food, Utilities.opType.add, "food", tonumber),
    rem = Utilities.changeProperty(MM.party.get_food, MM.party.set_food, Utilities.opType.rem, "food", tonumber),
    default = Utilities.showProperty(MM.party.get_food, "food")
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    callback = subCommands
}
