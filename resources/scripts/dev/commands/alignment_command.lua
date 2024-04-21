local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local subCommands = {
    get = Utilities.showProperty(MM.party.get_alignment, "alignment", MM.serialize.party_alignment),
    set = Utilities.changeProperty(MM.party.get_alignment, MM.party.set_alignment, Utilities.opType.set, "alignment",
        MM.deserialize.party_alignment, MM.serialize.party_alignment),
    default = Utilities.showProperty(MM.party.get_alignment, "alignment", MM.serialize.party_alignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
