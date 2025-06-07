local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local subCommands = {
    {
        name = "get",
        callback = Utilities.showProperty(Game.party.getGold, "gold"),
        description = "Shows the current amount of gold in the party."
    },
    {
        name = "add",
        callback = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.add, "gold",
            tonumber),
        params = {
            { name = "gold", type = "number", optional = false, description = "Amount of gold to add." }
        },
        description = "Adds the specified amount of gold to the party."
    },
    {
        name = "rem",
        callback = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.rem, "gold",
            tonumber),
        params = {
            { name = "gold", type = "number", optional = false, description = "Amount of gold to remove." }
        },
        description = "Removes the specified amount of gold from the party."
    },
    {
        name = "set",
        callback = Utilities.changeProperty(Game.party.getGold, Game.party.setGold, Utilities.opType.set, "gold",
            tonumber),
        params = {
            { name = "gold", type = "number", optional = false, description = "Amount of gold to set." }
        },
        description = "Sets the party's gold to the specified value."
    }
}

return {
    name = "gold",
    description = "Change the amount of gold.",
    details = "",
    subCommands = subCommands
}
