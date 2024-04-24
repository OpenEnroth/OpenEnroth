local Utilities = require "dev.commands.command_utils"
local game = requireGame()

local subCommands = {
    get = Utilities.showProperty(game.party.getAlignment, "alignment", game.serialize.partyAlignment),
    set = Utilities.changeProperty(game.party.getAlignment, game.party.setAlignment, Utilities.opType.set, "alignment",
        game.deserialize.partyAlignment, game.serialize.partyAlignment),
    default = Utilities.showProperty(game.party.getAlignment, "alignment", game.serialize.partyAlignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
