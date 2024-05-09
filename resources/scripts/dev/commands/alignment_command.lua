local CommandUtils = require "dev.commands.command_utils"
local Utilities = require "utils"
local Game = require "bindings.game"

local function cycleAlignment()
    local alignment = Game.party.getAlignment()
    if alignment == Game.PartyAlignment.Good then
        alignment = Game.PartyAlignment.Neutral
        Game.party.setQBit(Game.QBits.LightPath, false)
    elseif alignment == Game.PartyAlignment.Neutral then
        alignment = Game.PartyAlignment.Evil
        Game.party.setQBit(Game.QBits.DarkPath, true)
        Game.party.setQBit(Game.QBits.LightPath, false)
    elseif alignment == Game.PartyAlignment.Evil then
        alignment = Game.PartyAlignment.Good
        Game.party.setQBit(Game.QBits.DarkPath, false)
        Game.party.setQBit(Game.QBits.LightPath, true)
    end
    Game.party.setAlignment(alignment)
    return string.format("Alignment changed to %s", Utilities.enumToString(Game.PartyAlignment, alignment)), true
end

local subCommands = {
    get = CommandUtils.showProperty(Game.party.getAlignment, "alignment", Game.serialize.partyAlignment),
    set = CommandUtils.changeProperty(Game.party.getAlignment, Game.party.setAlignment, CommandUtils.opType.set,
        "alignment",
        Game.deserialize.partyAlignment, Game.serialize.partyAlignment),
    cycle = cycleAlignment,
    default = CommandUtils.showProperty(Game.party.getAlignment, "alignment", Game.serialize.partyAlignment)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
