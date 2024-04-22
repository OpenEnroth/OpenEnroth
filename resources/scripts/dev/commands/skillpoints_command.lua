local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local function addSpParty(skillPoints)
    local addSpToChar = Utilities.changeCharProperty("sp", Utilities.opType.add, true, tonumber)
    local count = MM.party.getPartySize()
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
    get = Utilities.showCharsProperty("sp"),
    rem = Utilities.changeCharProperty("sp", Utilities.opType.rem, true, tonumber),
    add = Utilities.changeCharProperty("sp", Utilities.opType.add, true, tonumber),
    set = Utilities.changeCharProperty("sp", Utilities.opType.set, true, tonumber),
    add_party = addSpParty,
    default = Utilities.showCharsProperty("sp")
}

return {
    name = "sp",
    description = "Change the amount of skill points for the party.",
    details = "",
    callback = subCommands
}
