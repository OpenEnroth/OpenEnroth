local CommandUtils = require "dev.commands.command_utils"
local Game = require "bindings.game"

local function cycleAlignment()
    local alignment = Game.party.getAlignment()
    if alignment == Game.PartyAlignment.Good then
        alignment = Game.PartyAlignment.Neutral
        Game.party.setQBit(Game.QBits.DarkPath, false)
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
    return
        string.format("Alignment changed to %s", enumToString(Game.PartyAlignment, alignment)),
        true
end

local getter = Game.party.getAlignment
local setter = Game.party.setAlignment

local serializer = function (value)
    return enumToString(Game.PartyAlignment, value)
end

local deserializer = function (strValue)
    return stringToEnum(Game.PartyAlignment, strValue)
end

local subCommands = {
    get = CommandUtils.showProperty(getter, "alignment", serializer),
    set = CommandUtils.changeProperty(getter, setter, CommandUtils.opType.set, "alignment", deserializer, serializer),
    cycle = cycleAlignment,
    default = CommandUtils.showProperty(getter, "alignment", serializer)
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    callback = subCommands
}
