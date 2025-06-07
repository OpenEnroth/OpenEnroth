local CommandUtils = require "dev.commands.command_utils"
local Game = require "bindings.game"
local Utilities = require "utils"

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
    {
        name = "get",
        callback = CommandUtils.showProperty(getter, "alignment", serializer),
        description = "Shows the current alignment of the party."
    },
    {
        name = "set",
        callback = CommandUtils.changeProperty(getter, setter, CommandUtils.opType.set, "alignment", deserializer,
            serializer),
        params = {
            {
                name = "alignment",
                type = "enum",
                enumValues = Game.PartyAlignment,
                optional = false,
                defaultValue = function ()
                    return Utilities.getKeyByValue(Game.PartyAlignment, Game.party.getAlignment())
                end,
                description = "Alignment to set."
            }
        },
        description = "Sets the party's alignment to the specified value."
    },
    {
        name = "cycle",
        callback = cycleAlignment,
        description = "Cycles the party's alignment through Good, Neutral, and Evil."
    }
}

return {
    name = "alignment",
    description = "Change the alignment of the party.",
    details = "",
    subCommands = subCommands
}
