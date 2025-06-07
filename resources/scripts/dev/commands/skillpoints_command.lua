local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local function addSpParty(skillPoints)
    local addSpToChar = Utilities.changeCharProperty("sp", Utilities.opType.add, true, tonumber)
    local count = Game.party.getPartySize()
    local message = ""
    local allCallSucceeded = true
    for i = 1, count do
        local msg, isSuccess = addSpToChar(skillPoints, i)
        message = message .. msg .. "\n"
        allCallSucceeded = allCallSucceeded and isSuccess
    end
    return message, allCallSucceeded
end

local subCommands = {
    {
        name = "get",
        callback = Utilities.showCharsProperty("sp"),
        description = "Shows the current skill points of all characters in the party."
    },
    {
        name = "rem",
        callback = Utilities.changeCharProperty("sp", Utilities.opType.rem, true, tonumber),
        params = {
            { name = "sp",   type = "number",         optional = false, description = "Amount of skill points to remove." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to remove skill points from." }
        },
        description = "Removes the specified amount of skill points from the character."
    },
    {
        name = "add",
        callback = Utilities.changeCharProperty("sp", Utilities.opType.add, true, tonumber),
        params = {
            { name = "sp",   type = "number",         optional = false, description = "Amount of skill points to add." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to add skill points to." }
        },
        description = "Adds the specified amount of skill points to the character."
    },
    {
        name = "set",
        callback = Utilities.changeCharProperty("sp", Utilities.opType.set, true, tonumber),
        params = {
            { name = "sp",   type = "number",         optional = false, description = "Amount of skill points to set." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to set skill points for." }
        },
        description = "Sets the character's skill points to the specified value."
    },
    {
        name = "add_party",
        callback = addSpParty,
        params = {
            { name = "sp", type = "number", optional = false, description = "Amount of skill points to add to all characters in the party." }
        },
        description = "Adds the specified amount of skill points to all characters in the party."
    }
}

return {
    name = "sp",
    description = "Change the amount of skill points for the party.",
    details = "",
    subCommands = subCommands
}
