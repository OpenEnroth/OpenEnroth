local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.getAlignment, "alignment", MM.serialize.partyAlignment),
    set = Utilities.changeProperty(MM.party.getAlignment, MM.party.setAlignment, Utilities.opType.set, "alignment",
        MM.deserialize.partyAlignment, MM.serialize.partyAlignment),
    default = Utilities.showProperty(MM.party.getAlignment, "alignment", MM.serialize.partyAlignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
