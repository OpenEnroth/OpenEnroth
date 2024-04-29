local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local subCommands = {
    get = Utilities.showProperty(Game.party.getAlignment, "alignment", Game.serialize.partyAlignment),
    set = Utilities.changeProperty(Game.party.getAlignment, Game.party.setAlignment, Utilities.opType.set, "alignment",
        Game.deserialize.partyAlignment, Game.serialize.partyAlignment),
    default = Utilities.showProperty(Game.party.getAlignment, "alignment", Game.serialize.partyAlignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
