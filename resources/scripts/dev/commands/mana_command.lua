local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local function showCharsMana()
    local count = MM.party.getPartySize()
    local message = ""
    for i = 1, count do
        local info = MM.party.getCharacterInfo(i, { "name", "mana", "max_mana" })
        message = message .. info.name .. ": " .. info.mana .. "/" .. info.max_mana .. "\n"
    end
    return message, true
end

local subCommands = {
    get = showCharsMana,
    rem = Utilities.changeCharProperty("mana", Utilities.opType.rem, true, tonumber),
    add = Utilities.changeCharProperty("mana", Utilities.opType.add, true, tonumber),
    set = Utilities.changeCharProperty("mana", Utilities.opType.set, true, tonumber),
    default = showCharsMana
}

return {
    name = "mana",
    description = "Show and manipulate characters mana.",
    details = "",
    callback = subCommands
}
