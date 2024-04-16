local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local subcommands = {
    get = utils.show_property(mm.game.get_alignment, "alignment", mm.serialize.party_alignment),
    set = utils.change_property(mm.game.get_alignment, mm.game.set_alignment, utils.OP_TYPE.set, "alignment", mm.deserialize.party_alignment, mm.serialize.party_alignment),
    default = utils.show_property(mm.game.get_alignment, "alignment", mm.serialize.party_alignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subcommands
}
