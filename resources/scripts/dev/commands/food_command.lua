local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local subCommands = {
    {
        name = "get",
        callback = Utilities.showProperty(Game.party.getFood, "food"),
        description = "Shows the current amount of food in the party."
    },
    {
        name = "add",
        callback = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.add, "food",
            tonumber),
        params = {
            { name = "food", type = "number", optional = false, description = "Amount of food to add." }
        },
        description = "Adds the specified amount of food to the party."
    },
    {
        name = "rem",
        callback = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.rem, "food",
            tonumber),
        params = {
            { name = "food", type = "number", optional = false, description = "Amount of food to remove." }
        },
        description = "Removes the specified amount of food from the party."
    },
    {
        name = "set",
        callback = Utilities.changeProperty(Game.party.getFood, Game.party.setFood, Utilities.opType.set, "food",
            tonumber),
        params = {
            { name = "food", type = "number", optional = false, description = "Amount of food to set." }
        },
        description = "Sets the party's food to the specified value."
    }
}

return {
    name = "food",
    description = "Change the amount of food.",
    details = "",
    subCommands = subCommands
}
